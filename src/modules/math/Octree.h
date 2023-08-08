/**
 * @file
 */

#pragma once

#include <vector>
#include <list>
#include <algorithm>
#include <iterator>
#include "AABB.h"
#include "Frustum.h"
#include "core/Trace.h"
#include <glm/vec3.hpp>

namespace math {

extern math::AABB<int> computeAABB(const Frustum& area, const glm::vec3& gridSize);

/**
 * @note Given NODE type must implement @c aabb() and return math::AABB<TYPE>
 */
template<class NODE, typename TYPE = int>
class Octree {
public:
	typedef typename std::list<NODE> Contents;

	class OctreeNode;
	struct IOctreeListener {
		virtual ~IOctreeListener() {}
		virtual void onNodeCreated(const OctreeNode& parent, const OctreeNode& child) const {}
	};

	class OctreeNode {
		friend class Octree;
	private:
		/*
		 * +Y                        +Z
		 * |                         /
		 * |                        /
		 * |                       /
		 * |                      /
		 * |       O---------------O---------------O
		 * |      /               /               /|
		 * |     /       3       /       7       / |
		 * |    /               /               /  |
		 * |   O---------------O---------------O   |
		 * |  /               /               /|   |
		 * | /       2       /       6       / | 7 |
		 * |/               /               /  |   O
		 * O---------------O---------------O   |  /|
		 * |               |               |   | / |
		 * |               |               | 6 |/  |
		 * |               |               |   O   |
		 * |       2       |       6       |  /|   |
		 * |               |               | / | 5 |
		 * |               |               |/  |   O
		 * O---------------O---------------O   |  /
		 * |               |               |   | /
		 * |               |               | 4 |/
		 * |               |               |   O
		 * |       0       |       4       |  /
		 * |               |               | /
		 * |               |               |/
		 * O---------------O---------------O------------------+X
		 */
		static void split(const AABB<TYPE>& aabb, AABB<TYPE> (&result)[8]) {
			const glm::tvec3<TYPE>& center = aabb.getCenter();
			result[0] = AABB<TYPE>(aabb.mins(), center);

			glm::tvec3<TYPE> mins1(aabb.getLowerX(), aabb.getLowerY(), center.z);
			glm::tvec3<TYPE> maxs1(center.x, center.y, aabb.getUpperZ());
			result[1] = AABB<TYPE>(mins1, maxs1);

			glm::tvec3<TYPE> mins2(aabb.getLowerX(), center.y, aabb.getLowerZ());
			glm::tvec3<TYPE> maxs2(center.x, aabb.getUpperY(), center.z);
			result[2] = AABB<TYPE>(mins2, maxs2);

			glm::tvec3<TYPE> mins3(aabb.getLowerX(), center.y, center.z);
			glm::tvec3<TYPE> maxs3(center.x, aabb.getUpperY(), aabb.getUpperZ());
			result[3] = AABB<TYPE>(mins3, maxs3);

			glm::tvec3<TYPE> mins4(center.x, aabb.getLowerY(), aabb.getLowerZ());
			glm::tvec3<TYPE> maxs4(aabb.getUpperX(), center.y, center.z);
			result[4] = AABB<TYPE>(mins4, maxs4);

			glm::tvec3<TYPE> mins5(center.x, aabb.getLowerY(), center.z);
			glm::tvec3<TYPE> maxs5(aabb.getUpperX(), center.y, aabb.getUpperZ());
			result[5] = AABB<TYPE>(mins5, maxs5);

			glm::tvec3<TYPE> mins6(center.x, center.y, aabb.getLowerZ());
			glm::tvec3<TYPE> maxs6(aabb.getUpperX(), aabb.getUpperY(), center.z);
			result[6] = AABB<TYPE>(mins6, maxs6);

			glm::tvec3<TYPE> mins7(center.x, center.y, center.z);
			glm::tvec3<TYPE> maxs7(aabb.getUpperX(), aabb.getUpperY(), aabb.getUpperZ());
			result[7] = AABB<TYPE>(mins7, maxs7);
		}

		int _maxDepth;
		int _depth;
		const Octree* _octree;
		const AABB<TYPE> _aabb;
		Contents _contents;
		std::vector<OctreeNode> _nodes;

		template<class FUNC>
		inline void visit(FUNC&& func) const {
			core_trace_scoped(OctreeNodeVisit);
			func(*this);
			for (const OctreeNode& node : _nodes) {
				node.visit(func);
			}
		}

		void createNodes() {
			core_trace_scoped(OctreeNodeCreateNodes);
			if (_depth >= _maxDepth) {
				return;
			}

			const auto& _aabb = aabb();
			const glm::tvec3<TYPE>& _aabbSize = _aabb.getWidth();
			const glm::tvec3<TYPE> one((TYPE)1);
			if (_aabbSize.x <= one.x && _aabbSize.y <= one.y && _aabbSize.z <= one.z) {
				return;
			}

			AABB<TYPE> subareas[8];
			split(_aabb, subareas);
			_nodes.reserve(8);
			for (size_t i = 0u; i < 8; ++i) {
				_nodes.emplace_back(OctreeNode(subareas[i], _maxDepth, _depth + 1, _octree));
				if (_octree->_listener != nullptr) {
					_octree->_listener->onNodeCreated(*this, _nodes.back());
				}
			}
		}
	public:
		static inline AABB<TYPE> aabb(const typename core::remove_pointer<NODE>::type* item) {
			return item->aabb();
		}

		static inline AABB<TYPE> aabb(const typename core::remove_pointer<NODE>::type& item) {
			return item.aabb();
		}

		OctreeNode(const AABB<TYPE>& bounds, int maxDepth, int depth, const Octree* octree) :
				_maxDepth(maxDepth), _depth(depth), _octree(octree), _aabb(bounds) {
		}

		inline int depth() const {
			return _depth;
		}

		inline int count() const {
			core_trace_scoped(OctreeNodeCount);
			int count = 0;
			for (const typename Octree<NODE, TYPE>::OctreeNode& node : _nodes) {
				count += (int)node.count();
			}
			count += _contents.size();
			return count;
		}

		inline const AABB<TYPE>& aabb() const {
			return _aabb;
		}

		inline const Contents& getContents() const {
			return _contents;
		}

		void getAllContents(Contents& results) const {
			core_trace_scoped(OctreeNodeGetAllContents);
			for (const typename Octree<NODE, TYPE>::OctreeNode& node : _nodes) {
				if (node.isEmpty()) {
					continue;
				}
				node.getAllContents(results);
			}

			// faster for list - but slower for vector
			std::copy(_contents.begin(), _contents.end(), std::back_inserter(results));
		}

		bool remove(const NODE& item) {
			core_trace_scoped(OctreeNodeRemove);
			const AABB<TYPE>& area = aabb(item);
			if (!aabb().containsAABB(area)) {
				return false;
			}
			for (typename Octree<NODE, TYPE>::OctreeNode& node : _nodes) {
				if (!node.remove(item)) {
					continue;
				}
				return true;
			}

			typename Contents::iterator i = std::find(_contents.begin(), _contents.end(), item);
			if (i == _contents.end()) {
				return false;
			}
			_contents.erase(i);
			return true;
		}

		bool insert(const NODE& item) {
			core_trace_scoped(OctreeNodeInsert);
			const AABB<TYPE>& area = aabb(item);
			if (!aabb().containsAABB(area)) {
				return false;
			}

			if (_nodes.empty()) {
				createNodes();
			}

			for (typename Octree<NODE, TYPE>::OctreeNode& node : _nodes) {
				if (!node.insert(item)) {
					continue;
				}
				return true;
			}

			_contents.push_back(item);
			return true;
		}

		inline bool isLeaf() const {
			return _nodes.empty();
		}

		inline bool hasContent() const {
			return !_contents.empty();
		}

		inline bool isEmpty() const {
			return isLeaf() && _contents.empty();
		}

		void query(const AABB<TYPE>& queryArea, Contents& results) const {
			core_trace_scoped(OctreeNodeQueryAABB);
			for (const NODE& item : _contents) {
				if (intersects(queryArea, aabb(item))) {
					results.push_back(item);
				}
			}

			for (const typename Octree<NODE, TYPE>::OctreeNode& node : _nodes) {
				if (node.isEmpty()) {
					continue;
				}

				const AABB<TYPE>& aabb = node.aabb();
				if (aabb.containsAABB(queryArea)) {
					node.query(queryArea, results);
					// the queried area is completely part of the node - so no other node can be involved
					break;
				}

				if (queryArea.containsAABB(aabb)) {
					node.getAllContents(results);
					// the whole node content is part of the query
					continue;
				}

				if (intersects(aabb, queryArea)) {
					node.query(queryArea, results);
				}
			}
		}

		void query(const Frustum& queryArea, const AABB<TYPE>& queryAreaAABB, Contents& results) const {
			core_trace_scoped(OctreeNodeQueryFrustum);
			for (const NODE& item : _contents) {
				const auto& itemAABB = aabb(item);
				if (queryArea.isVisible(itemAABB.mins(), itemAABB.maxs())) {
					results.push_back(item);
				}
			}

			for (const typename Octree<NODE, TYPE>::OctreeNode& node : _nodes) {
				if (node.isEmpty()) {
					continue;
				}

				const AABB<TYPE>& aabb = node.aabb();
				if (aabb.containsAABB(queryAreaAABB)) {
					node.query(queryArea, queryAreaAABB, results);
					// the queried area is completely part of the node - so no other node can be involved
					break;
				}

				const FrustumResult result = queryArea.test(aabb.mins(), aabb.maxs());
				if (FrustumResult::Intersect == result) {
					// some children might be visible - but other nodes might also still contribute
					node.query(queryArea, queryAreaAABB, results);
				} else if (FrustumResult::Inside == result) {
					// the whole node content is part of the query
					node.getAllContents(results);
				}
			}
		}
	};
private:
	OctreeNode _root;
	// dirty flag can be used for query caches
	bool _dirty = false;
	const IOctreeListener* _listener = nullptr;

	template<class VISITOR>
	void visit(const Frustum& queryArea, const AABB<TYPE>& queryAABB, VISITOR&& visitor, const glm::vec<3, TYPE>& minSize) const {
		core_trace_scoped(OctreeVisitFrustum);
		const glm::tvec3<TYPE>& mins = queryAABB.mins();
		const glm::tvec3<TYPE>& width = queryAABB.getWidth();
		const TYPE maxX = mins.x + width.x;
		const TYPE maxY = mins.y + width.y;
		const TYPE maxZ = mins.z + width.z;
		glm::tvec3<TYPE> qmins;
		for (qmins.x = mins.x; qmins.x < maxX; qmins.x += minSize.x) {
			for (qmins.y = mins.y; qmins.y < maxY; qmins.y += minSize.y) {
				for (qmins.z = mins.z; qmins.z < maxZ; qmins.z += minSize.z) {
					const glm::tvec3<TYPE> qmaxs(qmins + minSize);
					if (!queryArea.isVisible(qmins, qmaxs)) {
						continue;
					}
					if (!visitor(qmins, qmaxs)) {
						break;
					}
				}
			}
		}
	}

public:
	Octree(const AABB<TYPE>& aabb, int maxDepth = 10) :
			_root(aabb, maxDepth, 0, this) {
	}

	inline int count() const {
		core_trace_scoped(OctreeCount);
		return _root.count();
	}

	inline bool insert(const NODE& item) {
		core_trace_scoped(OctreeInsert);
		if (_root.insert(item)) {
			_dirty = true;
			return true;
		}
		return false;
	}

	inline bool remove(const NODE& item) {
		core_trace_scoped(OctreeRemove);
		if (_root.remove(item)) {
			_dirty = true;
			return true;
		}
		return false;
	}

	inline const AABB<TYPE>& aabb() const {
		core_trace_scoped(OctreeAABB);
		return _root.aabb();
	}

	inline void query(const AABB<TYPE>& area, Contents& results) const {
		core_trace_scoped(OctreeQuery);
		_root.query(area, results);
	}

	inline void query(const Frustum& area, Contents& results) const {
		core_trace_scoped(OctreeQuery);
		const AABB<float>& areaAABB = area.aabb();
		_root.query(area, AABB<TYPE>(areaAABB.mins(), areaAABB.maxs()), results);
	}

	/**
	 * @brief Executes the given visitor for all visible nodes in this octree.
	 * @note As there might not be nodes yet for the potential visible nodes, the visitor
	 * is just called with the center of the potential node
	 * @param area The frustum area to visit visible AABBs for
	 * @param visitor The visitor to execute if an AABB is visible in the given frustum
	 * @param minSize The minimum size of the AABBs when the recursion is stopped.
	 */
	template<class VISITOR>
	inline void visit(const Frustum& area, VISITOR&& visitor, const glm::vec<3, TYPE>& minSize) {
		core_trace_scoped(OctreeVisit);
		const glm::vec3 fminsize(minSize);
		const math::AABB<int>& aabb = computeAABB(area, fminsize);
		visit(area, aabb, visitor, minSize);
	}

	template<class VISITOR>
	inline void visit(const glm::vec<3, TYPE>& mins, const glm::vec<3, TYPE>& maxs, VISITOR&& visitor, const glm::vec<3, TYPE>& minSize) {
		core_trace_scoped(OctreeVisit);
		const math::AABB<TYPE> aabb(mins, maxs);
		glm::tvec3<TYPE> qmins;
		for (qmins.x = mins.x; qmins.x < maxs.x; qmins.x += minSize.x) {
			for (qmins.y = mins.y; qmins.y < maxs.y; qmins.y += minSize.y) {
				for (qmins.z = mins.z; qmins.z < maxs.z; qmins.z += minSize.z) {
					const glm::tvec3<TYPE> qmaxs(qmins + minSize);
					if (!visitor(qmins, qmaxs)) {
						break;
					}
				}
			}
		}
	}

	void setListener(const IOctreeListener* func) {
		_listener = func;
	}

	void clear() {
		_dirty = true;
		_root._contents.clear();
		_root._nodes.clear();
		_root._nodes.reserve(4);
	}

	inline void markAsClean() {
		_dirty = false;
	}

	inline bool isDirty() {
		return _dirty;
	}

	inline void getContents(Contents& results) const {
		results.clear();
		//results.reserve(count());
		_root.getAllContents(results);
	}

	template<class FUNC>
	void visit(FUNC&& func) {
		_root.visit(func);
	}
};

}
