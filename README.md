# HandOfLesser: a Quest hand tracking driver for SteamVR

HandOfLesser is an experimental tool that uses OpenXR extensions to integrate Quest hand tracking into SteamVR.

Every contribution, whether it's a line of code, documentation, or feedback, is highly appreciated.

The project is currently a work in progress.

## Prerequisites

Before diving in, ensure your Quest is primed to stream the hand tracking data:

1. **Enable Developer Mode** on your Quest.
2. Access the Oculus app and enable **Developer Runtime Features** under the Beta section.
3. Make sure the **Public Test Channel** is disabled.

## Setup & Installation

Register the driver with SteamVR after building the project:

```ps1
vrpathreg.exe adddriver ...\output\drivers\handoflesser
```

## Usage

To begin using HandOfLesser for hand tracking in SteamVR:

1. Use gestures only to avoid Quest controllers being picked up instead.
2. Connect your Quest to your PC. You can use either **AirLink** or a wired connection.
3. Start the `HandOfLesser.exe` tracking app.
4. Launch **SteamVR**.
