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
        public float end;

        public MotionRange( float start, float end )
        {
            this.start = start;
            this.end = end;
        }
    }

    class Config
    {
        // Range values must match app!
        public static MotionRange[] CommonCurlRange = new MotionRange[3] {
			new MotionRange(-1.2f,  1    ),
            new MotionRange(-1.0f,  1    ),
            new MotionRange(-1.0f,  1    ),
        };

        public static MotionRange[] ThumbCurlRange = new MotionRange[3] {
            new MotionRange(-4.1f, 0.4f   ), 
            new MotionRange(-0.5f, 2.5f	  ),
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
}
