/**
 * @file
 */

#include "core/Path.h"
#include "core/Common.h"
#include "core/String.h"
#include "core/StringUtil.h"
#include <SDL3/SDL_platform.h>

namespace core {

Path::Path(const core::String &path) : _path(path) {
	core::string::replaceAllChars(_path, '\\', '/');
}

Path::Path(core::String &&path) : _path(core::move(path)) {
	core::string::replaceAllChars(_path, '\\', '/');
}

char Path::separator() const {
#ifdef SDL_PLATFORM_WINDOWS
	return '\\';
#else
	return '/';
#endif
}

core::String Path::toNativePath() const {
#ifdef SDL_PLATFORM_WINDOWS
	core::String path = _path;
	core::string::replaceAllChars(path, '/', '\\');
	return path;
#else
	return _path;
#endif
}

char Path::driveLetter() const {
	if (_path.size() >= 2 && _path[1] == ':') {
		return core::string::toUpper(_path[0]);
	}
#ifdef SDL_PLATFORM_WINDOWS
	return 'C';
#else
	return '/';
#endif
}

Path Path::dirname() const {
	core::String base = _path;
	if (base.size() == 1 && base.last() == '/') {
		return core::Path(base);
	}
	while (base.last() == '/') {
		base.erase(base.size() - 1);
	}
	const size_t pos = base.find_last_of("/");
	if (pos == core::String::npos) {
		return Path(".");
	}
	return core::Path(base.substr(0, pos));
}

Path Path::basename() const {
	core::String base = _path;
	if (base.size() == 1 && base.last() == '/') {
		return core::Path(base);
	}
	while (base.last() == '/') {
		base.erase(base.size() - 1);
	}
	const size_t pos = base.find_last_of("/");
	if (pos != core::String::npos) {
		base = base.substr(pos + 1);
	}
	return core::Path(base);
}

core::String Path::extension() const {
	const size_t pos = _path.find_last_of(".");
	if (pos == core::String::npos) {
		return core::String();
	}
	if (_path.find_last_of("/") > pos) {
		return core::String();
	}
	return _path.substr(pos + 1);
}

Path Path::removeExtension() const {
	const size_t pos = _path.find_last_of(".");
	if (pos == core::String::npos) {
		return *this;
	}
	if (_path.find_last_of("/") > pos) {
		return *this;
	}
	return core::Path(_path.substr(0, pos));
}

Path Path::replaceExtension(const core::String &newExtension) const {
	const size_t pos = _path.find_last_of(".");
	if (pos == core::String::npos || _path.find_last_of("/") > pos) {
		return core::Path(_path + "." + newExtension);
	}
	return core::Path(_path.substr(0, pos) + "." + newExtension);
}

bool Path::isRelativePath() const {
	const size_t size = _path.size();
#ifdef SDL_PLATFORM_WINDOWS
	if (size < 2) {
		return true;
	}
	// TODO: hm... not cool and most likely not enough
	return _path[1] != ':';
#else
	if (size == 0) {
		return true;
	}
	return _path[0] != '/';
#endif
}

bool Path::isAbsolutePath() const {
	if (_path.size() >= 3U && core::string::isAlpha(_path[0]) && _path[1] == ':' &&
		(_path[2] == '\\' || _path[2] == '/')) {
		return true;
	}
	return _path.size() > 1U && (_path[0] == '/' || _path[0] == '\\');
}

bool Path::isRootPath() const {
	if (_path.size() == 3U && core::string::isAlpha(_path[0]) && _path[1] == ':' &&
		(_path[2] == '\\' || _path[2] == '/')) {
		return true;
	}
	return _path.size() == 1U && (_path[0] == '/' || _path[0] == '\\');
}

core::DynamicArray<core::String> Path::components() const {
	core::DynamicArray<core::String> c;
	core::string::splitString(_path, c, "/");
	return c;
}

Path Path::append(const core::String &component) const {
	return Path(core::string::path(_path, component));
}

Path Path::append(const core::Path &component) const {
	return Path(core::string::path(_path, component.str()));
}

Path &Path::operator+=(const core::String &other) {
	_path = core::string::path(_path, other);
	return *this;
}

Path &Path::operator+=(const Path &other) {
	_path = core::string::path(_path, other._path);
	return *this;
}

bool Path::operator==(const core::String &other) const {
	core::String otherCleaned = other;
	core::string::replaceAllChars(otherCleaned, '\\', '/');
	if (otherCleaned.last() == '/') {
		otherCleaned.erase(otherCleaned.size() - 1);
	}
	core::String ownCleaned = _path;
	if (ownCleaned.last() == '/') {
		ownCleaned.erase(ownCleaned.size() - 1);
	}
	return ownCleaned == otherCleaned;
}

bool Path::operator!=(const core::String &other) const {
	return !(*this == other);
}

bool Path::operator==(const Path &other) const {
	return *this == other._path;
}

bool Path::operator!=(const Path &other) const {
	return *this != other._path;
}

Path operator+(const Path &lhs, const core::Path &rhs) {
	return Path(core::string::path(lhs.str(), rhs.str()));
}

Path operator+(const Path &lhs, const core::String &rhs) {
	return Path(core::string::path(lhs.str(), rhs));
}

} // namespace core
