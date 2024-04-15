#include "XrUtils.h"
#include <src/windows/windows_utils.h>

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

	XrHandEXT toOpenXRHandSide(HOL::HandSide side)
	{
		switch (side)
		{
			case HOL::LeftHand:
				return XrHandEXT::XR_HAND_LEFT_EXT;
			case HOL::RightHand:
				return XrHandEXT::XR_HAND_RIGHT_EXT;
			default:
				return XrHandEXT::XR_HAND_MAX_ENUM_EXT; // Just don't do this
		}
	}

	std::string getActiveOpenXRRuntimePath(int majorApiVersion)
	{
		std::string runtimePath = "Unknown";
		std::string path = "SOFTWARE\\Khronos\\OpenXR\\" + std::to_string(majorApiVersion);

		RegGetString(HKEY_LOCAL_MACHINE, path, "ActiveRuntime", runtimePath);

		return runtimePath;
	}

	std::string getActiveOpenXRRuntimeName(int majorApiVersion)
	{
		std::string runtimeName = getActiveOpenXRRuntimePath(majorApiVersion);

		// In an ideal world we'd go read the json, but I am lazy
		int lastSlash = runtimeName.find_last_of('\\');
		if (lastSlash > 0)
		{
			// Just get filename from path
			runtimeName = runtimeName.substr(lastSlash + 1);
		}

		return runtimeName;
	}

} // namespace HOL::OpenXR
