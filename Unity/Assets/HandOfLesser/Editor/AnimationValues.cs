using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HOL
{
    class AnimationValues
    {
        //public static readonly int BLENDTREE_COUNT = (5 * 4) * 2; // 3 curls, 1 splay, 5 fingers, 2 hands.
        //public static readonly int ANIMATION_COUNT = BLENDTREE_COUNT * 2; // Open and close. Will be more once we get fancy with bones.
        public static readonly string HAND_SIDE_OSC_PARAMETER_NAME = "hand_side";
        public static readonly string REMOTE_SMOOTHING_PARAMETER_NAME = "HOL_remote_smoothing";

        public static readonly string ALWAYS_1_PARAMETER = "HOL_always_one";

        // There are some animations that should set parameters to -1 or 1.
        // Instead they set them to -40 or 10. I have no idea why.
        // Until someone can explain this to me, we multiply the values by this
        public static readonly float SHOULD_BE_ONE_BUT_ISNT = 1f / 40f;

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
    }
}
