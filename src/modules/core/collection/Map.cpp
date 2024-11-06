/**
 * @file
 */

#include "Map.h"
#include <SDL3/SDL_stdinc.h>

namespace core {

size_t hashCharPtr::operator()(const char *p) const {
	size_t result = 0;
	const size_t prime = 31;
	const size_t s = SDL_strlen(p);
	for (size_t i = 0; i < s; ++i) {
		result = SDL_tolower(p[i]) + (result * prime);
	}
	return result;
}

bool hashCharCompare::operator()(const char *lhs, const char *rhs) const {
	return SDL_strcasecmp(lhs, rhs) == 0;
}

} // namespace core
