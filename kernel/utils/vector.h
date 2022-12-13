#pragma once
#include "../core/crl.h"
// #include "../core/exception.h"

// #include "iostream.h"
// #include "../core/sys.h"

template <class T>
class Vector3
{
public:
	T x, y, z;

	inline Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
};

template <class T>
class Vector2
{
public:
	T x, y;

	inline Vector2(T x, T y) : x(x), y(y) {}
};

typedef Vector2<byte> Vector2b;
typedef Vector2<float> Vector2f;
typedef Vector2<int> Vectro2i;

/*class container_empty : public exception
{
public:
	inline container_empty() : exception("Container is empty") {}
};*/

namespace std
{
	template <class T>
	class vector
	{
	protected:
		T *values;
		ull capacity, size;

	public:
		inline vector() : values(nullptr), size(0), capacity(0) {}
		inline vector(int capacity) : values(nullptr), size(0), capacity(0)
		{
			reserve(capacity);
		}
		inline vector(const vector<T> &other) : size(0), capacity(0)
		{
			assign(other.values, other.size);
		}
		inline vector(const T *vals, ull len) : capacity(0), size(0)
		{
			assign(vals, len);
		}
		inline ~vector()
		{
			if (values)
				delete[] values;
		}

		inline ull getSize() const
		{
			return size;
		}
		inline T *data() const
		{
			return values;
		}

		inline void reserve(int newCap)
		{
			capacity = newCap;
			T *newValues = new T[capacity];
			if (!newValues)
			{
				// throw bad_alloc
				return;
			}
			if (values)
			{
				int min = size < newCap ? size : newCap;
				for (int i = 0; i < min; i++)
					newValues[i] = values[i];
				delete[] values;
			}
			values = newValues;
		}
		inline void resize(int newSize)
		{
			if (newSize > capacity)
				reserve(newSize + 10);
			else if (newSize + 10 <= capacity)
				reserve(newSize);
			size = newSize;
		}
		inline void push_back(const T &value)
		{
			ull origSize = size;
			resize(size + 1);
			values[origSize] = value;
		}
		inline T pop_back()
		{
			if (!values || !size)
				return T();
			T popped = values[size - 1];
			resize(size - 1);
			return popped;
		}
		inline void assign(const T *ptr, ull len)
		{
			resize(len);
			for (ull i = 0; i < len; i++)
				values[i] = ptr[i];
		}
		inline void assign(const vector &other)
		{
			assign(other.values, other.size);
		}

		inline T *begin() const
		{
			return values;
		}
		inline T *end() const
		{
			return values + size;
		}

		inline const T &operator[](ull i) const
		{
			return values[i];
		}
		inline T &operator[](ull i)
		{
			return values[i];
		}
		inline void operator=(const vector &other)
		{
			resize(other.capacity);
			size = other.size;
			for (ull i = 0; i < size; i++)
				values[i] = other[i];
		}
	};
}