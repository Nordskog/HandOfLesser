using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HOL
{
    struct MotionRange
    {
        public float start;
        public float center;
        public float end;

        public MotionRange( float start, float end )
        {
            this.start = start;
            this.end = end;
            this.center = start + end / 2f;
        }

        public MotionRange(float start, float center, float end)
        {
            this.start = start;
            this.end = end;
            this.center = center;
        }
    }

    class Config
    {
        // Range values must match app!

        // These are in humanoid rig space. We know range in degrees between
        // -1 and 1, and use that as a basis ( see HUMAN_RIG_RANGE in the exe )
        public static MotionRange[] CommonCurlRange = new MotionRange[3] {
			new MotionRange(-1.3f,  1    ),
            new MotionRange(-1.0f,  1    ),
            new MotionRange(-1.0f,  1    ),
        };

        public static MotionRange[] ThumbCurlRange = new MotionRange[3] {
            new MotionRange(-4.1f, 0.4f   ), 
            new MotionRange(-0.5f, 1f	  ),
            new MotionRange(-2f, 1.27f  ),
        };

        public static MotionRange[] FingersplayRange = new MotionRange[5] {
            new MotionRange(-2.0f,  1       ), // index
            new MotionRange(-2.0f,  1       ),
            new MotionRange(-3.0f,  1       ),
            new MotionRange(-3.0f,  1.5f       ),
            new MotionRange(-2.0f,  1.5f    ), // thumb
        };
    }

    class SkeletalConfig
    {
        // Range values must match app!

        // These assume Y forward, Z up.
        
        public static MotionRange[] CommonCurlRange = new MotionRange[3] {
            new MotionRange(30f, 0,  -100f    ),
            new MotionRange(30f, 0,  -100f    ),
            new MotionRange(30f, 0,  -100f    ),
        };

        public static MotionRange[] ThumbCurlRange = new MotionRange[3] {
            new MotionRange(75, 45, -25f   ),
            new MotionRange(0, -27 -100f   ),
            new MotionRange(25, 0, -90f  ),
        };

        // Defined for left, flip for right
        public static MotionRange[] FingersplayRange = new MotionRange[5] {
            new MotionRange(-25.0f,  0,  60.0f       ),
            new MotionRange(-25.0f,  0,  60.0f       ),
            new MotionRange( -60.0f,  0, 25.0f       ), // index
            new MotionRange( -60.0f,  0, 25.0f       ),
            new MotionRange(-40.0f,  0,  40.0f       ), // thumb
        };
    }
}
