#include <syscall.h>
#include <rand.h>
#include <iostream.h>

enum class Direction : byte
{
	up,
	down,
	left,
	right,
};
enum class Cell : byte
{
	air,
	apple,
	snakeUp,
	snakeDown,
	snakeLeft,
	snakeRight,
	snakeHead
};

Direction currentDirection = Direction::right;
// Cell arena[Screen::screenHeight][Screen::screenWidth];
Cell **arena = nullptr;
Vector2b snakeHead(0, 0), snakeTail(0, 0);
bool gameOver = false;

void spawnApple()
{
	byte x, y;
	do
	{
		uint r = (uint)Random::get();
		x = r % Screen::screenWidth;
		y = r / Screen::screenWidth % Screen::screenHeight;
	} while (arena[y][x] != Cell::air);
	arena[y][x] = Cell::apple;
}
void Build()
{
	for (byte x = 0; x < 80; x++)
	{
		for (byte y = 0; y < 25; y++)
		{
			arena[y][x] = Cell::air;
		}
	}
	// place apples
	for (int i = 0; i < 80 * 25 / 10; i++)
		spawnApple();

	// place 3 snake segments
	arena[17][15] = Cell::snakeRight;
	arena[17][16] = Cell::snakeRight;
	arena[17][17] = Cell::snakeHead;

	snakeHead.y = 17;
	snakeHead.x = 17;

	snakeTail.y = 17;
	snakeTail.x = 15;

	currentDirection = Direction::right;
}
void Redraw()
{
	// Screen::paint(Vector2b(0, 0), Vector2b(79, 24), Screen::Cell::Color(Screen::Cell::Color::black));
	for (byte x = 0; x < 80; x++)
		for (byte y = 0; y < 25; y++)
		{
			switch (arena[y][x])
			{
			case Cell::air:
				Screen::paint(y, x, Screen::Cell::black);
				break;
			case Cell::apple:
				Screen::paint(y, x, Screen::Cell::red);
				break;
			default:
				Screen::paint(y, x, Screen::Cell::green);
			}
		}
}
void Cycle()
{
	// work on old position
	// update position and wrap around
	// do work on new position
	// update tail if needed

	bool hasEaten = false;
	switch (currentDirection)
	{
	case Direction::up:
		arena[snakeHead.y][snakeHead.x] = Cell::snakeUp;
		snakeHead.y--;
		if (snakeHead.y == 0xff)
			snakeHead.y = Screen::screenHeight - 1;
		break;
	case Direction::down:
		arena[snakeHead.y][snakeHead.x] = Cell::snakeDown;
		snakeHead.y++;
		if (snakeHead.y == Screen::screenHeight)
			snakeHead.y = 0;
		break;
	case Direction::left:
		arena[snakeHead.y][snakeHead.x] = Cell::snakeLeft;
		snakeHead.x--;
		if (snakeHead.x == 0xff)
			snakeHead.x = Screen::screenWidth - 1;
		break;
	case Direction::right:
		arena[snakeHead.y][snakeHead.x] = Cell::snakeRight;
		snakeHead.x++;
		if (snakeHead.x == Screen::screenWidth)
			snakeHead.x = 0;
		break;
	}

	switch (arena[snakeHead.y][snakeHead.x])
	{
	case Cell::air:
		break;
	case Cell::apple:
		hasEaten = true;
		break;
	default:
		gameOver = true;
		return;
	}

	if (hasEaten)
		spawnApple();
	else
	{
		Cell oldCell = arena[snakeTail.y][snakeTail.x];
		arena[snakeTail.y][snakeTail.x] = Cell::air;
		switch (oldCell)
		{
		case Cell::snakeDown:
			snakeTail.y++;
			if (snakeTail.y == Screen::screenHeight)
				snakeTail.y = 0;
			break;
		case Cell::snakeLeft:
			snakeTail.x--;
			if (snakeTail.x == 0xff)
				snakeTail.x = Screen::screenWidth - 1;
			break;
		case Cell::snakeRight:
			snakeTail.x++;
			if (snakeTail.x == Screen::screenWidth)
				snakeTail.x = 0;
			break;
		case Cell::snakeUp:
			snakeTail.y--;
			if (snakeTail.y == 0xff)
				snakeTail.y = Screen::screenHeight - 1;
			break;
		}
	}
}

qword lastCycle = 0;
constexpr qword timeDiffCount = 30;

int main()
{
	// inits
	Screen::clear();
	Screen::Cursor::disable();
	arena = new Cell *[Screen::screenHeight];
	for (byte i = 0; i < Screen::screenHeight; i++)
		arena[i] = new Cell[Screen::screenWidth];
	gameOver = false;
	Build();
	Redraw();

	lastCycle = Time::time();

	// game loop
	while (true)
	{
		Keyboard::KeyCode pressed = Keyboard::getKeyPressedEvent().getKeyCode();

		switch (pressed)
		{
		case Keyboard::KeyCode::W:
		case Keyboard::KeyCode::arrowUp:
			if (currentDirection != Direction::down)
				currentDirection = Direction::up;
			break;
		case Keyboard::KeyCode::A:
		case Keyboard::KeyCode::arrowLeft:
			if (currentDirection != Direction::right)
				currentDirection = Direction::left;
			break;
		case Keyboard::KeyCode::S:
		case Keyboard::KeyCode::arrowDown:
			if (currentDirection != Direction::up)
				currentDirection = Direction::down;
			break;
		case Keyboard::KeyCode::D:
		case Keyboard::KeyCode::arrowRight:
			if (currentDirection != Direction::left)
				currentDirection = Direction::right;
			break;
		case Keyboard::KeyCode::R:
			if (gameOver)
			{
				Screen::clear();
				Screen::Cursor::disable();
				gameOver = false;
				Build();
				Redraw();

				lastCycle = Time::time();
			}
			break;
		case Keyboard::KeyCode::Escape:
			Screen::clear();
			Screen::Cursor::enable();
			// for (byte i = 0; i < screenHeight; i++)
			// 	delete[] arena[i];
			// delete[] arena;
			return 0;
		}

		if (gameOver)
			continue;

		qword curCycle = Time::time();

		if (curCycle - lastCycle >= timeDiffCount)
		{
			Cycle();
			Redraw();
			lastCycle = curCycle;
		}

		// io_wait();
	}
}