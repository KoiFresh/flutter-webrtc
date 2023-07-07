#ifndef FLUTTER_WEBRTC_RTC_NATIVE_TASK_HXX
#define FLUTTER_WEBRTC_RTC_NATIVE_TASK_HXX

#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <mutex>
#include <thread>

namespace flutter_webrtc_plugin {

class FlutterNativeTask {
 public:
  static std::mutex mutex;
  static void Create(std::function<void(FlutterNativeTask* runner)> func);

  void Run();

  void InvokeMain(std::function<void()> func);

 private:
  FlutterNativeTask(std::function<void(FlutterNativeTask* runner)> func);

  void Finish();

  std::thread worker_thread_;

  std::function<void(FlutterNativeTask* runner)> worker_function_;
  std::function<void()> finish_function;
};

}  // namespace flutter_webrtc_plugin

#endif  // ! FLUTTER_WEBRTC_RTC_NATIVE_TASK_HXX
