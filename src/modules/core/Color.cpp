/**
 * @file
 */

#include "Color.h"
#include "core/Algorithm.h"
#include "core/ArrayLength.h"
#include "core/Common.h"
#include "core/GLM.h"
#include "core/Log.h"
#include "core/Pair.h"
#include "core/StringUtil.h"
#include "core/collection/Buffer.h"
#include "core/collection/DynamicArray.h"
#include "math/Octree.h"
#include <glm/ext/scalar_integer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/color_space.hpp>
#include <glm/gtx/type_aligned.hpp>

#include <SDL3/SDL.h>
#include <random>
#include <stdio.h>

namespace core {

const glm::vec4 &Color::Clear() {
	static const glm::vec4 v = glm::vec4(0.f, 0, 0, 0) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::White() {
	static const glm::vec4 v = glm::vec4(255.f, 255, 255, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Black() {
	static const glm::vec4 v = glm::vec4(0.f, 0, 0, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Lime() {
	static const glm::vec4 v = glm::vec4(109.f, 198, 2, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Pink() {
	static const glm::vec4 v = glm::vec4(248.f, 4, 62, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::LightBlue() {
	static const glm::vec4 v = glm::vec4(0.f, 153, 203, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::DarkBlue() {
	static const glm::vec4 v = glm::vec4(55.f, 116, 145, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Orange() {
	static const glm::vec4 v = glm::vec4(252.f, 167, 0, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Yellow() {
	static const glm::vec4 v = glm::vec4(255.f, 255, 0, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Sandy() {
	static const glm::vec4 v = glm::vec4(237.f, 232, 160, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::LightGray() {
	static const glm::vec4 v = glm::vec4(192.f, 192, 192, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Gray() {
	static const glm::vec4 v = glm::vec4(128.f, 128, 128, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::DarkGray() {
	static const glm::vec4 v = glm::vec4(84.f, 84, 84, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::LightRed() {
	static const glm::vec4 v = glm::vec4(255.f, 96, 96, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Red() {
	static const glm::vec4 v = glm::vec4(255.f, 0, 0, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::DarkRed() {
	static const glm::vec4 v = glm::vec4(128.f, 0, 0, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::LightGreen() {
	static const glm::vec4 v = glm::vec4(96.f, 255, 96, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Green() {
	static const glm::vec4 v = glm::vec4(0.f, 255, 0, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::DarkGreen() {
	static const glm::vec4 v = glm::vec4(0.f, 128, 0, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Blue() {
	static const glm::vec4 v = glm::vec4(0.f, 0, 255, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::SteelBlue() {
	static const glm::vec4 v = glm::vec4(35.f, 107, 142, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Olive() {
	static const glm::vec4 v = glm::vec4(128.f, 128, 0, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Purple() {
	static const glm::vec4 v = glm::vec4(128.f, 0, 128, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Cyan() {
	static const glm::vec4 v = glm::vec4(0.f, 255, 255, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::Brown() {
	static const glm::vec4 v = glm::vec4(107.f, 66, 38, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::LightBrown() {
	static const glm::vec4 v = glm::vec4(150.f, 107, 72, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const glm::vec4 &Color::DarkBrown() {
	static const glm::vec4 v = glm::vec4(82.f, 43, 26, 255) / glm::vec4(Color::magnitudef);
	return v;
}
const float Color::magnitudef = 255.0f;
const float Color::scaleFactor = 0.7f;

static constexpr const char *ColorReductionAlgorithmStr[]{"Octree", "Wu", "MedianCut", "KMeans", "NeuQuant"};
static_assert((int)core::Color::ColorReductionType::Max == lengthof(ColorReductionAlgorithmStr),
			  "Array size doesn't match with enum");

const char *Color::toColorReductionTypeString(Color::ColorReductionType type) {
	return ColorReductionAlgorithmStr[(int)type];
}

Color::ColorReductionType Color::toColorReductionType(const char *str) {
	for (int i = 0; i < lengthof(ColorReductionAlgorithmStr); ++i) {
		if (!SDL_strcmp(str, ColorReductionAlgorithmStr[i])) {
			return (Color::ColorReductionType)i;
		}
	}
	return ColorReductionType::Max;
}

struct ColorBox {
	RGBA min, max;
	core::Buffer<RGBA> pixels;
};

static int medianCutFindMedian(const core::Buffer<RGBA> &colors, int axis) {
	core::Buffer<int> values;
	for (const core::RGBA &color : colors) {
		if (axis == 0) {
			values.push_back(color.r);
		} else if (axis == 1) {
			values.push_back(color.g);
		} else {
			values.push_back(color.b);
		}
	}
	core::sort(values.begin(), values.end(), core::Less<int>());
	return values[values.size() / 2];
}

static core::Pair<ColorBox, ColorBox> medianCutSplitBox(const ColorBox &box) {
	int longestAxis = 0;
	if ((box.max.g - box.min.g) > (box.max.r - box.min.r)) {
		longestAxis = 1;
	}
	if ((box.max.b - box.min.b) > (box.max.g - box.min.g)) {
		longestAxis = 2;
	}

	int median = medianCutFindMedian(box.pixels, longestAxis);
	ColorBox box1, box2;
	for (const core::RGBA &color : box.pixels) {
		if (longestAxis == 0) {
			if (color.r < median) {
				box1.pixels.push_back(color);
			} else {
				box2.pixels.push_back(color);
			}
		} else if (longestAxis == 1) {
			if (color.g < median) {
				box1.pixels.push_back(color);
			} else {
				box2.pixels.push_back(color);
			}
		} else {
			if (color.b < median) {
				box1.pixels.push_back(color);
			} else {
				box2.pixels.push_back(color);
			}
		}
	}

	box1.min.r = box.min.r;
	box1.max.r = box.max.r;
	box1.min.g = box.min.g;
	box1.max.g = box.max.g;
	box1.min.b = box.min.b;
	box1.max.b = box.max.b;

	box2.min.r = box.min.r;
	box2.max.r = box.max.r;
	box2.min.g = box.min.g;
	box2.max.g = box.max.g;
	box2.min.b = box.min.b;
	box2.max.b = box.max.b;

	if (longestAxis == 0) {
		box1.max.r = median;
		box2.min.r = median;
	} else if (longestAxis == 1) {
		box1.max.g = median;
		box2.min.g = median;
	} else {
		box1.max.b = median;
		box2.min.b = median;
	}

	return core::Pair{box1, box2};
}

static int quantizeMedianCut(RGBA *targetBuf, size_t maxTargetBufColors, const RGBA *inputBuf, size_t inputBufColors) {
	core::Buffer<RGBA> pixels;
	pixels.append(inputBuf, inputBufColors);

	core::DynamicArray<ColorBox> boxes;
	boxes.emplace_back(ColorBox{{0, 0, 0, 255}, {255, 255, 255, 255}, pixels});

	while (boxes.size() < maxTargetBufColors) {
		size_t maxSize = 0;
		size_t maxIndex = 0;
		for (size_t i = 0; i < boxes.size(); ++i) {
			if (boxes[i].pixels.size() > maxSize) {
				maxSize = boxes[i].pixels.size();
				maxIndex = i;
			}
		}

		// Split the most populated box.
		core::Pair<ColorBox, ColorBox> boxesPair = medianCutSplitBox(boxes[maxIndex]);
		boxes.erase(maxIndex);
		boxes.push_back(boxesPair.first);
		boxes.push_back(boxesPair.second);
	}

	size_t n = 0;
	for (const ColorBox &box : boxes) {
		if (box.pixels.empty()) {
			continue;
		}
		uint32_t r = 0, g = 0, b = 0, a = 0;
		for (const core::RGBA &color : box.pixels) {
			r += color.r;
			g += color.g;
			b += color.b;
			a += color.a;
		}
		r /= box.pixels.size();
		g /= box.pixels.size();
		b /= box.pixels.size();
		a /= box.pixels.size();
		targetBuf[n++] = core::RGBA(r, g, b, a);
		if (n >= maxTargetBufColors) {
			return (int)n;
		}
	}
	for (size_t i = n; i < maxTargetBufColors; ++i) {
		targetBuf[i] = RGBA(0xFFFFFFFFU);
	}
	return (int)n;
}

static int quantizeOctree(RGBA *targetBuf, size_t maxTargetBufColors, const RGBA *inputBuf, size_t inputBufColors) {
	core_assert(glm::isPowerOfTwo(maxTargetBufColors));
	using BBox = math::AABB<uint8_t>;
	struct ColorNode {
		inline ColorNode(core::RGBA c) : color(c){};
		core::RGBA color;
		inline BBox aabb() const {
			return BBox(color.r, color.g, color.b, color.r + 1, color.g + 1, color.b + 1);
		}
	};
	const BBox aabb(0, 0, 0, 255, 255, 255);
	using Tree = math::Octree<ColorNode, uint8_t>;
	Tree octree(aabb, 32);
	for (size_t i = 0; i < inputBufColors; ++i) {
		octree.insert(inputBuf[i]);
	}
	size_t n = 0;
	const glm::ivec3 dim(8);
	const int rmax = aabb.getWidthX() + 1 - dim.r;
	const int gmax = aabb.getWidthY() + 1 - dim.g;
	const int bmax = aabb.getWidthZ() + 1 - dim.b;
	for (int r = 0; r <= rmax; r += dim.r) {
		for (int g = 0; g <= gmax; g += dim.g) {
			for (int b = 0; b <= bmax; b += dim.b) {
				Tree::Contents contents;
				const BBox queryAABB(r, g, b, r + dim.r - 1, g + dim.g - 1, b + dim.b - 1);
				octree.query(queryAABB, contents);
				const int k = (int)contents.size();
				if (k == 0) {
					continue;
				}
				targetBuf[n++] = contents.front().color;
				if (n >= maxTargetBufColors) {
					return (int)n;
				}
			}
		}
	}
	for (size_t i = n; i < maxTargetBufColors; ++i) {
		targetBuf[i] = RGBA(0xFFFFFFFFU);
	}
	return (int)n;
}

static inline float getDistance(const glm::vec4 &p1, const glm::vec4 &p2) {
	return glm::length(p1 - p2);
}

static int quantizeKMeans(RGBA *targetBuf, size_t maxTargetBufColors, const RGBA *inputBuf, size_t inputBufColors) {
	core::DynamicArray<glm::vec4> centers;
	centers.resize(maxTargetBufColors);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, (int)inputBufColors - 1);
	for (int i = 0; i < (int)maxTargetBufColors; i++) {
		centers[i] = core::Color::fromRGBA(inputBuf[dis(gen)]);
	}

	bool changed = true;
	while (changed) {
		changed = false;
		core::DynamicArray<core::DynamicArray<glm::vec4>> clusters(maxTargetBufColors);
		for (size_t i = 0; i < inputBufColors; ++i) {
			const glm::vec4 point = core::Color::fromRGBA(inputBuf[i]);
			int closest = 0;
			float closestDistance = getDistance(point, centers[0]);
			for (int n = 1; n < (int)maxTargetBufColors; n++) {
				float d = getDistance(point, centers[n]);
				if (d < closestDistance) {
					closest = n;
					closestDistance = d;
				}
			}
			clusters[closest].push_back(point);
		}
		for (int i = 0; i < (int)maxTargetBufColors; i++) {
			if (clusters[i].empty()) {
				continue;
			}
			glm::vec4 newCenter(0.0f);
			for (const glm::vec4 &point : clusters[i]) {
				newCenter += point;
			}
			newCenter /= clusters[i].size();
			if (getDistance(newCenter, centers[i]) > 0.0001f) {
				centers[i] = newCenter;
				changed = true;
			}
		}
	}

	size_t n = 0;
	for (const glm::vec4 &c : centers) {
		targetBuf[n++] = core::Color::getRGBA(c);
	}
	for (size_t i = n; i < maxTargetBufColors; ++i) {
		targetBuf[i] = RGBA(0xFFFFFFFFU);
	}
	return (int)n;
}

// Based on NeuQuant algorithm from jo_gif_quantize
static int quantizeNeuQuant(RGBA *targetBuf, size_t maxTargetBufColors, const RGBA *inputBuf, size_t inputBufColors) {
	const int numColors = (int)maxTargetBufColors;
	const int rgbaSize = (int)inputBufColors;
	int sample = 1;

	// defs for freq and bias
	const int intbiasshift = 16; /* bias for fractions */
	const int intbias = (((int)1) << intbiasshift);
	const int gammashift = 10; /* gamma = 1024 */
	const int betashift = 10;
	const int beta = (intbias >> betashift); /* beta = 1/1024 */
	const int betagamma = (intbias << (gammashift - betashift));

	// defs for decreasing radius factor
	const int radiusbiasshift = 6; /* at 32.0 biased by 6 bits */
	const int radiusbias = (((int)1) << radiusbiasshift);
	const int radiusdec = 30; /* factor of 1/30 each cycle */

	// defs for decreasing alpha factor
	const int alphabiasshift = 10; /* alpha starts at 1.0 */
	const int initalpha = (((int)1) << alphabiasshift);

	// radbias and alpharadbias used for radpower calculation
	const int radbiasshift = 8;
	const int radbias = (((int)1) << radbiasshift);
	const int alpharadbshift = (alphabiasshift + radbiasshift);
	const int alpharadbias = (((int)1) << alpharadbshift);

	sample = glm::clamp(sample, 1, 30);
	int network[256][3];
	int bias[256] = {}, freq[256];
	for (int i = 0; i < numColors; ++i) {
		// Put nurons evenly through the luminance spectrum.
		network[i][0] = network[i][1] = network[i][2] = (i << 12) / numColors;
		freq[i] = intbias / numColors;
	}
	// Learn
	{
		const int primes[5] = {499, 491, 487, 503};
		int step = 4;
		for (int i = 0; i < 4; ++i) {
			if (rgbaSize > primes[i] * 4 && (rgbaSize % primes[i])) { // TODO/Error? primes[i]*4?
				step = primes[i] * 4;
			}
		}
		sample = step == 4 ? 1 : sample;

		int alphadec = 30 + ((sample - 1) / 3);
		int samplepixels = rgbaSize / (4 * sample);
		int delta = samplepixels / 100;
		int alpha = initalpha;
		delta = delta == 0 ? 1 : delta;

		int radius = (numColors >> 3) * radiusbias;
		int rad = radius >> radiusbiasshift;
		rad = rad <= 1 ? 0 : rad;
		int radSq = rad * rad;
		int radpower[32];
		for (int i = 0; i < rad; i++) {
			radpower[i] = alpha * (((radSq - i * i) * radbias) / radSq);
		}

		// Randomly walk through the pixels and relax neurons to the "optimal" target.
		for (int i = 0, pix = 0; i < samplepixels;) {
			int r = inputBuf[pix].r << 4;
			int g = inputBuf[pix].g << 4;
			int b = inputBuf[pix].b << 4;
			int j = -1;
			{
				// finds closest neuron (min dist) and updates freq
				// finds best neuron (min dist-bias) and returns position
				// for frequently chosen neurons, freq[k] is high and bias[k] is negative
				// bias[k] = gamma*((1/numColors)-freq[k])

				int bestd = 0x7FFFFFFF, bestbiasd = 0x7FFFFFFF, bestpos = -1;
				for (int k = 0; k < numColors; k++) {
					int *n = network[k];
					int dist = abs(n[0] - r) + abs(n[1] - g) + abs(n[2] - b);
					if (dist < bestd) {
						bestd = dist;
						bestpos = k;
					}
					int biasdist = dist - ((bias[k]) >> (intbiasshift - 4));
					if (biasdist < bestbiasd) {
						bestbiasd = biasdist;
						j = k;
					}
					int betafreq = freq[k] >> betashift;
					freq[k] -= betafreq;
					bias[k] += betafreq << gammashift;
				}
				freq[bestpos] += beta;
				bias[bestpos] -= betagamma;
			}

			// Move neuron j towards biased (b,g,r) by factor alpha
			network[j][0] -= (network[j][0] - r) * alpha / initalpha;
			network[j][1] -= (network[j][1] - g) * alpha / initalpha;
			network[j][2] -= (network[j][2] - b) * alpha / initalpha;
			if (rad != 0) {
				// Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
				int lo = j - rad;
				lo = lo < -1 ? -1 : lo;
				int hi = j + rad;
				hi = hi > numColors ? numColors : hi;
				for (int jj = j + 1, m = 1; jj < hi; ++jj) {
					int a = radpower[m++];
					network[jj][0] -= (network[jj][0] - r) * a / alpharadbias;
					network[jj][1] -= (network[jj][1] - g) * a / alpharadbias;
					network[jj][2] -= (network[jj][2] - b) * a / alpharadbias;
				}
				for (int k = j - 1, m = 1; k > lo; --k) {
					int a = radpower[m++];
					network[k][0] -= (network[k][0] - r) * a / alpharadbias;
					network[k][1] -= (network[k][1] - g) * a / alpharadbias;
					network[k][2] -= (network[k][2] - b) * a / alpharadbias;
				}
			}

			pix += step;
			pix = pix >= rgbaSize ? pix - rgbaSize : pix;

			// every 1% of the image, move less over the following iterations.
			if (++i % delta == 0) {
				alpha -= alpha / alphadec;
				radius -= radius / radiusdec;
				rad = radius >> radiusbiasshift;
				rad = rad <= 1 ? 0 : rad;
				radSq = rad * rad;
				for (j = 0; j < rad; j++) {
					radpower[j] = alpha * ((radSq - j * j) * radbias / radSq);
				}
			}
		}
	}
	// Unbias network to give byte values 0..255
	for (int i = 0; i < numColors; i++) {
		targetBuf[i].r = network[i][0] >>= 4;
		targetBuf[i].g = network[i][1] >>= 4;
		targetBuf[i].b = network[i][2] >>= 4;
		targetBuf[i].a = 255;
	}
	return numColors;
}

static int quantizeWu(RGBA *targetBuf, size_t maxTargetBufColors, const RGBA *inputBuf, size_t inputBufColors) {
	core::Buffer<RGBA> pixels;
	pixels.append(inputBuf, inputBufColors);

	// Initialize the set of boxes with the full color range
	core::DynamicArray<ColorBox> boxes;
	boxes.emplace_back(ColorBox{{0, 0, 0, 255}, {255, 255, 255, 255}, pixels});

	// Iterate until we reach the desired number of boxes
	while (boxes.size() < maxTargetBufColors) {
		// Find the box with the largest volume (i.e., the most pixels)
		int maxVolume = std::numeric_limits<int>::min();
		size_t maxVolumeIndex = 0;
		for (size_t i = 0; i < boxes.size(); ++i) {
			int volume = (boxes[i].max.r - boxes[i].min.r + 1) * (boxes[i].max.g - boxes[i].min.g + 1) *
						 (boxes[i].max.b - boxes[i].min.b + 1);
			if (volume > maxVolume) {
				maxVolume = volume;
				maxVolumeIndex = i;
			}
		}

		// Split the box with the largest volume into two boxes along its longest dimension
		const ColorBox &box = boxes[maxVolumeIndex];
		const size_t pixelCount = box.pixels.size();
		if (pixelCount <= 0) {
			boxes.erase(maxVolumeIndex);
			continue;
		}

		int component = 0;
		int midpoint = 0;
		if (box.max.r - box.min.r > box.max.g - box.min.g && box.max.r - box.min.r > box.max.b - box.min.b) {
			component = 0;
			midpoint = (box.min.r + box.max.r) / 2;
		} else if (box.max.g - box.min.g > box.max.b - box.min.b) {
			component = 1;
			midpoint = (box.min.g + box.max.g) / 2;
		} else {
			component = 2;
			midpoint = (box.min.b + box.max.b) / 2;
		}

		ColorBox box1, box2;
		box1.pixels.reserve(pixelCount / 2);
		box2.pixels.reserve(pixelCount / 2);
		switch (component) {
		case 0:
			box1.min = box.min;
			box1.max = RGBA(midpoint, box.max.g, box.max.b, 255);
			box2.min = RGBA(midpoint + 1, box.min.g, box.min.b, 255);
			box2.max = box.max;
			for (const RGBA &pixel : box.pixels) {
				if (pixel.r <= midpoint) {
					box1.pixels.push_back(pixel);
				} else {
					box2.pixels.push_back(pixel);
				}
			}
			break;
		case 1:
			box1.min = box.min;
			box1.max = RGBA(box.max.r, midpoint, box.max.b, 255);
			box2.min = RGBA(box.min.r, midpoint + 1, box.min.b, 255);
			box2.max = box.max;
			for (const RGBA &pixel : box.pixels) {
				if (pixel.g <= midpoint) {
					box1.pixels.push_back(pixel);
				} else {
					box2.pixels.push_back(pixel);
				}
			}
			break;
		case 2:
			box1.min = box.min;
			box1.max = RGBA(box.max.r, box.max.g, midpoint);
			box2.min = RGBA(box.min.r, box.min.g, midpoint + 1);
			box2.max = box.max;
			for (const RGBA &pixel : box.pixels) {
				if (pixel.b <= midpoint) {
					box1.pixels.push_back(pixel);
				} else {
					box2.pixels.push_back(pixel);
				}
			}
			break;
		}

		// Replace the original box with the two split boxes
		boxes.erase(maxVolumeIndex);
		boxes.emplace_back(core::move(box1));
		boxes.emplace_back(core::move(box2));
	}

	size_t n = 0;
	for (const ColorBox &box : boxes) {
		if (box.pixels.empty()) {
			continue;
		}
		RGBA average(0, 0, 0, 255);
		for (const RGBA &pixel : box.pixels) {
			average.r += pixel.r;
			average.g += pixel.g;
			average.b += pixel.b;
		}
		average.r /= box.pixels.size();
		average.g /= box.pixels.size();
		average.b /= box.pixels.size();
		targetBuf[n++] = average;
	}

	for (size_t i = n; i < maxTargetBufColors; ++i) {
		targetBuf[i] = RGBA(0xFFFFFFFFU);
	}
	return (int)n;
}

int Color::quantize(RGBA *targetBuf, size_t maxTargetBufColors, const RGBA *inputBuf, size_t inputBufColors,
					ColorReductionType type) {
	if (inputBufColors <= maxTargetBufColors) {
		size_t n;
		for (n = 0; n < inputBufColors; ++n) {
			targetBuf[n] = inputBuf[n];
		}
		for (size_t i = n; i < maxTargetBufColors; ++i) {
			targetBuf[i] = RGBA(255, 255, 255, 255);
		}
		return (int)n;
	}
	switch (type) {
	case ColorReductionType::Wu:
		return quantizeWu(targetBuf, maxTargetBufColors, inputBuf, inputBufColors);
	case ColorReductionType::KMeans:
		return quantizeKMeans(targetBuf, maxTargetBufColors, inputBuf, inputBufColors);
	case ColorReductionType::NeuQuant:
		return quantizeNeuQuant(targetBuf, maxTargetBufColors, inputBuf, inputBufColors);
	case ColorReductionType::Octree:
		return quantizeOctree(targetBuf, maxTargetBufColors, inputBuf, inputBufColors);
	case ColorReductionType::MedianCut:
		return quantizeMedianCut(targetBuf, maxTargetBufColors, inputBuf, inputBufColors);
	default:
		break;
	}
	return -1;
}

glm::vec4 Color::fromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return glm::aligned_vec4(r, g, b, a) / Color::magnitudef;
}

core::RGBA Color::fromHSB(const float hue, const float saturation, const float brightness, const float alpha) {
	if (0.00001f > brightness) {
		return core::RGBA(0, 0, 0, alpha * 255.0f);
	}
	if (0.00001f > saturation) {
		return core::RGBA(brightness * 255.0f, brightness * 255.0f, brightness * 255.0f, alpha * 255.0f);
	}
	const float h = (hue - (float)SDL_floor(hue)) * 6.f;
	const float f = h - (float)SDL_floor(h);
	const uint8_t p = (uint8_t)(brightness * (1.f - saturation) * 255.0f);
	const uint8_t q = (uint8_t)(brightness * (1.f - saturation * f) * 255.0f);
	const uint8_t t = (uint8_t)(brightness * (1.f - (saturation * (1.f - f))) * 255.0f);
	core::RGBA color;
	color.a = alpha * 255.0f;
	switch (static_cast<int>(h)) {
	case 0:
		color.r = brightness;
		color.g = t;
		color.b = p;
		break;
	case 1:
		color.r = q;
		color.g = brightness;
		color.b = p;
		break;
	case 2:
		color.r = p;
		color.g = brightness;
		color.b = t;
		break;
	case 3:
		color.r = p;
		color.g = q;
		color.b = brightness;
		break;
	case 4:
		color.r = t;
		color.g = p;
		color.b = brightness;
		break;
	case 5:
		color.r = brightness;
		color.g = p;
		color.b = q;
		break;
	}
	return color;
}

core::String Color::toHex(const RGBA rgba, bool hashPrefix) {
	core::String hex;
	if (hashPrefix) {
		hex.append("#");
	}
	hex.append(core::string::format("%02x%02x%02x%02x", rgba.r, rgba.g, rgba.b, rgba.a));
	return hex;
}

core::RGBA Color::fromHex(const char *hex) {
	uint32_t r = 0x00;
	uint32_t g = 0x00;
	uint32_t b = 0x00;
	uint32_t a = 0xff;
	if (0 == SDL_strncmp(hex, "0x", 2) || 0 == SDL_strncmp(hex, "0X", 2)) {
		hex += 2;
	} else if (hex[0] == '#') {
		hex += 1;
	}
	if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) == 3) {
		a = 0xff;
	}
	return core::RGBA(r, g, b, a);
}

core::String Color::print(RGBA rgba, bool colorAsHex) {
	String buf = "\033[0m";
	if (colorAsHex) {
		buf = toHex(rgba);
		buf.append(" ");
	}
	if (rgba.a != 0) {
		buf.append("\033[38;2;");
		buf.append(rgba.r).append(";");
		buf.append(rgba.g).append(";");
		buf.append(rgba.b).append("m");
	}
	buf.append("\033[48;2;");
	buf.append(rgba.r).append(";");
	buf.append(rgba.g).append(";");
	buf.append(rgba.b).append("m");
	buf.append(u8"\u2587");
	buf.append("\033[0m");
	return buf;
}

// https://www.compuphase.com/cmetric.htm
static float getDistanceApprox(core::RGBA rgba, core::RGBA rgba2) {
	const int rmean = (rgba2.r + rgba.r) / 2;
	const int r = rgba2.r - rgba.r;
	const int g = rgba2.g - rgba.g;
	const int b = rgba2.b - rgba.b;
	return (float)(((512 + rmean) * r * r) >> 8) + 4.0f * g * g + (float)(((767 - rmean) * b * b) >> 8);
}

static float getDistanceHSB(const core::RGBA &rgba, float hue, float saturation, float brightness) {
	float chue;
	float csaturation;
	float cbrightness;
	core::Color::getHSB(rgba, chue, csaturation, cbrightness);

	const float weightHue = 0.8f;
	const float weightSaturation = 0.1f;
	const float weightValue = 0.1f;
	const float dH = chue - hue;
	const float dS = csaturation - saturation;
	const float dV = cbrightness - brightness;
	return weightHue * dH * dH + weightValue * dV * dV + weightSaturation * dS * dS;
}

static float getDistanceHSB(const core::RGBA &rgba, RGBA rgba2) {
	float hue;
	float saturation;
	float brightness;
	core::Color::getHSB(core::Color::fromRGBA(rgba), hue, saturation, brightness);
	return getDistanceHSB(rgba2, hue, saturation, brightness);
}

float Color::getDistance(RGBA rgba, RGBA rgba2, Distance d) {
	if (rgba == rgba2) {
		return 0.0f;
	}
	if (d == Distance::Approximation) {
		return getDistanceApprox(rgba, rgba2);
	}
	return getDistanceHSB(rgba, rgba2);
}

float Color::getDistance(RGBA rgba, float hue, float saturation, float brightness) {
	return getDistanceHSB(rgba, hue, saturation, brightness);
}

core::RGBA Color::flattenRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint8_t f) {
	if (f <= 1u) {
		return core::RGBA(r, g, b, a);
	}
	return core::RGBA(r / f * f, g / f * f, b / f * f, a);
}

void Color::getCIELab(const glm::vec4 &color, float &L, float &a, float &b) {
	float red, green, blue;
	if (color.r > 0.04045f) {
		red = glm::pow(((color.r + 0.055f) / 1.055f), 2.4f);
	} else {
		red = color.r / 12.92f;
	}

	if (color.g > 0.04045f) {
		green = glm::pow(((color.g + 0.055f) / 1.055f), 2.4f);
	} else {
		green = color.g / 12.92f;
	}

	if (color.b > 0.04045f) {
		blue = glm::pow(((color.b + 0.055f) / 1.055f), 2.4f);
	} else {
		blue = color.b / 12.92f;
	}

	red = red * 100.0f;
	green = green * 100.0f;
	blue = blue * 100.0f;

	// XYZ color space
	const float x = red * 0.4124f + green * 0.3576f + blue * 0.1805f;
	const float y = red * 0.2126f + green * 0.7152f + blue * 0.0722f;
	const float z = red * 0.0193f + green * 0.1192f + blue * 0.9505f;

	// standard illuminant D65
	float fx = x / 95.047f;
	float fy = y / 100.0f;
	float fz = z / 108.883f;

	if (fx > 0.008856f) {
		fx = glm::pow(fx, 1.0f / 3.0f);
	} else {
		fx = (7.787f * fx) + (4.0f / 29.0f);
	}

	if (fy > 0.008856f) {
		fy = glm::pow(fy, 1.0f / 3.0f);
	} else {
		fy = (7.787f * fy) + (4.0f / 29.0f);
	}

	if (fz > 0.008856f) {
		fz = glm::pow(fz, 1.0f / 3.0f);
	} else {
		fz = (7.787f * fz) + (4.0f / 29.0f);
	}

	L = 116.0f * fy - 16.0f;
	a = 500.0f * (fx - fy);
	b = 200.0f * (fy - fz);
}

RGBA Color::getRGBA(const glm::vec4 &color) {
	return RGBA{(uint8_t)(color.r * magnitude), (uint8_t)(color.g * magnitude), (uint8_t)(color.b * magnitude),
				(uint8_t)(color.a * magnitude)};
}

void Color::getHSB(core::RGBA color, float &chue, float &csaturation, float &cbrightness) {
	getHSB(fromRGBA(color), chue, csaturation, cbrightness);
}

void Color::getHSB(const glm::vec4 &color, float &chue, float &csaturation, float &cbrightness) {
	cbrightness = brightness(color);
	const float minBrightness = core_min(color.r, core_min(color.g, color.b));
	const float delta = cbrightness - minBrightness;
	if (SDL_fabs(delta) < 0.00001f) {
		chue = 0.f;
		csaturation = 0.f;
		return;
	}
	const float r = (cbrightness - color.r) / delta;
	const float g = (cbrightness - color.g) / delta;
	const float b = (cbrightness - color.b) / delta;
	if (SDL_fabs(color.r - cbrightness) < 0.00001f) {
		chue = b - g;
	} else if (SDL_fabs(color.g - cbrightness) < 0.00001f) {
		chue = 2.f + r - b;
	} else {
		chue = 4.f + g - r;
	}
	chue /= 6.f;
	if (chue < 0.f) {
		chue += 1.f;
	}
	csaturation = delta / cbrightness;
}

glm::vec4 Color::alpha(const glm::vec4 &c, float alpha) {
	return glm::vec4(c.r, c.g, c.b, alpha);
}

RGBA Color::alpha(const RGBA rgba, uint8_t alpha) {
	return RGBA(rgba.r, rgba.g, rgba.b, alpha);
}

float Color::brightness(const glm::vec4 &color) {
	return core_max(color.r, core_max(color.g, color.b));
}

uint8_t Color::brightness(const core::RGBA &color) {
	return core_max(color.r, core_max(color.g, color.b));
}

float Color::intensity(const glm::vec4 &color) {
	return (color.r + color.g + color.b) / 3.f;
}

glm::vec4 Color::gray(const glm::vec4 &color) {
	const float gray = (0.21f * color.r + 0.72f * color.g + 0.07f * color.b) / 3.0f;
	return glm::vec4(gray, gray, gray, color.a);
}

glm::vec3 Color::gray(const glm::vec3 &color) {
	const float gray = (0.21f * color.r + 0.72f * color.g + 0.07f * color.b) / 3.0f;
	return glm::vec3(gray, gray, gray);
}

core::RGBA Color::darker(const core::RGBA &color, float f) {
	return getRGBA(darker(fromRGBA(color), f));
}

glm::vec4 Color::darker(const glm::vec4 &color, float f) {
	f = (float)SDL_pow(scaleFactor, f);
	return glm::vec4(glm::clamp(glm::vec3(color) * f, 0.0f, 1.0f), color.a);
}

core::RGBA Color::brighter(const core::RGBA color, float f) {
	return getRGBA(brighter(fromRGBA(color), f));
}

glm::vec4 Color::brighter(const glm::vec4 &color, float f) {
	static float min = 21.f / magnitude;
	glm::vec3 result = glm::vec3(color);
	f = (float)SDL_pow(scaleFactor, f);
	if (glm::all(glm::epsilonEqual(glm::zero<glm::vec3>(), result, 0.00001f))) {
		return glm::vec4(min / f, min / f, min / f, color.a);
	}
	if (result.r > 0.f && result.r < min) {
		result.r = min;
	}
	if (result.g > 0.f && result.g < min) {
		result.g = min;
	}
	if (result.b > 0.f && result.b < min) {
		result.b = min;
	}
	return glm::vec4(glm::clamp(result / f, 0.f, 1.f), color.a);
}

} // namespace core
