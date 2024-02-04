#include "thread.h"

Thread::Thread(Task *parentTask, const registers_t &regs, byte *stack)
	: parentTask(parentTask), regs(regs), stack(stack)
{
	// if the main thread is not set yet, set to this
	if (parentTask->mainThread == nullptr)
		parentTask->mainThread = this;

	parentTask->threadCount++;

	// std::cout << "Created thread at " << (void *)this << '\n';
}
Thread::~Thread()
{
	if (stack)
		delete[] stack;

	if (--parentTask->threadCount == 0)
		delete parentTask;
}

bool Thread::IsMainThread() { return parentTask->mainThread == this; }