#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include "types.h"
#include <queue>

template <class T>
class ThreadSafeQueue
{
private:
	std::queue<T> _data;
	mutable std::mutex _mutex;

public:
	ThreadSafeQueue() {}
	~ThreadSafeQueue() {}

public:
	void push(const T &element)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_data.push(element);
	}
	T pop()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_data.empty())
			return nullptr;

		T element = _data.front();
		_data.pop();
		return element;
	}
	bool pop(T &element)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_data.empty())
			return false;

		element = _data.front();
		_data.pop();
		return true;
	}
	bool isEmpty() const
	{
		std::lock_guard<std::mutex> lock(_mutex);
		bool result = _data.empty();
		return result;
	}
	size_t count() const
	{
		std::lock_guard<std::mutex> lock(_mutex);
		size_t count = _data.size();
		return count;
	}
	void clear()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_data.empty())
			return;
		std::queue<T> empty;
		std::swap(_data, empty);
	}
};

#endif // THREADSAFEQUEUE_H
