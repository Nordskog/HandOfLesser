using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;

namespace HOL
{
    class ProgressDisplay
    {
        public static void updateAnimationProgress(int processed, int total)
        {
            float progress = (float)processed / (float)total;
            EditorUtility.DisplayProgressBar("Creating Animations", "Animation " + (processed + 1) + " of " + total, progress);
        }

        public static void updateBlendtreeProgress(int processed, int total)
        {
            float progress = (float)processed / (float)total;
            EditorUtility.DisplayProgressBar("Creating Controller", "Blendtree " + (processed + 1) + " of " + total, progress);
        }

        public static void clearProgress()
        {
            EditorUtility.ClearProgressBar();
        }

    }
}
