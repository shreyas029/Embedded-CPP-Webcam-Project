# Embedded-CPP-Webcam-Project

## Webcam Capture and Grayscale JPEG Conversion using GStreamer (C++)

## Environment
- Ubuntu 64-bit guest in Oracle VirtualBox 7.1.4
- HP Wide Vision HD Camera passed through from the host via **Devices → Webcams**
- GStreamer 1.28.2 (observed during testing)
- Video4Linux2 (`v4l2src`) camera input

## Progress and testing
1. Identified the host camera using Windows Device Manager (USB vendor/product ID `30C9:000E`).
2. Enabled VirtualBox USB support and installed the VirtualBox Extension Pack.
3. Attached the webcam through **Devices → Webcams → HP Wide Vision HD Camera**.
4. Confirmed `/dev/video0` and `/dev/video1` appeared in Ubuntu.
5. Verified GStreamer and the `v4l2src` plugin. Initial `gst-inspect-1.0 v4l2src` checks failed, but subsequent inspection succeeded.
6. Tested camera capabilities with `v4l2-ctl`; the webcam supports MJPEG formats including 640×480.
7. Successfully displayed a live webcam preview using a GStreamer pipeline with MJPEG decoding.
8. Compiled the C++ application successfully. The latest run returned `Cannot identify device '/dev/video0'`, so end-to-end PNG/JPEG generation is **not yet verified**. Reconnect the webcam and recheck its device path before retrying.

## Install dependencies
```bash
sudo apt update
sudo apt install -y build-essential pkg-config libgstreamer1.0-dev \
  gstreamer1.0-tools gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good gstreamer1.0-plugins-bad v4l-utils
```

## Check webcam access
In the running VirtualBox VM, select **Devices → Webcams → HP Wide Vision HD Camera**

```bash
ls -l /dev/video*
v4l2-ctl --list-devices
v4l2-ctl -d /dev/video0 --list-formats-ext
```

If the camera is attached but `/dev/video0` is not usable, disconnect/reconnect it from VirtualBox and rerun these checks. `/dev/video1` may be a non-capture device; verify capabilities rather than assuming it is a webcam.

## Compile
Place `webcam_capture.cpp` in your working directory, then run:

```bash
g++ webcam_capture.cpp -o webcam_capture $(pkg-config --cflags --libs gstreamer-1.0)
```

## Run
```bash
./webcam_capture
```

When successful, the program writes two files in the current directory:
- `captured.png` — colour frame captured from the webcam
- `grayscale.jpg` — grayscale JPEG converted from the PNG

View them with:
```bash
xdg-open captured.png
xdg-open grayscale.jpg
```

## How the application works
**Capture pipeline:**
```text
v4l2src device=/dev/video0 num-buffers=1 do-timestamp=true !
image/jpeg,width=640,height=480,framerate=15/1 !
jpegdec ! videoconvert ! video/x-raw,format=RGB !
pngenc ! filesink location=captured.png
```

**Grayscale conversion pipeline:**
```text
filesrc location=captured.png ! pngdec ! videoconvert !
video/x-raw,format=GRAY8 ! jpegenc !
filesink location=grayscale.jpg
```

The C++ code uses `gst_parse_launch()` to create each pipeline, waits for end-of-stream or error on the GStreamer bus, and releases pipeline resources.

## Troubleshooting
- **`Cannot identify device '/dev/video0'`**: Check VirtualBox webcam passthrough, `ls /dev/video*`, and `v4l2-ctl --list-devices`. The device may be disconnected or renumbered.
- **Missing `v4l2src`**: Check `gst-inspect-1.0 v4l2src` and install `gstreamer1.0-plugins-good` if necessary.

## Project files
```text
README.md
webcam_capture.cpp
```
