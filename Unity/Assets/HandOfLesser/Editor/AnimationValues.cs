using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HOL
{
    class AnimationValues
    {
        public static readonly int TOTAL_JOINT_COUNT = 4 * 5 * 2;

        public static float getValueForPose(AnimationClipPosition clipPos)
        {
            // Make configurable? Simple -1 0 1 for now
            // Curl is -1 to 1, negative value curling.

            // Splay Negative moves outwards ( palm facing down )
            // -3 to +3 seems reasonable for splay, need to calibrate later.
            // The leap OSC animations use -1 to 1 though. Maybe animation preview just looks wonky?

            // Will be different for individual joints too

            switch (clipPos)
            {
                case AnimationClipPosition.negative: return -1;
                case AnimationClipPosition.neutral: return 0;
                case AnimationClipPosition.positive: return 1;
            }

            return 0;
        }

        public static float getHumanoidValue( FingerType finger, FingerBendType joint,  AnimationClipPosition clipPos)
        {
            // Same as getValueForPose(), but returns our configured values
            // instead of -1 to +1, as that doesn't cover the full range of motion.
            // Remember that the app side must be using the same range values.
            MotionRange range;

            if (joint == FingerBendType.splay)
            {
                range = HOL.Config.FingersplayRange[(int)finger];
            }
            else
            {
                if (finger == FingerType.thumb)
                {
                    range = HOL.Config.ThumbCurlRange[(int)joint];
                }
                else
                {
                    range = HOL.Config.CommonCurlRange[(int)joint];
                }
            }

            switch (clipPos)
            {
                case AnimationClipPosition.negative: return range.start;
                case AnimationClipPosition.neutral: return 0;   // Don't use this
                case AnimationClipPosition.positive: return range.end;
            }
            return 0;
        }

        private static MotionRange getSkeletalRange( HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition clipPos)
        {
            // Same as getValueForPose(), but returns our configured values
            // instead of -1 to +1, as that doesn't cover the full range of motion.
            // Remember that the app side must be using the same range values.
            MotionRange range;

            if (joint == FingerBendType.splay)
            {
                range = HOL.SkeletalConfig.FingersplayRange[(int)finger];

                if (side == HandSide.right)
                {
                    // Swap and flip splay
                    (range.start, range.end) = (-range.end, -range.start); 
                }

            }
            else
            {
                if (finger == FingerType.thumb)
                {
                    range = HOL.SkeletalConfig.ThumbCurlRange[(int)joint];
                }
                else
                {
                    range = HOL.SkeletalConfig.CommonCurlRange[(int)joint];
                }
            }

            return range;
        }

        public static float getSkeletalValueFromCenter(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition clipPos)
        {
            return getSkeletalValue(side, finger, joint, clipPos) - getSkeletalValue(side, finger, joint, AnimationClipPosition.neutral);
        }

        public static float getSkeletalValue(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition clipPos)
        {
            MotionRange range = getSkeletalRange(side, finger, joint, clipPos);

            switch (clipPos)
            {
                case AnimationClipPosition.negative: return range.start;
                case AnimationClipPosition.neutral: return range.center;
                case AnimationClipPosition.positive: return range.end;
            }
            return 0;
        }
    }
}
