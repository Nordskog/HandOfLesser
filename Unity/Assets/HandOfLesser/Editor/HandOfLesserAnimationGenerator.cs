using HOL;
using System;
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

    private void generateBendAnimation(HandSide side, Finger finger, Joint joint, AnimationClipPosition position)
    {
        AnimationClip clip = new AnimationClip();
        setClipProperty(
            ref clip,
                HOL.Resources.getJointPropertyName(side, finger, joint),
                getValueForPose( position )
            );

        saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, position)));
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

        // https://forum.unity.com/threads/createasset-is-super-slow.291667/#post-3853330
        // Makes createAsset not be super slow. Note the call to StopAssetEditing() below.
        AssetDatabase.StartAssetEditing(); // Yes, Start goes first.

        int animationProcessed = 0;
        updateAnimationProgress(animationProcessed);

        try
        {
            foreach (Finger finger in fingers)
            {
                foreach (Joint joint in joints)
                {
                    // For each joint in each finger, generate 256 packed curl animations
                    generateBendAnimation(HandSide.left, finger, joint, AnimationClipPosition.negative);
                    generateBendAnimation(HandSide.left, finger, joint, AnimationClipPosition.positive);

                    generateBendAnimation(HandSide.right, finger, joint, AnimationClipPosition.negative);
                    generateBendAnimation(HandSide.right, finger, joint, AnimationClipPosition.positive);

                    animationProcessed += 4;
                    updateAnimationProgress(animationProcessed);
                }

                // And do the same for the splay, but only per finger
                animationProcessed += 6;
                updateAnimationProgress(animationProcessed);
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



    private int generateCurlBlendTree(AnimatorController controller, BlendTree rootTree, HandSide side, Finger finger, Joint joint  )
    {
        controller.AddParameter(HOL.Resources.getJointOSCName(finger, joint), AnimatorControllerParameterType.Float);

        BlendTree tree = new BlendTree();
        tree.blendType = BlendTreeType.Simple1D;
        tree.name = HOL.Resources.getJointPropertyName(side, finger, joint);
        tree.useAutomaticThresholds = false;    // Automatic probably would work fine
        tree.blendParameter = HOL.Resources.getJointParameterName(side, finger, joint); // Note how we are not using the OSC params directly

        rootTree.AddChild(tree);

        AnimationClip negativeAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
            HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, AnimationClipPosition.negative)));
        AnimationClip positiveAnimation = AssetDatabase.LoadAssetAtPath<AnimationClip>(
            HOL.Resources.getAnimationOutputPath(HOL.Resources.getAnimationClipName(side, finger, joint, AnimationClipPosition.positive)));

        tree.AddChild(negativeAnimation, -1);
        tree.AddChild(positiveAnimation, 1);

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
                parameter.name = HOL.Resources.getJointParameterName(side, finger, joint);
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
        foreach (Finger finger in fingers)
        {
            foreach (Joint joint in joints)
            {
                // Input OSC
                controller.AddParameter( HOL.Resources.getJointOSCName(finger,joint), AnimatorControllerParameterType.Float);

                // Output for left and right hand
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.left, finger, joint), AnimatorControllerParameterType.Float);
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.right, finger, joint), AnimatorControllerParameterType.Float);
            }
        }

        AvatarMask bothHandsMask = HOL.Resources.getBothHandsMask();

        // We cannot modify the weight or mask of any layers added to the controller, 
        // so we need to remove the base layer, and manually create it and our finger joint layer
        controller.RemoveLayer(0);
        addAnimatorLayer(controller, "Base Layer", 1, bothHandsMask);
        addAnimatorLayer(controller, "hand_side_machine", 1, bothHandsMask);
        addAnimatorLayer(controller, "finger_joints", 1, bothHandsMask);

        populateHandSwitchLayer(controller, controller.layers[1]);

        populateFingerJointLayer(controller, controller.layers[2]);

        AssetDatabase.SaveAssets();
    }

    private void populateFingerJointLayer(AnimatorController controller, AnimatorControllerLayer layer)
    {
        // This mostly works like your average joint state machine
        // Accept -1 to 1, blend between corresponding negative and positive animation.
        // Splay will be a bit special as it needs to rotate on a different axis depending on the curl of the joint.
        // Problem for future me. I guess the splay blendtree can just be separate?

        // When we create the root blendtree, it will be populated with a dummy "Blend" parameter, which doesn't exist.
        // Simply creating this parameter will not work, so we need to create a new one and use that as the param for the root blend tree.
        // BlendTrees require floats, so everything needs to be floats.
        controller.AddParameter("dummy", AnimatorControllerParameterType.Float);

        // State within this controller. TODO: attach to stuff
        AnimatorState rootState = layer.stateMachine.AddState("HandOfLesserHands");
        rootState.writeDefaultValues = false; // I Think?

        // Blendtree at the root of our state
        BlendTree rootBlendtree = new BlendTree();
        rootBlendtree.blendType = BlendTreeType.Simple1D;
        rootBlendtree.name = "HandRoot";
        rootBlendtree.useAutomaticThresholds = false;
        rootBlendtree.blendParameter = "dummy";

        rootState.motion = rootBlendtree;

        // Generate the blendtrees for our joints
        Finger[] fingers = (Finger[])Enum.GetValues(typeof(Finger));
        Joint[] joints = (Joint[])Enum.GetValues(typeof(Joint));

        int blendtreesProcessed = 0;
        updateBlendtreeProgress(blendtreesProcessed);
        foreach (Finger finger in fingers)
        {
            foreach (Joint joint in joints)
            {
                blendtreesProcessed += generateCurlBlendTree(controller, rootBlendtree, HandSide.left, finger, joint);
                blendtreesProcessed += generateCurlBlendTree(controller, rootBlendtree, HandSide.right, finger, joint);
                updateBlendtreeProgress(blendtreesProcessed);
            }

            updateBlendtreeProgress(blendtreesProcessed);
        }

        AssetDatabase.SaveAssets();

        clearProgress();
    }

    private void generateParameters()
    {
        VRCExpressionParameters paramAsset = VRCExpressionParameters.CreateInstance<VRCExpressionParameters>();

        // Is null, populated with defaults on save if none provided
        paramAsset.parameters = new VRCExpressionParameters.Parameter[BLENDTREE_COUNT + 1];

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
