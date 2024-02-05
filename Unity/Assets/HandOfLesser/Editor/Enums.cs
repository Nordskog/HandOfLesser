using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HOL
{
    enum FingerType
    {
        index,
        middle,
        ring,
        pinky,
        thumb
    }

    enum FingerBendType
    {
        first,
        second,
        third,
        splay
    }

    enum HandSide
    {
        left,
        right
    }

    enum PropertyType
    {
        OSC_Full,       // This should always be the same as input, as it required no processing
        OSC_Alternating,      
        OSC_Packed,     // We will use packed for network and full for local, so need to separate them

        input,      // full joint path with left/right qualified
        smooth,     // Output used for smoothing, used to drive avatarRig
        avatarRig   // Humanoid rig parameters. In the future humanoid rig will not be used.
    }

    enum AnimationClipPosition
    {
        negative, neutral, positive
    }

    enum TransmitType
    {
        full,
        alternating,
        packed
    }

    static class HandOfLesserExtensions
    {
        /////////////
        // TransmitType
        ////////////

        public static PropertyType toPropertyType(this TransmitType transmitType)
        {
            // In the editor there is a space, but the actual attribute does not have one. Unity what the hell.
            switch (transmitType)
            {
                case TransmitType.full: return PropertyType.OSC_Full;
                case TransmitType.alternating: return PropertyType.OSC_Alternating;
                case TransmitType.packed: return PropertyType.OSC_Packed;
                default:
                    return PropertyType.OSC_Full;
            }
        }

        ///////////////////////////
        // Hand
        ///////////////////////////

        public static string propertyName(this HandSide side)
        {
            // In the editor there is a space, but the actual attribute does not have one. Unity what the hell.
            switch (side)
            {
                case HandSide.left: return "LeftHand";
                case HandSide.right: return "RightHand";
                default: return "INVALID_ENUM";
            }
        }

        ///////////////////////////
        // Finger
        ///////////////////////////

        public static string propertyName(this FingerType finger)
        {
            switch (finger)
            {
                case FingerType.index: return "Index";
                case FingerType.middle: return "Middle";
                case FingerType.ring: return "Ring";
                case FingerType.pinky: return "Little";
                case FingerType.thumb: return "Thumb";
                default: return "INVALID_ENUM";
            }
        }

        ///////////////////////////
        // Joint
        ///////////////////////////

        public static string propertyName(this FingerBendType joint)
        {
            switch (joint)
            {
                case FingerBendType.first: return "1";
                case FingerBendType.second: return "2";
                case FingerBendType.third: return "3";
                case FingerBendType.splay: return "splay";
                default: return "INVALID_ENUM";
            }
        }

        ///////////////////////////
        // AnimationClipPosition
        ///////////////////////////

        public static string propertyName(this AnimationClipPosition pos)
        {
            switch (pos)
            {
                case AnimationClipPosition.negative: return "Negative";
                case AnimationClipPosition.neutral: return "Neutral";
                case AnimationClipPosition.positive: return "Positive";
                default: return "INVALID_ENUM";
            }
        }

        /////////////////////
        // PropertyType
        /////////////////////
        public static string propertyName(this PropertyType prop)
        {
            switch (prop)
            {
                case PropertyType.OSC_Full: return "input"; // No processing required, so just treat as input
                case PropertyType.OSC_Alternating: return "alternating";
                case PropertyType.OSC_Packed: return "packed";
                case PropertyType.input: return "input"; 
                case PropertyType.smooth: return "smooth";
                case PropertyType.avatarRig: return "avatarRig";
                default:
                    return "";
            }
        }

        public static string addPrefixNamespace(this PropertyType prop, string str)
        {
            return prop.propertyName() + "/" + str;
        }

        public static List<Enum> Values(this Enum theEnum)
        {
            return Enum.GetValues(theEnum.GetType()).Cast<Enum>().ToList();
        }
    }



}
