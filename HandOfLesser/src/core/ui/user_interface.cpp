#include "user_interface.h"

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <cmath>
#include <iostream>
#include <imgui_impl_win32.h>
#include <iterator>

#include "src/core/settings_global.h"
#include "src/core/ui/display_global.h"
#include <HandOfLesserCommon.h>
#include "src/core/HandOfLesserCore.h"

using namespace HOL;

static const int PANEL_WIDTH = 600;

UserInterface* UserInterface::Current = nullptr;

ImVec2 operator+(const ImVec2& v1, const ImVec2& v2)
{
	return ImVec2(v1.x + v2.x, v1.y + v2.y);
}

HOL::UserInterface::UserInterface()
{
	UserInterface::Current = this;
}

void UserInterface::init()
{
	initGLFW();
	initImgui();
	this->mVisualizer.init();

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

bool UserInterface::shouldCloseWindow()
{
	return glfwWindowShouldClose(this->mWindow);
}

void UserInterface::onFrame()
{
	this->mShouldTerminate = this->shouldCloseWindow();

	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	glClear(GL_COLOR_BUFFER_BIT); // Whendo we clear?
	// ImGui::ShowDemoWindow(); // Show demo window! :)

	buildInterface();

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

bool HOL::UserInterface::rightAlignButton(const char* label, int verticalLineOffset)
{
	float available_width = ImGui::GetContentRegionAvail().x;

	// note that hide_text_after_double_hash is false, otherwise it will calculate its width too
	float button_width
		= ImGui::CalcTextSize(label, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available_width - button_width);

	if (verticalLineOffset != 0)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetTextLineHeight() + (ImGui::GetStyle().FramePadding.x * 2 ) * verticalLineOffset));
	}



	return ImGui::Button(label);
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

	// TODO: Fix window sizing
	this->mWindow
		= glfwCreateWindow(PANEL_WIDTH * 1.5, PANEL_WIDTH * 1.5, "Hand of Lesser", NULL, NULL);
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
	UserInterface::Current->updateStyles(xscale); // xy should be the same?
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

	ImGui::Checkbox("Active", &HOL::display::HandTransform[side].active);
	ImGui::SameLine();
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

void HOL::UserInterface::buildVisual()
{
	mVisualizer.drawVisualizer();
}

void HOL::UserInterface::buildMisc()
{
	const char* restoreDefaultsLabel = "restore defaults";
	if (ImGui::Button(restoreDefaultsLabel))
		ImGui::OpenPopup(restoreDefaultsLabel);

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(restoreDefaultsLabel, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Restore all settings to default?");
		ImGui::Separator();

		if (ImGui::Button("OK##restoreDefaults", ImVec2(140, 0)))
		{
			Config = HOL::settings::HandOfLesserSettings();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel##restoreDefaults", ImVec2(140, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}


}


void HOL::UserInterface::buildSteamVR()
{
	bool syncSettings = false;

	ImGui::BeginChild(
		"LeftSkeletalWindow", ImVec2(scaleSize(PANEL_WIDTH), 0), ImGuiChildFlags_AutoResizeY);

	/////////////////
	// General
	/////////////////

	ImGui::SeparatorText("Transmission");

	ImGui::Checkbox("Transmit Controller position", &Config.steamvr.sendSteamVRControllerPosition);

	ImGui::Checkbox("Transmit skeletal input", &Config.skeletal.sendSkeletalInput);

	ImGui::Checkbox("Transmit SteamVR Input", &Config.steamvr.sendSteamVRInput);

	ImGui::SeparatorText("Input");

	syncSettings |= ImGui::Checkbox("Block Controller Input while handtracking",
									&Config.steamvr.blockControllerInputWhileHandTracking);

	ImGui::SeparatorText("General");

	syncSettings |= ImGui::InputFloat("Steam Pose offset (s)", &Config.steamvr.steamPoseTimeOffset, 0.01f, 0.1f, "%.3f");

	/////////////////
	// Skeletal 
	/////////////////

	ImGui::SeparatorText("Skeletal Input");

	ImGui::SameLine();
	if (rightAlignButton("Reset##Skeletal"))
	{
		Config.skeletal = HOL::settings::SkeletalInput();
	}

	bool lengthChanged = false;

	syncSettings |= ImGui::InputFloat(
		"Length multiplier",
					&Config.skeletal.jointLengthMultiplier,
					0.01f,
					0.1f,
					"%.3f");

	/////////////////
	// Skeletal Offset
	/////////////////

	ImGui::BeginChild("SkeletalTranslationInput",
					  ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0),
					  ImGuiChildFlags_AutoResizeY);

	ImGui::SeparatorText("Translation##Skeletal");

	auto& positionOffset = Config.skeletal.positionOffset;
	ImGui::InputFloat("posX##Skeletal", &positionOffset.x(), 0.001f, 0.01f, "%.3f");
	ImGui::InputFloat("posY##Skeletal", &positionOffset.y(), 0.001f, 0.01f, "%.3f");
	ImGui::InputFloat("posZ##Skeletal", &positionOffset.z(), 0.001f, 0.01f, "%.3f");

	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("OrientationInput##Skeletal", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

	ImGui::SeparatorText("Orientation##Skeletal");

	auto& orientationOffset = Config.skeletal.orientationOffset;
	ImGui::InputFloat("rotX##Skeletal", &orientationOffset.x(), 1.0f, 5.0f, "%.3f");
	ImGui::InputFloat("rotY##Skeletal", &orientationOffset.y(), 1.0f, 5.0f, "%.3f");
	ImGui::InputFloat("rotZ##Skeletal", &orientationOffset.z(), 1.0f, 5.0f, "%.3f");

	ImGui::EndChild();
	ImGui::EndChild();

	if (syncSettings)
	{
		HOL::HandOfLesserCore::Current->syncSettings();
	}
}


void HOL::UserInterface::buildMain()
{
	ImGui::BeginChild(
		"LeftMainWindow", ImVec2(scaleSize(PANEL_WIDTH), 0), ImGuiChildFlags_AutoResizeY);

	/////////////////////////
	// State
	///////////////////////

	ImGui::SeparatorText("State");

	ImGui::Text("OpenXR Runtime: %s", HOL::display::OpenXrRuntimeName.c_str());

	ImGui::Text("OpenXR Session state: %s",
				HOL::OpenXR::getOpenXrStateString(HOL::display::OpenXrInstanceState));

	//////////////////
	// Mode
	////////////////////

	ImGui::SeparatorText("Driver mode");

	std::vector<std::tuple<std::string, HOL::ControllerMode>> modeRadioButtons
		= {{"None", HOL::ControllerMode::NoControllerMode},
		   {"Emulate", HOL::ControllerMode::EmulateControllerMode},
		   {"Possess", HOL::ControllerMode::HookedControllerMode},
		   {"Offset", HOL::ControllerMode::OffsetControllerMode}};

	bool isFirstIteration = true;
	for (const auto& buttonContent : modeRadioButtons)
	{
		if (!isFirstIteration)
		{
			ImGui::SameLine();
		}

		if (ImGui::RadioButton(std::get<0>(buttonContent).c_str(),
							   (int*)&HOL::Config.handPose.controllerMode,
							   std::get<1>(buttonContent)))
		{
			HOL::HandOfLesserCore::Current->syncSettings();
		}

		isFirstIteration = false;
	}

	/////////////////
	// General
	/////////////////

	ImGui::SeparatorText("General");
	ImGui::InputInt("Prediction (ms)", &Config.general.motionPredictionMS);
	if (ImGui::InputInt("Update Interval (ms)", &Config.general.updateIntervalMS))
	{
		// We probably shouldn't sleep for less than 1ms, and sleeping for
		// negative time is probably undefined behavior.
		if (Config.general.updateIntervalMS < 1)
		{
			Config.general.updateIntervalMS = 1;
		}
	}

	ImGui::Checkbox("Force inactive", &Config.general.forceInactive);
	ImGui::SameLine();

	ImGui::SameLine();
	if (rightAlignButton("Reset##General"))
	{
		Config.general = HOL::settings::GeneralSettings();
	}

	/////////////////
	// Offset inputs
	/////////////////

	ImGui::SeparatorText("Controller type");

	if (ImGui::Button("None (zero offset)"))
	{
		Config.handPose.controllerType = ControllerType::NONE;
	}

	if (ImGui::Button("Index Knucles"))
	{
		Config.handPose.controllerType = ControllerType::ValveIndexKnucles;
	}

	if (ImGui::Button("Touch Airlink"))
	{
		Config.handPose.controllerType = ControllerType::OculusTouch_Airlink;
	}

	ImGui::SameLine();

	if (ImGui::Button("Touch VDXR"))
	{
		Config.handPose.controllerType = ControllerType::OculusTouch_VDXR;
	}

	ImGui::SeparatorText("Offset preset");

	if (ImGui::Button("Zero"))
	{
		HOL::settings::restoreDefaultControllerOffset(HOL::ZERO);
		HOL::HandOfLesserCore::Current->syncSettings();
	}

	ImGui::SameLine();

	if (ImGui::Button("Roughy VRC"))
	{
		HOL::settings::restoreDefaultControllerOffset(HOL::RoughyVRChatHand);
		HOL::HandOfLesserCore::Current->syncSettings();
	}

	ImGui::BeginChild("TranslationInput",
					  ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0),
					  ImGuiChildFlags_AutoResizeY);

	bool offsetChanged = false;
	ImGui::SeparatorText("Translation");

	auto& positionOffset = Config.handPose.positionOffset;
	offsetChanged |= ImGui::InputFloat("posX", &positionOffset.x(), 0.001f, 0.01f, "%.3f");
	offsetChanged |= ImGui::InputFloat("posY", &positionOffset.y(), 0.001f, 0.01f, "%.3f");
	offsetChanged |= ImGui::InputFloat("posZ", &positionOffset.z(), 0.001f, 0.01f, "%.3f");

	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("OrientationInput", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

	ImGui::SeparatorText("Orientation");

	auto& orientationOffset = Config.handPose.orientationOffset;
	offsetChanged |= ImGui::InputFloat("rotX", &orientationOffset.x(), 1.0f, 5.0f, "%.3f");
	offsetChanged |= ImGui::InputFloat("rotY", &orientationOffset.y(), 1.0f, 5.0f, "%.3f");
	offsetChanged |= ImGui::InputFloat("rotZ", &orientationOffset.z(), 1.0f, 5.0f, "%.3f");

	ImGui::EndChild();

	if (offsetChanged)
	{
		HOL::HandOfLesserCore::Current->syncSettings();
	}

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
}

void UserInterface::buildVRChatOSCSettings()

{
	// ImGui::BeginChild("VRChatSettings",
	//				  ImVec2(ImGui::GetContentRegionAvail().x * 1.f, 0),
	//				  ImGuiChildFlags_AutoResizeY);

	ImGui::BeginChild(
		"VRChatSettings", ImVec2(scaleSize(PANEL_WIDTH), 0), ImGuiChildFlags_AutoResizeY);

	ImGui::SeparatorText("VRChat");

	ImGui::Checkbox("Send full", &Config.vrchat.sendFull);
	ImGui::SameLine();
	ImGui::Checkbox("Send Packed", &Config.vrchat.sendPacked);
	ImGui::SameLine();
	ImGui::Checkbox("Send Alternating", &Config.vrchat.sendAlternating);

	ImGui::Checkbox("Interlace packed", &Config.vrchat.interlacePacked);
	ImGui::Checkbox("Use Unity Humanoid Splay", &Config.vrchat.useUnityHumanoidSplay);

	ImGui::Checkbox("Send OSC test data", &Config.vrchat.sendDebugOsc);

	if (ImGui::InputInt("Packed Update Interval (ms)", &Config.vrchat.packedUpdateInterval))
	{
		// Should never be anything but 100, and definitely never less
		if (Config.general.updateIntervalMS < 100)
		{
			Config.general.updateIntervalMS = 100;
		}
	}

	ImGui::InputFloat("Linear Velocity multiplier",
					  &Config.general.linearVelocityMultiplier,
					  0.05f,
					  0.1f,
					  "%.3f");
	ImGui::InputFloat("Angular Velocity multiplier",
					  &Config.general.angularVelocityMultiplier,
					  0.05f,
					  0.1f,
					  "%.3f");

	ImGui::SameLine();
	if (rightAlignButton("Reset##VRChat"))
	{
		Config.vrchat = HOL::settings::VRChatSettings();
	}

	if (Config.vrchat.sendDebugOsc)
	{
		ImGui::SeparatorText("OSC test data");
		ImGui::Checkbox("Alternate curl test (requires interlace)",
						&Config.vrchat.alternateCurlTest);
		ImGui::SliderFloat("Curl", &Config.vrchat.curlDebug, -1, 1);
		ImGui::SliderFloat("Splay", &Config.vrchat.splayDebug, -1, 1);
	}

	if (Config.vrchat.useUnityHumanoidSplay)
	{
		ImGui::SeparatorText("Offsets");

		InputFloatMultipleSingleLableWithButtons("thumbRotationOffset",
												 "Thumb rotation offset",
												 1.f,
												 1.f,
												 "%.1f",
												 90,
												 {&Config.fingerBend.ThumbAxisOffset[0],
												  &Config.fingerBend.ThumbAxisOffset[1],
												  &Config.fingerBend.ThumbAxisOffset[2]});

		ImGui::SameLine();
		if (rightAlignButton("Reset##VRChat_humanoid_thumb_axis"))
		{
			Config.fingerBend.ThumbAxisOffset = HOL::settings::FingerBendSettings().ThumbAxisOffset;
		}

		ImGui::SeparatorText("Curl Center");

		InputFloatMultipleSingleLableWithButtons("commonFingerCurlCenter",
												 "Common",
												 0.5f,
												 1.f,
												 "%.1f",
												 90,
												 {&Config.fingerBend.commonCurlCenter[0],
												  &Config.fingerBend.commonCurlCenter[1],
												  &Config.fingerBend.commonCurlCenter[2]});

		InputFloatMultipleSingleLableWithButtons("thumbFingerCurlCenter",
												 "Thumb",
												 0.5f,
												 1.f,
												 "%.1f",
												 90,
												 {&Config.fingerBend.thumbCurlCenter[0],
												  &Config.fingerBend.thumbCurlCenter[1],
												  &Config.fingerBend.thumbCurlCenter[2]});

		ImGui::SameLine();
		if (rightAlignButton("Reset##VRChat_humanoid_curl"))
		{
			// Well this is cancer
			auto defValues = HOL::settings::FingerBendSettings();
			std::copy(std::begin(defValues.commonCurlCenter),
					  std::end(defValues.commonCurlCenter),
					  std::begin(Config.fingerBend.commonCurlCenter));

			std::copy(std::begin(defValues.thumbCurlCenter),
					  std::end(defValues.thumbCurlCenter),
					  std::begin(Config.fingerBend.thumbCurlCenter));
		}

		ImGui::SeparatorText("Splay Center");

		InputFloatMultipleTopLableWithButtons("fingerSplayCenter",
											  {"Index", "Middle", "Ring", "Little", "Thumb"},
											  0.5f,
											  1.f,
											  "%.1f",
											  90,
											  {&Config.fingerBend.fingerSplayCenter[0],
											   &Config.fingerBend.fingerSplayCenter[1],
											   &Config.fingerBend.fingerSplayCenter[2],
											   &Config.fingerBend.fingerSplayCenter[3],
											   &Config.fingerBend.fingerSplayCenter[4]});

		// This section has 2-line groups so we need to compensate
		ImGui::SameLine();
		if (rightAlignButton("Reset##VRChat_humanoid_splay", 1))
		{
			// still cancer
			auto defValues = HOL::settings::FingerBendSettings();
			std::copy(std::begin(defValues.fingerSplayCenter),
					  std::end(defValues.fingerSplayCenter),
					  std::begin(Config.fingerBend.fingerSplayCenter));
		}
	}
	else
	{
		ImGui::SeparatorText("Curl Offset");

		InputFloatMultipleSingleLableWithButtons("Common curl offset",
												 "Common",
												 0.5f,
												 1.f,
												 "%.1f",
												 90,
												 {&Config.skeletalBend.commonCurlCenter[0],
												  &Config.skeletalBend.commonCurlCenter[1],
												  &Config.skeletalBend.commonCurlCenter[2]});

		InputFloatMultipleSingleLableWithButtons("Thumb curl offset",
												 "Thumb",
												 0.5f,
												 1.f,
												 "%.1f",
												 90,
												 {&Config.skeletalBend.thumbCurlCenter[0],
												  &Config.skeletalBend.thumbCurlCenter[1],
												  &Config.skeletalBend.thumbCurlCenter[2]});

		ImGui::SeparatorText("Splay Offset");

		InputFloatMultipleTopLableWithButtons("Splay offset",
											  {"Index", "Middle", "Ring", "Little", "Thumb"},
											  0.5f,
											  1.f,
											  "%.1f",
											  90,
											  {&Config.skeletalBend.fingerSplayCenter[0],
											   &Config.skeletalBend.fingerSplayCenter[1],
											   &Config.skeletalBend.fingerSplayCenter[2],
											   &Config.skeletalBend.fingerSplayCenter[3],
											   &Config.skeletalBend.fingerSplayCenter[4]});
	}

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

	ImGui::EndChild();
}

void UserInterface::buildInterface()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	auto io = ImGui::GetIO();
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));

	ImGui::Begin("HandOfLesser", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
	{
		if (ImGui::BeginTabItem("Main"))
		{
			buildMain();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("SteamVR"))
		{
			buildSteamVR();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("VRChat"))
		{
			buildVRChatOSCSettings();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Input"))
		{
			bool syncSettings = false;
			syncSettings |= ImGui::Checkbox("Send OSC Input", &Config.input.sendOscInput);

			if (syncSettings)
			{
				HOL::HandOfLesserCore::Current->syncSettings();
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Visual"))
		{
			buildVisual();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Misc"))
		{
			buildMisc();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

bool HOL::UserInterface::shouldTerminate()
{
	return mShouldTerminate;
}

Visualizer* HOL::UserInterface::getVisualizer()
{
	return &this->mVisualizer;
}
