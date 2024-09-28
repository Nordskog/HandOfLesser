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

        public static void setClipProperty(ref AnimationClip clip, string property, AnimationCurve curve)
        {
            // Single keyframe
            AnimationUtility.SetEditorCurve(
                    clip,
                    EditorCurveBinding.FloatCurve(
                        string.Empty,
                        typeof(Animator),
                        property
                        ),
                curve);
        }

        public static void setClipProperty(ref AnimationClip clip, string property, float startTime, float startValue, float EndTime, float endValue)
        {
            // Single keyframe
            AnimationUtility.SetEditorCurve(
                    clip,
                    EditorCurveBinding.FloatCurve(
                        string.Empty,
                        typeof(Animator),
                        property
                        ),
                AnimationCurve.Linear(startTime, startValue, EndTime, endValue));
        }

        public static void setClipPropertyLocalRotation( ref AnimationClip clip, Transform avatarRoot, Transform trans, Quaternion value)
        {
            AnimationCurve curveX = new AnimationCurve();
            AnimationCurve curveY = new AnimationCurve();
            AnimationCurve curveZ = new AnimationCurve();
            AnimationCurve curveW = new AnimationCurve();

            curveX.AddKey(0f, value.x);
            curveY.AddKey(0f, value.y);
            curveZ.AddKey(0f, value.z);
            curveW.AddKey(0f, value.w);

            // I guess relative to avatar is fine
            string relativePath = AnimationUtility.CalculateTransformPath(trans, avatarRoot);

            clip.SetCurve(relativePath, typeof(Transform), "localRotation.x", curveX);
            clip.SetCurve(relativePath, typeof(Transform), "localRotation.y", curveY);
            clip.SetCurve(relativePath, typeof(Transform), "localRotation.z", curveZ);
            clip.SetCurve(relativePath, typeof(Transform), "localRotation.w", curveW);
        }

    }
}
