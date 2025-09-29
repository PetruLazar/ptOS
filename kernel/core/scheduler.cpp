#include "scheduler.h"
#include "../utils/time.h"
#include <vector.h>
#include "../cpu/gdt.h"
#include "../utils/isriostream.h"

using namespace std;

namespace Scheduler
{
	static constexpr ull noExecutingThread = (ull)-1;
	static constexpr int preempt_interval = 5; // aka time-slice in terms of IRQ0 interrupt count
	word preempt_timer;

	vector<Thread *> *executingThreads;
	vector<Thread *> *sleepingThreads; // possible optimization: keep this vector ordered, so that the next thread to be waked up is always [0]
	vector<Thread *> *waitingThreads;
	ull currentThread;

	bool enabled = false, idling = false;

	extern "C" void idleTask();

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

	void Initialize()
	{
		Task *kernelTask = new Task(true);
		Thread *kernelMainThread = new Thread(kernelTask, registers_t());

		executingThreads = new vector<Thread *>();
		sleepingThreads = new vector<Thread *>();
		waitingThreads = new vector<Thread *>();
		executingThreads->push_back(kernelMainThread);
		currentThread = 0;

		enable();
	}
	void CleanUp()
	{
		disable();

		// CleanUp is assumed to be called from kernalMainThread
		Thread *kernelMainThread = getCurrentThread();
		delete kernelMainThread;
		executingThreads->erase(currentThread);

		if (executingThreads->getSize() > 0)
			cout << "Executing threads left!\n";
		delete executingThreads;

		if (sleepingThreads->getSize() > 0)
			cout << "Sleeping threads left!\n";
		delete sleepingThreads;

		if (waitingThreads->getSize() > 0)
			cout << "Blocked threads left!\n";
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

		// check if any sleeping threads should be waked up
		ull currTime = Time::driver_time();

		// since the array of sleeping threads is ordered from soonest to latest to wake up
		// only check first threads until one still needs to sleep
		ull sleepingThreadCount = sleepingThreads->getSize(),
			wakeUpCount = 0;
		for (ull i = 0; i < sleepingThreadCount; i++)
		{
			Thread *thread = sleepingThreads->at(i);
			if (!thread->finishedSleeping(currTime))
				break; // encountered a thread that still needs to sleep, exit loop

			executingThreads->push_back(thread);
			wakeUpCount++;
		}

		// remove all threads that have been waked up from the sleeping list
		if (wakeUpCount)
			sleepingThreads->erase(0, wakeUpCount);

		preempt_timer--;
		if (!preempt_timer || (getCurrentThread() == nullptr && executingThreads->getSize() > 0))
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

	void wakeupBlockedThreads(Thread *blockingThread, int returnedValue)
	{
		for (ull i = waitingThreads->getSize() - 1; i != (ull)-1; i--)
		{
			Thread *thread = waitingThreads->at(i);
			if (thread->getBlocker() == blockingThread)
			{
				waitingThreads->erase(i);
				// if task is alive, move to executing threads list
				// otherwise, it is time for cleanup
				if (!thread->getParentTask()->isDead())
				{
					thread->getRegs().rax = returnedValue;
					executingThreads->push_back(thread);
				}
				else
				{
					delete thread;
				}
			}
		}
	}
	void kill(Task *task, int returnedValue)
	{
		// mark task as dead
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

		// wake up every thread waiting for task threads and cleanup
		for (auto *&thread : taskThreads)
		{
			wakeupBlockedThreads(thread, -1);
			delete thread;
		}

		// reset list of threads
		taskThreads.resize(0);

		for (ull i = waitingThreads->getSize() - 1; i != (ull)-1; i--)
		{
			Thread *thread = waitingThreads->at(i);
			if (thread->getParentTask() == task)
			{
				taskThreads.push_back(thread);
				// do not erase threads from here so that cleanup
				// can take place later, when blocking threads finished or unblocks
			}
		}

		// wake up every thread waiting for task threads, but skip cleanup
		for (auto *&thread : taskThreads)
		{
			wakeupBlockedThreads(thread, -1);
			// ignore cleanup because this might block on another thread
			// the blocking thread might be a thread that uses data from this task
			// by not cleaning up the thread now, we ensure that the thread's data is available
			// to the blocking thread, which might be a driver thread
			// only cleanup thread when it is unblocked
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
			executingThreads->erase(currentThread);
			// skip updating sleepingThreads, as it will be updated in
			// the sleep function, the caller of this one
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
			wakeupBlockedThreads(current, (int)current->getRegs().rdi);

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
		Thread *thread = getCurrentThread();
		thread->sleepUntil(untilTime);
		// the preempt function will NOT update sleepingThreads
		preempt(regs, preemptReason::startedSleeping);

		// put thread in at the correct position
		ull pos, sleepingTheadCount = sleepingThreads->getSize();
		for (pos = 0; pos < sleepingTheadCount; pos++)
			if (sleepingThreads->at(pos)->finishedSleeping(untilTime) == false)
				break;

		sleepingThreads->insert(thread, pos);
		preempt_timer = preempt_interval;
	}
	bool waitForThreadUnchecked(registers_t &regs, Thread *thread)
	{
		getCurrentThread()->block(thread);
		preempt(regs, preemptReason::waitingIO);
		preempt_timer = preempt_interval;
		return true;
	}
	bool waitForThread(registers_t &regs, Thread *thread)
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

		// thread not found, blocking failed
		return false;
	}
	void unblockThread(registers_t &regs, Thread *blockingThread, Thread *blockedThread)
	{
		// do the actual unblocking
		ull waitingThreadsCount = waitingThreads->getSize();
		for (ull i = 0; i < waitingThreadsCount; i++)
		{
			Thread *waitingThread = waitingThreads->at(i);
			if (waitingThread == blockedThread && waitingThread->getBlocker() == blockingThread)
			{
				waitingThreads->erase(i);
				if (!blockedThread->getParentTask()->isDead())
				{
					// blockedThread is still alive
					executingThreads->push_back(blockedThread);
				}
				else
				{
					// blockedThread is dead (someone else killed it, or the main thread of it's task exited)
					// do clean-up
					delete blockedThread;
					return; // nothing else to do
				}
				break;
			}
		}

		// if cpu is idle, switch to blockedThread
		if (getCurrentThread() == nullptr)
			preempt(regs, preemptReason::timeSliceEnded);

		// do the cleanup if blockedThread is already dead
	}
	bool createThread(registers_t &regs)
	{
		Thread *parentThread = getCurrentThread();

		byte *stack = (byte *)Memory::Allocate(0x10000, 0x1000);
		// WIP:
		// stack will NOT be mapped into the virtual space of the parent task
		// this means that this function only works for kernel task, where the heap is identity mapped

		Thread *thread = new Thread(parentThread->getParentTask(), regs, stack);
	}

	Thread *getCurrentThread() { return currentThread == noExecutingThread ? nullptr : executingThreads->at(currentThread); }
}