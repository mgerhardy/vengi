/**
 * @file
 */

#include "SpriteStackFormat.h"
#include "core/Log.h"
#include "core/ScopedPtr.h"
#include "io/ZipArchive.h"
#include "scenegraph/SceneGraphNode.h"
#include <glm/common.hpp>

namespace voxelformat {

bool SpriteStackFormat::loadGroupsPalette(const core::String &filename, const io::ArchivePtr &archive,
										  scenegraph::SceneGraph &sceneGraph, palette::Palette &palette,
										  const LoadContext &ctx) {
	core::ScopedPtr<io::SeekableReadStream> stream(archive->readStream(filename));
	if (!stream) {
		Log::error("Could not load file %s", filename.c_str());
		return false;
	}
	const io::ArchivePtr &zipArchive = io::openZipArchive(stream);
	if (!zipArchive) {
		Log::error("Failed to open zip archive");
		return false;
	}

	return false;
}

} // namespace voxelformat
