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

        // Single tree that uses drivingProperty to blend between outputProperty negative and postiive
        public static BlendTree generateFlipflopBlendtreeInner( BlendTree parent, HandSide side, FingerType finger, FingerBendType joint, PropertyType drivingProperty, PropertyType outputProperty)
        {
            BlendTree tree = new BlendTree();

            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.name = HOL.Resources.getJointParameterName(side, finger, joint, outputProperty);
            tree.blendType = BlendTreeType.Simple1D;
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.hideFlags = HideFlags.HideInHierarchy;


            tree.blendParameter = HOL.Resources.getJointParameterName(side, finger, joint, drivingProperty);

            AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, outputProperty, AnimationClipPosition.negative)));
            AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, outputProperty, AnimationClipPosition.positive)));

            tree.AddChild(negativeAnimation, -1);
            tree.AddChild(positiveAnimation, 1);

            return tree;
        }


        public static int generateFlipFlopBlendTree(BlendTree parent, List<ChildMotion> childTrees, HandSide side, FingerType finger, FingerBendType joint, PropertyType targetProperty)
        {
            // So we have 3 blend trees.
            // 1. Uses itself (first or second ) to blend animations setting itself, effectively doing nothing.
            // 2. Uses interlaced input to blend animations setting itself, copying input to self.
            // 3. Blends between 1 and 2 according to FLIPFLOP param.

            // 0 will blend with self ( do nothing ) 1 will store new value

            // #3
            BlendTree tree = new BlendTree();
            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.blendType = BlendTreeType.Simple1D;
            tree.name = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.input);
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.blendParameter = HOL.Resources.INTERLACE_BIT_OSC_PARAMETER_NAME;
            tree.hideFlags = HideFlags.HideInHierarchy;

            // In order to see the DirectBlendParamter required for the parent Direct blendtree, we need to use a ChildMotion,.
            // However, you cannot add a ChildMotion to a blendtree, and modifying it after adding it has no effect.
            // For whatever reason, adding them to a list assigning that as an array directly to BlendTree.Children works.
            childTrees.Add(new ChildMotion()
            {
                directBlendParameter = HOL.Resources.ALWAYS_1_PARAMETER,
                motion = tree,
                timeScale = 1,
            });

            float startValue = 0;
            float endValue = 1;

            // when 0 write to first, when 1 write to second
            if (targetProperty==PropertyType.input_interlaced_first)
            {
                startValue = 1;
                endValue = 0;
            }

            // #1 writes target to target, #2 writes input to target. Blend is 0 to 1
            tree.AddChild(generateFlipflopBlendtreeInner(tree, side, finger, joint, targetProperty, targetProperty), startValue);
            tree.AddChild(generateFlipflopBlendtreeInner(tree, side, finger, joint, PropertyType.input_interlaced, targetProperty), endValue);

            return 1;
        }

        public static void addParameters(AnimatorController controller)
        {
            controller.AddParameter(HOL.Resources.INTERLACE_BIT_OSC_PARAMETER_NAME, AnimatorControllerParameterType.Float);

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
                    }
                }
            }
        }

        // A tree driven by the flipflop param. interlaced_first and interlaced_second will be set to either
        // interlaced_input or self depending on the flipflop value, so that one will contain the previous value,
        // and the other the current value.
        public static void populateFlipflopLayer(AnimatorController controller)
        {
            AnimatorControllerLayer layer = ControllerLayer.interlacePopulate.findLayer(controller);

            // State within this controller. TODO: attach to stuff
            AnimatorState rootState = layer.stateMachine.AddState("HOLFlipFlop");
            rootState.writeDefaultValues = true;

            // Blendtree at the root of our state
            BlendTree rootBlendtree = new BlendTree();
            AssetDatabase.AddObjectToAsset(rootBlendtree, rootState);

            rootBlendtree.name = "Flipflop";
            rootBlendtree.blendType = BlendTreeType.Direct;
            rootBlendtree.useAutomaticThresholds = false;
            rootBlendtree.blendParameter = HOL.Resources.ALWAYS_1_PARAMETER;

            rootState.motion = rootBlendtree;

            int blendtreesProcessed = 0;
            ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, FLIPFLOP_BLENDTREE_COUNT);

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            List<ChildMotion> childTrees = new List<ChildMotion>();
            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        blendtreesProcessed += generateFlipFlopBlendTree(rootBlendtree, childTrees, side, finger, joint, PropertyType.input_interlaced_first);
                        blendtreesProcessed += generateFlipFlopBlendTree(rootBlendtree, childTrees, side, finger, joint, PropertyType.input_interlaced_second);
                        ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, FLIPFLOP_BLENDTREE_COUNT);

                    }
                }
            }
            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            // Have to be added like this in order to set directblendparameter
            rootBlendtree.children = childTrees.ToArray();

            AssetDatabase.SaveAssets();

            ProgressDisplay.clearProgress();
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

            // supposedly don't actually need these
            //AssetDatabase.SaveAssets();
            //AssetDatabase.Refresh();

            // clearProgress();
        }
    }

}
