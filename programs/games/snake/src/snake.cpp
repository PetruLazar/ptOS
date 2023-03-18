#include "../../../../libc/syscall.h"
#include "../../../../libc/rand.h"

struct Vector2b
{
	byte x, y;
};

static constexpr byte screenWidth = 80;
static constexpr byte screenHeight = 25;

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
// Cell **arena = nullptr;
Cell arena[screenHeight][screenWidth];
Vector2b snakeHead, snakeTail;
bool gameOver = false;

void spawnApple()
{
	byte x, y;
	do
	{
		uint r = (uint)Random::get();
		x = r % screenWidth;
		y = r / screenWidth % screenHeight;
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
				syscall_screen_paint(y, x, Color::black);
				break;
			case Cell::apple:
				syscall_screen_paint(y, x, Color::red);
				break;
			default:
				syscall_screen_paint(y, x, Color::green);
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
			snakeHead.y = screenHeight - 1;
		break;
	case Direction::down:
		arena[snakeHead.y][snakeHead.x] = Cell::snakeDown;
		snakeHead.y++;
		if (snakeHead.y == screenHeight)
			snakeHead.y = 0;
		break;
	case Direction::left:
		arena[snakeHead.y][snakeHead.x] = Cell::snakeLeft;
		snakeHead.x--;
		if (snakeHead.x == 0xff)
			snakeHead.x = screenWidth - 1;
		break;
	case Direction::right:
		arena[snakeHead.y][snakeHead.x] = Cell::snakeRight;
		snakeHead.x++;
		if (snakeHead.x == screenWidth)
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
			if (snakeTail.y == screenHeight)
				snakeTail.y = 0;
			break;
		case Cell::snakeLeft:
			snakeTail.x--;
			if (snakeTail.x == 0xff)
				snakeTail.x = screenWidth - 1;
			break;
		case Cell::snakeRight:
			snakeTail.x++;
			if (snakeTail.x == screenWidth)
				snakeTail.x = 0;
			break;
		case Cell::snakeUp:
			snakeTail.y--;
			if (snakeTail.y == 0xff)
				snakeTail.y = screenHeight - 1;
			break;
		}
	}
}

qword lastCycle = 0;
constexpr qword timeDiffCount = 0x3;

extern "C" void main()
{
	// inits
	syscall_screen_clear();
	syscall_cursor_disable();
	// arena = new Cell *[screenHeight];
	// for (byte i = 0; i < screenHeight; i++)
	// 	arena[i] = new Cell[screenWidth];
	gameOver = false;
	Build();
	Redraw();

	lastCycle = syscall_time_get();

	// game loop
	while (true)
	{
		KeyEvent::KeyCode pressed = syscall_keyboard_getKeyPressedEvent().getKeyCode();

		switch (pressed)
		{
		case KeyEvent::KeyCode::W:
		case KeyEvent::KeyCode::arrowUp:
			if (currentDirection != Direction::down)
				currentDirection = Direction::up;
			break;
		case KeyEvent::KeyCode::A:
		case KeyEvent::KeyCode::arrowLeft:
			if (currentDirection != Direction::right)
				currentDirection = Direction::left;
			break;
		case KeyEvent::KeyCode::S:
		case KeyEvent::KeyCode::arrowDown:
			if (currentDirection != Direction::up)
				currentDirection = Direction::down;
			break;
		case KeyEvent::KeyCode::D:
		case KeyEvent::KeyCode::arrowRight:
			if (currentDirection != Direction::left)
				currentDirection = Direction::right;
			break;
		case KeyEvent::KeyCode::R:
			if (gameOver)
			{
				syscall_screen_clear();
				syscall_cursor_disable();
				gameOver = false;
				Build();
				Redraw();

				lastCycle = syscall_time_get();
			}
			break;
		case KeyEvent::KeyCode::Escape:
			syscall_screen_clear();
			syscall_cursor_enable_def();
			// for (byte i = 0; i < screenHeight; i++)
			// 	delete[] arena[i];
			// delete[] arena;
			return;
		}

		if (gameOver)
			continue;

		qword curCycle = syscall_time_get();

		if (curCycle - lastCycle >= timeDiffCount)
		{
			Cycle();
			Redraw();
			lastCycle = curCycle;
		}

		// io_wait();
	}
}