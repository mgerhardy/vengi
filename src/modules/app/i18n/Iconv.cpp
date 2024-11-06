/**
 * @file
 */

// tinygettext - A gettext replacement that works directly on .po files
// Copyright (c) 2009 Ingo Ruhnke <grumbel@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "Iconv.h"
#include <SDL3/SDL_stdinc.h>

namespace app {

IConv::IConv() : _toCharset(), _fromCharset(), _cd(nullptr) {
}

IConv::IConv(const core::String &from_charset_, const core::String &to_charset_)
	: _toCharset(), _fromCharset(), _cd(nullptr) {
	setCharsets(from_charset_, to_charset_);
}

IConv::~IConv() {
	if (_cd)
		SDL_iconv_close((SDL_iconv_t)_cd);
}

bool IConv::setCharsets(const core::String &from_charset_, const core::String &to_charset_) {
	if (_cd)
		SDL_iconv_close((SDL_iconv_t)_cd);

	_fromCharset = from_charset_.toUpper();
	_toCharset = to_charset_.toUpper();

	if (_toCharset == _fromCharset) {
		_cd = nullptr;
	} else {
		_cd = SDL_iconv_open(_toCharset.c_str(), _fromCharset.c_str());
		if (_cd == (SDL_iconv_t)-1) {
			return false;
		}
	}
	return true;
}

/// Convert a string from encoding to another.
core::String IConv::convert(const core::String &text) {
	if (!_cd) {
		return text;
	}
	size_t inbytesleft = text.size();
	size_t outbytesleft = 4 * inbytesleft; // Worst case scenario: ASCII -> UTF-32?

	// We try to avoid to much copying around, so we write directly into
	// a core::String
	const char *inbuf = &text[0];
	core::String result(outbytesleft, 'X');
	char *outbuf = &result[0];

	// Try to convert the text.
	size_t ret = SDL_iconv((SDL_iconv_t)_cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	if (ret == (size_t)-1) {
		return text;
	}

	return result.substr(0, 4 * text.size() - outbytesleft);
}

} // namespace app
