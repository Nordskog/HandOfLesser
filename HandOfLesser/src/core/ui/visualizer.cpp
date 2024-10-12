#include "visualizer.h"

#include "imgui.h"
#include <HandOfLesserCommon.h>
#include "src/core/settings_global.h"
#include <src/core/HandOfLesserCore.h>

namespace HOL
{
	Visualizer::Visualizer()
	{
		this->mRawZoom = 5;
		applyZoom();

		this->mCameraPosition = Eigen::Vector3f(0, 0, this->mCameraZoom);
		this->mCameraRotation = Eigen::Vector3f(0, 0, 0);
		this->mCameraAim = Eigen::Vector3f(0, 0, 0);
		this->mCameraOrientation = Eigen::Quaternionf::Identity();

		this->mViewMatrix = Eigen::Matrix4f::Identity();
		this->mProjectionMatrix = Eigen::Matrix4f::Identity();

		this->mActiveDrawQueue = &this->mDrawSwap0;
		this->mSwapQueue = &this->mDrawSwap1;
		this->mDrawQueue = &this->mDrawSwap2;
	}

	void Visualizer::init()
	{
		// Must be called on UI thread!
		this->mUiThreadId = std::this_thread::get_id();
	}

	void Visualizer::centerTo(Eigen::Vector3f center)
	{
		Eigen::Vector3f move = center - mCameraAim;

		this->mCameraAim += move;
		this->mCameraPosition += move;
	}

	void Visualizer::swapOuterDrawQueue()
	{
		// Swap swap and draw
		mDrawSwapLock.lock();
		std::swap(this->mActiveDrawQueue, this->mDrawQueue);
		mDrawSwapLock.unlock();
	}

	void Visualizer::swapInnerDrawQueue()
	{
		// Swap swap and draw
		mDrawSwapLock.lock();
		std::swap(this->mActiveDrawQueue, this->mDrawQueue);
		mDrawSwapLock.unlock();
	}

	void Visualizer::clearDrawQueue()
	{
		this->mDrawQueue->points.clear();
		this->mDrawQueue->lines.clear();
	}

	void Visualizer::drawVisualizer()
	{
		this->swapInnerDrawQueue();

		drawAxis();

		// Auto-resizing? it's a mystery
		// This nonsense is good enough for now
		ImVec2 parentWindowSize = ImGui::GetContentRegionAvail();

		ImGui::BeginChild("VisualWindow",
						  parentWindowSize,
						  ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX);

		ImVec2 upperLeft = ImGui::GetCursorScreenPos();
		ImVec2 lowerRight = ImGui::GetContentRegionAvail();

		/*
		ImGui::GetWindowDrawList()->AddCircleFilled(
			upperLeft, 5, IM_COL32(255, 0, 0, 255)); // Red dot
		ImGui::GetWindowDrawList()->AddCircleFilled(
			lowerRight, 5, IM_COL32(0, 255, 0, 255)); // Green dot

		// Draw lines
		ImGui::GetWindowDrawList()->AddLine(
			upperLeft, lowerRight, IM_COL32(23, 139, 255, 255)); // Yellow line
		*/

		calculateProjectionMatrix();
		drawLines();
		drawPoints();
		this->clearInternalDrawQueue(); // Leave clean slate for next frame

		// handle input and draw widgets after drawing scene,
		// so we know the area we don't want to be interactable
		ImGui::SliderFloat("FoV", &this->mFov, 10.f, 179.f, "%.3f");
		if (ImGui::Checkbox("Follow left", &HOL::Config.visualizer.followLeftHand))
		{
			if (Config.visualizer.followLeftHand)
			{
				Config.visualizer.followRightHand = false;
			}
		}

		ImGui::SameLine();

		if (ImGui::Checkbox("Follow Right", &HOL::Config.visualizer.followRightHand))
		{
			if (Config.visualizer.followRightHand)
			{
				Config.visualizer.followLeftHand = false;
			}
		}

		ImVec2 uiBounds = ImVec2(0, ImGui::GetCursorScreenPos().y);
		ImGui::SameLine(); // so we get the actual end position of the slider
		uiBounds.x = ImGui::GetCursorScreenPos().x;

		handleInput(uiBounds);

		ImGui::GetWindowDrawList()->AddCircleFilled(
			ImVec2((upperLeft.x + lowerRight.x) / 2.0f, (upperLeft.y + lowerRight.y) / 2.0f),
			3,
			IM_COL32(23, 139, 255, 255)); // Green dot

		ImGui::EndChild();
	}

	void Visualizer::drawPoints()
	{
		auto queues = {this->mActiveDrawQueue, &this->mInternalDrawQueue};

		for (auto queue : queues)
		{
			for (auto& point : queue->points)
			{

				auto projected = projectToScreen(point.position);

				ImGui::GetWindowDrawList()->AddCircleFilled(
					projected, point.size, point.color); // Red dot
			}
		}
	}

	void Visualizer::drawLines()
	{
		auto queues = {this->mActiveDrawQueue, &this->mInternalDrawQueue};

		for (auto queue : queues)
		{
			for (auto& line : queue->lines)
			{
				ImGui::GetWindowDrawList()->AddLine(projectToScreen(line.start),
													projectToScreen(line.end),
													line.color,
													line.width); // Red dot
			}
		}
	}

	void Visualizer::clearInternalDrawQueue()
	{
		this->mInternalDrawQueue.points.clear();
		this->mInternalDrawQueue.lines.clear();
	}

	DrawQueue* Visualizer::getDrawQueueForSubmit()
	{
		// Internal queue if on UI thread, otherwise draw queue.
		return std::this_thread::get_id() == this->mUiThreadId
							   ? &this->mInternalDrawQueue
							   : this->mDrawQueue;
	}

	void Visualizer::handleInput(ImVec2 excludeBounds)
	{
		bool moved = false;

		ImVec2 leftMouseClickPos = ImGui::GetIO().MouseClickedPos[0];
		ImVec2 rightMouseClickPos = ImGui::GetIO().MouseClickedPos[1];
		ImVec2 mouseCurrentPos = ImGui::GetIO().MousePos;

		// Exclude bounds is the upper left corner where our other widgets live
		bool leftClickValid
			= leftMouseClickPos.x > excludeBounds.x || leftMouseClickPos.y > excludeBounds.y;
		bool rightClickValid
			= rightMouseClickPos.x > excludeBounds.x || rightMouseClickPos.y > excludeBounds.y;
		bool posValid = mouseCurrentPos.x > excludeBounds.x || mouseCurrentPos.y > excludeBounds.y;

		if (rightClickValid && ImGui::IsMouseDragging(1)) // Right click
		{
			moved = true;
			ImVec2 delta = ImGui::GetIO().MouseDelta;

			float sensitivity = 0.5f;

			Eigen::Vector3f move(-delta.x * sensitivity, delta.y * sensitivity, 0);

			// Modulate by zoom level
			float zoomMultiplier = this->mCameraZoom / 1000.f;
			move *= zoomMultiplier;

			move = this->mCameraOrientation * move;

			mCameraAim += move;

			this->centerToAim();
		}

		if (leftClickValid && ImGui::IsMouseDragging(0)) // Left click
		{
			moved = true;
			// Adjust camera rotation based on mouse movement
			float rotationSensitivity = 0.1f; // Adjust as needed
			float pitch = ImGui::GetIO().MouseDelta.y * rotationSensitivity;
			float yaw = ImGui::GetIO().MouseDelta.x * rotationSensitivity;

			// Would add delta to existing rotation, but quaternions end up with weird will when you
			// do that.
			mCameraRotation.x() -= pitch;
			mCameraRotation.y() -= yaw;

			mCameraRotation.x() = std::clamp(mCameraRotation.x(), -90.0f, 90.0f);

			// Create quaternion from pitch/yaw input
			// Do separately and combine so we apply yaw first, then pitch
			Eigen::Quaternionf pitchOffset
				= HOL::quaternionFromEulerAnglesDegrees(Eigen::Vector3f(mCameraRotation.x(), 0, 0));
			Eigen::Quaternionf yawOffset
				= HOL::quaternionFromEulerAnglesDegrees(Eigen::Vector3f(0, mCameraRotation.y(), 0));

			this->mCameraOrientation = yawOffset * pitchOffset;

			this->centerToAim();
		}

		if (posValid)
		{
			// Adjust zoom level based on mouse wheel input
			float zoomDelta = ImGui::GetIO().MouseWheel;

			if (zoomDelta != 0)
			{
				moved = true;
				float zoomSensitivity = 0.2f;

				mRawZoom -= zoomDelta * zoomSensitivity;

				applyZoom();

				centerToAim();
			}
		}

		/*
		if (moved)
		{
			printf("Pos: %.3f, %.3f, %.3f\n",
				   mCameraPosition.x(),
				   mCameraPosition.y(),
				   mCameraPosition.z());

			printf("Aim: %.3f, %.3f, %.3f\n",
			mCameraAim.x(), mCameraAim.y(), mCameraAim.z());

			printf("Rot: %.3f, %.3f, %.3f\n",
				mCameraRotation.x(),
				mCameraRotation.y(),
				mCameraRotation.z());
		}
		*/
	}

	// Stolen from
	// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix.html
	static void gluPerspective(const float& angleOfView,
							   const float& imageAspectRatio,
							   const float& n,
							   const float& f,
							   float& b,
							   float& t,
							   float& l,
							   float& r)
	{
		float scale = tan(angleOfView * 0.5 * std::numbers::pi_v<float> / 180) * n;
		r = imageAspectRatio * scale, l = -r;
		t = scale, b = -t;
	}

	static void glFrustum(const float& b,
						  const float& t,
						  const float& l,
						  const float& r,
						  const float& n,
						  const float& f,
						  Eigen::Matrix4Xf& M)
	{
		// Set OpenGL perspective projection matrix
		M(0, 0) = 2.f * n / (r - l);
		M(0, 1) = 0;
		M(0, 2) = 0;
		M(0, 3) = 0;

		M(1, 0) = 0;
		M(1, 1) = 2.f * n / (t - b);
		M(1, 2) = 0;
		M(1, 3) = 0;

		M(2, 0) = (r + l) / (r - l);
		M(2, 1) = (t + b) / (t - b);
		M(2, 2) = -(f + n) / (f - n);
		M(2, 3) = -1;

		M(3, 0) = 0;
		M(3, 1) = 0;
		M(3, 2) = -2.f * f * n / (f - n);
		M(3, 3) = 0;
	}

	void Visualizer::calculateProjectionMatrix()
	{
		Eigen::Matrix4Xf rotationMatrix = Eigen::Matrix4f::Identity();

		rotationMatrix.block<3, 3>(0, 0) = mCameraOrientation.toRotationMatrix();

		Eigen::Affine3f translationAff(
			Eigen::Translation3f(mCameraPosition.x(), mCameraPosition.y(), mCameraPosition.z()));

		this->mViewMatrix = translationAff.matrix() * rotationMatrix;

		// World -> camera, so inverse
		this->mViewMatrix = this->mViewMatrix.matrix().inverse();

		ImVec2 upperLeft = ImGui::GetCursorScreenPos();
		ImVec2 lowerRight = ImGui::GetContentRegionAvail();
		ImVec2 screenSize(lowerRight.x - upperLeft.x, lowerRight.y - upperLeft.y);

		float ratio = screenSize.x / screenSize.y; // Calculate or provide the aspect
												   // ratio (width / height)
		// of the viewport
		float nearPlane = 0.1; // Specify the distance to the near clipping plane
		float farPlane = 1000; // Specify the distance to the far clipping plane

		float b, t, l, r;
		gluPerspective(this->mFov, ratio, nearPlane, farPlane, b, t, l, r);
		glFrustum(b, t, l, r, nearPlane, farPlane, this->mProjectionMatrix);
	}

	ImVec2 Visualizer::projectToScreen(const Eigen::Vector3f& position)
	{
		Eigen::Vector4f homogeneousPosition(position.x(), position.y(), position.z(), 1.0f);

		Eigen::Vector4f screenPosition = (mProjectionMatrix * mViewMatrix) * homogeneousPosition;

		// Normalize by W to do perspective magic
		screenPosition /= screenPosition.w();

		// Ehh clip space is weird
		screenPosition.y() = -screenPosition.y();

		// Get size of windoe we're drawing into
		ImVec2 upperLeft = ImGui::GetCursorScreenPos();
		ImVec2 screenSize = ImGui::GetContentRegionAvail();

		float width = (screenSize.x - upperLeft.x);
		float height = (screenSize.y - upperLeft.y);

		// -1 to 1 : 0 to 1
		screenPosition.x() += 1.0f;
		screenPosition.y() += 1.0f;
		screenPosition.x() /= 2.0f;
		screenPosition.y() /= 2.0f;

		// And back into our screen space
		screenPosition.x() *= width;
		screenPosition.y() *= height;

		return ImVec2(screenPosition.x() + upperLeft.x, screenPosition.y() + upperLeft.y);
	}
	void Visualizer::drawAxis()
	{
		auto colorGrey = IM_COL32(155, 155, 155, 255);
		auto colorRed = IM_COL32(255, 0, 0, 255);
		auto colorGreen = IM_COL32(0, 255, 0, 255);
		auto colorBlue = IM_COL32(23, 139, 255, 255);

		float width = 1;

		float halfWidth = width / 2.f;

		Eigen::Vector3f topLeft = Eigen::Vector3f(-halfWidth, halfWidth, halfWidth);
		Eigen::Vector3f topRight = Eigen::Vector3f(halfWidth, halfWidth, halfWidth);
		Eigen::Vector3f bottomRight = Eigen::Vector3f(halfWidth, -halfWidth, halfWidth);
		Eigen::Vector3f bottomLeft = Eigen::Vector3f(-halfWidth, -halfWidth, halfWidth);

		Eigen::Vector3f axisCenter = bottomLeft;
		axisCenter.z() = -halfWidth;

		this->submitPoint(topLeft, colorGrey, 5);
		this->submitPoint(topRight, colorGrey, 5);
		this->submitPoint(bottomRight, colorGrey, 5);
		this->submitPoint(bottomLeft, colorBlue, 5);

		this->submitLine(axisCenter, bottomLeft, colorBlue, 5);

		topLeft.z() = -halfWidth;
		topRight.z() = -halfWidth;
		bottomRight.z() = -halfWidth;
		bottomLeft.z() = -halfWidth;

		this->submitLine(axisCenter, topLeft, colorGreen, 5);
		this->submitLine(axisCenter, bottomRight, colorRed, 5);

		this->submitPoint(topLeft, colorGreen, 5);
		this->submitPoint(topRight, colorGrey, 5);
		this->submitPoint(bottomRight, colorRed, 5);
		this->submitPoint(bottomLeft, colorGrey, 5);

		this->submitPoint(this->mCameraAim, colorGrey, 8);
	}
	void Visualizer::clearDraw()
	{
		this->mActiveDrawQueue->points.clear();
		this->mActiveDrawQueue->lines.clear();
	}
	void Visualizer::centerToAim()
	{
		auto aimToNewCam = Eigen::Vector3f(0, 0, 1);
		aimToNewCam *= this->mCameraZoom;

		this->mCameraPosition = this->mCameraOrientation * aimToNewCam;
		mCameraPosition += this->mCameraAim;
	}
	void Visualizer::applyZoom()
	{
		mCameraZoom = mRawZoom * mRawZoom;

		if (mCameraZoom < 0.01)
		{
			mCameraZoom = 0.01;
		}
	}

	void Visualizer::submitPoint(const Eigen::Vector3f& position, ImU32 color, float size)
	{
		DrawQueue* queue = getDrawQueueForSubmit();

		queue->points.push_back({position, color, size});
	}

	void Visualizer::submitLine(const Eigen::Vector3f& start,
								const Eigen::Vector3f& end,
								ImU32 color,
								float width)
	{
		DrawQueue* queue = getDrawQueueForSubmit();

		queue->lines.push_back({start, end, color, width});
	}

} // namespace HOL