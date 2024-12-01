#include "scheduler.h"
#include "../utils/time.h"
#include <vector.h>
#include "../cpu/gdt.h"
#include "../utils/isriostream.h"

using namespace std;

class SleepingThreadInfo : public Thread::ThreadInfo // info about a task that is sleeping
{
public:
	inline SleepingThreadInfo(ull sleepUntil) : ThreadInfo(sleepUntil) {}
	inline SleepingThreadInfo(ThreadInfo info) : ThreadInfo(info) {}

	inline bool shouldWake(ull currTime) { return currTime >= data; }
};

enum class WaitCondition
{
	thread,
	task,
	irq,
};
class ThreadWaitingThreadInfo : public Thread::ThreadInfo // info about a thread that waits for another thread to finish
{
public:
	inline ThreadWaitingThreadInfo(Thread *thread) : ThreadInfo(((ull)WaitCondition::thread << 56) | ((ull)thread & 0xffffffffffffff)) {}

	inline bool operator==(ThreadInfo info) { return data == info.data; }
};
class IrqWaitingThreadInfo : public Thread::ThreadInfo // info about a thread that waits for an irq
{
public:
	inline IrqWaitingThreadInfo(int irq_no) : ThreadInfo(((ull)WaitCondition::irq << 56) | irq_no) {}

	inline bool operator==(ThreadInfo info) { return data == info.data; }
};
class TaskWaitingThreadInfo : public Thread::ThreadInfo // info about a thread that waits for a task
{
public:
	inline TaskWaitingThreadInfo(Task *task) : ThreadInfo(((ull)WaitCondition::task << 56) | ((ull)task & 0xffffffffffffff)) {}

	inline bool operator==(ThreadInfo info) { return data == info.data; }
};

namespace Scheduler
{
	static constexpr ull noExecutingThread = (ull)-1;
	static constexpr int preempt_interval = 5;
	word preempt_timer;

	vector<Thread *> *executingThreads;
	vector<Thread *> *sleepingThreads; // possible optimization: keep this vector ordered, so that the next thread to be waked up is always [0]
	vector<Thread *> *waitingThreads;
	ull currentThread;

	bool enabled = false, idling = false;

	extern "C" void idleTask();
	// registers_t idleTask;

	void enable()
	{
		preempt_timer = preempt_interval;
		enabled = true;
	}
	void disable()
	{
		enabled = false;
	}
	bool isEnabled() { return enabled; }

	void Initialize(Task *terminalTask)
	{
		// terminal task is currently always running, so an idle task is not needed

		executingThreads = new vector<Thread *>();
		sleepingThreads = new vector<Thread *>();
		waitingThreads = new vector<Thread *>();
		executingThreads->push_back(terminalTask->getMainThread());
		currentThread = 0;

		enable();
	}
	void CleanUp()
	{
		disable();

		delete executingThreads;
		if (sleepingThreads->getSize() > 0)
			cout << "oh no, sleeping tasks left\n";
		delete sleepingThreads;
		if (waitingThreads->getSize() > 0)
			cout << "oh no, blocked tasks left\n";
		delete waitingThreads;
	}

	void add(Thread *thread)
	{
		disableInterrupts();
		executingThreads->push_back(thread);
		enableInterrupts();
	}

	void tick(registers_t &regs)
	{
		if (!enabled)
			return;

		// go through sleeping tasks and see if any are to be waked up
		bool awakened = false;
		ull currTime = Time::driver_time();

		for (ull i = sleepingThreads->getSize() - 1; i != (ull)-1; i--)
		{
			Thread *thread = sleepingThreads->at(i);
			if (SleepingThreadInfo(thread->threadInfo).shouldWake(currTime))
			{
				sleepingThreads->erase(i);
				executingThreads->push_back(thread);
				awakened = true;
			}
		}

		preempt_timer--;
		if (!preempt_timer)
		{
			Scheduler::preempt(regs, preemptReason::timeSliceEnded);
			preempt_timer = preempt_interval;
		}
		if (idling)
		{
			regs.rip = (ull)idleTask;
			regs.cs = 0x8;
			regs.cr3 = &PageMapLevel4::getCurrent();
		}
	}
	void irqReceived(int irq_no)
	{
		if (!enabled)
			return;

		if (!irq_no)
			return; // IRQ0 is not handled here

		// go from 0 and go up, so the first function who started waiting will be awakened
		ull limit = waitingThreads->getSize();
		for (ull i = 0; i < limit; i++)
		{
			Thread *thread = waitingThreads->at(i);
			if (IrqWaitingThreadInfo(irq_no) == thread->threadInfo)
			{
				waitingThreads->erase(i);
				executingThreads->push_back(thread);
				return;
			}
		}
	}

	void wakeupBlockedThreads(Thread::ThreadInfo threadInfo, int returnedValue)
	{
		for (ull i = waitingThreads->getSize() - 1; i != (ull)-1; i--)
			if (threadInfo.data == waitingThreads->at(i)->threadInfo.data)
			{
				waitingThreads->at(i)->getRegs().rax = returnedValue;
				executingThreads->push_back(waitingThreads->at(i));
				waitingThreads->erase(i);
			}
	}
	void kill(Task *task, int returnedValue)
	{
		task->kill();

		// keep a list of threads belonging to the task
		vector<Thread *> taskThreads(8);

		// find the threads belonging to the task
		for (ull i = executingThreads->getSize() - 1; i != (ull)-1; i--)
		{
			Thread *thread = executingThreads->at(i);
			if (thread->getParentTask() == task)
			{
				taskThreads.push_back(thread);
				executingThreads->erase(i);
			}
		}
		for (ull i = sleepingThreads->getSize() - 1; i != (ull)-1; i--)
		{
			Thread *thread = sleepingThreads->at(i);
			if (thread->getParentTask() == task)
			{
				taskThreads.push_back(thread);
				sleepingThreads->erase(i);
			}
		}
		for (ull i = waitingThreads->getSize() - 1; i != (ull)-1; i--)
		{
			Thread *thread = waitingThreads->at(i);
			if (thread->getParentTask() == task)
			{
				taskThreads.push_back(thread);
				waitingThreads->erase(i);
			}
		}

		// wake up every thread waiting for task threads and cleanup
		for (auto *&t : taskThreads)
		{
			wakeupBlockedThreads(ThreadWaitingThreadInfo(t), -1);
			delete t;
		}
	}
	void kill(Thread *thread, int returnValue)
	{
		cout << "kill(Thread*, int) not supported";
	}
	void preempt(registers_t &regs, preemptReason reason)
	{
		if (!enabled)
			return;

		//  cycle through all threads, or idle if there are none
		Thread *current = getCurrentThread(); // get current thread
		switch (reason)
		{
		case preemptReason::timeSliceEnded: // go to the next task
			currentThread++;
			break;
		case preemptReason::startedSleeping: // move task from executing to sleeping list
			// currentTask++;
			executingThreads->erase(currentThread);
			sleepingThreads->push_back(current);
			break;
		case preemptReason::waitingIO: // move task from executing to io blocked list
			executingThreads->erase(currentThread);
			waitingThreads->push_back(current);
			break;
		case preemptReason::taskExited: // remove task from executing list
			executingThreads->erase(currentThread);
			break;
		}

		//  find the task to switch to, otherwise, 0
		ull size = executingThreads->getSize();
		if (currentThread >= size) // loop back the thread list
		{
			currentThread = 0;
			if (currentThread == size) // no thread in list
				currentThread = noExecutingThread;
		}
		Thread *target = getCurrentThread();
		if (current != target)
		{
			if (target)
			{
				Thread::switchContext(current, target, regs);
				if (regs.cs == GDT::USER_CS)
				{
					ISR::std::cout << "Entering user mode:\n"
									  "cs:rip = "
								   << ostream::base::hex
								   << regs.cs << ':' << regs.rip << "\nss:rsp = " << regs.ss << ':' << regs.rsp << '\n'
								   << ostream::base::dec;
					ISR::std::cout << "regs:\n";
					isr_DisplyMemoryBlock((byte *)&regs - 0x20, sizeof(regs) + 0x40);
				}
				if (!current)
					idling = false;
			}
			else
			{
				if (current)
					current->getRegs() = regs;
				idling = true;
				regs.rip = (ull)idleTask;
				regs.cs = GDT::KERNEL_CS;
				regs.ss = GDT::KERNEL_DS;
				regs.cr3 = &PageMapLevel4::getCurrent();
			}
		}
		if (reason == preemptReason::taskExited)
		{
			// check for threads waiting for this thread to finish
			wakeupBlockedThreads(ThreadWaitingThreadInfo(current), (int)current->getRegs().rdi);

			// if thread is main thread
			Task *parentTask = current->getParentTask();
			if (parentTask->getMainThread() == current)
				kill(parentTask, (int)current->getRegs().rdi);

			// clean up thread
			delete current;
		}
	}
	// wakes up all threads waiting for a thread or task specified by threadInfo

	void sleep(registers_t &regs, ull untilTime)
	{
		getCurrentThread()->threadInfo = SleepingThreadInfo(untilTime);
		preempt(regs, preemptReason::startedSleeping);
		preempt_timer = preempt_interval;
	}
	// WARNING: only ONE task will be awakened if multiple tasks are waiting for the same irq
	// cannot wait for irq 0
	void waitForIrq(registers_t &regs, IDT::Irq_no irq_no)
	{
		if (irq_no == IDT::Irq_no::timer)
			return;
		getCurrentThread()->threadInfo = IrqWaitingThreadInfo((int)irq_no);
		preempt(regs, preemptReason::waitingIO);
		preempt_timer = preempt_interval;
	}
	void waitForThreadUnchecked(registers_t &regs, Thread *thread)
	{
		getCurrentThread()->threadInfo = ThreadWaitingThreadInfo(thread);
		preempt(regs, preemptReason::waitingIO);
		preempt_timer = preempt_interval;
	}
	void waitForThread(registers_t &regs, Thread *thread)
	{
		// check that the task exists, do nothing otherwise
		for (auto &t : *executingThreads)
			if (thread == t)
				return waitForThreadUnchecked(regs, thread);
		for (auto &t : *sleepingThreads)
			if (thread == t)
				return waitForThreadUnchecked(regs, thread);
		for (auto &t : *waitingThreads)
			if (thread == t)
				return waitForThreadUnchecked(regs, thread);
	}

	Thread *getCurrentThread() { return currentThread == noExecutingThread ? nullptr : executingThreads->at(currentThread); }
}