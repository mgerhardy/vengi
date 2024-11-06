/**
 * @file
 */

#include "StringUtil.h"
#include "core/ArrayLength.h"
#include "core/Common.h"
#include "core/StandardLib.h"
#include <SDL3/SDL_stdinc.h>
#include <ctype.h>
#include <glm/vec3.hpp>
#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace core {
namespace string {

size_t len(const char *string) {
	return SDL_strlen(string);
}

int64_t toLong(const char *str) {
	return ::atol(str);
}

int toInt(const char *str) {
	return SDL_atoi(str);
}

float toFloat(const core::String &str) {
	return (float)SDL_atof(str.c_str());
}

double toDouble(const core::String &str) {
	return (double)SDL_atof(str.c_str());
}

double toDouble(const char *str) {
	return (double)SDL_atof(str);
}

bool startsWith(const char *string, const char *token) {
	return !SDL_strncmp(string, token, SDL_strlen(token));
}

bool startsWith(const core::String &string, const char *token) {
	const size_t tokensize = SDL_strlen(token);
	if (string.size() < tokensize) {
		return false;
	}
	return !string.compare(0, tokensize, token);
}

bool contains(const char *haystack, const char *needle) {
	const char *pos = (const char *)SDL_strstr(haystack, needle);
	return pos != nullptr;
}

char toUpper(char in) {
	return (char)SDL_toupper((int)in);
}

char toLower(char in) {
	return (char)SDL_tolower((int)in);
}

bool iequals(const core::String &a, const core::String &b) {
	const size_t sz = a.size();
	if (b.size() != sz) {
		return false;
	}
	for (size_t i = 0u; i < sz; ++i) {
		if (toLower(a[i]) != toLower(b[i])) {
			return false;
		}
	}
	return true;
}

const char *after(const char *input, int character) {
	const char *s = SDL_strrchr(input, character);
	if (s != nullptr) {
		++s;
	}
	return s;
}

bool endsWith(const core::String &string, const core::String &end) {
	const size_t strLength = string.size();
	const size_t endLength = end.size();
	if (strLength >= endLength) {
		const size_t index = strLength - endLength;
		return string.compare(index, endLength, end) == 0;
	}
	return false;
}

core::String toHex(int32_t number) {
	constexpr size_t hexChars = sizeof(int32_t) << 1;
	static const char *digits = "0123456789abcdef";
	core::String rc(8, '0');
	for (size_t i = 0, j = (hexChars - 1) * 4; i < hexChars; ++i, j -= 4) {
		rc[i] = digits[(number >> j) & 0x0f];
	}
	return rc;
}

core::String eraseAllChars(const core::String &str, char chr) {
	if (str.empty()) {
		return str;
	}
	core::String tmp;
	tmp.reserve(str.size() + 1);
	for (auto c : str) {
		if (c == chr) {
			continue;
		}
		tmp += c;
	}
	return tmp;
}

core::String removeAnsiColors(const char* message) {
	core::String out(SDL_strlen(message) + 1, '\0');
	int i = 0;
	for (const char *c = message; *c != '\0'; ++c) {
		// https://en.wikipedia.org/wiki/ANSI_escape_code
		if (*c >= 030 && *c < 037 && *(c + 1) == '[') {
			c += 2;
			while (*c != 'm' && *c != '\0') {
				++c;
			}
			continue;
		}
		out[i++] = *c;
	}
	return out;
}

char *getBeforeToken(char **buffer, const char *token, size_t bufferSize) {
	if (bufferSize <= 0) {
		return nullptr;
	}
	char *begin = *buffer;
	const size_t length = SDL_strlen(token);
	while (**buffer) {
		if (bufferSize <= 0) {
			return nullptr;
		}
		if (SDL_strncmp(*buffer, token, length) == 0) {
			**buffer = '\0';
			*buffer += length;
			return begin;
		}
		++(*buffer);
		--bufferSize;
	}
	*buffer = begin;
	return nullptr;
}

bool formatBuf(char *buf, size_t bufSize, const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	const bool fit = SDL_vsnprintf(buf, bufSize, msg, ap) < (int)bufSize;
	buf[bufSize - 1] = '\0';
	va_end(ap);
	return fit;
}

// TODO: take NAME_MAX into account
core::String sanitizeFilename(const core::String &input) {
	static const char unsafeChars[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
									   0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
									   0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x22, 0x2a,
									   0x2f, 0x3a, 0x3c, 0x3e, 0x3f, 0x5c, 0x7c, 0x7f, 0x00};

	if (input.size() == 0) {
		return input;
	}

	core::String output = input;
	char *c = output.c_str();
	for (; *c; ++c) {
		if (SDL_strchr(unsafeChars, *c)) {
			*c = '_';
		}
	}
	while (output.contains("  ")) {
		output = replaceAll(output, "  ", " ");
	}
	return core::string::trim(output);
}

core::String format(const char *msg, ...) {
	va_list ap;
	const size_t bufSize = 1024;
	char text[bufSize];

	va_start(ap, msg);
	SDL_vsnprintf(text, bufSize, msg, ap);
	text[sizeof(text) - 1] = '\0';
	va_end(ap);

	return core::String(text);
}

core::String humanSize(uint64_t bytes) {
	static const char *units[] = {"B", "KB", "MB", "GB", "TB"};
	static const char length = lengthof(units);

	int unitIdx = 0;
	double dblBytes = bytes;
	for (; bytes / 1024 > 0 && unitIdx < length - 1; ++unitIdx, bytes /= 1024) {
		dblBytes = bytes / 1024.0;
	}

	return core::string::format("%.02lf%s", dblBytes, units[unitIdx]);
}

core::String urlEncode(const core::String &inBuf) {
	const char *inBufPos = inBuf.c_str();
	const size_t maxSize = inBuf.size() * 3;
	core::String outBuf;
	outBuf.reserve(maxSize);
	while (inBufPos[0] != '\0') {
		const uint8_t inChr = inBufPos[0];
		if (inChr == '-' || inChr == '.' || inChr == '~' || inChr == '_' || isalnum(inChr)) {
			outBuf += inChr;
		} else {
			outBuf += '%';
			outBuf += toHex((char)(inChr >> 4));
			outBuf += toHex((char)(inChr & 15));
		}
		++inBufPos;
	}
	return outBuf;
}

core::String urlPathEncode(const core::String &inBuf) {
	const char *inBufPos = inBuf.c_str();
	const size_t maxSize = inBuf.size() * 3;
	core::String outBuf;
	outBuf.reserve(maxSize);
	while (inBufPos[0] != '\0') {
		const uint8_t inChr = inBufPos[0];
		if (inChr == '/' || inChr == '-' || inChr == '.' || inChr == '~' || inChr == '_' || isalnum(inChr)) {
			outBuf += inChr;
		} else {
			outBuf += '%';
			outBuf += toHex((char)(inChr >> 4));
			outBuf += toHex((char)(inChr & 15));
		}
		++inBufPos;
	}
	return outBuf;
}

void replaceAllChars(core::String &str, char in, char out) {
	replaceAllChars(str.c_str(), in, out);
}

void replaceAllChars(char *str, char in, char out) {
	char *p = str;
	while (*p != '\0') {
		if (*p == in) {
			*p = out;
		}
		++p;
	}
}

core::String replaceAll(const core::String &str, const core::String &searchStr, const char *replaceStr) {
	return replaceAll(str, searchStr, replaceStr, SDL_strlen(replaceStr));
}

core::String replaceAll(const core::String &str, const core::String &searchStr, const char *replaceStr,
						size_t replaceStrSize) {
	if (str.empty()) {
		return str;
	}
	if (searchStr.empty()) {
		return str;
	}

	core::String s = str;
	for (size_t pos = s.find(searchStr); pos != core::String::npos; pos = s.find(searchStr, pos + replaceStrSize)) {
		s.replace(pos, searchStr.size(), replaceStr);
	}
	return s;
}

core::String replaceAll(const core::String &str, const core::String &searchStr, const core::String &replaceStr) {
	if (searchStr.size() == 1 && replaceStr.size() == 1) {
		core::String copy = str;
		replaceAllChars(copy, searchStr[0], replaceStr[0]);
		return copy;
	}
	return replaceAll(str, searchStr, replaceStr.c_str(), replaceStr.size());
}

void splitString(const core::String &string, core::DynamicArray<core::String> &tokens, const char *delimiters) {
	// Skip delimiters at beginning.
	size_t lastPos = string.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	size_t pos = string.find_first_of(delimiters, lastPos);

	while (core::String::npos != pos || core::String::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(string.substr(lastPos, pos - lastPos));
		// Skip delimiters. Note the "not_of"
		lastPos = string.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = string.find_first_of(delimiters, lastPos);
	}
}

bool isNumber(const core::String &in) {
	char *end = nullptr;
	double val = SDL_strtod(in.c_str(), &end);
	return end != in.c_str() && *end == '\0' && val != HUGE_VAL;
}

bool isAlphaNum(int c) {
	return ::SDL_isalnum(c);
}

bool isAlpha(int c) {
	return ::SDL_isalpha(c);
}

bool isInteger(const core::String &in) {
	for (size_t i = 0u; i < in.size(); i++) {
		if (!SDL_isdigit(in[i])) {
			if (i == 0 && in[i] == '-') {
				continue;
			}
			return false;
		}
	}
	return true;
}

bool isIntegerWithPostfix(const core::String &in) {
	for (size_t i = 0u; i < in.size(); i++) {
		if (!SDL_isdigit(in[i])) {
			if (i == 0 && in[i] == '-') {
				continue;
			}
			if (in[i] != 'u' && in[i] != 'U') {
				return false;
			}
		}
	}
	return true;
}

bool isAbsolutePath(const core::String &in) {
	if (in.size() >= 3U && isAlpha(in[0]) && in[1] == ':' && (in[2] == '\\' || in[2] == '/')) {
		return true;
	}
	return in.size() > 1U && (in[0] == '/' || in[0] == '\\');
}

bool isRootPath(const core::String &in) {
	if (in.size() == 3U && isAlpha(in[0]) && in[1] == ':' && (in[2] == '\\' || in[2] == '/')) {
		return true;
	}
	return in.size() == 1U && (in[0] == '/' || in[0] == '\\');
}

// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C++
size_t levensteinDistance(const core::String &source, const core::String &target) {
	if (source.size() > target.size()) {
		return levensteinDistance(target, source);
	}

	const size_t minSize = source.size();
	const size_t maxSize = target.size();
	core::DynamicArray<size_t> levDist(minSize + 1);

	for (size_t i = 0; i <= minSize; ++i) {
		levDist[i] = i;
	}

	for (size_t j = 1; j <= maxSize; ++j) {
		size_t previousDiagonal = levDist[0];
		++levDist[0];

		for (size_t i = 1; i <= minSize; ++i) {
			const size_t previousDiagonalSave = levDist[i];
			if (source[i - 1] == target[j - 1]) {
				levDist[i] = previousDiagonal;
			} else {
				levDist[i] = core_min(core_min(levDist[i - 1], levDist[i]), previousDiagonal) + 1;
			}
			previousDiagonal = previousDiagonalSave;
		}
	}

	return levDist[minSize];
}

static bool patternMatch(const char *text, const char *pattern);
static bool patternMatchMulti(const char *text, const char *pattern) {
	const char *p = pattern;
	const char *t = text;
	char c;

	for (;;) {
		c = *p++;
		if (c != '?' && c != '*') {
			break;
		}
		if (*t++ == '\0' && c == '?') {
			return false;
		}
	}

	if (c == '\0') {
		return true;
	}

	const size_t l = SDL_strlen(t);
	for (size_t i = 0; i < l; ++i) {
		if (*t == c && patternMatch(p - 1, t)) {
			return true;
		}
		if (*t++ == '\0') {
			return false;
		}
	}
	return false;
}

static bool patternMatch(const char *text, const char *pattern) {
	const char *p = pattern;
	const char *t = text;
	char c;

	while ((c = *p++) != '\0') {
		switch (c) {
		case '*':
			return patternMatchMulti(t, p);
		case '?':
			if (*t == '\0') {
				return false;
			}
			++t;
			break;
		default:
			if (c != *t++) {
				return false;
			}
			break;
		}
	}
	return *t == '\0';
}

bool matches(const char *text, const core::String &pattern) {
	if (pattern.empty()) {
		return true;
	}
	return patternMatch(text, pattern.c_str());
}

bool matches(const char *text, const char *pattern) {
	if (pattern == nullptr || pattern[0] == '\0') {
		return true;
	}
	return patternMatch(text, pattern);
}

bool fileMatchesMultiple(const char *text, const char *patterns) {
	char buf[4096];
	SDL_strlcpy(buf, patterns, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	char *sep = SDL_strstr(buf, ",");
	if (sep == nullptr) {
		return core::string::matches(text, buf);
	}

	char *f = buf;
	while (*sep == ',') {
		*sep = '\0';
		char patternBuf[32];
		SDL_strlcpy(patternBuf, f, sizeof(patternBuf));
		if (core::string::matches(text, patternBuf)) {
			return true;
		}
		f = ++sep;
		sep = SDL_strchr(f, ',');
		if (sep == nullptr) {
			break;
		}
	}
	char patternBuf[32];
	SDL_strlcpy(patternBuf, f, sizeof(patternBuf));
	return core::string::matches(text, patternBuf);
}

static void camelCase(core::String &str, bool upperCamelCase) {
	if (str.empty()) {
		return;
	}

	size_t startIndex = str.find_first_not_of("_", 0);
	if (startIndex == core::String::npos) {
		str = "";
		return;
	}
	if (startIndex > 0) {
		str = str.substr(startIndex);
	}
	size_t pos = str.find_first_of("_", 0);
	while (core::String::npos != pos) {
		core::String sub = str.substr(0, pos);
		core::String second = str.substr(pos + 1, str.size() - (pos + 1));
		if (!second.empty()) {
			second[0] = toUpper(second[0]);
			sub.append(second);
		}
		str = sub;
		if (str.empty()) {
			return;
		}
		pos = str.find_first_of("_", pos);
	}
	if (str.empty()) {
		return;
	}
	if (!upperCamelCase) {
		str[0] = toLower(str[0]);
	} else {
		str[0] = toUpper(str[0]);
	}
}

core::String lowerCamelCase(const core::String &str) {
	core::String copy = str;
	lowerCamelCase(copy);
	return copy;
}

core::String upperCamelCase(const core::String &str) {
	core::String copy = str;
	upperCamelCase(copy);
	return copy;
}

void upperCamelCase(core::String &str) {
	camelCase(str, true);
}

void lowerCamelCase(core::String &str) {
	camelCase(str, false);
}

char *append(char *buf, size_t bufsize, const char *string) {
	const size_t bufl = SDL_strlen(buf);
	if (bufl >= bufsize) {
		return nullptr;
	}
	const size_t remaining = bufsize - bufl;
	if (remaining <= 1u) {
		return nullptr;
	}
	const size_t l = SDL_strlen(string);
	if (remaining <= l) {
		return nullptr;
	}
	char *p = buf + bufl;
	for (size_t i = 0u; i < l; ++i) {
		*p++ = *string++;
	}
	*p = '\0';
	return p;
}

int count(const char *buf, char chr) {
	if (buf == nullptr) {
		return 0;
	}
	int count = 0;
	for (;;) {
		buf = SDL_strchr(buf, chr);
		if (buf == nullptr) {
			break;
		}
		++buf;
		++count;
	}
	return count;
}

#define IS_DIGIT(x) (static_cast<unsigned int>((x) - '0') < static_cast<unsigned int>(10))
#define ISUPPERHEX(X) (((X) >= 'A') && ((X) <= 'F'))
#define ISLOWERHEX(X) (((X) >= 'a') && ((X) <= 'f'))

int parseHex(const char *hex, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
	if (hex[0] == '#') {
		++hex;
	} else if (strlen(hex) >= 2 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
		hex += 2;
	}
	int n = 0;
	r = g = b = 0u;
	a = 255u;
	const size_t l = strlen(hex);
	if (l % 2 != 0) {
		return -1;
	}
	if (l == 0) {
		return 0;
	}

	while (hex[0] != '\0') {
		uint8_t value = 0;
		for (int i = 0; i < 2; ++i) {
			uint8_t v;
			if (IS_DIGIT((unsigned int)*hex)) {
				v = *hex - '0';
			} else if (ISUPPERHEX((int)*hex)) {
				v = 10 + (*hex - 'A');
			} else if (ISLOWERHEX((int)*hex)) {
				v = 10 + (*hex - 'a');
			} else {
				return -1;
			}
			value *= 16;
			value += v;
			++hex;
		}
		switch (n) {
		case 0:
			r = value;
			break;
		case 1:
			g = value;
			break;
		case 2:
			b = value;
			break;
		case 3:
			a = value;
			break;
		}
		++n;
	}

	return n;
}

// Tries to parse a floating point number located at s.
// this code is part of tiny_obj_loader
//
// s_end should be a location in the string where reading should absolutely
// stop. For example at the end of the string, to prevent buffer overflows.
//
// Parses the following EBNF grammar:
//   sign    = "+" | "-" ;
//   END     = ? anything not in digit ?
//   digit   = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
//   integer = [sign] , digit , {digit} ;
//   decimal = integer , ["." , integer] ;
//   float   = ( decimal , END ) | ( decimal , ("E" | "e") , integer , END ) ;
//
//  Valid strings are for example:
//   -0  +3.1417e+2  -0.0E-3  1.0324  -1.41   11e2
//
// If the parsing is a success, result is set to the parsed value and true
// is returned.
//
// The function is greedy and will parse until any of the following happens:
//  - a non-conforming character is encountered.
//  - s_end is reached.
//
// The following situations triggers a failure:
//  - s >= s_end.
//  - parse failure.
//
static bool tryParseDouble(const char *s, const char *s_end, double *result) {
	if (s >= s_end) {
		return false;
	}

	double mantissa = 0.0;
	// This exponent is base 2 rather than 10.
	// However the exponent we parse is supposed to be one of ten,
	// thus we must take care to convert the exponent/and or the
	// mantissa to a * 2^E, where a is the mantissa and E is the
	// exponent.
	// To get the final double we will use ldexp, it requires the
	// exponent to be in base 2.
	int exponent = 0;

	// NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
	// TO JUMP OVER DEFINITIONS.
	char sign = '+';
	char exp_sign = '+';
	char const *curr = s;

	// How many characters were read in a loop.
	int read = 0;
	// Tells whether a loop terminated due to reaching s_end.
	bool end_not_reached = false;
	bool leading_decimal_dots = false;

	/*
			BEGIN PARSING.
	*/

	// Find out what sign we've got.
	if (*curr == '+' || *curr == '-') {
		sign = *curr;
		curr++;
		if ((curr != s_end) && (*curr == '.')) {
			// accept. Somethig like `.7e+2`, `-.5234`
			leading_decimal_dots = true;
		}
	} else if (IS_DIGIT(*curr)) { /* Pass through. */
	} else if (*curr == '.') {
		// accept. Somethig like `.7e+2`, `-.5234`
		leading_decimal_dots = true;
	} else {
		goto fail;
	}

	// Read the integer part.
	end_not_reached = (curr != s_end);
	if (!leading_decimal_dots) {
		while (end_not_reached && IS_DIGIT(*curr)) {
			mantissa *= 10;
			mantissa += static_cast<int>(*curr - 0x30);
			curr++;
			read++;
			end_not_reached = (curr != s_end);
		}

		// We must make sure we actually got something.
		if (read == 0)
			goto fail;
	}

	// We allow numbers of form "#", "###" etc.
	if (!end_not_reached)
		goto assemble;

	// Read the decimal part.
	if (*curr == '.') {
		curr++;
		read = 1;
		end_not_reached = (curr != s_end);
		while (end_not_reached && IS_DIGIT(*curr)) {
			static const double pow_lut[] = {
				1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001,
			};
			const int lut_entries = lengthof(pow_lut);

			// NOTE: Don't use powf here, it will absolutely murder precision.
			mantissa += static_cast<int>(*curr - 0x30) * (read < lut_entries ? pow_lut[read] : powf(10.0, -read));
			read++;
			curr++;
			end_not_reached = (curr != s_end);
		}
	} else if (*curr == 'e' || *curr == 'E') {
	} else {
		goto assemble;
	}

	if (!end_not_reached)
		goto assemble;

	// Read the exponent part.
	if (*curr == 'e' || *curr == 'E') {
		curr++;
		// Figure out if a sign is present and if it is.
		end_not_reached = (curr != s_end);
		if (end_not_reached && (*curr == '+' || *curr == '-')) {
			exp_sign = *curr;
			curr++;
		} else if (IS_DIGIT(*curr)) { /* Pass through. */
		} else {
			// Empty E is not allowed.
			goto fail;
		}

		read = 0;
		end_not_reached = (curr != s_end);
		while (end_not_reached && IS_DIGIT(*curr)) {
			// To avoid annoying MSVC's min/max macro definiton,
			// Use hardcoded int max value
			if (exponent > (2147483647 / 10)) { // 2147483647 = std::numeric_limits<int>::max()
				// Integer overflow
				goto fail;
			}
			exponent *= 10;
			exponent += static_cast<int>(*curr - 0x30);
			curr++;
			read++;
			end_not_reached = (curr != s_end);
		}
		exponent *= (exp_sign == '+' ? 1 : -1);
		if (read == 0)
			goto fail;
	}

assemble:
	*result = (sign == '+' ? 1 : -1) * (exponent ? ldexp(mantissa * powf(5.0, exponent), exponent) : mantissa);
	return true;
fail:
	return false;
}

#undef IS_DIGIT

static inline float parseReal(const char **token, double default_value = 0.0) {
	(*token) += strspn((*token), " \t");
	const char *end = (*token) + strcspn((*token), " \t\r");
	double val = default_value;
	tryParseDouble((*token), end, &val);
	float f = static_cast<float>(val);
	(*token) = end;
	return f;
}

bool parseReal(const char **token, float *out) {
	(*token) += strspn((*token), " \t");
	const char *end = (*token) + strcspn((*token), " \t\r");
	double val;
	bool ret = tryParseDouble((*token), end, &val);
	if (ret) {
		float f = static_cast<float>(val);
		(*out) = f;
	}
	(*token) = end;
	return ret;
}

void parseReal2(float *x, float *y, const char **token, float default_x, float default_y) {
	*x = parseReal(token, default_x);
	*y = parseReal(token, default_y);
}

void parseReal3(float *x, float *y, float *z, const char **token, float default_x, float default_y, float default_z) {
	*x = parseReal(token, default_x);
	*y = parseReal(token, default_y);
	*z = parseReal(token, default_z);
}

void parseIVec3(const core::String &in, int32_t *out, const char *delimiters) {
	core::DynamicArray<core::String> tokens;
	tokens.reserve(3);
	splitString(in, tokens, delimiters);
	if (tokens.size() > 3) {
		tokens.resize(3);
	}
	for (size_t i = 0; i < tokens.size(); ++i) {
		out[(int)i] = tokens[i].toInt();
	}
}

void parseVec3(const core::String &in, float *out, const char *delimiters) {
	core::DynamicArray<core::String> tokens;
	tokens.reserve(3);
	splitString(in, tokens, delimiters);
	if (tokens.size() > 3) {
		tokens.resize(3);
	}
	for (size_t i = 0; i < tokens.size(); ++i) {
		out[(int)i] = tokens[i].toFloat();
	}
}

char *strncpyz(const char *input, size_t inputSize, char *target, size_t targetSize) {
	core_assert(targetSize > 0);
	while (--targetSize > 0 && inputSize > 0 && *input != '\0') {
		*target++ = *input++;
		--inputSize;
	}
	*target = '\0';
	return target;
}

core::String sanitizeDirPath(core::String str) {
	str.replaceAllChars('\\', '/');
	while (endsWith(str, "/")) {
		str.erase(str.size() - 1, 1);
	}
	return str.append("/");
}

core::String extractDir(const core::String &str) {
	const size_t pos = str.rfind("/");
	if (pos == core::String::npos) {
		return "";
	}
	return str.substr(0, pos + 1);
}

bool isSamePath(const core::String &a, const core::String &b) {
	return sanitizeDirPath(a) == sanitizeDirPath(b);
}

core::String cleanPath(const core::String &str) {
	core::String tmp;
	tmp.reserve(str.size() + 1);
	for (auto c : str) {
		if (c == ' ') {
			tmp += '_';
		}
		if (isalnum(c) || c == '.' || c == '_' || c == '-') {
			tmp += toLower(c);
		}
	}
	return tmp;
}

core::String stripExtension(const core::String &str) {
	const size_t pos = str.rfind(".");
	if (pos == core::String::npos) {
		return str;
	}
	return str.substr(0, pos);
}

core::String replaceExtension(const core::String &filename, const core::String &newExtension) {
	if (newExtension.first() == '.') {
		return replaceExtension(filename, newExtension.substr(1));
	}
	const size_t pos = filename.rfind(".");
	if (pos == core::String::npos) {
		return filename + "." + newExtension;
	}
	return filename.substr(0, pos + 1) + newExtension;
}

core::String extractExtension(const core::String &str) {
	const core::String filename = extractFilenameWithExtension(str);
	const size_t pos = filename.rfind(".");
	if (pos == core::String::npos) {
		return "";
	}
	return filename.substr(pos + 1);
}

core::String extractAllExtensions(const core::String &str) {
	const size_t pos = str.find(".");
	if (pos == core::String::npos) {
		return "";
	}
	return str.substr(pos + 1);
}

core::String extractFilenameWithExtension(const core::String &str) {
	const size_t pathPos = str.rfind('/');
	if (pathPos == core::String::npos) {
		return str;
	}
	return str.substr(pathPos + 1);
}

core::String addFilenamePrefix(const core::String &filename, const core::String &prefix) {
	const core::String &path = extractDir(filename);
	const core::String &file = extractFilenameWithExtension(filename);
	return core::string::path(path, prefix + file);
}

} // namespace string
} // namespace core
