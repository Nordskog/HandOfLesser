using HOL;
using System;
using System.Collections.Generic;
using UnityEditor;
using UnityEditor.Animations;
using UnityEngine;
using VRC.SDK3.Avatars.Components;
using VRC.SDK3.Avatars.ScriptableObjects;
using Joint = HOL.Joint;

public class HandOfLesserAnimationGenerator : EditorWindow
{
    public static readonly int BLENDTREE_COUNT = (5 * 4) * 2; // 3 curls, 1 splay, 5 fingers, 2 hands.
    public static readonly int ANIMATION_COUNT = BLENDTREE_COUNT * 2; // Open and close. Will be more once we get fancy with bones.
    public static readonly string HAND_SIDE_OSC_PARAMETER_NAME = "hand_side";
    public static readonly string REMOTE_SMOOTHING_PARAMETER_NAME = "HOL_remote_smoothing";

    public static readonly string ALWAYS_1_PARAMETER = "HOL_always_one";

    // There are some animations that should set parameters to -1 or 1.
    // Instead they set them to -40 or 10. I have no idea why.
    // Until someone can explain this to me, we multiply the values by this ( 1 / 40 )
    public static readonly float SHOULD_BE_ONE_BUT_ISNT= 0.025f;

    [MenuItem("Window/HandOfLesser")]
    public static void ShowWindow()
    {
        GetWindow<HandOfLesserAnimationGenerator>("Hand of Lesser");
    }

    private void OnGUI()
    {
        if (GUILayout.Button("Generate Animations"))
        {
            generateAnimations();
        }

        if (GUILayout.Button("Generate Controller"))
        {
            generateAnimationController();
        }

        if (GUILayout.Button("Generate Parameters"))
        {
            generateParameters();
        }
    }


    private float getValueForPose(AnimationClipPosition clipPos)
    {
        // Make configurable? Simple -1 0 1 for now
        // Curl is -1 to 1, negative value curling.

        // Splay Negative moves outwards ( palm facing down )
        // -3 to +3 seems reasonable for splay, need to calibrate later.
        // The leap OSC animations use -1 to 1 though. Maybe animation preview just looks wonky?

        // Will be different for individual joints too

        switch (clipPos)
        {
            case AnimationClipPosition.negative: return -1;
            case AnimationClipPosition.neutral: return 0;
            case AnimationClipPosition.positive: return 1;
        }

        return 0;
    }

    private void setClipProperty(ref AnimationClip clip, string property, float value)
    {
        // Single keyframe
        AnimationUtility.SetEditorCurve(
                clip,
                EditorCurveBinding.FloatCurve(
                    string.Empty,
                    typeof(Animator),
                    property
                    ),
            AnimationCurve.Linear(0, value, 0, value));
    }

    private void saveClip(AnimationClip clip, string savePath)
    {
        AssetDatabase.CreateAsset(clip, savePath);
    }

    private void generateSmoothingAnimation(HandSide side, Finger finger, Joint joint, AnimationClipPosition position)
    {
        // This just sets the proxy parameter to whatever position, and we blend between these to do the smoothing
        // Note that these should always go from -1 to 1, which will not necessarily be the case of the normal finger animations
        // Right now they all just use the AnimationClipPosition though
        AnimationClip clip = new AnimationClip();
        setClipProperty(
            ref clip,
                HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.proxy),
                getValueForPose(position) * SHOULD_BE_ONE_BUT_ISNT // See definition
            );

        // SHOULD_BE_ONE_BUT_ISNT is used because when you have this animation output 1, it outputs 40 instead.
        // I have no idea why, but account for the scaling ahead of time and it's fine. fucking unity.

        saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, position, PropertyType.proxy)));
    }

    private void generateBendAnimation(HandSide side, Finger finger, Joint joint, AnimationClipPosition position)
    {
        AnimationClip clip = new AnimationClip();
        setClipProperty(
            ref clip,
                HOL.Resources.getJointPropertyName(side, finger, joint),
                getValueForPose( position )
            );

        saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, position, PropertyType.normal)));
    }

    private void updateAnimationProgress(int processed)
    {
        float progress = (float)processed / (float)ANIMATION_COUNT;
        EditorUtility.DisplayProgressBar("Creating Animations", "Animation " + (processed + 1) + " of " + ANIMATION_COUNT, progress);
    }

    private void updateBlendtreeProgress(int processed)
    {
        float progress = (float)processed / (float)BLENDTREE_COUNT;
        EditorUtility.DisplayProgressBar("Creating Controller", "Blendtree " + (processed + 1) + " of " + BLENDTREE_COUNT, progress);
    }

    private void clearProgress()
    {
        EditorUtility.ClearProgressBar();
    }

    private void generateAnimations()
    {
        HOL.Resources.createOutputDirectories();

        Finger[] fingers = (Finger[])Enum.GetValues(typeof(Finger));
        Joint[] joints = (Joint[])Enum.GetValues(typeof(Joint));
        HandSide[] sides = (HandSide[])Enum.GetValues(typeof(HandSide));

        // https://forum.unity.com/threads/createasset-is-super-slow.291667/#post-3853330
        // Makes createAsset not be super slow. Note the call to StopAssetEditing() below.
        AssetDatabase.StartAssetEditing(); // Yes, Start goes first.

        int animationProcessed = 0;
        updateAnimationProgress(animationProcessed);

        try
        {

            foreach (HandSide side in sides)
            {
                foreach (Finger finger in fingers)
                {
                    foreach (Joint joint in joints)
                    {
                        // These animatiosn drive the humanoid rig ( for now )
                        generateBendAnimation(side, finger, joint, AnimationClipPosition.negative);
                        generateBendAnimation(side, finger, joint, AnimationClipPosition.positive);

                        // While these drive the proxy parameter used for smoothing that drives the above
                        generateSmoothingAnimation(side, finger, joint, AnimationClipPosition.negative);
                        generateSmoothingAnimation(side, finger, joint, AnimationClipPosition.positive);

                        animationProcessed += 4;
                        updateAnimationProgress(animationProcessed);
                    }

                    animationProcessed += 6;
                    updateAnimationProgress(animationProcessed);
                }
            }
            
        }
        catch( Exception ex)
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

        clearProgress();
    }



    private int generateCurlBlendTree(BlendTree rootTree, List<ChildMotion> childTrees, HandSide side, Finger finger, Joint joint  )
    {
        BlendTree tree = new BlendTree();
        tree.blendType = BlendTreeType.Simple1D;
        tree.name = HOL.Resources.getJointPropertyName(side, finger, joint);
        tree.useAutomaticThresholds = false;    // Automatic probably would work fine
        tree.blendParameter = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.proxy); // Note that we're using the smoothed proxy here

        // In order to see the DirectBlendParamter required for the parent Direct blendtree, we need to use a ChildMotion,.
        // However, you cannot add a ChildMotion to a blendtree, and modifying it after adding it has no effect.
        // For whatever reason, adding them to a list assigning that as an array directly to BlendTree.Children works.
        childTrees.Add(new ChildMotion()
        {
            directBlendParameter = ALWAYS_1_PARAMETER,
            motion = tree,
            timeScale = 1,
        });

        AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
            HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, AnimationClipPosition.negative, PropertyType.normal)));
        AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
            HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, AnimationClipPosition.positive, PropertyType.normal)));

        tree.AddChild(negativeAnimation, -1);
        tree.AddChild(positiveAnimation, 1);

        return 1;
    }

    private BlendTree generatSmoothingBlendtreeInner( BlendTree rootTree, HandSide side, Finger finger, Joint joint, PropertyType propertType)
    {
        // generats blendtrees 1 and 2 for generateSmoothingBlendtree()
        BlendTree tree = new BlendTree();
        tree.blendType = BlendTreeType.Simple1D;
        tree.name = HOL.Resources.getJointParameterName(side, finger, joint, propertType);
        tree.useAutomaticThresholds = false;    // Automatic probably would work fine

        // Blend either by original param or proxy param
        tree.blendParameter = HOL.Resources.getJointParameterName(side, finger, joint, propertType);

        // Property type is reused here, first .normal denoting the blendtree driven by the original value,
        // and .proxy the one driven by the proxy.
        // Both drive the animations setting the proxy
        AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
            HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, AnimationClipPosition.negative, PropertyType.proxy)));
        AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
            HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, AnimationClipPosition.positive, PropertyType.proxy)));

        tree.AddChild(negativeAnimation, -1);
        tree.AddChild(positiveAnimation, 1);

        return tree;
    }

    private int generateSmoothingBlendtree(BlendTree rootTree, List<ChildMotion> childTrees, HandSide side, Finger finger, Joint joint)
    {
        // So we have 3 blend trees.
        // 1. Takes the original input value and blends between animations setting the proxy value to -1 / 1
        // 2. Takes the proxy value and does the same
        // 3. Blends between 1 and 2 according to REMOTE_SMOOTHING_PARAMETER_NAME. There'll be a local one too eventually.

        // #3
        BlendTree tree = new BlendTree();
        tree.blendType = BlendTreeType.Simple1D;
        tree.name = HOL.Resources.getJointPropertyName(side, finger, joint);
        tree.useAutomaticThresholds = false;    // Automatic probably would work fine
        tree.blendParameter = REMOTE_SMOOTHING_PARAMETER_NAME;

        // In order to see the DirectBlendParamter required for the parent Direct blendtree, we need to use a ChildMotion,.
        // However, you cannot add a ChildMotion to a blendtree, and modifying it after adding it has no effect.
        // For whatever reason, adding them to a list assigning that as an array directly to BlendTree.Children works.
        childTrees.Add(new ChildMotion()
        {
            directBlendParameter = ALWAYS_1_PARAMETER,
            motion = tree,
            timeScale = 1,
        });

        // Generate #1 and #2
        tree.AddChild(generatSmoothingBlendtreeInner(tree, side, finger, joint, PropertyType.normal), 0);
        tree.AddChild(generatSmoothingBlendtreeInner(tree, side, finger, joint, PropertyType.proxy), 1);

        return 1;
    }

    private void addAnimatorLayer(AnimatorController controller, string name, float weight, AvatarMask mask)
    {
        // https://forum.unity.com/threads/animator-controller-layer-and-default-weight.527167/
        // You cannot programtically modify a layer after it has been created, so in order to set a weight
        // and avatarMask, you must manually crate both the layer and its stateMachine, set your settings, then add it to the controller

        AnimatorStateMachine stateMachine = new AnimatorStateMachine
        {
            name = controller.MakeUniqueLayerName(name),
            hideFlags = HideFlags.HideInHierarchy
        };

        controller.AddLayer(new AnimatorControllerLayer
        {
            stateMachine = stateMachine,
            name = stateMachine.name,
            defaultWeight = weight,

            avatarMask = mask
        });

        // Statemachine isn't saved otherwise?
        AssetDatabase.AddObjectToAsset(stateMachine, controller);
    }

    private AnimatorState generateDummyState(AnimatorControllerLayer layer, string name)
    {
        AnimatorState state = layer.stateMachine.AddState(name);
        state.writeDefaultValues = false; // I Think?

        return state;
    }

    private AnimatorState generateSingleHandState(AnimatorController controller, AnimatorControllerLayer layer, HandSide side)
    {
        AnimatorState state = layer.stateMachine.AddState(side.propertyName());
        state.writeDefaultValues = false; // I Think?

        // Driver to copy raw osc params to hand
        VRCAvatarParameterDriver driver = state.AddStateMachineBehaviour<VRCAvatarParameterDriver>();

        Finger[] fingers = (Finger[])Enum.GetValues(typeof(Finger));
        Joint[] joints = (Joint[])Enum.GetValues(typeof(Joint));

        foreach (Finger finger in fingers)
        {
            foreach (Joint joint in joints)
            {
                var parameter = new VRC.SDKBase.VRC_AvatarParameterDriver.Parameter();
                driver.parameters.Add(parameter);
                //parameter.source = HOL.Resources.getJointOSCName(finger, joint)
                parameter.type = VRC.SDKBase.VRC_AvatarParameterDriver.ChangeType.Copy;

                // Name is the parameter it will drive, source is the paremter that will drive it.
                // Amazing naming convention you idiots
                parameter.name = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.normal);
                parameter.source = HOL.Resources.getJointOSCName(finger, joint);
            }
        }


        return state;
    }

    private AnimatorStateTransition generateTransition(AnimatorState to, AnimatorStateMachine stateMachine, bool exitImmediately = false)
    {
        AnimatorStateTransition transition = stateMachine.AddAnyStateTransition(to);

        // While we could just go full speed, nothing really updates fast enough to warrant that.
        // VRChat recommends 20ms duration for paramter driver to be executed, so do this for now.
        if (exitImmediately)
        {
            // When not setting any params it should be immediate
            transition.hasExitTime = true;
            transition.exitTime = 0;
            transition.duration = 0;
        }
        else
        {
            transition.hasExitTime = true;
            transition.exitTime = 0.02f;
            transition.duration = 0;
        }

        return transition;
    }
    
    private void populateHandSwitchLayer(AnimatorController controller, AnimatorControllerLayer layer)
    {
        // This layer will be responsible for taking the raw osc values
        // and updating either the left or right hand values.
        // VRchat has some issues with rapidly swapping back and forth 
        // between the same two states, so we go:
        // Wait for left -> update left -> wait for right -> update right -> wait for left ...

        // Holding zone while waiting for hand side param to change
        var rightToLeftState = generateDummyState(layer, "rightToLeft");
        var lefToRightState = generateDummyState(layer, "leftToRight");

        // State that will house the driver setting our params
        var leftState = generateSingleHandState(controller, layer, HandSide.left);
        var rightState = generateSingleHandState(controller, layer, HandSide.right);

        // The two dummy states just sit and wait until the hand side param changes
        // Once it does it enters the corresponding state, it does its thing, and 
        // then proceeds to the opposite dummy state, which repeats the process.
        AnimatorStateTransition transition;
        {
            transition = generateTransition(leftState, layer.stateMachine);
            transition.AddCondition(AnimatorConditionMode.Equals, 0, HAND_SIDE_OSC_PARAMETER_NAME);
            rightToLeftState.AddTransition(transition);
        }
        {
            transition = generateTransition(rightState, layer.stateMachine);
            transition.AddCondition(AnimatorConditionMode.Equals, 1, HAND_SIDE_OSC_PARAMETER_NAME);
            lefToRightState.AddTransition(transition);
        }

        // From the left/right hand states we don't need any conditions,
        // they'll exit immediately.
        leftState.AddTransition( generateTransition(lefToRightState, layer.stateMachine, true ));
        rightState.AddTransition(generateTransition(rightToLeftState, layer.stateMachine, true));
    }

    private void populateSmoothingLayer(AnimatorController controller, AnimatorControllerLayer layer)
    {
        // State within this controller. TODO: attach to stuff
        AnimatorState rootState = layer.stateMachine.AddState("HOLSmoothing");
        rootState.writeDefaultValues = false; // I Think?

        // Blendtree at the root of our state
        BlendTree rootBlendtree = new BlendTree();
        rootBlendtree.blendType = BlendTreeType.Direct;
        rootBlendtree.name = "HandRoot";
        rootBlendtree.useAutomaticThresholds = false;
        rootBlendtree.blendParameter = ALWAYS_1_PARAMETER;

        rootState.motion = rootBlendtree;

        // Generate the blendtrees for our joints
        Finger[] fingers = (Finger[])Enum.GetValues(typeof(Finger));
        Joint[] joints = (Joint[])Enum.GetValues(typeof(Joint));
        HandSide[] sides = (HandSide[])Enum.GetValues(typeof(HandSide));

        int blendtreesProcessed = 0;
        updateBlendtreeProgress(blendtreesProcessed);

        // Cannot add directly to parent tree, see generateSmoothingBlendtree()
        List<ChildMotion> childTrees = new List<ChildMotion>();
        foreach (HandSide side in sides)
        {
            foreach (Finger finger in fingers)
            {
                foreach (Joint joint in joints)
                {
                    blendtreesProcessed += generateSmoothingBlendtree(rootBlendtree, childTrees, side, finger, joint);
                    updateBlendtreeProgress(blendtreesProcessed);
                }
            }
        }

        // Cannot add directly to parent tree, see generateSmoothingBlendtree()
        // Have to be added like this in order to set directblendparameter
        rootBlendtree.children = childTrees.ToArray();

        AssetDatabase.SaveAssets();

        clearProgress();
    }

    private void generateAnimationController()
    {
        // This requires 2 separate layers.
        // The first will read the osc hand side value
        // and switch to the corresponding state. The state will
        // dump the bend values into either left or right params on entry.
        HOL.Resources.createOutputDirectories();

        // New animation controller
        AnimatorController controller = AnimatorController.CreateAnimatorControllerAtPath(HOL.Resources.getAnimationControllerOutputPath());

        Finger[] fingers = (Finger[])Enum.GetValues(typeof(Finger));
        Joint[] joints = (Joint[])Enum.GetValues(typeof(Joint));
        // Add all the parameters we'll be working with, they need to be present here to be driven
        controller.AddParameter(HAND_SIDE_OSC_PARAMETER_NAME, AnimatorControllerParameterType.Int); // left/right switch

        controller.AddParameter(new AnimatorControllerParameter()
        {
            name = REMOTE_SMOOTHING_PARAMETER_NAME,
            type = AnimatorControllerParameterType.Float,
            defaultFloat = 0.7f  // whatever we set the smoothing to. Can we even edit via code afterwards? 
        });

        controller.AddParameter(new AnimatorControllerParameter(){
            name = ALWAYS_1_PARAMETER,
            type = AnimatorControllerParameterType.Float,
            defaultFloat = 1
        });

        foreach (Finger finger in fingers)
        {
            foreach (Joint joint in joints)
            {
                // Input OSC
                controller.AddParameter( HOL.Resources.getJointOSCName(finger,joint), AnimatorControllerParameterType.Float);

                // Output for left and right hand
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.left, finger, joint, PropertyType.normal), AnimatorControllerParameterType.Float);
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.right, finger, joint, PropertyType.normal), AnimatorControllerParameterType.Float);

                // Smoothened proxy values
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.left, finger, joint, PropertyType.proxy), AnimatorControllerParameterType.Float);
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.right, finger, joint, PropertyType.proxy), AnimatorControllerParameterType.Float);
            }
        }

        AvatarMask bothHandsMask = HOL.Resources.getBothHandsMask();

        // We cannot modify the weight or mask of any layers added to the controller, 
        // so we need to remove the base layer, and manually create it and our finger joint layer
        controller.RemoveLayer(0);
        addAnimatorLayer(controller, "Base Layer", 1, bothHandsMask);
        addAnimatorLayer(controller, "hand_side_machine", 1, bothHandsMask);
        addAnimatorLayer(controller, "joint_smoothing_machine", 1, bothHandsMask);
        addAnimatorLayer(controller, "finger_joints", 1, bothHandsMask);

        populateHandSwitchLayer(controller, controller.layers[1]);

        populateSmoothingLayer(controller, controller.layers[2]);

        populateFingerJointLayer(controller, controller.layers[3]);

        AssetDatabase.SaveAssets();
    }

    private void populateFingerJointLayer(AnimatorController controller, AnimatorControllerLayer layer)
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
        rootBlendtree.blendParameter = ALWAYS_1_PARAMETER;

        rootState.motion = rootBlendtree;

        // Generate the blendtrees for our joints
        Finger[] fingers = (Finger[])Enum.GetValues(typeof(Finger));
        Joint[] joints = (Joint[])Enum.GetValues(typeof(Joint));
        HandSide[] sides = (HandSide[])Enum.GetValues(typeof(HandSide));

        int blendtreesProcessed = 0;
        updateBlendtreeProgress(blendtreesProcessed);

        // Cannot add directly to parent tree, see generateSmoothingBlendtree()
        List<ChildMotion> childTrees = new List<ChildMotion>();
        foreach (HandSide side in sides)
        {
            foreach (Finger finger in fingers)
            {
                foreach (Joint joint in joints)
                {
                    blendtreesProcessed += generateCurlBlendTree(rootBlendtree, childTrees, side, finger, joint);
                    updateBlendtreeProgress(blendtreesProcessed);
                }
            }
        }

        // Cannot add directly to parent tree, see generateSmoothingBlendtree()
        // Have to be added like this in order to set directblendparameter
        rootBlendtree.children = childTrees.ToArray();

        AssetDatabase.SaveAssets();

        clearProgress();
    }

    private void generateParameters()
    {
        VRCExpressionParameters paramAsset = VRCExpressionParameters.CreateInstance<VRCExpressionParameters>();

        // Is null, populated with defaults on save if none provided
        paramAsset.parameters = new VRCExpressionParameters.Parameter[BLENDTREE_COUNT / 2 + 1];

        // Generate the blendtrees for our joints   
        Finger[] fingers = (Finger[])Enum.GetValues(typeof(Finger));
        Joint[] joints = (Joint[])Enum.GetValues(typeof(Joint));

        int paramCounter = 0;
        foreach (Finger finger in fingers)
        {
            foreach (Joint joint in joints)
            {
                // Curl
                VRCExpressionParameters.Parameter newParam = new VRCExpressionParameters.Parameter();
                newParam.name = HOL.Resources.getJointOSCName(finger, joint);
                newParam.valueType = VRCExpressionParameters.ValueType.Float;
                newParam.defaultValue = 0;

                paramAsset.parameters[paramCounter] = newParam;
                paramCounter++;
            }
        }

        { 
            // Hand side
            VRCExpressionParameters.Parameter newParam = new VRCExpressionParameters.Parameter();
            newParam.name = HAND_SIDE_OSC_PARAMETER_NAME;
            newParam.valueType = VRCExpressionParameters.ValueType.Bool;
            newParam.defaultValue = 0;

            paramAsset.parameters[paramCounter] = newParam;
        }

        AssetDatabase.CreateAsset(paramAsset, HOL.Resources.getParametersPath());
        AssetDatabase.SaveAssets();
    }
}
