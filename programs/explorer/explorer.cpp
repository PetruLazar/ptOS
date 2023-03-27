#include "explorer.h"
#include <screen.h>
#include <filesystem.h>

using namespace std;

namespace Explorer
{
	void Start()
	{
		cout << "\nWelcome to PTOS File Explorer:\n";
		string16 currentDirectory = u"C:/";

		while (true)
		{
			// display current directory's contents
			ull count = 0;
			auto it = Filesystem::GetDirectoryIterator(currentDirectory);
			cout << "\nCurrently in \"" << currentDirectory << "\":\n";
			while (!it->finished())
			{
				cout << '\t' << ++count << ". " << it->getString();
				if (it->isFile())
					cout << " (" << it->getSize() << " bytes)\n";
				else
					cout << '\n';
				it->advance();
			}
			cout << '\n'
				 << '\t' << ++count << ". Remove entry\n"
				 << '\t' << ++count << ". New folder\n"
				 << '\t' << ++count << ". New file\n";

			// let the user choose an entry
			uint choice;
			cout << "\nChoose an option: ";
			cin >> choice;
			if (!choice)
			{
				delete it;
				break;
			}

			// navigate to that choice
			it->reset();
			choice--;
			bool remove = false;
			while (true)
			{
				if (it->finished())
				{
					if (!remove)
						switch (choice)
						{
						case 0:
							cout << "You chose to remove an entry.\n";
							remove = true;
							cout << "What to remove: ";
							cin >> choice;
							it->reset();
							choice--;
							continue;
						case 1:
						case 2:
						{
							cout << "You chose to create a new " << (choice == 1 ? "folder" : "file") << ".\nInsert name: ";
							string16 filename;
							cin >> filename;
							filename = currentDirectory + filename;
							bool success = (choice == 1 ? Filesystem::CreateDirectory(filename + u'/') : Filesystem::CreateFile(filename));
							if (!success)
								cout << "Operation failed.\n";
						}
						break;
						}
					cout << "Invalid choice.\n";
					remove = false;
					break;
				}
				if (choice == 0)
				{
					// this is the choice
					if (it->isDirectory())
					{
						string16 dirname = it->getString();
						if (dirname == u".")
						{
							if (remove)
								cout << "Cammpt remove \".\"\n";
						}
						else if (dirname == u"..")
						{
							if (remove)
								cout << "Cannot remove \"..\"\n";
							else
								do
								{
									currentDirectory.pop_back();
								} while (currentDirectory[currentDirectory.length() - 1] != u'/');
						}
						else
						{
							if (remove)
							{
								if (!Filesystem::RemoveDirectory(currentDirectory + it->getString() + u'/'))
									cout << "Failed to remove directory \"" << it->getString() << "\"\n";
								remove = false;
							}
							else
								currentDirectory += it->getString() + u'/';
						}
					}
					else
					{
						if (remove)
						{
							if (!Filesystem::RemoveFile(currentDirectory + it->getString()))
								cout << "Failed to remove file \"" << it->getString() << "\"\n";
							remove = false;
						}
						else
							cout << "You chose the file \"" << it->getString() << "\" (" << it->getSize() << " bytes)\n";
					}
					break;
				}
				it->advance();
				choice--;
			}

			delete it;
		}
		return;
	}
}