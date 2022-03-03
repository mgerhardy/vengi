/**
 * @file
 */

#pragma once

#include "io/FileStream.h"
#include "voxel/RawVolume.h"
#include "voxel/tests/AbstractVoxelTest.h"
#include "voxelformat/Format.h"
#include "io/Filesystem.h"
#include "core/Var.h"
#include "core/GameConfig.h"

namespace voxel {

class AbstractVoxFormatTest: public AbstractVoxelTest {
protected:
	static const voxel::Voxel Empty;

	void dump(const core::String& srcFilename, const voxel::SceneGraph &sceneGraph);
	void dump(const core::String& structName, voxel::RawVolume* v, const core::String& filename);

	void testFirstAndLastPaletteIndex(const core::String &filename, voxel::Format *format, bool includingColor,
									  bool includingRegion);
	void testFirstAndLastPaletteIndexConversion(voxel::Format &srcFormat, const core::String &destFilename,
												voxel::Format &destFormat, bool includingColor, bool includingRegion);

	void testRGB(RawVolume *volume);

	void testSaveMultipleLayers(const core::String& filename, voxel::Format* format);

	void testSave(const core::String& filename, voxel::Format* format);

	void testSaveLoadVoxel(const core::String& filename, voxel::Format* format, int mins = 0, int maxs = 1);

	void testLoadSaveAndLoad(const core::String &srcFilename, voxel::Format &srcFormat,
							 const core::String &destFilename, voxel::Format &destFormat, bool includingColor,
							 bool includingRegion);

	io::FilePtr open(const core::String& filename, io::FileMode mode = io::FileMode::Read) {
		const io::FilePtr& file = io::filesystem()->open(core::String(filename), mode);
		return file;
	}

	voxel::RawVolume* load(const core::String& filename, voxel::Format& format) {
		const io::FilePtr& file = open(filename);
		if (!file->validHandle()) {
			return nullptr;
		}
		io::FileStream stream(file);
		voxel::RawVolume* v = format.load(filename, stream);
		return v;
	}

	virtual bool onInitApp() {
		if (!AbstractVoxelTest::onInitApp()) {
			return false;
		}
		core::Var::get(cfg::VoxformatFrame, "0");
		return true;
	}
};

}
