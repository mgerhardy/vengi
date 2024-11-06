/**
 * @file
 */

#pragma once

#include "core/String.h"
#include "core/concurrent/Concurrency.h"

struct SDL_Mutex;

namespace core {

class core_thread_capability("mutex") ReadWriteLock {
private:
	const core::String _name;
	mutable SDL_Mutex* _mutex;
public:
	ReadWriteLock(const core::String& name);
	~ReadWriteLock();

	void lockRead() const core_thread_acquire_shared();

	void unlockRead() const core_thread_release();

	void lockWrite() core_thread_acquire();

	void unlockWrite() core_thread_release();
};

class core_thread_scoped_capability ScopedReadLock {
private:
	const ReadWriteLock& _lock;
public:
	inline ScopedReadLock(const ReadWriteLock& lock) core_thread_acquire_shared(lock) : _lock(lock) {
		_lock.lockRead();
	}
	inline ~ScopedReadLock() core_thread_release() {
		_lock.unlockRead();
	}
};

class core_thread_scoped_capability ScopedWriteLock {
private:
	ReadWriteLock& _lock;
public:
	inline ScopedWriteLock(ReadWriteLock& lock) core_thread_acquire(lock): _lock(lock) {
		_lock.lockWrite();
	}
	inline ~ScopedWriteLock() core_thread_release() {
		_lock.unlockWrite();
	}
};

}
