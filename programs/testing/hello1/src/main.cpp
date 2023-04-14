#include <syscall.h>
#include <cstring.h>
#include <iostream.h>

using namespace std;

int main()
{
	for (int i = 0; i < 80; i++)
	{
		cout << '1';
		Time::sleep(100);
	}
	return 0;
}