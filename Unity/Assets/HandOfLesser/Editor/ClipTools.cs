using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEngine;

namespace HOL
{
    class ClipTools
    {
        public static void saveClip(AnimationClip clip, string savePath)
        {
            AssetDatabase.CreateAsset(clip, savePath);
        }

        public static void setClipProperty(ref AnimationClip clip, string property, float value)
        {
            // Single keyframe
            AnimationUtility.SetEditorCurve(
                    clip,
                    EditorCurveBinding.FloatCurve(
                        string.Empty,
                        typeof(Animator),
                        property
                        ),
                AnimationCurve.Linear(0, value, 0, value));
        }

    }
}
