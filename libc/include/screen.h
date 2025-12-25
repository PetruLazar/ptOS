#pragma once
#include <types.h>
#include <vector.h>

namespace Screen
{
	struct Cell
	{
		char character;
		enum Color : byte
		{
			bright = 0b1000,
			red = 0b100,
			green = 0b10,
			blue = 0b1,
			black = 0b0,
			white = 0b111
		} color;

		inline Cell(char character, Color bg, Color fg) : character(character), color(Color(bg << 4 | fg)) {}
		inline Cell() : character(' '), color(white) {}

		inline void setFGColor(Color color) { this->color = Color(this->color & 0xf0 | color); }
		inline void setBgColor(Color color) { this->color = Color(this->color & 0x0f | (color << 4)); }
		inline void setColor(Color bg, Color fg) { color = Color(bg << 4 | fg); }
		inline Color getBgColor() { return Color(color >> 4); }
		inline Color getFGColor() { return Color(color & 0xf); }
	};

	static constexpr byte screenWidth = 80;
	static constexpr byte screenHeight = 25;
	static constexpr byte tabSize = 4;

	inline short Vec2ToLinear(Vector2b pos)
	{
		return pos.y * screenWidth + pos.x;
	}
	inline Vector2b LinToVec2(short pos)
	{
		return Vector2b(pos % screenWidth, pos / screenWidth);
	}

	void clear();
	void scrollUp();
	void scrollDown();
	void print(char ch, short pos);
	void print(const char *msg, short pos);
	inline void print(const char *msg, byte line, byte col) { return print(msg, line * screenWidth + col); }
	inline void print(const char *msg, Vector2b pos) { return print(msg, Vec2ToLinear(pos)); }

	void fill(Vector2b topleft, Vector2b botright, char character);
	void paint(Vector2b topleft, Vector2b botright, Cell::Color color);
	void fill(Vector2b topleft, Vector2b botright, Cell content);

	namespace Cursor
	{
		void set(short pos);
		inline void set(byte line, byte col) { set(line * screenWidth + col); }
		inline void set(Vector2b pos) { set(pos.y * screenWidth + pos.x); }
		short get();
		inline Vector2b getV2() { return LinToVec2(get()); }

		bool isEnabled();
	}
}

#include <syscall.h>