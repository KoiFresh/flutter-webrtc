#include "flutter_frame_capturer.h"
#include <stdio.h>
#include <stdlib.h>

#include <png.h>

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

  bool success = SaveFrame();
  mutex_.unlock();

  std::shared_ptr<MethodResultProxy> result_ptr(result.release());
  if (success) {
    result_ptr->Success();
  } else {
    result_ptr->Error("1", "The captured frame could not be saved.");
  }
}

// see:
// https://stackoverflow.com/questions/21853224/save-opengl-screen-pixels-to-png-using-libpng
bool FlutterFrameCapturer::SaveFrame() {
  if (frame_ == nullptr) {
    return false;
  }

  int width = frame_.get()->width();
  int height = frame_.get()->height();
  int bytes_per_pixel = 4;
  uint8_t* pixels = new uint8_t[width * height * bytes_per_pixel];

  frame_.get()->ConvertToARGB(RTCVideoFrame::Type::kABGR, pixels,
                              /* unused */ -1, width, height);

  png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png) {
    return false;
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
    png_destroy_write_struct(&png, &info);
    return false;
  }

  FILE* fp = fopen(path_.c_str(), "wb");
  if (!fp) {
    png_destroy_write_struct(&png, &info);
    return false;
  }

  png_init_io(png, fp);
  png_set_IHDR(png, info, width, height, 8 /* depth */,
               PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_colorp palette =
      (png_colorp)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
  if (!palette) {
    fclose(fp);
    png_destroy_write_struct(&png, &info);
    return false;
  }
  png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
  png_write_info(png, info);
  png_set_packing(png);

  png_bytepp rows = (png_bytepp)png_malloc(png, height * sizeof(png_bytep));
  for (int row = 0; row < height; ++row) {
    rows[row] = (png_bytep)(pixels + row * width * bytes_per_pixel);
  }

  png_write_image(png, rows);
  png_write_end(png, info);
  png_free(png, palette);
  png_destroy_write_struct(&png, &info);

  fclose(fp);
  delete[] rows;
  return true;
}

}  // namespace flutter_webrtc_plugin