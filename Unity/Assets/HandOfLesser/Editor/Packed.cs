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
        public static readonly int STEP_COUNT = 16; // 4 bits worth
        public static readonly int PACKED_BLENDTREE_COUNT = AnimationValues.TOTAL_JOINT_COUNT / 2; // all joints but only for one hand
        public static readonly int PACKED_ANIMATION_COUNT = PACKED_BLENDTREE_COUNT * 256; // 256 animations for each left/right pair of joints

        private static int generateSingleBlendTree(BlendTree rootTree, List<ChildMotion> childTrees, FingerType finger, FingerBendType joint)
        {
            BlendTree tree = new BlendTree();
            tree.blendType = BlendTreeType.Simple1D;
            tree.name = HOL.Resources.getJointParameterName(null, finger, joint, PropertyType.OSC_Packed);
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.blendParameter = HOL.Resources.getJointParameterName(null, finger, joint, PropertyType.OSC_Packed);

            childTrees.Add(new ChildMotion()
            {
                directBlendParameter = HOL.Resources.ALWAYS_1_PARAMETER,
                motion = tree,
                timeScale = 1,
            });

            // Turns out you can't use integers in a blendtree, so -1 to 1 it is
            float threshold = -1;
            for (int i = 0; i < STEP_COUNT; i++)
            {
                for (int j = 0; j < STEP_COUNT; j++)
                {
                    string animationPath = HOL.Resources.getAnimationOutputPath(HOL.Resources.getPackedAnimationClipName(finger, joint, i, j));
                    AnimationClip animation = AssetDatabase.LoadAssetAtPath<AnimationClip>(animationPath);

                    tree.AddChild(animation, threshold);
                    threshold += (2.0f / 255f);
                }
            }

            return 1;
        }

        public static void populatedPackedLayer(AnimatorController controller, AnimatorControllerLayer layer)
        {
            // State within this controller. TODO: attach to stuff
            AnimatorState rootState = layer.stateMachine.AddState("HOLPacked");
            rootState.writeDefaultValues = false; // I Think?

            // Blendtree at the root of our state
            BlendTree rootBlendtree = new BlendTree();
            rootBlendtree.blendType = BlendTreeType.Direct;
            rootBlendtree.name = "HandRoot";
            rootBlendtree.useAutomaticThresholds = false;
            rootBlendtree.blendParameter = HOL.Resources.ALWAYS_1_PARAMETER;
            rootState.motion = rootBlendtree;

            int blendtreesProcessed = 0;
            ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, PACKED_BLENDTREE_COUNT);

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            List<ChildMotion> childTrees = new List<ChildMotion>();

            foreach (FingerType finger in new FingerType().Values())
            {
                foreach (FingerBendType joint in new FingerBendType().Values())
                {
                    blendtreesProcessed += generateSingleBlendTree(rootBlendtree, childTrees, finger, joint);
                    ProgressDisplay.updateBlendtreeProgress(blendtreesProcessed, PACKED_BLENDTREE_COUNT);
                }
            }

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            // Have to be added like this in order to set directblendparameter
            rootBlendtree.children = childTrees.ToArray();
        }

        private static float getValueAtStep(float step, float stepCount, float startVal, float endVal)
        {
            // Steps 0-15, or 16 steps. Subtract 1 from count to make the math work.
            float ratio = step / (stepCount - 1);
            return (ratio * endVal) + ((1f - ratio) * startVal);
        }

        private static void generatePackedAnimation(FingerType finger, FingerBendType joint, int leftStep, int rightStep)
        {
            // Animations driving HOL/Input params
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getJointParameterName(HandSide.left, finger, joint, PropertyType.input),
                    getValueAtStep(leftStep, STEP_COUNT, -1, 1)
                );

            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getJointParameterName(HandSide.right, finger, joint, PropertyType.input),
                    getValueAtStep(rightStep, STEP_COUNT, -1, 1)
                );

            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getPackedAnimationClipName(finger, joint, leftStep, rightStep)));
        }
        


        private static int generateAnimationsForSingleJoint(FingerType finger, FingerBendType joint)
        {
            // For each set of joints ( left hand, right hand ),
            // Generate 256 unique animations that cycle through all possible combinations of curl states for those two joints.
            // For the first 16 animations, the left hand will be full negative ( -1 ), while the right joint cycles through 16 poses from -1 to 1.
            // This repeats for the next 16 poses, with the left hand joint moving 1/16th of the distance between -1 and 1 for each set.

            for (int i = 0; i < STEP_COUNT; i++)
            {
                for (int j = 0; j < STEP_COUNT; j++)
                {
                    generatePackedAnimation(
                        finger,
                        joint, 
                        i,
                        j);
                }
            }

            // Return number of animations processed
            return STEP_COUNT * STEP_COUNT;
        }

        public static void generateAnimations()
        {
            HOL.Resources.createOutputDirectories();

            // https://forum.unity.com/threads/createasset-is-super-slow.291667/#post-3853330
            // Makes createAsset not be super slow. Note the call to StopAssetEditing() below.
            AssetDatabase.StartAssetEditing(); // Yes, Start goes first.

            int animationProcessed = 0;
            ProgressDisplay.updateAnimationProgress(animationProcessed, PACKED_ANIMATION_COUNT);

            try
            {
                foreach (FingerType finger in new FingerType().Values())
                {
                    foreach (FingerBendType joint in new FingerBendType().Values())
                    {
                        // For each joint in each finger, generate 256 packed curl animations
                        animationProcessed += generateAnimationsForSingleJoint(finger, joint);
                        ProgressDisplay.updateAnimationProgress(animationProcessed, PACKED_ANIMATION_COUNT);
                    }
                }

            }
            catch (Exception ex)
            {
                Debug.LogException(ex);
            }
            finally
            {
                AssetDatabase.StopAssetEditing();
            }


            // supposedly don't actually need these
            //AssetDatabase.SaveAssets();
            //AssetDatabase.Refresh();

            ProgressDisplay.clearProgress();
        }


    }


}
