#pragma once

#include <nlohmann/json.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace nlohmann
{
	template <typename T>
	void get_to_if_present(const nlohmann::json& j, const std::string& key, T& value)
	{
		if (j.contains(key))
		{
			j.at(key).get_to(value);
		}
	}

	template <> struct adl_serializer<Eigen::Vector3f>
	{
		static void to_json(json& j, const Eigen::Vector3f& vec)
		{
			j = json{{"x", vec.x()}, {"y", vec.y()}, {"z", vec.z()}};
		}

		// Convert JSON to Eigen::Vector3f
		static void from_json(const json& j, Eigen::Vector3f& vec)
		{
			vec.x() = j.at("x").get<float>();
			vec.y() = j.at("y").get<float>();
			vec.z() = j.at("z").get<float>();
		}
	};
} // namespace nlohmann
