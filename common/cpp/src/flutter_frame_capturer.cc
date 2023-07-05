#include "flutter_frame_capturer.h"
#include <stdio.h>
#include <stdlib.h>

#include "lodepng.h"

namespace flutter_webrtc_plugin {

FlutterFrameCapturer::FlutterFrameCapturer(RTCVideoTrack* track,
                                           std::string path) {
  track_ = track;
  path_ = path;
}

void FlutterFrameCapturer::OnFrame(scoped_refptr<RTCVideoFrame> frame) {
  if (frame_ != nullptr) {
    return;
  }

  frame_ = frame.get()->Copy();
  mutex_.unlock();
}

void FlutterFrameCapturer::CaptureFrame(
    std::unique_ptr<MethodResultProxy> result) {
  mutex_.lock();
  track_->AddRenderer(this);
  // Here the OnFrame method has to unlock the mutex
  mutex_.lock();
  track_->RemoveRenderer(this);

  int error = SaveFrame();
  mutex_.unlock();

  std::shared_ptr<MethodResultProxy> result_ptr(result.release());
  if (error) {
    result_ptr->Error(std::to_string(error), lodepng_error_text(error));
  } else {
    result_ptr->Success();
  }
}

int FlutterFrameCapturer::SaveFrame() {
  if (frame_ == nullptr) {
    return false;
  }

  int width = frame_.get()->width();
  int height = frame_.get()->height();
  int bytes_per_pixel = 4;
  uint8_t* pixels = new uint8_t[width * height * bytes_per_pixel];

  frame_.get()->ConvertToARGB(RTCVideoFrame::Type::kABGR, pixels,
                              /* unused */ -1, width, height);

  std::vector<unsigned char> png;

  unsigned int error = lodepng::encode(png, pixels, width, height);
  if (error) {
    return error;
  }

  error = lodepng::save_file(png, path_);
  return error;
}

}  // namespace flutter_webrtc_plugin