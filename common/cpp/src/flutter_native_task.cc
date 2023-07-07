#include "flutter_native_task.h"

#include <future>
#include <iostream>
#include <thread>

#if RTC_FLUTTER_NATIVE_TASK_LINUX
#include <glib.h>
#endif

#if RTC_FLUTTER_NATIVE_TASK_LINUX
#define RTC_FLUTTER_NATIVE_TASK_IS_SUPPORTED 1
#endif

namespace flutter_webrtc_plugin {
FlutterNativeTask::FlutterNativeTask(
    std::function<void(FlutterNativeTask* runner)> func) {
  worker_function_ = func;
}

// static
void FlutterNativeTask::Create(
    std::function<void(FlutterNativeTask* runner)> func) {
  FlutterNativeTask* task = new FlutterNativeTask(func);
  task->Run();
}

void FlutterNativeTask::Run() {
  worker_thread_ = std::thread([this]() { worker_function_(this); });

#if !(RTC_FLUTTER_NATIVE_TASK_IS_SUPPORTED)
  std::cerr << "Whoops! Looks Like FlutterNativeTask ist not supported on your "
               "platform. The task will run in sync."
            << std::endl;
  worker_thread_.join();
  delete this;
#endif
}

void FlutterNativeTask::InvokeMain(std::function<void()> func) {
  finish_function = func;

#if RTC_FLUTTER_NATIVE_TASK_LINUX
  g_idle_add(
      [](void* data) {
        FlutterNativeTask* task = (FlutterNativeTask*)data;
        task->Finish();
        return FALSE;
      },
      this);
  // TODO:
  // #if RTC_FLUTTER_NATIVE_TASK_WIN
  // #if RTC_FLUTTER_NATIVE_TASK_ELINUX
#else
  Finish();
#endif
}

void FlutterNativeTask::Finish() {
#if RTC_FLUTTER_NATIVE_TASK_IS_SUPPORTED
  worker_thread_.join();
#endif
  finish_function();
#if RTC_FLUTTER_NATIVE_TASK_IS_SUPPORTED
  delete this;
#endif
}

}  // namespace flutter_webrtc_plugin