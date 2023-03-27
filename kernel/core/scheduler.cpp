#include "scheduler.h"
#include <vector.h>

using namespace std;

namespace Scheduler
{
	vector<Task *> *tasks;
	ull currentTask;

	void Initialize(Task *terminalTask, Task *idleTask)
	{
		tasks = new vector<Task *>();
		tasks->push_back(idleTask);
		tasks->push_back(terminalTask);
		currentTask = 0;
	}
	void CleanUp()
	{
		
		delete tasks;
	}

	void add(Task *task)
	{
		disableInterrupts();
		tasks->push_back(task);
		enableInterrupts();
	}

	void preempt(registers_t &regs, bool erase)
	{
		//  cycle through all tasks, except [0], the idle task
		Task *current = tasks->at(currentTask); // get current task
		if (erase)
			// remove the task from the list
			tasks->erase(currentTask);
		else
			// go to the next task
			currentTask++;
		//  find the task to switch to, otherwise, 0
		ull taskCount = tasks->getSize();
		if (currentTask == taskCount)
		{
			currentTask = 1;
			if (currentTask == taskCount)
				currentTask = 0;
		}
		Task *target = tasks->at(currentTask);
		if (current != target)
			Task::switchContext(current, target, regs);
		if (erase)
			delete current;
	}
}