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

        public static string getAnimationClipName(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition position, PropertyType propertyType)
        {
            // getJointParameterName() will add the proxy suffix
            string clipName = getJointParameterName(side, finger, joint, propertyType);
            clipName = clipName + "_" + position.propertyName();

            return clipName;
        }

        public static string getJointParameterName( HandSide side, FingerType finger, FingerBendType joint, PropertyType propertyType)
        {
            // Shared osc name with the side tacked on
            // This is what will ultimately drive the finger animations, or rather the smoothened proxy that will
            // be used to drive them.
            // Basically OSC name -> (right/left state machine) -> Parameter Name -> Proxy name -> animations
            string name = side.propertyName() + "_" + getJointOSCName(finger, joint);

            // The only other place we will be generating anything for the proxy stuff is the animation clip name,
            // which uses the value from this function, so this should be the only place we add it.
            if (propertyType == PropertyType.proxy)
            {
                name = name + "_proxy";
            }

            return name;
        }

        public static string getJointOSCName(FingerType finger, FingerBendType joint)
        {
            StringBuilder builder = new StringBuilder();

            builder.Append(finger.propertyName());

            if (joint == FingerBendType.splay)
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

        public static string getJointPropertyName(HandSide? side, FingerType finger, FingerBendType joint)
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

            if (joint == FingerBendType.splay)
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
