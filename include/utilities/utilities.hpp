#pragma once
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <vector>
#include <string>
#include <atlas/math/math.hpp>
#include "nlohmann/json.hpp"

using Colour = atlas::math::Vector;

namespace poly::utils {
	struct BMP_info {
		int m_total_height;
		int m_total_width;
		int m_start_height;
		int m_start_width;
		int m_end_height;
		int m_end_width;
		std::vector<Colour> m_image;
		BMP_info();
	};

	template<typename T, typename = std::enable_if<std::is_floating_point<T>::value>>
	constexpr T pi = static_cast<T>(3.14159265358979323846);

	void saveToBMP(std::string const& filename, poly::utils::BMP_info const& w);
	void saveToBMP(nlohmann::json const& json, poly::utils::BMP_info const& w);

	Colour random_colour_generate();
	Colour colour_validate(Colour const& colour);
	atlas::math::Vector reflect_over_normal(const atlas::math::Vector& wi, const atlas::math::Normal n);

}
#endif // !UTILITY_HPP