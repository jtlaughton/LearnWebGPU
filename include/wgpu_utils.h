//
// Created by jonalama on 12/14/25.
//

#ifndef WGPU_UTILS_H
#define WGPU_UTILS_H
#include <webgpu/webgpu_cpp.h>

wgpu::Adapter requestAdapterSync(wgpu::Instance instance, wgpu::RequestAdapterOptions const * options);
wgpu::Device requestDeviceSync(wgpu::Adapter adapter, wgpu::DeviceDescriptor const * descriptor);
void inspectAdapter(wgpu::Adapter adapter);
void inspectDevice(wgpu::Device device);

#endif //WGPU_UTILS_H
