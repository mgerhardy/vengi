/**
 * @file
 */

#include "AbstractFormatTest.h"

namespace voxelformat {

class SpriteStackFormatTest : public AbstractFormatTest {};

TEST_F(SpriteStackFormatTest, testLoad) {
	testLoad("spritestack.zip", 1);
}

} // namespace voxelformat
