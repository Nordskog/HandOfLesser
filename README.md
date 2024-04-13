# HandOfLesser: a Quest hand tracking driver for SteamVR

HandOfLesser is an experimental tool that uses OpenXR extensions to integrate OpenXR hand-tracking into SteamVR, and VRChat.

Every contribution, whether it's a line of code, documentation, or feedback, is highly appreciated.

The project is currently a work in progress, including these instructions that have not really been updated properly.

We have a discord: https://discord.gg/k9QNcvvJmF

Currently no build is provided, and the below instructions are intended for developers.
There'll be a release with an installer eventually.

## Prerequisites

`Forward tracking data` must be enabled if using Virtual Desktop.

## Build instructions

Clone recursively and build with cmake. You may want to use Visual Studio 2022.

## Setup & Installation

Register the driver with SteamVR after building the project:

```ps1
vrpathreg.exe adddriver ...\output\drivers\handoflesser
```

Copy the Unity/Assets folder to your Avatar's unity project.
Under Windows you will find a `Hand Of Lesser` window. 
Hit each of the buttons in succession and assign the generated `Controller` and `Parameters` to your gesture layer.

## Usage

If you are using Virtual Desktop, you can use the exe without the driver, but will be unable to offset your hands.
Using Quest Link the driver is required for you to be able to switch between controllers and hands.

To begin using HandOfLesser for hand tracking in SteamVR:

1. Connect your Quest to your PC. You can use either Quest Link or Virtual Desktop.
2. Ensure the active OpenXR runtime is set to either Oculus or VDXR, depending on which you are using.
3. Launch **SteamVR**.
4. Start the `HandOfLesser.exe` tracking app.

You WILL need to tweak all the values, primarily the Curl and Splay Centers.