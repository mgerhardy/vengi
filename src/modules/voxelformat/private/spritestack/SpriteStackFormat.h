/**
 * @file
 */

#pragma once

#include "io/Archive.h"
#include "io/Stream.h"
#include "voxelformat/Format.h"

namespace voxelformat {

/**
 * @brief SpriteStack (*.zip)
 *
 * @ingroup Formats
 */
class SpriteStackFormat : public PaletteFormat {
private:
	bool loadGroupsPalette(const core::String &filename, const io::ArchivePtr &archive,
						   scenegraph::SceneGraph &sceneGraph, palette::Palette &palette,
						   const LoadContext &ctx) override;

public:
	bool saveGroups(const scenegraph::SceneGraph &sceneGraph, const core::String &filename,
					const io::ArchivePtr &archive, const SaveContext &ctx) override {
		return false;
	}

	static const io::FormatDescription &format() {
		static io::FormatDescription f{"SpriteStack", {"zip"}, {}, 0u};
		return f;
	}
};

} // namespace voxelformat
