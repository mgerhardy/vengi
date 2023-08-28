/**
 * @file
 */

#include "voxedit-util/modifier/brush/ShapeBrush.h"
#include "app/tests/AbstractTest.h"
#include "scenegraph/SceneGraph.h"
#include "voxedit-util/modifier/ModifierType.h"
#include "voxedit-util/modifier/ModifierVolumeWrapper.h"
#include "voxel/Voxel.h"

namespace voxedit {

class ShapeBrushTest : public app::AbstractTest {
protected:
	void prepare(ShapeBrush &brush, BrushContext &brushContext, const glm::ivec3 &mins, const glm::ivec3 &maxs) {
		brushContext.cursorVoxel = voxel::createVoxel(voxel::VoxelType::Generic, 1);
		brushContext.cursorPosition = mins;
		brushContext.cursorFace = voxel::FaceNames::PositiveX;
		EXPECT_TRUE(brush.start(brushContext));
		if (brush.singleMode()) {
			EXPECT_FALSE(brush.active());
		} else {
			EXPECT_TRUE(brush.active());
			brushContext.cursorPosition = maxs;
			brush.step(brushContext);
		}
	}

	void testMirror(math::Axis axis, const glm::ivec3 &expectedMins, const glm::ivec3 &expectedMaxs) {
		ShapeBrush brush;
		BrushContext brushContext;
		ASSERT_TRUE(brush.init());
		const glm::ivec3 regMins(-2);
		const glm::ivec3 regMaxs = regMins;
		prepare(brush, brushContext, regMins, regMaxs);
		EXPECT_TRUE(brush.setMirrorAxis(axis, glm::ivec3(0)));
		const voxel::Region region(-3, 3);
		voxel::RawVolume volume(region);
		scenegraph::SceneGraph sceneGraph;
		ModifierVolumeWrapper wrapper(&volume, ModifierType::Place, {});
		brush.execute(sceneGraph, wrapper, brushContext);
		const voxel::Region dirtyRegion = wrapper.dirtyRegion();
		EXPECT_TRUE(dirtyRegion.isValid());
		EXPECT_NE(voxel::Voxel(), brushContext.cursorVoxel);
		EXPECT_EQ(brushContext.cursorVoxel, volume.voxel(regMins));
		EXPECT_EQ(dirtyRegion.getLowerCorner(), expectedMins);
		EXPECT_EQ(dirtyRegion.getUpperCorner(), expectedMaxs);
		brush.shutdown();
	}
};

TEST_F(ShapeBrushTest, testCenterPositive) {
	ShapeBrush brush;
	BrushContext brushContext;
	ASSERT_TRUE(brush.init());
	brush.setCenterMode(true);

	const glm::ivec3 mins(0);
	const glm::ivec3 maxs(1);
	prepare(brush, brushContext, mins, maxs);
	const voxel::Region region = brush.calcRegion(brushContext);
	const glm::ivec3 &dim = region.getDimensionsInVoxels();
	EXPECT_EQ(glm::ivec3(3), dim);
	brush.shutdown();
}

TEST_F(ShapeBrushTest, testCenterNegative) {
	ShapeBrush brush;
	BrushContext brushContext;
	ASSERT_TRUE(brush.init());
	brush.setCenterMode(true);
	prepare(brush, brushContext, glm::ivec3(0), glm::ivec3(-1));
	const voxel::Region region = brush.calcRegion(brushContext);
	const glm::ivec3 &dim = region.getDimensionsInVoxels();
	EXPECT_EQ(glm::ivec3(3), dim);
	brush.shutdown();
}

TEST_F(ShapeBrushTest, testModifierStartStop) {
	ShapeBrush brush;
	BrushContext brushContext;
	ASSERT_TRUE(brush.init());
	EXPECT_TRUE(brush.start(brushContext));
	EXPECT_TRUE(brush.active());
	brush.stop(brushContext);
	EXPECT_FALSE(brush.active());
	brush.shutdown();
}

TEST_F(ShapeBrushTest, testModifierDim) {
	ShapeBrush brush;
	BrushContext brushContext;
	ASSERT_TRUE(brush.init());
	prepare(brush, brushContext, glm::ivec3(-1), glm::ivec3(1));
	const voxel::Region region = brush.calcRegion(brushContext);
	const glm::ivec3 &dim = region.getDimensionsInVoxels();
	EXPECT_EQ(glm::ivec3(3), dim);
	brush.shutdown();
}

TEST_F(ShapeBrushTest, testModifierActionMirrorAxisX) {
	testMirror(math::Axis::X, glm::ivec3(-2), glm::ivec3(-2, -2, 1));
}

TEST_F(ShapeBrushTest, testModifierActionMirrorAxisY) {
	testMirror(math::Axis::Y, glm::ivec3(-2), glm::ivec3(-2, 1, -2));
}

TEST_F(ShapeBrushTest, testModifierActionMirrorAxisZ) {
	testMirror(math::Axis::Z, glm::ivec3(-2), glm::ivec3(1, -2, -2));
}

TEST_F(ShapeBrushTest, DISABLED_testPlace) {
	// TODO: implement me
}

TEST_F(ShapeBrushTest, DISABLED_testErase) {
	// TODO: implement me
}

TEST_F(ShapeBrushTest, DISABLED_testPaint) {
	// TODO: implement me
}

}; // namespace voxedit
