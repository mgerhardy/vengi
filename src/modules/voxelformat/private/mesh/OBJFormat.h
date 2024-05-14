/**
 * @file
 */

#pragma once

#include "MeshFormat.h"
#include "io/Stream.h"

namespace tinyobj {
struct mesh_t;
struct attrib_t;
struct material_t;
} // namespace tinyobj

namespace voxelformat {
/**
 * @brief Wavefront Object
 *
 * https://en.wikipedia.org/wiki/Wavefront_.obj_file
 *
 * @ingroup Formats
 */
class OBJFormat : public MeshFormat {
private:
	bool writeMtlFile(io::SeekableWriteStream &stream, const core::String &mtlId, const core::String &mapKd) const;
protected:
	bool voxelizeGroups(const core::String &filename, const io::ArchivePtr &archive,
						scenegraph::SceneGraph &sceneGraph, const LoadContext &ctx) override;

public:
	bool saveMeshes(const core::Map<int, int> &, const scenegraph::SceneGraph &, const Meshes &meshes,
					const core::String &filename, const io::ArchivePtr &archive, const glm::vec3 &scale, bool quad,
					bool withColor, bool withTexCoords) override;
};
} // namespace voxelformat
