#pragma once

#include "paths.hpp"
#include "cameras/camera.hpp"

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
class scene_slab;
class Texture;
class Tracer;
class ViewPlane;

/* BEGIN KD TREE */



