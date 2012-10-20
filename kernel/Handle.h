#pragma once

/*
* 非线程安全
*/
template <class T>
class Handle
{
public:
	Handle(T* pinfo = 0):p(pinfo), count(new std::size_t(1)){}
	Handle(const Handle &i):p(i.p), count(i.count)
	{
		++*count;
	}
	~Handle()
	{
		decr_count();
	}

	Handle& operator=(const Handle& rhs)
	{
		++*rhs.count;
		decr_count();
		p = rhs.p;
		count = rhs.count;
		return *this;
	}
	T *operator->() const
	{
		if(p)
			return p;
		else
			throw std::logic_error("unbound Handle");
	}
	T &operator*() const
	{
		if(p)
			return *p;
		else
			throw std::logic_error("unbound Handle");
	}
private:
	T *p;
	std::size_t *count;

	void decr_count()
	{
		if(--*count == 0)
		{
			delete p;
			delete count;
		}
	}
};