#pragma once

#include "imgui.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <mutex>

namespace HOL
{
	struct Point
	{
		Eigen::Vector3f position;
		ImU32 color;
		float size;
	};

	struct Line
	{
		Eigen::Vector3f start;
		Eigen::Vector3f end;
		ImU32 color;
		float width;
	};

	struct DrawQueue
	{
		std::vector<Point> points;
		std::vector<Line> lines;
	};

	class Visualizer
	{

	public:
		void drawVisualizer();
		void submitPoint(const Eigen::Vector3f& position, ImU32 color, float size);
		void submitLine(const Eigen::Vector3f& start,
						const Eigen::Vector3f& end,
						ImU32 color,
						float width);

		Visualizer();

		void centerTo(Eigen::Vector3f center);

		void swapDrawQueue(); // At end of main loop frame
		void clearDrawQueue();

	private:
		void drawPoints();
		void drawLines();

		void handleInput(ImVec2 excludeBounds);

		void calculateProjectionMatrix();
		ImVec2 projectToScreen(const Eigen::Vector3f& position);

		void drawAxis();
		void clearDraw();
		void centerToAim();
		void applyZoom();

		Eigen::Vector3f mCameraPosition;
		Eigen::Vector3f mCameraAim;
		Eigen::Quaternionf mCameraOrientation;
		Eigen::Vector3f mCameraRotation;

		float mRawZoom = 25;
		float mCameraZoom = 700;

		Eigen::Matrix4Xf mViewMatrix;
		Eigen::Matrix4Xf mProjectionMatrix;

		void swapActiveQueue();	// Immediately before drawing

		std::mutex mDrawSwapLock;

		// draw directly into draw queue 
		bool mInternalDraw = false;

		DrawQueue* mActiveDrawQueue;	// Draw this
		DrawQueue* mSwapQueue;			// Both swap with this
		DrawQueue* mDrawQueue;			// Drawn into this

		DrawQueue mDrawSwap1;
		DrawQueue mDrawSwap2;
		DrawQueue mDrawSwap3;

		float mFov = 90;
	};
} // namespace HOL