#pragma once

#include <atlas/math/math.hpp>
#include "lights/light.hpp"
#include "samplers/sampler.hpp"
#include "structures/world.hpp"

namespace poly::light
{

	class AmbientOcclusion : public Light
	{
	public:
		AmbientOcclusion();

		void min_amount_set(float amount);

		void sampler_set(std::shared_ptr<poly::sampler::Sampler> s_ptr, float tightness);

		atlas::math::Vector direction_get([[maybe_unused]] poly::structures::SurfaceInteraction&sr);

		// Specific to ambient occlusion, we need access to samples mapped to hemisphere
		atlas::math::Vector shadow_direction_get([[maybe_unused]] poly::structures::SurfaceInteraction&sr, unsigned int sample_index);

		Colour L(poly::structures::SurfaceInteraction &sr, poly::structures::World const& world);

		atlas::math::Point location() const override;

	protected:
		std::shared_ptr<poly::sampler::Sampler> m_sampler;
		Colour min_amount;
	};
} // namespace poly::light