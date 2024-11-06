/**
 * @file
 */

#include "File.h"
#include "core/Log.h"
#include "core/StringUtil.h"
#include "io/FormatDescription.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_iostream.h>
#ifdef __EMSCRIPTEN__
#include "system/emscripten_browser_file.h"
#endif

namespace io {

void normalizePath(core::String& str) {
	core::string::replaceAllChars(str, '\\', '/');
#ifndef SDL_PLATFORM_WINDOWS
	if (str.size() >= 3 && str[0] != '\0' && core::string::isAlpha(str[0]) && str[1] == ':' && (str[2] == '\\' || str[2] == '/')) {
		str.erase(0, 2);
	}
#endif
}

File::File(const core::String& rawPath, FileMode mode) :
		IOResource(), _rawPath(rawPath), _mode(mode) {
	normalizePath(_rawPath);
	_file = createRWops(mode);
}

File::File(core::String&& rawPath, FileMode mode) :
		IOResource(), _rawPath(core::move(rawPath)), _mode(mode) {
	normalizePath(_rawPath);
	_file = createRWops(mode);
}

File::~File() {
	close();
}

bool File::validHandle() const {
	return _file != nullptr;
}

bool File::isAnyOf(const io::FormatDescription* desc) const {
	const core::String& ext = extension();
	while (desc->valid()) {
		if (desc->matchesExtension(ext)) {
			// TODO: isA check
			return true;
		}
		++desc;
	}
	return false;
}

bool File::exists() const {
	if (_mode == FileMode::Read || _mode == FileMode::SysRead) {
		return _file != nullptr;
	}

	// try to open in read mode
	SDL_IOStream* ops = createRWops(FileMode::SysRead);
	if (ops != nullptr) {
		SDL_CloseIO(ops);
		return true;
	}
	return false;
}

const core::String& File::name() const {
	return _rawPath;
}

core::String File::load() {
	char *buf = nullptr;
	const int len = read((void **) &buf);
	if (buf == nullptr || len <= 0) {
		delete[] buf;
		return "";
	}
	core::String f(buf, len);
	delete[] buf;
	return f;
}

void File::error(const char *msg, ...) const {
	va_list ap;
	constexpr size_t bufSize = 1024;
	char text[bufSize];

	va_start(ap, msg);
	SDL_vsnprintf(text, bufSize, msg, ap);
	text[sizeof(text) - 1] = '\0';
	va_end(ap);

	_error = text;
	Log::debug("path: '%s' (mode: %i): %s", _rawPath.c_str(), (int)_mode, _error.c_str());
}

SDL_IOStream* File::createRWops(FileMode mode) const {
	if (_rawPath.empty()) {
		error("Can't open file - no path given");
		return nullptr;
	}
	const char *fmode = "rb";
	if (mode == FileMode::Write || mode == FileMode::SysWrite) {
		fmode = "wb";
	} else if (mode == FileMode::Append) {
		fmode = "ab";
	}
	SDL_IOStream *rwops = SDL_IOFromFile(_rawPath.c_str(), fmode);
	if (rwops == nullptr) {
		error("%s", SDL_GetError());
	}
	return rwops;
}

long File::write(io::ReadStream &stream) const {
	if (_file == nullptr) {
		error("Invalid file handle - can't write to file");
		return -1L;
	}
	if (_mode != FileMode::Write && _mode != FileMode::SysWrite) {
		error("Invalid file mode given - can't write to file");
		return -1L;
	}
	char buf[4096 * 10];
	long l = 0;
	while (!stream.eos()) {
		const int len = stream.read(buf, sizeof(buf));
		if (len == -1) {
			return -1L;
		}
		const size_t written = SDL_WriteIO(_file, buf, len);
		if (written == 0) {
			error("Error writing file - failed to write buffer of length %i", (int)len);
			return -1L;
		}
		l += len;
	}

	Log::debug("%i bytes were written into path %s", (int)l, _rawPath.c_str());
	return l;
}

long File::write(const unsigned char *buf, size_t len) const {
	if (_file == nullptr) {
		Log::debug("Invalid file handle - can write buffer of length %i (path: %s)",
				(int)len, _rawPath.c_str());
		return -1L;
	}
	if (_mode != FileMode::Write && _mode != FileMode::SysWrite) {
		Log::debug("Invalid file mode given - can write buffer of length %i (path: %s)",
				(int)len, _rawPath.c_str());
		return -1L;
	}

	int remaining = (int)len;
	while (remaining > 0) {
		const size_t written = SDL_WriteIO(_file, buf, remaining);
		if (written == 0) {
			Log::debug("Error writing file - can write buffer of length %i (remaining: %i) (path: %s)",
					(int)len, remaining, _rawPath.c_str());
			return -1L;
		}

		remaining -= (int)written;
		buf += written;
	}

	Log::debug("%i bytes were written into path %s", (int)len, _rawPath.c_str());
	return (long)len;
}

core::String File::dir() const {
	return core::string::extractDir(name());
}

core::String File::fileName() const {
	return core::string::extractFilename(name());
}

core::String File::extension() const {
	const char *ext = SDL_strrchr(name().c_str(), '.');
	if (ext == nullptr) {
		return "";
	}
	++ext;
	return core::String(ext);
}

long File::length() const {
	if (!exists()) {
		return -1l;
	}

	const long pos = tell();
	seek(0, SDL_IO_SEEK_END);
	const long end = tell();
	seek(pos, SDL_IO_SEEK_SET);
	return end;
}

int File::read(void **buffer) {
	*buffer = nullptr;
	const long len = length();
	if (len <= 0) {
		return len;
	}
	*buffer = new uint8_t[len];
	return read(*buffer, len);
}

int File::read(void *buffer, int n) {
	const size_t blockSize = 0x10000;
	uint8_t *buf;
	size_t remaining, len;

	len = remaining = n;
	buf = (uint8_t *) buffer;

	seek(0, SDL_IO_SEEK_SET);

	while (remaining != 0u) {
		size_t block = remaining;
		if (block > blockSize) {
			block = blockSize;
		}
		const int readAmount = read(buf, 1, block);

		/* end of file reached */
		if (readAmount == 0) {
			return int(len - remaining + readAmount);
		} else if (readAmount == -1) {
			Log::debug("Read error while reading %s", _rawPath.c_str());
			return -1;
		}

		/* do some progress bar thing here... */
		remaining -= readAmount;
		buf += readAmount;
	}
	Log::debug("Read %i bytes from %s", (int)len, _rawPath.c_str());
	return (int)len;
}

int File::read(void *buf, size_t size, size_t maxnum) {
	if (_mode != FileMode::Read && _mode != FileMode::SysRead) {
		_state = IOSTATE_FAILED;
		Log::debug("File %s is not opened in read mode", _rawPath.c_str());
		return -1;
	}
	const int n = (int)SDL_ReadIO(_file, buf, size * maxnum) / size;
	if (n == 0) {
		_state = IOSTATE_LOADED;
		Log::trace("File %s: read successful", _rawPath.c_str());
	} else if (n == -1) {
		_state = IOSTATE_FAILED;
		Log::trace("File %s: read failed", _rawPath.c_str());
	} else {
		Log::trace("File %s: read %i bytes", _rawPath.c_str(), n);
	}

	return n;
}

bool File::flush() {
	if (_file != nullptr) {
		SDL_CloseIO(_file);
		if (_mode == FileMode::Write || _mode == FileMode::SysWrite) {
			_mode = FileMode::Append;
		}
		_file = createRWops(_mode);
		return _file != nullptr;
	}
	return false;
}

void File::close() {
	if (_file != nullptr) {
		SDL_CloseIO(_file);
		_file = nullptr;
#ifdef __EMSCRIPTEN__
		if (_mode == FileMode::SysWrite) {
			_file = createRWops(FileMode::SysRead);
			_mode = FileMode::SysRead;
			if (_file == nullptr) {
				Log::error("Failed to download file %s", _rawPath.c_str());
			} else {
				uint8_t *buf = nullptr;
				const int len = read((void **)&buf);
				if (len > 0) {
					emscripten_browser_file::download(_rawPath.c_str(), "application/octet-stream", buf, (size_t)len);
				}
				delete[] buf;
				SDL_CloseIO(_file);
				_file = nullptr;
			}
			_mode = FileMode::SysWrite;
		}
#endif
	}
}

bool File::open(FileMode mode) {
	if (_file != nullptr) {
		Log::debug("File %s is already open", _rawPath.c_str());
		return false;
	}
	_mode = mode;
	_file = createRWops(mode);
	return _file != nullptr;
}

long File::tell() const {
	if (_file != nullptr) {
		return SDL_TellIO(_file);
	}
	return -1L;
}

long File::seek(long offset, int seekType) const {
	if (_file != nullptr) {
		return SDL_SeekIO(_file, offset, (SDL_IOWhence)seekType);
	}
	return -1L;
}

}
