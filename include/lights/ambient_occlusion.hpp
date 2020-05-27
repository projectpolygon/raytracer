#pragma once

class AmbientOcclusion : public Light {
public:
    AmbientOcclusion() : Light()
    {
      min_amount = Colour(1.0f, 1.0f, 1.0f);
    }

    void min_amount_set(float amount)
    {
      min_amount = Colour(1.0f, 1.0f, 1.0f) * amount;
    }

    void sampler_set(std::shared_ptr<Sampler> s_ptr, float tightness)
    {
      m_sampler = s_ptr;
      m_sampler->map_samples_to_hemisphere(tightness);
    }

    math::Vector direction_get([[maybe_unused]] ShadeRec& sr)
    {
      return math::Vector(0.0f, 0.0f, 0.0f);
    }

    // Specific to ambient occlusion, we need access to samples mapped to hemisphere
    math::Vector shadow_direction_get([[maybe_unused]] ShadeRec& sr, unsigned int sample_index)
    {
      // Take any one index of our samplers samples
      // This enables us to have multiple threads access the samples
      // without needing to guard access with mutexes
      std::vector<float> sp = m_sampler->sample_hemisphere(sample_index);

      // For multithreading, its important that these aren't global
      atlas::math::Vector w = sr.m_normal;
      atlas::math::Vector u = glm::normalize(glm::cross(math::Vector(0.0f, 1.0f, 0.0f), w));
      atlas::math::Vector v = glm::normalize(glm::cross(w, u));

      return (sp.at(0) * u
              + sp.at(1) * v
              + sp.at(2) * w);
    }

    Colour L(ShadeRec& sr)
    {
      // Only build the occlusion shadows for the first hitpoints of locations
      if (sr.depth != 0) {
        return m_colour * m_ls;
      }

      int num_samples = m_sampler->num_samples_get();
      Colour average(0.0f, 0.0f, 0.0f);

      // Update the hitpoint coordinate system
      math::Point new_origin = sr.hitpoint_get();

      for (int i = 0; i < num_samples; i++) {
        math::Vector new_dir = glm::normalize(shadow_direction_get(sr, i));
        math::Ray shadow_ray(new_origin, new_dir);

        // in_shadow consumes the bulk of the processing power
        if (in_shadow(shadow_ray, sr)) {
          average += min_amount * m_colour * m_ls;
        }
        else {
          average += m_colour * m_ls;
        }
      }
      return average / (float)num_samples;
    }

protected:
    std::shared_ptr<Sampler> m_sampler;
    Colour min_amount;
};