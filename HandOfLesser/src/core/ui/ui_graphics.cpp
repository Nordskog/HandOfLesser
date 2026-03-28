#include "ui_graphics.h"

#include <cmath>

#include "imgui.h"

namespace
{
	// Keep the drawing helpers stateless and scale from the UI's current DPI factor.
	float scaleSize(float scale, float size)
	{
		return size * scale;
	}
} // namespace

void HOL::UiGraphics::drawInputGestureHand(
	float scale,
	HOL::HandSide side,
	const FingerHighlights& fingerHighlights,
	const FingerFlags& curledFingers,
	const std::vector<std::pair<HOL::FingerType, HOL::FingerType>>& tapPairs,
	bool showThumb)
{
	const ImVec2 size = ImVec2(scaleSize(scale, 100.0f), scaleSize(scale, 126.0f));
	ImGui::Dummy(size);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);

	// Mirror only the side-specific features. The four main fingers use explicit handed
	// slot ordering below so index/pinky stay on the correct side of the palm.
	const float sign = side == HOL::LeftHand ? -1.0f : 1.0f;
	const float palmWidth = scaleSize(scale, 44.0f);
	const float palmHeight = scaleSize(scale, 40.0f);
	const float fingerWidth = scaleSize(scale, 10.0f);
	const float curledFingerHeight = scaleSize(scale, 24.0f);
	const float fingerSpacing = scaleSize(scale, 4.0f);
	const float tipHeight = scaleSize(scale, 7.0f);
	const ImU32 lineColor = ImGui::GetColorU32(ImGuiCol_Text);
	const ImU32 palmColor = ImGui::GetColorU32(ImGuiCol_FrameBg);
	const ImU32 palmBorderColor = ImGui::GetColorU32(ImGuiCol_Border);
	const ImU32 fingerColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
	const ImU32 secondaryFingerColor = IM_COL32(232, 205, 96, 255);
	const ImU32 primaryFingerColor = IM_COL32(96, 214, 122, 255);

	const ImVec2 palmMin = ImVec2(center.x - palmWidth * 0.5f, center.y + scaleSize(scale, 6.0f));
	const ImVec2 palmMax = ImVec2(center.x + palmWidth * 0.5f, palmMin.y + palmHeight);
	const float cornerRounding = scaleSize(scale, 6.0f);

	drawList->AddRectFilled(palmMin, palmMax, palmColor, cornerRounding);
	drawList->AddRect(palmMin, palmMax, palmBorderColor, cornerRounding, 0, scaleSize(scale, 1.0f));

	std::array<ImVec2, HOL::FingerType_MAX> tipPositions{};

	auto drawFingerBlock
		= [&](ImVec2 blockMin, ImVec2 blockMax, HOL::FingerType finger, bool tipAtBottom)
	{
		drawList->AddRectFilled(blockMin, blockMax, fingerColor, scaleSize(scale, 2.0f));
		drawList->AddRect(
			blockMin, blockMax, palmBorderColor, scaleSize(scale, 2.0f), 0, scaleSize(scale, 1.0f));

		ImVec2 tipMin = blockMin;
		ImVec2 tipMax = blockMax;
		if (tipAtBottom)
		{
			tipMin.y = tipMax.y - tipHeight;
		}
		else
		{
			tipMax.y = tipMin.y + tipHeight;
		}

		if (fingerHighlights[finger] != HOL::InputGestureHighlight_None)
		{
			drawList->AddRectFilled(tipMin,
									tipMax,
									fingerHighlights[finger] == HOL::InputGestureHighlight_Primary
										? primaryFingerColor
										: secondaryFingerColor,
									scaleSize(scale, 2.0f));
		}

		// Store the center of the outer edge of the highlighted tip so pinch arcs can anchor
		// to the tip face, rather than to a corner of the block.
		ImVec2 tipCenter = ImVec2((tipMin.x + tipMax.x) * 0.5f, (tipMin.y + tipMax.y) * 0.5f);
		if (!tipAtBottom)
		{
			tipCenter.y = tipMin.y;
		}
		else
		{
			tipCenter.y = tipMax.y;
		}

		tipPositions[finger] = tipCenter;
	};

	auto drawStraightFinger = [&](float xOffset, float length, HOL::FingerType finger)
	{
		float x = center.x + xOffset - fingerWidth * 0.5f;
		ImVec2 blockMin = ImVec2(x, palmMin.y - scaleSize(scale, length));
		ImVec2 blockMax = ImVec2(x + fingerWidth, palmMin.y);
		drawFingerBlock(blockMin, blockMax, finger, false);
	};

	auto drawCurledFinger = [&](float xOffset, HOL::FingerType finger)
	{
		float x = center.x + xOffset - fingerWidth * 0.5f;
		ImVec2 blockMin = ImVec2(x, palmMin.y - scaleSize(scale, 3.0f));
		ImVec2 blockMax = ImVec2(x + fingerWidth, blockMin.y + curledFingerHeight);
		drawFingerBlock(blockMin, blockMax, finger, true);
	};

	auto drawFinger = [&](float xOffset, float length, HOL::FingerType finger)
	{
		if (curledFingers[finger])
		{
			drawCurledFinger(xOffset, finger);
		}
		else
		{
			drawStraightFinger(xOffset, length, finger);
		}
	};

	const float fingerSpan = palmWidth - fingerWidth;
	auto getFingerOffset = [&](int slot)
	{
		// Index and pinky align to the palm edges, with middle/ring distributed evenly
		// between them. Right hands invert the slot order so the thumb-facing side matches.
		float offset = -fingerSpan * 0.5f + (fingerSpan / 3.0f) * slot;
		return side == HOL::LeftHand ? offset : -offset;
	};

	if (showThumb)
	{
		// The thumb uses a horizontal block rather than the vertical finger layout above, so
		// it is drawn separately and mirrored across the palm.
		float thumbLength = scaleSize(scale, 22.0f);
		float thumbY = palmMax.y - scaleSize(scale, 16.0f);
		ImVec2 blockMin;
		ImVec2 blockMax;
		if (side == HOL::LeftHand)
		{
			blockMin = ImVec2(palmMin.x - thumbLength + fingerSpacing, thumbY);
			blockMax = ImVec2(palmMin.x + fingerSpacing, thumbY + fingerWidth);
		}
		else
		{
			blockMin = ImVec2(palmMax.x - fingerSpacing, thumbY);
			blockMax = ImVec2(palmMax.x + thumbLength - fingerSpacing, thumbY + fingerWidth);
		}

		drawList->AddRectFilled(blockMin, blockMax, fingerColor, scaleSize(scale, 2.0f));
		drawList->AddRect(
			blockMin, blockMax, palmBorderColor, scaleSize(scale, 2.0f), 0, scaleSize(scale, 1.0f));

		ImVec2 tipMin = blockMin;
		ImVec2 tipMax = blockMax;
		if (side == HOL::LeftHand)
		{
			tipMax.x = tipMin.x + tipHeight;
		}
		else
		{
			tipMin.x = tipMax.x - tipHeight;
		}

		if (fingerHighlights[HOL::FingerThumb] != HOL::InputGestureHighlight_None)
		{
			drawList->AddRectFilled(tipMin,
									tipMax,
									fingerHighlights[HOL::FingerThumb]
											== HOL::InputGestureHighlight_Primary
										? primaryFingerColor
										: secondaryFingerColor,
									scaleSize(scale, 2.0f));
		}

		ImVec2 thumbTipCenter = ImVec2((tipMin.x + tipMax.x) * 0.5f, (tipMin.y + tipMax.y) * 0.5f);
		if (side == HOL::LeftHand)
		{
			thumbTipCenter.x = tipMin.x;
		}
		else
		{
			thumbTipCenter.x = tipMax.x;
		}

		tipPositions[HOL::FingerThumb] = thumbTipCenter;
	}

	// Extended fingers use uneven heights so the glyph still reads as a hand. Curled fingers
	// ignore those values and use one shared height to read as a closed fist.
	drawFinger(getFingerOffset(0), 28.0f, HOL::FingerIndex);
	drawFinger(getFingerOffset(1), 38.0f, HOL::FingerMiddle);
	drawFinger(getFingerOffset(2), 34.0f, HOL::FingerRing);
	drawFinger(getFingerOffset(3), 26.0f, HOL::FingerLittle);

	for (const auto& tapPair : tapPairs)
	{
		ImVec2 start = tipPositions[tapPair.first];
		ImVec2 end = tipPositions[tapPair.second];
		ImVec2 direction = ImVec2(end.x - start.x, end.y - start.y);
		float directionLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);
		if (directionLength <= 0.0f)
		{
			continue;
		}

		direction.x /= directionLength;
		direction.y /= directionLength;
		ImVec2 normal = ImVec2(-direction.y, direction.x);
		float arrowLength = scaleSize(scale, 6.0f);
		float arrowWidth = scaleSize(scale, 3.0f);
		float tipGap = scaleSize(scale, 12.0f);
		// Keep the arc away from the tip blocks, then bulge it outward from the hand so it
		// reads as a separate gesture indicator rather than touching the highlighted tips.
		float curveDepth = scaleSize(scale, 12.0f) * (side == HOL::LeftHand ? 1.0f : -1.0f);

		ImVec2 curveStart = ImVec2(start.x + direction.x * tipGap, start.y + direction.y * tipGap);
		ImVec2 curveEnd = ImVec2(end.x - direction.x * tipGap, end.y - direction.y * tipGap);

		ImVec2 control = ImVec2((curveStart.x + curveEnd.x) * 0.5f - normal.x * curveDepth,
								(curveStart.y + curveEnd.y) * 0.5f - normal.y * curveDepth);

		drawList->AddBezierQuadratic(
			curveStart, control, curveEnd, lineColor, scaleSize(scale, 2.0f));

		ImVec2 nearStart = ImVec2(curveStart.x + direction.x * arrowLength,
								  curveStart.y + direction.y * arrowLength);
		ImVec2 nearEnd = ImVec2(curveEnd.x - direction.x * arrowLength,
								curveEnd.y - direction.y * arrowLength);

		drawList->AddTriangleFilled(
			curveStart,
			ImVec2(nearStart.x + normal.x * arrowWidth, nearStart.y + normal.y * arrowWidth),
			ImVec2(nearStart.x - normal.x * arrowWidth, nearStart.y - normal.y * arrowWidth),
			lineColor);
		drawList->AddTriangleFilled(
			curveEnd,
			ImVec2(nearEnd.x + normal.x * arrowWidth, nearEnd.y + normal.y * arrowWidth),
			ImVec2(nearEnd.x - normal.x * arrowWidth, nearEnd.y - normal.y * arrowWidth),
			lineColor);
	}
}

void HOL::UiGraphics::drawSingleBinding(
	float scale,
	const char* title,
	const char* output,
	const char* description,
	HOL::HandSide side,
	const FingerHighlights& fingerHighlights,
	const FingerFlags& curledFingers,
	const std::vector<std::pair<HOL::FingerType, HOL::FingerType>>& tapPairs,
	bool showThumb)
{
	ImGui::Text("%s", title);
	ImGui::SameLine();
	ImGui::TextDisabled("%s", output);
	ImGui::TextWrapped("%s", description);
	ImGui::PushID(title);
	drawInputGestureHand(scale, side, fingerHighlights, curledFingers, tapPairs, showThumb);
	ImGui::PopID();
	ImGui::Spacing();
}

void HOL::UiGraphics::drawSequenceBinding(float scale,
										  const char* title,
										  const char* output,
										  const char* description,
										  HOL::HandSide side,
										  const std::vector<FingerHighlights>& steps)
{
	ImGui::Text("%s", title);
	ImGui::SameLine();
	ImGui::TextDisabled("%s", output);
	ImGui::TextWrapped("%s", description);

	FingerFlags noCurledFingers{};
	noCurledFingers.fill(false);

	// Sequence gestures re-use the same hand glyph several times, keeping each step aligned
	// to a fixed top edge so the row does not wobble vertically.
	int stepIndex = 0;
	float sequenceStartY = ImGui::GetCursorPosY();
	for (const FingerHighlights& step : steps)
	{
		std::vector<std::pair<HOL::FingerType, HOL::FingerType>> tapPairs;
		for (int finger = 0; finger < HOL::FingerType_MAX; finger++)
		{
			HOL::FingerType fingerType = (HOL::FingerType)finger;
			if (fingerType != HOL::FingerThumb
				&& step[fingerType] == HOL::InputGestureHighlight_Primary)
			{
				tapPairs.push_back({HOL::FingerThumb, fingerType});
			}
		}

		ImGui::PushID(stepIndex++);
		ImGui::SetCursorPosY(sequenceStartY);
		drawInputGestureHand(scale, side, step, noCurledFingers, tapPairs, true);
		ImGui::PopID();

		if (stepIndex < (int)steps.size())
		{
			ImGui::SameLine();
			ImGui::SetCursorPosY(sequenceStartY + scaleSize(scale, 42.0f));
			ImGui::TextDisabled("->");
			ImGui::SameLine();
			ImGui::SetCursorPosY(sequenceStartY);
		}
	}

	ImGui::Spacing();
}

void HOL::UiGraphics::drawDualBinding(
	float scale,
	const char* title,
	const char* output,
	const char* description,
	const FingerHighlights& fingerHighlights,
	const FingerFlags& curledFingers,
	const std::vector<std::pair<HOL::FingerType, HOL::FingerType>>& tapPairs,
	bool showThumb)
{
	ImGui::Text("%s", title);
	ImGui::SameLine();
	ImGui::TextDisabled("%s", output);
	ImGui::TextWrapped("%s", description);

	ImGui::PushID(title);
	drawInputGestureHand(
		scale, HOL::LeftHand, fingerHighlights, curledFingers, tapPairs, showThumb);
	ImGui::SameLine();
	drawInputGestureHand(
		scale, HOL::RightHand, fingerHighlights, curledFingers, tapPairs, showThumb);
	ImGui::PopID();
	ImGui::Spacing();
}
