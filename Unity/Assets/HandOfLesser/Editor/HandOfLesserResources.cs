using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEngine;

namespace HOL
{
    class Resources
    {
        private static readonly string HANDOFLESSER_PATH = "Assets/HandOfLesser";

        public static string getPackedAnimationClipName(Finger finger, Joint? joint, int leftStep, int rightStep)
        {
            StringBuilder builder = new StringBuilder();

            // Splay if only finger provided, curl if joint also provided
            if (joint.HasValue)
            {
                builder.Append("curl");
            }
            else
            {
                builder.Append("splay");
            }

            builder.Append("_");
            builder.Append(finger.propertyName());
            builder.Append("_");

            // Only if curl
            if (joint.HasValue)
            {
                builder.Append(joint.Value.propertyName());
                builder.Append("_");
            }

            builder.Append(leftStep.ToString());
            builder.Append("_");
            builder.Append(rightStep.ToString());

            return builder.ToString();
        }

        public static string getJointOSCName(Finger finger, Joint? joint)
        {
            StringBuilder builder = new StringBuilder();

            builder.Append(finger.propertyName());

            if (joint.HasValue)
            {
                builder.Append("_");
                builder.Append(joint.Value.propertyName());
                builder.Append("_");
                builder.Append("curl");
            }
            else
            {
                builder.Append("_");
                builder.Append("splay");
            }

            // This is what we'll be using for osc stuff, so keep it all lower case
            return builder.ToString().ToLower();
        }

        public static string getJointPropertyName(HandSide? side, Finger finger, Joint? joint)
        {
            // Right Hand.Thumb.1 Stretched
            StringBuilder builder = new StringBuilder();

            // We reuse this for blendtree names that cover both hands, so don't need side
            if (side.HasValue)
            {
                builder.Append(side.Value.propertyName());
                builder.Append(".");
            }

            builder.Append(finger.propertyName());

            if (joint.HasValue)
            {
                builder.Append(".");
                builder.Append(joint.Value.propertyName());
                builder.Append(" ");
                builder.Append("Stretched");
            }
            else
            {
                builder.Append(".");
                builder.Append("Spread");
            }

            return builder.ToString();
        }

        public static string getAnimationOutputPath(string animationClipName)
        {
            return Path.Combine("Assets", "HandOfLesser", "generated", "animations", animationClipName + ".anim");
        }

        public static string getAnimationControllerOutputPath()
        {
            return Path.Combine("Assets", "HandOfLesser", "generated", "handoflesser_controller" + ".controller");
        }

        public static void createOutputDirectories()
        {
            if (!AssetDatabase.IsValidFolder("Assets/HandOfLesser"))
            {
                AssetDatabase.CreateFolder("Assets", "HandOfLesser");
            }

            if (!AssetDatabase.IsValidFolder("Assets/HandOfLesser/generated"))
            {
                AssetDatabase.CreateFolder("Assets/HandOfLesser", "generated");
            }

            if (!AssetDatabase.IsValidFolder("Assets/HandOfLesser/generated/animations"))
            {
                AssetDatabase.CreateFolder("Assets/HandOfLesser/generated", "animations");
            }
        }

        public static AvatarMask getBothHandsMask()
        {
            return AssetDatabase.LoadAssetAtPath<AvatarMask>(HANDOFLESSER_PATH + "/vrc_handsonly.mask");
        }

        public static string getParametersPath()
        {
            return HANDOFLESSER_PATH + "/generated/handoflesser_parameters.asset";
        }

    }
}
