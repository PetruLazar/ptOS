#pragma once
#include <vector.h>
#include <iostream.h>

namespace std
{
	template <class T>
	class basic_string : public vector<T>
	{
		using base = vector<T>;

		static int compareUnit(T v1, T v2)
		{
			return v1 > v2 ? 1 : (v1 < v2 ? -1 : 0);
		}

	public:
		inline basic_string() : vector<T>(1)
		{
			base::push_back(0);
		}
		inline basic_string(const T *ptr, ull len) : vector<T>(len + 1)
		{
			base::assign(ptr, len);
			base::push_back(0);
		}
		inline basic_string(const T *ptr)
		{
			assign(ptr);
		}
		inline basic_string(const basic_string &other)
		{
			base::assign(other.data(), other.size);
		}

		inline ull length() const { return base::size - 1; }
		inline static ull length(const T *ptr)
		{
			for (ull i = 0;; i++)
				if (!ptr[i])
					return i;
		}

		// assign
		inline void assign(const T *ptr, ull len)
		{
			base::resize(len + 1);
			base::assign(ptr, len);
			base::push_back(0);
		}
		inline void assign(const T *ptr)
		{
			assign(ptr, length(ptr));
		}
		inline void assign(const basic_string &other)
		{
			base::assign(other.values, other.size);
		}

		inline void push_back(T val)
		{
			base::values[length()] = val;
			base::push_back(0);
		}
		inline T pop_back()
		{
			ull len = length();
			base::resize(len);
			T ret = base::values[--len];
			base::values[len] = 0;
			return ret;
		}

		inline void insert(const T *ptr, ull len, ull pos)
		{
			ull oldLen = length();
			base::resize(base::size + len);

			// move old values
			for (ull i = oldLen; i > pos; i--)
				base::values[i + len] = base::values[i];
			base::values[pos + len] = base::values[pos];

			// actual insertion
			for (ull i = 0; i < len; i++)
				base::values[pos + i] = ptr[i];
		}
		inline void insert(const T *ptr, ull pos)
		{
			insert(ptr, length(ptr), pos);
		}
		inline void insert(const basic_string &str, ull pos)
		{
			insert(str.data(), str.length(), pos);
		}
		inline void append(const T *ptr, ull len)
		{
			insert(ptr, len, length());
		}
		inline void append(const T *ptr)
		{
			insert(ptr, length(ptr), length());
		}
		inline void append(const basic_string &str)
		{
			insert(str.data(), str.length(), length());
		}
		inline ull replaceFirst(T toFind, T replaceWith)
		{
			ull pos = firstOf(toFind);
			if (pos != npos)
				at(pos) = replaceWith;
			return pos;
		}
		inline ull replaceLast(T toFind, T replaceWith)
		{
			ull pos = lastOf(toFind);
			if (pos != npos)
				at(pos) = replaceWith;
			return pos;
		}

		inline void resize(ull len)
		{
			at(len) = 0;
			base::resize(++len);
		}
		inline void erase()
		{
			base::resize(1);
			at(0) = 0;
		}
		inline void erase(ull pos)
		{
			erase(pos, 1);
		}
		inline void erase(ull pos, ull len)
		{
			for (ull i = pos + len; i <= length(); i++)
				at(i - len) = at(i);
			base::resize(base::size - len);
		}
		inline int compare(const T *ptr, ull len) const
		{
			++len;
			ull l = base::size < len ? base::size : len;
			for (ull i = 0; i < l; i++)
			{
				int cmp = compareUnit(at(i), ptr[i]);
				if (cmp)
					return cmp;
			}
			return 0;
		}
		inline int compare(const T *ptr) const
		{
			return compare(ptr, length(ptr));
		}
		inline int compare(const basic_string &other) const
		{
			return compare(other.data(), other.length());
		}

		inline ull firstOf(T val) const
		{
			ull len = length();
			for (ull i = 0; i < len; i++)
				if (at(i) == val)
					return i;
			return npos;
		}
		inline ull lastOf(T val) const
		{
			for (ull i = length() - 1; i != npos; i--)
				if (at(i) == val)
					return i;
			return npos;
		}
		inline ull firstOf(const T *ptr, ull len) const
		{
			ull l = base::size - len;
			for (ull i = 0; i < l; i++)
			{
				bool found = true;
				for (ull j = 0; j < len; j++)
					if (ptr[j] != at(i + j))
					{
						found = false;
						break;
					}
				if (found)
					return i;
			}
			return npos;
		}
		inline ull firstOf(const T *ptr) const
		{
			return firstOf(ptr, length(ptr));
		}
		inline ull firstOf(const basic_string &str) const
		{
			return firstOf(str.data(), str.length());
		}
		inline ull lastOf(const T *ptr, ull len) const
		{
			ull l = base::size - len;
			for (ull i = base::size - len - 1; i != npos; i--)
			{
				bool found = true;
				for (ull j = 0; j < len; j++)
					if (ptr[j] != at(i + j))
					{
						found = false;
						break;
					}
				if (found)
					return i;
			}
			return npos;
		}
		inline ull lastOf(const T *ptr) const
		{
			return lastOf(ptr, length(ptr));
		}
		inline ull lastOf(const basic_string &str) const
		{
			return lastOf(str.data(), str.length());
		}

		// hide begin() and end() with new implementations
		inline T *end() const
		{
			return base::values + length();
		}

		inline T &at(ull i) { return base::values[i]; }
		inline basic_string at(ull i, ull len) const
		{
			return basic_string(base::values + i, len);
		}
		inline const T &at(ull i) const { return base::values[i]; }

		// operators
		inline basic_string operator+(const T *ptr) const
		{
			basic_string result = *this;
			result.append(ptr);
			return result;
		}
		inline basic_string operator+(const basic_string &other) const
		{
			basic_string result = *this;
			result.append(other);
			return result;
		}
		inline basic_string operator+(T val) const
		{
			basic_string result = *this;
			result.push_back(val);
			return result;
		}

		inline void operator=(const T *ptr)
		{
			assign(ptr);
		}
		inline void operator=(const basic_string &other)
		{
			assign(other);
		}
		inline void operator+=(const T *ptr) { append(ptr); }
		inline void operator+=(const basic_string &other) { append(other); }
		inline void operator+=(T val) { push_back(val); }

		inline bool operator==(const T *ptr) const { return compare(ptr) == 0; }
		inline bool operator==(const basic_string &other) const { return compare(other) == 0; }
		inline bool operator!=(const T *ptr) const { return compare(ptr) != 0; }
		inline bool operator!=(const basic_string &other) const { return compare(other) != 0; }
		inline bool operator<(const T *ptr) const { return compare(ptr) == -1; }
		inline bool operator<(const basic_string &other) const { return compare(other) == -1; }
		inline bool operator<=(const T *ptr) const { return compare(ptr) <= 0; }
		inline bool operator<=(const basic_string &other) const { return compare(other) <= 0; }
		inline bool operator>(const T *ptr) const { return compare(ptr) == 1; }
		inline bool operator>(const basic_string &other) const { return compare(other) == 1; }
		inline bool operator>=(const T *ptr) const { return compare(ptr) >= 0; }
		inline bool operator>=(const basic_string &other) const { return compare(other) >= 0; }

		static constexpr ull npos = (ull)(-1);
	};

	template <class T>
	inline basic_string<T> operator+(const T *ptr, const basic_string<T> &str)
	{
		basic_string<T> result = str;
		result.insert(ptr, 0);
		return result;
	}
	template <class T>
	inline basic_string<T> operator+(T val, const basic_string<T> &str)
	{
		basic_string<T> result;
		result.push_back(val);
		result.append(str);
		return result;
	}

	typedef basic_string<char> string;		 // 8 bits char string
	typedef basic_string<char16_t> string16; // 16 bits char string
	typedef basic_string<wchar_t> wstring;	 // 32 bits char string

	inline string to_string(llong x)
	{
		char str[32];
		lltos(str, x);
		return string(str);
	}
	inline string to_string(ull x)
	{
		char str[32];
		ulltos(str, x);
		return string(str);
	}
	inline string to_string(short x) { return to_string((llong)x); }
	inline string to_string(ushort x) { return to_string((ull)x); }
	inline string to_string(int x) { return to_string((llong)x); }
	inline string to_string(uint x) { return to_string((ull)x); }

	inline ostream &operator<<(ostream &os, const string &str)
	{
		return os << str.data();
	}
	inline ostream &operator<<(ostream &os, const string16 &str)
	{
		ull len = str.length();
		char *tempBuffer = new char[len + 1];
		for (ull i = 0; i < len; i++)
		{
			char16_t ch = str[i];
			tempBuffer[i] = (ch >> 8) ? '?' : (char)str[i];
		}
		tempBuffer[len] = 0;
		os << tempBuffer;
		delete[] tempBuffer;
		return os;
	}

	inline istream &operator>>(istream &os, string &str)
	{
		str.erase();
		while (true)
		{
			char ch;
			os >> ch;
			switch (ch)
			{
			case '\b': // delete one char
				if (str[0] == 0)
					break;
				cout << "\b \b";
				str.pop_back();
				break;
			case '\r': // finish reading
				cout << '\n';
				return os;
			case 0:
				break;
			default:
				cout << ch;
				str.push_back(ch);
			}
		}
	}
	inline istream &operator>>(istream &os, string16 &str)
	{
		str.erase();
		while (true)
		{
			char ch;
			os >> ch;
			switch (ch)
			{
			case '\b': // delete one char
				if (str[0] == 0)
					break;
				cout << "\b \b";
				str.pop_back();
				break;
			case '\r': // finish reading
				cout << '\n';
				return os;
			case 0:
				break;
			default:
				cout << ch;
				str.push_back(ch);
			}
		}
	}
}