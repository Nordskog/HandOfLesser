#pragma once

#include <Eigen/Core>

float computeAngleBetweenVectors(const Eigen::Vector3f& a, const Eigen::Vector3f& b);

float computeOpenXrCurl(
	const Eigen::Vector3f& metacarpal,
	const Eigen::Vector3f& proximal,
	const Eigen::Vector3f& intermediate,
	const Eigen::Vector3f& distal,
	const Eigen::Vector3f& tip
);

Eigen::Vector3f computeOpenXrNormal(
	const Eigen::Vector3f& metacarpal,
	const Eigen::Vector3f& proximal,
	const Eigen::Vector3f& intermediate
);

float computeOpenXrSplay(const Eigen::Vector3f& normal1, const Eigen::Vector3f& normal2);
