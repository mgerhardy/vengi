/**
 * @file
 */

#pragma once

#include "core/concurrent/Atomic.h"
#include "core/Common.h"
#include "core/StandardLib.h"
#include <new>
#include <stddef.h>
#include <type_traits>

namespace core {

template<class T>
class SharedPtr {
private:
	template <typename U> friend class SharedPtr;

	struct __enableIfHelper {
		int b;
	};

	T *_ptr;
	core::AtomicInt *_refCnt;

	int count() const {
		if (_refCnt == nullptr) {
			return 0;
		}
		return *_refCnt;
	}

	void increase() {
		if (_refCnt == nullptr) {
			return;
		}
		_refCnt->increment(1);
	}

	int decrease() {
		if (_refCnt == nullptr) {
			return -1;
		}
		return _refCnt->decrement(1) - 1;
	}
public:
	constexpr SharedPtr() : _ptr(nullptr), _refCnt(nullptr) {
	}

	constexpr SharedPtr(decltype(nullptr)) : _ptr(nullptr), _refCnt(nullptr) {
	}

	SharedPtr(const SharedPtr &obj) : _ptr(obj.get()), _refCnt(obj.refCnt()) {
		increase();
	}

	SharedPtr(SharedPtr &&obj) noexcept : _ptr(obj.get()), _refCnt(obj.refCnt()) {
		obj._ptr = nullptr;
		obj._refCnt = nullptr;
	}

	template <class U>
	SharedPtr(const SharedPtr<U> &obj, typename std::enable_if<std::is_convertible<U*, T*>::value, __enableIfHelper>::type = __enableIfHelper()) :
			_ptr(obj.get()), _refCnt(obj.refCnt()) {
		increase();
	}

	template <class U>
	SharedPtr(SharedPtr<U> &&obj, typename std::enable_if<std::is_convertible<U*, T*>::value, __enableIfHelper>::type = __enableIfHelper()) :
			_ptr(obj.get()), _refCnt(obj.refCnt()) {
		obj._ptr = nullptr;
		obj._refCnt = nullptr;
	}

	template<typename ... Args>
	static SharedPtr<T> create(Args&&... args) {
		const size_t size = sizeof(T) + sizeof(AtomicInt);
		void *ptr = core_malloc(size);
		SharedPtr<T> d;
		d._ptr = new (ptr) T(core::forward<Args>(args)...);
		d._refCnt = new ((void*)((uint8_t*)ptr + sizeof(*_ptr))) AtomicInt(1);
		return d;
	}

	SharedPtr &operator=(const SharedPtr &obj) {
		if (&obj == this || _ptr == obj._ptr) {
			return *this;
		}
		release();
		_ptr = obj._ptr;
		_refCnt = obj._refCnt;
		increase();
		return *this;
	}

	SharedPtr &operator=(SharedPtr &&obj) noexcept {
		if (&obj == this || _ptr == obj._ptr) {
			return *this;
		}
		release();
		_ptr = obj._ptr;
		_refCnt = obj._refCnt;
		obj._ptr = nullptr;
		obj._refCnt = nullptr;
		return *this;
	}

	~SharedPtr() {
		release();
	}

	core::AtomicInt* refCnt() const {
		return _refCnt;
	}

	void release() {
		if (decrease() == 0) {
			if (_ptr != nullptr) {
				_ptr->~T();
			}
			if (_refCnt != nullptr) {
				_refCnt->~AtomicInt();
			}
			core_free((void*)_ptr);
		}
		_ptr = nullptr;
		_refCnt = nullptr;
	}

	T *get() const {
		return _ptr;
	}

	T *operator->() const {
		return _ptr;
	}

	void operator=(decltype(nullptr)) {
		release();
	}

	operator bool() const {
		return *this != nullptr;
	}

	bool operator==(const SharedPtr &rhs) const {
		return _ptr == rhs._ptr;
	}

	bool operator!=(const SharedPtr &rhs) const {
		return _ptr != rhs._ptr;
	}

	bool operator<(const SharedPtr &rhs) const {
		return _ptr < rhs._ptr;
	}

	bool operator>(const SharedPtr &rhs) const {
		return _ptr > rhs._ptr;
	}

	bool operator<=(const SharedPtr &rhs) const {
		return _ptr <= rhs._ptr;
	}

	bool operator>=(const SharedPtr &rhs) const {
		return _ptr >= rhs._ptr;
	}

	bool operator==(decltype(nullptr)) const {
		return nullptr == _ptr;
	}

	bool operator!=(decltype(nullptr)) const {
		return nullptr != _ptr;
	}
};

template<class T, typename ... Args>
static SharedPtr<T> make_shared(Args&&... args) {
	return SharedPtr<T>::create(core::forward<Args>(args)...);
}

}
