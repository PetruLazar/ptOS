#include "screen.h"
#include "../cpu/ports.h"
#include "../utils/string.h"

using namespace std;

short screenCursor = 0, screenSize = 80 * 25;

namespace Screen
{
	void applyBuffer();
	void makeSpaceInBuffer();

	Cell *const video_memory = (Cell *)0xb8000;
	Cell *buffer = nullptr;

	int bufferSize, startInBuffer = 0, bufferCursor = 0;

	void Initialize()
	{
		// contents = new vector<Cell *>();
		screenSize = screenWidth * screenHeight;
		bufferSize = screenSize * 10; // 10 screens
		buffer = new Cell[bufferSize];

		clear();
	}
	void Cleanup()
	{
		delete[] buffer;
	}
	void clear()
	{
		for (word i = 0; i < bufferSize; i++)
			buffer[i] = Cell();
		startInBuffer = 0;
		bufferCursor = 0;
		applyBuffer();
		Cursor::set(0);
		if (!Cursor::isEnabled())
			Cursor::enable();
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
		if (bufferCursor < screenWidth)
			bufferCursor = 0;
		else
			bufferCursor -= screenWidth;
		if (startInBuffer < screenWidth)
			startInBuffer = 0;
		else
			startInBuffer -= screenWidth;
	}
	void scrollUp()
	{
		if (startInBuffer == 0)
			return;
		screenCursor += screenWidth;
		startInBuffer -= screenWidth;
		if (screenCursor >= screenSize)
		{
			if (Cursor::isEnabled())
				Cursor::disable();
		}
		else if (screenCursor >= 0)
		{
			Cursor::set(screenCursor);
			if (!Cursor::isEnabled())
				Cursor::enable();
		}
		applyBuffer();
	}
	void scrollDown()
	{
		if (screenCursor < 0)
			return;
		screenCursor -= screenWidth;
		startInBuffer += screenWidth;
		if (screenCursor < 0)
		{
			if (Cursor::isEnabled())
				Cursor::disable();
		}
		else if (screenCursor < screenSize)
		{
			Cursor::set(screenCursor);
			if (!Cursor::isEnabled())
				Cursor::enable();
		}
		applyBuffer();
	}
	void print(const char *msg)
	{
		while (screenCursor < 0)
		{
			screenCursor += screenWidth;
			startInBuffer -= screenWidth;
		}
		while (screenCursor >= screenSize)
		{
			screenCursor -= screenWidth;
			startInBuffer += screenWidth;
		}

		for (int i = 0; msg[i]; i++)
		{
			switch (msg[i])
			{
			case '\n':
			{
				int delta = screenWidth - bufferCursor % screenWidth;
				bufferCursor += delta;
				screenCursor += delta;
			}
			break;
			case '\b':
			{
				if (bufferCursor)
				{
					bufferCursor--;
					screenCursor--;
					if (screenCursor < 0)
					{
						screenCursor += screenWidth;
						startInBuffer -= screenWidth;
					}
				}
			}
			break;
			case '\t':
			{
				short delta = tabSize - (screenCursor % screenWidth) % tabSize;
				bufferCursor += delta;
				screenCursor += delta;
			}
			break;
			case '\a':
				break;
			default:
				buffer[bufferCursor++].character = msg[i];
				screenCursor++;
			}

			if (bufferCursor >= bufferSize)
			{
				// scroll up the buffer
				makeSpaceInBuffer();
			}
			if (screenCursor >= screenSize)
			{
				// scroll up the screen
				screenCursor -= screenWidth;
				startInBuffer += screenWidth;
			}
		}
		Cursor::set(screenCursor);
		if (!Cursor::isEnabled())
			Cursor::enable();
		applyBuffer();
	}
	void print(char ch)
	{
		while (screenCursor < 0)
		{
			screenCursor += screenWidth;
			startInBuffer -= screenWidth;
		}
		while (screenCursor >= screenSize)
		{
			screenCursor -= screenWidth;
			startInBuffer += screenWidth;
		}

		switch (ch)
		{
		case '\n':
		{
			int diff = screenWidth - bufferCursor % screenWidth;
			bufferCursor += diff;
			screenCursor += diff;
		}
		break;
		case '\b':
		{
			if (bufferCursor)
			{
				bufferCursor--;
				screenCursor--;
				if (screenCursor < 0)
				{
					screenCursor += screenWidth;
					startInBuffer -= screenWidth;
				}
			}
			bufferCursor--;
		}
		break;
		case '\t':
		{
			short delta = tabSize - (screenCursor % screenWidth) % tabSize;
			bufferCursor += delta;
			screenCursor += delta;
		}
		break;
		case '\a':
			break;
		default:
			buffer[bufferCursor++].character = ch;
			screenCursor++;
		}

		if (bufferCursor >= bufferSize)
		{
			// scroll up the buffer
			makeSpaceInBuffer();
		}
		if (screenCursor >= screenSize)
		{
			// scroll up the screen
			screenCursor -= screenWidth;
			startInBuffer += screenWidth;
		}
		Cursor::set(screenCursor);
		if (!Cursor::isEnabled())
			Cursor::enable();
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
	void paint(byte line, byte col, Cell::Color color)
	{
		short linear = line * screenWidth + col;
		byte cclr = video_memory[linear].color;
		cclr &= 0xf;
		cclr |= color << 4;
		video_memory[linear].color = (Cell::Color)cclr;
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
				byte cclr = video_memory[linebase + col].color;
				cclr &= 0xf;
				cclr |= color << 4;
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

	namespace Cursor
	{
		bool enabled = true;

		void update()
		{
			if (!enabled)
				return;
			outb(0x3d4, 0xf);
			outb(0x3d5, screenCursor & 0xff);
			outb(0x3d4, 0xe);
			outb(0x3d5, screenCursor >> 8);
		}
		void set(short pos)
		{
			screenCursor = pos;
			update();
		}
		short get() { return screenCursor; }

		void enable(byte start, byte end)
		{
			outb(0x3d4, 0x0a);
			outb(0x3d5, inb(0x3d5) & 0xc0 | start);
			outb(0x3d4, 0x0a);
			outb(0x3d5, inb(0x3d5) & 0xe0 | end);
			enabled = true;
		}
		void disable()
		{
			outb(0x3d4, 0x0a);
			outb(0x3d5, 0x20);
			enabled = false;
		}

		bool isEnabled() { return enabled; }
	}
}
