//
// Created by jonalama on 12/14/25.
//

#include "Application.h"

#include "GLFW/glfw3.h"
#include "glfw3webgpu.h"
#include "wgpu_utils.h"

#include <bits/this_thread_sleep.h>
#include <iostream>

struct DummyUserData {};

bool Application::Initialize() {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	instance = wgpu::CreateInstance(nullptr);

	std::cout << "Requesting adapter..." << std::endl;
	surface = wgpu::Surface::Acquire( glfwCreateWindowWGPUSurface(instance.Get(), window));
	if (!surface) {
		std::cerr << "Failed to create GLFW surface" << std::endl;
		glfwTerminate();
		return false;
	}

	wgpu::RequestAdapterOptions adapterOptions = {};
	adapterOptions.nextInChain = nullptr;
	adapterOptions.compatibleSurface = surface;
	adapter = requestAdapterSync(instance, &adapterOptions);
	std::cout << "Got adapter" << std::endl;

	std::cout << "Requesting device..." << std::endl;
	wgpu::DeviceDescriptor deviceDesc = {};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label.data = "My Device";
	deviceDesc.label.length = WGPU_STRLEN;
	deviceDesc.requiredFeatureCount = 0;
	deviceDesc.requiredLimits = nullptr;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label.data = "The Default Queue";
	deviceDesc.defaultQueue.label.length = WGPU_STRLEN;

	// Device lost callback - new API signature
	DummyUserData data;
	auto callback = [](wgpu::Device const& /*device unused*/, wgpu::DeviceLostReason reason, wgpu::StringView message, DummyUserData* /*userdata*/) {
		std::cout << "Device Lost: reason " << static_cast<uint32_t>(reason);
		if (message.data)
			std::cout << " (" << std::string(message.data, message.length) << ")";
		std::cout << std::endl;
	};
	deviceDesc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous, callback, &data);

	// Device error callback
	auto errorCallback = [](wgpu::Device const& /*device unused*/, wgpu::ErrorType type, wgpu::StringView message, DummyUserData* /*userdata*/) {
		std::cout << "Uncaptured device error: type " << static_cast<uint32_t>(type);
		if (message.data)
			std::cout << " (" << std::string(message.data, message.length) << ")";
		std::cout << std::endl;
	};
	deviceDesc.SetUncapturedErrorCallback(errorCallback, &data);

	device = requestDeviceSync(adapter, &deviceDesc);
	std::cout << "Got device" << std::endl;

	queue = device.GetQueue();

	// Configure the surface IMMEDIATELY after getting the queue
	wgpu::SurfaceCapabilities surfaceCaps = {};
	surface.GetCapabilities(adapter, &surfaceCaps);

	wgpu::SurfaceConfiguration surfaceConfig = {};
	surfaceConfig.nextInChain = nullptr;
	surfaceConfig.viewFormatCount = 0;
	surfaceConfig.viewFormats = nullptr;
	surfaceConfig.device = device;
	surfaceConfig.format = surfaceCaps.formats[0];  // Use the first supported format
	surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
	surfaceConfig.width = 640;
	surfaceConfig.height = 480;
	surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
	surfaceConfig.alphaMode = wgpu::CompositeAlphaMode::Auto;  // Changed to Opaque

	surface.Configure(&surfaceConfig);

	return true;
}

void Application::Terminate() {
	// how do I release here
	surface.Unconfigure();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::MainLoop() {
	glfwPollEvents();

	auto [ surfaceTexture, targetView ] = GetNextSurfaceViewData();
	if (!targetView) return;

	// create the command encoder
	wgpu::CommandEncoderDescriptor encoderDesc = {};
	encoderDesc.nextInChain = nullptr;
	encoderDesc.label.data = "My command encoder";
	encoderDesc.label.length = WGPU_STRLEN;
	wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

	// make a color attachment (like a clear color in OpenGL
	wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = targetView;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
	renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
	renderPassColorAttachment.clearValue = wgpu::Color{ 0.9, 0.1, 0.2, 1.0};
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

	// make a descriptor for what we want to render (the color)
	wgpu::RenderPassDescriptor renderPassDesc = {};
	renderPassDesc.nextInChain = nullptr;
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;
	renderPassDesc.depthStencilAttachment = nullptr;
	renderPassDesc.timestampWrites = nullptr;

	// make the render pass? - claude explain
	wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);
	renderPass.End();

	// submit a command finish. Not sure why we need to do this - claude explain
	wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.nextInChain = nullptr;
	cmdBufferDescriptor.label.data = "Command buffer";
	cmdBufferDescriptor.label.length = WGPU_STRLEN;
	wgpu::CommandBuffer command = encoder.Finish(&cmdBufferDescriptor);

	// submit the command queue
	queue.Submit(1, &command);

	// present surface
	surface.Present();

	device.Tick();
}

bool Application::IsRunning() {
	return !glfwWindowShouldClose(window);
}

std::pair<wgpu::SurfaceTexture, wgpu::TextureView> Application::GetNextSurfaceViewData() {
	wgpu::SurfaceTexture surfaceTexture = {};
	surface.GetCurrentTexture(&surfaceTexture);

	if (surfaceTexture.status == wgpu::SurfaceGetCurrentTextureStatus::Error)
		return {surfaceTexture, nullptr};

	wgpu::TextureViewDescriptor viewDescriptor = {};
	viewDescriptor.nextInChain = nullptr;
	viewDescriptor.label.data = "Surface Texture View";
	viewDescriptor.label.length = WGPU_STRLEN;
	viewDescriptor.format = surfaceTexture.texture.GetFormat();
	viewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
	viewDescriptor.baseMipLevel = 0;
	viewDescriptor.mipLevelCount = 1;
	viewDescriptor.baseArrayLayer = 0;
	viewDescriptor.arrayLayerCount = 1;
	viewDescriptor.aspect = wgpu::TextureAspect::All;

	wgpu::TextureView targetView = surfaceTexture.texture.CreateView(&viewDescriptor);

	return {surfaceTexture, targetView};
}