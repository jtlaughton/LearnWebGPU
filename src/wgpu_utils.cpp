//
// Created by jonalama on 12/14/25.
//

#include <vector>
#include <cassert>
#include <iostream>

#include <wgpu_utils.h>

wgpu::Adapter requestAdapterSync(wgpu::Instance instance, wgpu::RequestAdapterOptions const * options) {
	struct UserData {
		wgpu::Adapter adapter = nullptr;
		bool requestEnded = false;
	};
	UserData userData;

	auto callback = [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message, UserData* pUserData) {
		if (status == wgpu::RequestAdapterStatus::Success) {
			pUserData->adapter = adapter;
		} else {
			std::cout << "Could not get WebGPU adapter: " << std::string_view(message) << std::endl;
		}
		pUserData->requestEnded = true;
	};

	instance.RequestAdapter(options, wgpu::CallbackMode::AllowSpontaneous, callback, &userData);

	assert(userData.requestEnded);

	return userData.adapter;
}

wgpu::Device requestDeviceSync(wgpu::Adapter adapter, wgpu::DeviceDescriptor const * descriptor) {
	struct UserData {
		wgpu::Device device = nullptr;
		bool requestEnded = false;
	};
	UserData userData;

	auto callback = [](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message, UserData* pUserData){
		if (status == wgpu::RequestDeviceStatus::Success) {
			pUserData->device = device;
		}
		else {
			std::cout << "Could not get WebGPU device: " << std::string_view(message) << std::endl;
		}

		pUserData->requestEnded = true;
	};

	adapter.RequestDevice(descriptor, wgpu::CallbackMode::AllowSpontaneous, callback, &userData);

	assert(userData.requestEnded);

	return userData.device;
}

void inspectAdapter(wgpu::Adapter adapter) {
	wgpu::AdapterInfo adapterInfo = {};
	adapter.GetInfo(&adapterInfo);

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

	// Get limits
	wgpu::SupportedFeatures features = {};
	adapter.GetFeatures(&features);

	std::cout << "Adapter features: " << features.featureCount << " features" << std::endl;

	wgpuSupportedFeaturesFreeMembers(features);

	// Limits
	wgpu::Limits limits = {};

	if (adapter.GetLimits(&limits)) {
		std::cout << "Adapter limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << limits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << limits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << limits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << limits.maxTextureArrayLayers << std::endl;
	}
}

void inspectDevice(wgpu::Device device) {
	wgpu::SupportedFeatures features = {};
	device.GetFeatures(&features);

	std::cout << "Device features: " << features.featureCount << " features" << std::endl;

	wgpu::Limits limits = {};

	if (device.GetLimits(&limits)) {
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