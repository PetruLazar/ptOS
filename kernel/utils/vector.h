#pragma once
#include "../core/crl.h"
// #include "../core/exception.h"
// #include "../core/mem.h"

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
		inline vector(ull capacity) : values(nullptr), size(0), capacity(0)
		{
			reserve(capacity);
		}
		inline vector(ull size, const T &init) : values(nullptr), size(0), capacity(0)
		{
			resize(size);
			for (ull i = 0; i < size; i++)
				values[i] = init;
		}
		inline vector(const vector<T> &other) : values(nullptr), size(0), capacity(0)
		{
			assign(other.values, other.size);
		}
		inline vector(const T *vals, ull len) : values(nullptr), capacity(0), size(0)
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
		inline const T *data() const
		{
			return values;
		}
		inline T *data()
		{
			return values;
		}

		inline void reserve(ull newCap)
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
				ull min = size < newCap ? size : newCap;
				for (ull i = 0; i < min; i++)
					newValues[i] = values[i];
				delete[] values;
			}
			values = newValues;
		}
		inline void resize(ull newSize)
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
			assign(other);
		}
	};
	template <>
	class vector<bool> : vector<ull>
	{
		ull actualSize;

		ull getCompactSize(ull bitSize)
		{
			if (bitSize == 0)
				return 0;
			return (bitSize - 1) / 64 + 1;
		}

	public:
		using base = vector<ull>;

		class bitref
		{
			using parent = vector<bool>;

			parent *p;
			ull index;

		public:
			inline bitref(parent *p, ull index) : p(p), index(index) {}

			/*inline void operator++() { index++ }
			inline bool operator==(const bitref &other) { return p == other.p && index == other.index; }*/

			inline operator bool()
			{
				ull *entry = ((base *)p)->data() + (index >> 6),
					shift = (ull)1 << (index & 63);

				return *entry & shift;
			}
			inline void operator=(bool val)
			{
				ull *entry = ((base *)p)->data() + (index >> 6),
					shift = (ull)1 << (index & 63);

				if (val)
					*entry |= shift;
				else
					*entry &= ~shift;
			}
		};

		inline vector() : base(), actualSize(0) {}
		inline vector(ull capacity) : base(getCompactSize(capacity)), actualSize(0) {}
		inline vector(ull size, bool val) : base(getCompactSize(size), val ? 0xffffffffffffffff : 0), actualSize(size) {}
		inline vector(const vector<bool> &other) : base((const base &)other), actualSize(other.actualSize) {}
		inline vector(const bool *vals, ull len) : base()
		{
			assign(vals, len);
		}
		// inline ~vector() {}

		inline ull getSize() const { return actualSize; }

		inline void reserve(ull newCap)
		{
			base::reserve(getCompactSize(newCap));
		}
		inline void resize(ull newSize)
		{
			base::resize(getCompactSize(newSize));
			actualSize = newSize;
		}
		inline void push_back(bool val)
		{
			ull oldSize = actualSize;
			resize(actualSize + 1);
			operator[](oldSize) = val;
		}
		inline bool pop_back()
		{
			if (getSize() == 0)
				return false;
			bool val = operator[](--actualSize);
			resize(actualSize);
			return val;
		}
		inline void assign(const bool *ptr, ull len)
		{
			resize(len);
			for (ull i = 0; i < len; i++)
				operator[](i) = ptr[i];
		}
		inline void assign(const vector<bool> &other)
		{
			base::assign((const base &)other);
			actualSize = other.actualSize;
		}

		// begin
		// end

		inline bool operator[](ull index) const
		{
			ull actualIndex = index >> 6;
			ull shift = index & 63;
			return base::values[actualIndex] & ((ull)1 << shift);
		}
		inline bitref operator[](ull index)
		{
			return bitref(this, index);
		}
		inline void operator=(const vector<bool> &other) { assign(other); }
	};
}