using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor.Animations;
using VRC.SDK3.Avatars.Components;

namespace HOL
{
    class Alternating
    {
        private static AnimatorState generateDummyState(AnimatorControllerLayer layer, string name)
        {
            AnimatorState state = layer.stateMachine.AddState(name);
            state.writeDefaultValues = true; // Must be true or values are multiplied depending on umber of blendtrees in controller!?!?!

            return state;
        }

    public static AnimatorState generateSingleHandState(AnimatorControllerLayer layer, HandSide side)
        {
            AnimatorState state = layer.stateMachine.AddState(side.propertyName());
            state.writeDefaultValues = true; // Must be true or values are multiplied depending on umber of blendtrees in controller!?!?!

            // Driver to copy raw osc params to hand
            VRCAvatarParameterDriver driver = state.AddStateMachineBehaviour<VRCAvatarParameterDriver>();

            FingerType[] fingers = (FingerType[])Enum.GetValues(typeof(FingerType));
            FingerBendType[] joints = (FingerBendType[])Enum.GetValues(typeof(FingerBendType));

            foreach (FingerType finger in fingers)
            {
                foreach (FingerBendType joint in joints)
                {
                    var parameter = new VRC.SDKBase.VRC_AvatarParameterDriver.Parameter();
                    driver.parameters.Add(parameter);
                    //parameter.source = HOL.Resources.getJointOSCName(finger, joint)
                    parameter.type = VRC.SDKBase.VRC_AvatarParameterDriver.ChangeType.Copy;

                    // Name is the parameter it will drive, source is the paremter that will drive it.
                    // Amazing naming convention you idiots
                    parameter.name = HOL.Resources.getJointParameterName(side, finger, joint, PropertyType.input);
                    parameter.source = HOL.Resources.getJointParameterName(null, finger, joint, PropertyType.OSC_Alternating);
                }
            }


            return state;
        }

        public static AnimatorStateTransition generateTransition(AnimatorState to, AnimatorStateMachine stateMachine, bool exitImmediately = false)
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

        public static void populateHandSwitchLayer(AnimatorControllerLayer layer)
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
            var leftState = generateSingleHandState(layer, HandSide.left);
            var rightState = generateSingleHandState(layer, HandSide.right);

            // The two dummy states just sit and wait until the hand side param changes
            // Once it does it enters the corresponding state, it does its thing, and 
            // then proceeds to the opposite dummy state, which repeats the process.
            AnimatorStateTransition transition;
            {
                transition = generateTransition(leftState, layer.stateMachine);
                transition.AddCondition(AnimatorConditionMode.Equals, 0, HOL.Resources.HAND_SIDE_OSC_PARAMETER_NAME);
                rightToLeftState.AddTransition(transition);
            }
            {
                transition = generateTransition(rightState, layer.stateMachine);
                transition.AddCondition(AnimatorConditionMode.Equals, 1, HOL.Resources.HAND_SIDE_OSC_PARAMETER_NAME);
                lefToRightState.AddTransition(transition);
            }

            // From the left/right hand states we don't need any conditions,
            // they'll exit immediately.
            leftState.AddTransition(generateTransition(lefToRightState, layer.stateMachine, true));
            rightState.AddTransition(generateTransition(rightToLeftState, layer.stateMachine, true));
        }

    }
}
