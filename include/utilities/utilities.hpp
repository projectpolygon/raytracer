#pragma once
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <vector>
#include <string>
#include <stb_image.h>
#include <stb_image_write.h>
#include <atlas/math/math.hpp>

using Colour = atlas::math::Vector;

namespace poly::utils {
		struct BMP_info{
				int m_total_height;
				int m_total_width;
				int m_start_height;
				int m_start_width;
				int m_end_height;
				int m_end_width;
				std::vector<Colour> m_image;
		};
}



void saveToBMP(std::string const& filename,
							 poly::utils::BMP_info& w);

Colour random_colour_generate();
#endif // !UTILITY_HPP