<div align="center">
  <img src="images/Doom-Logo-1993.png" width="300" alt="doom-logo"/>
</div>

# ACAP DOOM

<table border="2" cellpadding="10" cellspacing="0" width="100%">
  <tr>
    <td align="center">
      <strong>⚠️ IMPORTANT ⚠️</strong><br/>
      This application is <strong>not affiliated</strong> with id Software LLC or Axis Communications AB.<br/>
      <strong>Please read and respect the LICENSE</strong> to ensure compliance.<br/>
      <strong>UNOFFICIAL APP</strong><br/>
      Requires "Allow unsigned apps" to be enabled on the device.
    </td>
  </tr>
</table>

<table border="2" cellpadding="10" cellspacing="0" width="100%">
  <tr>
    <td align="center">
      <strong>⚠️ IMPORTANT ⚠️</strong><br/>
      <strong>This application is not actively maintained.</strong><br/>
      It may not work on all devices or firmware versions.<br/>
      Requires AXIS OS firmware version <strong>11.11.220</strong> or later, and is validated up to AXIS OS <strong>13</strong>.
    </td>
  </tr>
</table>

## What is this?

A port of the classic **DOOM** engine originally developed by id Software LLC, adapted for Axis network cameras.

Runs on devices based on **ARTPEC-7**, **ARTPEC-8**, or newer ARTPEC SoCs. \
Refer to your device manual to determine your platform.

This application is intended to be built on a Linux or macOS system.

## Build Docker images

**[Docker](https://docs.docker.com/engine/install/) and Make must be installed before proceeding.**

```sh
make dockersetup
```

This may take some time, as all required dependencies are built during this step.

## Build the ACAP package

**⚠️ NOTE** that you will need a WAD file to play and install the ACAP application. \
Please read the [**WAD File**](#wad-file) section below.

For ARTPEC-7 devices:

```sh
make armv7hf
```

For ARTPEC-8 devices:

```sh
make aarch64
```

This may take some time, as both the application binary and the web UI are built.

## Install the ACAP application

⚠️ You will need to enable unsigned ACAP packages on the Apps page: **Allow unsigned apps**

Run ```setuptarget.sh``` to create ```credentials.json``` (requires `jq` to be installed) \
Set the device IP and credentials in ```credentials.json``` and:

```sh
source setuptarget.sh && make install
```

Otherwise, go to the device web interface at:

```html
http://<YOUR_DEVICE_IP>/camera/index.html#/apps
```

and select ```Add app``` \
Upload the correct ```.eap``` file to the device to install it.

## Run the ACAP application

Open the app website and press the **START** button.

## WAD file

WAD (which, according to the Doom Bible, is an acronym for "Where's All the Data?") \
is the file format used by Doom and all Doom-engine-based games for storing data.

The WAD file used in ACAP Doom is the shareware version of Doom (`doom1.wad`, version 1.9, SHA-256 `1d7d43be501e67d927e415e0b8f3e29c3bf33075e859721816f652a526cac771`).

`doom1.wad` will automatically be downloaded when running ```make dockersetup``` \
When building the ACAP package, it will be copied to the project directory.

<table border="2" cellpadding="10" cellspacing="0" width="100%">
  <tr>
    <td align="center">
      <strong>⚠️ IMPORTANT ⚠️</strong><br/>
      The user is responsible for ensuring they have the legal right
      to use any WAD file deployed with this application.<br/>
      WAD SHOULD BE PLACED IN THE SAME DIRECTORY AS THIS README FILE
    </td>
  </tr>
</table>

The ACAP package is assembled from the project metadata in [`manifest.json`](manifest.json) and the `acap-build`
command in the Docker packaging script, which attaches `doom1.wad`, [`dropin_service.service`](dropin_service.service),
`libwebsockets`, and `sndserver` to the final app bundle. \

More about `doom1.wad` here: <https://doomwiki.org/wiki/DOOM1.WAD>

## Expert users: Add your own WAD file

Place your WAD file in the following directory on the device:

```sh
/usr/local/packages/acap_doom
```

or, on AXIS OS 13.1 and newer:

```sh
/opt/apps/acap_doom
```

**Note:** Not all WAD files have been tested.

## Expert users: Development

Check available build commands and helpers:

```sh
make help
```

To just build the application binary for current arch:

```sh
# Will build with debug symbols:
FINAL=n make build

# Deploy debug binary to target:
FINAL=n make deploy
```

Build and deploy the web code (requires Node.js and Yarn to be installed):

```sh
make web

# Deploy the ACAP web to target:
make deployweb
```

Trace logs on current sourced target device (requires Python with paramiko and scp):

```sh
make log
```

## FAQ

### Q: What is "ACAP"?

**A:** AXIS Camera Application Platform: An open application platform for software-based solutions built around Axis devices.
More info [here](https://www.axis.com/developer-community/open-source/acap).

### Q: Why can't I control the game?

**A:** Controls only work within the ACAP Doom webpage.

### Q: Why are the controls slow or unresponsive?

**A:** There are a few steps you can take to improve the responsiveness of your game input:

1. **Try turning off other running ACAP applications.**
2. **Reload the video stream or the application UI.**
3. **Lower the stream resolution.**
4. **Ensure you are on the same network as your device for optimal performance.**

### Q: Why does it not work on my device?

**A:** Only tested on a limited set of ARTPEC-7 and ARTPEC-8 devices.

### Q: Which browsers are supported?

**A:** Tested on latest stable versions of Chrome and Firefox. Other browsers are unverified.

### Q: Why does it not install?

**A:** You will need either [sign the ACAP package](https://www.axis.com/support/acap-signing) or enable unsigned ACAP
packages *(no longer an option with AXIS OS 13 unless using a developer mode device)* on the Apps page:
**`Allow unsigned apps`**
Also ensure that the application matches the device architecture.

### Q: Why am I not hearing any sounds?

**A:** You need to plug speakers or headphones to the audio output jack of your device.

### Q: Why is there no music?

**A:** Music support has not been implemented yet.

### Q: Does it only work on devices with an ARTPEC chip?

**A:** Yes, it currently only works on devices based on ARTPEC-7 or newer ARTPEC SoCs.

### Q: What is the purpose of this?

**A:** This project is primarily a technical experiment demonstrating what is possible with the ACAP SDK.

## Screenshots

<img src="images/screenshot1.png" width="800" alt="screenshot1"/>
<img src="images/screenshot2.png" width="800" alt="screenshot2"/>

## Credits

<table border="2" cellpadding="10" cellspacing="0" width="100%">
  <tr>
    <td align="center">
      <strong>⚠️ IMPORTANT ⚠️</strong><br/>
      <strong>All assets, libraries, and tools are the properties of their respective owners.</strong><br/>
      DOOM is a registered trademark of id Software LLC.<br/>
    </td>
  </tr>
</table>

- id Software DOOM: <https://github.com/id-Software/DOOM>
- Doom wallpaper: <https://www.wallpaperflare.com/doom-wallpaper-doom-2016-video-games-doom-game-crowd-wallpaper-pbidr>
- Doom logo: <https://logos.fandom.com/wiki/Doom_(1993_video_game)>
- Doom font: <https://www.dafont.com/doom.font>
- Doom cursors: <http://www.rw-designer.com/cursor-set/doom-1>
- Libwebsockets: <https://libwebsockets.org/>
- PipeWire: <https://pipewire.org/>
- React: <https://react.dev/>
- Material UI: <https://mui.com/>
