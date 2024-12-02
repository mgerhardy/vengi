/**
 * @file
 */

#include "MeshState.h"
#include "app/App.h"
#include "core/Log.h"
#include "palette/NormalPalette.h"
#include "voxel/MaterialColor.h"
#include "voxel/Mesh.h"
#include "voxel/SurfaceExtractor.h"

namespace voxel {

bool MeshState::init() {
	_meshMode = core::Var::getSafe(cfg::VoxelMeshMode);
	_meshMode->markClean();

	_threadPool.init();
	Log::debug("Threadpool size: %i", (int)_threadPool.size());
	return true;
}

void MeshState::construct() {
	_meshSize = core::Var::get(cfg::VoxelMeshSize, "62", core::CV_READONLY);
}

glm::vec3 MeshState::VolumeData::centerPos() const {
	const glm::vec4 center((_mins + _maxs) * 0.5f, 1.0f);
	const glm::vec3 pos = _model * center;
	return pos;
}

glm::vec3 MeshState::VolumeData::centerPos(int x, int y, int z) const {
	// we want the center of the voxel
	const glm::vec4 center((float)x + 0.5f - _pivot.x, (float)y + 0.5f  - _pivot.y, (float)z + 0.5f  - _pivot.z, 0.0f);
	const glm::vec3 pos = _model * center;
	return pos;
}

const glm::vec3 &MeshState::mins(int idx) const {
	return _volumeData[idx]._mins;
}

const glm::vec3 &MeshState::maxs(int idx) const {
	return _volumeData[idx]._maxs;
}

glm::vec3 MeshState::centerPos(int idx) const {
	return _volumeData[idx].centerPos();
}

glm::vec3 MeshState::centerPos(int idx, int x, int y, int z) const {
	return _volumeData[idx].centerPos(x, y, z);
}

const glm::mat4 &MeshState::model(int idx) const {
	return _volumeData[idx]._model;
}

const glm::vec3 &MeshState::pivot(int idx) const {
	return _volumeData[idx]._pivot;
}

void MeshState::setModel(int idx, const glm::mat4 &model) {
	_volumeData[idx]._model = model;
}

bool MeshState::setModelMatrix(int idx, const glm::mat4 &model, const glm::vec3 &pivot, const glm::vec3 &mins,
									   const glm::vec3 &maxs) {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		Log::error("Given id %i is out of bounds", idx);
		return false;
	}
	if (reference(idx) == -1 && volume(idx) == nullptr) {
		Log::error("No volume found at: %i", idx);
		return false;
	}
	VolumeData &state = _volumeData[idx];
	state._model = model;
	state._pivot = pivot;
	state._mins = mins;
	state._maxs = maxs;
	return true;
}

void MeshState::clear() {
	for (int i = 0; i < MeshType_Max; ++i) {
		for (const auto &iter : _meshes[i]) {
			for (voxel::Mesh *mesh : iter->value) {
				delete mesh;
			}
		}
		_meshes[i].clear();
	}
}

void MeshState::addOrReplaceMeshes(MeshState::ExtractionCtx &result, MeshType type) {
	auto iter = _meshes[type].find(result.mins);
	if (iter != _meshes[type].end()) {
		delete iter->value[result.idx];
		iter->value[result.idx] = new voxel::Mesh(core::move(result.mesh.mesh[type]));
		return;
	}
	_meshes[type].emplace(result.mins, Meshes());
	auto newIter = _meshes[type].find(result.mins);
	newIter->value[result.idx] = new voxel::Mesh(core::move(result.mesh.mesh[type]));
}

int MeshState::pop() {
	MeshState::ExtractionCtx result;
	while (_pendingQueue.pop(result)) {
		if (_volumeData[result.idx]._rawVolume == nullptr) {
			continue;
		}
		addOrReplaceMeshes(result, MeshType_Opaque);
		addOrReplaceMeshes(result, MeshType_Transparency);
		return result.idx;
	}
	return -1;
}

bool MeshState::deleteMeshes(const glm::ivec3 &pos, int idx) {
	bool d = false;
	for (int i = 0; i < MeshType_Max; ++i) {
		auto &meshes = _meshes[i];
		auto iter = meshes.find(pos);
		if (iter != meshes.end()) {
			MeshState::Meshes &array = iter->value;
			voxel::Mesh *mesh = array[idx];
			delete mesh;
			array[idx] = nullptr;
			d = true;
		}
	}
	return d;
}

bool MeshState::deleteMeshes(int idx) {
	bool d = false;
	for (int i = 0; i < MeshType_Max; ++i) {
		auto &meshes = _meshes[i];
		for (const auto &iter : meshes) {
			MeshState::Meshes &array = iter->value;
			voxel::Mesh *mesh = array[idx];
			delete mesh;
			array[idx] = nullptr;
			d = true;
		}
	}
	return d;
}

const MeshState::MeshesMap &MeshState::meshes(MeshType type) const {
	return _meshes[type];
}

void MeshState::count(MeshType meshType, int idx, size_t &vertCount, size_t &normalsCount, size_t &indCount) const {
	for (const auto &i : _meshes[meshType]) {
		const MeshState::Meshes &meshes = i->value;
		const voxel::Mesh *mesh = meshes[idx];
		if (mesh == nullptr || mesh->getNoOfIndices() <= 0) {
			continue;
		}
		const voxel::VertexArray &vertexVector = mesh->getVertexVector();
		const voxel::NormalArray &normalVector = mesh->getNormalVector();
		const voxel::IndexArray &indexVector = mesh->getIndexVector();
		vertCount += vertexVector.size();
		normalsCount += normalVector.size();
		indCount += indexVector.size();
	}
}

const palette::Palette &MeshState::palette(int idx) const {
	if (idx < 0 || idx > MAX_VOLUMES || !_volumeData[idx]._palette.hasValue()) {
		return voxel::getPalette();
	}
	return *_volumeData[idx]._palette.value();
}

const palette::NormalPalette &MeshState::normalsPalette(int idx) const {
	if (idx < 0 || idx > MAX_VOLUMES || !_volumeData[idx]._normalPalette.hasValue()) {
		static palette::NormalPalette normalPalette;
		return normalPalette;
	}
	return *_volumeData[idx]._normalPalette.value();
}

voxel::Region MeshState::calculateExtractRegion(int x, int y, int z, const glm::ivec3 &meshSize) const {
	const glm::ivec3 mins(x * meshSize.x, y * meshSize.y, z * meshSize.z);
	const glm::ivec3 maxs = mins + meshSize - 1;
	return voxel::Region{mins, maxs};
}

bool MeshState::runScheduledExtractions(size_t maxExtraction) {
	const size_t n = _extractRegions.size();
	if (n == 0) {
		return false;
	}
	if (maxExtraction == 0) {
		return true;
	}
	voxel::SurfaceExtractionType type = (voxel::SurfaceExtractionType)_meshMode->intVal();
	size_t i;
	for (i = 0; i < n; ++i) {
		ExtractRegion extractRegion;
		if (!_extractRegions.pop(extractRegion)) {
			break;
		}
		const int idx = extractRegion.idx;
		const voxel::RawVolume *v = volume(idx);
		if (v == nullptr) {
			continue;
		}
		const voxel::Region &finalRegion = extractRegion.region;
		bool onlyAir = true;
		const voxel::Region copyRegion(finalRegion.getLowerCorner() - 2, finalRegion.getUpperCorner() + 2);
		if (!copyRegion.isValid()) {
			continue;
		}
		voxel::RawVolume copy(v, copyRegion, &onlyAir);
		const glm::ivec3 &mins = finalRegion.getLowerCorner();
		if (!onlyAir) {
			const palette::Palette &pal = palette(resolveIdx(idx));
			++_pendingExtractorTasks;
			_threadPool.enqueue([type, movedPal = core::move(pal), movedCopy = core::move(copy), mins, idx,
								 finalRegion, this]() {
				++_runningExtractorTasks;
				voxel::ChunkMesh mesh(65536, 65536, true);
				voxel::SurfaceExtractionContext ctx = voxel::createContext(type, &movedCopy, finalRegion, movedPal, mesh, mins);
				voxel::extractSurface(ctx);
				_pendingQueue.emplace(mins, idx, core::move(mesh));
				Log::debug("Enqueue mesh for idx: %i (%i:%i:%i)", idx, mins.x, mins.y, mins.z);
				--_runningExtractorTasks;
				--_pendingExtractorTasks;
			});
		} else {
			_pendingQueue.emplace(mins, idx, core::move(voxel::ChunkMesh(0, 0)));
		}
		--maxExtraction;
		if (maxExtraction == 0) {
			break;
		}
	}

	return true;
}

bool MeshState::update() {
	bool triggerClear = false;
	if (_meshMode->isDirty()) {
		_meshMode->markClean();
		clearPendingExtractions();

		for (int i = 0; i < MAX_VOLUMES; ++i) {
			if (voxel::RawVolume *v = volume(i)) {
				scheduleRegionExtraction(i, v->region());
			}
		}
		triggerClear = true;
	}
	runScheduledExtractions();
	return triggerClear;
}

bool MeshState::scheduleRegionExtraction(int idx, const voxel::Region &region) {
	core_trace_scoped(MeshStateScheduleExtraction);
	const int bufferIndex = resolveIdx(idx);
	voxel::RawVolume *v = volume(bufferIndex);
	if (v == nullptr) {
		return false;
	}

	const int s = _meshSize->intVal();
	const glm::ivec3 meshSize(s);
	const glm::ivec3 meshSizeMinusOne(s - 1);
	voxel::Region completeRegion = v->region();
	completeRegion.shiftUpperCorner(1, 1, 1);

	// convert to step coordinates that are needed to extract
	// the given region mesh size ranges
	// the boundaries are special - that's why we take care of this with
	// the offset of 1 - see the cubic surface extractor docs
	const glm::ivec3 &l = (region.getLowerCorner() - meshSizeMinusOne) / meshSize;
	const glm::ivec3 &u = (region.getUpperCorner() + 1) / meshSize;

	bool deletedMesh = false;
	Log::debug("modified region: %s", region.toString().c_str());
	for (int x = l.x; x <= u.x; ++x) {
		for (int y = l.y; y <= u.y; ++y) {
			for (int z = l.z; z <= u.z; ++z) {
				const voxel::Region &finalRegion = calculateExtractRegion(x, y, z, meshSize);
				const glm::ivec3 &mins = finalRegion.getLowerCorner();

				if (!voxel::intersects(completeRegion, finalRegion)) {
					deleteMeshes(mins, bufferIndex);
					deletedMesh = true;
					continue;
				}

				Log::debug("extract region: %s", finalRegion.toString().c_str());
				_extractRegions.emplace(finalRegion, bufferIndex, hidden(bufferIndex));
			}
		}
	}
	return deletedMesh;
}

void MeshState::extractAllPending() {
	while (runScheduledExtractions(100)) {
	}
	waitForPendingExtractions();
}

void MeshState::waitForPendingExtractions() {
	while (_pendingExtractorTasks > 0) {
		app::App::getInstance()->wait(1);
	}
}

void MeshState::clearPendingExtractions() {
	_threadPool.abort();
	while (_runningExtractorTasks > 0) {
		app::App::getInstance()->wait(1);
	}
	_pendingQueue.clear();
	_pendingExtractorTasks = 0;
}

voxel::SurfaceExtractionType MeshState::meshMode() const {
	return (voxel::SurfaceExtractionType)_meshMode->intVal();
}

int MeshState::resolveIdx(int idx) const {
	const int ref = reference(idx);
	if (ref != -1) {
		return resolveIdx(ref);
	}
	return idx;
}

bool MeshState::sameNormalPalette(int idx, const palette::NormalPalette *palette) const {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return false;
	}
	const palette::NormalPalette *normalPalette = _volumeData[idx]._normalPalette.value();
	if (normalPalette == nullptr) {
		return palette == nullptr;
	}
	if (palette == nullptr) {
		return false;
	}
	return normalPalette->hash() == palette->hash();
}

voxel::RawVolume *MeshState::setVolume(int idx, voxel::RawVolume *v, palette::Palette *palette, palette::NormalPalette *normalPalette, bool meshDelete,
									   bool &meshDeleted) {
	meshDeleted = false;
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return nullptr;
	}
	_volumeData[idx]._palette.setValue(palette);
	_volumeData[idx]._normalPalette.setValue(normalPalette);
	voxel::RawVolume *old = volume(idx);
	if (old == v) {
		return nullptr;
	}
	core_trace_scoped(RawVolumeRendererSetVolume);
	_volumeData[idx]._rawVolume = v;
	if (meshDelete) {
		deleteMeshes(idx);
		meshDeleted = true;
	}
	const size_t n = _extractRegions.size();
	for (size_t i = 0; i < n; ++i) {
		if (_extractRegions[i].idx == idx) {
			_extractRegions[i].idx = -1;
		}
	}

	return old;
}

core::DynamicArray<voxel::RawVolume *> MeshState::shutdown() {
	_threadPool.shutdown();
	clear();
	core::DynamicArray<voxel::RawVolume *> old;
	old.reserve(MAX_VOLUMES);
	for (int idx = 0; idx < (int)_volumeData.size(); ++idx) {
		VolumeData &state = _volumeData[idx];
		// hand over the ownership to the caller
		old.push_back(state._rawVolume);
		state._rawVolume = nullptr;
	}
	return old;
}

void MeshState::resetReferences() {
	for (auto &s : _volumeData) {
		s._reference = -1;
	}
}

int MeshState::reference(int idx) const {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return -1;
	}
	return _volumeData[idx]._reference;
}

void MeshState::setReference(int idx, int referencedIdx) {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return;
	}
	VolumeData &state = _volumeData[idx];
	state._reference = referencedIdx;
}

bool MeshState::hidden(int idx) const {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return true;
	}
	return _volumeData[idx]._hidden;
}

void MeshState::hide(int idx, bool hide) {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return;
	}
	_volumeData[idx]._hidden = hide;
}

video::Face MeshState::cullFace(int idx) const {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return video::Face::Back;
	}
	return _volumeData[idx]._cullFace;
}

void MeshState::setCullFace(int idx, video::Face face) {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return;
	}
	_volumeData[idx]._cullFace = face;
}

bool MeshState::grayed(int idx) const {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return true;
	}
	return _volumeData[idx]._gray;
}

void MeshState::gray(int idx, bool gray) {
	if (idx < 0 || idx >= MAX_VOLUMES) {
		return;
	}
	_volumeData[idx]._gray = gray;
}

} // namespace voxel
