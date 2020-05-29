#pragma once

#include "utilities/utilities.hpp"
#include "structures/world.hpp"
#include "structures/scene_slab.hpp"

namespace poly::structures
{
	scene_slab::scene_slab(
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
} // namespace poly::structures