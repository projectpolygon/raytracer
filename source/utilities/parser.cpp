#include <thread>
#include <iostream>
#include "utilities/paths.hpp"
#include "utilities/parser.hpp"

#include "objects/sphere.hpp"
#include "objects/torus.hpp"
#include "objects/triangle.hpp"
#include "objects/mesh.hpp"
#include "objects/plane.hpp"

#include "materials/reflective.hpp"
#include "materials/transparent.hpp"

#include "lights/ambient_occlusion.hpp"

#include "structures/KDTree.hpp"

namespace poly::utils
{
	/**
	Creates an atlas vector object from JSON

	@param vector_json the JSON formatted list of size 3 used to represent a 3D
	vector

	@returns the parsed vector object
	*/
	math::Vector parse_vector(nlohmann::json vector_json)
	{
		// TODO: Make more generic for different sizes
		return {vector_json[0], vector_json[1], vector_json[2]};
	}

	/**
Creates a new material from JSON specifiction

@param material_json the JSON formatted object used to represent the material
properties

@returns the parsed Material as a shared pointer
*/
	std::shared_ptr<poly::material::Material>
	parse_material(nlohmann::json material_json)
	{
		auto material_type = material_json["type"];
		if (material_type == "matte") {
			return std::make_shared<poly::material::Matte>(
				material_json["diffuse"],
				Colour{parse_vector(material_json["colour"])});
		}
		else if (material_type == "reflective") {
			return std::make_shared<poly::material::Reflective>(
				material_json["reflective"],
				material_json["diffuse"],
				material_json["spectral"],
				parse_vector(material_json["colour"]),
				material_json["tightness"]);
		}
		else if (material_type == "transparent") {
			return std::make_shared<poly::material::Transparent>(
				material_json["reflective"],
				material_json["transparent"],
				material_json["diffuse"],
				material_json["spectral"],

				parse_vector(material_json["colour"]),
				material_json["refractive_index"],
				material_json["tightness"]);
		}
		else {
			throw std::runtime_error("incorrect material parameters");
		}
	}

	/**
	Opens and parses a file given its name in the current directory.
	If file is not found, the program terminates.

	@param file_handle the name of the JSON file to be parsed

	@throws parse_error if json can not be parsed

	@returns the parsed Material as a shared pointer
	*/
	nlohmann::json open_json_file(const char* file_handle)
	{
		try {
			std::ifstream fs;
			std::string line, json_string;
			fs.open(file_handle, std::fstream::in);

			while (fs >> line) {
				json_string += line;
			}

			fs.close();
			return nlohmann::json::parse(json_string);
		}
		catch (std::runtime_error& error) {
			std::wcerr << error.what() << std::endl;
			exit(1);
		}
	}

	/**
	Creates a sampler and binds it to the world given JSON sampler parameters

	@param w the world to bind the sampler to
	@param task the JSON object holding the parsed data

	@throws nlohmann::detail::type_error if one or more parameters do not exist

	@returns void
	*/
	void parse_sampler(poly::structures::World& w, nlohmann::json& task)
	{
		try {
			auto cam_type = task["camera"]["sampler"]["type"];

			if (cam_type == "jittered") {
				w.m_sampler = std::make_shared<poly::sampler::AA_Jittered>(
					task["camera"]["sampler"]["samples"],
					task["camera"]["sampler"]["sets"]);
			}
			else {
				throw std::runtime_error("Incorrect sampler parameters");
			}
		}
		catch (const nlohmann::detail::type_error& e) {
			std::wcerr << "Incorrect sampler parameters" << std::endl;
			throw e;
		}
	}

	/*
	 * Adds json data about object to a world object by reference
	 */
	void parse_objects(poly::structures::World& w, nlohmann::json& task)
	{
		for (auto obj : task["objects"]) {
			if (obj["type"] == "mesh") {
				std::string path_to_object(ShaderPath);
				path_to_object.append(obj["object_file"]);
				std::string path_to_material(ShaderPath);
				path_to_material.append(obj["material_file"]);

				std::vector<std::shared_ptr<poly::object::Object>> object_list;

				std::shared_ptr<poly::object::Mesh> s =
					std::make_shared<poly::object::Mesh>(
						path_to_object.c_str(),
						path_to_material.c_str(),
						parse_vector(obj["position"]));

				std::shared_ptr<poly::material::Material> material =
					parse_material(obj["material"]);
				s->material_set(material);
				s->scale(parse_vector(obj["scale"]));
				// s->translate(parse_vector(obj["position"]));
				s->dump_to_list(object_list);

				w.m_scene.push_back(std::make_shared<poly::structures::KDTree>(
					object_list, 80, 30, 0.75f, 10, 50));
			}
			else if (obj["type"] == "sphere") {
				std::shared_ptr<poly::object::Sphere> s =
					std::make_shared<poly::object::Sphere>(
						parse_vector(obj["centre"]), obj["radius"]);
				std::shared_ptr<poly::material::Material> material =
					parse_material(obj["material"]);
				s->material_set(material);
				w.m_scene.push_back(s);
			}
			else if (obj["type"] == "torus") {
				std::shared_ptr<poly::object::Torus> s =
					std::make_shared<poly::object::Torus>(
						parse_vector(obj["centre"]),
						obj["inner_radius"],
						obj["outer_radius"]);

				std::shared_ptr<poly::material::Material> material =
					parse_material(obj["material"]);

				s->material_set(material);
				w.m_scene.push_back(s);
			}
			else if (obj["type"] == "triangle") {
				// std::vector<std::vector<float>> points = obj["points"];
				std::vector<math::Vector> points = {
					parse_vector(obj["points"][0]),
					parse_vector(obj["points"][1]),
					parse_vector(obj["points"][2])};
				math::Vector position = parse_vector(obj["position"]);
				std::shared_ptr<poly::object::Triangle> s =
					std::make_shared<poly::object::Triangle>(points, position);

				std::shared_ptr<poly::material::Material> material =
					parse_material(obj["material"]);
				s->material_set(material);
				w.m_scene.push_back(s);
			}
			else if (obj["type"] == "plane") {
				math::Vector normal	  = parse_vector(obj["normal"]);
				math::Vector position = parse_vector(obj["position"]);
				std::shared_ptr<poly::material::Material> material =
					parse_material(obj["material"]);
				std::shared_ptr<poly::object::Plane> p =
					std::make_shared<poly::object::Plane>(normal, position);
				p->material_set(material);
				w.m_scene.push_back(p);
			}
			else {
				std::cerr << "Type: " << obj["type"] << " not suppported"
						  << std::endl;
				throw std::runtime_error("ERROR: object type %s not supported");
			}
		}
	}

	/*
	 * Parses a json array of lights and adds them to the world
	 */
	void parse_light(poly::structures::World& w, nlohmann::json& task)
	{
		for (auto light : task["lights"]) {
			std::shared_ptr<poly::light::Light> l;
			if (light["type"] == "point") {
				l = std::make_shared<poly::light::PointLight>(
					parse_vector(light["position"]));
				l->radiance_scale(light["intensity"]);
				w.m_lights.push_back(l);
			}
			else if (light["type"] == "ambient") {
				l = std::make_shared<poly::light::AmbientLight>();
				l->colour_set(parse_vector(light["colour"]));
				l->radiance_scale(light["intensity"]);
				w.m_ambient = l;
			}
			else {
				throw std::runtime_error("Incorrect light parameters");
			}
		}
	}

	/**
	Creates a world given JSON parameters

	@param task the JSON object holding the world's information

	@throws nlohmann::detail::type_error if one or more parameters do not exist

	@returns poly::structures::World object
	*/
	void create_world(nlohmann::json& task, poly::structures::World& w)
	{
		std::shared_ptr<poly::structures::ViewPlane> vp =
			std::make_shared<poly::structures::ViewPlane>();
		try {
			vp->vres	  = task["image_height"];
			vp->hres	  = task["image_width"];
			vp->max_depth = task["max_depth"];
		}
		catch (const nlohmann::detail::type_error& e) {
			std::wcerr << "ERROR: incorrect viewplane parameters" << std::endl;
			std::wcerr << e.what() << std::endl;
			exit(1);
		}

		w.m_vp			 = vp;
		w.m_tracer		 = std::make_shared<poly::structures::WhittedTracer>(w);
		w.m_scene		 = std::vector<std::shared_ptr<poly::object::Object>>();
		w.m_start_width	 = task["slab_startx"];
		w.m_start_height = task["slab_starty"];
		w.m_end_width	 = task["slab_endx"];
		w.m_end_height	 = task["slab_endy"];
		try {
			w.m_slab_size = task["slab_size"];
		}
		catch ([[maybe_unused]] nlohmann::detail::type_error& e) {
			std::clog << "WARN: no slab size set" << std::endl;
			w.m_slab_size = vp->hres / 4;
		}

		try {
			w.m_background = Colour{parse_vector(task["background"])};
		}
		catch ([[maybe_unused]] nlohmann::detail::type_error& e) {
			std::clog << "WARN: no background was set" << std::endl;
			w.m_background = {0.0f, 0.0f, 0.0f};
		}

		try {
			parse_sampler(w, task);
			parse_objects(w, task);
			parse_light(w, task);
		}
		catch (const nlohmann::detail::type_error& e) {
			std::wcerr << e.what() << std::endl;
			std::wcerr << "ERROR: error parsing samples, light, or object"
					   << std::endl;
			exit(1);
		}
		catch (const std::runtime_error& e) {
			std::wcerr << e.what() << std::endl;
			exit(1);
		}
	}

	/**
	Creates a camera given JSON parameters

	@param json the JSON object holding the camera's information

	@throws nlohmann::detail::type_error if one or more parameters do not exist

	@returns poly::camera::PinholeCamera object
	*/
	poly::camera::PinholeCamera parse_camera(nlohmann::json& json)
	{
		nlohmann::json camera_json = json["camera"];

		// Create the camera and set how many threads it can render on
		poly::camera::PinholeCamera cam =
			poly::camera::PinholeCamera(camera_json["distance"]);

		// Autodetect possible threads
		std::size_t maximum_threads_allowed = json["max_threads"];

		const auto processor_count = std::thread::hardware_concurrency();
		std::clog << "INFO: detecting " << processor_count << " cores"
				  << std::endl;

		if (processor_count < maximum_threads_allowed && processor_count > 0) {
			std::clog << "INFO: using " << processor_count << " cores"
					  << std::endl;
			cam.set_max_threads(processor_count);
		}
		else if (maximum_threads_allowed > 0) {
			std::clog << "INFO: using max of " << maximum_threads_allowed
					  << " cores" << std::endl;
			cam.set_max_threads(maximum_threads_allowed);
		}
		else {
			std::clog << "INFO: using default of 1 core" << std::endl;
			cam.set_max_threads(1);
		}

		// Set the camera position and view parameters
		cam.eye_set(parse_vector(camera_json["eye"]));
		cam.lookat_set(parse_vector(camera_json["lookat"]));
		cam.upvec_set(parse_vector(camera_json["up"]));
		cam.uvw_compute();
		return cam;
	}

	/**
	Creates a output information object that will store the information
	about the image generated, as well as the data itself

	@param json the JSON object holding the container's information

	@throws nlohmann::detail::type_error if one or more parameters do not exist

	@returns poly::utils::BMP_info object
	*/
	poly::utils::BMP_info create_output_container(nlohmann::json& json)
	{
		poly::utils::BMP_info expected_output;
		expected_output.m_total_height = json["image_height"];
		expected_output.m_total_width  = json["image_width"];
		expected_output.m_start_width  = json["slab_startx"];
		expected_output.m_start_height = json["slab_starty"];
		expected_output.m_end_width	   = json["slab_endx"];
		expected_output.m_end_height   = json["slab_endy"];

		// Ensure bounds are kept within the total image
		if (expected_output.m_end_height > expected_output.m_total_height) {
			expected_output.m_end_height = expected_output.m_total_height;
		}

		if (expected_output.m_end_width > expected_output.m_total_width) {
			expected_output.m_end_width = expected_output.m_total_width;
		}

		return expected_output;
	}
} // namespace poly::utils
