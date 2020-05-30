#pragma once
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <vector>
#include <string>
#include <stb_image.h>
#include <stb_image_write.h>
#include <atlas/math/math.hpp>

using Colour = atlas::math::Vector;

void saveToBMP(std::string const &filename,
	std::size_t width,
	std::size_t height,
	std::vector<Colour> const &image);

Colour random_colour_generate();
#endif // !UTILITY_HPP