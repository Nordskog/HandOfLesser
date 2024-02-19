#include "user_interface.h"

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cmath>
#include <iostream>
#include <imgui_impl_win32.h>

#include "src/core/settings_global.h"
#include "src/core/ui/display_global.h"
#include <HandOfLesserCommon.h>

using namespace HOL;

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
	scale *= 1.25f;
	this->mScale = scale;

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

	this->mWindow = glfwCreateWindow(640 * 4, 480 * 4, "Hand of Lesser", NULL, NULL);
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
	ImGui_ImplGlfw_InitForOpenGL(this->mWindow,
								 true); // Second param install_callback=true will install GLFW
										// callbacks and chain to existing ones.
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

float UserInterface::scaleSize(float size)
{
	return size * this->mScale;
}

void UserInterface::buildSingleHandTransformDisplay(HOL::HandSide side)
{
	const char* sideName = (side == HOL::LeftHand ? "Left" : "Right");
	const char* windowName = (side == HOL::LeftHand ? "LeftChild" : "RightRight");
	int width = (side == HOL::LeftHand ? ImGui::GetContentRegionAvail().x * 0.5f : 0);

	ImGui::BeginChild(windowName, ImVec2(width, 0), ImGuiChildFlags_AutoResizeY);

	ImGui::SeparatorText(sideName);

	ImGui::Checkbox("Valid", &HOL::display::HandTransform[side].positionValid);
	ImGui::SameLine();
	ImGui::Checkbox("Tracked", &HOL::display::HandTransform[side].positionTracked);

	ImGui::SeparatorText("Position");

	ImGui::Text("Raw   : %.3f, %.3f, %.3f",
				HOL::display::HandTransform[side].rawPose.position.x(),
				HOL::display::HandTransform[side].rawPose.position.y(),
				HOL::display::HandTransform[side].rawPose.position.z());

	ImGui::Text("Final : %.3f, %.3f, %.3f",
				HOL::display::HandTransform[side].finalPose.position.x(),
				HOL::display::HandTransform[side].finalPose.position.y(),
				HOL::display::HandTransform[side].finalPose.position.z());

	ImGui::Text("Offset: %.3f, %.3f, %.3f",
				HOL::display::HandTransform[side].finalTranslationOffset.x(),
				HOL::display::HandTransform[side].finalTranslationOffset.y(),
				HOL::display::HandTransform[side].finalTranslationOffset.z());

	ImGui::SeparatorText("Orientation");

	{
		Eigen::Vector3f asEuler
			= HOL::display::HandTransform[side].rawPose.orientation.toRotationMatrix().eulerAngles(
				0, 1, 2);
		ImGui::Text("Raw   : %.3f, %.3f, %.3f",
					HOL::radiansToDegrees(asEuler.x()),
					HOL::radiansToDegrees(asEuler.y()),
					HOL::radiansToDegrees(asEuler.z()));
	}

	{
		Eigen::Vector3f asEuler = HOL::display::HandTransform[side]
									  .finalPose.orientation.toRotationMatrix()
									  .eulerAngles(0, 1, 2);
		ImGui::Text("Final : %.3f, %.3f, %.3f",
					HOL::radiansToDegrees(asEuler.x()),
					HOL::radiansToDegrees(asEuler.y()),
					HOL::radiansToDegrees(asEuler.z()));
	}

	{
		ImGui::Text("Offset: %.3f, %.3f, %.3f",
					HOL::display::HandTransform[side].finalOrientationOffset.x(),
					HOL::display::HandTransform[side].finalOrientationOffset.y(),
					HOL::display::HandTransform[side].finalOrientationOffset.z());
	}

	ImGui::EndChild();
}

void UserInterface::buildSingleFingerTrackingDisplay(
	const char* windowLabel,
	HOL::HandSide side,
	HOL::FingerBend bendValues[HOL::FingerType_MAX],
	bool treatAsInt,
	bool treatAsRadians)
{

	int width = (side == HOL::LeftHand ? ImGui::GetContentRegionAvail().x * 0.5f : 0);

	ImGui::BeginChild(windowLabel, ImVec2(width, 0), ImGuiChildFlags_AutoResizeY);

	// ImGui::SeparatorText(sideName);

	HOL::FingerTrackingDisplay* displayValues = &HOL::display::FingerTracking[(int)side];
	const char* rowNames[] = {"Index", "Middle", "Ring", "Little", "Thumb"};

	// Text fields are not affected by item width, so place manually
	float labelWidth = scaleSize(60);
	float valueWidth = scaleSize(55);

	for (int i = 0; i < HOL::FingerType_MAX; i++)
	{
		float positionOffset = labelWidth;

		ImGui::Text("%s: ", rowNames[i]);
		ImGui::SameLine(positionOffset);

		for (int j = 0; j < HOL::FingerBendType_MAX; j++)
		{
			// Pad positive values with a space so negative values stay aligned.
			// There is a format thing to add a + for positive numbers, but not just a space.

			float value = bendValues[i].bend[j];
			if (treatAsRadians)
			{
				value = HOL::radiansToDegrees(value);
			}

			if (treatAsInt)
			{
				value = value;
				ImGui::Text(value >= 0 ? " %d" : "%d", (int)std::roundf(value));
			}
			else
			{
				value = value;
				ImGui::Text(value >= 0 ? " %.2f" : "%.2f", value);
			}

			positionOffset += valueWidth;

			if (j < HOL::FingerBendType_MAX - 1)
			{
				ImGui::SameLine(positionOffset);
			}
		}
	}

	ImGui::EndChild();
}

void UserInterface::InputFloatMultipleSingleLableWithButtons(
	std::string inputLabelBase,
	std::string visibleLabel,
	float smallIncrement,
	float largeIncrement,
	std::string format,
	int width,
	const std::vector<const float*>& values)
{
	ImGui::PushItemWidth(scaleSize(width));

	for (int i = 0; i < values.size(); i++)
	{
		ImGui::InputFloat(("##" + inputLabelBase + std::to_string(i)).c_str(),
						  (float*)values[i],
						  smallIncrement,
						  largeIncrement,
						  format.c_str());
		ImGui::SameLine();
	}

	ImGui::Text(visibleLabel.c_str());

	ImGui::PopItemWidth();
}

void UserInterface::InputFloatMultipleTopLableWithButtons(
	std::string inputLabelBase,
	const std::vector<std::string>& visibleLabel,
	float smallIncrement,
	float largeIncrement,
	std::string format,
	int width,
	const std::vector<const float*>& values)
{
	ImGui::PushItemWidth(scaleSize(width));

	for (int i = 0; i < values.size(); i++)
	{
		ImGui::BeginGroup();

		const char* label;
		if (i >= visibleLabel.size())
		{
			label = "";
		}
		else
		{
			label = visibleLabel[i].c_str();
		}

		ImGui::Text(label);

		ImGui::InputFloat(("##" + inputLabelBase + std::to_string(i)).c_str(),
						  (float*)values[i],
						  smallIncrement,
						  largeIncrement,
						  format.c_str());

		ImGui::EndGroup();

		if (i < values.size() - 1)
			ImGui::SameLine();
	}

	ImGui::PopItemWidth();
}

void UserInterface::BuildVRChatOSCSettings()

{
	ImGui::BeginChild("VRChatSettings",
					  ImVec2(ImGui::GetContentRegionAvail().x * 1.f, 0),
					  ImGuiChildFlags_AutoResizeY);

	ImGui::SeparatorText("VRChat");

	InputFloatMultipleSingleLableWithButtons("thumbRotationOffset",
											 "Thumb rotation offset",
											 1.f,
											 1.f,
											 "%.1f",
											 90,
											 {&HOL::settings::ThumbAxisOffset[0],
											  &HOL::settings::ThumbAxisOffset[1],
											  &HOL::settings::ThumbAxisOffset[2]});

	ImGui::SeparatorText("Curl Center");

	InputFloatMultipleSingleLableWithButtons("commonFingerCurlCenter",
											 "Common",
											 0.1f,
											 1.f,
											 "%.1f",
											 90,
											 {&HOL::settings::CommonCurlCenter[0],
											  &HOL::settings::CommonCurlCenter[1],
											  &HOL::settings::CommonCurlCenter[2]});

	InputFloatMultipleSingleLableWithButtons("thumbFingerCurlCenter",
											 "Thumb",
											 0.1f,
											 1.f,
											 "%.1f",
											 90,
											 {&HOL::settings::ThumbCurlCenter[0],
											  &HOL::settings::ThumbCurlCenter[1],
											  &HOL::settings::ThumbCurlCenter[2]});

	ImGui::SeparatorText("Splay Center");

	InputFloatMultipleTopLableWithButtons("fingerSplayCenter",
										  {"Index", "Middle", "Ring", "Little", "Thumb"},
										  0.1f,
										  1.f,
										  "%.1f",
										  90,
										  {&HOL::settings::FingerSplayCenter[0],
										   &HOL::settings::FingerSplayCenter[1],
										   &HOL::settings::FingerSplayCenter[2],
										   &HOL::settings::FingerSplayCenter[3],
										   &HOL::settings::FingerSplayCenter[4]});

	// Raw
	ImGui::SeparatorText("Raw bend");
	buildSingleFingerTrackingDisplay("rawBendLeft",
									 HOL::LeftHand,
									 HOL::display::FingerTracking[HOL::LeftHand].rawBend,
									 false,
									 true);
	ImGui::SameLine();
	buildSingleFingerTrackingDisplay("rawBendRight",
									 HOL::RightHand,
									 HOL::display::FingerTracking[HOL::RightHand].rawBend,
									 false,
									 true);

	// Humanoid
	ImGui::SeparatorText("Humanoid bend");
	buildSingleFingerTrackingDisplay("humanoidBendLeft",
									 HOL::LeftHand,
									 HOL::display::FingerTracking[HOL::LeftHand].humanoidBend,
									 false,
									 false);
	ImGui::SameLine();
	buildSingleFingerTrackingDisplay("humanoidBendRight",
									 HOL::RightHand,
									 HOL::display::FingerTracking[HOL::RightHand].humanoidBend,
									 false,
									 false);

	// Encoded, 0-255 in left hand, -1 to +1 that we'll be sending to vrchat in right hand
	ImGui::SeparatorText("Packed bend");
	buildSingleFingerTrackingDisplay("packedBendBothHandsRaw",
									 HOL::LeftHand,
									 HOL::display::FingerTracking[HOL::LeftHand].packedBend,
									 true,
									 false);
	ImGui::SameLine();
	buildSingleFingerTrackingDisplay("packedBendBothHandsVrchat",
									 HOL::RightHand,
									 HOL::display::FingerTracking[HOL::RightHand].packedBend,
									 false,
									 false);

	ImGui::EndChild();
}

void UserInterface::buildMainInterface()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImGui::Begin("Controllers", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	ImGui::BeginChild("LeftMainWindow", ImVec2(scaleSize(600), 0), ImGuiChildFlags_AutoResizeY);

	/////////////////////////
	// State
	///////////////////////

	ImGui::SeparatorText("State");

	ImGui::Text("OpenXR Runtime: %s", HOL::display::OpenXrRuntimeName.c_str());

	ImGui::Text("OpenXR Session state: %s",
				HOL::OpenXR::getOpenXrStateString(HOL::display::OpenXrInstanceState));

	/////////////////
	// General
	/////////////////

	ImGui::SeparatorText("General");
	ImGui::InputInt("Prediction (ms)", &HOL::settings::MotionPredictionMS);
	if (ImGui::InputInt("Update Interval (ms)", &HOL::settings::UpdateIntervalMS))
	{
		// We probably shouldn't sleep for less than 1ms, and sleeping for
		// negative time is probably undefined behavior.
		if (HOL::settings::UpdateIntervalMS < 1)
		{
			HOL::settings::UpdateIntervalMS = 1;
		}
	}

	/////////////////
	// Offset inputs
	/////////////////

	ImGui::BeginChild("TranslationInput",
					  ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0),
					  ImGuiChildFlags_AutoResizeY);

	ImGui::SeparatorText("Translation");
	ImGui::InputFloat("posX", &HOL::settings::PositionOffset.x(), 0.001f, 0.01f, "%.3f");
	ImGui::InputFloat("posY", &HOL::settings::PositionOffset.y(), 0.001f, 0.01f, "%.3f");
	ImGui::InputFloat("posZ", &HOL::settings::PositionOffset.z(), 0.001f, 0.01f, "%.3f");

	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("OrientationInput", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

	ImGui::SeparatorText("Orientation");
	ImGui::InputFloat("rotX", &HOL::settings::OrientationOffset.x(), 1.0f, 5.0f, "%.3f");
	ImGui::InputFloat("rotY", &HOL::settings::OrientationOffset.y(), 1.0f, 5.0f, "%.3f");
	ImGui::InputFloat("rotZ", &HOL::settings::OrientationOffset.z(), 1.0f, 5.0f, "%.3f");

	ImGui::EndChild();

	///////////////////
	// Hand pose
	///////////////////

	// Hand transform
	buildSingleHandTransformDisplay(HOL::LeftHand);
	ImGui::SameLine();
	buildSingleHandTransformDisplay(HOL::RightHand);

	///////////////////
	// Right window
	///////////////////

	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("RightMainWindow", ImVec2(scaleSize(600), 0), ImGuiChildFlags_AutoResizeY);

	/////////////////
	// VRChat
	/////////////////

	BuildVRChatOSCSettings();

	ImGui::EndChild();
	ImGui::End();
}
