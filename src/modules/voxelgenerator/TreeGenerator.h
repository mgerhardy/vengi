/**
 * @file
 */

#pragma once

#include "math/Random.h"
#include "TreeContext.h"
#include "Spiral.h"
#include "ShapeGenerator.h"
#include "SpaceColonization.h"
#include "math/AABB.h"
#include "core/Log.h"
#include "voxel/MaterialColor.h"
#include "voxel/Palette.h"
#include "voxel/Voxel.h"

namespace voxelgenerator {
/**
 * Tree generators
 */
namespace tree {

/**
 * @brief Tree space colonization algorithm
 */
class Tree : public SpaceColonization {
private:
	const int _trunkHeight;
	const float _trunkSizeFactor;

	void generateBranches(Branches& branches, const glm::vec3& direction, float maxSize, float branchLength) const;

public:
	/**
	 * @param[in] position The floor position of the trunk
	 * @param[in] trunkHeight The height of the trunk in voxels
	 */
	Tree(const glm::ivec3& position, int trunkHeight, int branchLength,
		int crownWidth, int crownHeight, int crownDepth, float branchSize, unsigned int seed, float trunkSizeFactor);
};


/**
 * @brief Creates a L form
 * @param[in,out] volume The volume (RawVolume) to place the voxels into
 * @param[in] pos The position to place the object at (lower left corner)
 * @param[in] width The width (x-axis) of the object
 * @param[in] height The height (y-axis) of the object
 * @param[in] depth The height (z-axis) of the object
 * @param[in] voxel The Voxel to build the object with
 */
template<class Volume>
static glm::ivec3 createL(Volume& volume, const glm::ivec3& pos, int width, int depth, int height, int thickness, const voxel::Voxel& voxel) {
	glm::ivec3 p = pos;
	if (width != 0) {
		shape::createCubeNoCenter(volume, p, width, thickness, thickness, voxel);
		p.x += width;
		shape::createCubeNoCenter(volume, p, thickness, height, thickness, voxel);
		p.x += thickness / 2;
		p.z += thickness / 2;
	} else if (depth != 0) {
		shape::createCubeNoCenter(volume, p, thickness, thickness, depth, voxel);
		p.z += depth;
		shape::createCubeNoCenter(volume, p, thickness, height, thickness, voxel);
		p.x += thickness / 2;
		p.z += thickness / 2;
	} else {
		core_assert(false);
	}
	p.y += height;
	return p;
}

/**
 * @brief Creates an ellipsis tree with side branches and smaller ellipsis on top of those branches
 * @sa createTreeEllipsis()
 */
template<class Volume>
void createTreeBranchEllipsis(Volume& volume, const voxelgenerator::TreeBranchEllipsis& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	const int top = ctx.pos.y + ctx.trunkHeight;
	shape::createCubeNoCenter(volume, ctx.pos - glm::ivec3(1), ctx.trunkStrength + 2, 1, ctx.trunkStrength + 2, trunkVoxel);
	shape::createCubeNoCenter(volume, ctx.pos, ctx.trunkStrength, ctx.trunkHeight, ctx.trunkStrength, trunkVoxel);
	if (ctx.trunkHeight <= 8) {
		return;
	}
	int branches[] = {1, 2, 3, 4};
	const int n = random.random(1, 4);
	for (int i = n; i < n + 4; ++i) {
		const int thickness = core_max(1, ctx.trunkStrength / 2);
		const int branchHeight = ctx.branchHeight;
		const int branchLength = ctx.branchLength;

		glm::ivec3 branch = ctx.pos;
		branch.y = random.random(ctx.pos.y + 2, top - 2);

		const int delta = (ctx.trunkStrength - thickness) / 2;
		glm::ivec3 leavesPos{0};
		switch (branches[i % 4]) {
		case 1:
			branch.x += delta;
			leavesPos = createL(volume, branch, 0, branchLength, branchHeight, thickness, trunkVoxel);
			break;
		case 2:
			branch.x += delta;
			leavesPos = createL(volume, branch, 0, -branchLength, branchHeight, thickness, trunkVoxel);
			break;
		case 3:
			branch.z += delta;
			leavesPos = createL(volume, branch, branchLength, 0, branchHeight, thickness, trunkVoxel);
			break;
		case 4:
			branch.z += delta;
			leavesPos = createL(volume, branch, -branchLength, 0, branchHeight, thickness, trunkVoxel);
			break;
		}
		leavesPos.y += branchHeight / 2;
		shape::createEllipse(volume, leavesPos, math::Axis::Y, branchHeight, branchHeight, branchHeight, leavesVoxel);
	}
	const glm::ivec3 leafesPos(ctx.pos.x + ctx.trunkStrength / 2, top + ctx.leavesHeight / 2, ctx.pos.z + ctx.trunkStrength / 2);
	shape::createEllipse(volume, leafesPos, math::Axis::Y, ctx.leavesWidth, ctx.leavesHeight, ctx.leavesDepth, leavesVoxel);
}

/**
 * @brief Creates the trunk for a tree - the full height of the tree is taken
 */
template<class Volume>
static void createTrunk(Volume& volume, const voxelgenerator::TreeConfig& ctx, const voxel::Voxel& voxel) {
	glm::ivec3 end = ctx.pos;
	end.y += ctx.trunkHeight;
	shape::createLine(volume, ctx.pos, end, voxel, ctx.trunkStrength);
}

/**
 * @return The end of the trunk to start the leaves from
 */
template<class Volume>
static glm::ivec3 createBezierTrunk(Volume& volume, const voxelgenerator::TreePalm& ctx, const voxel::Voxel& voxel) {
	const glm::ivec3& trunkTop = glm::ivec3(ctx.pos.x, ctx.pos.y + ctx.trunkHeight, ctx.pos.z);
	const int shiftX = ctx.trunkWidth;
	const int shiftZ = ctx.trunkDepth;
	glm::ivec3 end = trunkTop;
	end.x = trunkTop.x + shiftX;
	end.z = trunkTop.z + shiftZ;
	float trunkSize = (float)ctx.trunkStrength;
	const glm::ivec3 control(ctx.pos.x, ctx.pos.y + ctx.trunkControlOffset, ctx.pos.z);
	shape::createBezierFunc(volume, ctx.pos, end, control, voxel,
		[&] (Volume& volume, const glm::ivec3& last, const glm::ivec3& pos, const voxel::Voxel& voxel) {
			shape::createLine(volume, pos, last, voxel, core_max(1, (int)glm::ceil(trunkSize)));
			trunkSize *= ctx.trunkFactor;
		},
		ctx.trunkHeight);
	end.y -= 1;
	return end;
}

template<class Volume>
void createTreePalm(Volume& volume, const voxelgenerator::TreePalm& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	const glm::ivec3& start = createBezierTrunk(volume, ctx, trunkVoxel);
	const float stepWidth = glm::radians(360.0f / (float)ctx.branches);
	const float w = (float)ctx.leavesWidth;
	float angle = random.randomf(0.0f, glm::two_pi<float>());
	for (int b = 0; b < ctx.branches; ++b) {
		float branchSize = (float)ctx.branchSize;
		const float x = glm::cos(angle);
		const float z = glm::sin(angle);
		const int randomLength = random.random(ctx.leavesHeight, ctx.leavesHeight + ctx.randomLeavesHeightOffset);
		const glm::ivec3 control(start.x - x * (w / 2.0f), start.y + ctx.branchControlOffset, start.z - z * (w / 2.0f));
		const glm::ivec3 end(start.x - x * w, start.y - randomLength, start.z - z * w);
		shape::createBezierFunc(volume, start, end, control, leavesVoxel,
			[&] (Volume& vol, const glm::ivec3& last, const glm::ivec3& pos, const voxel::Voxel& voxel) {
				// TODO: this should be some kind of polygon - not a line - we want a flat leaf
				shape::createLine(vol, pos, last, voxel, core_max(1, (int)glm::ceil(branchSize)));
				branchSize *= ctx.branchFactor;
			},
			ctx.leavesHeight / 4);
		angle += stepWidth;
	}
}

template<class Volume>
void createTreeEllipsis(Volume& volume, const voxelgenerator::TreeEllipsis& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	createTrunk(volume, ctx, trunkVoxel);
	const glm::ivec3 leavesCenter(ctx.pos.x, ctx.pos.y + ctx.trunkHeight + ctx.leavesHeight / 2, ctx.pos.z);
	shape::createEllipse(volume, leavesCenter, math::Axis::Y, ctx.leavesWidth, ctx.leavesHeight, ctx.leavesDepth, leavesVoxel);
}

template<class Volume>
void createTreeCone(Volume& volume, const voxelgenerator::TreeCone& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	createTrunk(volume, ctx, trunkVoxel);
	const glm::ivec3 leavesCenter(ctx.pos.x, ctx.pos.y + ctx.trunkHeight + ctx.leavesHeight / 2, ctx.pos.z);
	shape::createCone(volume, leavesCenter, math::Axis::Y, false, ctx.leavesHeight, ctx.leavesWidth, ctx.leavesDepth, leavesVoxel);
}

/**
 * @brief Creates a fir with several branches based on lines falling down from the top of the tree
 */
template<class Volume>
void createTreeFir(Volume& volume, const voxelgenerator::TreeFir& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	createTrunk(volume, ctx, trunkVoxel);

	const float stepWidth = glm::radians(360.0f / (float)ctx.branches);
	float angle = random.randomf(0.0f, glm::two_pi<float>());
	glm::ivec3 leafesPos = glm::ivec3(ctx.pos.x, ctx.pos.y + ctx.trunkHeight, ctx.pos.z);
	const int halfHeight = ((ctx.amount - 1) * ctx.stepHeight) / 2;
	glm::ivec3 center(ctx.pos.x, ctx.pos.y + ctx.trunkHeight - halfHeight, ctx.pos.z);
	shape::createCube(volume, center, ctx.trunkStrength, halfHeight * 2, ctx.trunkStrength, leavesVoxel);

	float w = ctx.w;
	for (int n = 0; n < ctx.amount; ++n) {
		for (int b = 0; b < ctx.branches; ++b) {
			glm::ivec3 start = leafesPos;
			glm::ivec3 end = start;
			const float x = glm::cos(angle);
			const float z = glm::sin(angle);
			const int randomZ = random.random(4, 8);
			end.y -= randomZ;
			end.x -= x * w;
			end.z -= z * w;
			shape::createLine(volume, start, end, leavesVoxel, ctx.branchStrength);
			glm::ivec3 end2 = end;
			end2.y -= ctx.branchDownwardOffset;
			end2.x -= x * w * ctx.branchPositionFactor;
			end2.z -= z * w * ctx.branchPositionFactor;
			shape::createLine(volume, end, end2, leavesVoxel, ctx.branchStrength);
			angle += stepWidth;
			w += (float)(1.0 / (double)(b + 1));
		}
		leafesPos.y -= ctx.stepHeight;
	}
}

template<class Volume>
void createTreePine(Volume& volume, const voxelgenerator::TreePine& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	createTrunk(volume, ctx, trunkVoxel);
	const int singleStepHeight = ctx.singleLeafHeight + ctx.singleStepDelta;
	const int steps = core_max(1, ctx.leavesHeight / singleStepHeight);
	const int stepWidth = ctx.leavesWidth / steps;
	const int stepDepth = ctx.leavesDepth / steps;
	int currentWidth = ctx.startWidth;
	int currentDepth = ctx.startDepth;
	const int top = ctx.pos.y + ctx.trunkHeight;
	glm::ivec3 leavesPos(ctx.pos.x, top, ctx.pos.z);
	for (int i = 0; i < steps; ++i) {
		shape::createDome(volume, leavesPos, math::Axis::Y, false, currentWidth, ctx.singleLeafHeight, currentDepth, leavesVoxel);
		leavesPos.y -= ctx.singleStepDelta;
		shape::createDome(volume, leavesPos, math::Axis::Y, false, currentWidth + 1, ctx.singleLeafHeight, currentDepth + 1, leavesVoxel);
		currentDepth += stepDepth;
		currentWidth += stepWidth;
		leavesPos.y -= ctx.singleLeafHeight;
	}
}

/**
 * @sa createTreeDomeHangingLeaves()
 */
template<class Volume>
void createTreeDome(Volume& volume, const voxelgenerator::TreeDome& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	createTrunk(volume, ctx, trunkVoxel);
	const glm::ivec3 leavesCenter(ctx.pos.x, ctx.pos.y + ctx.trunkHeight + ctx.leavesHeight / 2, ctx.pos.z);
	shape::createDome(volume, leavesCenter, math::Axis::Y, false, ctx.leavesWidth, ctx.leavesHeight, ctx.leavesDepth, leavesVoxel);
}

/**
 * @brief Creates a dome based tree with leaves hanging down from the dome.
 * @sa createTreeDome()
 */
template<class Volume>
void createTreeDomeHangingLeaves(Volume& volume, const voxelgenerator::TreeDomeHanging& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	createTrunk(volume, ctx, trunkVoxel);
	const glm::ivec3 leavesCenter(ctx.pos.x, ctx.pos.y + ctx.trunkHeight + ctx.leavesHeight / 2, ctx.pos.z);
	shape::createDome(volume, leavesCenter, math::Axis::Y, false, ctx.leavesWidth, ctx.leavesHeight, ctx.leavesDepth, leavesVoxel);
	const float stepWidth = glm::radians(360.0f / (float)ctx.branches);
	float angle = random.randomf(0.0f, glm::two_pi<float>());
	// leaves falling down
	const int y = ctx.pos.y + ctx.trunkHeight + 1;
	for (int b = 0; b < ctx.branches; ++b) {
		const float x = glm::cos(angle);
		const float z = glm::sin(angle);
		const int randomLength = random.random(ctx.hangingLeavesLengthMin, ctx.hangingLeavesLengthMax);

		const glm::ivec3 start(glm::round((float)ctx.pos.x - x * ((float)ctx.leavesWidth - 1.0f) / 2.0f), y, glm::round((float)ctx.pos.z - z * ((float)ctx.leavesDepth - 1.0f) / 2.0f));
		const glm::ivec3 end(start.x, start.y - randomLength, start.z);
		shape::createLine(volume, start, end, leavesVoxel, ctx.hangingLeavesThickness);

		angle += stepWidth;
	}
}

/**
 * @sa createTreeCubeSideCubes()
 */
template<class Volume>
void createTreeCube(Volume& volume, const voxelgenerator::TreeCube& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	createTrunk(volume, ctx, trunkVoxel);

	const glm::ivec3 leavesCenter(ctx.pos.x, ctx.pos.y + ctx.trunkHeight + ctx.leavesHeight / 2, ctx.pos.z);
	shape::createCube(volume, leavesCenter, ctx.leavesWidth, ctx.leavesHeight, ctx.leavesDepth, leavesVoxel);
	shape::createCube(volume, leavesCenter, ctx.leavesWidth + 2, ctx.leavesHeight - 2, ctx.leavesDepth - 2, leavesVoxel);
	shape::createCube(volume, leavesCenter, ctx.leavesWidth - 2, ctx.leavesHeight + 2, ctx.leavesDepth - 2, leavesVoxel);
	shape::createCube(volume, leavesCenter, ctx.leavesWidth - 2, ctx.leavesHeight - 2, ctx.leavesDepth + 2, leavesVoxel);
}

/**
 * @brief Creates a cube based tree with small cubes on the four side faces
 * @sa createTreeCube()
 */
template<class Volume>
void createTreeCubeSideCubes(Volume& volume, const voxelgenerator::TreeCube& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	createTrunk(volume, ctx, trunkVoxel);

	const glm::ivec3& leafesPos = glm::ivec3(ctx.pos.x, ctx.pos.y + ctx.trunkHeight + ctx.leavesHeight / 2, ctx.pos.z);
	shape::createCube(volume, leafesPos, ctx.leavesWidth, ctx.leavesHeight, ctx.leavesDepth, leavesVoxel);
	shape::createCube(volume, leafesPos, ctx.leavesWidth + 2, ctx.leavesHeight - 2, ctx.leavesDepth - 2, leavesVoxel);
	shape::createCube(volume, leafesPos, ctx.leavesWidth - 2, ctx.leavesHeight + 2, ctx.leavesDepth - 2, leavesVoxel);
	shape::createCube(volume, leafesPos, ctx.leavesWidth - 2, ctx.leavesHeight - 2, ctx.leavesDepth + 2, leavesVoxel);

	Spiral o;
	o.next();
	const int halfWidth = ctx.leavesWidth / 2;
	const int halfHeight = ctx.leavesHeight / 2;
	const int halfDepth = ctx.leavesDepth / 2;
	for (int i = 0; i < 4; ++i) {
		glm::ivec3 leafesPos2 = leafesPos;
		leafesPos2.x += o.x() * halfWidth;
		leafesPos2.z += o.z() * halfDepth;
		shape::createEllipse(volume, leafesPos2, math::Axis::Y, halfWidth, halfHeight, halfDepth, leavesVoxel);
		o.next(2);
	}
}

template<class Volume>
void createSpaceColonizationTree(Volume& volume, const voxelgenerator::TreeSpaceColonization& ctx, math::Random& random, const voxel::Voxel &trunkVoxel, const voxel::Voxel &leavesVoxel) {
	Tree tree(ctx.pos, ctx.trunkHeight, ctx.branchSize, ctx.leavesWidth, ctx.leavesHeight,
			ctx.leavesDepth, (float)ctx.trunkStrength, ctx.seed, ctx.trunkFactor);
	tree.grow();
	tree.generate(volume, trunkVoxel);
	const int leafSize = core_max(core_max(ctx.leavesWidth, ctx.leavesHeight), ctx.leavesDepth);
	SpaceColonization::RandomSize rndSize(random, leafSize);
	tree.generateLeaves(volume, leavesVoxel, rndSize);
}

/**
 * @brief Delegates to the corresponding create method for the given TreeType in the TreeContext
 */
template<class Volume>
void createTree(Volume& volume, const voxelgenerator::TreeContext& ctx, math::Random& random) {
	const voxel::Palette &palette = voxel::getPalette();
	const uint8_t green = palette.getClosestMatch(core::RGBA(123, 162, 63));
	const uint8_t brown = palette.getClosestMatch(core::RGBA(143, 90, 60));
	const voxel::Voxel trunkVoxel = voxel::createVoxel(palette, brown);
	const voxel::Voxel leavesVoxel = voxel::createVoxel(palette, green);
	if (ctx.cfg.type == TreeType::BranchesEllipsis) {
		createTreeBranchEllipsis(volume, ctx.branchellipsis, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::Ellipsis) {
		createTreeEllipsis(volume, ctx.ellipsis, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::Palm) {
		createTreePalm(volume, ctx.palm, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::Cone) {
		createTreeCone(volume, ctx.cone, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::Fir) {
		createTreeFir(volume, ctx.fir, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::Pine) {
		createTreePine(volume, ctx.pine, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::Dome) {
		createTreeDome(volume, ctx.dome, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::DomeHangingLeaves) {
		createTreeDomeHangingLeaves(volume, ctx.domehanging, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::Cube) {
		createTreeCube(volume, ctx.cube, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::CubeSideCubes) {
		createTreeCubeSideCubes(volume, ctx.cube, random, trunkVoxel, leavesVoxel);
	} else if (ctx.cfg.type == TreeType::SpaceColonization) {
		createSpaceColonizationTree(volume, ctx.spacecolonization, random, trunkVoxel, leavesVoxel);
	}
}

}
}
