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

        public static string getAnimationClipName(HandSide side, Finger finger, Joint joint, AnimationClipPosition position)
        {
            string clipName = getJointParameterName(side, finger, joint);
            return clipName + "_" + position.propertyName();
        }

        public static string getJointParameterName( HandSide side, Finger finger, Joint joint)
        {
            // Shared osc name with the side tacked on
            return side.propertyName() + "_" + getJointOSCName(finger, joint);
        }

        public static string getJointOSCName(Finger finger, Joint joint)
        {
            StringBuilder builder = new StringBuilder();

            builder.Append(finger.propertyName());

            if (joint == Joint.splay)
            {
                builder.Append("_");
                builder.Append("splay");
            }
            else
            {
                builder.Append("_");
                builder.Append(joint.propertyName());
                builder.Append("_");
                builder.Append("curl");
            }

            // This is what we'll be using for osc stuff, so keep it all lower case
            return builder.ToString().ToLower();
        }

        public static string getJointPropertyName(HandSide? side, Finger finger, Joint joint)
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

            if (joint == Joint.splay)
            {
                builder.Append(".");
                builder.Append("Spread");
            }
            else
            {
                builder.Append(".");
                builder.Append(joint.propertyName());
                builder.Append(" ");
                builder.Append("Stretched");
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
