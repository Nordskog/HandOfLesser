#pragma once

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
		static UserInterface* mCurrent;	// We only have a single window for now
		void initGLFW();
		void initImgui();
		static void error_callback(int error, const char* description);
		static void windows_scale_callback(GLFWwindow* window, float xscale, float yscale);

		void updateStyles(float scale);



};