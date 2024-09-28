using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

public class FpsLimitWindow : EditorWindow
{

    static int sFpsLimit = 60;

    [MenuItem("Window/FpsLimit")]
    public static void ShowWindow()
    {
        GetWindow<FpsLimitWindow>("Fps Limit");
    }

    private void OnGUI()
    {
        sFpsLimit = EditorGUILayout.IntField("FpsLimit:", sFpsLimit);

        if (GUILayout.Button("Apply"))
        {
            Application.targetFrameRate = sFpsLimit;
        }
    }

}
