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
    class FingerBend
    {
        public static readonly int HUMANOID_BLENDTREE_COUNT = AnimationValues.TOTAL_JOINT_COUNT; // Bend joints ( including splay ) * fingers * hands
        public static readonly int HUMANOID_ANIMATION_COUNT = HUMANOID_BLENDTREE_COUNT * 2; // Positive and negative animation for each

        public static int generateBendAnimation(HandSide side, FingerType finger, FingerBendType joint, AnimationClipPosition position)
        {
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.avatarRig),
                    AnimationValues.getValueForPose(position)
                ); ;

            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.avatarRig, position)));

            return 1;
        }

        public static int generateCurlBlendTree(BlendTree rootTree, List<ChildMotion> childTrees, HandSide side, FingerType finger, FingerBendType joint)
        {
            BlendTree tree = new BlendTree();
            tree.blendType = BlendTreeType.Simple1D;
            tree.name = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.avatarRig);
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.blendParameter = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.smooth); // Note that we're using the smoothed proxy here

            // In order to see the DirectBlendParamter required for the parent Direct blendtree, we need to use a ChildMotion,.
            // However, you cannot add a ChildMotion to a blendtree, and modifying it after adding it has no effect.
            // For whatever reason, adding them to a list assigning that as an array directly to BlendTree.Children works.
            childTrees.Add(new ChildMotion()
            {
                directBlendParameter = HOL.Resources.ALWAYS_1_PARAMETER,
                motion = tree,
                timeScale = 1,
            });

            AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.avatarRig, AnimationClipPosition.negative)));
            AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, PropertyType.avatarRig, AnimationClipPosition.positive)));

            tree.AddChild(negativeAnimation, -1);
            tree.AddChild(positiveAnimation, 1);

            return 1;
        }

        public static void populateFingerJointLayer(AnimatorController controller, AnimatorControllerLayer layer)
        {
            // This mostly works like your average joint state machine
            // Accept -1 to 1, blend between corresponding negative and positive animation.
            // Splay will be a bit special as it needs to rotate on a different axis depending on the curl of the joint.
            // Problem for future me. I guess the splay blendtree can just be separate?

            // State within this controller. TODO: attach to stuff
            AnimatorState rootState = layer.stateMachine.AddState("HandOfLesserHands");
            rootState.writeDefaultValues = false; // I Think?

            // Blendtree at the root of our state
            BlendTree rootBlendtree = new BlendTree();
            rootBlendtree.blendType = BlendTreeType.Simple1D;
            rootBlendtree.name = "HandRoot";
            rootBlendtree.useAutomaticThresholds = false;
            rootBlendtree.blendParameter = HOL.Resources.ALWAYS_1_PARAMETER;

            rootState.motion = rootBlendtree;

            // Generate the blendtrees for our joints
            FingerType[] fingers = (FingerType[])Enum.GetValues(typeof(FingerType));
            FingerBendType[] joints = (FingerBendType[])Enum.GetValues(typeof(FingerBendType));
            HandSide[] sides = (HandSide[])Enum.GetValues(typeof(HandSide));

            int blendtreesProcessed = 0;        // Cannot add directly to parent tree, see generateSmoothingBlendtree()

            ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, HUMANOID_BLENDTREE_COUNT);

            List<ChildMotion> childTrees = new List<ChildMotion>();
            foreach (HandSide side in sides)
            {
                foreach (FingerType finger in fingers)
                {
                    foreach (FingerBendType joint in joints)
                    {
                        blendtreesProcessed += generateCurlBlendTree(rootBlendtree, childTrees, side, finger, joint);
                        ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, HUMANOID_BLENDTREE_COUNT);
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
            ProgressDisplay.updateAnimationProgress(animationProcessed, HUMANOID_ANIMATION_COUNT);

            foreach (HandSide side in new HandSide().Values())
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        // These animatiosn drive the humanoid rig ( for now )
                        animationProcessed += generateBendAnimation(side, finger, joint, AnimationClipPosition.negative);
                        animationProcessed += generateBendAnimation(side, finger, joint, AnimationClipPosition.positive);

                        ProgressDisplay.updateAnimationProgress(animationProcessed, HUMANOID_ANIMATION_COUNT);
                    }
                }
            }

            // supposedly don't actually need these
            //AssetDatabase.SaveAssets();
            //AssetDatabase.Refresh();

            ProgressDisplay.clearProgress();
        }

    }
}
