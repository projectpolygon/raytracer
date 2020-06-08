#include "utilities/paths.hpp"
#include "utilities/parser.hpp"

#include "objects/sphere.hpp"
#include "objects/torus.hpp"
#include "objects/triangle.hpp"
#include "objects/mesh.hpp"

#include "structures/KDTree.hpp"

namespace poly::utils {

		/*
		 * Parses json array data and converts it into atlas code
		 */
		math::Vector parse_vector(nlohmann::json vector_json)
		{
			// TODO: Make more generic for different sizes
			return { vector_json[0], vector_json[1], vector_json[2] };
		}

		/*
		 * Parses json data about material and returns a pointer
		 */
		std::shared_ptr<poly::material::Material> parse_material(nlohmann::json material_json)
		{
			auto material_type = material_json["type"];
			if (material_type == "matte") {
				return std::make_shared<poly::material::Matte>(
					material_json["diffuse"], Colour{parse_vector(material_json["colour"])});
			} else {
				throw std::runtime_error("incorrect material parameters");
			}
		}

		/*
		 * Opens json file by its handle and returns a json object
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

			} catch (std::runtime_error& error) {
				std::wcerr << error.what() << std::endl;
				exit(1);
			}
		}

		/*
		 * Adds json data about sampler to a world object by reference
		 */
		void parse_sampler(poly::structures::World& w, nlohmann::json& task)
		{
			try {
				auto cam_type = task["camera"]["sampler"]["type"];

				if (cam_type == "jittered") {
					w.m_sampler = std::make_shared<poly::sampler::AA_Jittered>(
						task["camera"]["sampler"]["samples"], task["camera"]["sampler"]["sets"]);
				} else {
					throw std::runtime_error("Incorrect sampler parameters");
				}
			} catch (const nlohmann::detail::type_error& e) {
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
						std::make_shared<poly::object::Mesh>(path_to_object.c_str(), path_to_material.c_str(), parse_vector(obj["position"]));

					std::shared_ptr<poly::material::Material> material = parse_material(obj["material"]);
					s->material_set(material);
					s->scale(parse_vector(obj["scale"]));
					//s->translate(parse_vector(obj["position"]));
					s->dump_to_list(object_list);

					w.m_scene.push_back(std::make_shared<poly::structures::KDTree>(object_list, 80, 1, 0.5f, 10, 25));
				}
				else if (obj["type"] == "sphere") {
					std::shared_ptr<poly::object::Sphere> s =
						std::make_shared<poly::object::Sphere>(parse_vector(obj["centre"]), obj["radius"]);

					std::shared_ptr<poly::material::Material> material = parse_material(obj["material"]);
					s->material_set(material);
					w.m_scene.push_back(s);
				} 
				else if (obj["type"] == "torus") {
					std::shared_ptr<poly::object::Torus> s =
						std::make_shared<poly::object::Torus>(parse_vector(obj["centre"]), obj["inner_radius"], obj["outer_radius"]);

					std::shared_ptr<poly::material::Material> material = parse_material(obj["material"]);

					s->material_set(material);
					w.m_scene.push_back(s);
				}
				else if (obj["type"] == "triangle") {
					//std::vector<std::vector<float>> points = obj["points"];
					std::vector<math::Vector> points = {parse_vector(obj["points"][0]), parse_vector(obj["points"][1]), parse_vector(obj["points"][2])};
					math::Vector position = parse_vector(obj["position"]);
					std::shared_ptr<poly::object::Triangle> s =
						std::make_shared<poly::object::Triangle>(points, position);

					std::shared_ptr<poly::material::Material> material = parse_material(obj["material"]);
					s->material_set(material);
					w.m_scene.push_back(s);
				}
				else {
					throw std::runtime_error("Incorrect object type");
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
					l = std::make_shared<poly::light::PointLight>(parse_vector(light["position"]));
					l->radiance_scale(light["intensity"]);
					w.m_lights.push_back(l);
				} else if (light["type"] == "ambient"){
					l = std::make_shared<poly::light::AmbientLight>();
					l->colour_set(parse_vector(light["colour"]));
					l->radiance_scale(light["intensity"]);
					w.m_ambient = l;
				} else {
					throw std::runtime_error("Incorrect light parameters");
				}
			}
		}

		/*
		 * Main driver for parsing a json configuration for a ray tracer
		 */
		poly::structures::World create_world(nlohmann::json& task)
		{
			poly::structures::World w;

			std::shared_ptr<poly::structures::ViewPlane> vp = std::make_shared<poly::structures::ViewPlane>();
			try {
				vp->vres = task["image_height"];
				vp->hres = task["image_width"];
				vp->max_depth = task["max_depth"];
			} catch (const nlohmann::detail::type_error& e) {
				std::wcerr << "Incorrect job parameters" << std::endl;
				std::wcerr << e.what() << std::endl;
			}

			w.m_vp = vp;
			w.m_tracer = std::make_shared<poly::structures::WhittedTracer>(&w);
			w.m_scene = std::vector<std::shared_ptr<poly::object::Object>>();
			w.m_start_width = task["slab_startx"];
			w.m_start_height = task["slab_starty"];
			w.m_end_width = task["slab_endx"];
			w.m_end_height = task["slab_endy"];
			w.m_slab_size = 200;// (w.m_end_width - w.m_start_width) / (unsigned int)task["max_threads"];

			try {
				w.m_background = Colour{task["background"]};
			} catch ([[maybe_unused]]nlohmann::detail::type_error&  e) {
				w.m_background = {0.0f, 0.0f, 0.0f};
			}

			try {
				parse_sampler(w, task);
				parse_objects(w, task);
				parse_light(w, task);
			} catch (const nlohmann::detail::type_error& e) {
				std::wcerr << e.what() << std::endl;
				exit(1);
			} catch (const std::runtime_error& e) {
				std::wcerr << e.what() << std::endl;
				exit(1);
			}

			return w;
		}

		/*
		 * Parses camera from json data and returns a camera object
		 */
		poly::camera::PinholeCamera parse_camera(nlohmann::json camera_json)
		{
			poly::camera::PinholeCamera cam = poly::camera::PinholeCamera(camera_json["distance"]);
			cam.eye_set(parse_vector(camera_json["eye"]));
			cam.lookat_set(parse_vector(camera_json["lookat"]));
			cam.upvec_set(parse_vector(camera_json["up"]));
			cam.uvw_compute();
			return cam;
		}
}