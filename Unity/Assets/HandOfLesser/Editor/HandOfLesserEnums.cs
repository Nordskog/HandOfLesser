using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HOL
{
    enum Finger
    {
        index,
        middle,
        ring,
        pinky,
        thumb
    }

    enum Joint
    {
        first,
        second,
        third,
    }

    enum HandSide
    {
        left,
        right
    }

    enum JointMotionDirection
    {
        curl, 
        splay
    }

    enum AnimationClipPosition
    {
        negative, neutral, positive
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

        public static string propertyName(this Finger finger)
        {
            switch (finger)
            {
                case Finger.index: return "Index";
                case Finger.middle: return "Middle";
                case Finger.ring: return "Ring";
                case Finger.pinky: return "Little";
                case Finger.thumb: return "Thumb";
                default: return "INVALID_ENUM";
            }

        }

        ///////////////////////////
        // Joint
        ///////////////////////////

        public static string propertyName(this Joint joint)
        {
            switch (joint)
            {
                case Joint.first: return "1";
                case Joint.second: return "2";
                case Joint.third: return "3";
                default: return "INVALID_ENUM";
            }
        }

        ///////////////////////////
        // JointMotionDirection
        ///////////////////////////

        public static string propertyName(this JointMotionDirection finger)
        {
            switch (finger)
            {
                case JointMotionDirection.curl: return "Stretched";
                case JointMotionDirection.splay: return "Spread";
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
    }


}
