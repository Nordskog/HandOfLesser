using HOL;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEditor.Animations;
using UnityEngine;

namespace HOL
{
    class InterlacedFlipFlop
    {
        public static readonly int FLIPFLOP_BLENDTREE_COUNT = AnimationValues.TOTAL_JOINT_COUNT; // Bend joints ( including splay ) * fingers * hands
        public static readonly int FLIPFLOP_ANIMATION_COUNT = FLIPFLOP_BLENDTREE_COUNT * 4; // Positive and negative animation for each, for prev and next.

        public static int generateInterlacedFlipflopAnimation(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition position, PropertyType property)
        {
            // This just sets the proxy parameter to whatever position, and we blend between these to do the smoothing
            // Note that these should always go from -1 to 1, which will not necessarily be the case of the normal finger animations
            // Right now they all just use the AnimationClipPosition though
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getJointParameterName(side, finger, joint, property),
                    AnimationValues.getValueForPose(position) // See definition
                );

            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, property, position)));

            return 1;
        }

        public static void addParameters(AnimatorController controller)
        {
            controller.AddParameter(HOL.Resources.INTERLACE_BIT_OSC_PARAMETER_NAME, AnimatorControllerParameterType.Int);

            // All the params used for interlace
            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        // left and right hand joints
                        controller.AddParameter(HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.input_interlaced), AnimatorControllerParameterType.Float);

                        controller.AddParameter(HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.input_interlaced_first), AnimatorControllerParameterType.Float);
                        controller.AddParameter(HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.input_interlaced_second), AnimatorControllerParameterType.Float);

                        controller.AddParameter(HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.OSC_Packed_first), AnimatorControllerParameterType.Float);
                        controller.AddParameter(HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.OSC_Packed_second), AnimatorControllerParameterType.Float);
                    }
                }
            }
        }

        // Animations that set the first/second (current/prev) interlaced params to -1 or 1
        public static void generateAnimations()
        {
            HOL.Resources.createOutputDirectories();

            int animationProcessed = 0;
            ProgressDisplay.updateAnimationProgress(animationProcessed, FLIPFLOP_ANIMATION_COUNT);

            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        // drive first/second interlaced params from negative to positive so we can set them dynamically
                        animationProcessed += generateInterlacedFlipflopAnimation(side, finger, joint, AnimationClipPosition.negative, PropertyType.input_interlaced_first);
                        animationProcessed += generateInterlacedFlipflopAnimation(side, finger, joint, AnimationClipPosition.positive, PropertyType.input_interlaced_first);

                        animationProcessed += generateInterlacedFlipflopAnimation(side, finger, joint, AnimationClipPosition.negative, PropertyType.input_interlaced_second);
                        animationProcessed += generateInterlacedFlipflopAnimation(side, finger, joint, AnimationClipPosition.positive, PropertyType.input_interlaced_second);

                        ProgressDisplay.updateAnimationProgress(animationProcessed, FLIPFLOP_ANIMATION_COUNT);
                    }
                }
            }
        }
    }

}
