/**
 * @file
 */

#include "Atomic.h"
#include <SDL3/SDL_atomic.h>

namespace core {

AtomicBool::AtomicBool(bool value) {
	SDL_SetAtomicInt(&_value, (int)value);
}

AtomicBool::operator bool() const {
	return SDL_GetAtomicInt(const_cast<SDL_AtomicInt*>(&_value)) == 1;
}

bool AtomicBool::compare_exchange(bool expectedVal, bool newVal) {
	return (bool)SDL_CompareAndSwapAtomicInt(&_value, (int)expectedVal, (int)newVal);
}

bool AtomicBool::exchange(bool rhs) {
	return (bool)SDL_SetAtomicInt(&_value, rhs);
}

void AtomicBool::operator=(bool rhs) {
	SDL_SetAtomicInt(&_value, (int)rhs);
}

void AtomicBool::operator=(const AtomicBool& rhs) {
	SDL_SetAtomicInt(&_value, SDL_GetAtomicInt(const_cast<SDL_AtomicInt*>(&rhs._value)));
}

bool AtomicBool::operator==(bool rhs) const {
	return (bool)(*this) == rhs;
}

bool AtomicBool::operator==(const AtomicBool& rhs) const {
	return (bool)(*this) == (bool)rhs;
}

AtomicInt::AtomicInt(int value) {
	SDL_SetAtomicInt(&_value, value);
}

AtomicInt::operator int() const {
	return SDL_GetAtomicInt(const_cast<SDL_AtomicInt*>(&_value));
}

int AtomicInt::exchange(int rhs) {
	return SDL_SetAtomicInt(&_value, rhs);
}

bool AtomicInt::compare_exchange(int expectedVal, int newVal) {
	return (bool)SDL_CompareAndSwapAtomicInt(&_value, expectedVal, newVal);
}

void AtomicInt::operator=(int rhs) {
	SDL_SetAtomicInt(&_value, rhs);
}

void AtomicInt::operator=(const AtomicInt& rhs) {
	SDL_SetAtomicInt(&_value, SDL_GetAtomicInt(const_cast<SDL_AtomicInt*>(&rhs._value)));
}

bool AtomicInt::operator==(int rhs) const {
	return (int)(*this) == rhs;
}

bool AtomicInt::operator==(const AtomicInt& rhs) const {
	return (int)(*this) == (int)rhs;
}

AtomicInt& AtomicInt::operator--() {
	SDL_AddAtomicInt(&_value, -1);
	return *this;
}

AtomicInt& AtomicInt::operator++() {
	SDL_AddAtomicInt(&_value, 1);
	return *this;
}

int AtomicInt::decrement(int value) {
	return SDL_AddAtomicInt(&_value, -value);
}

int AtomicInt::increment(int value) {
	return SDL_AddAtomicInt(&_value, value);
}

}
