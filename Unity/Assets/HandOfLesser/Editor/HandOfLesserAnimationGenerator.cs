using HOL;
using System;
using UnityEditor;
using UnityEditor.Animations;
using UnityEngine;
using VRC.SDK3.Avatars.ScriptableObjects;
using Joint = HOL.Joint;

public class HandOfLesserAnimationGenerator : EditorWindow
{
    public static readonly int STEP_COUNT = 16;
    public static readonly int BLENDTREE_COUNT = ((5 * 3) + 5); // 3 curls for each finger + splay ( shared between left right )
    public static readonly int ANIMATION_COUNT = BLENDTREE_COUNT * 256; // 256 animations for each blendtree

    [MenuItem("Window/HandOfLesser")]
    public static void ShowWindow()
    {
        GetWindow<HandOfLesserAnimationGenerator>("Hand of Lesser");
    }

    private void OnGUI()
    {
        if (GUILayout.Button("Generate Animations"))
        {
            generatePacked();
        }

        if (GUILayout.Button("Generate Controller"))
        {
            generateBlendtree();
        }

        if (GUILayout.Button("Generate Parameters"))
        {
            generateParameters();
        }
    }


    private float getValueForPose(Finger finger, Joint joint, JointMotionDirection direction, AnimationClipPosition clipPos)
    {
        // Make configurable? Simple -1 0 1 for now
        // Curl is -1 to 1, negative value curling.
        
        // Splay Negative moves outwards ( palm facing down )
        // -3 to +3 seems reasonable for splay, need to calibrate later.
        // The leap OSC animations use -1 to 1 though. Maybe animation preview just looks wonky?

        // Will be different for individual joints too

        switch( direction )
        {
            case JointMotionDirection.curl:
            {
                    switch (clipPos)
                    {
                        case AnimationClipPosition.negative: return -1;
                        case AnimationClipPosition.neutral: return 0;
                        case AnimationClipPosition.positive: return 1;
                    }

                    return 0;
            }

            case JointMotionDirection.splay:
            {
                switch (clipPos)
                {
                    case AnimationClipPosition.negative: return -3;
                    case AnimationClipPosition.neutral: return 0;
                    case AnimationClipPosition.positive: return 3;
                }

                return 0;
            }
        }

        return 0;
    }

    private float getValueAtStep(float step, float stepCount, float startVal, float endVal)
    {
        // Steps 0-15, or 16 steps. Subtract 1 from count ot make the math work.
        float ratio = step / (stepCount-1);
        return (ratio * endVal) + ((1f - ratio) * startVal);
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

    private void generatePackedAnimation(Finger finger, Joint? joint, int leftStep, int rightStep)
    {
        AnimationClip clip = new AnimationClip();
        setClipProperty(
            ref clip,
                HOL.Resources.getJointPropertyName(HandSide.left, finger, joint),
                getValueAtStep(leftStep, STEP_COUNT, -1, 1)
            );

        setClipProperty(
            ref clip,
                HOL.Resources.getJointPropertyName(HandSide.right, finger, joint),
                getValueAtStep(rightStep, STEP_COUNT, -1, 1)
            );

        saveClip(clip, HOL.Resources.getAnimationOutputPath(HOL.Resources.getPackedAnimationClipName(finger, joint, leftStep, rightStep)));
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

    private int generateAnimationsForSingleJoint( Finger finger, Joint? joint )
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
                    joint,  // Nullable
                    i,
                    j);
            }
        }

        // Return number of animations processed
        return STEP_COUNT * STEP_COUNT;
    }

    private void generatePacked()
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
                    animationProcessed += generateAnimationsForSingleJoint(finger, joint);
                    updateAnimationProgress(animationProcessed);
                }

                // And do the same for the splay, but only per finger
                animationProcessed +=  generateAnimationsForSingleJoint(finger, null);
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



    private int generateSingleBlendTree(AnimatorController controller, BlendTree rootTree, Finger finger, Joint? joint  )
    {
        controller.AddParameter(HOL.Resources.getJointOSCName(finger, joint), AnimatorControllerParameterType.Float);

        BlendTree tree = new BlendTree();
        tree.blendType = BlendTreeType.Simple1D;
        tree.name = HOL.Resources.getJointPropertyName(null, finger, joint);
        tree.useAutomaticThresholds = false;    // Automatic probably would work fine
        tree.blendParameter = HOL.Resources.getJointOSCName(finger, joint);

        rootTree.AddChild(tree);

        // Turns out you can't use integers in a blendtree, so -1 to 1 it is
        float threshold = -1;
        for (int i = 0; i < STEP_COUNT; i++)
        {
            for (int j = 0; j < STEP_COUNT; j++)
            {
                string animationPath = HOL.Resources.getAnimationOutputPath(HOL.Resources.getPackedAnimationClipName(finger, joint, i, j));
                AnimationClip animation = AssetDatabase.LoadAssetAtPath<AnimationClip>(animationPath);

                tree.AddChild(animation, threshold);
                threshold += ( 2.0f / 255f );
            }
        }

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

    private void generateBlendtree()
    {
        HOL.Resources.createOutputDirectories();

        // New animation controller
        AnimatorController controller = AnimatorController.CreateAnimatorControllerAtPath(HOL.Resources.getAnimationControllerOutputPath());

        AvatarMask bothHandsMask = HOL.Resources.getBothHandsMask();

        // We cannot modify the weight or mask of any layers added to the controller, 
        // so we need to remove the base layer, and manually create it and our finger joint layer
        controller.RemoveLayer(0);
        addAnimatorLayer(controller, "Base Layer", 1, bothHandsMask);
        addAnimatorLayer(controller, "finger_joints", 1, bothHandsMask);

        AnimatorControllerLayer layer = controller.layers[1];

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
                blendtreesProcessed += generateSingleBlendTree(controller,rootBlendtree, finger, joint);
                updateBlendtreeProgress(blendtreesProcessed);

              //  break;
            }

           // break;

            blendtreesProcessed += generateSingleBlendTree(controller, rootBlendtree, finger, null);
            updateBlendtreeProgress(blendtreesProcessed);
        }
        
        AssetDatabase.SaveAssets();

        clearProgress();
    }
    
    private void generateParameters()
    {
        VRCExpressionParameters paramAsset = VRCExpressionParameters.CreateInstance<VRCExpressionParameters>();

        // Is null, populated with defaults on save if none provided
        paramAsset.parameters = new VRCExpressionParameters.Parameter[BLENDTREE_COUNT]; 

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

            {
                // Splay
                VRCExpressionParameters.Parameter newParam = new VRCExpressionParameters.Parameter();
                newParam.name = HOL.Resources.getJointOSCName(finger, null);
                newParam.valueType = VRCExpressionParameters.ValueType.Float;
                newParam.defaultValue = 0;

                paramAsset.parameters[paramCounter] = newParam;
                paramCounter++;
            }
        }

        AssetDatabase.CreateAsset(paramAsset, HOL.Resources.getParametersPath());
        AssetDatabase.SaveAssets();
    }
}
