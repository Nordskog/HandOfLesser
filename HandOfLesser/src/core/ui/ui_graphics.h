#pragma once

#include <array>
#include <utility>
#include <vector>

#include <HandOfLesserCommon.h>

namespace HOL
{
	enum InputGestureHighlight
	{
		InputGestureHighlight_None,
		InputGestureHighlight_Secondary,
		InputGestureHighlight_Primary,
		InputGestureHighlight_MAX
	};

	class UiGraphics
	{
	public:
		using FingerHighlights = std::array<HOL::InputGestureHighlight, HOL::FingerType_MAX>;
		using FingerFlags = std::array<bool, HOL::FingerType_MAX>;

		static void drawInputGestureHand(
			float scale,
			HOL::HandSide side,
			const FingerHighlights& fingerHighlights,
			const FingerFlags& curledFingers,
			const std::vector<std::pair<HOL::FingerType, HOL::FingerType>>& tapPairs,
			bool showThumb = true);

		static void
		drawSingleBinding(float scale,
						  const char* title,
						  const char* output,
						  const char* description,
						  HOL::HandSide side,
						  const FingerHighlights& fingerHighlights,
						  const FingerFlags& curledFingers,
						  const std::vector<std::pair<HOL::FingerType, HOL::FingerType>>& tapPairs
						  = {},
						  bool showThumb = true);

		static void drawSequenceBinding(float scale,
										const char* title,
										const char* output,
										const char* description,
										HOL::HandSide side,
										const std::vector<FingerHighlights>& steps);

		static void
		drawDualBinding(float scale,
						const char* title,
						const char* output,
						const char* description,
						const FingerHighlights& fingerHighlights,
						const FingerFlags& curledFingers,
						const std::vector<std::pair<HOL::FingerType, HOL::FingerType>>& tapPairs
						= {},
						bool showThumb = true);
	};
} // namespace HOL
