/**
 * @file
 */

#include "voxelformat/private/mesh/MeshFormat.h"
#include "core/Color.h"
#include "core/tests/TestColorHelper.h"
#include "io/Archive.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/SceneGraphNode.h"
#include "video/ShapeBuilder.h"
#include "voxel/MaterialColor.h"
#include "voxel/RawVolume.h"
#include "voxelformat/VolumeFormat.h"
#include "voxelformat/tests/AbstractFormatTest.h"

namespace voxelformat {

class MeshFormatTest : public AbstractFormatTest {};

TEST_F(MeshFormatTest, testSubdivide) {
	MeshFormat::TriCollection tinyTris;
	voxelformat::TexturedTri tri;
	tri.vertices[0] = glm::vec3(-8.77272797, -11.43335, -0.154544264);
	tri.vertices[1] = glm::vec3(-8.77272701, 11.1000004, -0.154543981);
	tri.vertices[2] = glm::vec3(8.77272701, 11.1000004, -0.154543981);
	MeshFormat::subdivideTri(tri, tinyTris);
	EXPECT_EQ(1024u, tinyTris.size());
}

TEST_F(MeshFormatTest, testColorAt) {
	const image::ImagePtr &texture = image::loadImage("palette-nippon.png");
	ASSERT_TRUE(texture);
	ASSERT_EQ(256, texture->width());
	ASSERT_EQ(1, texture->height());

	palette::Palette pal;
	pal.nippon();

	voxelformat::TexturedTri tri;
	tri.texture = texture;
	for (int i = 0; i < 256; ++i) {
		tri.uv[0] = glm::vec2((float)i / 256.0f, 0.0f);
		tri.uv[1] = glm::vec2((float)i / 256.0f, 1.0f);
		tri.uv[2] = glm::vec2((float)(i + 1) / 256.0f, 1.0f);
		const core::RGBA color = tri.colorAt(tri.centerUV());
		ASSERT_EQ(pal.color(i), color) << "i: " << i << " " << core::Color::print(pal.color(i)) << " vs "
									   << core::Color::print(color);
	}
}

TEST_F(MeshFormatTest, testLookupTexture) {
	EXPECT_NE(MeshFormat::lookupTexture("glTF/cube/chr_knight.gox", "glTF/cube/Cube_BaseColor.png"), "");
	EXPECT_NE(MeshFormat::lookupTexture("glTF/cube/chr_knight.gox", "Cube_BaseColor.png"), "");
	EXPECT_NE(MeshFormat::lookupTexture("glTF/chr_knight.gox", "./cube/Cube_BaseColor.png"), "");
	EXPECT_NE(MeshFormat::lookupTexture("glTF/chr_knight.gox", "cube/Cube_BaseColor.png"), "");
	// TODO: EXPECT_NE(MeshFormat::lookupTexture("glTF/foo/bar/chr_knight.gox", "../../cube/Cube_BaseColor.png"), "");
}

TEST_F(MeshFormatTest, testCalculateAABB) {
	MeshFormat::TriCollection tris;
	voxelformat::TexturedTri tri;

	{
		tri.vertices[0] = glm::vec3(0, 0, 0);
		tri.vertices[1] = glm::vec3(10, 0, 0);
		tri.vertices[2] = glm::vec3(10, 0, 10);
		tris.push_back(tri);
	}

	{
		tri.vertices[0] = glm::vec3(0, 0, 0);
		tri.vertices[1] = glm::vec3(-10, 0, 0);
		tri.vertices[2] = glm::vec3(-10, 0, -10);
		tris.push_back(tri);
	}

	glm::vec3 mins, maxs;
	ASSERT_TRUE(MeshFormat::calculateAABB(tris, mins, maxs));
	EXPECT_FLOAT_EQ(mins.x, -10.0f);
	EXPECT_FLOAT_EQ(mins.y, 0.0f);
	EXPECT_FLOAT_EQ(mins.z, -10.0f);
	EXPECT_FLOAT_EQ(maxs.x, 10.0f);
	EXPECT_FLOAT_EQ(maxs.y, 0.0f);
	EXPECT_FLOAT_EQ(maxs.z, 10.0f);
}

TEST_F(MeshFormatTest, testAreAllTrisAxisAligned) {
	MeshFormat::TriCollection tris;
	voxelformat::TexturedTri tri;

	{
		tri.vertices[0] = glm::vec3(0, 0, 0);
		tri.vertices[1] = glm::vec3(10, 0, 0);
		tri.vertices[2] = glm::vec3(10, 0, 10);
		tris.push_back(tri);
	}

	{
		tri.vertices[0] = glm::vec3(0, 0, 0);
		tri.vertices[1] = glm::vec3(-10, 0, 0);
		tri.vertices[2] = glm::vec3(-10, 0, -10);
		tris.push_back(tri);
	}

	EXPECT_TRUE(MeshFormat::isVoxelMesh(tris));

	{
		tri.vertices[0] = glm::vec3(0, 0, 0);
		tri.vertices[1] = glm::vec3(-10, 1, 0);
		tri.vertices[2] = glm::vec3(-10, 0, -10);
		tris.push_back(tri);
	}

	EXPECT_FALSE(MeshFormat::isVoxelMesh(tris));
}

TEST_F(MeshFormatTest, testVoxelizeColor) {
	class TestMesh : public MeshFormat {
	public:
		bool saveMeshes(const core::Map<int, int> &, const scenegraph::SceneGraph &, const Meshes &,
						const core::String &, const io::ArchivePtr &, const glm::vec3 &, bool, bool, bool) override {
			return false;
		}
		void voxelize(scenegraph::SceneGraph &sceneGraph, const MeshFormat::TriCollection &tris) {
			voxelizeNode("test", sceneGraph, tris);
			sceneGraph.updateTransforms();
		}
	};

	TestMesh mesh;
	MeshFormat::TriCollection tris;

	video::ShapeBuilder b;
	scenegraph::SceneGraph sceneGraph;

	voxel::getPalette().nippon();
	const core::RGBA nipponRed = voxel::getPalette().color(37);
	const core::RGBA nipponBlue = voxel::getPalette().color(202);
	const float size = 10.0f;
	b.setPosition({size, 0.0f, size});
	b.setColor(core::Color::fromRGBA(nipponRed));
	b.pyramid({size, size, size});

	const video::ShapeBuilder::Indices &indices = b.getIndices();
	const video::ShapeBuilder::Vertices &vertices = b.getVertices();

	// color of the tip is green
	video::ShapeBuilder::Colors colors = b.getColors();
	const core::RGBA nipponGreen = voxel::getPalette().color(145);
	colors[0] = core::Color::fromRGBA(nipponGreen);
	colors[1] = core::Color::fromRGBA(nipponBlue);

	const int n = (int)indices.size();
	for (int i = 0; i < n; i += 3) {
		voxelformat::TexturedTri tri;
		for (int j = 0; j < 3; ++j) {
			tri.vertices[j] = vertices[indices[i + j]];
			tri.color[j] = core::Color::getRGBA(colors[indices[i + j]]);
		}
		tris.push_back(tri);
	}
	mesh.voxelize(sceneGraph, tris);
	scenegraph::SceneGraphNode *node = sceneGraph.findNodeByName("test");
	ASSERT_NE(nullptr, node);
	voxel::getPalette() = node->palette();
	const voxel::RawVolume *v = node->volume();
	EXPECT_COLOR_NEAR(nipponRed, node->palette().color(v->voxel(0, 0, 0).getColor()), 0.01f);
	EXPECT_COLOR_NEAR(nipponRed, node->palette().color(v->voxel(size * 2 - 1, 0, size * 2 - 1).getColor()), 0.01f);
	EXPECT_COLOR_NEAR(nipponBlue, node->palette().color(v->voxel(0, 0, size * 2 - 1).getColor()), 0.06f);
	EXPECT_COLOR_NEAR(nipponRed, node->palette().color(v->voxel(size * 2 - 1, 0, 0).getColor()), 0.06f);
	EXPECT_COLOR_NEAR(nipponGreen, node->palette().color(v->voxel(size - 1, size - 1, size - 1).getColor()), 0.01f);
}

} // namespace voxelformat
