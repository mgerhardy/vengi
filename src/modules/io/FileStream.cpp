/**
 * @file
 */

#include "FileStream.h"
#include "core/Log.h"
#include "io/File.h"
#include <SDL3/SDL_iostream.h>
#include <stdio.h>
#include <stdarg.h>

namespace io {

FileStream::FileStream(const FilePtr &file) : _file(file) {
	if (_file) {
		_rwops = _file->_file;
		if (_rwops) {
			_size = SDL_GetIOSize(_rwops);
		}
	}
}

FileStream::~FileStream() {
}

bool FileStream::valid() const {
	return _file && _file->validHandle();
}

bool FileStream::flush() {
	if (_rwops == nullptr) {
		return false;
	}
	return _file->flush();
}

int FileStream::write(const void *buf, size_t size) {
	if (_rwops == nullptr) {
		Log::debug("No file handle");
		return -1;
	}
	if (size == 0) {
		return 0;
	}
	const int64_t written = (int64_t)SDL_WriteIO(_rwops, buf, size);
	if (written != (int64_t)size) {
		Log::error("File write error: %s (%i vs %i)", SDL_GetError(), (int)written, (int)size);
		return -1;
	}
	_pos = SDL_TellIO(_rwops);
	_size = core_max(_size, _pos);
	return (int)written;
}

int FileStream::read(void *dataPtr, size_t dataSize) {
	if (_rwops == nullptr) {
		return -1;
	}
	uint8_t *b = (uint8_t*)dataPtr;
	size_t completeBytesRead = 0;
	while (completeBytesRead < dataSize) {
		size_t bytesRead = SDL_ReadIO(_rwops, b, (dataSize - completeBytesRead));
		b += bytesRead;
		completeBytesRead += bytesRead;
		if (bytesRead == 0) {
			break;
		}
	}
	_pos = SDL_TellIO(_rwops);
	if (completeBytesRead == 0) {
		return -1;
	}
	return (int)completeBytesRead;
}

int64_t FileStream::seek(int64_t position, int whence) {
	if (_rwops == nullptr) {
		return -1;
	}
	int64_t p = SDL_SeekIO(_rwops, position, (SDL_IOWhence)whence);
	_pos = SDL_TellIO(_rwops);
	if (p == -1) {
		return -1;
	}
	return _pos;
}

} // namespace io
