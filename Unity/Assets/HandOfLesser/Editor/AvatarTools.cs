using HOL;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace HOL
{
    class AvatarTools
    {
        public static Transform getFingerBone(GameObject avatarRoot, HandSide side, FingerType finger, FingerBendType joint)
        {
            Animator animator = avatarRoot.GetComponent<Animator>();

            // Ultimately we'll keep track of the bones after we duplicate them
            // but for now we just search for things manually
            Transform hand = animator.GetBoneTransform(side == HandSide.left ? HumanBodyBones.LeftHand : HumanBodyBones.RightHand);

            // Atm the fingers aren't even mapped, so do a simple search for them.
            Transform fingerRoot = getFingerRoot(hand, finger);

            switch(joint)
            {
                case FingerBendType.first:
                    return fingerRoot;
                case FingerBendType.second:
                    return fingerRoot.GetChild(0);
                case FingerBendType.third:
                    return fingerRoot.GetChild(0).GetChild(0);
                default:    // Also if splay
                    return null;
            }
        }

        private static Transform getFingerRoot(Transform hand, FingerType finger)
        {
            int childCount = hand.childCount;
            for (int i = 0; i < childCount; i++)
            {
                Transform child = hand.GetChild(i);
                if (child.childCount > 0 )
                {
                    if (child.name.ContainsInvariantCultureIgnoreCase(finger.propertyName()) )
                    {
                        return child;
                    }

                    if (finger == FingerType.pinky)
                    {
                        // pinky or little aaaahhhhhhh
                        if (child.name.ContainsInvariantCultureIgnoreCase("pinky"))
                        {
                            return child;
                        }
                    }
                }
            }

            Debug.Log("Could not find finger root! Finger: " + finger.propertyName());

            return null;
        }

    }
}
