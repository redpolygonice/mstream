#pragma once

#include <stddef.h>  // defines NULL
#include <assert.h>

template <class T>
class Singleton
{
public:
	static T* instance() {
		if(!_instance) _instance = new T;
		assert(_instance != NULL);
		return _instance;
	}
protected:
	Singleton() {}
	~Singleton() {}
private:
	Singleton(Singleton const&);
	Singleton& operator=(Singleton const&);
	static T* _instance;
};

template <class T> T* Singleton<T>::_instance = NULL;
