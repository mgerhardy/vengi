/**
 * @file
 */

#include "ConditionVariable.h"
#include "core/concurrent/Lock.h"
#include <SDL3/SDL_mutex.h>

namespace core {

ConditionVariable::ConditionVariable() {
	_conditionVariable = SDL_CreateCondition();
}

ConditionVariable::~ConditionVariable() {
	SDL_DestroyCondition(_conditionVariable);
}

bool ConditionVariable::notify_one() {
	SDL_SignalCondition(_conditionVariable);
	return true;
}

bool ConditionVariable::notify_all() {
	SDL_BroadcastCondition(_conditionVariable);
	return true;
}

bool ConditionVariable::wait(Lock& lock) {
	SDL_WaitCondition(_conditionVariable, lock.handle());
	return true;
}

ConditionVariableState ConditionVariable::waitTimeout(Lock& lock, uint32_t millis) {
	const int retVal = SDL_WaitConditionTimeout(_conditionVariable, lock.handle(), millis);
	if (retVal == 0) {
		return ConditionVariableState::Signaled;
	}
	if (retVal == -1) {
		return ConditionVariableState::Error;
	}
	return ConditionVariableState::Timeout;
}

}
