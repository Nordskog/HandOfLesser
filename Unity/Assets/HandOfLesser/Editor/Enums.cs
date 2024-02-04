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
        // Proxy will be the smoothed value
        normal, proxy
    }

    enum AnimationClipPosition
    {
        negative, neutral, positive
    }

    enum TransmitType
    {
        full, alternating, packed
    }

    static class HandOfLesserExtensions
    {

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

        public static List<Enum> Values(this Enum theEnum)
        {
            return Enum.GetValues(theEnum.GetType()).Cast<Enum>().ToList();
        }
    }



}
