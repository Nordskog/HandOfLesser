#pragma once

#include <HandOfLesserCommon.h>
#include <GLFW/glfw3.h>
#include "visualizer.h"

namespace HOL
{
	class UserInterface
	{
	public:
		UserInterface();
		void init();
		void terminate();
		void onFrame();
		void buildInterface();
		bool shouldTerminate();
		static UserInterface* Current; // We only have a single window for now
		Visualizer* getVisualizer();

	private:
		GLFWwindow* mWindow = nullptr;

		void initGLFW();
		void initImgui();
		float mScale = 1.f;
		static void error_callback(int error, const char* description);
		static void windows_scale_callback(GLFWwindow* window, float xscale, float yscale);
		float scaleSize(float size);
		bool mShouldTerminate = false;
		bool shouldCloseWindow();

		Visualizer mVisualizer;

		void updateStyles(float scale);

		bool rightAlignButton(const char* label, int verticalLineOffset = 0);

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

		void buildMain();
		void buildSkeletal();
		void buildVRChatOSCSettings();
		void buildInput();
		void buildVisual();
	};
} // namespace HOL
