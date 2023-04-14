#include "scheduler.h"
#include "../utils/time.h"
#include <vector.h>
#include "../cpu/gdt.h"

using namespace std;

class SleepingTaskInfo : public Task::TaskInfo // info about a task that is sleeping
{
public:
	inline SleepingTaskInfo(ull sleepUntil) : TaskInfo(sleepUntil) {}
	inline SleepingTaskInfo(TaskInfo info) : TaskInfo(info) {}

	inline bool shouldWake(ull currTime) { return currTime >= data; }
};

enum class WaitCondition
{
	task,
	irq,
};
class TaskWaitingTaskInfo : public Task::TaskInfo // info about a task that waits for another task to finish
{
public:
	inline TaskWaitingTaskInfo(Task *task) : TaskInfo(((ull)WaitCondition::task << 56) | (ull)task & 0xffffffffffffff) {}

	inline bool operator==(TaskInfo info) { return data == info.data; }
};
class IrqWaitingTaskInfo : public Task::TaskInfo // info about a task that waits for an irq
{
public:
	inline IrqWaitingTaskInfo(int irq_no) : TaskInfo(((ull)WaitCondition::irq << 56) | irq_no) {}

	inline bool operator==(TaskInfo info) { return data == info.data; }
};

namespace Scheduler
{
	static constexpr ull noExecutingTask = (ull)-1;
	static constexpr int preempt_interval = 5;
	word preempt_timer;

	vector<Task *> *executingTasks;
	vector<Task *> *sleepingTasks;
	vector<Task *> *waitingTasks;
	ull currentTask;

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

	void Initialize(Task *terminalTask)
	{
		// terminal task is currently always running, so an idle task is not needed

		executingTasks = new vector<Task *>();
		sleepingTasks = new vector<Task *>();
		waitingTasks = new vector<Task *>();
		executingTasks->push_back(terminalTask);
		currentTask = 0;

		enable();
	}
	void CleanUp()
	{
		disable();

		delete executingTasks;
		if (sleepingTasks->getSize() > 0)
			cout << "oh no, sleeping\n";
		delete sleepingTasks;
		if (waitingTasks->getSize() > 0)
			cout << "oh no, blocked\n";
		delete waitingTasks;
	}

	void add(Task *task)
	{
		disableInterrupts();
		executingTasks->push_back(task);
		enableInterrupts();
	}

	void tick(registers_t &regs)
	{
		if (!enabled)
			return;

		// go through sleeping tasks and see if any are to be waked up
		bool awakened = false;
		ull currTime = Time::time();
		for (ull i = sleepingTasks->getSize() - 1; i != (ull)-1; i--)
		{
			Task *task = sleepingTasks->at(i);
			if (SleepingTaskInfo(task->taskInfo).shouldWake(currTime))
			{
				sleepingTasks->erase(i);
				executingTasks->push_back(task);
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
	}

	void preempt(registers_t &regs, preemptReason reason)
	{
		if (!enabled)
			return;
		//  cycle through all tasks, or idle if there are none
		Task *current = getCurrentTask(); // get current task
		switch (reason)
		{
		case preemptReason::timeSliceEnded: // go to the next task
			currentTask++;
			break;
		case preemptReason::startedSleeping: // move task from executing to sleeping list
			// currentTask++;
			executingTasks->erase(currentTask);
			sleepingTasks->push_back(current);
			break;
		case preemptReason::waitingIO: // move task from executing to io blocked list
			executingTasks->erase(currentTask);
			waitingTasks->push_back(current);
			break;
		case preemptReason::taskExited: // remove task from executing list
			executingTasks->erase(currentTask);
			break;
		}

		//  find the task to switch to, otherwise, 0
		ull size = executingTasks->getSize();
		if (currentTask >= size) // loopback the task list
		{
			currentTask = 0;
			if (currentTask == size) // no task in list
				currentTask = noExecutingTask;
		}
		Task *target = getCurrentTask();
		if (current != target)
		{
			if (target)
			{
				Task::switchContext(current, target, regs);
				if (regs.cs == GDT::USER_CS)
				{
					cout << "Entering user mode:\n"
							"cs:rip = "
						 << ostream::base::hex
						 << regs.cs << ':' << regs.rip << "\nss:rsp = " << regs.ss << ':' << regs.rsp << '\n'
						 << ostream::base::dec;
					cout << "regs:\n";
					DisplyMemoryBlock((byte *)&regs - 0x20, sizeof(regs) + 0x40);
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
			// check for tasks waiting for this task to finish
			TaskWaitingTaskInfo info(current);
			for (ull i = waitingTasks->getSize() - 1; i != (ull)-1; i--)
				if (info == waitingTasks->at(i)->taskInfo)
				{
					executingTasks->push_back(waitingTasks->at(i));
					waitingTasks->erase(i);
				}
			delete current;
		}
	}

	void sleep(registers_t &regs, ull untilTime)
	{
		getCurrentTask()->taskInfo = SleepingTaskInfo(untilTime);
		preempt(regs, preemptReason::startedSleeping);
		preempt_timer = preempt_interval;
	}
	void waitForIrq(registers_t &regs, int irq_no)
	{
		getCurrentTask()->taskInfo = IrqWaitingTaskInfo(irq_no);
		preempt(regs, preemptReason::waitingIO);
		preempt_timer = preempt_interval;
	}
	void waitForTaskUnchecked(registers_t &regs, Task *task)
	{
		getCurrentTask()->taskInfo = TaskWaitingTaskInfo(task);
		preempt(regs, preemptReason::waitingIO);
		preempt_timer = preempt_interval;
	}
	void waitForTask(registers_t &regs, Task *task)
	{
		// check that the task exists, do nothing otherwise
		for (auto &t : *executingTasks)
			if (task == t)
				return waitForTaskUnchecked(regs, task);
		for (auto &t : *sleepingTasks)
			if (task == t)
				return waitForTaskUnchecked(regs, task);
		for (auto &t : *waitingTasks)
			if (task == t)
				return waitForTaskUnchecked(regs, task);
	}

	inline Task *getCurrentTask() { return currentTask == noExecutingTask ? nullptr : executingTasks->at(currentTask); }
}