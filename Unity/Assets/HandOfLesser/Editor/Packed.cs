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
    class Packed
    {
        private static readonly int STEP_COUNT = 16; // 4 bits worth
        private static readonly int PACKED_BLENDTREE_COUNT = AnimationValues.TOTAL_JOINT_COUNT * 2; // all joints for both hands

        private static readonly int PACKED_ANIMATION_COUNT_RIGHT = AnimationValues.TOTAL_JOINT_COUNT / 2; // neg/pos for each joint on one hand
        private static readonly int PACKED_ANIMATION_COUNT_LEFT = (AnimationValues.TOTAL_JOINT_COUNT / 2 ) * STEP_COUNT; // 16 steps for each joint on one hand

        private static readonly int PACKED_ANIMATION_COUNT = PACKED_ANIMATION_COUNT_RIGHT + PACKED_ANIMATION_COUNT_LEFT;
        private static readonly int PACKED_VALUE_COUNT = 256;    // Max an 8bit int can store

        private static int generateSingleBlendTree(BlendTree parent, List<ChildMotion> childTrees, HandSide side, FingerType finger, FingerBendType joint, PropertyType inputProperty, PropertyType outputProperty)
        {
            BlendTree tree = new BlendTree();
            AssetDatabase.AddObjectToAsset(tree, parent);

            tree.name = HOL.Resources.getJointParameterName(null, finger, joint, PropertyType.OSC_Packed);
            tree.blendType = BlendTreeType.Simple1D;
            tree.useAutomaticThresholds = false;    
            tree.blendParameter = HOL.Resources.getJointParameterName(null, finger, joint, inputProperty);

            childTrees.Add(new ChildMotion()
            {
                directBlendParameter = HOL.Resources.ALWAYS_1_PARAMETER,
                motion = tree,
                timeScale = 1,
            });

            if (side == HandSide.left)
            {
                // 0-15 -> 0
                // 16-31 -> 1
                // Repeat.
                // Right denotes the left step animation that each section will paly.
                // You only need to set the threshold at the beginning and the end,
                // so 0 and 15 for the first step

                for (int i = 0; i < STEP_COUNT; i++)
                {
                    // Start
                    {
                        string animationPath = HOL.Resources.getAnimationOutputPath(HOL.Resources.getPackedAnimationClipName(finger, joint, i, outputProperty));
                        AnimationClip animation = AssetDatabase.LoadAssetAtPath<AnimationClip>(animationPath);

                        float threshold = i*STEP_COUNT;
                        tree.AddChild(animation, threshold);
                    }

                    // End
                    {
                        string animationPath = HOL.Resources.getAnimationOutputPath(HOL.Resources.getPackedAnimationClipName(finger, joint, i, outputProperty));
                        AnimationClip animation = AssetDatabase.LoadAssetAtPath<AnimationClip>(animationPath);

                        float threshold = i * STEP_COUNT + ( STEP_COUNT - 1 );
                        tree.AddChild(animation, threshold);
                    }
                }
            }
            else
            {
                // 0 - negative 
                // 15 - positive
                // 16 - negative
                // 31 - positive
                // Repeat 16 times
                // This extracts the interleaved values for the right hand

                for (int i = 0; i < PACKED_VALUE_COUNT; i+=STEP_COUNT)
                {
                    // Negative
                    {
                        string animationPath = HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(HandSide.right, finger, joint, outputProperty, AnimationClipPosition.negative));
                        AnimationClip animation = AssetDatabase.LoadAssetAtPath<AnimationClip>(animationPath);

                        float threshold = i;
                        tree.AddChild(animation, threshold);
                    }

                    // Positive
                    {
                        string animationPath = HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(HandSide.right, finger, joint, outputProperty, AnimationClipPosition.positive));
                        AnimationClip animation = AssetDatabase.LoadAssetAtPath<AnimationClip>(animationPath);

                        float threshold = i + (STEP_COUNT - 1);
                        tree.AddChild(animation, threshold);
                    }
                }
            }

            return 1;
        }

        public static void populatePackedLayer(AnimatorController controller)
        {
            AnimatorControllerLayer layer = ControllerLayer.inputPacked.findLayer(controller);

            // State within this controller. TODO: attach to stuff
            AnimatorState rootState = layer.stateMachine.AddState("HOLPacked");
            rootState.writeDefaultValues = true; // Must be true or values are multiplied depending on umber of blendtrees in controller!?!?!

            // Blendtree at the root of our state
            BlendTree rootBlendtree = new BlendTree();
            AssetDatabase.AddObjectToAsset(rootBlendtree, rootState);

            rootBlendtree.name = "HandRoot";
            rootBlendtree.blendType = BlendTreeType.Direct;
            rootBlendtree.useAutomaticThresholds = false;
            rootBlendtree.blendParameter = HOL.Resources.ALWAYS_1_PARAMETER;
            
            rootState.motion = rootBlendtree;

            int blendtreesProcessed = 0;
            ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, PACKED_BLENDTREE_COUNT);

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            List<ChildMotion> childTrees = new List<ChildMotion>();

            // We now repeat this for interlaced, first and second using different inputs and writing to different outputs
            foreach (FingerType finger in new FingerType().Values())
            {
                foreach (FingerBendType joint in new FingerBendType().Values())
                {
                    // interlaced
                    blendtreesProcessed += generateSingleBlendTree(rootBlendtree, childTrees, HandSide.left, finger, joint, PropertyType.OSC_Packed, PropertyType.input_interlaced);
                    blendtreesProcessed += generateSingleBlendTree(rootBlendtree, childTrees, HandSide.right, finger, joint, PropertyType.OSC_Packed, PropertyType.input_interlaced);

                    // interlaced_first
                    blendtreesProcessed += generateSingleBlendTree(rootBlendtree, childTrees, HandSide.left, finger, joint, PropertyType.OSC_Packed_first, PropertyType.input_interlaced_first);
                    blendtreesProcessed += generateSingleBlendTree(rootBlendtree, childTrees, HandSide.right, finger, joint, PropertyType.OSC_Packed_first, PropertyType.input_interlaced_first);

                    // interlaced second
                    blendtreesProcessed += generateSingleBlendTree(rootBlendtree, childTrees, HandSide.left, finger, joint, PropertyType.OSC_Packed_second, PropertyType.input_interlaced_second);
                    blendtreesProcessed += generateSingleBlendTree(rootBlendtree, childTrees, HandSide.right, finger, joint, PropertyType.OSC_Packed_second, PropertyType.input_interlaced_second);
                    ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, PACKED_BLENDTREE_COUNT);
                }
            }

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            // Have to be added like this in order to set directblendparameter
            rootBlendtree.children = childTrees.ToArray();
        }

        // 0 to 15
        private static float getValueAtStep(float step, float stepCount, float startVal, float endVal)
        {
            // basically a re-range
            // Steps 0-15, or 16 steps. Subtract 1 from count to make the math work.
            // Also works for 0-255 if stepCount is 256
            float ratio = step / (stepCount - 1);
            return (ratio * endVal) + ((1f - ratio) * startVal);
        }

        private static void generatePackedAnimationLeft(FingerType finger, FingerBendType joint, int leftStep, PropertyType outputProperty)
        {
            // Animations for steps 0-15 of the left hand only
            // drives input
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getJointParameterName(HandSide.left, finger, joint, outputProperty),
                    getValueAtStep(leftStep, STEP_COUNT, -1, 1)
                );

            // Note separate animation clip name getter
            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getPackedAnimationClipName(finger, joint, leftStep, outputProperty)));
        }

        public static int generatedPackedAnimationRight(FingerType finger, FingerBendType joint, AnimationClipPosition position, PropertyType outputProperty)
        {
            // Basically just a negative and positive animation for each joint, with a multiplier because unity is a buggy mess
            // Drives input
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getJointParameterName(HandSide.right, finger, joint, outputProperty),
                    AnimationValues.getValueForPose(position)
                );

            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(HandSide.right, finger, joint, outputProperty, position)));

            return 1;
        }

        public static void generateAnimations(bool interlaced)
        {
            HOL.Resources.createOutputDirectories();

            int animationProcessed = 0;
            ProgressDisplay.updateAnimationProgress(animationProcessed, PACKED_ANIMATION_COUNT);

            PropertyType[] properties;
            if (interlaced)
            {
                properties = new PropertyType[] { PropertyType.input_interlaced, PropertyType.input_interlaced_first, PropertyType.input_interlaced_second };
            }
            else
            {
                properties = new PropertyType[] { PropertyType.input };
            }

            foreach (FingerType finger in new FingerType().Values())
            {
                foreach (FingerBendType joint in new FingerBendType().Values())
                {
                    // Because reasons, we need 3 sets of packed animations:
                    // #1: normal, output to interlaced
                    // #2 output to interlaced_first
                    // # output to interlaced_second

                    foreach(PropertyType property in properties)
                    {
                        // We use two different methods for unpacking the data into left/right values.
                        // See each generator method for details, and generateSingleBlendTree() for how they are used
                        animationProcessed += generatedPackedAnimationRight(finger, joint, AnimationClipPosition.negative, property);
                        animationProcessed += generatedPackedAnimationRight(finger, joint, AnimationClipPosition.positive, property);

                        for (int i = 0; i < STEP_COUNT; i++)
                        {
                            generatePackedAnimationLeft(finger, joint, i, property);
                            animationProcessed++;
                        }
                    }

                    ProgressDisplay.updateAnimationProgress(animationProcessed, PACKED_ANIMATION_COUNT);
                }
            }
        }
    }


}
