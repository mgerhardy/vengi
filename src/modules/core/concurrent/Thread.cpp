/**
 * @file
 */

#include "Thread.h"
#include <SDL3/SDL_thread.h>

namespace core {

Thread::Thread(const core::String &name, ThreadFunction fn, void *data) {
	_thread = SDL_CreateThread(fn, name.c_str(), data);
}

Thread::~Thread() {
	SDL_WaitThread(_thread, nullptr);
}

void Thread::detach() {
	SDL_DetachThread(_thread);
	_thread = nullptr;
}

ThreadId Thread::id() const {
	return SDL_GetThreadID(_thread);
}

int Thread::join() {
	int status = 0;
	SDL_WaitThread(_thread, &status);
	_thread = nullptr;
	return status;
}

}
