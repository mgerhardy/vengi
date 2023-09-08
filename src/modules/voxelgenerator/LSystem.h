/**
 * @file
 */

#pragma once

#include "core/GLM.h"
#include "core/String.h"
#include "core/StringUtil.h"
#include "math/Random.h"
#include "core/collection/DynamicArray.h"
#include "core/collection/Stack.h"
#include "voxel/Voxel.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/random.hpp>

/**
 * Voxel generators
 */
namespace voxelgenerator {
/**
 * L-System
 */
namespace lsystem {

struct TurtleStep {
	glm::vec3 pos { 0.0f };
	glm::vec3 rotation = glm::up;
	float width;
	voxel::Voxel voxel;
};

struct Rule {
	char a = 'A';
	core::String b = "B";
};

extern bool parseRules(const core::String& rulesStr, core::DynamicArray<Rule>& rules);

/**
 * @brief Generate voxels according to the given L-System rules
 *
 * @li @c F Draw line forwards
 * @li @c ( Set voxel type
 * @li @c b Move backwards (no drawing)
 * @li @c L Leaf
 * @li @c + Rotate right
 * @li @c - Rotate left
 * @li @c > Rotate forward
 * @li @c < Rotate back
 * @li @c # Increment width
 * @li @c ! Decrement width
 * @li @c [ Push
 * @li @c ] Pop
 */
template<class Volume>
void generate(Volume& volume, const glm::ivec3& position, const core::String &axiom, const core::DynamicArray<Rule> &rules, float angle, float length, float width,
				 float widthIncrement, uint8_t iterations, math::Random& random, float leafRadius = 8.0f) {
	const float leafDistance = glm::round(2.0f * leafRadius);
	// apply a factor to close potential holes
	const int leavesVoxelCnt = (int)(glm::pow(leafDistance, 3) * 2.0);

	angle = glm::radians(angle);
	core::String sentence = axiom;
	for (int i = 0; i < iterations; i++) {
		core::String nextSentence = "";

		for (size_t j = 0; j < sentence.size(); ++j) {
			const char current = sentence[j];
			bool found = false;
			for (const auto &rule : rules) {
				if (rule.a == current) {
					found = true;
					nextSentence += rule.b;
					break;
				}
			}
			if (!found) {
				nextSentence += current;
			}
		}

		sentence = nextSentence;
	}

	core::Stack<TurtleStep, 512> stack;

	TurtleStep step;
	step.width = width;
	step.voxel = voxel::createVoxel(voxel::VoxelType::Generic, 0);

	for (size_t i = 0u; i < sentence.size(); ++i) {
		const char c = sentence[i];
		switch (c) {
		case 'F': {
			// Draw line forwards
			for (int j = 0; j < (int)length; j++) {
				float r = step.width / 2.0f;
				for (float x = -r; x < r; x++) {
					for (float y = -r; y < r; y++) {
						for (float z = -r; z < r; z++) {
							const glm::ivec3 dest(glm::round(step.pos + glm::vec3(x, y, z)));
							volume.setVoxel(position + dest, step.voxel);
						}
					}
				}
				step.pos += 1.0f * step.rotation;
			}
			break;
		}
		case '(': {
			// Set voxel type
			++i;
			int begin = i;
			size_t slength = 0u;
			while (sentence[i] >= '0' && sentence[i] <= '9') {
				++slength;
				++i;
			}
			core::String voxelString(sentence.c_str() + begin, slength);
			const int colorIndex = core::string::toInt(voxelString);
			if (colorIndex == 0) {
				step.voxel = voxel::Voxel();
			} else if (colorIndex > 0 && colorIndex < 256) {
				step.voxel = voxel::createVoxel(voxel::VoxelType::Generic, colorIndex);
			}
			break;
		}
		case 'b': {
			// Move backwards (no drawing)
			for (int j = 0; j < (int)length; j++) {
				step.pos -= 1.0f * step.rotation;
			}
			break;
		}

		case 'L': {
			// Leaf
			for (int j = 0; j < leavesVoxelCnt; j++) {
				const glm::vec3& r = glm::ballRand(leafRadius);
				const glm::ivec3 p = position + glm::ivec3(glm::round(step.pos + r));
				volume.setVoxel(p, step.voxel);
			}
			break;
		}

		case '+':
			// Rotate right
			step.rotation = glm::rotateZ(step.rotation, angle);
			break;

		case '-':
			// Rotate left
			step.rotation = glm::rotateZ(step.rotation, -angle);
			break;

		case '>':
			// Rotate forward
			step.rotation = glm::rotateX(step.rotation, angle);
			break;

		case '<':
			// Rotate back
			step.rotation = glm::rotateX(step.rotation, -angle);
			break;

		case '#':
			// Increment width
			step.width += widthIncrement;
			break;

		case '!':
			// Decrement width
			step.width -= widthIncrement;
			step.width = glm::max(1.1f, step.width);
			break;

		case '[':
			// Push
			stack.push(step);
			break;

		case ']':
			// Pop
			step = stack.top();
			stack.pop();
			break;
		}
	}
}

}
}
