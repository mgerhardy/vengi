/**
 * @file
 */

#pragma once

#include "core/IComponent.h"
#include "voxel/Region.h"
#include "voxel/Voxel.h"
#include "voxelformat/SceneGraphNode.h"
#include "core/collection/RingBuffer.h"
#include "core/String.h"
#include <stdint.h>
#include <stddef.h>

namespace voxel {
class RawVolume;
}

namespace voxedit {

enum class MementoType {
	/**
	 * voxel volume modifications
	 */
	Modification,
	// scene graph actions
	SceneNodeMove,
	SceneNodeAdded,
	SceneNodeRemoved,
	SceneNodeRenamed,
	SceneNodeTransform,

	Max
};

/**
 * @brief Holds the data of a memento state
 *
 * The given buffer is owned by this class and represents a compressed volume
 */
class MementoData {
	friend struct MementoState;
	friend class MementoHandler;
private:
	/**
	 * @brief How big is the buffer with the compressed volume data
	 */
	size_t _compressedSize = 0;
	/**
	 * @brief The compressed volume data
	 */
	uint8_t* _buffer = nullptr;
	/**
	 * The region the given volume data is for
	 */
	voxel::Region _region {};

	MementoData(const uint8_t* buf, size_t bufSize, const voxel::Region& _region);
public:
	constexpr MementoData() {}
	MementoData(MementoData&& o) noexcept;
	MementoData(const MementoData& o);
	~MementoData();

	inline size_t size() const { return _compressedSize; }

	MementoData& operator=(MementoData &&o) noexcept;

	/**
	 * @brief Converts the given @c mementoData into a volume
	 * @note Keep in mind that you own the returned memory
	 * @return The volume from the given memento data or @c null if the memento data
	 * did not contain a valid volume buffer
	 */
	static voxel::RawVolume* toVolume(const MementoData& mementoData);
	/**
	 * @brief Converts the given volume into a @c MementoData structure (and perform the compression)
	 * @param[in] volume The volume to create the memento state for. This might be @c null.
	 */
	static MementoData fromVolume(const voxel::RawVolume* volume);
};

struct MementoState {
	MementoType type;
	MementoData data;
	int parentId;
	int nodeId;
	uint32_t keyFrame;
	core::String name;
	glm::mat4x4 localMatrix;
	/**
	 * @note This region might be different from the region given in the @c MementoData. In case of an @c MementoHandler::undo()
	 * call, we have to make sure that the region of the previous state is re-extracted.
	 */
	voxel::Region region;

	MementoState() :
			type(MementoType::Max), parentId(0), nodeId(0), keyFrame(0) {
	}

	MementoState(MementoType _type, const MementoData &_data, int _parentId, int _nodeId, const core::String &_name,
				 const voxel::Region &_region, const glm::mat4x4 &_localMatrix, uint32_t _frameId = 0)
		: type(_type), data(_data), parentId(_parentId), nodeId(_nodeId), keyFrame(_frameId), name(_name), localMatrix(_localMatrix),
		  region(_region) {
	}

	MementoState(MementoType _type, MementoData &&_data, int _parentId, int _nodeId, core::String &&_name,
				 voxel::Region &&_region)
		: type(_type), data(_data), parentId(_parentId), nodeId(_nodeId), keyFrame(0), name(_name), region(_region) {
	}

	inline bool valid() const {
		return type != MementoType::Max;
	}

	/**
	 * Some types (@c MementoType) don't have a volume attached.
	 */
	inline bool hasVolumeData() const {
		return data._buffer != nullptr;
	}

	inline const voxel::Region& dataRegion() const {
		return data._region;
	}
};

/**
 * @brief Class that manages the undo and redo steps for the scene
 */
class MementoHandler : public core::IComponent {
private:
	core::RingBuffer<MementoState, 64u> _states;
	uint8_t _statePosition = 0u;
	int _locked = 0;

	bool markUndoPreamble(int nodeId);
public:
	MementoHandler();
	~MementoHandler();

	void construct() override;
	bool init() override;
	void shutdown() override;

	/**
	 * @brief Locks the handler for accepting new states or perform undo() or redo() steps
	 * @sa @c unlock()
	 */
	void lock();
	/**
	 * @brief Unlocks the handler for accepting new states or perform undo() or redo() steps again
	 * @sa @c lock()
	 */
	void unlock();

	void print() const;

	void clearStates();

	/**
	 * @brief Add a new state entry to the memento handler that you can return to.
	 * @note This is adding the current active state to the handler - you can then undo to the previous state.
	 * That is the reason why you always have to add the initial (maybe empty) state, too
	 * @note Keep in mind, that there is a maximum of states that can get handled here.
	 * @param[in] parentId The node's parentId that was modified
	 * @param[in] nodeId The nodeId that was modified
	 * @param[in] name The name of the nodeId
	 * @param[in] volume The state of the volume
	 * @param[in] type The @c MementoType - has influence on undo() and redo() state position changes.
	 */
	void markUndo(int parentId, int nodeId, const core::String &name, const voxel::RawVolume *volume, MementoType type,
				  const voxel::Region &region, const glm::mat4 &transformMatrix, voxelformat::KeyFrameIndex keyFrameIdx);

	void markNodeRemoved(const voxelformat::SceneGraphNode &node);
	void markNodeAdded(const voxelformat::SceneGraphNode &node);
	void markNodeTransform(const voxelformat::SceneGraphNode &node, voxelformat::KeyFrameIndex keyFrameIdx);
	void markModification(const voxelformat::SceneGraphNode &node, const voxel::Region& modifiedRegion);
	void markNodeRenamed(const voxelformat::SceneGraphNode &node);
	void markNodeMoved(int targetId, int sourceId);

	/**
	 * @brief The scene graph is giving new nodes for each insert - thus while undo redo we get new node ids for each new node.
	 * This method will update the references for the old node id to the new one
	 */
	void updateNodeId(int nodeId, int newNodeId);

	/**
	 * @note Keep in mind that the returned state contains memory for the voxel::RawVolume that you take ownership for
	 */
	MementoState undo();
	/**
	 * @note Keep in mind that the returned state contains memory for the voxel::RawVolume that you take ownership for
	 */
	MementoState redo();
	bool canUndo() const;
	bool canRedo() const;

	const MementoState& state() const;

	size_t stateSize() const;
	uint8_t statePosition() const;
};

/**
 * @brief Locks the memento handler to accept further state changes for undo/redo.
 * @note This is useful in situations where an undo or redo would result in actions that by
 * itself would generate new memento states, too.
 */
class ScopedMementoHandlerLock {
private:
	MementoHandler& _handler;
public:
	ScopedMementoHandlerLock(MementoHandler& handler) : _handler(handler) {
		_handler.lock();
	}
	~ScopedMementoHandlerLock() {
		_handler.unlock();
	}
};

inline const MementoState& MementoHandler::state() const {
	return _states[_statePosition];
}

inline uint8_t MementoHandler::statePosition() const {
	return _statePosition;
}

inline size_t MementoHandler::stateSize() const {
	return _states.size();
}

inline bool MementoHandler::canUndo() const {
	if (_locked > 0) {
		return false;
	}
	if (stateSize() <= 1) {
		return false;
	}
	return _statePosition > 0;
}

inline bool MementoHandler::canRedo() const {
	if (_locked > 0) {
		return false;
	}
	if (stateSize() <= 1) {
		return false;
	}
	return _statePosition <= stateSize() - 2;
}

}
