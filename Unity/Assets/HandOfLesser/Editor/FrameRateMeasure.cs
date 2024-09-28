using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEditor.Animations;
using UnityEngine;
using VRC.SDK3.Avatars.Components;

namespace HOL
{
    class FrameRateMeasure
    {
        // Initially this was 0 to 1000 so we would be getting frametimes in ms,
        // but due to the wonky nature of things, the result must be multiplied by 0.666...
        // to get the actual frametime. Probably because 1/3rds of the time the animation
        // gets evaluated for 2 frames ( or both states at the same time ? ) so the values gets inflated.
        private static float FPS_START_VALUE = 0;
        private static float FPS_END_VALUE = 666.666f;


        ///////////////////////////////////////////////////
        /// State machine measurement stuff
        ////////////////////////////////////////////////


        private static AnimatorState generateDummyState(AnimatorControllerLayer layer, string name)
        {
            AnimatorState state = layer.stateMachine.AddState(name);
            state.writeDefaultValues = true; // Must be true or values are multiplied depending on umber of blendtrees in controller!?!?!

            state.motion = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(
                    HOL.Resources.getAnimationClipName(PropertyType.fps)));

            return state;
        }

        public static AnimatorStateTransition generateTransition(AnimatorState from, AnimatorState to, AnimatorStateMachine stateMachine, bool exitImmediately = false)
        {
            AnimatorStateTransition transition = from.AddTransition(to);

            // While we could just go full speed, nothing really updates fast enough to warrant that.
            // VRChat recommends 20ms duration for paramter driver to be executed, so do this for now.
            if (exitImmediately)
            {
                // When not setting any params it should be immediate
                transition.hasExitTime = false;
                transition.hasFixedDuration = true;
                transition.exitTime = 0;
                transition.duration = 0.0000000001f; // If duration is 0 the animation never plays
                transition.canTransitionToSelf = false;
            }

            transition.AddCondition(AnimatorConditionMode.Greater, 0, HOL.Resources.ALWAYS_1_PARAMETER);

            return transition;
        }


        public static void populateFpsMeasureLayer(AnimatorController controller)
        {
            // since it's only a single parameter generate them separately
            generateFramerateAnimation();
            generateSmoothingAnimations();

            AnimatorControllerLayer layer = ControllerLayer.fpsMeasure.findLayer(controller);

            // This layer will be responsible for taking the raw osc values
            // and updating either the left or right hand values.
            // VRchat has some issues with rapidly swapping back and forth 
            // between the same two states, so we go:
            // Wait for left -> update left -> wait for right -> update right -> wait for left ...

            // Holding zone while waiting for hand side param to change
            var rightToLeftState = generateDummyState(layer, "rightToLeft");
            var lefToRightState = generateDummyState(layer, "leftToRight");

            // State that will house the driver setting our params
            var leftState = generateDummyState(layer, "right");
            var rightState = generateDummyState(layer, "right");

            // The two dummy states just sit and wait until the hand side param changes
            // Once it does it enters the corresponding state, it does its thing, and 
            // then proceeds to the opposite dummy state, which repeats the process.

            // From the left/right hand states we don't need any conditions,
            // they'll exit immediately.
            generateTransition(rightToLeftState, leftState, layer.stateMachine, true);
            generateTransition(lefToRightState, rightState, layer.stateMachine, true);

            generateTransition(leftState, lefToRightState, layer.stateMachine, true);
            generateTransition(rightState, rightToLeftState, layer.stateMachine, true);
        }

        public static void addParameters(AnimatorController controller)
        {
            controller.AddParameter(new AnimatorControllerParameter()
            {
                name = HOL.Resources.FPS_SMOOTHING_PARAMETER,
                type = AnimatorControllerParameterType.Float,
                defaultFloat = 0.99f
            });

            controller.AddParameter( HOL.Resources.getParameterName(PropertyType.fps), AnimatorControllerParameterType.Float);
            controller.AddParameter(HOL.Resources.getParameterName(PropertyType.fps_smooth), AnimatorControllerParameterType.Float);
        }

        private static void generateSingleAnimation(PropertyType propertyType)
        {
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getParameterName(propertyType),
                    0.0f, FPS_START_VALUE,
                    1f, FPS_END_VALUE
                );

            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(propertyType));
        }

        private static int generateFramerateAnimation()
        {
            generateSingleAnimation(PropertyType.fps);
            generateSingleAnimation(PropertyType.fps_smooth);

            return 1;
        }

        ///////////////////////////////////////////////////
        /// Smoothing layer stuff
        ////////////////////////////////////////////////

        private static float getValueForFpsPosition(AnimationClipPosition position)
        {
            switch(position)
            {
                case AnimationClipPosition.negative: return FPS_START_VALUE;
                case AnimationClipPosition.neutral: return -1; // Unused
                case AnimationClipPosition.positive: return FPS_END_VALUE;
            }

            return -1;
        }

        public static int generateSmoothingAnimation(PropertyType outputProperty, AnimationClipPosition position)
        {
            // This just sets the proxy parameter to whatever position, and we blend between these to do the smoothing
            // Note that these should always go from -1 to 1, which will not necessarily be the case of the normal finger animations
            // Right now they all just use the AnimationClipPosition though
            AnimationClip clip = new AnimationClip();
            ClipTools.setClipProperty(
                ref clip,
                    HOL.Resources.getParameterName(outputProperty),
                    getValueForFpsPosition(position) // See definition
                );

            // SHOULD_BE_ONE_BUT_ISNT is used because when you have this animation output 1, it outputs 40 instead.
            // I have no idea why, but account for the scaling ahead of time and it's fine. fucking unity.

            ClipTools.saveClip(clip, HOL.Resources.getAnimationOutputPath(outputProperty, position));

            return 1;
        }

        public static BlendTree generatSmoothingBlendtreeInner(BlendTree parent, PropertyType propertType)
        {
            // generats blendtrees 1 and 2 for generateSmoothingBlendtree()
            BlendTree tree = new BlendTree();

            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.name = HOL.Resources.getParameterName( propertType);
            tree.blendType = BlendTreeType.Simple1D;
            tree.useAutomaticThresholds = false;    
            tree.hideFlags = HideFlags.HideInHierarchy;


            // Blend either by original param or proxy param
            tree.blendParameter = HOL.Resources.getParameterName( propertType);

            // Property type is reused here, first .normal denoting the blendtree driven by the original value,
            // and .proxy the one driven by the proxy.
            // Both drive the animations setting the proxy
            AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(PropertyType.fps_smooth, AnimationClipPosition.negative));
            AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
                HOL.Resources.getAnimationOutputPath(PropertyType.fps_smooth, AnimationClipPosition.positive));

            tree.AddChild(negativeAnimation, FPS_START_VALUE);
            tree.AddChild(positiveAnimation, FPS_END_VALUE);

            return tree;
        }


        public static int generateSmoothingBlendtree(BlendTree parent, List<ChildMotion> childTrees)
        {
            // So we have 3 blend trees.
            // 1. Takes the original input value and blends between animations setting the proxy value to -1 / 1
            // 2. Takes the proxy value and does the same
            // 3. Blends between 1 and 2 according to FPS_SMOOTHING_PARAMETER. There'll be a local one too eventually.

            // #3
            BlendTree tree = new BlendTree();
            AssetDatabase.AddObjectToAsset(tree, parent);
            tree.blendType = BlendTreeType.Simple1D;
            tree.name = HOL.Resources.getParameterName(PropertyType.fps);
            tree.useAutomaticThresholds = false;    // Automatic probably would work fine
            tree.blendParameter = HOL.Resources.FPS_SMOOTHING_PARAMETER;
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
            tree.AddChild(generatSmoothingBlendtreeInner(tree, PropertyType.fps), 0);
            tree.AddChild(generatSmoothingBlendtreeInner(tree, PropertyType.fps_smooth), 1);

            return 1;
        }

        public static void populateFpsSmoothingLayer(AnimatorController controller)
        {
            AnimatorControllerLayer layer = ControllerLayer.fpsSmoothing.findLayer(controller);

            // State within this controller. TODO: attach to stuff
            AnimatorState rootState = layer.stateMachine.AddState("HOLFpsSmoothing");
            rootState.writeDefaultValues = true; // Must be true or values are multiplied depending on umber of blendtrees in controller!?!?!

            // Blendtree at the root of our state
            BlendTree rootBlendtree = new BlendTree();
            AssetDatabase.AddObjectToAsset(rootBlendtree, rootState);

            rootBlendtree.name = "fpsSmoothing";
            rootBlendtree.blendType = BlendTreeType.Direct;
            rootBlendtree.useAutomaticThresholds = false;
            rootBlendtree.blendParameter = HOL.Resources.ALWAYS_1_PARAMETER;

            rootState.motion = rootBlendtree;

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            List<ChildMotion> childTrees = new List<ChildMotion>();

            generateSmoothingBlendtree(rootBlendtree, childTrees);

            // Cannot add directly to parent tree, see generateSmoothingBlendtree()
            // Have to be added like this in order to set directblendparameter
            rootBlendtree.children = childTrees.ToArray();

            AssetDatabase.SaveAssets();

            ProgressDisplay.clearProgress();
        }

        public static void generateSmoothingAnimations()
        {
            HOL.Resources.createOutputDirectories();
            // While these drive the proxy parameter used for smoothing that drives the above
            generateSmoothingAnimation(PropertyType.fps, AnimationClipPosition.negative);
            generateSmoothingAnimation(PropertyType.fps, AnimationClipPosition.positive);

            generateSmoothingAnimation(PropertyType.fps_smooth, AnimationClipPosition.negative);
            generateSmoothingAnimation(PropertyType.fps_smooth, AnimationClipPosition.positive);



        }









    }
}
