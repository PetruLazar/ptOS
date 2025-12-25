#include "screen.h"
#include "../cpu/ports.h"
#include <string.h>
#include "../debug/verbose.h"

using namespace std;


namespace Screen
{
	short screenSize;
	int bufferSize, startInBuffer = 0;

	namespace Cursor
	{
		bool enabled = true;
		short screenPos = 0;
		int bufferPos = 0;

		void update()
		{
			if (!enabled)
				return;
			outb(0x3d4, 0xf);
			outb(0x3d5, screenPos & 0xff);
			outb(0x3d4, 0xe);
			outb(0x3d5, screenPos >> 8);
		}
		void driver_set(short pos)
		{
			screenPos = pos;
			bufferPos = screenPos + Screen::startInBuffer;
			update();
		}
		short driver_get() { return screenPos; }

		void driver_enable(byte start, byte end)
		{
			outb(0x3d4, 0x0a);
			outb(0x3d5, inb(0x3d5) & 0xc0 | start);
			outb(0x3d4, 0x0a);
			outb(0x3d5, inb(0x3d5) & 0xe0 | end);
			enabled = true;
		}
		void driver_disable()
		{
			outb(0x3d4, 0x0a);
			outb(0x3d5, 0x20);
			enabled = false;
		}

		bool isEnabled() { return enabled; }
	}

	void applyBuffer();
	void makeSpaceInBuffer();

	Cell *const video_memory = (Cell *)0xb8000;
	Cell *buffer = nullptr;

	#ifdef VERBOSE_LOGGING
	void driver_print_uninitialized(const char* msg)
	{
		for (int i = 0; msg[i]; i++)
		{
			switch (msg[i])
			{
			case '\n':
			{
				int delta = screenWidth - Cursor::screenPos % screenWidth;
				Cursor::screenPos += delta;
			}
			break;
			case '\b':
			{
				if (Cursor::screenPos)
					Cursor::screenPos--;
			}
			break;
			case '\t':
			{
				short delta = tabSize - (Cursor::screenPos % screenWidth) % tabSize;
				Cursor::screenPos += delta;
			}
			break;
			case '\a':
				break;
			default:
				video_memory[Cursor::screenPos++].character = msg[i];
			}


			if (Cursor::screenPos >= screenWidth * screenHeight)
			{
				// scroll up the screen
				Cursor::screenPos -= screenWidth;
				for (int i = 0; i < screenWidth * (screenHeight - 1); i++)
					video_memory[i] = video_memory[i + screenWidth];
				for (int i = screenWidth * (screenHeight - 1); i < screenWidth * screenHeight; i++)
					video_memory[i] = Cell(' ', Cell::Color::black, Cell::Color::white);
			}
		}
		Cursor::update();
	}
	void driver_clear_uninitialized()
	{
		for (int i = 0; i < screenWidth * screenHeight; i++)
			video_memory[i] = Cell(' ', Cell::Color::black, Cell::Color::white);
		Cursor::driver_set(0);
	}
	void Initialize_vervose()
	{
		screenSize = screenWidth * screenHeight;
		bufferSize = screenSize * 10; // 10 screens
		buffer = new Cell[bufferSize];

		for (int i = 0; i < screenWidth * screenHeight; i++)
			buffer[i] = video_memory[i];

		Cursor::bufferPos = Cursor::screenPos;
	}
	#endif

	void Initialize()
	{
		SCREENDRIVER_INITIALIZE_REDIRECT;

		screenSize = screenWidth * screenHeight;
		bufferSize = screenSize * 10; // 10 screens
		buffer = new Cell[bufferSize];

		driver_clear();
	}
	void Cleanup()
	{
		delete[] buffer;
	}
	void driver_clear()
	{
		SCREENDRIVER_CLEAR_REDIRECT;

		for (word i = 0; i < bufferSize; i++)
			buffer[i] = Cell();
		startInBuffer = 0;
		applyBuffer();
		Cursor::driver_set(0);
		if (!Cursor::isEnabled())
			Cursor::driver_enable();
	}
	void applyBuffer()
	{
		int max;
		if (startInBuffer + screenSize > bufferSize)
		{
			// fill the screen outside buffer with spaces
			max = bufferSize - startInBuffer;
			for (int i = max; i < screenSize; i++)
				video_memory[i] = Cell();
		}
		else
			max = screenSize;
		for (int i = 0; i < max; i++)
			video_memory[i] = buffer[i + startInBuffer];
	}
	void makeSpaceInBuffer()
	{
		int lim = bufferSize - screenWidth, j;
		for (j = 0; j < lim; j++)
			buffer[j] = buffer[j + screenWidth];
		for (; j < bufferSize; j++)
			buffer[j] = Cell();
		if (Cursor::bufferPos < screenWidth)
			Cursor::bufferPos = 0;
		else
			Cursor::bufferPos -= screenWidth;
		if (startInBuffer < screenWidth)
			startInBuffer = 0;
		else
			startInBuffer -= screenWidth;
	}
	void scrollUp()
	{
		if (startInBuffer == 0)
			return;
		Cursor::screenPos += screenWidth;
		startInBuffer -= screenWidth;
		if (Cursor::screenPos >= screenSize)
		{
			if (Cursor::isEnabled())
				Cursor::driver_disable();
		}
		else if (Cursor::screenPos >= 0)
		{
			Cursor::update();
			if (!Cursor::isEnabled())
				Cursor::driver_enable();
		}
		applyBuffer();
	}
	void scrollDown()
	{
		if (Cursor::screenPos < 0)
			return;
		Cursor::screenPos -= screenWidth;
		startInBuffer += screenWidth;
		if (Cursor::screenPos < 0)
		{
			if (Cursor::isEnabled())
				Cursor::driver_disable();
		}
		else if (Cursor::screenPos < screenSize)
		{
			Cursor::update();
			if (!Cursor::isEnabled())
				Cursor::driver_enable();
		}
		applyBuffer();
	}
	void driver_print(const char *msg)
	{
		SCREENDRIVER_PRINT_REDIRECT;

		while (Cursor::screenPos < 0)
		{
			Cursor::screenPos += screenWidth;
			startInBuffer -= screenWidth;
		}
		while (Cursor::screenPos >= screenSize)
		{
			Cursor::screenPos -= screenWidth;
			startInBuffer += screenWidth;
		}

		for (int i = 0; msg[i]; i++)
		{
			switch (msg[i])
			{
			case '\n':
			{
				int delta = screenWidth - Cursor::bufferPos % screenWidth;
				Cursor::bufferPos += delta;
				Cursor::screenPos += delta;
			}
			break;
			case '\b':
			{
				if (Cursor::bufferPos)
				{
					Cursor::bufferPos--;
					Cursor::screenPos--;
					if (Cursor::screenPos < 0)
					{
						Cursor::screenPos += screenWidth;
						startInBuffer -= screenWidth;
					}
				}
			}
			break;
			case '\t':
			{
				short delta = tabSize - (Cursor::screenPos % screenWidth) % tabSize;
				Cursor::bufferPos += delta;
				Cursor::screenPos += delta;
			}
			break;
			case '\a':
				break;
			default:
				buffer[Cursor::bufferPos++].character = msg[i];
				Cursor::screenPos++;
			}

			if (Cursor::bufferPos >= bufferSize)
			{
				// scroll up the buffer
				makeSpaceInBuffer();
			}
			if (Cursor::screenPos >= screenSize)
			{
				// scroll up the screen
				Cursor::screenPos -= screenWidth;
				startInBuffer += screenWidth;
			}
		}
		Cursor::update();
		if (!Cursor::isEnabled())
			Cursor::driver_enable();
		applyBuffer();
	}
	void driver_print(char ch)
	{
		while (Cursor::screenPos < 0)
		{
			Cursor::screenPos += screenWidth;
			startInBuffer -= screenWidth;
		}
		while (Cursor::screenPos >= screenSize)
		{
			Cursor::screenPos -= screenWidth;
			startInBuffer += screenWidth;
		}

		switch (ch)
		{
		case '\n':
		{
			int diff = screenWidth - Cursor::bufferPos % screenWidth;
			Cursor::bufferPos += diff;
			Cursor::screenPos += diff;
		}
		break;
		case '\b':
		{
			if (Cursor::bufferPos)
			{
				Cursor::bufferPos--;
				Cursor::screenPos--;
				if (Cursor::screenPos < 0)
				{
					Cursor::screenPos += screenWidth;
					startInBuffer -= screenWidth;
				}
			}
			Cursor::bufferPos--;
		}
		break;
		case '\t':
		{
			short delta = tabSize - (Cursor::screenPos % screenWidth) % tabSize;
			Cursor::bufferPos += delta;
			Cursor::screenPos += delta;
		}
		break;
		case '\a':
			break;
		default:
			buffer[Cursor::bufferPos++].character = ch;
			Cursor::screenPos++;
		}

		if (Cursor::bufferPos >= bufferSize)
		{
			// scroll up the buffer
			makeSpaceInBuffer();
		}
		if (Cursor::screenPos >= screenSize)
		{
			// scroll up the screen
			Cursor::screenPos -= screenWidth;
			startInBuffer += screenWidth;
		}
		Cursor::update();
		if (!Cursor::isEnabled())
			Cursor::driver_enable();
		applyBuffer();
	}
	void print(char ch, short pos)
	{
		pos += startInBuffer;
		while (pos >= bufferSize)
		{
			makeSpaceInBuffer();
			pos -= screenWidth;
		}
		buffer[pos].character = ch;
		applyBuffer();
	}
	void driver_paint(byte line, byte col, Cell::Color bgColor)
	{
		short linear = line * screenWidth + col;
		byte cclr = buffer[startInBuffer + linear].color;
		cclr &= 0xf;
		cclr |= bgColor << 4;
		video_memory[linear].color = (Cell::Color)cclr;
		buffer[startInBuffer + linear].color = (Cell::Color)cclr;
	}
	void driver_paint(byte line, byte col, Cell::Color textColor, Cell::Color bgColor)
	{
		short linear = line * screenWidth + col;
		byte cclr = textColor;
		cclr &= 0xf;
		cclr |= bgColor << 4;
		video_memory[linear].color = (Cell::Color)cclr;
		buffer[startInBuffer + linear].color = (Cell::Color)cclr;
	}
	void print(const char *msg, short pos)
	{
		pos += startInBuffer;
		while (pos >= bufferSize)
		{
			makeSpaceInBuffer();
			pos -= screenWidth;
		}
		for (int i = 0; msg[i]; i++)
		{
			switch (msg[i])
			{
			case '\n':
				pos += screenWidth - pos % screenWidth;
				break;
			case '\b':
				pos--;
				break;
			case '\t':
				pos += tabSize - pos % tabSize;
			case '\a':
				break;
			default:
				buffer[pos++].character = msg[i];
			}
			if (pos >= bufferSize)
			{
				makeSpaceInBuffer();
				pos -= screenWidth;
			}
			if (pos >= screenSize)
			{
				applyBuffer();
				return;
			}
		}
		applyBuffer();
	}

	void fill(Vector2<byte> topleft, Vector2<byte> botright, char character)
	{
		for (byte line = topleft.y; line <= botright.y; line++)
		{
			short linebase = line * screenWidth;
			for (byte col = topleft.x; col <= botright.x; col++)
				video_memory[linebase + col].character = character;
		}
	}
	void paint(Vector2<byte> topleft, Vector2<byte> botright, Cell::Color color)
	{
		for (byte line = topleft.y; line <= botright.y; line++)
		{
			short linebase = line * screenWidth;
			for (byte col = topleft.x; col <= botright.x; col++)
			{
				byte cclr = buffer[startInBuffer + linebase + col].color;
				cclr &= 0xf;
				cclr |= color << 4;
				buffer[startInBuffer + linebase + col].color = (Cell::Color)cclr;
				video_memory[linebase + col].color = (Cell::Color)cclr;
			}
		}
	}
	void fill(Vector2<byte> topleft, Vector2<byte> botright, Cell content)
	{
		for (byte line = topleft.y; line <= botright.y; line++)
		{
			short linebase = line * screenWidth;
			for (byte col = topleft.x; col <= botright.x; col++)
				video_memory[linebase + col] = content;
		}
	}
}
