#include "utilities/utilities.hpp"

using namespace atlas;
using Colour = math::Vector;

/**
 * Saves a BMP image file based on the given array of pixels. All pixel values
 * have to be in the range [0, 1].
 *
 * @param filename The name of the file to save to.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param image The array of pixels representing the image.
 */
void saveToBMP(std::string const& filename,
							 poly::utils::BMP_info& info)
{
	std::vector<unsigned char> data(info.m_image.size() * 3);
	int width = info.m_total_width;
	int height = info.m_total_height;

	for (int i{height - info.m_end_height}, k{0}; i < height - info.m_start_height; ++i)
	{
		for (int j{info.m_start_width}; j < info.m_end_width; ++j, k += 3) {
			Colour pixel = info.m_image[(i * width) + j];
			data[k + 0]  = static_cast<unsigned char>(pixel.r * 255);
			data[k + 1]  = static_cast<unsigned char>(pixel.g * 255);
			data[k + 2]  = static_cast<unsigned char>(pixel.b * 255);
		}
	}

	stbi_write_bmp(filename.c_str(),
		static_cast<int>(info.m_end_width - info.m_start_width),
		static_cast<int>(info.m_end_height - info.m_start_height),
		3,
		data.data());
}

Colour random_colour_generate()
{
	unsigned int granularity = 256;
	Colour colour;
	colour.x = (rand() % granularity) / (float)granularity;
	colour.y = (rand() % granularity) / (float)granularity;
	colour.z = (rand() % granularity) / (float)granularity;
	return colour;
}
