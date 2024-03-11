/**
 * @file
 */

#include <gtest/gtest.h>
#include "core/Color.h"
#include "core/RGBA.h"
#include "core/ArrayLength.h"
#include "core/StringUtil.h"
#include "core/collection/BufferView.h"
#include <SDL_endian.h>

namespace core {

inline std::ostream &operator<<(::std::ostream &os, const core::BufferView<RGBA> &dt) {
	core::String palStr;
	core::String line;
	for (size_t i = 0; i < dt.size(); ++i) {
		if (i % 16 == 0 && !line.empty()) {
			palStr.append(core::string::format("%03i %s\n", (int)i - 16, line.c_str()));
			line = "";
		}
		const core::String c = core::Color::print(dt[i], false);
		line += c;
	}
	if (!line.empty()) {
		palStr.append(core::string::format("%03i %s\n", (int)dt.size() / 16 * 16, line.c_str()));
	}
	return os << palStr.c_str();
}

TEST(ColorTest, testRGBA) {
	core::RGBA color;
	color.rgba = SDL_SwapLE32(0xff6699fe);
	EXPECT_EQ(0xfe, color.r);
	EXPECT_EQ(0x99, color.g);
	EXPECT_EQ(0x66, color.b);
	EXPECT_EQ(0xff, color.a);

	const glm::vec4 fcolor = core::Color::fromRGBA(color);
	EXPECT_FLOAT_EQ(color.r / core::Color::magnitudef, fcolor.r);
	EXPECT_FLOAT_EQ(color.g / core::Color::magnitudef, fcolor.g);
	EXPECT_FLOAT_EQ(color.b / core::Color::magnitudef, fcolor.b);
	EXPECT_FLOAT_EQ(color.a / core::Color::magnitudef, fcolor.a);
	EXPECT_FLOAT_EQ(1.0f, fcolor.a);

	core::RGBA convertedBack = core::Color::getRGBA(fcolor);
	EXPECT_EQ(0xfe, convertedBack.r);
	EXPECT_EQ(0x99, convertedBack.g);
	EXPECT_EQ(0x66, convertedBack.b);
	EXPECT_EQ(0xff, convertedBack.a);
}

TEST(ColorTest, testHex) {
	EXPECT_EQ(glm::vec4(1.0f), core::Color::fromHex("#ffffff"));
	EXPECT_EQ(glm::vec4(1.0f), core::Color::fromHex("0xffffff"));
	EXPECT_EQ(glm::vec4(1.0f), core::Color::fromHex("0xffffffff"));
	EXPECT_EQ(glm::vec4(0.0f), core::Color::fromHex("0x00000000"));
	EXPECT_EQ(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), core::Color::fromHex("0xff0000ff"));
	EXPECT_EQ(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), core::Color::fromHex("#ff0000ff"));
}

TEST(ColorTest, testQuantize) {
	const core::RGBA buf[] {
		core::RGBA(0x00, 0x00, 0x00), core::RGBA(0x7d, 0x7d, 0x7d), core::RGBA(0x4c, 0xb3, 0x76), core::RGBA(0x43, 0x60, 0x86), core::RGBA(0x7a, 0x7a, 0x7a), core::RGBA(0x4e, 0x7f, 0x9c), core::RGBA(0x25, 0x66, 0x47), core::RGBA(0x53, 0x53, 0x53), core::RGBA(0xdc, 0xaf, 0x70),
		core::RGBA(0xdc, 0xaf, 0x70), core::RGBA(0x13, 0x5b, 0xcf), core::RGBA(0x12, 0x5a, 0xd4), core::RGBA(0xa0, 0xd3, 0xdb), core::RGBA(0x7a, 0x7c, 0x7e), core::RGBA(0x7c, 0x8b, 0x8f), core::RGBA(0x7e, 0x82, 0x87), core::RGBA(0x73, 0x73, 0x73), core::RGBA(0x31, 0x51, 0x66),
		core::RGBA(0x31, 0xb2, 0x45), core::RGBA(0x54, 0xc3, 0xc2), core::RGBA(0xf4, 0xf0, 0xda), core::RGBA(0x86, 0x70, 0x66), core::RGBA(0x89, 0x43, 0x26), core::RGBA(0x83, 0x83, 0x83), core::RGBA(0x9f, 0xd3, 0xdc), core::RGBA(0x32, 0x43, 0x64), core::RGBA(0x36, 0x34, 0xb4),
		core::RGBA(0x23, 0xc7, 0xf6), core::RGBA(0x7c, 0x7c, 0x7c), core::RGBA(0x77, 0xbf, 0x8e), core::RGBA(0xdc, 0xdc, 0xdc), core::RGBA(0x29, 0x65, 0x95), core::RGBA(0x19, 0x4f, 0x7b), core::RGBA(0x53, 0x8b, 0xa5), core::RGBA(0x5e, 0x96, 0xbd), core::RGBA(0xdd, 0xdd, 0xdd),
		core::RGBA(0xe5, 0xe5, 0xe5), core::RGBA(0x00, 0xff, 0xff), core::RGBA(0x0d, 0x00, 0xda), core::RGBA(0x41, 0x57, 0x78), core::RGBA(0x0d, 0x0f, 0xe1), core::RGBA(0x4e, 0xec, 0xf9), core::RGBA(0xdb, 0xdb, 0xdb), core::RGBA(0xa1, 0xa1, 0xa1), core::RGBA(0xa6, 0xa6, 0xa6),
		core::RGBA(0x06, 0x30, 0xbc), core::RGBA(0x00, 0x26, 0xaf), core::RGBA(0x39, 0x58, 0x6b), core::RGBA(0x65, 0x87, 0x65), core::RGBA(0x1d, 0x12, 0x14), core::RGBA(0x00, 0xff, 0xff), core::RGBA(0x00, 0x5f, 0xde), core::RGBA(0x31, 0x27, 0x1a), core::RGBA(0x4e, 0x87, 0xa6),
		core::RGBA(0x2a, 0x74, 0xa4), core::RGBA(0x00, 0x00, 0xff), core::RGBA(0x8f, 0x8c, 0x81), core::RGBA(0xd5, 0xdb, 0x61), core::RGBA(0x2e, 0x50, 0x88), core::RGBA(0x17, 0x59, 0x3c), core::RGBA(0x33, 0x56, 0x82), core::RGBA(0x67, 0x67, 0x67), core::RGBA(0x00, 0xb9, 0xff),
		core::RGBA(0x5b, 0x9a, 0xb8), core::RGBA(0x38, 0x73, 0x94), core::RGBA(0x34, 0x5f, 0x79), core::RGBA(0x51, 0x90, 0xb6), core::RGBA(0x6a, 0x6a, 0x6a), core::RGBA(0x5b, 0x9a, 0xb8), core::RGBA(0x40, 0x59, 0x6a), core::RGBA(0x7a, 0x7a, 0x7a), core::RGBA(0xc2, 0xc2, 0xc2),
		core::RGBA(0x65, 0xa0, 0xc9), core::RGBA(0x6b, 0x6b, 0x84), core::RGBA(0x2d, 0x2d, 0xdd), core::RGBA(0x00, 0x00, 0x66), core::RGBA(0x00, 0x61, 0xff), core::RGBA(0x84, 0x84, 0x84), core::RGBA(0xf1, 0xf1, 0xdf), core::RGBA(0xff, 0xad, 0x7d), core::RGBA(0xfb, 0xfb, 0xef),
		core::RGBA(0x1d, 0x83, 0x0f), core::RGBA(0xb0, 0xa4, 0x9e), core::RGBA(0x65, 0xc0, 0x94), core::RGBA(0x3b, 0x59, 0x85), core::RGBA(0x42, 0x74, 0x8d), core::RGBA(0x1b, 0x8c, 0xe3), core::RGBA(0x34, 0x36, 0x6f), core::RGBA(0x33, 0x40, 0x54), core::RGBA(0x45, 0x76, 0x8f),
		core::RGBA(0xbf, 0x0a, 0x57), core::RGBA(0x21, 0x98, 0xf1), core::RGBA(0xff, 0xff, 0xec), core::RGBA(0xb2, 0xb2, 0xb2), core::RGBA(0xb2, 0xb2, 0xb2), core::RGBA(0xff, 0xff, 0xff), core::RGBA(0x2d, 0x5d, 0x7e), core::RGBA(0x7c, 0x7c, 0x7c), core::RGBA(0x7a, 0x7a, 0x7a),
		core::RGBA(0x7c, 0xaf, 0xcf), core::RGBA(0x78, 0xaa, 0xca), core::RGBA(0x6a, 0x6c, 0x6d), core::RGBA(0xf4, 0xef, 0xd3), core::RGBA(0x28, 0xbd, 0xc4), core::RGBA(0x69, 0xdd, 0x92), core::RGBA(0x53, 0xae, 0x73), core::RGBA(0x0c, 0x51, 0x20), core::RGBA(0x52, 0x87, 0xa5),
		core::RGBA(0x2a, 0x40, 0x94), core::RGBA(0x7a, 0x7a, 0x7a), core::RGBA(0x75, 0x71, 0x8a), core::RGBA(0x76, 0x76, 0x76), core::RGBA(0x1a, 0x16, 0x2c), core::RGBA(0x1a, 0x16, 0x2c), core::RGBA(0x1a, 0x16, 0x2c), core::RGBA(0x2d, 0x28, 0xa6), core::RGBA(0xb1, 0xc4, 0x54),
		core::RGBA(0x51, 0x67, 0x7c), core::RGBA(0x49, 0x49, 0x49), core::RGBA(0x34, 0x34, 0x34), core::RGBA(0xd1, 0x89, 0x34), core::RGBA(0xa5, 0xdf, 0xdd), core::RGBA(0x0f, 0x09, 0x0c), core::RGBA(0x31, 0x63, 0x97), core::RGBA(0x42, 0xa0, 0xe3), core::RGBA(0x4d, 0x84, 0xa1),
		core::RGBA(0x49, 0x85, 0x9e), core::RGBA(0x1f, 0x71, 0xdd), core::RGBA(0xa8, 0xe2, 0xe7), core::RGBA(0x74, 0x80, 0x6d), core::RGBA(0x3c, 0x3a, 0x2a), core::RGBA(0x7c, 0x7c, 0x7c), core::RGBA(0x5a, 0x5a, 0x5a), core::RGBA(0x75, 0xd9, 0x51), core::RGBA(0x34, 0x5e, 0x81),
		core::RGBA(0x84, 0xc0, 0xce), core::RGBA(0x45, 0x5f, 0x88), core::RGBA(0x86, 0x8b, 0x8e), core::RGBA(0xd7, 0xdd, 0x74), core::RGBA(0x59, 0x59, 0x59), core::RGBA(0x33, 0x41, 0x76), core::RGBA(0x00, 0x8c, 0x0a), core::RGBA(0x17, 0xa4, 0x04), core::RGBA(0x59, 0x92, 0xb3),
		core::RGBA(0xb0, 0xb0, 0xb0), core::RGBA(0x43, 0x43, 0x47), core::RGBA(0x1d, 0x6b, 0x9e), core::RGBA(0x70, 0xfd, 0xfe), core::RGBA(0xe5, 0xe5, 0xe5), core::RGBA(0x4c, 0x4a, 0x4b), core::RGBA(0xbd, 0xc6, 0xbf), core::RGBA(0xdd, 0xed, 0xfb), core::RGBA(0x09, 0x1b, 0xab),
		core::RGBA(0x4f, 0x54, 0x7d), core::RGBA(0x71, 0x71, 0x71), core::RGBA(0xdf, 0xe6, 0xea), core::RGBA(0xe3, 0xe8, 0xeb), core::RGBA(0x41, 0x81, 0x9b), core::RGBA(0x74, 0x74, 0x74), core::RGBA(0xa1, 0xb2, 0xd1), core::RGBA(0xf6, 0xf6, 0xf6), core::RGBA(0x87, 0x87, 0x87),
		core::RGBA(0x39, 0x5a, 0xb0), core::RGBA(0x32, 0x5c, 0xac), core::RGBA(0x15, 0x2c, 0x47), core::RGBA(0x65, 0xc8, 0x78), core::RGBA(0x35, 0x34, 0xdf), core::RGBA(0xc7, 0xc7, 0xc7), core::RGBA(0xa5, 0xaf, 0x72), core::RGBA(0xbe, 0xc7, 0xac), core::RGBA(0x9f, 0xd3, 0xdc),
		core::RGBA(0xca, 0xca, 0xca), core::RGBA(0x42, 0x5c, 0x96), core::RGBA(0x12, 0x12, 0x12), core::RGBA(0xf4, 0xbf, 0xa2), core::RGBA(0x14, 0x74, 0xcf), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0x1d, 0x56, 0xac),
		core::RGBA(0x1d, 0x57, 0xae), core::RGBA(0x1d, 0x57, 0xae), core::RGBA(0x1d, 0x57, 0xae), core::RGBA(0x24, 0x3c, 0x50), core::RGBA(0x8d, 0xcd, 0xdd), core::RGBA(0x4d, 0x7a, 0xaf), core::RGBA(0x0e, 0x20, 0x34), core::RGBA(0x36, 0x6b, 0xcf), core::RGBA(0x35, 0x5d, 0x7e),
		core::RGBA(0x7b, 0xb8, 0xc7), core::RGBA(0x5f, 0x86, 0xbb), core::RGBA(0x1e, 0x2e, 0x3f), core::RGBA(0x3a, 0x6b, 0xc5), core::RGBA(0x30, 0x53, 0x6e), core::RGBA(0xe0, 0xf3, 0xf7), core::RGBA(0x50, 0x77, 0xa9), core::RGBA(0x29, 0x55, 0xaa), core::RGBA(0x21, 0x37, 0x4e),
		core::RGBA(0xcd, 0xc5, 0xdc), core::RGBA(0x60, 0x3b, 0x60), core::RGBA(0x85, 0x67, 0x85), core::RGBA(0xa6, 0x79, 0xa6), core::RGBA(0xaa, 0x7e, 0xaa), core::RGBA(0xa8, 0x79, 0xa8), core::RGBA(0xa8, 0x79, 0xa8), core::RGBA(0xa8, 0x79, 0xa8), core::RGBA(0xaa, 0xe6, 0xe1),
		core::RGBA(0xaa, 0xe6, 0xe1), core::RGBA(0x45, 0x7d, 0x98), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0),
		core::RGBA(0x40, 0x2a, 0x94), core::RGBA(0x7a, 0x7a, 0x7a), core::RGBA(0x71, 0x75, 0x8a), core::RGBA(0x76, 0x76, 0x76), core::RGBA(0x16, 0x1a, 0x2c), core::RGBA(0x16, 0x2c, 0x1a), core::RGBA(0x16, 0x1a, 0x2c), core::RGBA(0x28, 0x2d, 0xa6), core::RGBA(0xc4, 0xb1, 0x54),
		core::RGBA(0x67, 0x51, 0x7c), core::RGBA(0x49, 0x49, 0x49), core::RGBA(0x34, 0x34, 0x34), core::RGBA(0x89, 0xd1, 0x34), core::RGBA(0xdf, 0xa5, 0xdd), core::RGBA(0x09, 0x0c, 0x0f), core::RGBA(0x63, 0x31, 0x97), core::RGBA(0xa0, 0x42, 0xe3), core::RGBA(0x84, 0x4d, 0xa1),
		core::RGBA(0x85, 0x49, 0x9e), core::RGBA(0x71, 0x1f, 0xdd), core::RGBA(0xe2, 0xa8, 0xe7), core::RGBA(0x80, 0x74, 0x6d), core::RGBA(0x3a, 0x3c, 0x2a), core::RGBA(0x7c, 0x7c, 0x7c), core::RGBA(0x5a, 0x5a, 0x5a), core::RGBA(0xd9, 0x75, 0x51), core::RGBA(0x5e, 0x34, 0x81),
		core::RGBA(0xc0, 0x84, 0xce), core::RGBA(0x5f, 0x45, 0x88), core::RGBA(0x8b, 0x86, 0x8e), core::RGBA(0xdd, 0xd7, 0x74), core::RGBA(0x59, 0x59, 0x59), core::RGBA(0x41, 0x76, 0x33), core::RGBA(0x8c, 0x00, 0x0a), core::RGBA(0xa4, 0x17, 0x04), core::RGBA(0x92, 0x59, 0xb3),
		core::RGBA(0xb0, 0xb0, 0xb0), core::RGBA(0x43, 0x43, 0x47), core::RGBA(0x6b, 0x1d, 0x9e), core::RGBA(0xfd, 0x70, 0xfe), core::RGBA(0xe5, 0xe5, 0xe5), core::RGBA(0x4a, 0x4b, 0x4c), core::RGBA(0xc6, 0xbd, 0xbf), core::RGBA(0xed, 0xdd, 0xfb), core::RGBA(0x1b, 0x09, 0xab),
		core::RGBA(0x54, 0x4f, 0x7d), core::RGBA(0x71, 0x71, 0x71), core::RGBA(0xe6, 0xdf, 0xea), core::RGBA(0xe8, 0xe3, 0xeb), core::RGBA(0x81, 0x41, 0x9b), core::RGBA(0x74, 0x74, 0x74), core::RGBA(0xb2, 0xa1, 0xd1), core::RGBA(0xf6, 0xf6, 0xf6), core::RGBA(0x87, 0x87, 0x87),
		core::RGBA(0x5a, 0x39, 0xb0), core::RGBA(0x5c, 0x32, 0xac), core::RGBA(0x2c, 0x15, 0x47), core::RGBA(0xc8, 0x65, 0x78), core::RGBA(0x34, 0x35, 0xdf), core::RGBA(0xc7, 0xc7, 0xc7), core::RGBA(0xaf, 0xa5, 0x72), core::RGBA(0xc7, 0xbe, 0xac), core::RGBA(0xd3, 0x9f, 0xdc),
		core::RGBA(0xca, 0xca, 0xca), core::RGBA(0x5c, 0x42, 0x96), core::RGBA(0x12, 0x12, 0x12), core::RGBA(0xbf, 0xf4, 0xa2), core::RGBA(0x74, 0x14, 0xcf), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0x56, 0x1d, 0xac),
		core::RGBA(0x57, 0x1d, 0xae), core::RGBA(0x57, 0x1d, 0xae), core::RGBA(0x57, 0x1d, 0xae), core::RGBA(0x3c, 0x24, 0x50), core::RGBA(0xcd, 0x8d, 0xdd), core::RGBA(0x7a, 0xaf, 0x4d), core::RGBA(0x20, 0x0e, 0x34), core::RGBA(0x6b, 0x36, 0xcf), core::RGBA(0x5d, 0x35, 0x7e),
		core::RGBA(0xb8, 0x7b, 0xc7), core::RGBA(0x86, 0x5f, 0xbb), core::RGBA(0x2e, 0x1e, 0x3f), core::RGBA(0x6b, 0x3a, 0xc5), core::RGBA(0x53, 0x30, 0x6e), core::RGBA(0xf3, 0xf7, 0xe0), core::RGBA(0x77, 0x50, 0xa9), core::RGBA(0x55, 0x29, 0xaa), core::RGBA(0x37, 0x21, 0x4e),
		core::RGBA(0xc5, 0xcd, 0xdc), core::RGBA(0x3b, 0x60, 0x60), core::RGBA(0x67, 0x85, 0x85), core::RGBA(0x79, 0xa6, 0xa6), core::RGBA(0x7e, 0xaa, 0xaa), core::RGBA(0x79, 0xa8, 0xa8), core::RGBA(0x79, 0xa8, 0xa8), core::RGBA(0x79, 0xa8, 0xa8), core::RGBA(0xe6, 0xaa, 0xe1),
		core::RGBA(0xe6, 0xaa, 0xe1), core::RGBA(0x7d, 0x45, 0x98), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf0),
		core::RGBA(0xf0, 0xf1, 0xf0), core::RGBA(0xf1, 0xf0, 0xf0), core::RGBA(0xf0, 0xf0, 0xf1), core::RGBA(0xf1, 0xf1, 0xf0), core::RGBA(0xf4, 0xf2, 0xf2), core::RGBA(0xfe, 0xd0, 0xff), core::RGBA(0xc0, 0xf0, 0x50), core::RGBA(0x20, 0xf0, 0x10), core::RGBA(0xd0, 0xf0, 0xc0),
		core::RGBA(0xf2, 0xf0, 0xf0), core::RGBA(0xf0, 0xf2, 0xf0), core::RGBA(0xf0, 0xf0, 0xf2), core::RGBA(0xf0, 0xf1, 0xf1), core::RGBA(0xf3, 0xf3, 0xf3), core::RGBA(0xf1, 0xf6, 0xd0), core::RGBA(0xe0, 0xc0, 0x50), core::RGBA(0xf0, 0x20, 0x10), core::RGBA(0xf0, 0xd0, 0xc0),
		core::RGBA(0xf3, 0xf0, 0xf0), core::RGBA(0xf0, 0xf3, 0xf0), core::RGBA(0xf0, 0xf0, 0xf3), core::RGBA(0xf1, 0xf0, 0xf1), core::RGBA(0xf4, 0xf2, 0xf4), core::RGBA(0xf2, 0xf7, 0xd0), core::RGBA(0xd0, 0xc0, 0x50), core::RGBA(0xf0, 0x20, 0x10), core::RGBA(0xf0, 0xd0, 0xc0),
		core::RGBA(0xf4, 0xf0, 0xf0), core::RGBA(0xf0, 0xf4, 0xf0), core::RGBA(0xf0, 0xf0, 0xf4), core::RGBA(0xf1, 0xf1, 0xf0), core::RGBA(0xf5, 0xf1, 0xf5), core::RGBA(0xf8, 0xf8, 0xd0), core::RGBA(0xc0, 0xc0, 0x50), core::RGBA(0xf0, 0x20, 0x10), core::RGBA(0xf0, 0xd0, 0xc0),
		core::RGBA(0xf5, 0xf0, 0xf0), core::RGBA(0xf0, 0xf5, 0xf0), core::RGBA(0xf0, 0xf0, 0xf5), core::RGBA(0x24, 0x21, 0x32), core::RGBA(0x24, 0x21, 0x32), core::RGBA(0x24, 0x21, 0x32), core::RGBA(0x24, 0x21, 0x32), core::RGBA(0x24, 0x21, 0x32), core::RGBA(0x24, 0x21, 0x32)
	};
	core::RGBA targetBuf[256] {};
	int n;
	n = core::Color::quantize(targetBuf, lengthof(targetBuf), buf, lengthof(buf), core::Color::ColorReductionType::Octree);
	EXPECT_EQ(256, n) << "Failed with octree.\n" << core::BufferView<RGBA>(targetBuf, n) << "\n" << core::BufferView<RGBA>(buf, lengthof(buf));

	n = core::Color::quantize(targetBuf, lengthof(targetBuf), buf, lengthof(buf), core::Color::ColorReductionType::Wu);
	EXPECT_EQ(161, n) << "Failed with Wu.\n" << core::BufferView<RGBA>(targetBuf, n) << "\n" << core::BufferView<RGBA>(buf, lengthof(buf));

	// n = core::Color::quantize(targetBuf, lengthof(targetBuf), buf, lengthof(buf), core::Color::ColorReductionType::MedianCut);
	// EXPECT_EQ(72, n) << "Failed with median cut.\n" << core::BufferView<RGBA>(targetBuf, n) << "\n" << core::BufferView<RGBA>(buf, lengthof(buf));

	n = core::Color::quantize(targetBuf, lengthof(targetBuf), buf, lengthof(buf), core::Color::ColorReductionType::KMeans);
	EXPECT_EQ(256, n) << "Failed with k-means.\n" << core::BufferView<RGBA>(targetBuf, n) << "\n" << core::BufferView<RGBA>(buf, lengthof(buf));
}

TEST(ColorTest, testDistanceMin) {
	const core::RGBA color1(255, 0, 0, 255);
	const core::RGBA color2(255, 0, 0, 255);
	EXPECT_FLOAT_EQ(0.0f, core::Color::getDistance(color1, color2));
	EXPECT_FLOAT_EQ(0.0f, core::Color::getDistanceApprox(color1, color2));
}

TEST(ColorTest, testDistanceMax) {
	const core::RGBA color1(0, 0, 0, 255);
	const core::RGBA color2(255, 255, 255, 255);
	EXPECT_FLOAT_EQ(0.1f, core::Color::getDistance(color1, color2));
	EXPECT_FLOAT_EQ(584970.0f, core::Color::getDistanceApprox(color1, color2));
}

}
