# [Release] QtMoCap — private webcam mocap (HOG boxes + COCO skeleton)

Hi all — sharing a small tool I built on Kali: **QtMoCap**, a private,
local-only motion-capture toy. Webcam in, bounding boxes + skeleton out.
No cloud, no accounts, MIT licensed.

## What it does
- Mirror selfie view (640x480) with live **HOG people boxes** + person count + FPS
- **COCO DNN skeleton overlay** (18 joints, async thread so video never freezes)
- Backlight-proofed detection (luma equalization, dense scan)
- 📷 Snap to PNG, ⏺ record to MJPEG AVI, camera switcher

## Stack
Qt6 Widgets + OpenCV 4 (HOG + DNN), C++17. Tested on Kali rolling
running bare-metal on a Chromebook (Google Lillipup) — yes, really.

## Install
```bash
sudo dpkg -i qtmocap_0.1.1_amd64.deb
# one-time 200M pose model:
mkdir -p qtmocap/models
curl -sSL -o qtmocap/models/pose_coco.caffemodel \
  https://huggingface.co/dylanholmes/openpose-caffemodels/resolve/main/coco.caffemodel
curl -sS -o qtmocap/models/pose_coco.prototxt \
  https://raw.githubusercontent.com/opencv/opencv_extra/4.x/testdata/dnn/openpose_pose_coco.prototxt
```
Source + docs: https://github.com/jack6000000-mok/cto (qtmocap/ folder, MIT)

## Notes
- Without the model it still does HOG boxes (💀 button shows N/A).
- Skeleton lags video ~3s on CPU by design (async thread) — boxes stay live.
- Feedback welcome: false-positive ghosts on curtains at current thresholds?

— oday
