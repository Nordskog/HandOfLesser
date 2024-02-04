using HOL;
using System;
using System.Collections.Generic;
using UnityEditor;
using UnityEditor.Animations;
using UnityEngine;
using VRC.SDK3.Avatars.Components;
using VRC.SDK3.Avatars.ScriptableObjects;

public class HandOfLesserAnimationGenerator : EditorWindow
{
    static TransmitType sTransmitType = TransmitType.alternating;

    [MenuItem("Window/HandOfLesser")]
    public static void ShowWindow()
    {
        GetWindow<HandOfLesserAnimationGenerator>("Hand of Lesser");
    }

    private void OnGUI()
    {
        if (GUILayout.Button("Generate Animations"))
        {
            generateAnimations(sTransmitType);
        }

        if (GUILayout.Button("Generate Controller"))
        {
            generateAnimationController(sTransmitType);
        }

        if (GUILayout.Button("Generate Parameters"))
        {
            generateAvatarParameters(sTransmitType);
        }
    }

    private static void addAnimatorLayer(AnimatorController controller, string name, float weight, AvatarMask mask)
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

    private static void generateControllerLayers(AnimatorController controller, TransmitType transmitType)
    {
        AvatarMask bothHandsMask = HOL.Resources.getBothHandsMask();

        // We cannot modify the weight or mask of any layers added to the controller, 
        // so we need to remove the base layer, and manually create it and our finger joint layer
        controller.RemoveLayer(0);

        addAnimatorLayer(controller, "Base Layer", 1, bothHandsMask);
        switch (transmitType)
        {
            case TransmitType.alternating:
                addAnimatorLayer(controller, "hand_side_machine", 1, bothHandsMask);
                break;
            case TransmitType.packed:
                addAnimatorLayer(controller, "joint_unpacker", 1, bothHandsMask);
                break;
            default:
                break;
        }
        addAnimatorLayer(controller, "joint_smoothing_machine", 1, bothHandsMask);
        addAnimatorLayer(controller, "finger_joints", 1, bothHandsMask);
    }

    private static void populateControllerLayers(AnimatorController controller, TransmitType transmitType)
    {
        switch (transmitType)
        {
            case TransmitType.alternating:
                Alternating.populateHandSwitchLayer(controller, controller.layers[1]);
                break;
            case TransmitType.packed:   // TODO generate unpacking layer
                Alternating.populateHandSwitchLayer(controller, controller.layers[1]);
                break;
            default:
                break;
        }
        Smoothing.populateSmoothingLayer(controller, controller.layers[2]);
        FingerBend.populateFingerJointLayer(controller, controller.layers[3]);
    }

    private void generateControllerParameters(AnimatorController controller, TransmitType transmitType)
    {
        // Add all the parameters we'll be working with, they need to be present here to be driven

        if (transmitType == TransmitType.alternating)
        {
            // Only if generating controller for alternating update
            controller.AddParameter(AnimationValues.HAND_SIDE_OSC_PARAMETER_NAME, AnimatorControllerParameterType.Int); // left/right switch
        }

        controller.AddParameter(new AnimatorControllerParameter()
        {
            name = AnimationValues.REMOTE_SMOOTHING_PARAMETER_NAME,
            type = AnimatorControllerParameterType.Float,
            defaultFloat = 0.7f  // whatever we set the smoothing to. Can we even edit via code afterwards? 
        });

        controller.AddParameter(new AnimatorControllerParameter()
        {
            name = AnimationValues.ALWAYS_1_PARAMETER,
            type = AnimatorControllerParameterType.Float,
            defaultFloat = 1
        });

        foreach (FingerType finger in new FingerType().Values())
        {
            foreach (FingerBendType joint in new FingerBendType().Values())
            {
                // Input OSC
                controller.AddParameter(HOL.Resources.getJointOSCName(finger, joint), AnimatorControllerParameterType.Float);

                // Output for left and right hand
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.left, finger, joint, PropertyType.normal), AnimatorControllerParameterType.Float);
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.right, finger, joint, PropertyType.normal), AnimatorControllerParameterType.Float);

                // Smoothened proxy values
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.left, finger, joint, PropertyType.proxy), AnimatorControllerParameterType.Float);
                controller.AddParameter(HOL.Resources.getJointParameterName(HandSide.right, finger, joint, PropertyType.proxy), AnimatorControllerParameterType.Float);
            }
        }
    }

    private void generateAnimationController(TransmitType transmitType)
    {
        HOL.Resources.createOutputDirectories();

        // New animation controller
        AnimatorController controller = AnimatorController.CreateAnimatorControllerAtPath(HOL.Resources.getAnimationControllerOutputPath());

        generateControllerParameters(controller, transmitType);
        generateControllerLayers(controller, transmitType);
        populateControllerLayers(controller, transmitType);

        AssetDatabase.SaveAssets();
    }

    private void generateAnimations(TransmitType transmitType)
    {
        // https://forum.unity.com/threads/createasset-is-super-slow.291667/#post-3853330
        // Makes createAsset not be super slow. Note the call to StopAssetEditing() below.
        AssetDatabase.StartAssetEditing(); // Yes, Start goes first.

        try
        {
            FingerBend.generateAnimations();
            Smoothing.generateAnimations();
            switch (transmitType)
            {
                case TransmitType.packed:
                    {
                        // Generate packed
                        break;
                    }
                default:
                    break;
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
    }


        private void generateAvatarParameters(TransmitType transmitType)
        {
            VRCExpressionParameters paramAsset = VRCExpressionParameters.CreateInstance<VRCExpressionParameters>();

            int parameterCount = AnimationValues.TOTAL_JOINT_COUNT;
            switch (transmitType)
            {
                case TransmitType.alternating:
                    parameterCount = AnimationValues.TOTAL_JOINT_COUNT / 2; // only send one hand at the time
                    parameterCount += 1; // For side param
                    break;
                case TransmitType.packed:
                    parameterCount = AnimationValues.TOTAL_JOINT_COUNT / 2; // only send one hand at the time
                    break;
                default:
                    break;
            }

            // Is null, populated with defaults on save if none provided
            paramAsset.parameters = new VRCExpressionParameters.Parameter[parameterCount];

            int paramCounter = 0;
            foreach (FingerType finger in new FingerType().Values())
            {
                foreach (FingerBendType joint in new FingerBendType().Values())
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

            if (transmitType == TransmitType.alternating)
            {
                // Hand side
                VRCExpressionParameters.Parameter newParam = new VRCExpressionParameters.Parameter();
                newParam.name = AnimationValues.HAND_SIDE_OSC_PARAMETER_NAME;
                newParam.valueType = VRCExpressionParameters.ValueType.Bool;
                newParam.defaultValue = 0;

                paramAsset.parameters[paramCounter] = newParam;
            }

            AssetDatabase.CreateAsset(paramAsset, HOL.Resources.getParametersPath());
            AssetDatabase.SaveAssets();
        }
    }
