#include <cfloat>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <zeus/timer.hpp>

#include "integrators/SPPMIntegrator.hpp"
#include "utilities/parser.hpp"

#define POLY_USING_SPPM

using namespace atlas;

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "ERROR: you must specify a taskfile" << std::endl;
		exit(1);
	}

	// Time seed RNG
	srand((unsigned int)time(0));

	/*
	Open the JSON file specified as the first argument to the program

	If the file cannot be parsed, or has incorrect parameters, the
	program will automatically terminate with exit code 1
	*/
	std::clog << "INFO: reading taskfile '" << argv[1] << std::endl;
	nlohmann::json taskfile;
	try {
		taskfile = poly::utils::open_json_file(argv[1]);
	}
	catch (...) {
		std::cerr << "ERROR: taskfile could not be parsed. Exiting..."
				  << std::endl;
		exit(1);
	}

	/*
	Create the world (objects, lights, materials, textures, image information)
	If any object fails construction, the program will exit with code 1
	*/
	poly::structures::World world;
	try {
		poly::utils::create_world(taskfile, world);
	}
	catch (const nlohmann::detail::type_error& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "ERROR: world could not be parsed. Exiting..."
				  << std::endl;
		exit(1);
	}

	// Create the camera
	poly::camera::PinholeCamera camera;
	try {
		camera = poly::utils::parse_camera(taskfile);
	}
	catch (const nlohmann::detail::type_error& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "ERROR: camera could not be parsed. Exiting..."
				  << std::endl;
		exit(1);
	}

	// Generate the correct output container
	poly::utils::BMP_info output;
	try {
		output = poly::utils::BMP_info(
			poly::utils::create_output_container(taskfile));
	}
	catch (const nlohmann::detail::type_error& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "ERROR: output information could not be parsed"
				  << std::endl;
		exit(1);
	}

	// Start a render timer
	zeus::Timer<float> render_timer = zeus::Timer<float>();
	render_timer.start();

#ifdef POLY_USING_SPPM
	// Try parsin an integrator
	poly::integrators::SPPMIntegrator stoch_prog_phot_mapper;
	bool using_sppm =
		poly::utils::create_SPPMIntegrator(stoch_prog_phot_mapper, taskfile);
	if (using_sppm == true) {
		stoch_prog_phot_mapper.render(world, camera, output);
	}
#else
	bool using_sppm = false;
#endif

	if (using_sppm == false) {
		// Create the required output file
		camera.multithread_render_scene(world, output);
	}

	// Output render time
	std::clog << "\nINFO: Time to render was: " << render_timer.elapsed()
			  << std::endl;

	// Output to BMP file
	saveToBMP(taskfile, output);

	return 0;
}
