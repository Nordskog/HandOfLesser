#include "XrUtils.h"

namespace HOL::OpenXR
{
	bool hasExtension(std::vector<xr::ExtensionProperties> const& extensions, const char* name)
	{
		auto b = extensions.begin();
		auto e = extensions.end();

		const std::string name_string = name;

		return (e
				!= std::find_if(b,
								e,
								[&](const xr::ExtensionProperties& prop)
								{ return prop.extensionName == name_string; }));
	}

	bool handleXR(std::string what, XrResult res)
	{
		if (res != XrResult::XR_SUCCESS)
		{
			std::cout << what << " failed with " << res << std::endl;
			return false;
		}

		return true;
	}

	Eigen::Vector3f toEigenVector(const XrVector3f& xrVector)
	{
		return Eigen::Vector3f(xrVector.x, xrVector.y, xrVector.z);
	}

	Eigen::Quaternionf toEigenQuaternion(const XrQuaternionf& xrQuat)
	{
		return Eigen::Quaternionf(xrQuat.w, xrQuat.x, xrQuat.y, xrQuat.z);
	}

}
