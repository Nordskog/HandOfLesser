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

        if (GUILayout.Button("60"))
        {
            sFpsLimit = 60;
            Application.targetFrameRate = sFpsLimit;
        }

        if (GUILayout.Button("120"))
        {
            sFpsLimit = 120;
            Application.targetFrameRate = sFpsLimit;
        }

        if (GUILayout.Button("240"))
        {
            sFpsLimit = 240;
            Application.targetFrameRate = sFpsLimit;
        }
    }

}
