/**
 * @file
 */

#include "core/ArrayLength.h"
#include "core/collection/DynamicArray.h"
#include "io/FilesystemEntry.h"
#include <SDL3/SDL_platform.h>

#if defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS) || defined(__EMSCRIPTEN__)
#include "core/Log.h"
#include "core/String.h"
#include "core/StringUtil.h"
#include "core/Tokenizer.h"
#include "io/Filesystem.h"
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#ifdef SDL_PLATFORM_MACOS
#include <libproc.h>
#include <sysdir.h>
#endif

namespace io {
namespace priv {

static const char *getHome() {
	const int uid = (int)getuid();
	if (uid != 0) {
		return SDL_getenv("HOME");
	}
	return nullptr;
}

/**
 * @brief Replace the shell variable for the home dir
 */
static inline core::String replaceHome(const core::String &in) {
	const char *envHome = getHome();
	if (envHome == nullptr) {
		return in;
	}
	core::String out = core::string::replaceAll(in, "$HOME", envHome);
	out = core::string::replaceAll(out, "~", envHome);
	return core::string::replaceAll(out, "${HOME}", envHome);
}

#ifdef SDL_PLATFORM_MACOS

// needs at least OSX 10.12
static core::String appleDir(sysdir_search_path_directory_t dir) {
	char path[PATH_MAX];
	sysdir_search_path_enumeration_state state = sysdir_start_search_path_enumeration(dir, SYSDIR_DOMAIN_MASK_USER);
	while ((state = sysdir_get_next_search_path_enumeration(state, path))) {
		// indicates a user dir
		if (path[0] == '~') {
			return replaceHome(path);
		}
	}
	return "";
}

#else // SDL_PLATFORM_MACOS

static core::String load(const core::String &file) {
	FILE *fp = fopen(file.c_str(), "r");
	if (fp == nullptr) {
		Log::debug("Could not open file %s", file.c_str());
		return "";
	}

	if (fseek(fp, 0L, SEEK_END) != 0) {
		Log::debug("Error: fseek failed");
		fclose(fp);
		return "";
	}

	long int bufsize = ftell(fp);
	if (bufsize == -1) {
		Log::debug("Error: ftell failed");
		fclose(fp);
		return "";
	}

	if (fseek(fp, 0L, SEEK_SET) != 0) {
		Log::debug("Error: fseek failed");
		fclose(fp);
		return "";
	}

	char *source = (char *)core_malloc(bufsize + 1);
	size_t newLen = fread(source, 1, bufsize, fp);
	if (ferror(fp) != 0) {
		perror("Error: failed to read shader file. ");
		fclose(fp);
		return "";
	}
	fclose(fp);

	source[newLen++] = '\0';
	core::String str(source);
	core_free(source);
	return str;
}

#endif

} // namespace priv

bool initState(io::FilesystemState &state) {
#ifndef __EMSCRIPTEN__
	const char *envHome = priv::getHome();
	if (envHome == nullptr) {
		Log::debug("HOME env var not found");
		return false;
	}
#if defined SDL_PLATFORM_MACOS
	state._directories[FilesystemDirectories::FS_Dir_Download] = priv::appleDir(SYSDIR_DIRECTORY_DOWNLOADS);
	state._directories[FilesystemDirectories::FS_Dir_Documents] = priv::appleDir(SYSDIR_DIRECTORY_DOCUMENT);
	state._directories[FilesystemDirectories::FS_Dir_Pictures] = priv::appleDir(SYSDIR_DIRECTORY_PICTURES);
	state._directories[FilesystemDirectories::FS_Dir_Desktop] = priv::appleDir(SYSDIR_DIRECTORY_DESKTOP);
	state._directories[FilesystemDirectories::FS_Dir_Public] = priv::appleDir(SYSDIR_DIRECTORY_SHARED_PUBLIC);
#else
	core::String xdgDir = core::string::path(envHome, ".config", "user-dirs.dirs");
	if (access(xdgDir.c_str(), F_OK) != 0) {
		Log::debug("Can't read xdg user dirs: %s doesn't exists", xdgDir.c_str());
		const char *xdgConfigDirs = SDL_getenv("XDG_CONFIG_DIRS");
		if (xdgConfigDirs == nullptr) {
			xdgConfigDirs = "/etc/xdg";
		}
		xdgDir = core::string::path(xdgConfigDirs, "user-dirs.defaults");
		if (access(xdgDir.c_str(), F_OK) != 0) {
			Log::debug("Can't read xdg dirs: %s doesn't exists", xdgDir.c_str());
			return false;
		}
	}
	const core::String &xdgDirsContent = priv::load(xdgDir);
	if (xdgDirsContent.empty()) {
		Log::debug("Could not read %s", xdgDir.c_str());
		return false;
	}
	core::TokenizerConfig cfg;
	core::Tokenizer tok(cfg, xdgDirsContent, "=");
	while (tok.hasNext()) {
		const core::String var = tok.next();
		if (!tok.hasNext()) {
			return false;
		}
		// https://www.freedesktop.org/wiki/Software/xdg-user-dirs/
		const core::String value = tok.next();
		if (var.contains("DOWNLOAD")) {
			state._directories[FilesystemDirectories::FS_Dir_Download] = priv::replaceHome(value);
		} else if (var.contains("DOCUMENTS")) {
			state._directories[FilesystemDirectories::FS_Dir_Documents] = priv::replaceHome(value);
		} else if (var.contains("DESKTOP")) {
			state._directories[FilesystemDirectories::FS_Dir_Desktop] = priv::replaceHome(value);
		} else if (var.contains("PICTURES")) {
			state._directories[FilesystemDirectories::FS_Dir_Pictures] = priv::replaceHome(value);
		} else if (var.contains("PUBLICSHARE")) {
			state._directories[FilesystemDirectories::FS_Dir_Public] = priv::replaceHome(value);
		}
	}
#endif // !SDL_PLATFORM_MACOS

	for (int n = 0; n < FilesystemDirectories::FS_Dir_Max; ++n) {
		if (state._directories[n].empty()) {
			continue;
		}
		state._directories[n] = core::string::sanitizeDirPath(state._directories[n]);
		if (core::string::isAbsolutePath(state._directories[n])) {
			continue;
		}
		state._directories[n] = core::string::path(envHome, state._directories[n]);
	}

	state._thisPc.push_back({"Root directory", "/"});
	state._thisPc.push_back({"Home", envHome});
#endif // !__EMSCRIPTEN__
	return true;
}

bool fs_mkdir(const char *path) {
	const int ret = mkdir(path, 0740);
	if (ret == 0) {
		return true;
	}
	if (errno == EEXIST) {
		return true;
	}
	Log::error("Failed to mkdir %s: %s", path, strerror(errno));
	return false;
}

bool fs_rmdir(const char *path) {
	const int ret = rmdir(path);
	if (ret != 0) {
		Log::error("Failed to rmdir %s: %s", path, strerror(errno));
	}
	return ret == 0;
}

bool fs_unlink(const char *path) {
	const int ret = unlink(path);
	if (ret != 0) {
		Log::error("Failed to unlink %s: %s", path, strerror(errno));
	}
	return ret == 0;
}

bool fs_exists(const char *path) {
	const int ret = access(path, F_OK);
	if (ret != 0) {
		Log::trace("Failed to access %s: %s", path, strerror(errno));
	}
	return ret == 0;
}

bool fs_chdir(const char *path) {
	const int ret = chdir(path);
	if (ret != 0) {
		Log::error("Failed to chdir to %s: %s", path, strerror(errno));
	}
	return ret == 0;
}

core::String fs_cwd() {
	char buf[4096];
	const char *p = getcwd(buf, lengthof(buf));
	if (p == nullptr) {
		Log::error("Failed to get current working dir: %s", strerror(errno));
	}
	return p;
}

core::String fs_realpath(const char *path) {
	if (path[0] == '\0') {
		// unified with _fullpath on windows
		return fs_cwd();
	}
	char buf[PATH_MAX];
	const char *rp = realpath(path, buf);
	if (rp == nullptr) {
		return "";
	}
	return rp;
}

bool fs_stat(const char *path, FilesystemEntry &entry) {
	struct stat s;
	const int ret = stat(path, &s);
	if (ret != 0) {
		Log::debug("Failed to stat %s: %s", path, strerror(errno));
		return false;
	}

	if (entry.type == FilesystemEntry::Type::unknown) {
		entry.type = (s.st_mode & S_IFDIR) ? FilesystemEntry::Type::dir : FilesystemEntry::Type::file;
	}
	entry.mtime = (uint64_t)s.st_mtime * 1000;
	entry.size = s.st_size;

	return true;
}

core::String fs_readlink(const char *path) {
	char buf[4096];
	ssize_t len = readlink(path, buf, lengthof(buf));
	if (len == -1) {
		return "";
	}

	buf[len] = '\0';
	return buf;
}

bool fs_writeable(const char *path) {
	return access(path, W_OK) == 0;
}

static int fs_scandir_filter(const struct dirent *dent) {
	return strcmp(dent->d_name, ".") != 0 && strcmp(dent->d_name, "..") != 0;
}

static int fs_scandir_sort(const struct dirent **a, const struct dirent **b) {
	return strcoll((*a)->d_name, (*b)->d_name);
}

core::DynamicArray<FilesystemEntry> fs_scandir(const char *path) {
	struct dirent **files = nullptr;
	const int n = scandir(path, &files, fs_scandir_filter, fs_scandir_sort);
	core::DynamicArray<FilesystemEntry> entries;
	entries.reserve(n);
	for (int i = 0; i < n; ++i) {
		const struct dirent *ent = files[i];
		FilesystemEntry entry;
		entry.name = ent->d_name;
		switch (ent->d_type) {
		case DT_DIR:
			entry.type = FilesystemEntry::Type::dir;
			break;
		case DT_REG:
			entry.type = FilesystemEntry::Type::file;
			break;
		case DT_LNK:
			entry.type = FilesystemEntry::Type::link;
			break;
		default:
			entry.type = FilesystemEntry::Type::unknown;
			break;
		}
		entries.push_back(entry);
	}
	for (int i = 0; i < n; i++) {
		free(files[i]);
	}
	free(files);
	return entries;
}

bool fs_hidden(const char *path) {
	if (path == nullptr) {
		return false;
	}
	return path[0] == '.';
}

} // namespace io

#endif
