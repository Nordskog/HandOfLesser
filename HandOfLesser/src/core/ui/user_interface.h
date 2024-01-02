#pragma once

#include <HandOfLesserCommon.h>
#include <GLFW/glfw3.h>

class UserInterface
{
public:
	void init();
	void terminate();
	bool shouldClose();
	void onFrame();
	void buildMainInterface();

private:
	GLFWwindow* mWindow = nullptr;
	static UserInterface* mCurrent; // We only have a single window for now
	void initGLFW();
	void initImgui();
	float mScale = 1.f;
	static void error_callback(int error, const char* description);
	static void windows_scale_callback(GLFWwindow* window, float xscale, float yscale);
	float scaleSize(float size);

	void updateStyles(float scale);

	void InputFloatMultipleSingleLableWithButtons(std::string inputLabelBase,
												  std::string visibleLabel,
												  float smallIncrement,
												  float largeIncrement,
												  std::string format,
												  int width,
												  const std::vector<const float*>& values);

	void InputFloatMultipleTopLableWithButtons(std::string inputLabelBase,
											   const std::vector<std::string>& visibleLabel,
											   float smallIncrement,
											   float largeIncrement,
											   std::string format,
											   int width,
											   const std::vector<const float*>& values);

	void buildSingleFingerTrackingDisplay(HOL::HandSide side);
	void buildSingleHandTransformDisplay(HOL::HandSide side);
	void buildHandTransformDisplay();
	void buildFingerTrackingDisplay();
	void BuildVRChatOSCSettings();
};