/**
 * @file
 */

#include "Archive.h"
#include "core/Algorithm.h"
#include "core/SharedPtr.h"
#include "core/StringUtil.h"
#include "io/Filesystem.h"
#include "io/FilesystemArchive.h"
#include "io/Stream.h"
#include "io/ZipArchive.h"

namespace io {

bool Archive::init(const core::String &path, io::SeekableReadStream *stream) {
	return true;
}

void Archive::shutdown() {
	_files.clear();
}

bool Archive::exists(const core::String &file) const {
	return core::find_if(_files.begin(), _files.end(), [&](const auto &e1) { return e1.fullPath == file; }) !=
		   _files.end();
}

SeekableWriteStream* Archive::writeStream(const core::String &filePath) {
	return nullptr;
}

bool isSupportedArchive(const core::String &filename) {
	const core::String &ext = core::string::extractExtension(filename);
	return ext == "zip" || ext == "pk3";
}

ArchivePtr openArchive(const io::FilesystemPtr &fs, const core::String &path, io::SeekableReadStream *stream) {
	const core::String &ext = core::string::extractExtension(path);
	if (ext == "zip" || ext == "pk3" || ext == "thing") {
		return openZipArchive(stream);
	}
	return openFilesystemArchive(fs, path);
}

} // namespace io
