#pragma once

#include <nlohmann/json.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>

// Defines specializations so nlohmann can serialize third-party types

namespace nlohmann
{
	template <> 
	struct adl_serializer<Eigen::Vector3f>
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