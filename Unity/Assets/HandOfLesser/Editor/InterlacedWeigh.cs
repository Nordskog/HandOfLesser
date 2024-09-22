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
    class InterlacedWeigh
    {
        public static readonly int WEIGH_BLENDTREE_COUNT = AnimationValues.TOTAL_JOINT_COUNT; // Bend joints ( including splay ) * fingers * hands
        public static readonly int WEIGH_ANIMATION_COUNT = WEIGH_BLENDTREE_COUNT * 2; // Positive and negative animation for each, for prev and next.

        public static int generatedInterlacedWeightAnimation(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition position, PropertyType property)
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
        public static BlendTree generateFlipflopBlendtreeInner( BlendTree parent, HandSide side, FingerType finger, FingerBendType joint, PropertyType drivingProperty, bool invert)
        {
            BlendTree tree = new BlendTree();

            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.name = HOL.Resources.getJointParameterName(side, finger, joint, drivingProperty);
            tree.blendType = BlendTreeType.Simple1D;
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.hideFlags = HideFlags.HideInHierarchy;


            tree.blendParameter = HOL.Resources.getJointParameterName(side, finger, joint, drivingProperty);

            AnimationClipPosition firstPosition = AnimationClipPosition.negative;
            AnimationClipPosition secondPosition = AnimationClipPosition.positive;

            // Invert second one driving the same value so the cancel eachother out, and giving us their distance
            if (invert)
            {
                firstPosition = AnimationClipPosition.positive;
                secondPosition = AnimationClipPosition.negative;
            }

            AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.interlaced_weight, firstPosition)));
            AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.interlaced_weight, secondPosition)));

            tree.AddChild(negativeAnimation, -1);
            tree.AddChild(positiveAnimation, 1);

            return tree;
        }


        public static int generateWeighBlendtree(BlendTree parent, List<ChildMotion> childTrees, HandSide side, FingerType finger, FingerBendType joint)
        {
            // So we have 3 blend trees.
            // 1. Just blends the two child blendtrees 50/50
            // 2. blend by interlace_first and write weight -1 to 1
            // 3. blend by interlace_second and write weight 1 to -1 ( inverted !) 

            // This results in us getting the difference between interlace_first and interlace_second.
            // Later we will use this to use the average of first and second if close, or only one ( the latest ) if distant.

            // #3
            BlendTree tree = new BlendTree();
            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.blendType = BlendTreeType.Simple1D;
            tree.name = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.input);
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.blendParameter = HOL.Resources.ALWAYS_HALF_PARAMETER; // Always 50/50 blend
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

            // #1 writes target to target, #2 writes input to target. Blend is 0 to 1
            tree.AddChild(generateFlipflopBlendtreeInner(tree, side, finger, joint, PropertyType.input_interlaced_first, false), 0);
            tree.AddChild(generateFlipflopBlendtreeInner(tree, side, finger, joint, PropertyType.input_interlaced_second, true), 1);

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
                        controller.AddParameter(HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.interlaced_weight), AnimatorControllerParameterType.Float);
                    }
                }
            }
        }

        // Blend between prev/next interlaced values driving the same value between -1/1 and 1/-1.
        // They will cancel out and gives us the distance of the two values, which we can use to weight prev/next later.
        public static void populateWeighLayer(AnimatorController controller)
        {
            AnimatorControllerLayer layer = ControllerLayer.interlaceWeigh.findLayer(controller);

            // State within this controller. TODO: attach to stuff
            AnimatorState rootState = layer.stateMachine.AddState("HOLWeigh");
            rootState.writeDefaultValues = true; // Must be true or values are multiplied depending on umber of blendtrees in controller!?!?!

            // Blendtree at the root of our state
            BlendTree rootBlendtree = new BlendTree();
            AssetDatabase.AddObjectToAsset(rootBlendtree, rootState);

            rootBlendtree.name = "Weigh";
            rootBlendtree.blendType = BlendTreeType.Direct;
            rootBlendtree.useAutomaticThresholds = false;
            rootBlendtree.blendParameter = HOL.Resources.ALWAYS_1_PARAMETER;

            rootState.motion = rootBlendtree;

            int blendtreesProcessed = 0;
            ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, WEIGH_BLENDTREE_COUNT);

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            List<ChildMotion> childTrees = new List<ChildMotion>();
            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        blendtreesProcessed += generateWeighBlendtree(rootBlendtree, childTrees, side, finger, joint);
                        ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, WEIGH_BLENDTREE_COUNT);

                    }
                }
            }
            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            // Have to be added like this in order to set directblendparameter
            rootBlendtree.children = childTrees.ToArray();

            AssetDatabase.SaveAssets();

            ProgressDisplay.clearProgress();
        }

        // Animations that the weight property for each joint to -1 or 1 
        public static void generateAnimations()
        {
            HOL.Resources.createOutputDirectories();

            int animationProcessed = 0;
            ProgressDisplay.updateAnimationProgress(animationProcessed, WEIGH_ANIMATION_COUNT);

            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        animationProcessed += generatedInterlacedWeightAnimation(side, finger, joint, AnimationClipPosition.negative, PropertyType.interlaced_weight);
                        animationProcessed += generatedInterlacedWeightAnimation(side, finger, joint, AnimationClipPosition.positive, PropertyType.interlaced_weight);

                        ProgressDisplay.updateAnimationProgress(animationProcessed, WEIGH_ANIMATION_COUNT);
                    }
                }
            }
        }
    }

}
