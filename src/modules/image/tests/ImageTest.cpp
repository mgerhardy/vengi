/**
 * @file
 */

#include "app/tests/AbstractTest.h"
#include "image/Image.h"
#include "math/tests/TestMathHelper.h"
#include "core/tests/TestColorHelper.h"
#include "io/BufferedReadWriteStream.h"
#include <glm/vec2.hpp>
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/string_cast.hpp>

namespace glm {
::std::ostream &operator<<(::std::ostream &os, const vec2 &v) {
	os << to_string(v);
	return os;
}
::std::ostream &operator<<(::std::ostream &os, const ivec2 &v) {
	os << to_string(v);
	return os;
}
}

namespace image {

class ImageTest : public app::AbstractTest {
private:
	static constexpr core::RGBA r = core::RGBA(255, 0, 0);
	static constexpr core::RGBA b = core::RGBA(0, 0, 0);
protected:
	static constexpr core::RGBA img1[]{
		r, r, b, b, b, b,
		r, r, b, b, b, b,
		b, b, b, b, b, b,
		b, b, b, b, b, b,
		b, b, b, b, b, b,
		b, b, b, b, b, b,
	};
	static constexpr core::RGBA img2[]{
		b, b, b, b, b, b,
		b, b, b, b, b, b,
		r, r, b, b, b, b,
		r, r, b, b, b, b,
		b, b, b, b, b, b,
		b, b, b, b, b, b,
	};
	static constexpr core::RGBA img3[]{
		b, b, b, b, b, b,
		b, b, b, b, b, b,
		b, b, b, b, b, b,
		b, b, b, b, b, b,
		b, b, r, r, b, b,
		b, b, r, r, b, b,
	};
};

TEST_F(ImageTest, testWriteJPEG) {
	io::BufferedReadWriteStream stream1;
	ASSERT_TRUE(image::Image::writeJPEG(stream1, (const uint8_t*)img1, 6, 6, 4));
	stream1.seek(0);
	ASSERT_TRUE(image::createEmptyImage("image")->load(stream1, (int)stream1.size()));

	io::BufferedReadWriteStream stream2;
	ASSERT_TRUE(image::Image::writeJPEG(stream2, (const uint8_t*)img2, 6, 6, 4));
	stream2.seek(0);
	ASSERT_TRUE(image::createEmptyImage("image")->load(stream2, (int)stream1.size()));

	io::BufferedReadWriteStream stream3;
	ASSERT_TRUE(image::Image::writeJPEG(stream3, (const uint8_t*)img3, 6, 6, 4));
	stream3.seek(0);
	ASSERT_TRUE(image::createEmptyImage("image")->load(stream3, (int)stream1.size()));
}

TEST_F(ImageTest, testWritePng) {
	io::BufferedReadWriteStream stream1;
	ASSERT_TRUE(image::Image::writePng(stream1, (const uint8_t*)img1, 6, 6, 4));
	stream1.seek(0);
	ASSERT_TRUE(image::createEmptyImage("image")->load(stream1, (int)stream1.size()));

	io::BufferedReadWriteStream stream2;
	ASSERT_TRUE(image::Image::writePng(stream2, (const uint8_t*)img2, 6, 6, 4));
	stream2.seek(0);
	ASSERT_TRUE(image::createEmptyImage("image")->load(stream2, (int)stream1.size()));

	io::BufferedReadWriteStream stream3;
	ASSERT_TRUE(image::Image::writePng(stream3, (const uint8_t*)img3, 6, 6, 4));
	stream3.seek(0);
	ASSERT_TRUE(image::createEmptyImage("image")->load(stream3, (int)stream1.size()));
}

TEST_F(ImageTest, testGet) {
	const image::ImagePtr& img = image::loadImage("test-palette-in.png");
	const core::RGBA rgba = img->colorAt(33, 7);
	const core::RGBA expected(243, 238, 236);
	EXPECT_EQ(rgba, expected) << image::print(img);
}

TEST_F(ImageTest, testUV) {
	EXPECT_VEC_NEAR(glm::vec2(0.001953f, 0.5f), image::Image::uv(0, 0, 256, 1), 0.000001f) << "lower left corner of the image";
	EXPECT_VEC_NEAR(glm::vec2(0.75, 0.25f), image::Image::uv(1, 1, 2, 2), 0.000001f) << "lower left corner of the image";
}

TEST_F(ImageTest, testUVPixelBoundaries) {
	const image::ImagePtr& img = image::loadImage("test-palette-in.png");
	EXPECT_VEC_NEAR(glm::vec2(0.0119048f, 0.0067568f), img->uv(0, img->height() - 1), 0.000001f) << "lower left corner of the image (w: " << img->width() << ", h: " << img->height() << ")";
	EXPECT_VEC_NEAR(glm::vec2(0.9880952f, 0.0067568f), img->uv(img->width() - 1, img->height() - 1), 0.000001f) << "lower right corner of the image (w: " << img->width() << ", h: " << img->height() << ")";
	EXPECT_VEC_NEAR(glm::vec2(0.9880952f, 0.9932432f), img->uv(img->width() - 1, 0), 0.000001f) << "upper right corner of the image (w: " << img->width() << ", h: " << img->height() << ")";
	EXPECT_VEC_NEAR(glm::vec2(0.0119048f, 0.9932432f), img->uv(0, 0), 0.000001f) << "upper left corner of the image (w: " << img->width() << ", h: " << img->height() << ")";
}

TEST_F(ImageTest, testUVPixelConversion) {
	const image::ImagePtr& img = image::loadImage("test-palette-in.png");
	for (int x = 0; x < img->width(); ++x) {
		for (int y = 0; y < img->height(); ++y) {
			const glm::vec2 &uv = img->uv(x, y);
			const glm::ivec2 &pixels = img->pixels(uv);
			ASSERT_EQ(x, pixels.x) << "Failed to convert " << x << ":" << y << " to uv and back to pixels (uv: " << uv
								   << ", pixels: " << pixels << ") image: (w: " << img->width() << ", h: " << img->height() << ")";
			ASSERT_EQ(y, pixels.y) << "Failed to convert " << x << ":" << y << " to uv and back to pixels (uv: " << uv
								   << ", pixels: " << pixels << ") image: (w: " << img->width() << ", h: " << img->height() << ")";
		}
	}
}

}
