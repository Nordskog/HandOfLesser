#include "user_interface.h"

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <imgui_impl_win32.h>

#include "settings_global.h"
#include "display_global.h"

UserInterface* UserInterface::mCurrent = nullptr;

void UserInterface::init()
{
	UserInterface::mCurrent = this;
	initGLFW();
	initImgui();

	float xscale, yscale = 0;
	glfwGetWindowContentScale(this->mWindow, &xscale, &yscale);
	updateStyles(xscale);
}

void UserInterface::terminate()
{
	// Which to cleanup first?
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(this->mWindow);
	this->mWindow = nullptr;
	glfwTerminate();
}

bool UserInterface::shouldClose()
{
	return glfwWindowShouldClose(this->mWindow);
}

void UserInterface::onFrame()
{
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	glClear(GL_COLOR_BUFFER_BIT); // Whendo we clear?
	// ImGui::ShowDemoWindow(); // Show demo window! :)

	buildMainInterface();

	// Do stuff

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(this->mWindow);
}

void UserInterface::updateStyles(float scale)
{
	scale *= 2;

	ImGuiIO& io = ImGui::GetIO();

	// Setup Dear ImGui style
	ImGuiStyle& style = ImGui::GetStyle();

	ImGuiStyle styleold = style; // Backup colors
	style = ImGuiStyle(); // IMPORTANT: ScaleAllSizes will change the original size, so we should
						  // reset all style config
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.TabBorderSize = 1.0f;
	style.WindowRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.ScaleAllSizes(scale);

	io.FontGlobalScale = scale; // Fonts look blurry, need to rebuild atlas I guess.

	return;
}

void UserInterface::initGLFW()
{
	glfwSetErrorCallback(UserInterface::error_callback);

	if (glfwInit() != GLFW_TRUE)
	{
		std::cerr << "glfw init failed!" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	this->mWindow = glfwCreateWindow(640 * 4, 480 * 4, "My Title", NULL, NULL);
	if (!this->mWindow)
	{
		std::cerr << "glfw windows creation failed!" << std::endl;
	}

	// glfwSetWindowSizeCallback(this->mWindow, framebuffer_size_callback);
	glfwSetWindowContentScaleCallback(this->mWindow, windows_scale_callback);
	glfwMakeContextCurrent(this->mWindow);
	glfwSwapInterval(1);

	std::cout << "GLFW init finished" << std::endl;
}

void UserInterface::initImgui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(
		this->mWindow, true
	); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
	std::cout << "Imgui init finished" << std::endl;
}

void UserInterface::error_callback(int error, const char* description)
{
}

void UserInterface::windows_scale_callback(GLFWwindow* window, float xscale, float yscale)
{
	std::cout << "New scale: " << xscale << std::endl;
	UserInterface::mCurrent->updateStyles(xscale); // xy should be the same?
}

void UserInterface::buildMainInterface()
{
	ImGui::Begin("My First Tool", NULL, ImGuiWindowFlags_MenuBar);

	ImGui::SeparatorText("Orientation");
	ImGui::InputFloat("rotX", &HOL::settings::OrientationOffset.x(), 1.0f, 5.0f, "%.3f");
	ImGui::InputFloat("rotY", &HOL::settings::OrientationOffset.y(), 1.0f, 5.0f, "%.3f");
	ImGui::InputFloat("rotZ", &HOL::settings::OrientationOffset.z(), 1.0f, 5.0f, "%.3f");

	{
		Eigen::Vector3f asEuler
			= HOL::display::FinalOffsetLeft.orientation.toRotationMatrix().eulerAngles(0, 1, 2);
		ImGui::Text("Left: %.3f, %.3f, %.3f", asEuler.x(), asEuler.y(), asEuler.z());
	}

	{
		Eigen::Vector3f asEuler
			= HOL::display::FinalOffsetRight.orientation.toRotationMatrix().eulerAngles(0, 1, 2);
		ImGui::Text("Right: %.3f, %.3f, %.3f", asEuler.x(), asEuler.y(), asEuler.z());
	}

	ImGui::SeparatorText("Translation");
	ImGui::InputFloat("posX", &HOL::settings::PositionOffset.x(), 0.001f, 0.01f, "%.3f");
	ImGui::InputFloat("posY", &HOL::settings::PositionOffset.y(), 0.001f, 0.01f, "%.3f");
	ImGui::InputFloat("posZ", &HOL::settings::PositionOffset.z(), 0.001f, 0.01f, "%.3f");

	ImGui::Text(
		"Left: %.3f, %.3f, %.3f",
		HOL::display::FinalOffsetLeft.position.x(),
		HOL::display::FinalOffsetLeft.position.y(),
		HOL::display::FinalOffsetLeft.position.z()
	);

	ImGui::Text(
		"Right: %.3f, %.3f, %.3f",
		HOL::display::FinalOffsetRight.position.x(),
		HOL::display::FinalOffsetRight.position.y(),
		HOL::display::FinalOffsetRight.position.z()
	);

	ImGui::SeparatorText("Raw position");

	ImGui::Text(
		"Left Pos: %.3f, %.3f, %.3f",
		HOL::display::RawPoseLeft.position.x(),
		HOL::display::RawPoseLeft.position.y(),
		HOL::display::RawPoseLeft.position.z()
	);

	{
		Eigen::Vector3f asEuler
			= HOL::display::RawPoseLeft.orientation.toRotationMatrix().eulerAngles(0, 1, 2);
		ImGui::Text("Left Rot: %.3f, %.3f, %.3f", asEuler.x(), asEuler.y(), asEuler.z());
	}

	ImGui::Text(
		"Right: %.3f, %.3f, %.3f",
		HOL::display::RawPoseRight.position.x(),
		HOL::display::RawPoseRight.position.y(),
		HOL::display::RawPoseRight.position.z()
	);

	{
		Eigen::Vector3f asEuler
			= HOL::display::RawPoseRight.orientation.toRotationMatrix().eulerAngles(0, 1, 2);
		ImGui::Text("Right Rot: %.3f, %.3f, %.3f", asEuler.x(), asEuler.y(), asEuler.z());
	}

	ImGui::SeparatorText("Final position");

	ImGui::Text(
		"Left: %.3f, %.3f, %.3f",
		HOL::display::FinalPoseLeft.position.x(),
		HOL::display::FinalPoseLeft.position.y(),
		HOL::display::FinalPoseLeft.position.z()
	);

	{
		Eigen::Vector3f asEuler
			= HOL::display::FinalPoseLeft.orientation.toRotationMatrix().eulerAngles(0, 1, 2);
		ImGui::Text("Left Rot: %.3f, %.3f, %.3f", asEuler.x(), asEuler.y(), asEuler.z());
	}

	ImGui::Text(
		"Right: %.3f, %.3f, %.3f",
		HOL::display::FinalPoseRight.position.x(),
		HOL::display::FinalPoseRight.position.y(),
		HOL::display::FinalPoseRight.position.z()
	);

	{
		Eigen::Vector3f asEuler
			= HOL::display::FinalPoseRight.orientation.toRotationMatrix().eulerAngles(0, 1, 2);
		ImGui::Text("Right Rot: %.3f, %.3f, %.3f", asEuler.x(), asEuler.y(), asEuler.z());
	}

	ImGui::End();
}
