#pragma once

#include <thread>
#include <mutex>

#include <atlas/math/math.hpp>
#include <atlas/math/ray.hpp>
#include <atlas/math/solvers.hpp>
#include <atlas/utils/load_obj_file.hpp>

#include <fmt/printf.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <vector>
#include <cstdlib>

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cmath>
#include <iostream>

#include "paths.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define UNUSED(c) (void)c

static const std::vector<std::string> IncludeDir{ ShaderPath };

/* 
	REFERENCES:
	===========
	Intersection equations and system structure adapted from:
	"Ray Tracing from the Ground Up"
	by Kevin Suffern (2016)
	https://learning-oreilly-com.ezproxy.library.uvic.ca/library/view/ray-tracing-from/9781498774703/K00474_C019.xhtml#art_Dest_19-5

	KD-Tree implementation and Bounding Boxes adapted from
	"Physicall Based Rendering, From Theory To Implementation"
	by Matt Pharr, Wenzel Jakob, and Greg Humphreys (2004-2019)
	http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Kd-Tree_Accelerator.html
*/

using namespace atlas;
using Colour = math::Vector;

void saveToBMP(std::string const& filename,
	std::size_t width,
	std::size_t height,
	std::vector<Colour> const& image);

Colour random_colour_generate()
{
	unsigned int granularity = 256;
	Colour colour;
	colour.x = (rand() % granularity)
		/ (float)granularity;
	colour.y = (rand() % granularity)
		/ (float)granularity;
	colour.z = (rand() % granularity)
		/ (float)granularity;
	return colour;
}

// Forward declarations
class AcceleratorStruct;
class BRDF;
class BTDF;
class Camera;
class KDTree;
class Light;
class Mapping;
class Material;
class Object;
class Sampler;
class SceneSlab;
class Texture;
class Tracer;
class ViewPlane;
class World;

class Bounds3D {
public:

	math::Vector pMin;
	math::Vector pMax;

	Bounds3D() 
	{
		pMin = math::Vector(0.0f, 0.0f, 0.0f);
		pMax = math::Vector(0.0f, 0.0f, 0.0f);
	}
	Bounds3D(math::Vector _pMin, math::Vector _pMax) : pMin{ _pMin }, pMax{ _pMax }{}

	inline bool get_intersects(const math::Ray<math::Vector>& ray, float* hitt0, float* hitt1) const 
	{
		float t0 = 0;
		float t1 = std::numeric_limits<float>::max();
		for (int i = 0; i < 3; ++i) {
			float invRayDir = 1 / ray.d[i];
			float tNear = (pMin[i] - ray.o[i]) * invRayDir;
			float tFar = (pMax[i] - ray.o[i]) * invRayDir;
			if (tNear > tFar) {
				std::swap(tNear, tFar);
			}

			tFar *= 1.0f + 2.0f * (3.0f * (float)std::numeric_limits<float>::epsilon() * 0.5f)
				/ (1.0f - 3.0f * (float)std::numeric_limits<float>::epsilon() * 0.5f);

			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar < t1 ? tFar : t1;
			if (t0 > t1) {
				return false;
			}

		}
		if (hitt0) { 
			*hitt0 = t0; 
		}
		if (hitt1) {
			*hitt1 = t1;
		}
		return true;
	}

	math::Vector diagonal() const 
	{ 
		return pMax - pMin; 
	}

	float surfaceArea() const 
	{
		math::Vector d = diagonal();
		return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	int maximum_extent() const 
	{
		math::Vector d = diagonal();
		if (d.x > d.y&& d.x > d.z)
			return 0;
		else if (d.y > d.z)
			return 1;
		else
			return 2;
	}

private:
};

class Camera {
public:
	Camera() = default;

	virtual void render_slab(std::shared_ptr<SceneSlab> slab) const = 0;
	virtual void render_scene(World& world) const = 0;

	void eye_set(atlas::math::Point const& eye)
	{
		m_eye = eye;
	}

	void lookat_set(atlas::math::Point const& lookat)
	{
		m_lookat = lookat;
	}

	void upvec_set(atlas::math::Vector const& up)
	{
		m_up = up;
	}

	virtual void uvw_compute()
	{
		atlas::math::Vector w = glm::normalize(m_eye - m_lookat);
		atlas::math::Vector u = glm::normalize(glm::cross(m_up, w));
		atlas::math::Vector v = glm::normalize(glm::cross(w, u));

		this->m_u = u;
		this->m_v = v;
		this->m_w = w;
	};

protected:
	atlas::math::Point m_eye;
	atlas::math::Point m_lookat;
	atlas::math::Point m_up;
	atlas::math::Vector m_u, m_v, m_w;
};

class ViewPlane {
public:
	ViewPlane() = default;
	int hres;
	int vres;
	int gamma;
	unsigned int max_depth;
};

class World {
public:
	Colour m_background; // Default pixel colour

	std::shared_ptr<Sampler> m_sampler; // The sampler for AA

	std::vector<std::shared_ptr<Object>> m_scene; // Objects in our scene

	std::shared_ptr<Light> m_ambient; // Ambient light in our scene (gets handled specially)
	std::vector<std::shared_ptr<Light>> m_lights; // Lights in our scene
	std::vector<Colour> m_image; // Output as 1D array

	std::shared_ptr<ViewPlane> m_vp;
	//std::shared_ptr<AcceleratorStruct> m_acceleratorStructure;
	std::shared_ptr<Tracer> m_tracer;
	
	unsigned int m_slab_size;

	World() /*:m_acceleratorStructure{ nullptr } */{}
	~World() {}

	//void accelerator_set(std::shared_ptr<AcceleratorStruct> acc_struct)
	//{
	//	m_acceleratorStructure = acc_struct;
	//}


};

class SceneSlab {
public:
	std::shared_ptr<World> world;
	std::shared_ptr<std::mutex> storage_mutex;
	std::shared_ptr<std::vector<std::vector<Colour>>> storage;
	int start_x;
	int end_x;
	int start_y;
	int end_y;

	SceneSlab(
		std::shared_ptr<World> _world,
		std::shared_ptr<std::mutex> _storage_mutex,
		std::shared_ptr<std::vector<std::vector<Colour>>> _storage,
		int _start_x,
		int _end_x,
		int _start_y,
		int _end_y)
	{
		world = _world;
		storage_mutex = _storage_mutex;
		storage = _storage;
		start_x = _start_x;
		end_x = _end_x;
		start_y = _start_y;
		end_y = _end_y;
	}


};

class ShadeRec {
public:
	float m_tmin;
	float u, v;
	unsigned int depth;
	atlas::math::Ray<atlas::math::Vector> m_ray;
	World& m_world;
    	std::shared_ptr<Material> m_material;
	Colour m_colour;
    	atlas::math::Normal m_normal;

	ShadeRec(World& world) 
		: m_world(world), m_material{nullptr},
		m_colour{ Colour(0.0f, 0.0f, 0.0f) }, 
		m_normal{math::Vector(0.0f, 0.0f, 0.0f)}
	{
		m_tmin = std::numeric_limits<float>::max();
		depth = 0;
	}

	math::Vector hitpoint_get() const
	{
		return m_ray.o + m_ray.d * m_tmin;
	}
};

class Sampler {
public:
	unsigned int granularity = 5000;

	Sampler() {
		count = 0;
		num_samples = 1;
		num_sets = 1;
	}
	~Sampler() {}

	unsigned int num_samples_get() { return num_samples * num_sets; }
	virtual void generate_samples() = 0;
	virtual std::vector<float> sample_unit_square() = 0;
	virtual std::vector<float> sample_unit_square(const unsigned int) = 0;
	virtual std::vector<float> sample_hemisphere() = 0;
    virtual std::vector<float> sample_hemisphere(const unsigned int) = 0;
	virtual void map_samples_to_hemisphere(const float e) = 0;

protected:
	unsigned int num_samples;
	unsigned int num_sets;
	unsigned int count;
	std::vector<std::vector<float>> samples;
	std::vector<std::vector<float>> hemisphere_samples;
};

class Object {
public:
	std::shared_ptr<Material> m_material;

	Object()
	{
		m_epsilon = 0.001f;
	}
	virtual bool hit(math::Ray<math::Vector>const& R, ShadeRec& sr) const = 0;
	virtual bool shadow_hit(math::Ray<math::Vector>const& R, float& t) const = 0;

	virtual Bounds3D boundbox_get() const
	{
		return bounds;
	}

	void material_set(std::shared_ptr<Material> const& material)
	{
		m_material = material;
	}
	std::shared_ptr<Material> material_get() const
	{
		return m_material;
	}

protected:
	float m_epsilon;
	Bounds3D bounds;
};

/* BEGIN KD TREE */

class AcceleratorStruct : public Object{
public:
	AcceleratorStruct() {}
	virtual bool hit(const math::Ray<math::Vector>& ray, ShadeRec& sr) const = 0;
	virtual bool shadow_hit(const math::Ray<math::Vector>& ray, float& t) const = 0;
	virtual Bounds3D boundbox_get() const = 0;
};

struct KDNode {
public:
	void init_leaf(int* primNums,
		int np,
		std::vector<int>* primitiveIndices)
	{
		flags = 3;
		nPrims |= (np << 2);
		if (np == 0)
			onePrimitive = 0;
		else if (np == 1)
			onePrimitive = primNums[0];
		else {
			offset_in_object_indices = (int)primitiveIndices->size();
			for (int i = 0; i < np; ++i)
				primitiveIndices->push_back(primNums[i]);
		}
	}

	// This node has children if it is an interior node
	void init_interior(int axis, int ac, float s) 
	{
		split = s;
		flags = axis;
		aboveChild |= (ac << 2);
	}

	float SplitPos() const 
	{ 
		return split; 
	}
	int nPrimitives() const 
	{
		return nPrims >> 2; 
	}
	int SplitAxis() const 
	{ 
		return flags & 3; 
	}
	bool IsLeaf() const 
	{ 
		return (flags & 3) == 3; 
	}
	int AboveChild() const 
	{ 
		return aboveChild >> 2; 
	}

	union {
		float split;                  // Interior
		int onePrimitive;             // Leaf
		int offset_in_object_indices;   // Leaf
	};
	union {
		int flags;         // Both
		int nPrims;        // Leaf
		int aboveChild;    // Interior
	};

};

class KDTree : public AcceleratorStruct {
public:
	KDTree(const std::vector<std::shared_ptr<Object>>& p,
		int isectCost, int traversalCost,
		float emptyBonus, int maxPrims,
		int maxDepth)
		: intersectCost(isectCost),
		traversalCost(traversalCost),
		maxPrims(maxPrims),
		emptyBonus(emptyBonus),
		objects(p)
	{
		m_nextFreeNode = 0;
		m_allocatedNodes = 0;
		if (maxDepth <= 0)
			maxDepth = (int)std::round(8 + 1.3f * log(objects.size()) / log(2));

		// Generate the bounding for the tree
		std::vector<Bounds3D> primBounds;
		for (const std::shared_ptr<Object>& obj : objects) {
			Bounds3D b = obj->boundbox_get();
			m_bounds = union_bounds(m_bounds, b);
			primBounds.push_back(b);
		}

		// Working memory so that we are avoiding VLA's
		std::unique_ptr<BoundEdge[]> edges[3];
		for (int i = 0; i < 3; ++i) {
			edges[i].reset(new BoundEdge[2 * objects.size()]);
		}
		std::unique_ptr<int[]> prims0(new int[objects.size()]); // Left branches
		std::unique_ptr<int[]> prims1(new int[((size_t)maxDepth + 1) * objects.size()]); // Right branches

		// Init the trackers for our objects. 
		// Must match order of the storage array
		std::unique_ptr<int[]> primNums(new int[objects.size()]);
		for (size_t i = 0; i < objects.size(); ++i)
			primNums[i] = (unsigned int)i;

		// Start recursive build
		tree_build(0,
			m_bounds,
			primBounds,
			primNums.get(),
			(int)objects.size(),
			maxDepth,
			edges, prims0.get(), prims1.get(), // Working Space
			0);
	}

	Bounds3D boundbox_get() const {
		return m_bounds;
	}

	Bounds3D union_bounds(Bounds3D const& b1,
		Bounds3D const& b2)
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

	struct KDToDo {
		const KDNode* node;
		float tMin, tMax;
	};

	// INTERSECT a ray with the tree
	bool hit(const math::Ray<math::Vector>& ray, ShadeRec& sr) const
	{
		// First, check if we intersect the box at all
		float tMin, tMax;
		if (!m_bounds.get_intersects(ray, &tMin, &tMax)) {
			return false;
		}

		math::Vector invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
		constexpr int maxTodo = 128;
		KDToDo todo[maxTodo];
		int todoPos = 0;

		bool hit = false;
		const KDNode* node = &m_nodes[0];
		while (node != nullptr) {
			//if (ray.tMax < tMin) break; // Our rays go forever
			// While we aren't at a leaf, go down the list
			if (!node->IsLeaf()) {
				int axis = node->SplitAxis();
				float dist_to_split = (node->SplitPos() - ray.o[axis]) * invDir[axis];

				const KDNode* firstChild;
				const KDNode* secondChild;

				int belowFirst = (ray.o[axis] < node->SplitPos())
					|| (ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);

				// If we are below first, then we are starting below the split plane
				if (belowFirst) {
					firstChild = node + 1;
					secondChild = &m_nodes[node->AboveChild()];
				}
				else {
					firstChild = &m_nodes[node->AboveChild()];
					secondChild = node + 1;
				}

				// Depending on where in the box the split occurs, we may not need to check the whole volume
				if (dist_to_split > tMax || dist_to_split <= 0)
				{
					// Split happens after we leave the box, no need to check second
					node = firstChild;
				}
				else if (dist_to_split < tMin) {
					// Split happens before we ever enter, no need to check first
					node = secondChild;
				}
				else {
					// Split happens in between. Need to check both sides
					todo[todoPos].node = secondChild;
					todo[todoPos].tMin = dist_to_split;
					todo[todoPos].tMax = tMax;
					todoPos++;

					node = firstChild;
					tMax = dist_to_split;
				}

			}
			else {
				// This node is a leaf, need to check if we hit any of the contained objects
				int number_objects_in_node = node->nPrimitives();
				if (number_objects_in_node == 1) {
					const std::shared_ptr<Object>& obj = objects.at(node->onePrimitive);
					if (obj->hit(ray, sr)) {
						hit = true;
					}

				}
				else {
					for (int i = 0; i < number_objects_in_node; ++i) {
						int index = all_leaf_object_indices[(size_t)node->offset_in_object_indices + i];
						const std::shared_ptr<Object>& obj = objects.at(index);
						if (obj->hit(ray, sr)) {
							hit = true;
						}

					}
				}

				// No need to go further down this branch.
				// Retrieve the next branch start
				if (todoPos > 0) {
					todoPos--;
					node = todo[todoPos].node;
					tMin = todo[todoPos].tMin;
					tMax = todo[todoPos].tMax;
				}
				else {
					// None left. We're done
					break;
				}
			}
		}
		return hit;
	}

	bool shadow_hit(const math::Ray<math::Vector>& ray, float& t) const
	{
		// First, check if we intersect the box at all
		float tMin, tMax;
		if (!m_bounds.get_intersects(ray, &tMin, &tMax)) {
			return false;
		}

		math::Vector invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
		constexpr int maxTodo = 128;
		KDToDo todo[maxTodo];
		int todoPos = 0;

		bool hit = false;
		const KDNode* node = &m_nodes[0];
		while (node != nullptr) {
			//if (ray.tMax < tMin) break; // Our rays go forever
			// While we aren't at a leaf, go down the list
			if (!node->IsLeaf()) {
				int axis = node->SplitAxis();
				float dist_to_split = (node->SplitPos() - ray.o[axis]) * invDir[axis];

				const KDNode* firstChild;
				const KDNode* secondChild;

				int belowFirst = (ray.o[axis] < node->SplitPos())
					|| (ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);

				// If we are below first, then we are starting below the split plane
				if (belowFirst) {
					firstChild = node + 1;
					secondChild = &m_nodes[node->AboveChild()];
				}
				else {
					firstChild = &m_nodes[node->AboveChild()];
					secondChild = node + 1;
				}

				// Depending on where in the box the split occurs, we may not need to check the whole volume
				if (dist_to_split > tMax || dist_to_split <= 0)
				{
					// Split happens after we leave the box, no need to check second
					node = firstChild;
				}
				else if (dist_to_split < tMin) {
					// Split happens before we ever enter, no need to check first
					node = secondChild;
				}
				else {
					// Split happens in between. Need to check both sides
					todo[todoPos].node = secondChild;
					todo[todoPos].tMin = dist_to_split;
					todo[todoPos].tMax = tMax;
					todoPos++;

					node = firstChild;
					tMax = dist_to_split;
				}

			}
			else {
				// This node is a leaf, need to check if we hit any of the contained objects
				int number_objects_in_node = node->nPrimitives();
				if (number_objects_in_node == 1) {
					const std::shared_ptr<Object>& obj = objects.at(node->onePrimitive);
					if (obj->shadow_hit(ray, t) && t > m_epsilon) {
						hit = true;
					}

				}
				else {
					for (int i = 0; i < number_objects_in_node; ++i) {
						int index = all_leaf_object_indices[(size_t)node->offset_in_object_indices + i];
						const std::shared_ptr<Object>& obj = objects.at(index);
						if (obj->shadow_hit(ray, t) && t > m_epsilon) {
							hit = true;
						}

					}
				}

				// No need to go further down this branch.
				// Retrieve the next branch start
				if (todoPos > 0) {
					todoPos--;
					node = todo[todoPos].node;
					tMin = todo[todoPos].tMin;
					tMax = todo[todoPos].tMax;
				}
				else {
					// None left. We're done
					break;
				}
			}
		}
		return hit;
	}


private:
	const int intersectCost, traversalCost, maxPrims;
	const float emptyBonus;
	std::vector<std::shared_ptr<Object>> objects;
	std::vector<int> all_leaf_object_indices;
	KDNode* m_nodes;
	int m_allocatedNodes, m_nextFreeNode;
	Bounds3D m_bounds;

	enum class EdgeType { Start, End };
	struct BoundEdge {
		BoundEdge() = default;
		BoundEdge(float t, int primNum, bool starting)
			: value(t), primNum(primNum) {
			type = starting ? EdgeType::Start : EdgeType::End;
		}

		float value;
		int primNum;
		EdgeType type;
	};

	void tree_build(int node_index,
		const Bounds3D& node_bounds,
		const std::vector<Bounds3D>& allPrimBounds,
		int* node_object_indices,
		int num_objects,
		int depth,
		const std::unique_ptr<BoundEdge[]> edges[3], int* below_objs_list, int* above_objs_list, // Working space
		int useless_refine_cnt)
	{
		// If the next free node is outside of what we have allocated, reallocate and copy over to a 2N array
		if (m_nextFreeNode == m_allocatedNodes) {
			int new_num_to_allocate = std::max(2 * m_allocatedNodes, 512);
			KDNode* temp_list = (KDNode*)malloc(new_num_to_allocate * sizeof(KDNode));
			if (m_allocatedNodes > 0) {
			    std::memcpy(temp_list, m_nodes, (size_t)m_allocatedNodes * sizeof(KDNode));
				free(m_nodes);
			}
			m_nodes = temp_list;
			m_allocatedNodes = new_num_to_allocate;
		}

		m_nextFreeNode++;

		// If the number of objects in this node is less than our maximum per leaf, or we are at depth 0, stop recursion
		if (num_objects <= maxPrims || depth == 0) {
			// this node gets made into a leaf. Multiple object indices are stored in the 'all_leaf_object_indices' to keep leaves small
			m_nodes[node_index].init_leaf(node_object_indices, num_objects, &all_leaf_object_indices);
			return;
		}

		int bestAxis = -1, bestOffset = -1;
		float bestCost = std::numeric_limits<float>::max(); // This might mean that no other axes are tried!!
		float oldCost = intersectCost * float(num_objects);
		float invTotalSA = 1 / node_bounds.surfaceArea();;
		int num_edges = 2 * num_objects;
		math::Vector bounds_diagonal = node_bounds.pMax - node_bounds.pMin;

		// The axis that we will split on
		int axis = node_bounds.maximum_extent();

		int retries = 0;
		// first try; retries = 0; x-axis attempt
		// second try; retries = 1; y-axis attempt
		// third try; retries = 2; z-axis attempt
		while (retries < 3) {
			//retrySplit:

				// Initialize edges for this axis
			for (int i = 0; i < num_objects; i++) {
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
					else return e0.value < e1.value;
				});

			// Select the split point by using the heuristic weighting
			// adjustment can be done using higher traversal or higher
			// overlap penalties
			int nBelow = 0;
			int nAbove = num_objects;
			int num_axes = 3;
			for (int i = 0; i < num_edges; ++i) {

				// If we have an end, then we have checked one full object
				if (edges[axis][i].type == EdgeType::End) {
					--nAbove;
				}

				// If this edge is strictly inside bounds
				float current_edge = edges[axis][i].value;
				if (current_edge > node_bounds.pMin[axis]
					&& current_edge < node_bounds.pMax[axis]) {
					int otherAxis0 = (axis + 1) % num_axes;
					int otherAxis1 = (axis + 2) % num_axes;

					// Compute the area (volume) below and above the split
					float area_below = 2 * (
						bounds_diagonal[otherAxis0] * bounds_diagonal[otherAxis1]
						+ (current_edge - node_bounds.pMin[axis])
						* (bounds_diagonal[otherAxis0] + bounds_diagonal[otherAxis1]));
					float area_above = 2 * (
						bounds_diagonal[otherAxis0] * bounds_diagonal[otherAxis1]
						+ (node_bounds.pMax[axis] - current_edge)
						* (bounds_diagonal[otherAxis0] + bounds_diagonal[otherAxis1]));

					// Probability of being below or above
					float pBelow = area_below * invTotalSA;
					float pAbove = area_above * invTotalSA;

					// If either is 0, then we get a small bonus for 
					// having a split that empties a large amount of area
					float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0.0f;

					// Compute the cost of this split.
					float cost = traversalCost + intersectCost
						* (1 - eb) * (pBelow * nBelow + pAbove * nAbove);
					if (cost < bestCost) {
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

			if (bestAxis == -1) { // This will never be hit! The float is ININITY...so EVERYTHING will select a best axis
				retries++;
				axis = (axis + 1) % num_axes;
			}
			else {
				break;
			}
		}

		// If the cost of this refinement is worse than before we refined, 
		// increment the tracker so if we go too far we can stop splitting
		if (bestCost > oldCost) {
			useless_refine_cnt++;
		}

		// If the cost for this split is 'bad' make this a leaf and be done with it
		if ((bestCost > 4 * oldCost && num_objects < 16)
			|| bestAxis == -1
			|| useless_refine_cnt >= 3) {
			m_nodes[node_index].init_leaf(node_object_indices, num_objects, &all_leaf_object_indices);
			return;
		}

		// Move the primitives for this list into the correct side of the split
		unsigned int num_objects_below = 0;
		unsigned int num_objects_above = 0;
		for (int i = 0; i < bestOffset; ++i) {
			if (edges[bestAxis][i].type == EdgeType::Start) {
				below_objs_list[num_objects_below++] = edges[bestAxis][i].primNum;
			}
		}
		for (int i = bestOffset + 1; i < num_edges; ++i) {
			if (edges[bestAxis][i].type == EdgeType::End) {
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
};

class BRDF {
public:
	BRDF() = default;
	virtual Colour f(ShadeRec const& sr,
		atlas::math::Vector& w_o,
		atlas::math::Vector& w_i) const = 0;
	virtual Colour rho(ShadeRec const& sr,
		atlas::math::Vector& w_o) const = 0;
};

class BTDF {
public:
	BTDF(float kt, float ior)
	{
		m_kt = kt;
		m_index_refraction = ior;
	}

	virtual bool tot_int_refl(ShadeRec const& sr) const = 0;
	virtual Colour f(ShadeRec const& sr,
		atlas::math::Vector& w_o,
		atlas::math::Vector& w_i) const = 0;
	virtual Colour sample_f(ShadeRec const& sr,
		atlas::math::Vector& w_o,
		atlas::math::Vector& w_t) const = 0;
	virtual Colour rho(ShadeRec const& sr,
		atlas::math::Vector& w_o) const = 0;
protected:
	float m_index_refraction;
	float m_kt;
};

class Material {
public:
	Material() = default;
	virtual Colour shade(ShadeRec& sr) const = 0;
};

class Texture {
public:
	Texture() = default;
	virtual Colour colour_get(ShadeRec const& sr) const = 0;
};

class Tracer {
public:
	Tracer() = default;
	Tracer(World* _world) 
		: m_world{_world}
	{
		
	}

	//virtual Colour trace_ray(math::Ray<math::Vector> const& ray) const;
	virtual Colour trace_ray([[maybe_unused]]math::Ray<math::Vector> const& ray, [[maybe_unused]] const unsigned int depth) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}
	//virtual Colour trace_ray(math::Ray<math::Vector> const& ray, float& tmin, const unsigned int depth) const;
protected:
	World* m_world;
};

class Light {
public:
	Light() : m_ls{ 1.0f }, m_colour{ 1.0f, 1.0f, 1.0f }, m_surface_epsilon{ 0.001f } {}
	virtual atlas::math::Vector direction_get(ShadeRec& sr) = 0;
	virtual Colour L(ShadeRec& sr) = 0;
	
	void radiance_scale(float b)
	{
		m_ls = b;
	}

	void colour_set(Colour const& c) 
	{
		m_colour = c;
	}

protected:

	float m_ls;
	Colour m_colour;
	float m_surface_epsilon;

	virtual bool in_shadow(math::Ray<math::Vector> const& shadow_ray,
		ShadeRec const& sr)
	{
		float t;
		for (std::shared_ptr<Object> object : sr.m_world.m_scene) {
			if (object->shadow_hit(shadow_ray, t) && t > m_surface_epsilon) {
				return true;
			}
		}
		return false;
	}
};

class Mapping {
public:
	Mapping() = default;
	virtual void texel_coord_get(math::Point hitpoint, int hres, int vres, int& row, int& col) const = 0;
};


class AmbientLight : public Light {
public:
	AmbientLight() : Light() {}

	atlas::math::Vector direction_get([[maybe_unused]] ShadeRec& sr)
	{
		return atlas::math::Vector(0.0f, 0.0f, 0.0f);
	}

	Colour L([[maybe_unused]] ShadeRec& sr)
	{
		return m_colour * m_ls;
	}
};

class AmbientOcclusion : public Light {
public:
	AmbientOcclusion() : Light()
	{
		min_amount = Colour(1.0f, 1.0f, 1.0f);
	}

	void min_amount_set(float amount)
	{
		min_amount = Colour(1.0f, 1.0f, 1.0f) * amount;
	}

	void sampler_set(std::shared_ptr<Sampler> s_ptr, float tightness)
	{
		m_sampler = s_ptr;
		m_sampler->map_samples_to_hemisphere(tightness);
	}

	math::Vector direction_get([[maybe_unused]] ShadeRec& sr)
	{
		return math::Vector(0.0f, 0.0f, 0.0f);
	}

	// Specific to ambient occlusion, we need access to samples mapped to hemisphere
	math::Vector shadow_direction_get([[maybe_unused]] ShadeRec& sr, unsigned int sample_index)
	{
		// Take any one index of our samplers samples
		// This enables us to have multiple threads access the samples
		// without needing to guard access with mutexes
		std::vector<float> sp = m_sampler->sample_hemisphere(sample_index);

		// For multithreading, its important that these aren't global
		atlas::math::Vector w = sr.m_normal;
		atlas::math::Vector u = glm::normalize(glm::cross(math::Vector(0.0f, 1.0f, 0.0f), w));
		atlas::math::Vector v = glm::normalize(glm::cross(w, u));

		return (sp.at(0) * u
			+ sp.at(1) * v
			+ sp.at(2) * w);
	}

	Colour L(ShadeRec& sr)
	{
		// Only build the occlusion shadows for the first hitpoints of locations
		if (sr.depth != 0) {
			return m_colour * m_ls;
		}

		int num_samples = m_sampler->num_samples_get();
		Colour average(0.0f, 0.0f, 0.0f);

		// Update the hitpoint coordinate system
		math::Point new_origin = sr.hitpoint_get();

		for (int i = 0; i < num_samples; i++) {
			math::Vector new_dir = glm::normalize(shadow_direction_get(sr, i));
			math::Ray shadow_ray(new_origin, new_dir);

			// in_shadow consumes the bulk of the processing power
			if (in_shadow(shadow_ray, sr)) {
				average += min_amount * m_colour * m_ls;
			}
			else {
				average += m_colour * m_ls;
			}
		}
		return average / (float)num_samples;
	}

protected:
	std::shared_ptr<Sampler> m_sampler;
	Colour min_amount;
};

class DirectionalLight : public Light
{
public:

	DirectionalLight() : Light()
	{
		m_direction = atlas::math::Vector(0.0f, 0.0f, 1.0f);
	}

	void direction_set(atlas::math::Vector const& direction)
	{
		m_direction = glm::normalize(direction);
	}

	atlas::math::Vector direction_get([[maybe_unused]]ShadeRec& sr)
	{
		return m_direction;
	}

	Colour L(ShadeRec& sr)
	{
		math::Point new_origin = sr.hitpoint_get();
		math::Vector new_direction = glm::normalize(direction_get(sr));

		math::Ray shadow_ray(new_origin	+ (m_surface_epsilon* new_direction),
			new_direction);

		if (in_shadow(shadow_ray, sr)) {
			return Colour(0.0f, 0.0f, 0.0f);
		}
		else {
			return m_colour * m_ls;
		}
	}

protected:
	atlas::math::Vector m_direction;
};

class PointLight : public Light
{
public:

	PointLight() : Light()
	{
		m_location = atlas::math::Vector(0.0f, 0.0f, 0.0f);
	}

	PointLight(math::Vector const& location) : Light()
	{
		m_location = location;
	}

	void location_set(atlas::math::Point const& location)
	{
		m_location = glm::normalize(location);
	}

	atlas::math::Vector direction_get(ShadeRec& sr)
	{
		math::Vector surface_point = sr.hitpoint_get();
		return glm::normalize(m_location - surface_point);
	}

	bool in_shadow(math::Ray<math::Vector> const& shadow_ray,
		ShadeRec const& sr)
	{
		float t{std::numeric_limits<float>::max()};

		// Max distance between hitpoint and light
		math::Vector line_between = m_location - shadow_ray.o;
		float line_distance = sqrt(glm::dot(line_between, line_between));

		for (std::shared_ptr<Object> object : sr.m_world.m_scene) {
			// If we hit an object with distance less than max
			if (object->shadow_hit(shadow_ray, t) && t < line_distance) {
				return true;
			}
		}

		return false;
	}

	Colour L(ShadeRec& sr)
	{
		math::Point new_origin = sr.hitpoint_get();
		math::Vector new_direction = glm::normalize(direction_get(sr));

		math::Ray shadow_ray(new_origin
			+ (m_surface_epsilon
				* new_direction),
			new_direction);
		if (in_shadow(shadow_ray, sr)) {
			return Colour(0.0f, 0.0f, 0.0f);
		}
		else {
			math::Vector vector = glm::normalize(m_location
				- sr.hitpoint_get());
			float r_squared = glm::dot(vector, vector);
			return m_colour * m_ls / (r_squared);
		}
	}

protected:
	atlas::math::Point m_location;
};

//class RectMapping : public Mapping {
//public:
//	RectMapping(std::vector<math::Point> const& points){
//		assert(points.size() == 3);
//		o = points.at(0);
//		r = points.at(1);
//		u = points.at(2);
//	}
//	void texel_coord_get(math::Point hitpoint, int hres, int vres, int& row, int& col) const 
//	{
//		math::Vector offset = hitpoint - o;
//		float u = glm::dot(offset, r);
//		float v = glm::dot(offset, u);
//
//	}
//protected:
//	math::Vector o, u, r;
//};

// Constant colour is the default texture (really, NO texture)
class ConstantColour : public Texture {
public:
	ConstantColour() = default;
	ConstantColour(Colour const& c) :m_colour{c} {}
	Colour colour_get([[maybe_unused]] ShadeRec const& sr) const {
		return m_colour;
	}
	void colour_set(Colour const& c) {
		m_colour = c;
	}
private:
	Colour m_colour;
};

class ImageTexture : public Texture {
public:
	ImageTexture(std::string const& s) {
		int n;
		unsigned char* stbi_data = stbi_load(s.c_str(), &hres, &vres, &n, 3);

		if (stbi_data == nullptr) {
			std::cout << "ERROR: file '" << s << "' was not in the path" << std::endl;
			exit(-1);
		}
		
		// Create colours from the image data
		std::vector<Colour> vec_dat = std::vector<Colour>(hres * (size_t)vres);
		image_data = std::vector<std::vector<Colour>>(vres, std::vector<Colour>(hres));
		for (int i = 0; i < (int)vec_dat.size(); i++) {
			for (int j = 0; j < n; j++){ 
				vec_dat[i][j] = static_cast<float>(stbi_data[i * n + j]) / 255.0f;
			}
		}
		
		stbi_image_free(stbi_data);

		// Create a 2D vector to simplify
		for (int i = 0; i < vres; i++) {
			for (int j = 0; j < hres; j++) {
				image_data.at(vres - i - 1).at(j) = vec_dat[i * hres + j];
			}
		}
	}

	Colour colour_get([[maybe_unused]] ShadeRec const& sr) const {

		int row, col;
		if (mapper) {
			mapper->texel_coord_get(sr.hitpoint_get(), hres, vres, row, col);
		}
		else {
			row = (int)(sr.v * (vres - 1)) % (vres - 1);
			col = (int)(sr.u * (hres - 1)) % (hres - 1);
		}

		return image_data.at(row).at(col);
	}
private:
	int hres, vres;
	std::vector<std::vector<Colour>> image_data;
	std::shared_ptr<Mapping> mapper;
};

class WhittedTracer : public Tracer {
public:
	WhittedTracer(World* _world) :Tracer(_world) {}

	Colour trace_ray(math::Ray<math::Vector> const& ray, const unsigned int depth) const
	{
		if (depth > m_world->m_vp->max_depth) {
			return Colour(0.0f, 0.0f, 0.0f);
		}
		else {
			ShadeRec temp_sr = ShadeRec(*(m_world));
			bool did_hit = false;
			for (auto obj : m_world->m_scene) {
				if (obj->hit(ray, temp_sr)) {
					did_hit = true;
				}
			}

			// If this ray hit an object, return material's shading
			if (did_hit && temp_sr.m_material != nullptr) {
				temp_sr.depth = depth;
				temp_sr.m_ray = ray;
				return temp_sr.m_material->shade(temp_sr);
			}
			else {
				return m_world->m_background;
			}
		}
	}
};

class LambertianBRDF : public BRDF {
public:
	LambertianBRDF()
	{
		m_kd = 1.0f;
		m_cd = random_colour_generate();
	}

	LambertianBRDF(const float kd, Colour const& colour)
	{
		m_kd = kd;
		m_cd = colour;
	}

	Colour f([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o,
		[[maybe_unused]] atlas::math::Vector& w_i) const
	{
		return m_kd * m_cd * glm::one_over_pi<float>();
	}

	Colour rho([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o) const
	{
		return (Colour)(m_cd * m_kd);
	}

protected:
	float m_kd;
	Colour m_cd;
};

class PerfectSpecular : public BRDF {
public:
	PerfectSpecular()
	{
		m_kd = 1.0f;
		m_cd = random_colour_generate();
	}

	PerfectSpecular(const float kd, Colour const& colour)
	{
		m_kd = kd;
		m_cd = colour;
	}

	Colour f([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o,
		[[maybe_unused]] atlas::math::Vector& w_i) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}

	Colour sample_f([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o,
		[[maybe_unused]] atlas::math::Vector& w_r) const
	{
		float ndotwo = glm::dot(sr.m_normal, w_o);
		w_r = glm::normalize(-w_o + (2.0f * sr.m_normal * ndotwo));
		return m_kd * m_cd / (fabs(glm::dot(sr.m_normal, w_r)));
	}

	Colour rho([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}

protected:
	float m_kd;
	Colour m_cd;
};

class GlossySpecularBRDF : public BRDF {
public:
	GlossySpecularBRDF()
	{
		m_kd = 1.0f;
		m_cd = random_colour_generate();
		m_exp = 1.0f;
	}

	GlossySpecularBRDF(const float kd, Colour const& colour, float exp)
	{
		m_kd = kd;
		m_cd = colour;
		m_exp = exp;
	}

	Colour f([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o,
		[[maybe_unused]] atlas::math::Vector& w_i) const
	{
		Colour L(0.0f,0.0f,0.0f);
		float n_dot_wi = glm::dot(w_i, sr.m_normal);
		math::Vector r(-w_i + 2.0f * sr.m_normal * n_dot_wi);
		float r_dot_wo = glm::dot(w_o, r);

		if (r_dot_wo > 0.0f) {
			L = m_kd * m_cd * pow(r_dot_wo, m_exp);
		}

		return L;
	}

	Colour rho([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}

protected:
	float m_kd;
	Colour m_cd;
	float m_exp;
};

class SV_LambertianBRDF : public BRDF{
public:
	SV_LambertianBRDF(const float kd, std::shared_ptr<Texture> tex)
	{
		m_kd = kd;
		m_cd = tex;
	}

	Colour f([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o,
		[[maybe_unused]] atlas::math::Vector& w_i) const
	{
		return m_kd * m_cd->colour_get(sr) * glm::one_over_pi<float>();
	}

	Colour rho([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o) const
	{
		return (Colour)(m_cd->colour_get(sr) * m_kd);
	}

protected:
	float m_kd;
	std::shared_ptr<Texture> m_cd;
};

class PerfectTransmitter : public BTDF {
public:
	PerfectTransmitter() : BTDF(1.0f, 1.0f) {}
	PerfectTransmitter(float kt, float _ior) : BTDF(kt, _ior){}

	bool tot_int_refl(ShadeRec const& sr) const 
	{
		math::Vector w_o = -sr.m_ray.d;
		float cti = glm::dot(w_o, sr.m_normal);
		float eta = m_index_refraction;

		if (cti < 0.0f) {
			eta = 1.0f / eta;
		}

		return (1.0f - ((1.0f - cti * cti) / (eta * eta))) < 0.0f;
	}
	Colour f([[maybe_unused]]ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o,
		[[maybe_unused]] atlas::math::Vector& w_i) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}

	Colour sample_f(ShadeRec const& sr,
		atlas::math::Vector& w_o,
		atlas::math::Vector& w_t) const
	{
		math::Vector normal = sr.m_normal;
		float cti = glm::dot(sr.m_normal, w_o);
		float eta = m_index_refraction;

		if (cti < 0.0f) {
			normal = -normal;
			cti = -cti;
			eta = 1.0f / eta;
		}

		float temp = 1.0f - ((1.0f - cti * cti) / (eta * eta));
		float ct2 = sqrt(temp);
		w_t = (-w_o / eta) - (ct2 - (cti / eta)) * normal;
		Colour retcol = (m_kt / (eta * eta)) * (Colour(1.0f, 1.0f, 1.0f) * (1.0f / (abs(glm::dot(sr.m_normal, w_t)))));
		return retcol;

	}

	Colour rho([[maybe_unused]] ShadeRec const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}
};

class Matte : public Material {
public:
	Matte() {
		m_diffuse = std::make_shared<LambertianBRDF>(
			1.0f,
			random_colour_generate());
	}
	Matte(float f, Colour const& c)
	{
		m_diffuse = std::make_shared<LambertianBRDF>(f, c);
	}

protected:

	std::shared_ptr<LambertianBRDF> m_diffuse;

	Colour shade(ShadeRec& sr) const {
		// Render loop
		Colour r = Colour(0.0f, 0.0f, 0.0f);
		Colour a;
		atlas::math::Vector nullVec(0.0f, 0.0f, 0.0f);

		if (sr.m_world.m_ambient) {
			a = m_diffuse->rho(sr, nullVec)
				* sr.m_world.m_ambient->L(sr);
		}

		for (std::shared_ptr<Light> light : sr.m_world.m_lights) {
			Colour brdf = m_diffuse->f(sr, nullVec, nullVec);
			Colour L = light->L(sr);
			float angle = glm::dot(sr.m_normal,
				light->direction_get(sr));
			if (angle > 0) {
				r += (brdf
					* L
					* angle);
			}
			else {
				r += Colour(0.0f, 0.0f, 0.0f);
			}
		}

		return (a + r);
	}
};

class Phong : public Material {
public:
	Phong() {
		m_diffuse = std::make_shared<LambertianBRDF>(
			1.0f,
			random_colour_generate());
		m_specular = std::make_shared<GlossySpecularBRDF>(
			1.0f,
			random_colour_generate(),
			1.0f);
	}
	Phong(float f_diffuse, float f_spec, Colour c, float exp)
	{
		m_diffuse = std::make_shared<LambertianBRDF>(f_diffuse, c);
		m_specular = std::make_shared<GlossySpecularBRDF>(f_spec,c,exp);
	}

protected:

	std::shared_ptr<LambertianBRDF> m_diffuse;
	std::shared_ptr<GlossySpecularBRDF> m_specular;

	virtual Colour shade(ShadeRec& sr) const {
		// Render loop
		Colour r = Colour(0.0f, 0.0f, 0.0f);
		Colour a;
		atlas::math::Vector nullVec(0.0f, 0.0f, 0.0f);

		if (sr.m_world.m_ambient) {
			a = m_diffuse->rho(sr, nullVec)
				* sr.m_world.m_ambient->L(sr);
		}

		math::Vector w_o = -sr.m_ray.d;
		for (std::shared_ptr<Light> light : sr.m_world.m_lights) {
			Colour L = light->L(sr);
			math::Vector w_i = light->direction_get(sr);
			
			float angle = glm::dot(sr.m_normal, w_i);
			if (angle >= 0) {
				r += ((m_diffuse->f(sr, nullVec, nullVec) 
					+ m_specular->f(sr, w_o, w_i))
					* L
					* angle);
			}
			else {
				r += Colour(0.0f, 0.0f, 0.0f);
			}
		}

		return (a + r);
	}
};

class SV_Matte : public Material {
public:
	SV_Matte(float f, std::shared_ptr<Texture> tex)
	{
		m_diffuse = std::make_shared<SV_LambertianBRDF>(f, tex);
	}

	SV_Matte(float f, Colour const& c)
	{
		m_diffuse = std::make_shared<SV_LambertianBRDF>(f, std::make_shared<ConstantColour>(c));
	}

	SV_Matte(float f, std::string const& s)
	{
		m_diffuse = std::make_shared<SV_LambertianBRDF>(f, std::make_shared<ImageTexture>(s));
	}
protected:

	std::shared_ptr<SV_LambertianBRDF> m_diffuse;

	Colour shade(ShadeRec& sr) const {
		// Render loop
		Colour r = Colour(0.0f, 0.0f, 0.0f);
		Colour a;
		atlas::math::Vector nullVec(0.0f, 0.0f, 0.0f);

		if (sr.m_world.m_ambient) {
			a = m_diffuse->rho(sr, nullVec)
				* sr.m_world.m_ambient->L(sr);
		}

		for (std::shared_ptr<Light> light : sr.m_world.m_lights) {
			Colour brdf = m_diffuse->f(sr, nullVec, nullVec);
			Colour L = light->L(sr);
			float angle = glm::dot(sr.m_normal,
				light->direction_get(sr));
			if (angle >= 0) {
				r += (brdf
					* L
					* angle);
			}
			else {
				r += Colour(0.0f, 0.0f, 0.0f);
			}
		}

		return (a + r);
	}
};

class Transparent : public Phong {
public:
	Transparent() 
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>();
		m_transmitted_btdf = std::make_shared<PerfectTransmitter>();
	}
	Transparent(const float amount_refl,
		const float amount_trans,
		float f_diffuse, 
		float f_spec, 
		Colour const& _colour, 
		float _ior, 
		float _exp)
		: Phong(f_diffuse, f_spec, _colour, _exp)
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>(amount_refl, _colour);
		m_transmitted_btdf = std::make_shared<PerfectTransmitter>(amount_trans, _ior);
	}
	Colour shade(ShadeRec& sr) const
	{
		Colour L = Phong::shade(sr);
		math::Vector w_o = -sr.m_ray.d;
		math::Vector w_r;

		// Get the reflected colour and direction of the reflection
		Colour reflected_colour = m_reflected_brdf->sample_f(sr, w_o, w_r);
		math::Ray<math::Vector> reflected_ray(sr.hitpoint_get(), w_r);

		// If we have internal reflection, then the ray does not transmit
		if (m_transmitted_btdf->tot_int_refl(sr)) {
			L += sr.m_world.m_tracer->trace_ray(reflected_ray, sr.depth + 1);
		}
		else {
			L += reflected_colour
				* sr.m_world.m_tracer->trace_ray(reflected_ray, sr.depth + 1)
				* fabs(glm::dot(sr.m_normal, w_r));

			math::Vector w_t;
			Colour transmitted_colour = m_transmitted_btdf->sample_f(sr, w_o, w_t);
			math::Ray<math::Vector> transmitted_ray(sr.hitpoint_get(), w_t);
			L += transmitted_colour
				* sr.m_world.m_tracer->trace_ray(transmitted_ray, sr.depth + 1)
				* fabs(glm::dot(sr.m_normal, w_t));
		}

		return L;
	}
protected:
	std::shared_ptr<PerfectSpecular> m_reflected_brdf;
	std::shared_ptr<PerfectTransmitter> m_transmitted_btdf;
};

class Reflective : public Phong {
public:
	Reflective()
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>();
	}
	Reflective(const float amount_refl,
		float f_diffuse,
		float f_spec,
		Colour const& _colour,
		float _exp)
		: Phong(f_diffuse, f_spec, _colour, _exp)
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>(amount_refl, _colour);
	}
	Colour shade(ShadeRec& sr) const
	{
		Colour L = Phong::shade(sr);
		math::Vector w_o = -sr.m_ray.d;
		math::Vector w_r;

		// Get the reflected colour and direction of the reflection
		Colour reflected_colour = m_reflected_brdf->sample_f(sr, w_o, w_r);
		math::Ray<math::Vector> reflected_ray(sr.hitpoint_get(), w_r);

		L += reflected_colour
			* sr.m_world.m_tracer->trace_ray(reflected_ray, sr.depth + 1)
			* fabs(glm::dot(sr.m_normal, w_r));		

		return L;
	}
protected:
	std::shared_ptr<PerfectSpecular> m_reflected_brdf;
};

class AA_Jittered : public Sampler
{
public:

	AA_Jittered()
	{
		generate_samples();
	}

	AA_Jittered(unsigned int num_samp, unsigned int num_set)
	{
		num_samples = num_samp;
		num_sets = num_set;
		generate_samples();
	}

	void generate_samples()
	{
		unsigned int n = (int)sqrt(num_samples);
		
		// Ensure we're using a perfect square
		assert(n * n == num_samples);

		for (unsigned int set = 0; set < num_sets; set++) {
			for (unsigned int j = 0; j < n; j++) {
				for (unsigned int i = 0; i < n; i++) {
					float new_x = ((float)j
						+ ((float)(rand() % granularity))
						/ ((float)granularity))
						/ (float)n;
					float new_y = ((float)i
						+ ((float)(rand() % granularity))
						/ ((float)granularity))
						/ (float)n;
					std::vector<float> sample = { new_x, new_y };
					samples.push_back(sample);
				}
			}
		}
	}

	std::vector<float> sample_unit_square()
	{
		count = ((count + 1) % (num_samples * num_sets));
		return samples.at(count);
	}

	std::vector<float> sample_unit_square(const unsigned int index)
	{
		return samples.at(index % (num_samples * num_sets));
	}

	std::vector<float> sample_hemisphere() 
	{
		count = ((count + 1) % (num_samples * num_sets));
		return hemisphere_samples.at(count);
	}
	
	std::vector<float> sample_hemisphere(const unsigned int index)
	{
		// Ensures that we are only reading (for multithreading)
		return hemisphere_samples.at(index % (num_samples * num_sets));
	}

	void map_samples_to_hemisphere(const float e)
	{
		int size = (int)samples.size();
		hemisphere_samples.reserve(num_samples * (size_t)num_sets);
		for (int i = 0; i < size; i++) {
			float cos_phi = (float)cos(2.0f * (float)glm::pi<float>() * samples.at(i).at(0));
			float sin_phi = (float)sin(2.0f * (float)glm::pi<float>() * samples.at(i).at(0));
			float cos_theta = (float)pow((float)(1.0f - samples.at(i).at(1)),
				(float)(1.0f / (e + 1.0f)));
			float sin_theta = (float)sqrt(1.0f - (cos_theta * (float)cos_theta));
			float pu = sin_theta * cos_phi;
			float pv = sin_theta * sin_phi;
			float pw = cos_theta;

			hemisphere_samples.push_back(std::vector<float>{pu, pv, pw});
		}
	}

};

class Sphere : public Object {
public:
	Sphere(math::Vector position, float radius)
	{
		this->r = radius;
		this->C = position;

		// Create the bounds
		bounds = Bounds3D(
			math::Vector(
				position.x - r,
				position.y - r, 
				position.z - r), 
			math::Vector(
				position.x + r,
				position.y + r,
				position.z + r));
	}

	Bounds3D boundbox_get() const 
	{
		return bounds;
	};

	math::Vector normal_get(math::Ray<math::Vector>const& R,
		float t) const
	{
		math::Vector point = R.o + t * R.d;
		return glm::normalize(point - C);
	}

	bool hit(math::Ray<math::Vector>const& R,
		ShadeRec& sr) const
	{
		float t{ std::numeric_limits<float>::max() };
		bool intersect = this->closest_intersect_get(R, t);

		// If this object is hit, set the shaderec with the relevant material and information about the hit point
		if (intersect && t < sr.m_tmin) {
			sr.m_normal = normal_get(R, t);
			sr.m_ray = R;
			sr.m_tmin = t;
			sr.m_material = m_material;
		}

		return intersect;
	}

	bool shadow_hit(math::Ray<math::Vector>const& R,
		float& t) const
	{
		float temp_t;
		if (this->closest_intersect_get(R, temp_t) && temp_t < t && temp_t > m_epsilon) {
			t = temp_t;
			return true;
		}
		return false;
		//return this->closest_intersect_get(R,t);
	}

	bool closest_intersect_get(const math::Ray<math::Vector>& R,
		float& t_min) const
	{
		float a = (float)glm::dot(R.d, R.d);
		float b = (float)2.0f * glm::dot(R.d, R.o - this->C);
		float c = (float)glm::dot(R.o - this->C, R.o - this->C)
			- (float)(this->r * this->r);

		float disc = b * b - (4.0f * a * c);
		if (zeus::geq((float)disc, 0.0f)) {
			const float e = std::sqrt(disc);
			const float denom = 2.0f * a;

			const float eps = 0.1f;
			// Look at the negative root first
			float t = (-b - e) / denom;
			if (zeus::geq(t, eps))
			{
				t_min = t;
				return true;
			}

			// Now the positive root
			t = (-b + e) / denom;
			if (zeus::geq(t, eps))
			{
				t_min = t;
				return true;
			}
		}
		return false;
	}

private:
	math::Vector C; // Center of the sphere
	double r; // Radius of the sphere
};

class Torus : public Object {

public:
	Torus(math::Vector center, float _a, float _b)
	{
		this->center = center;
		m_a = _a;
		m_b = _b;

		// Create the bounds
		bounds = Bounds3D(
			math::Vector(
				center.x - _a - _b,
				center.y - _b,
				center.z - _a - _b),
			math::Vector(
				center.x + _a + _b,
				center.y + _b,
				center.z + _a + _b));
	}

	Bounds3D boundbox_get() const
	{
		return bounds;
	};

	math::Vector normal_get(math::Ray<math::Vector> const& R,
		float t) const
	{
		math::Vector point = (R.o - center) + t * R.d;
		float xyz_squared = point.x * point.x
			+ point.y * point.y
			+ point.z * point.z;
		float ab_squared = m_a * m_a
			+ m_b * m_b;
		float n_x = 4 * point.x * (xyz_squared - ab_squared);
		float n_y = 4 * point.y * (xyz_squared - ab_squared
			+ (2 * m_a * m_a));
		float n_z = 4 * point.z * (xyz_squared - ab_squared);
		math::Vector normal = glm::normalize(math::Vector(n_x, n_y, n_z));
		return normal;
	}

	std::vector<double> rcoeffs_get(math::Ray<math::Vector>const& R) const
	{
		math::Vector O = R.o;
		math::Vector D = glm::normalize(R.d);

		// Allows us to move the torus around the screen
		O.x -= this->center.x;
		O.y -= this->center.y;
		O.z -= this->center.z;

		double sum_d_sqrd = glm::dot(D, D);
		double e = glm::dot(O, O)
			- (m_a * (double)m_a)
			- (m_b * (double)m_b);
		double od = glm::dot(O, D);
		double four_a_sqrd = 4.0 * m_a * m_a;

		// Coefficients
		double c_4 = sum_d_sqrd * sum_d_sqrd;

		double c_3 = 4.0 * sum_d_sqrd * od;

		double c_2 = 2.0 * sum_d_sqrd * e
			+ 4.0 * od * od
			+ four_a_sqrd * (double)D.y * D.y;

		double c_1 = 4.0 * od * e + 2.0 * four_a_sqrd * O.y * D.y;

		double c_0 = e * e
			- four_a_sqrd * ((double)m_b * m_b - (double)O.y * O.y);

		return std::vector<double>{c_0, c_1, c_2, c_3, c_4};
	}

	bool hit(math::Ray<math::Vector>const& R,
		ShadeRec& sr) const
	{
		float t{ std::numeric_limits<float>::max() };
		bool intersect = this->closest_intersect_get(R, t);

		// If this object is hit, set the shaderec with the relevant material and information about the hit point
		if (intersect && t < sr.m_tmin) {
			sr.m_normal = normal_get(R, t);
			sr.m_ray = R;
			sr.m_tmin = t;
			sr.m_material = m_material;
		}

		return intersect;
	}

	bool shadow_hit(math::Ray<math::Vector>const& R,
		float& t) const
	{
		float temp_t;
		if (this->closest_intersect_get(R, temp_t) && temp_t < t && t > m_epsilon) {
			t = temp_t;
			return true;
		}
		else {
			return false;
		}
	}

	bool closest_intersect_get(math::Ray<math::Vector>const& R,
		float& t_min) const
	{
		std::vector<double> coeffs = rcoeffs_get(R);
		std::vector<double> roots;
		math::solve_quartic(coeffs, roots);

		double curr_min = DBL_MAX;
		bool intersect = false;

		for (double root : roots)
		{
			if (root < curr_min && root > 0)
			{
				curr_min = root;
				intersect = true;
			}
		}

		t_min = (float)curr_min;

		return intersect;
	}

private:

	float m_a;
	float m_b;
	math::Vector center;

};

class Plane : public Object {

public:

	Plane(math::Vector normal, math::Vector position)
	{
		this->normal = glm::normalize(normal);
		this->position = position;

		if (this->normal == math::Vector(0.0f, 1.0f, 0.0f)) {
			// Create the bounds
			bounds = Bounds3D(
				math::Vector(
					-std::numeric_limits<float>::max(),
					this->position.y - m_epsilon,
					-std::numeric_limits<float>::max()),
				math::Vector(
					std::numeric_limits<float>::max(),
					this->position.y + m_epsilon,
					std::numeric_limits<float>::max()));
		}
		else {
			// Create the bounds
			bounds = Bounds3D(
				math::Vector(
					std::numeric_limits<float>::min(),
					std::numeric_limits<float>::min(),
					std::numeric_limits<float>::min()),
				math::Vector(
					std::numeric_limits<float>::max(),
					std::numeric_limits<float>::max(),
					std::numeric_limits<float>::max()));
		}

		
	}

	bool closest_intersect_get(math::Ray<math::Vector>const& R,
		float& t_min) const
	{
		float d = glm::dot(this->normal, this->position);
		float num = d - glm::dot(this->normal, R.o);
		float den = glm::dot(this->normal, R.d);

		if (den < m_epsilon && den > -m_epsilon) {
			return false;
		}

		t_min = ((float)num / den);

		if (t_min > 0.0) {
			return true;
		}
		else {
			return false;
		}
	}

	bool hit(math::Ray<math::Vector>const& R,
		ShadeRec& sr) const
	{
		float t{ std::numeric_limits<float>::max() };
		bool intersect = this->closest_intersect_get(R, t);

		// If this object is hit, set the shaderec with the relevant material and information about the hit point
		if (intersect && t < sr.m_tmin) {
			sr.m_normal = normal; // Override
			sr.m_ray = R;
			sr.m_tmin = t;
			sr.m_material = m_material;
		}

		return intersect;
	}

	bool shadow_hit([[maybe_unused]]math::Ray<math::Vector>const& R,
		[[maybe_unused]]float& t) const
	{
		// Plane doesn't need to cast shadows or occlude
		return false; 
	}

private:
	math::Vector normal;
	math::Vector position;
};

class Triangle : public Object {

public:

	Triangle(std::vector<math::Vector> vertices,
		math::Vector position)
	{
		this->vertices = vertices;
		this->position = position;

		boundbox_calc();
	}

	math::Vector normal_get() const
	{
		math::Vector dir1 = vertices.at(0) - vertices.at(1);
		math::Vector dir2 = vertices.at(0) - vertices.at(2);

		math::Vector normal = glm::normalize(
			glm::cross(dir1, dir2));
		return normal;
	}

	float t_get(const math::Ray<math::Vector>& R) const
	{
		double px = position.x;
		double py = position.y;
		double pz = position.z;

		double ax = vertices.at(0).x + px;
		double ay = vertices.at(0).y + py;
		double az = vertices.at(0).z + pz;

		double bx = vertices.at(1).x + px;
		double by = vertices.at(1).y + py;
		double bz = vertices.at(1).z + pz;

		double cx = vertices.at(2).x + px;
		double cy = vertices.at(2).y + py;
		double cz = vertices.at(2).z + pz;

		double a = ax - bx;
		double b = ax - cx;
		double c = R.d.x;

		double d = ax - R.o.x;
		double e = ay - by;
		double f = ay - cy;
		double g = R.d.y;

		double h = ay - R.o.y;
		double i = az - bz;
		double j = az - cz;
		double k = R.d.z;

		double l = az - R.o.z;

		double beta_num =
			d * (f * k - g * j)
			+ b * (g * l - h * k)
			+ c * (h * j - f * l);
		double gamma_num =
			a * (h * k - g * l)
			+ d * (g * i - e * k)
			+ c * (e * l - h * i);
		double t_num =
			a * (f * l - h * j)
			+ b * (h * i - e * l)
			+ d * (e * j - f * i);
		double den =
			a * (f * k - g * j)
			+ b * (g * i - e * k)
			+ c * (e * j - f * i);

		double beta = beta_num / den;

		if (beta < 0.0) {
			return 0;
		}

		double gamma = gamma_num / den;
		if (gamma < 0.0) {
			return 0;
		}

		if (beta + gamma > 1.0) {
			return 0;
		}

		double t = t_num / den;

		return (float)t;
	}

	bool closest_intersect_get(math::Ray<math::Vector>const& R,
		float& t_min) const
	{
		float t = t_get(R);
		if (t > m_epsilon) {
			t_min = t;
			return true;
		}
		return false;
	}

	virtual bool hit(math::Ray<math::Vector>const& R,
		ShadeRec& sr) const
	{
		float t{ std::numeric_limits<float>::max() };
		bool intersect = this->closest_intersect_get(R, t);

		// If this object is hit, set the shaderec with the relevant material and information about the hit point
		if (intersect && t < sr.m_tmin) {
			sr.m_normal = normal_get(); // Override
			sr.m_ray = R;
			sr.m_tmin = t;
			sr.m_material = m_material;
		}

		return intersect;
	}

	bool shadow_hit(math::Ray<math::Vector>const& R,
		float& t) const
	{
		bool hit = this->closest_intersect_get(R, t);
		if (hit && t > m_epsilon) {
			return hit;
		}
		else {
			return false;
		}
	}

	void scale(math::Vector const& scale){
		for (size_t i = 0; i < vertices.size();i++) {
			vertices.at(i) = vertices.at(i) * scale;
		}
		boundbox_calc();
	}

	void translate(math::Vector const& pos) {
		for (size_t i = 0; i < vertices.size(); i++) {
			vertices.at(i) = vertices.at(i) + pos;
		}
		boundbox_calc();
	}

protected:

	std::vector<math::Vector> vertices;
	math::Vector position;

	void boundbox_calc() {
		float minX = vertices.at(0).x;
		float minY = vertices.at(0).y;
		float minZ = vertices.at(0).z;
		float maxX = vertices.at(0).x;
		float maxY = vertices.at(0).y;
		float maxZ = vertices.at(0).z;

		for (math::Vector vert : vertices)
		{
			vert += position;

			if (vert.x < minX) {
				minX = vert.x;
			}
			if (vert.y < minY) {
				minY = vert.y;
			}
			if (vert.z < minZ) {
				minZ = vert.z;
			}
			if (vert.x > maxX) {
				maxX = vert.x;
			}
			if (vert.y > maxY) {
				maxY = vert.y;
			}
			if (vert.z > maxZ) {
				maxZ = vert.z;
			}

		}

		bounds = Bounds3D(
			math::Vector(
				minX,
				minY,
				minZ),
			math::Vector(
				maxX,
				maxY,
				maxZ));
	}
};

class SmoothMeshUVTriangle : public Triangle {
public:
	SmoothMeshUVTriangle(std::vector<math::Vector> vertices,
		std::vector<math::Vector2> uvs,
		std::vector<math::Vector> normals,
		math::Vector position) : Triangle(vertices, position) {
		m_uvs = uvs;
		m_normals = normals;
	}

	bool hit(math::Ray<math::Vector>const& R,
		ShadeRec& sr) const
	{
		float px = position.x;
		float py = position.y;
		float pz = position.z;

		float ax = vertices.at(0).x + px;
		float ay = vertices.at(0).y + py;
		float az = vertices.at(0).z + pz;

		float bx = vertices.at(1).x + px;
		float by = vertices.at(1).y + py;
		float bz = vertices.at(1).z + pz;

		float cx = vertices.at(2).x + px;
		float cy = vertices.at(2).y + py;
		float cz = vertices.at(2).z + pz;

		float a = ax - bx;
		float b = ax - cx;
		float c = R.d.x;

		float d = ax - R.o.x;
		float e = ay - by;
		float f = ay - cy;
		float g = R.d.y;

		float h = ay - R.o.y;
		float i = az - bz;
		float j = az - cz;
		float k = R.d.z;

		float l = az - R.o.z;

		float beta_num =
			d * (f * k - g * j)
			+ b * (g * l - h * k)
			+ c * (h * j - f * l);
		float gamma_num =
			a * (h * k - g * l)
			+ d * (g * i - e * k)
			+ c * (e * l - h * i);
		float t_num =
			a * (f * l - h * j)
			+ b * (h * i - e * l)
			+ d * (e * j - f * i);
		float den =
			a * (f * k - g * j)
			+ b * (g * i - e * k)
			+ c * (e * j - f * i);

		float beta = beta_num / den;

		if (beta < 0.0) {
			return false;
		}

		float gamma = gamma_num / den;
		if (gamma < 0.0) {
			return false;
		}

		if (beta + gamma > 1.0f) {
			return false;
		}

		float t = t_num / den;

		if (t <= m_epsilon) {
			return false;
		}

		// If this object is hit, set the shaderec with the relevant material and information about the hit point
		if (t < sr.m_tmin) {
			sr.m_ray = R;
			sr.m_tmin = t;
			sr.m_material = m_material;
			if (m_uvs.size() == 3) {
				sr.u = interpolate_u(beta, gamma);
				sr.v = interpolate_v(beta, gamma);
			}
			if (m_normals.size() == 3) {
				sr.m_normal = interpolate_norm(beta, gamma);
			}
			else {
				sr.m_normal = normal_get(); // Override
			}
		}

		return true;
	}

	math::Vector interpolate_norm(float& beta, float& gamma) const
	{
		return (1 - beta - gamma) * m_normals.at(0) + (beta * m_normals.at(1)) + (gamma * m_normals.at(2));
	}

	float interpolate_u(float& beta, float& gamma) const
	{
		return (1 - beta - gamma) * m_uvs.at(0).x + (beta * m_uvs.at(1).x) + (gamma * m_uvs.at(2).x);
	}

	float interpolate_v(float& beta, float& gamma) const
	{
		return (1 - beta - gamma) * m_uvs.at(0).y + (beta * m_uvs.at(1).y) + (gamma * m_uvs.at(2).y);
	}

	void set_uvs(std::vector<math::Vector2> const& uvs)
	{
		if(uvs.size() == 3){
			m_uvs = uvs;
		}
	}

protected:
	std::vector<math::Vector2> m_uvs;
	std::vector<math::Vector> m_normals;
};

class Mesh {
public:
	std::vector<std::shared_ptr<SmoothMeshUVTriangle>> m_triangles;
	math::Vector m_position;

	Mesh(std::string const& filename, std::string const& mat_path, math::Vector position)
	{
		m_position = position;
		std::optional<atlas::utils::ObjMesh> opt_mesh = atlas::utils::load_obj_mesh(filename, mat_path);
		if (opt_mesh.has_value()) {
			m_shapes = opt_mesh.value();
		}
		else {
			std::cout << "ERROR: file '" << filename << "' was not in the path" << std::endl;
			exit(-1);
		}

		for (auto shape : m_shapes.shapes) {
			size_t num_indices = shape.indices.size();
			for (size_t i{}; i < num_indices;i+=3) {
				std::vector<math::Vector> vertices{};
				std::vector<math::Vector2> uvs{};
				std::vector<math::Vector> normals{};
				unsigned int j = 0;
				while (j < 3) {
					size_t index = shape.indices.at(i + j++);
					vertices.push_back(shape.vertices.at(index).position);
					if (shape.has_normals) {
						normals.push_back(shape.vertices.at(index).normal);
					}
					if (shape.has_texture_coords) {
						uvs.push_back(shape.vertices.at(index).tex_coord);
					}
				}
				std::shared_ptr<SmoothMeshUVTriangle> new_tri = std::make_shared<SmoothMeshUVTriangle>(vertices, uvs, normals, m_position);
				m_triangles.push_back(new_tri);
			}
		}
	}

	void material_set(std::shared_ptr<Material> const& material)
	{
		for (auto triangle : m_triangles) {
			triangle->material_set(material);
		}
	}

	void translate(math::Vector const& position) {
		for (auto triangle : m_triangles) {
			triangle->translate(position);
		}
	}

	void scale(math::Vector const& scale) {
		for (auto triangle : m_triangles) {
			triangle->scale(scale);
		}
	}

	void fake_uvs()
	{
		std::vector<math::Vector2> fake = { math::Vector2(0.0f, 0.0f), math::Vector2(1.0f, 0.0f), math::Vector2(0.5f, 1.0f) };
		for (auto triangle : m_triangles) {
			triangle->set_uvs(fake);
		}
	}

	std::vector<std::shared_ptr<SmoothMeshUVTriangle>>& repr_get() {
		return m_triangles;
	}

	void dump_to_list(std::vector<std::shared_ptr<Object>>& list) {
		for (auto tri : m_triangles) {
			list.push_back(tri);
		}
	}

protected:
	atlas::utils::ObjMesh m_shapes;
};

class PinholeCamera : public Camera {
public:
	float m_d;

	PinholeCamera(float d)
	{
		m_d = d;
	}	

	Colour colour_validate(Colour const& colour) const
	{
		if (colour.x > 1.0f
			|| colour.y > 1.0f
			|| colour.z > 1.0f
			|| colour.x < 0.0f
			|| colour.y < 0.0f
			|| colour.z < 0.0f)
		{
			// Out of gamut turns RED
			return Colour(1.0f, 0.0f, 0.0f);
		}
		else {
			return colour;
		}
	}

	void multithread_render_scene(World& world, unsigned int num_threads)
	{
		int wheight = (int)world.m_vp->vres;
		int wwidth = (int)world.m_vp->hres;

		// vector for which we will allow threads to read from an retrieve slabs to render
		std::shared_ptr<std::mutex> slablist_mutex = std::make_shared<std::mutex>();
		std::vector<std::shared_ptr<SceneSlab>> slabs;

		std::shared_ptr<std::vector<std::vector<Colour>>> storage = std::make_shared<std::vector<std::vector<Colour>>>(world.m_vp->vres, std::vector<Colour>(world.m_vp->hres));
		std::shared_ptr<std::mutex> storage_mutex = std::make_shared<std::mutex>();
		std::vector<std::thread> thread_list;

		std::shared_ptr<World> world_ptr = std::make_shared<World>(world);

		int slab_width = world.m_slab_size;
		int slab_height = world.m_slab_size;

		for (int i = 0; i < wheight; i += world.m_slab_size)
		{
			// Reset slab width on each height loop
			slab_width = world.m_slab_size;

			// Check that we aren't flying off the bottom of our image
			if (i + slab_height > wheight) {
				slab_height = wheight - i - 1;
			}

			for (int j = 0; j < wwidth; j += world.m_slab_size)
			{
				if (j + slab_width > wwidth) {
					slab_width = wwidth - j - 1;
				}

				int w_center = wwidth / 2;
				int h_center = wheight / 2;

				std::shared_ptr<SceneSlab> new_ti = std::make_shared<SceneSlab>(
					world_ptr,//std::make_shared<World>(world),
					storage_mutex,
					storage,
					j - w_center,
					j + slab_width - w_center,
					i - h_center,
					i + slab_height - h_center);

				// Add this slab to our pool
				slabs.push_back(new_ti);
			}
		}

		// Creat the specified number of threads and let them work on the pool of slabs
		std::cout << "INFO: rendering on " << num_threads << " threads" << std::endl;
		for (unsigned int i = 0; i < num_threads; i++) {
			thread_list.push_back(std::thread(
				[this, slablist_mutex, &slabs]
				{
					std::shared_ptr<SceneSlab> slab_ptr = nullptr;
					size_t full_size = slabs.size();
					while (true) {
						{ // sanity check scope for the mutex
							const std::lock_guard<std::mutex> lock(*slablist_mutex);
							if (slabs.empty()) {
								break;
							}
							slab_ptr = slabs.back();
							slabs.pop_back();
							std::cout << "\r                                         ";
							std::cout << "\rLOADING: " << ((float)slabs.size() * 100.0f/ full_size) << "% to go. " << slabs.size() << " slabs left";
						}
						this->render_slab(slab_ptr);
					}
				})
			);
		}


		// Join all the threads
		for (std::thread& thread : thread_list) {
			if (thread.joinable())
			{
				thread.join();
			}
		}

		// reformat the 2D vector into a single dimensional array
		for (auto row : *(storage)) {
			for (auto el : row)
			{
				world.m_image.push_back(el);
			}
		}
	}

	void render_scene(World& world) const
	{
		int wheight = (int)world.m_vp->vres;
		int wwidth = (int)world.m_vp->hres;

		for (int i = (wheight / 2) - 1; i >= -(wheight / 2); i--){
			for (int j = -(wwidth / 2); j < (wwidth / 2); j++){
				// std::vector<std::vector<double>> subsamples =
				//     world.m_sampler->subsample(4);
				Colour average = Colour(0.0f, 0.0f, 0.0f);
				unsigned int count = 0;
				//for(std::vector<double> sample : subsamples){
				int max_num_samples = world.m_sampler->num_samples_get();

				// For anti-aliasing
				for (int s = 0; s < max_num_samples; s++) {
					ShadeRec sr = ShadeRec(world);
					sr.m_colour = world.m_background;

					std::vector<float> sample =
						world.m_sampler->sample_unit_square();

					atlas::math::Vector x = m_u * ((float)j
						+ (float)sample.at(0));
					atlas::math::Vector y = m_v * ((float)i
						+ (float)sample.at(1));
					atlas::math::Vector z = -m_w * (float)m_d;

					atlas::math::Vector direction =
						glm::normalize((x + y + z));

					atlas::math::Ray<atlas::math::Vector> ray(m_eye,
						direction);

					for (std::shared_ptr<Object> obj : world.m_scene)
					{
						obj->hit(ray, sr);
					}

					if (sr.m_material)
					{
						average += sr.m_material->shade(sr);
					}
					else {
						average += sr.m_colour;
					}
					count++;
				}

				world.m_image.push_back(colour_validate(average * (1.0f / (float)count))
				);
			}
			std::cout << "\r                                         ";
			std::cout << "\rLOADING: " << i + (wheight / 2) << " rows left";
		}
	}

protected:
	void render_slab(std::shared_ptr<SceneSlab> slab) const
	{
		World world = *(slab->world);
		std::shared_ptr<std::mutex> storage_mutex = slab->storage_mutex;
		std::shared_ptr<std::vector<std::vector<Colour>>> storage = slab->storage;

		int start_x = slab->start_x;
		int end_x = slab->end_x;
		int start_y = slab->start_y;
		int end_y = slab->end_y;

		std::vector<std::vector<Colour>> temp_storage = std::vector<std::vector<Colour>>(world.m_slab_size, std::vector<Colour>(world.m_slab_size));

		for (int i = start_y; i < end_y; i++)
		{
			for (int j = start_x; j < end_x; j++)
			{
				Colour average = Colour(0.0f, 0.0f, 0.0f);
				unsigned int count = 0;
				int max_num_samples = world.m_sampler->num_samples_get();

				// For anti-aliasing
				for (int s = 0; s < max_num_samples; s++) {
					ShadeRec sr = ShadeRec(world);
					sr.m_colour = world.m_background;
					sr.depth = 0;

					// Get the sample offsets [0, 1)
					std::vector<float> sample =
						world.m_sampler->sample_unit_square(s);

					math::Vector x = m_u * ((float)j
						+ (float)sample.at(0));
					math::Vector y = m_v * ((float)i
						+ (float)sample.at(1));
					math::Vector z = -m_w * (float)m_d;

					math::Vector direction =
						glm::normalize((x + y + z));

					math::Ray<math::Vector> ray(m_eye, direction);

					bool hit = false;
					for (std::shared_ptr<Object> obj : world.m_scene) {
						if (obj->hit(ray, sr)) {
							hit = true;
						}
					}

					// If we hit an object, it will have set the material
					if (hit && sr.m_material)
					{
						average += sr.m_material->shade(sr);
					}
					else {
						average += sr.m_colour;
					}
					count++;
				}

				int row_0_indexed = (int)i - start_y;
				int col_0_indexed = (int)j - start_x;

				temp_storage.at(row_0_indexed).at(col_0_indexed) = colour_validate(average * (1 / (float)count));
			}
		}


		for (int i = start_y; i < end_y; i++)
		{
			const std::lock_guard<std::mutex> lock(*storage_mutex);
			for (int j = start_x; j < end_x; j++) {

				//// 0,0 is in the center of the screen
				int row = (int)i + (int)(world.m_vp->vres / 2);
				int col = (int)j + (int)(world.m_vp->hres / 2);

				// Align the temp slab with the overall scene
				int row_0_indexed = (int)i - start_y;
				int col_0_indexed = (int)j - start_x;

				// Copy out info to the final storage location
				storage->at(world.m_vp->vres - row - 1).at(col) = temp_storage.at(row_0_indexed).at(col_0_indexed);

			}
		}

	}
};
