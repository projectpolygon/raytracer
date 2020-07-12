#pragma once

#include <atlas/math/ray.hpp>
#include <atlas/math/math.hpp>
#include "structures/bounds.hpp"
#include "objects/object.hpp"

namespace poly::structures {

	class AcceleratorStruct : public poly::object::Object
	{
	public:
		//AcceleratorStruct();
		virtual bool hit(const math::Ray<math::Vector> &ray, SurfaceInteraction&sr) const = 0;
		virtual bool shadow_hit(const math::Ray<math::Vector> &ray, float &t) const = 0;
		virtual Bounds3D get_boundbox() const = 0;
	};

	struct KDNode
	{
	public:
		void init_leaf(int *primNums,
					   int np,
					   std::vector<int> *primitiveIndices);

		// This node has children if it is an interior node
		void init_interior(int axis, int ac, float s);

		float SplitPos() const;
		int nPrimitives() const;
		int SplitAxis() const;
		bool IsLeaf() const;
		int AboveChild() const;

		union {
			float split;				  // Interior
			int onePrimitive;			  // Leaf
			int offset_in_object_indices; // Leaf
		};
		union {
			int flags;		// Both
			int nPrims;		// Leaf
			int aboveChild; // Interior
		};
	};

	class KDTree : public AcceleratorStruct
	{
	public:
		KDTree(const std::vector<std::shared_ptr<poly::object::Object>> &p,
			   int isectCost, int traversalCost,
			   float emptyBonus, int maxPrims,
			   int maxDepth);

		Bounds3D get_boundbox() const;

		Bounds3D union_bounds(Bounds3D const &b1,
							  Bounds3D const &b2);

		//Bounds3D bound_world() {}

		struct KDToDo;

		// INTERSECT a ray with the tree
		bool hit(const math::Ray<math::Vector> &ray, SurfaceInteraction &sr) const;

		bool shadow_hit(const math::Ray<math::Vector> &ray, float &t) const;

	private:
		const int intersectCost, traversalCost, maxPrims;
		const float emptyBonus;
		std::vector<std::shared_ptr<Object>> objects;
		std::vector<int> all_leaf_object_indices;
		KDNode *m_nodes;
		int m_allocatedNodes, m_nextFreeNode;
		Bounds3D m_bounds;

		enum class EdgeType
		{
			Start,
			End
		};
		
		struct BoundEdge
		{
			BoundEdge() = default;
			BoundEdge(float t, int primNum, bool starting)
				: value(t), primNum(primNum)
			{
				type = starting ? EdgeType::Start : EdgeType::End;
			}

			float value;
			int primNum;
			EdgeType type;
		};

		void tree_build(int node_index,
			const Bounds3D &node_bounds,
			const std::vector<Bounds3D> &allPrimBounds,
			int *node_object_indices,
			int num_objects,
			int depth,
			const std::unique_ptr<BoundEdge[]> edges[3], int *below_objs_list, int *above_objs_list, // Working space
			int useless_refine_cnt);
	};

} // namespace poly::structures