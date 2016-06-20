/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


void *operator new(std::size_t n)
{
	return mowgli_alloc(n);
}


void operator delete(void *ptr)
noexcept
{
	mowgli_free(ptr);
}


void operator delete(void *ptr, long unsigned int)
noexcept
{
	mowgli_free(ptr);
}


template<class T>
struct allocator
{
	using value_type         = T;
	using pointer            = T *;
	using const_pointer      = const T *;
	using reference          = T &;
	using const_reference    = const T &;
	using size_type          = std::size_t;
	using difference_type    = std::ptrdiff_t;

	size_type max_size() const
	{
		return std::numeric_limits<size_type>::max();
	}

	pointer address(reference x) const
	{
		return &x;
	}

	const_pointer address(const_reference x) const
	{
		return &x;
	}

	pointer allocate(size_type n, const_pointer hint = 0)
	{
		return reinterpret_cast<pointer>(mowgli_alloc(n * sizeof(T)));
	}

	void deallocate(pointer p, size_type n)
	{
		mowgli_free(p);
	}

	allocator() = default;
	allocator(allocator &&) = default;
	allocator(const allocator &) = default;
	template<class U> allocator(const allocator<U> &) {}
};


template<class T1,
         class T2>
bool operator==(const allocator<T1> &a, const allocator<T2> &b)
{
	return true;
}


template<class T1,
         class T2>
bool operator!=(const allocator<T1> &a, const allocator<T2> &b)
{
	return false;
}


struct v8allocator
:private allocator<uint8_t>
,v8::ArrayBuffer::Allocator
{
	virtual void *Allocate(size_t size)
	{
		return allocator::allocate(size);
	}

	virtual void *AllocateUninitialized(size_t size)
	{
		return allocator::allocate(size);
	}

	virtual void Free(void *const ptr, size_t size)
	{
		allocator::deallocate(reinterpret_cast<uint8_t *>(ptr), size);
	}
};
