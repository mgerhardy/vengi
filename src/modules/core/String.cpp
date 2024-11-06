/**
 * @file
 */

#include "String.h"
#include "core/Common.h"
#include "core/StandardLib.h"
#include "engine-config.h"
#include <string.h>
#include <inttypes.h>
#include "core/Assert.h"
#include <SDL3/SDL_stdinc.h>

namespace core {

const size_t String::npos = ~0u;

size_t StringHash::operator()(const core::String &p) const {
	size_t result = 0;
	const size_t prime = 31;
	const size_t s = p.size();
	for (size_t i = 0; i < s; ++i) {
		result = SDL_tolower(p[i]) + (result * prime);
	}
	return result;
}

static inline constexpr size_t align(size_t val, size_t size) {
	const size_t len = size - 1u;
	return (size_t)((val + len) & ~len);
}

void String::checkBufferSize(size_t len) {
	if (onStack() && len <= _stackBufCapacity) {
		return;
	}

	if (_data._capacity > 0u && len <= _data._capacity) {
		return;
	}
	if (_data._capacity <= 0) {
		// string must be moved over because it exceeds the internal buffer
		_data._str = (char*)SDL_malloc(len);
		SDL_memcpy(_data._str, _buf, _data._size + 1);
		_data._capacity = len;
	} else {
		_data._capacity = align(len, 32);
		_data._str = (char*)SDL_realloc(_data._str, _data._capacity);
	}
}

void String::copyBuf(const char *str, size_t len) {
	_buf[0] = '\0';

	_data._size = len;

	if (len >= _stackBufCapacity) {
		if (_data._capacity > 0u) {
			SDL_free(_data._str);
		}
		_data._capacity = len + 1;
		_data._str = (char*)SDL_malloc(_data._capacity);
	} else {
		if (_data._capacity > 0u) {
			SDL_free(_data._str);
		}
		_data._str = _buf;
		_data._capacity = 0u;
	}

	SDL_memcpy(_data._str, str, len);
	_data._str[len] = '\0';
}

String::String(size_t len, char chr) {
	_buf[0] = '\0';
	_data._size = len;

	if (len >= _stackBufCapacity) {
		_data._capacity = len + 1;
		_data._str = (char*)SDL_malloc(_data._capacity);
	} else {
		_data._str = _buf;
		_data._capacity = 0u;
	}

	for (size_t i = 0; i < len; ++i) {
		_data._str[i] = chr;
	}
	_data._str[len] = '\0';
}

String::String(const char *str) :
		String(str, SDL_strlen(str)) {
}

String::String(const String &str) :
		String(str.c_str(), str.size()) {
}

String::String(const char *str, size_t len) {
	copyBuf(str, len);
}

String::String(String &&str) noexcept {
	if (str.onStack()) {
		SDL_memcpy(_buf, str._buf, _stackBufCapacity);
		str._buf[0] = '\0';
		_data._str = _buf;
		_data._size = str._data._size;
	} else {
		_data._str = str._data._str;
		_data._capacity = str._data._capacity;
		_data._size = str._data._size;
		str._data._str = str._buf;
		str._buf[0] = '\0';
		str._data._capacity = 0u;
		str._data._size = 0u;
	}
}

String::~String() {
	if (onStack()) {
		return;
	}
	SDL_free(_data._str);
#ifdef DEBUG
	_data._str = nullptr;
	_data._capacity = 0u;
#endif
}

const char& String::operator[](size_t idx) const {
	core_assert(_data._str && idx <= _data._size);
	return _data._str[idx];
}

char& String::operator[](size_t idx) {
	core_assert(_data._str && idx <= _data._size);
	return _data._str[idx];
}

int String::compare(const char *str, size_t len) const {
	return SDL_strncmp(_data._str, str, len);
}

int String::compare(size_t index, size_t len, const String& str) const {
	return SDL_strncmp(_data._str + index, str.c_str(), core_min(len, str.size()));
}

int String::compare(const String& str) const {
	return SDL_strcmp(_data._str, str.c_str());
}

int String::compare(const char *str) const {
	return SDL_strcmp(_data._str, str);
}

bool String::equals(const char *str) const {
	const size_t strLen = SDL_strlen(str);
	if (_data._size != strLen) {
		return false;
	}
	if (SDL_memcmp(_data._str, str, strLen) != 0) {
		return false;
	}
	return true;
}

void String::replaceAllChars(char in, char out) {
	for (char* i = _data._str; i < _data._str + _data._size; ++i) {
		if (*i == in) {
			*i = out;
		}
	}
}

void String::reserve(size_t bytes) {
	checkBufferSize(bytes);
}

void String::clear() {
	_data._size = 0u;
	if (onStack()) {
		_data._str[0] = '\0';
		return;
	}
	SDL_free(_data._str);
	_data._str = _buf;
	_data._capacity = 0u;
}

bool String::contains(const core::String& str) const {
	return contains(str.c_str(), str.size());
}

void String::erase(size_t index, size_t length) {
	if (core_unlikely(index >= size())) {
		return;
	}

	const size_t end = index + length;
	if (length == npos || end >= _data._size) {
		_data._str[index] = '\0';
		_data._size = index;
		return;
	}

	for (; index + length <= _data._size; ++index) {
		_data._str[index] = _data._str[index + length];
	}
	_data._size -= length;
}

bool String::contains(const char *str, size_t len) const {
	const String other(str, len);
	char *pos = (char*)SDL_strstr(c_str(), other.c_str());
	return pos != nullptr;
}

String String::substr(size_t index, size_t len) const {
	if (core_unlikely(index >= size())) {
		return String();
	}
	return String(_data._str + index, core_min(len, _data._size - index));
}

String String::lower(const char *string) {
	core::String str(string);
	for (char* i = str._data._str; i != str._data._str + str._data._size; ++i) {
		*i = (char)SDL_tolower(*i);
	}
	return str;
}

String String::upper(const char *string) {
	core::String str(string);
	for (char* i = str._data._str; i != str._data._str + str._data._size; ++i) {
		*i = (char)SDL_toupper(*i);
	}
	return str;
}

String String::toLower() const {
	return lower(_data._str);
}

String String::toUpper() const {
	return upper(_data._str);
}

void String::updateSize() {
	_data._size = SDL_strlen(c_str());
}

String &String::operator=(const String &str) {
	if (&str == this) {
		return *this;
	}
	copyBuf(str.c_str(), str.size());
	return *this;
}

String &String::operator=(const char *str) {
	if (str == c_str()) {
		return *this;
	}
	if (str > begin() && str < end()) {
		// str pointer might get invalid after the assignment
		core::String copy(str);
		*this = copy;
		return *this;
	}
	const size_t len = SDL_strlen(str);
	copyBuf(str, len);
	return *this;
}

String &String::operator=(String &&str) noexcept {
	if (&str == this) {
		return *this;
	}
	if (_data._capacity > 0u) {
		_data._capacity = 0u;
		SDL_free(_data._str);
	}
	if (str.onStack()) {
		SDL_memcpy(_buf, str._buf, str.size() + 1);
		str._buf[0] = '\0';
		_data._str = _buf;
		_data._size = str._data._size;
	} else {
		_data._str = str._data._str;
		_data._capacity = str._data._capacity;
		_data._size = str._data._size;
		str._data._str = str._buf;
		str._data._capacity = 0u;
		str._data._size = 0u;
	}
	return *this;
}

String &String::operator=(char c) {
	_data._str = _buf;
	_buf[0] = c;
	_buf[1] = '\0';
	_data._size = SDL_strlen(_buf);
	return *this;
}

String &String::operator+=(const char *str) {
	if (str == nullptr) {
		return *this;
	}
	const size_t len = SDL_strlen(str);
	if (len > 0) {
		checkBufferSize(_data._size + len + 1);
		SDL_memcpy(_data._str + _data._size, str, len + 1);
		_data._size += len;
	}
	return *this;
}

String &String::operator+=(const String &str) {
	return operator+=(str.c_str());
}

String& String::append(int c) {
	char text[16];
	SDL_snprintf(text, sizeof(text), "%i", c);
	append(text);
	return *this;
}

String& String::append(float c) {
	char text[16];
	SDL_snprintf(text, sizeof(text), "%f", c);
	append(text);
	return *this;
}

String& String::append(const char *str) {
	operator+=(str);
	return *this;
}

String& String::append(const char *str, size_t len) {
	if (len == 0) {
		return *this;
	}
	checkBufferSize(_data._size + len + 1);
	SDL_memcpy(_data._str + _data._size, str, len);
	_data._str[_data._size + len] = '\0';
	_data._size += len;
	return *this;
}

String& String::append(const String &str) {
	operator+=(str.c_str());
	return *this;
}

void String::replace(size_t index, size_t length, const char *str) {
	erase(index, length);
	insert(index, str);
}

void String::insert(size_t index, const char *str, size_t len) {
	if (len == 0) {
		return;
	}
	if (index == size()) {
		append(str, len);
		return;
	}
	const size_t newSize = _data._size + len + 1;
	checkBufferSize(newSize);
	SDL_memmove(_data._str + index + len, _data._str + index, _data._size - index + 1);
	SDL_memcpy(_data._str + index, str, len);
	_data._size += len;
}

void String::insert(size_t index, const char *str) {
	if (index == size()) {
		append(str);
		return;
	}
	const size_t length = SDL_strlen(str);
	if (length == 0) {
		return;
	}
	const size_t newSize = _data._size + length + 1;
	checkBufferSize(newSize);
	SDL_memmove(_data._str + index + length, _data._str + index, _data._size - index + 1);
	SDL_memcpy(_data._str + index, str, length);
	_data._size += length;
}

size_t String::rfind(const char *s) const {
	const size_t tokenLength = SDL_strlen(s);
	for (int i = (int)(_data._size - tokenLength); i >= 0; --i) {
		if (!SDL_strncmp(_data._str + i, s, tokenLength)) {
			return i;
		}
	}
	return npos;
}

size_t String::rfind(char c) const {
	if (core_unlikely(_data._size == 0)) {
		return npos;
	}
	for (int i = (int)_data._size - 1; i >= 0; --i) {
		if (_data._str[i] == c) {
			return i;
		}
	}
	return npos;
}

size_t String::find_first_of(const char *str, size_t pos) const {
	for (size_t idx = pos; idx < _data._size; ++idx) {
		if (SDL_strchr(str, (*this)[idx]) != nullptr) {
			return idx;
		}
	}

	return npos;
}

size_t String::find_first_of(char c, size_t pos) const {
	if (core_unlikely(pos >= _data._size)) {
		return npos;
	}
	const char *p = SDL_strchr(_data._str + pos, c);
	if (p == nullptr) {
		return npos;
	}
	return (size_t)(p - _data._str);
}

size_t String::find(const char* str, size_t pos) const {
	if (core_unlikely(pos >= size())) {
		return npos;
	}

	const char *f = SDL_strstr(c_str() + pos, str);
	if (f == nullptr) {
		return npos;
	}

	return (size_t)(f - c_str());
}

size_t String::find(const String &str, size_t pos) const {
	return find(str.c_str(), pos);
}

size_t String::find_first_not_of(const char *str, size_t pos) const {
	for (size_t idx = pos; idx < _data._size; ++idx) {
		if (SDL_strchr(str, (*this)[idx]) == nullptr) {
			return idx;
		}
	}
	return npos;
}

size_t String::find_last_of(const char *chars, size_t pos) const {
	if (core_unlikely(pos >= size())) {
		return npos;
	}

	for (int idx = (int)size() - 1; idx >= (int)pos; --idx) {
		if (SDL_strchr(chars, _data._str[idx]) != nullptr) {
			return idx;
		}
	}
	return npos;
}

String String::trim() const {
	if (core_unlikely(empty())) {
		return String();
	}

	String copy(*this);
	while (copy._data._size >= 1 && SDL_isspace(copy._data._str[copy._data._size - 1])) {
		--copy._data._size;
	}
	copy._data._str[copy._data._size] = 0;

	const char *pos = copy._data._str;
	while (SDL_isspace(*pos)) {
		++pos;
	}

	if (pos != copy._data._str) {
		const size_t delta = pos - copy._data._str;
		copy._data._size -= delta;
		SDL_memmove(copy._data._str, pos, copy._data._size + 1);
	}
	return copy;
}

core::String String::format(const char *msg, ...) {
	va_list ap;
	constexpr size_t bufSize = 1024;
	char text[bufSize];

	va_start(ap, msg);
	SDL_vsnprintf(text, bufSize, msg, ap);
	text[sizeof(text) - 1] = '\0';
	va_end(ap);

	return String(text);
}

char String::last() const {
	if (empty()) {
		return '\0';
	}
	return _data._str[_data._size - 1];
}

char String::first() const {
	if (empty()) {
		return '\0';
	}
	return _data._str[0];
}

String &String::operator+=(char c) {
	if (c == '\0') {
		return *this;
	}
	checkBufferSize(_data._size + 2);
	_data._str[_data._size + 0] = c;
	_data._str[_data._size + 1] = '\0';
	++_data._size;
	return *this;
}

bool String::operator==(const String &rhs) const {
	return equals(rhs.c_str());
}

bool String::operator==(const char *rhs) const {
	return equals(rhs);
}

bool String::operator!=(const String &rhs) const {
	return !equals(rhs.c_str());
}

bool String::operator !=(const char *rhs) const {
	return !equals(rhs);
}

bool String::operator<(const String &rhs) const {
	return compare(rhs) < 0;
}

bool String::operator<=(const String &rhs) const {
	return compare(rhs) <= 0;
}

bool String::operator>(const String &rhs) const {
	return compare(rhs) > 0;
}

bool String::operator>=(const String &rhs) const {
	return compare(rhs) >= 0;
}

bool operator==(const char* lhs, const String &rhs) {
	return rhs == lhs;
}

bool operator!=(const char* lhs, const String &rhs) {
	return rhs != lhs;
}

String operator+(const String &lhs, const String &rhs) {
	String tmp(lhs);
	tmp += rhs;
	return tmp;
}

String operator+(const char *lhs, const String &rhs) {
	String tmp(lhs);
	tmp += rhs;
	return tmp;
}

String operator+(const String &lhs, const char *rhs) {
	String tmp(lhs);
	tmp += rhs;
	return tmp;
}

String operator+(char lhs, const String &rhs) {
	const char buf[2] = {lhs, '\0'};
	String tmp(buf);
	tmp += rhs;
	return tmp;
}

String operator+(const String &lhs, char rhs) {
	String tmp(lhs);
	tmp += rhs;
	return tmp;
}

int String::toInt() const {
	return SDL_atoi(_data._str);
}

float String::toFloat() const {
	return (float)SDL_atof(_data._str);
}

}
