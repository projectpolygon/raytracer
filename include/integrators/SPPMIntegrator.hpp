#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP


#include "structures/world.hpp"

namespace poly::integrators {
	class SPPMIntegrator {
	public:
		SPPMIntegrator();
		void render(poly::structures::World const& world, poly::utils::BMP_info& output);

	};
}
/**
Steps:
the entire algorithm repeats for N iterations

1. Shoot out rays from camera. At every intersection with an object (1 ONLY), create a visiblePoint. Store these "Visible Points" in a KD-Tree, indexed based on location in the scene.

2. Shoot out rays from each light, intersection against the scenery. On each interstedtion, check nearby visible point KD-Tree. For each nearby visible point, add the photon's value to it's light contribution 

PSEUDOCODE FOR ALGORITHM

*/
#endif // !INTEGRATOR_HPP