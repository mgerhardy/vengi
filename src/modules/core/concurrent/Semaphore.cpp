/**
 * @file
 */

#include "Semaphore.h"
#include <SDL3/SDL_mutex.h>

namespace core {

Semaphore::Semaphore(uint32_t initialValue) :
		_semaphore(SDL_CreateSemaphore(initialValue)) {
}

Semaphore::~Semaphore() {
	SDL_DestroySemaphore(_semaphore);
}

bool Semaphore::waitAndDecrease() {
	SDL_WaitSemaphore(_semaphore);
	return true;
}

bool Semaphore::increase() {
	SDL_SignalSemaphore(_semaphore);
	return true;
}

SemaphoreWaitState Semaphore::tryWait() {
	const bool val = SDL_TryWaitSemaphore(_semaphore);
	if (!val) {
		return SemaphoreWaitState::WouldBlock;
	}
	return SemaphoreWaitState::Success;
}

bool Semaphore::waitTimeout(uint32_t timeout) {
	return SDL_WaitSemaphoreTimeout(_semaphore, timeout);
}

uint32_t Semaphore::value() {
	return SDL_GetSemaphoreValue(_semaphore);
}

}
