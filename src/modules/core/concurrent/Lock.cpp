/**
 * @file
 */

#include "Lock.h"
#include <SDL3/SDL_mutex.h>

namespace core {

#ifdef TRACY_ENABLE
Lock::Lock(const tracy::SourceLocationData& srcloc) : _mutex(SDL_CreateMutex()), _srcLoc(srcloc), _ctx(&_srcLoc) {
}

void Lock::Mark(const tracy::SourceLocationData *srcloc) {
	_ctx.Mark(srcloc);
}

void Lock::CustomName(const char *name, size_t size) {
	_ctx.CustomName(name, size);
}
#else
Lock::Lock() :
		_mutex(SDL_CreateMutex()) {
}
#endif

Lock::~Lock() {
	SDL_DestroyMutex(_mutex);
}

SDL_Mutex* Lock::handle() {
	return _mutex;
}

void Lock::lock() const {
#ifdef TRACY_ENABLE
	const auto runAfter = _ctx.BeforeLock();
	SDL_LockMutex(_mutex);
	if (runAfter) {
		_ctx.AfterLock();
	}
#else
	SDL_LockMutex(_mutex);
#endif
}

void Lock::unlock() const {
	SDL_UnlockMutex(_mutex);
#ifdef TRACY_ENABLE
	_ctx.AfterUnlock();
#endif
}

bool Lock::try_lock() const {
	const bool acquired = SDL_TryLockMutex(_mutex) == 0;
#ifdef TRACY_ENABLE
	_ctx.AfterTryLock(acquired);
#endif
	return acquired;
}

}
