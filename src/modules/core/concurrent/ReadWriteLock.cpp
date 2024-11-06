/**
 * @file
 */

#include "ReadWriteLock.h"
#include <SDL3/SDL_mutex.h>

namespace core {

ReadWriteLock::ReadWriteLock(const core::String& name) :
		_name(name), _mutex(SDL_CreateMutex()) {
}

ReadWriteLock::~ReadWriteLock() {
	SDL_DestroyMutex(_mutex);
}

void ReadWriteLock::lockRead() const {
	SDL_LockMutex(_mutex);
}

void ReadWriteLock::unlockRead() const {
	SDL_UnlockMutex(_mutex);
}

void ReadWriteLock::lockWrite() {
	SDL_LockMutex(_mutex);
}

void ReadWriteLock::unlockWrite() {
	SDL_UnlockMutex(_mutex);
}

}
