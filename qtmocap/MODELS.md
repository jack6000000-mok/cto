Model download (one time, 200M):
  mkdir -p qtmocap/models
  curl -sSL -o qtmocap/models/pose_coco.caffemodel https://huggingface.co/dylanholmes/openpose-caffemodels/resolve/main/coco.caffemodel
  curl -sS -o qtmocap/models/pose_coco.prototxt https://raw.githubusercontent.com/opencv/opencv_extra/4.x/testdata/dnn/openpose_pose_coco.prototxt
