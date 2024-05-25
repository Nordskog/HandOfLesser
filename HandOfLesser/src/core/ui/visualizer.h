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
		void init(); // Call on UI thread!!

		void centerTo(Eigen::Vector3f center);

		void swapOuterDrawQueue(); // At end of main loop frame
		void clearDrawQueue();	// before drawing ( frame start )

	private:
		void drawPoints();
		void drawLines();
		void swapInnerDrawQueue(); // before drawing
		void clearInternalDrawQueue(); // before drawing

		DrawQueue* getDrawQueueForSubmit();

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

		std::mutex mDrawSwapLock;

		std::thread::id mUiThreadId; // Different queues for different threads

		DrawQueue* mActiveDrawQueue; // Draw this
		DrawQueue* mSwapQueue;		 // Swap both with this
		DrawQueue* mDrawQueue;		 // Drawn into this

		DrawQueue mInternalDrawQueue;	// For drawing internal stuff outside of main loop
		DrawQueue mDrawSwap0;
		DrawQueue mDrawSwap1;
		DrawQueue mDrawSwap2;



		float mFov = 90;
	};
} // namespace HOL