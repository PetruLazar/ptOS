#include <iostream.h>
#include <syscall.h>

using namespace std;

int main()
{
	while (true)
	{
		Keyboard::KeyCode keyCode = Keyboard::getKeyPressedEvent().getKeyCode();

		if (keyCode == Keyboard::KeyCode::Escape)
			break;

		cout << '2';
	}
	cout << '\n';
	return 0;
}