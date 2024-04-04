#pragma once

#include <HandOfLesserCommon.h>
#include <GLFW/glfw3.h>

namespace HOL
{
	class UserInterface
	{
	public:
		void init();
		void terminate();
		void onFrame();
		void buildMainInterface();
		bool shouldTerminate();

	private:
		GLFWwindow* mWindow = nullptr;
		static UserInterface* mCurrent; // We only have a single window for now
		void initGLFW();
		void initImgui();
		float mScale = 1.f;
		static void error_callback(int error, const char* description);
		static void windows_scale_callback(GLFWwindow* window, float xscale, float yscale);
		float scaleSize(float size);
		bool mShouldTerminate = false;
		bool shouldCloseWindow();

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

		void buildSingleFingerTrackingDisplay(const char* windowLabel,
											  HOL::HandSide side,
											  HOL::FingerBend bendValues[HOL::FingerType_MAX],
											  bool treatAsRadians = false,
											  bool treatAsInt = false);
		void buildSingleHandTransformDisplay(HOL::HandSide side);
		void BuildVRChatOSCSettings();
	};
}

