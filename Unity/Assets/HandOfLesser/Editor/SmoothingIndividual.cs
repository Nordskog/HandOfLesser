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
    class SmoothingIndividual
    {
        public static readonly int SMOOTHING_INDIVIDUAL_BLENDTREE_COUNT = AnimationValues.TOTAL_JOINT_COUNT; // Bend joints ( including splay ) * fingers * hands
        public static readonly int SMOOTHING_INDIVIDUAL_ANIMATION_COUNT = SMOOTHING_INDIVIDUAL_BLENDTREE_COUNT * 2;

        // Just sets input, because we don't normally generate a 1-to-1 input animation.
        public static int generateIndivdiualSmoothingAnimation(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition position)
        {
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.smoothing_individual),
                    AnimationValues.getValueForPose(position) // See definition
                );

            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.smoothing_individual, position)));

            return 1;
        }

        public static void addParameters(AnimatorController controller)
        {
            // All the params used for interlace weight
            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        // left and right hand joints
                        controller.AddParameter(HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.smoothing_individual), AnimatorControllerParameterType.Float);
                    }
                }
            }
        }


        // Single tree that uses drivingProperty to blend between outputProperty negative and postiive
        public static BlendTree generateValueDriverBlendtree( BlendTree parent, HandSide side, FingerType finger, FingerBendType joint, PropertyType drivingProperty, PropertyType outputProperty)
        {
            BlendTree tree = new BlendTree();

            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.name = HOL.Resources.getParameterName(outputProperty);
            tree.blendType = BlendTreeType.Simple1D;
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.hideFlags = HideFlags.HideInHierarchy;


            tree.blendParameter = HOL.Resources.getParameterName(drivingProperty);

            AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, outputProperty, AnimationClipPosition.negative)));
            AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, outputProperty, AnimationClipPosition.positive)));

            tree.AddChild(negativeAnimation, -1);
            tree.AddChild(positiveAnimation, 1);

            return tree;
        }

        public static int generateFlipFlopBlendTree(BlendTree parent, List<ChildMotion> childTrees, HandSide side, FingerType finger, FingerBendType joint)
        {
            BlendTree tree = new BlendTree();
            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.blendType = BlendTreeType.Simple1D;
            tree.name = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.smoothing_weight);
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.blendParameter = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.interlaced_weight);
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

            // If weight is near 0, use average, otherwise use latest value.
            tree.AddChild(generateValueDriverBlendtree(tree, side, finger, joint, PropertyType.smoothing_adjusted, PropertyType.smoothing_individual), -0.1f);
            tree.AddChild(generateValueDriverBlendtree(tree, side, finger, joint, PropertyType.smoothing_adjusted_max, PropertyType.smoothing_individual), -0.06667f);
            tree.AddChild(generateValueDriverBlendtree(tree, side, finger, joint, PropertyType.smoothing_adjusted_max, PropertyType.smoothing_individual), 0.066667f);
            tree.AddChild(generateValueDriverBlendtree(tree, side, finger, joint, PropertyType.smoothing_adjusted, PropertyType.smoothing_individual), 0.1f);

            return 1;
        }

        // Blend between average of first/second, and the latest value, so the greater the distance the more it will move towards the latest.
        public static void populateSmoothingIndividualLayer(AnimatorController controller)
        {
            addParameters(controller);
            AnimatorControllerLayer layer = ControllerLayer.smoothingIndividual.findLayer(controller);

            AnimatorState rootState = layer.stateMachine.AddState("HOLSmoothingIndividual");
            rootState.writeDefaultValues = true; // Must be true or values are multiplied depending on umber of blendtrees in controller!?!?!

            // Blendtree at the root of our state
            BlendTree rootBlendtree = new BlendTree();
            AssetDatabase.AddObjectToAsset(rootBlendtree, rootState);

            rootBlendtree.name = "SmoothingIndividual";
            rootBlendtree.blendType = BlendTreeType.Direct;
            rootBlendtree.useAutomaticThresholds = false;
            rootBlendtree.blendParameter = HOL.Resources.ALWAYS_1_PARAMETER;

            rootState.motion = rootBlendtree;

            int blendtreesProcessed = 0;
            ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, SMOOTHING_INDIVIDUAL_BLENDTREE_COUNT);

            List<ChildMotion> childTrees = new List<ChildMotion>();
            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        blendtreesProcessed += generateFlipFlopBlendTree(rootBlendtree, childTrees, side, finger, joint);
                        ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, SMOOTHING_INDIVIDUAL_BLENDTREE_COUNT);
                    }
                }
            }
            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            // Have to be added like this in order to set directblendparameter
            rootBlendtree.children = childTrees.ToArray();

            AssetDatabase.SaveAssets();

            ProgressDisplay.clearProgress();
        }

        // We don't normally need animations to drive input, because we either set them directly ( full )
        // or use fancy animations to set them very oddly ( packed ). When using interlaced we do need them tho.
        public static void generateAnimations()
        {
            HOL.Resources.createOutputDirectories();

            int animationProcessed = 0;
            ProgressDisplay.updateAnimationProgress(animationProcessed, SMOOTHING_INDIVIDUAL_ANIMATION_COUNT);

            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        animationProcessed += generateIndivdiualSmoothingAnimation(side, finger, joint, AnimationClipPosition.negative);
                        animationProcessed += generateIndivdiualSmoothingAnimation(side, finger, joint, AnimationClipPosition.positive);

                        ProgressDisplay.updateAnimationProgress(animationProcessed, SMOOTHING_INDIVIDUAL_ANIMATION_COUNT);
                    }
                }
            }
        }
    }

}
