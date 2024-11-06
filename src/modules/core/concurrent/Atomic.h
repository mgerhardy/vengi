/**
 * @file
 */

#pragma once

#include <SDL3/SDL_atomic.h>

namespace core {

class AtomicBool {
private:
	SDL_AtomicInt _value { 0 };
public:
	AtomicBool(bool value);

	operator bool() const;

	bool exchange(bool rhs);
	bool compare_exchange(bool expectedVal, bool newVal);

	void operator=(bool rhs);
	void operator=(const AtomicBool& rhs);

	bool operator==(bool rhs) const;
	bool operator==(const AtomicBool& rhs) const;
};

class AtomicInt {
private:
	SDL_AtomicInt _value;
public:
	AtomicInt(int value = 0);

	operator int() const;

	int exchange(int rhs);
	bool compare_exchange(int expectedVal, int newVal);

	void operator=(int rhs);
	void operator=(const AtomicInt& rhs);

	int decrement(int value = 1);
	int increment(int value = 1);

	AtomicInt& operator--();
	AtomicInt& operator++();

	bool operator==(int rhs) const;
	bool operator==(const AtomicInt& rhs) const;
};

template<class T>
class AtomicPtr {
private:
	void* _ptr;
public:
	AtomicPtr(T* value = nullptr) {
		SDL_SetAtomicPointer(&_ptr, (void*)value);
	}

	operator T*() {
		return (T*)SDL_GetAtomicPointer(&_ptr);
	}

	operator const T*() const {
		return (const T*)SDL_GetAtomicPointer(const_cast<void**>(&_ptr));
	}

	T* exchange(T* value) {
		return (T*)SDL_SetAtomicPointer(&_ptr, (void*)value);
	}

	T* compare_exchange(T* expectedPtr, T* newPtr) {
		return (T*)SDL_CompareAndSwapAtomicPointer(&_ptr, (void*)expectedPtr, (void*)newPtr);
	}

	void operator=(T* value) {
		SDL_SetAtomicPointer(&_ptr, (void*)value);
	}

	void operator=(const AtomicPtr& value) {
		if (&value == this) {
			return;
		}
		SDL_SetAtomicPointer(&_ptr, value._ptr);
	}

	bool operator==(T* value) const {
		return (const T*)(*this) == value;
	}

	bool operator==(const AtomicPtr& value) const {
		return (const T*)(*this) == (const T*)value;
	}
};

}
