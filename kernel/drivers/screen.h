#pragma once
#include "../utils/types.h"
#include "../utils/vector.h"

class Screen
{
	static void applyBuffer();
	static void makeSpaceInBuffer();

public:
	class Cell
	{
	public:
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

	static void Initialize();
	static void Cleanup();
	static void clear();
	static void scrollUp();
	static void scrollDown();
	static void print(char ch);
	static void print(char ch, short pos);
	static void paint(byte line, byte col, Cell::Color color);
	static void print(const char *msg);
	static void print(const char *msg, short pos);
	inline static void print(const char *msg, byte line, byte col) { return print(msg, line * screenWidth + col); }
	inline static void print(const char *msg, Vector2b pos) { return print(msg, Vec2ToLinear(pos)); }

	static void fill(Vector2b topleft, Vector2b botright, char character);
	static void paint(Vector2b topleft, Vector2b botright, Cell::Color color);
	static void fill(Vector2b topleft, Vector2b botright, Cell content);

	inline static short Vec2ToLinear(Vector2b pos)
	{
		return pos.y * screenWidth + pos.x;
	}
	inline static Vector2b LinToVec2(short pos)
	{
		return Vector2b(pos % screenWidth, pos / screenHeight);
	}

	class Cursor
	{
		static bool enabled;

	public:
		static void update();
		static void set(short pos);
		inline static void set(byte line, byte col) { set(line * screenWidth + col); }
		inline static void set(Vector2b pos) { set(pos.y * screenWidth + pos.x); }
		static short get();
		inline static Vector2b getV2() { return LinToVec2(get()); }

		static void enable(byte start = 0xf, byte end = 0xd);
		static void disable();

		inline static bool isEnabled() { return enabled; }
	};
};