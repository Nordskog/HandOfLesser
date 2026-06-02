#pragma once

#include "imgui.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <chrono>
#include <deque>
#include <mutex>
#include <vector>

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

	struct Triangle
	{
		Eigen::Vector3f p0;
		Eigen::Vector3f p1;
		Eigen::Vector3f p2;
		ImU32 color;
	};

	struct DrawQueue
	{
		std::vector<Point> points;
		std::vector<Line> lines;
		std::vector<Triangle> triangles;
	};

	struct TimedTrailPoint
	{
		Eigen::Vector3f position;
		std::chrono::steady_clock::time_point time;
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
		void submitTriangle(const Eigen::Vector3f& p0,
							const Eigen::Vector3f& p1,
							const Eigen::Vector3f& p2,
							ImU32 color);
		void submitCone(const Eigen::Vector3f& origin,
						const Eigen::Vector3f& forward,
						float fovDegrees,
						float length,
						ImU32 fillColor,
						ImU32 lineColor,
						float lineWidth = 1.5f);
		void submitOrientationAxes(const Eigen::Vector3f& position,
								   const Eigen::Quaternionf& orientation,
								   float axisLength = 0.05f,
								   float lineWidth = 2.0f);

		Visualizer();
		void init(); // Call on UI thread!!

		void centerTo(Eigen::Vector3f center);
		bool isActive() const;
		void setActive(bool active);

		void swapOuterDrawQueue(); // At end of main loop frame
		void clearDrawQueue();	// before drawing ( frame start )

	private:
		static constexpr std::chrono::seconds ControllerTrailDuration{3};
		static ImU32 fadeColor(ImU32 baseColor, float alpha);

		void drawPoints();
		void drawLines();
		void drawTriangles();
		void updateControllerTrails();
		void drawControllerTrails();
		void drawModifierCones();
		void clearControllerTrails();
		void swapInnerDrawQueue(); // before drawing
		void clearInternalDrawQueue(); // before drawing

		DrawQueue* getDrawQueueForSubmit();

		void handleInput(ImVec2 excludeBounds);

		void calculateProjectionMatrix();
		ImVec2 projectToScreen(const Eigen::Vector3f& position);

		void drawAxis();
		void centerToAim();
		void applyZoom();

		Eigen::Vector3f mCameraPosition;
		Eigen::Vector3f mCameraAim;
		Eigen::Quaternionf mCameraOrientation;
		Eigen::Vector3f mCameraRotation;

		float mRawZoom = 25;
		float mCameraZoom = 700;

		Eigen::Matrix4f mViewMatrix;
		Eigen::Matrix4f mProjectionMatrix;

		std::mutex mDrawSwapLock;

		std::thread::id mUiThreadId; // Different queues for different threads

		DrawQueue* mActiveDrawQueue; // Draw this
		DrawQueue* mSwapQueue;		 // Swap both with this
		DrawQueue* mDrawQueue;		 // Drawn into this

		DrawQueue mInternalDrawQueue;	// For drawing internal stuff outside of main loop
		DrawQueue mDrawSwap0;
		DrawQueue mDrawSwap1;
		DrawQueue mDrawSwap2;

		std::deque<TimedTrailPoint> mControllerTrails[2];

		bool mIsActive = false;

		float mFov = 90;
	};
} // namespace HOL
