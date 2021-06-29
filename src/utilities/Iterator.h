#pragma once

template<typename T>
class IIterator
{
public:
	virtual bool hasNext() = 0;
	virtual T next() = 0;
};