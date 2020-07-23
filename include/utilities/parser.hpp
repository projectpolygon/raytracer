#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#include "cameras/pinhole.hpp"
#include "materials/matte.hpp"
#include "objects/sphere.hpp"
#include "lights/point_light.hpp"
#include "lights/ambient.hpp"
#include "samplers/jittered.hpp"
#include "tracers/whitted_tracer.hpp"
#include "structures/world.hpp"
#include "integrators/SPPMIntegrator.hpp"

namespace poly::utils {

		/*
		 * Parses json array data and converts it into atlas code
		 */
		math::Vector parse_vector(nlohmann::json vector_json);

		/*
		 * Parses json data about material and returns a pointer
		 */
		std::shared_ptr<poly::material::Material> parse_material(nlohmann::json material_json);

		/*
		 * Opens json file by its handle and returns a json object
		 */
		nlohmann::json open_json_file(const char* file_handle);

		/*
		 * Adds json data about sampler to a world object by reference
		 */
		void parse_sampler(poly::structures::World& w, nlohmann::json& task);

		/*
		 * Adds json data about object to a world object by reference
		 */
		void parse_objects(poly::structures::World& w, nlohmann::json& task);

		/*
		 * Parses a json array of lights and adds them to the world
		 */
		void parse_light(poly::structures::World& w, nlohmann::json& task);

		/*
		 * Main driver for parsing a json configuration for a ray tracer
		 */
		void create_world(nlohmann::json& task, poly::structures::World& w);

		/*
		 * Parses camera from json data and returns a camera object
		 */
		poly::camera::PinholeCamera parse_camera(nlohmann::json& camera_json);

		/*
		 * Parses expected output and create an object for writing output to during a render
		 */
		poly::utils::BMP_info create_output_container(nlohmann::json& json);

		/*
		 * Parses the integrators parameters and creates a object for a photon mapping render
		 */
		poly::integrators::SPPMIntegrator create_SPPMIntegrator(nlohmann::json& json);
}