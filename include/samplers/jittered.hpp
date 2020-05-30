#pragma once

#ifndef JITTERED_HPP
#define JITTERED_HPP

#include <vector>
#include <zeus/constants.hpp>
#include "samplers/sampler.hpp"

namespace poly::sampler
{

	class AA_Jittered : public Sampler
	{
	public:
		AA_Jittered();

		AA_Jittered(unsigned int num_samp, unsigned int num_set);

		void generate_samples();

		std::vector<float> sample_unit_square();

		std::vector<float> sample_unit_square(const unsigned int index);

		std::vector<float> sample_hemisphere();

		std::vector<float> sample_hemisphere(const unsigned int index);
		
		void map_samples_to_hemisphere(const float e);
	};
} // namespace poly::sampler
#endif // !JITTERED_HPP