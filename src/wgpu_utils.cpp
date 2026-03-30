//
// Created by jonalama on 12/14/25.
//

#include <vector>
#include <iostream>

#include <wgpu_utils.h>

// The wrapper exposes a synchronous single-arg overload for both of these
wgpu::Adapter requestAdapterSync(wgpu::Instance instance, wgpu::RequestAdapterOptions const* options) {
	return instance.requestAdapter(*options);
}

wgpu::Device requestDeviceSync(wgpu::Adapter adapter, wgpu::DeviceDescriptor const* descriptor) {
	return adapter.requestDevice(*descriptor);
}

void inspectAdapter(wgpu::Adapter adapter) {
	wgpu::AdapterInfo adapterInfo = {};
	adapter.getInfo(&adapterInfo);

	std::cout << "Adapter info:" << std::endl;
	if (adapterInfo.vendor.data)
		std::cout << " - vendor: " << std::string(adapterInfo.vendor.data, adapterInfo.vendor.length) << std::endl;
	if (adapterInfo.architecture.data)
		std::cout << " - architecture: " << std::string(adapterInfo.architecture.data, adapterInfo.architecture.length) << std::endl;
	if (adapterInfo.device.data)
		std::cout << " - device: " << std::string(adapterInfo.device.data, adapterInfo.device.length) << std::endl;
	if (adapterInfo.description.data)
		std::cout << " - description: " << std::string(adapterInfo.description.data, adapterInfo.description.length) << std::endl;
	std::cout << " - backendType: " << static_cast<uint32_t>(adapterInfo.backendType) << std::endl;
	std::cout << " - adapterType: " << static_cast<uint32_t>(adapterInfo.adapterType) << std::endl;
	std::cout << " - vendorID: 0x" << std::hex << adapterInfo.vendorID << std::dec << std::endl;
	std::cout << " - deviceID: 0x" << std::hex << adapterInfo.deviceID << std::dec << std::endl;

	wgpuAdapterInfoFreeMembers(adapterInfo);

	wgpu::SupportedFeatures features = {};
	adapter.getFeatures(&features);
	std::cout << "Adapter features: " << features.featureCount << " features" << std::endl;
	wgpuSupportedFeaturesFreeMembers(features);

	wgpu::Limits limits = {};
	if (adapter.getLimits(&limits)) {
		std::cout << "Adapter limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << limits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << limits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << limits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << limits.maxTextureArrayLayers << std::endl;
	}
}

void inspectDevice(wgpu::Device device) {
	wgpu::SupportedFeatures features = {};
	device.getFeatures(&features);
	std::cout << "Device features: " << features.featureCount << " features" << std::endl;

	wgpu::Limits limits = {};
	if (device.getLimits(&limits)) {
		std::cout << "Device limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << limits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << limits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << limits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << limits.maxTextureArrayLayers << std::endl;
		std::cout << " - maxBindGroups: " << limits.maxBindGroups << std::endl;
		std::cout << " - maxVertexBuffers: " << limits.maxVertexBuffers << std::endl;
		std::cout << " - maxVertexAttributes: " << limits.maxVertexAttributes << std::endl;
	}
}