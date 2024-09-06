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
    class Smoothing
    {
        public static readonly int SMOOTHING_BLENDTREE_COUNT = AnimationValues.TOTAL_JOINT_COUNT; // Bend joints ( including splay ) * fingers * hands
        public static readonly int SMOOTHING_ANIMATION_COUNT = SMOOTHING_BLENDTREE_COUNT * 2; // Positive and negative animation for each

        public static int generateSmoothingAnimation(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition position)
        {
            // This just sets the proxy parameter to whatever position, and we blend between these to do the smoothing
            // Note that these should always go from -1 to 1, which will not necessarily be the case of the normal finger animations
            // Right now they all just use the AnimationClipPosition though
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.smooth),
                    AnimationValues.getValueForPose(position) // See definition
                );

            // SHOULD_BE_ONE_BUT_ISNT is used because when you have this animation output 1, it outputs 40 instead.
            // I have no idea why, but account for the scaling ahead of time and it's fine. fucking unity.

            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.smooth, position)));

            return 1;
        }

        public static BlendTree generatSmoothingBlendtreeInner( BlendTree parent, HandSide side, FingerType finger, FingerBendType joint, PropertyType propertType)
        {
            // generats blendtrees 1 and 2 for generateSmoothingBlendtree()
            BlendTree tree = new BlendTree();

            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.name = HOL.Resources.getJointParameterName(side, finger, joint, propertType);
            tree.blendType = BlendTreeType.Simple1D;
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.hideFlags = HideFlags.HideInHierarchy;


            // Blend either by original param or proxy param
            tree.blendParameter = HOL.Resources.getJointParameterName(side, finger, joint, propertType);

            // Property type is reused here, first .normal denoting the blendtree driven by the original value,
            // and .proxy the one driven by the proxy.
            // Both drive the animations setting the proxy
            AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.smooth, AnimationClipPosition.negative)));
            AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.smooth, AnimationClipPosition.positive)));

            tree.AddChild(negativeAnimation, -1);
            tree.AddChild(positiveAnimation, 1);

            return tree;
        }


        public static int generateSmoothingBlendtree(BlendTree parent, List<ChildMotion> childTrees, HandSide side, FingerType finger, FingerBendType joint)
        {
            // So we have 3 blend trees.
            // 1. Takes the original input value and blends between animations setting the proxy value to -1 / 1
            // 2. Takes the proxy value and does the same
            // 3. Blends between 1 and 2 according to REMOTE_SMOOTHING_PARAMETER_NAME. There'll be a local one too eventually.

            // #3
            BlendTree tree = new BlendTree();
            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.blendType = BlendTreeType.Simple1D;
            tree.name = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.input);
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.blendParameter = HOL.Resources.REMOTE_SMOOTHING_PARAMETER_NAME;
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

            // Generate #1 and #2
            tree.AddChild(generatSmoothingBlendtreeInner(tree, side, finger, joint, PropertyType.input), 0);
            tree.AddChild(generatSmoothingBlendtreeInner(tree, side, finger, joint, PropertyType.smooth), 1);

            return 1;
        }

        public static void populateSmoothingLayer(AnimatorControllerLayer layer)
        {
            // State within this controller. TODO: attach to stuff
            AnimatorState rootState = layer.stateMachine.AddState("HOLSmoothing");
            rootState.writeDefaultValues = true; // Must be true or values are multiplied depending on umber of blendtrees in controller!?!?!

            // Blendtree at the root of our state
            BlendTree rootBlendtree = new BlendTree();
            AssetDatabase.AddObjectToAsset(rootBlendtree, rootState);

            rootBlendtree.name = "HandRoot_smoothing";
            rootBlendtree.blendType = BlendTreeType.Direct;
            rootBlendtree.useAutomaticThresholds = false;
            rootBlendtree.blendParameter = HOL.Resources.ALWAYS_1_PARAMETER;

            rootState.motion = rootBlendtree;

            int blendtreesProcessed = 0;
            ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, SMOOTHING_BLENDTREE_COUNT);

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            List<ChildMotion> childTrees = new List<ChildMotion>();
            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        blendtreesProcessed += generateSmoothingBlendtree(rootBlendtree, childTrees, side, finger, joint);
                        ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, SMOOTHING_BLENDTREE_COUNT);
                    }
                }
            }
            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            // Have to be added like this in order to set directblendparameter
            rootBlendtree.children = childTrees.ToArray();

            AssetDatabase.SaveAssets();

            ProgressDisplay.clearProgress();
        }

        public static void generateAnimations()
        {
            HOL.Resources.createOutputDirectories();

            int animationProcessed = 0;
            ProgressDisplay.updateAnimationProgress(animationProcessed, SMOOTHING_ANIMATION_COUNT);

            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        // While these drive the proxy parameter used for smoothing that drives the above
                        animationProcessed += generateSmoothingAnimation(side, finger, joint, AnimationClipPosition.negative);
                        animationProcessed += generateSmoothingAnimation(side, finger, joint, AnimationClipPosition.positive);

                        ProgressDisplay.updateAnimationProgress(animationProcessed, SMOOTHING_ANIMATION_COUNT);
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
