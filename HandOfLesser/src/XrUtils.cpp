#include "XrUtils.h"
#include <xr_linear.h>

bool hasExtension(std::vector<xr::ExtensionProperties> const& extensions, const char* name)
{
    auto b = extensions.begin();
    auto e = extensions.end();

    const std::string name_string = name;

    return (e !=
        std::find_if(b, e, [&](const xr::ExtensionProperties& prop)
            {
                return prop.extensionName == name_string;
            }));
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

XrQuaternionf quaternionFromEulerAngles(float x, float y, float z)
{
    XrQuaternionf QuatAroundX;
    XrQuaternionf QuatAroundY;
    XrQuaternionf QuatAroundZ;

    XrVector3f xAxis(1.0, 0.0, 0.0);
    XrVector3f yAxis(0.0, 1.0, 0.0);
    XrVector3f zAxis(0.0, 0.0, 1.0);


    XrQuaternionf_CreateFromAxisAngle( &QuatAroundX, &xAxis, x);
    XrQuaternionf_CreateFromAxisAngle( &QuatAroundY, &yAxis, y);
    XrQuaternionf_CreateFromAxisAngle( &QuatAroundZ, &zAxis, z);
    
    XrQuaternionf tempOrientation;
    XrQuaternionf finalOrientation;
    XrQuaternionf_Multiply(&tempOrientation, &QuatAroundX, &QuatAroundY);
    XrQuaternionf_Multiply(&finalOrientation, &tempOrientation, &QuatAroundZ);

    return finalOrientation;
}

XrQuaternionf quaternionFromEulerAnglesDegrees(float x, float y, float z)
{
    return quaternionFromEulerAngles(
        degreesToRadians(x),
        degreesToRadians(y),
        degreesToRadians(z)
    );
}

XrQuaternionf quaternionFromEulerAnglesDegrees(XrVector3f degrees)
{
    return quaternionFromEulerAnglesDegrees(degrees.x, degrees.y, degrees.z);
}

float degreesToRadians(float degrees)
{
    return degrees * MATH_PI / 180.0f;
}

XrVector3f flipRotation( XrVector3f trans)
{
    // Left/right translation only needs to flip z?
    return XrVector3f(trans.x, trans.y, -trans.z);
}

XrVector3f flipTranslation(XrVector3f trans)
{
    // left/right rotation needs to flip x?
    return XrVector3f(-trans.x, trans.y, trans.z);
}
