/**
 * @file
 */

#include "app/tests/AbstractTest.h"
#include "util/CustomButtonNames.h"
#include "util/KeybindingParser.h"
#include <SDL3/SDL.h>

namespace util {

namespace keybindingparser {
static const core::String CFG = R"(
w +foo all
alt+w "somecommand +" all
left_alt+l "someothercommand +" all
CTRL+a +bar all
CTRL+w +bar all
SHIFT+w +xyz all
SHIFT+ctrl+ALT+w allmodscommand all
ctrl+SHIFT+w ctrlshiftmodcommand all
left_alt altmodcommand all
ctrl++ "echo +" all
left_mouse void all
right_mouse void all
double_right_mouse void all
wheelup void all
wheeldown void all
+ "echo only+" all
)";
}

class KeybindingParserTest : public app::AbstractTest {
};

TEST_F(KeybindingParserTest, testParsing) {
	KeybindingParser p(keybindingparser::CFG);
	const BindMap& m = p.getBindings();
	ASSERT_FALSE(m.empty());
	ASSERT_EQ(0, p.invalidBindings()) << p.lastError();
	const size_t expected = 16;
	EXPECT_EQ(expected, m.size());

	int key = 'w';
	int count = 0;
	auto range = m.equal_range(key);
	bool allmodscommand = false;
	bool ctrlshiftmodcommand = false;
	bool xyz = false;
	bool bar = false;
	bool somecommand = false;
	bool foo = false;
	for (auto i = range.first; i != range.second; ++i) {
		const CommandModifierPair& pair = i->second;
		const core::String& command = pair.command;
		const int16_t mod = pair.modifier;
		if ((mod & SDL_KMOD_SHIFT) && (mod & SDL_KMOD_ALT) && (mod & SDL_KMOD_CONTROL)) {
			EXPECT_EQ("allmodscommand", command);
			EXPECT_FALSE(allmodscommand);
			allmodscommand = true;
		} else if ((mod & SDL_KMOD_SHIFT) && (mod & SDL_KMOD_CONTROL)) {
			EXPECT_EQ("ctrlshiftmodcommand", command);
			EXPECT_FALSE(ctrlshiftmodcommand);
			ctrlshiftmodcommand = true;
		} else if (mod & SDL_KMOD_SHIFT) {
			EXPECT_EQ("+xyz", command);
			EXPECT_FALSE(xyz);
			xyz = true;
		} else if (mod & SDL_KMOD_CONTROL) {
			EXPECT_EQ("+bar", command);
			EXPECT_FALSE(bar);
			bar = true;
		} else if (mod & SDL_KMOD_ALT) {
			EXPECT_EQ("somecommand +", command);
			EXPECT_FALSE(somecommand);
			somecommand = true;
		} else {
			EXPECT_EQ("+foo", command);
			EXPECT_FALSE(foo);
			foo = true;
		}
		++count;
	}

	ASSERT_TRUE(allmodscommand);
	ASSERT_TRUE(ctrlshiftmodcommand);
	ASSERT_TRUE(xyz);
	ASSERT_TRUE(bar);
	ASSERT_TRUE(somecommand);
	ASSERT_TRUE(foo);

	const int expectedWBindings = 6;
	EXPECT_EQ(expectedWBindings, count);

	key = 'l';
	count = 0;
	range = m.equal_range(key);
	for (auto i = range.first; i != range.second; ++i) {
		const CommandModifierPair& pair = i->second;
		const core::String& command = pair.command;
		const int16_t mod = pair.modifier;
		EXPECT_EQ("someothercommand +", command);
		EXPECT_TRUE(mod & SDL_KMOD_LALT) << "command " << command << " modifier wasn't parsed properly";
		EXPECT_TRUE(!(mod & SDL_KMOD_RALT)) << "command " << command << " modifier wasn't parsed properly";
		++count;
	}
	EXPECT_EQ(1, count);

	key = '+';
	count = 0;
	range = m.equal_range(key);
	for (auto i = range.first; i != range.second; ++i) {
		const CommandModifierPair& pair = i->second;
		const core::String& command = pair.command;
		const int16_t mod = pair.modifier;
		if ((mod & SDL_KMOD_CONTROL)) {
			EXPECT_EQ("echo +", command);
			EXPECT_TRUE(mod & SDL_KMOD_CONTROL) << "command " << command << " modifier wasn't parsed properly";
			++count;
		} else {
			if (mod == 0) {
				EXPECT_EQ("echo only+", command);
				++count;
			}
		}
	}
	EXPECT_EQ(2, count);

	key = SDLK_LALT;
	count = 0;
	range = m.equal_range(key);
	for (auto i = range.first; i != range.second; ++i) {
		const CommandModifierPair& pair = i->second;
		const core::String& command = pair.command;
		const int16_t mod = pair.modifier;
		EXPECT_EQ("altmodcommand", command);
		EXPECT_EQ(0, mod);
		++count;
	}
	EXPECT_EQ(1, count);
}

}
