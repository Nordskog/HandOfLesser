# HandOfLesser: a Quest hand tracking driver for SteamVR

HandOfLesser is an experimental tool that uses OpenXR extensions to integrate OpenXR hand-tracking into SteamVR, and VRChat.

Every contribution, whether it's a line of code, documentation, or feedback, is highly appreciated.

The project is currently a work in progress, including these instructions that have not really been updated properly.

We have a discord: https://discord.gg/k9QNcvvJmF

Currently no build is provided, and the below instructions are intended for developers.   
There'll be a release with an installer eventually.

## Showcase

[2024-04-13 06-42-41.webm](https://github.com/Nordskog/HandOfLesser/assets/8961771/30bae3db-b8bf-4cad-a15d-c500190379ba)


Shows switching from controllers to hand-tracking.   
Currently we use 160bits, which results in a bit of jitter. This can be improved for very little cost.    
In the second part of the video the update rate is lowered to the same 10hz update rate ( 100ms intervals ) used by OSC to simulate what it looks like over the network.   

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
