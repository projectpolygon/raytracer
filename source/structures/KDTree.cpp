#include <algorithm>
#include <vector>
#include <iostream>
#include "structures/KDTree.hpp"
#include "structures/bounds.hpp"

namespace poly::structures
{
	void KDNode::init_leaf(int *primNums,
						   int np,
						   std::vector<int> *primitiveIndices)
	{
		flags = 3;
		nPrims |= (np << 2);
		if (np == 0)
			onePrimitive = 0;
		else if (np == 1)
			onePrimitive = primNums[0];
		else
		{
			offset_in_object_indices = (int)primitiveIndices->size();
			for (int i = 0; i < np; ++i)
				primitiveIndices->push_back(primNums[i]);
		}
	}

	// This node has children if it is an interior node
	void KDNode::init_interior(int axis, int ac, float s)
	{
		split = s;
		flags = axis;
		aboveChild |= (ac << 2);
	}

	float KDNode::SplitPos() const
	{
		return split;
	}
	int KDNode::nPrimitives() const
	{
		return nPrims >> 2;
	}
	int KDNode::SplitAxis() const
	{
		return flags & 3;
	}
	bool KDNode::IsLeaf() const
	{
		return (flags & 3) == 3;
	}
	int KDNode::AboveChild() const
	{
		return aboveChild >> 2;
	}

	KDTree::KDTree(const std::vector<std::shared_ptr<poly::object::Object>>& p,
		int isectCost, int traversalCost,
		float emptyBonus, int maxPrims,
		int max_tree_height)
		: intersectCost(isectCost),
		traversalCost(traversalCost),
		maxPrims(maxPrims),
		emptyBonus(emptyBonus),
		objects(p)
	{
		m_nextFreeNode = 0;
		m_allocatedNodes = 0;
		if (max_tree_height <= 0) {
			max_tree_height = (int)std::round(8 + 1.3f * log(objects.size()) / log(2));
			std::clog << "INFO: KD-Tree max tree height not set. Auto-configuring to use " << max_tree_height << std::endl;
		}

		// Sanity check that we HAVE objects
		assert(p.size() > 0);

		// Generate the bounding for the tree
		std::vector<Bounds3D> primBounds;
		m_bounds = objects.at(0)->boundbox_get();
		for (const std::shared_ptr<Object>& obj : objects)
		{
			Bounds3D b = obj->get_boundbox();
			m_bounds = union_bounds(m_bounds, b);
			primBounds.push_back(b);
		}

		// Working memory so that we are avoiding VLA's
		std::unique_ptr<BoundEdge[]> edges[3];
		for (int i = 0; i < 3; ++i)
		{
			edges[i].reset(new BoundEdge[2 * objects.size()]);
		}
		std::unique_ptr<int[]> prims0(new int[objects.size()]);							 // Left branches
		std::unique_ptr<int[]> prims1(new int[((size_t)max_tree_height + 1) * objects.size()]); // Right branches

		// Init the trackers for our objects.
		// Must match order of the storage array
		std::unique_ptr<int[]> object_indices(new int[objects.size()]);
		for (size_t i = 0; i < objects.size(); ++i) {
			object_indices[i] = (unsigned int)i;
		}

		// Start recursive build
		tree_build(0,
				   m_bounds,
				   primBounds,
				   object_indices.get(),
				   (int)objects.size(),
				   max_tree_height,
				   edges, prims0.get(), prims1.get(), // Working Space
				   0);
	}

	/*
	* Recursively build the KDTree
	* Starts by descending to the left of each split, then traverses back up the tree 
	* building the above branches in a depth first manner
	*/
	void KDTree::tree_build(int node_index,
		const Bounds3D& node_bounds,
		const std::vector<Bounds3D>& allPrimBounds,
		int* node_object_indices,
		int num_objects,
		int depth,
		const std::unique_ptr<BoundEdge[]> edges[3], int* below_objs_list, int* above_objs_list, // Working space
		int useless_refine_cnt)
	{
		// If the next free node is outside of what we have allocated, reallocate and copy over to a 2N array
		if (m_nextFreeNode == m_allocatedNodes)
		{
			int new_num_to_allocate = std::max(2 * m_allocatedNodes, 512);
			KDNode* temp_list = (KDNode*)malloc(new_num_to_allocate * sizeof(KDNode));
			if (m_allocatedNodes > 0)
			{
				std::memcpy(temp_list, m_nodes, (size_t)m_allocatedNodes * sizeof(KDNode));
				free(m_nodes);
			}
			m_nodes = temp_list;
			m_allocatedNodes = new_num_to_allocate;
		}

		++m_nextFreeNode;

		// If the number of objects in this node is less than our maximum per leaf, or we are at depth 0, stop recursion
		if (num_objects <= maxPrims || depth == 0)
		{
			// this node gets made into a leaf. Multiple object indices are stored in the 'all_leaf_object_indices' to keep leaves small
			m_nodes[node_index].init_leaf(node_object_indices, num_objects, &all_leaf_object_indices);
			return;
		}

		int bestAxis = -1, bestOffset = -1;
		float bestCost = std::numeric_limits<float>::max(); // This might mean that no other axes are tried!!
		float oldCost = intersectCost * float(num_objects);
		float invTotalSA = 1 / node_bounds.surfaceArea();
		int num_edges = 2 * num_objects;
		math::Vector bounds_diagonal = node_bounds.pMax - node_bounds.pMin;

		// The axis that we will split on
		int axis = node_bounds.maximum_extent();

		int retries = 0;
		// first try; retries = 0; x-axis attempt
		// second try; retries = 1; y-axis attempt
		// third try; retries = 2; z-axis attempt
		while (retries < 3)
		{
			//retrySplit:

			// Initialize edges for this axis
			for (int i = 0; i < num_objects; i++)
			{
				int pn = node_object_indices[i];
				const Bounds3D& all_bounds = allPrimBounds[pn];
				edges[axis][2 * i] = BoundEdge(all_bounds.pMin[axis], pn, true);
				edges[axis][2 * i + 1] = BoundEdge(all_bounds.pMax[axis], pn, false);
			}

			// Sort the edges so that we encounter them in the correct order
			std::sort(&edges[axis][0], &edges[axis][num_edges],
				[](const BoundEdge& e0, const BoundEdge& e1) -> bool {
					if (e0.value == e1.value)
						return (int)e0.type < (int)e1.type;
					else
						return e0.value < e1.value;
				});

			// Select the split point by using the heuristic weighting
			// adjustment can be done using higher traversal or higher
			// overlap penalties
			int nBelow = 0;
			int nAbove = num_objects;
			int num_axes = 3;
			for (int i = 0; i < num_edges; ++i)
			{

				// If we have an end, then we have checked one full object
				if (edges[axis][i].type == EdgeType::End)
				{
					--nAbove;
				}

				// If this edge is strictly inside bounds
				float current_edge = edges[axis][i].value;
				if (current_edge > node_bounds.pMin[axis] && current_edge < node_bounds.pMax[axis])
				{
					int otherAxis0 = (axis + 1) % num_axes;
					int otherAxis1 = (axis + 2) % num_axes;

					// Compute the area (volume) below and above the split
					float area_below = 2 * (bounds_diagonal[otherAxis0] * bounds_diagonal[otherAxis1] + (current_edge - node_bounds.pMin[axis]) * (bounds_diagonal[otherAxis0] + bounds_diagonal[otherAxis1]));
					float area_above = 2 * (bounds_diagonal[otherAxis0] * bounds_diagonal[otherAxis1] + (node_bounds.pMax[axis] - current_edge) * (bounds_diagonal[otherAxis0] + bounds_diagonal[otherAxis1]));

					// Probability of being below or above
					float pBelow = area_below * invTotalSA;
					float pAbove = area_above * invTotalSA;

					// If either is 0, then we get a small bonus for
					// having a split that empties a large amount of area
					float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0.0f;

					// Compute the cost of this split.
					float cost = traversalCost + intersectCost * (1 - eb) * (pBelow * nBelow + pAbove * nAbove);
					if (cost < bestCost)
					{
						bestCost = cost;
						bestAxis = axis;
						bestOffset = i;
					}
				}

				// If this edge was a starting edge, we have one below now
				if (edges[axis][i].type == EdgeType::Start)
				{
					++nBelow;
				}
			}

			/*
			// Create a leaf if no good splits were found
			if (bestAxis == -1 && retries < 2) {
			  ++retries;
			  axis = (axis + 1) % 3;
			  goto retrySplit;
			}
			*/

			if (bestAxis == -1 && retries < 2)
			{ // This will never be hit! The float is ININITY...so EVERYTHING will select a best axis
				++retries;
				axis = (axis + 1) % num_axes;
			}
			else
			{
				break;
			}
		}

		// If the cost of this refinement is worse than before we refined,
		// increment the tracker so if we go too far we can stop splitting
		if (bestCost > oldCost)
		{
			useless_refine_cnt++;
		}

		// If the cost for this split is 'bad' make this a leaf and be done with it
		if ((bestCost > 4 * oldCost && num_objects < 16) || bestAxis == -1 || useless_refine_cnt >= 3)
		{
			m_nodes[node_index].init_leaf(node_object_indices, num_objects, &all_leaf_object_indices);
			return;
		}

		// Move the primitives for this list into the correct side of the split
		unsigned int num_objects_below = 0;
		unsigned int num_objects_above = 0;
		for (int i = 0; i < bestOffset; ++i)
		{
			if (edges[bestAxis][i].type == EdgeType::Start)
			{
				below_objs_list[num_objects_below++] = edges[bestAxis][i].primNum;
			}
		}
		for (int i = bestOffset + 1; i < num_edges; ++i)
		{
			if (edges[bestAxis][i].type == EdgeType::End)
			{
				above_objs_list[num_objects_above++] = edges[bestAxis][i].primNum;
			}
		}

		// Recurse onto the children lists and initialize
		float split_plane = edges[bestAxis][bestOffset].value;
		Bounds3D below_bounds = node_bounds;
		Bounds3D above_bounds = node_bounds;
		below_bounds.pMax[bestAxis] = split_plane;
		above_bounds.pMin[bestAxis] = split_plane;

		// Build the below subtree
		int belowChild = node_index + 1; // Next in list gets evaluated first
		tree_build(belowChild,
			below_bounds,
			allPrimBounds,
			below_objs_list,
			num_objects_below,
			depth - 1,
			edges,
			below_objs_list,
			above_objs_list + num_objects, // Offset because above objects are handled after
			useless_refine_cnt);

		// Build the above subtree
		int aboveChild = m_nextFreeNode; // next in the free nodes list is the right subtree of this node
		m_nodes[node_index].init_interior(bestAxis, aboveChild, split_plane);
		tree_build(aboveChild,
			above_bounds,
			allPrimBounds,
			above_objs_list,
			num_objects_above,
			depth - 1,
			edges,
			below_objs_list,
			above_objs_list + num_objects,
			useless_refine_cnt);
	}

	Bounds3D KDTree::get_boundbox() const
	{
		return m_bounds;
	}

	Bounds3D KDTree::union_bounds(Bounds3D const &b1,
								  Bounds3D const &b2)
	{
		return Bounds3D(
			math::Vector(
				std::min(b1.pMin.x, b2.pMin.x),
				std::min(b1.pMin.y, b2.pMin.y),
				std::min(b1.pMin.z, b2.pMin.z)),
			math::Vector(
				std::max(b1.pMax.x, b2.pMax.x),
				std::max(b1.pMax.y, b2.pMax.y),
				std::max(b1.pMax.z, b2.pMax.z)));
	}

	//Bounds3D bound_world() {}

	struct KDTree::KDToDo
	{
		const KDNode *node;
		double tMin, tMax;
	};

	// INTERSECT a ray with the tree
	bool KDTree::hit(const math::Ray<math::Vector> &ray, SurfaceInteraction &sr) const
	{
		// First, check if we intersect the box at all
		double tMin, tMax;
		if (!m_bounds.get_intersects(ray, &tMin, &tMax))
		{
			return false;
		}

		math::Vector invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
		constexpr int maxTodo = 512;
		KDToDo todo[maxTodo];
		int todoPos = 0;

		bool hit = false;
		const KDNode *node = &m_nodes[0];
		while (node != nullptr)
		{
			//if (ray.tMax < tMin) break; // Our rays go forever
			// While we aren't at a leaf, go down the list
			if (!node->IsLeaf())
			{
				int axis = node->SplitAxis();
				float dist_to_split = (node->SplitPos() - ray.o[axis]) * invDir[axis];

				const KDNode *firstChild;
				const KDNode *secondChild;

				int belowFirst = (ray.o[axis] < node->SplitPos()) || (ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);

				// If we are below first, then we are starting below the split plane
				if (belowFirst)
				{
					firstChild = node + 1;
					secondChild = &m_nodes[node->AboveChild()];
				}
				else
				{
					firstChild = &m_nodes[node->AboveChild()];
					secondChild = node + 1;
				}

				// Depending on where in the box the split occurs, we may not need to check the whole volume
				if (dist_to_split > tMax || dist_to_split <= 0)
				{
					// Split happens after we leave the box, no need to check second
					node = firstChild;
				}
				else if (dist_to_split < tMin)
				{
					// Split happens before we ever enter, no need to check first
					node = secondChild;
				}
				else
				{
					// Split happens in between. Need to check both sides
					todo[todoPos].node = secondChild;
					todo[todoPos].tMin = dist_to_split;
					todo[todoPos].tMax = tMax;
					todoPos++;

					node = firstChild;
					tMax = dist_to_split;
				}
			}
			else
			{
				// This node is a leaf, need to check if we hit any of the contained objects
				int number_objects_in_node = node->nPrimitives();
				if (number_objects_in_node == 1)
				{
					const std::shared_ptr<Object> &obj = objects.at(node->onePrimitive);
					if (obj->hit(ray, sr))
					{
						hit = true;
					}
				}
				else
				{
					for (int i = 0; i < number_objects_in_node; ++i)
					{
						int index = all_leaf_object_indices[(size_t)node->offset_in_object_indices + i];
						const std::shared_ptr<Object> &obj = objects.at(index);
						if (obj->hit(ray, sr))
						{
							hit = true;
						}
					}
				}

				// No need to go further down this branch.
				// Retrieve the next branch start
				if (todoPos > 0)
				{
					todoPos--;
					node = todo[todoPos].node;
					tMin = todo[todoPos].tMin;
					tMax = todo[todoPos].tMax;
				}
				else
				{
					// None left. We're done
					break;
				}
			}
		}
		return hit;
	}

	bool KDTree::shadow_hit(const math::Ray<math::Vector> &ray, float &t) const
	{
		// First, check if we intersect the box at all
		double tMin, tMax;
		if (!m_bounds.get_intersects(ray, &tMin, &tMax))
		{
			return false;
		}

		math::Vector invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
		constexpr int maxTodo = 512;
		KDToDo todo[maxTodo];
		int todoPos = 0;

		bool hit = false;
		const KDNode *node = &m_nodes[0];
		while (node != nullptr)
		{
			//if (ray.tMax < tMin) break; // Our rays go forever
			// While we aren't at a leaf, go down the list
			if (!node->IsLeaf())
			{
				int axis = node->SplitAxis();
				float dist_to_split = (node->SplitPos() - ray.o[axis]) * invDir[axis];

				const KDNode *firstChild;
				const KDNode *secondChild;

				int belowFirst = (ray.o[axis] < node->SplitPos()) || (ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);

				// If we are below first, then we are starting below the split plane
				if (belowFirst)
				{
					firstChild = node + 1;
					secondChild = &m_nodes[node->AboveChild()];
				}
				else
				{
					firstChild = &m_nodes[node->AboveChild()];
					secondChild = node + 1;
				}

				// Depending on where in the box the split occurs, we may not need to check the whole volume
				if (dist_to_split > tMax || dist_to_split <= 0)
				{
					// Split happens after we leave the box, no need to check second
					node = firstChild;
				}
				else if (dist_to_split < tMin)
				{
					// Split happens before we ever enter, no need to check first
					node = secondChild;
				}
				else
				{
					// Split happens in between. Need to check both sides
					todo[todoPos].node = secondChild;
					todo[todoPos].tMin = dist_to_split;
					todo[todoPos].tMax = tMax;
					todoPos++;

					node = firstChild;
					tMax = dist_to_split;
				}
			}
			else
			{
				// This node is a leaf, need to check if we hit any of the contained objects
				int number_objects_in_node = node->nPrimitives();
				if (number_objects_in_node == 1)
				{
					const std::shared_ptr<Object> &obj = objects.at(node->onePrimitive);
					if (obj->shadow_hit(ray, t) && t > m_epsilon)
					{
						hit = true;
					}
				}
				else
				{
					for (int i = 0; i < number_objects_in_node; ++i)
					{
						int index = all_leaf_object_indices[(size_t)node->offset_in_object_indices + i];
						const std::shared_ptr<Object> &obj = objects.at(index);
						if (obj->shadow_hit(ray, t) && t > m_epsilon)
						{
							hit = true;
						}
					}
				}

				// No need to go further down this branch.
				// Retrieve the next branch start
				if (todoPos > 0)
				{
					todoPos--;
					node = todo[todoPos].node;
					tMin = todo[todoPos].tMin;
					tMax = todo[todoPos].tMax;
				}
				else
				{
					// None left. We're done
					break;
				}
			}
		}
		return hit;
	}

	std::vector<std::shared_ptr<poly::object::Object>> KDTree::get_nearest_to_point(atlas::math::Point const& hitpoint, float radius_to_check, std::size_t max_num_points) const
	{
		std::vector<std::shared_ptr<poly::object::Object>> nearest_objects;

		// First, check if we are inside the box at all
		//double tMin, tMax;
		if (!m_bounds.inside_bounds(hitpoint, radius_to_check))
		{
			return nearest_objects;
		}

		constexpr int maxTodo = 512;
		KDToDo todo[maxTodo];
		int todoPos = 0;

		const KDNode* node = &m_nodes[0];
		while (node != nullptr)
		{
			//if (ray.tMax < tMin) break; // Our rays go forever
			// While we aren't at a leaf, go down the list
			if (!node->IsLeaf())
			{
				int axis = node->SplitAxis();
				float dist_to_split = std::abs(node->SplitPos() - hitpoint[axis]);

				const KDNode* firstChild;
				const KDNode* secondChild;

				int belowFirst = hitpoint[axis] < node->SplitPos();

				// If we are below first, then we are starting below the split plane
				if (belowFirst)
				{
					firstChild = node + 1;
					secondChild = &m_nodes[node->AboveChild()];
				}
				else
				{
					firstChild = &m_nodes[node->AboveChild()];
					secondChild = node + 1;
				}


				// Depending on where in the box the split occurs, we may not need to check the whole volume
				if (dist_to_split > radius_to_check)
				{
					// Split happens after we leave the box, no need to check second
					node = firstChild;
				}
				else
				{
					// Split happens in between. Need to check both sides
					todo[todoPos].node = secondChild;
					//todoVec.push_back(secondChild);

					todoPos++;

					node = firstChild;
				}
			}
			else
			{
				// This node is a leaf, need to check if we hit any of the contained objects
				int number_objects_in_node = node->nPrimitives();
				
				atlas::math::Ray ray(hitpoint, atlas::math::Vector(radius_to_check,0.0,0.0));
				poly::structures::SurfaceInteraction sr;

				if (number_objects_in_node == 1)
				{
					const std::shared_ptr<Object>& obj = objects.at(node->onePrimitive);
					if (obj->hit(ray, sr))
					{
						nearest_objects.push_back(obj);
					}
				}
				else
				{
					for (int i = 0; i < number_objects_in_node; ++i)
					{
						int index = all_leaf_object_indices[(size_t)node->offset_in_object_indices + i];
						const std::shared_ptr<Object>& obj = objects.at(index);
						if (obj->hit(ray, sr) && nearest_objects.size() < max_num_points)
						{
							nearest_objects.push_back(obj);
						}
					}
				}

				// No need to go further down this branch.
				// Retrieve the next branch start
				if (todoPos > 0 && nearest_objects.size() < max_num_points)
				{
					todoPos--;
					node = todo[todoPos].node;
				}
				else
				{
					// None left. We're done
					break;
				}
			}
		}
		return nearest_objects;
	}

} // namespace poly::structures
