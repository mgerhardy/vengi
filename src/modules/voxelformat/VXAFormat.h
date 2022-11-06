/**
 * @file
 */

#pragma once

#include "Format.h"

namespace voxelformat {

/**
 * @brief VoxEdit (Sandbox) (vxa)
 * Animation file that together with vxr and vxm files form the full asset
 * @sa VXMFormat
 * @sa VXRFormat
 *
 * @ingroup Formats
 */
class VXAFormat : public Format {
private:
	bool recursiveImportNodeSince3(const core::String &filename, io::SeekableReadStream& stream, SceneGraph& sceneGraph, SceneGraphNode& node, const core::String &animId, int version);
	bool recursiveImportNodeBefore3(const core::String &filename, io::SeekableReadStream& stream, SceneGraph& sceneGraph, SceneGraphNode& node, const core::String &animId, int version);
	bool saveRecursiveNode(const SceneGraph& sceneGraph, const SceneGraphNode& node, const core::String &filename, io::SeekableWriteStream& stream);
	bool loadGroups(const core::String &filename, io::SeekableReadStream& stream, SceneGraph& sceneGraph) override;
	bool saveGroups(const SceneGraph& sceneGraph, const core::String &filename, io::SeekableWriteStream& stream, ThumbnailCreator thumbnailCreator) override;
};

}
