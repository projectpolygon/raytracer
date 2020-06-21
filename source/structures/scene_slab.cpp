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


	/**
	Creates a vector of slabs ranging that renders point from [start_x, end_x) and [start_y, end_y)

	@param start_x the starting x value
	@param end_x the ending x value
	@param start_y the starting y value
	@param end_y the ending y value
	@param preferred_slab_size the prefferred slab dimension
	@param world pointer to the world holding the scene information
	@param storage_mutex to protect storage between threads
	@param storage finalized pixel storage location


	@returns std::vector<std::shared_ptr<poly::structures::scene_slab>> list of scene slab pointers
	*/
	std::vector<std::shared_ptr<poly::structures::scene_slab>> generate_slabs(
		int start_x, int end_x,
		int start_y, int end_y,
		std::size_t preferred_slab_size,
		std::shared_ptr<poly::structures::World> world,
		std::shared_ptr<std::mutex> storage_mutex,
		std::shared_ptr<std::vector<std::vector<Colour>>> storage)
	{
		// Initial slab dimensions
		int slab_width = static_cast<int>(preferred_slab_size);
		int slab_height = static_cast<int>(preferred_slab_size);

		// Correcting for the location of the slabs in the entire image
		int corrected_start_y = start_y - (world->m_vp->vres / 2);
		int corrected_end_y = end_y - (world->m_vp->vres / 2);
		int corrected_start_x = start_x - (world->m_vp->hres / 2);
		int corrected_end_x = end_x - (world->m_vp->hres / 2);

		// vector for which we will allow threads to read from an retrieve slabs to render
		std::vector<std::shared_ptr<poly::structures::scene_slab>> slabs;
		for (int i = corrected_start_y; i < corrected_end_y; i += static_cast<int>(preferred_slab_size))
		{
			// Reset slab width on each height loop
			slab_width = static_cast<int>(preferred_slab_size);

			// Check that we aren't flying off the bottom of our image
			if (i + slab_height > end_y)
			{
				slab_height = end_y - i;
			}

			for (int j = corrected_start_x; j < corrected_end_x; j += static_cast<int>(preferred_slab_size))
			{
				if (j + slab_width > end_x)
				{
					slab_width = end_x - j;
				}

				std::shared_ptr<poly::structures::scene_slab> new_ti = std::make_shared<poly::structures::scene_slab>(
					world,
					storage_mutex,
					storage,
					j,
					j + slab_width,
					i,
					i + slab_height);

				// Add this slab to our pool
				slabs.push_back(new_ti);
			}
		}
		return slabs;
	}
} // namespace poly::structures
