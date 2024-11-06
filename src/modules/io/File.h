/**
 * @file
 */

#pragma once

#include "core/String.h"
#include "core/SharedPtr.h"
#include "IOResource.h"
#include "io/Stream.h"

struct SDL_IOStream;

namespace io {

struct FormatDescription;
class Filesystem;

enum class FileMode {
	Read,		/**< reading from the virtual file system */
	Write,		/**< writing into the virtual file system */
	Append,		/**< appending to an existing file or create a new one */
	SysRead,	/**< reading from the given path - using virtual paths as fallback */
	SysWrite,	/**< writing into the given path */
	ReadNoHome	/**< reading from the virtual file system but skip user setting files in the home directories */
};

/**
 * @sa core::string::sanitizeDirPath()
 */
extern void normalizePath(core::String& str);

/**
 * @brief Wrapper for file based io.
 *
 * @see FileSystem
 */
class File : public IOResource {
	friend class FileStream;
	friend class Filesystem;
	friend class core::SharedPtr<io::File>;
protected:
	SDL_IOStream* _file;
	core::String _rawPath;
	FileMode _mode;
	mutable core::String _error;

	void error(CORE_FORMAT_STRING const char *msg, ...) const CORE_PRINTF_VARARG_FUNC(2);

public:
	File(const core::String& rawPath, FileMode mode = FileMode::Read);
	File(core::String&& rawPath, FileMode mode = FileMode::Read);
	virtual ~File();

	/**
	 * @brief Only needed after you have called close(). Otherwise the file is
	 * automatically opened in the given @c FileMode
	 * @return @c false if the file could not get opened, or it is still opened,
	 * @c true otherwise
	 */
	bool open(FileMode mode);
	void close();
	bool flush();
	int read(void *buf, size_t size, size_t maxnum);
	long tell() const;
	long seek(long offset, int seekType) const;

	bool isAnyOf(const io::FormatDescription* desc) const;

	/**
	 * @return The FileMode the file was opened with
	 */
	FileMode mode() const;

	bool exists() const;
	bool validHandle() const;
	/**
	 * @return -1 on error, otherwise the length of the file
	 */
	long length() const;
	/**
	 * @return The extension of the file - or en ampty string
	 * if no extension was found
	 */
	core::String extension() const;
	/**
	 * @return The path of the file, without the name - or an
	 * empty string if no path component was found
	 */
	core::String dir() const;
	/**
	 * @return Just the base file name component part - without
	 * path and extension
	 */
	core::String fileName() const;
	/**
	 * @return The full raw path of the file
	 */
	const core::String& name() const;

	SDL_IOStream* createRWops(FileMode mode) const;
	long write(io::ReadStream &stream) const;
	long write(const unsigned char *buf, size_t len) const;
	/**
	 * @brief Reads the content of the file into a buffer. The buffer is allocated inside this function.
	 * The returned memory is owned by the caller.
	 * @return The amount of read bytes. A value < 0 indicates an error. 0 means empty file.
	 */
	int read(void **buffer);
	int read(void *buffer, int n);
	/**
	 * @brief Load the file content as string
	 */
	core::String load();

	const core::String &lastError() const;
};

inline const core::String &File::lastError() const {
	return _error;
}

inline FileMode File::mode() const {
	return _mode;
}

typedef core::SharedPtr<File> FilePtr;

}
