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

        public static readonly string NAMESPACE = "HOL";

        public static readonly string HAND_SIDE_OSC_PARAMETER_NAME = addNamespacePrefix( 
            PropertyType.OSC_Alternating.addPrefixNamespace("hand_side"));

        public static readonly string REMOTE_SMOOTHING_PARAMETER_NAME = addNamespacePrefix("remote_smoothing");

        public static readonly string ALWAYS_1_PARAMETER = addNamespacePrefix( "always_one");

        private static string addNamespacePrefix(string param)
        {
            return NAMESPACE + "/" + param;
        }

        public static string getAnimationClipName(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition position, PropertyType propertyType)
        {
            string clipName = getJointParameterName(side, finger, joint, propertyType);
            clipName = clipName + "_" + position.propertyName();

            // Probably don't want slashes in filenames
            return clipName.Replace('/', '_');
        }

        public static string getJointParameterName( HandSide? side, FingerType finger, FingerBendType joint, PropertyType propertyType)
        {
            bool hasSide = true;

            switch(propertyType)
            {
                case PropertyType.OSC_Alternating:
                case PropertyType.OSC_Packed:
                {
                    // the param may be supplied even when dealing with these.
                    // This is easier than checking the enum over and over.
                    hasSide = false;
                    break;
                }
                default:
                {
                    if (!side.HasValue)
                    {
                        throw new ArgumentException("Side may only be null if the PropertType is OSC Alternating or Packed!");
                    }
                    break;
                }
            }

            if (propertyType == PropertyType.avatarRig)
            {
                // For now driving humanoid rig, will use our own animations for this in the future
                return getHumanoidRigJointPropertyName(side.Value, finger, joint);
            }

            string name = getPlainJointParameterName(finger, joint);
            if (hasSide)
            {
                // Some param that includes the hand side
                name = side.Value.propertyName() + "_" + name;
            }

            name = propertyType.addPrefixNamespace(name);

            // All parameters should be lower case, except for the HOL/ prefix.
            // If you want to change this you will have to mess with the c++ side too.
            // Note that getHumanoidRigJointPropertyName() returns above without
            // converting to lowercase. This is because the humanoid rig paramenters 
            // need to be a specific case, and the enum propertyNames reflect this.
            name = name.ToLower();

            return addNamespacePrefix(name);
        }

        // Generic finger/joint parameter name with no qualifiers, including no left/right side
        private static string getPlainJointParameterName(FingerType finger, FingerBendType joint)
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

            return builder.ToString();
        }

        private static string getHumanoidRigJointPropertyName(HandSide? side, FingerType finger, FingerBendType joint)
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
