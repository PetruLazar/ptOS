#include "../core/filesystem/filesystem.h"
#include "../core/scheduler.h"
#include <iostream.h>

using namespace std;

void unittests_run()
{
    // run all tests in program/tests one by one
    auto iterator = Filesystem::GetDirectoryIterator(u"e:/programs/tests");
    if (!iterator)
    {
        cout << "Test directory not found.";
    }
    else
    {
        int failCount = 0;

        while (!iterator->finished())
        {
            if (iterator->isFile())
            {
                cout << "Running test " << iterator->getString() << ":\n";
                bool passed = true;
                Task* task = Task::createTask(u"e:/programs/tests/" + iterator->getString());
                if (task == nullptr)
                {
                    cout << "Failed to start test.\n\n";
                    passed = false;
                }
                else
                {
                    Scheduler::add(task->getMainThread());
                    int retVal = Scheduler::waitForTask(task);
                    if (retVal == -1)
                    {
                        cout << "\nTest module crashed.\n";
                    }
                    if (retVal != 0)
                        passed = false;
                }

                if (!passed)
                    failCount++;
            }

            iterator->advance();
        }

        cout << "All test modules executed: " << failCount << " test modules contained failures.\n";
    }
}