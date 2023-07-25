#pragma once
#include <common.h>

class Queue {
public:
  WGPUQueue wgpu_queue;
  Queue(WGPUDevice device) { wgpu_queue = wgpuDeviceGetQueue(device); };
  ~Queue(){};
};
