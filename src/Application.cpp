//
// Created by jonalama on 12/14/25.
//

#define WEBGPU_CPP_IMPLEMENTATION 
#include "Application.h"

#include "GLFW/glfw3.h"
#include "glfw3webgpu.h"
#include "webgpu/webgpu.hpp"
#include "webgpu/webgpu.h"
#include "wgpu_utils.h"

#include <iostream>

const char* shaderSource = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";

void Application::InitializePipeline() {
	wgpu::ShaderModuleDescriptor shaderDesc;

	wgpu::ShaderSourceWGSL shaderCodeDesc;
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
	shaderDesc.nextInChain = &shaderCodeDesc.chain;
	shaderCodeDesc.code.data = shaderSource;
	shaderCodeDesc.code.length = WGPU_STRLEN;

	wgpu::ShaderModule shaderModule = device.createShaderModule(shaderDesc);

	wgpu::RenderPipelineDescriptor pipelineDesc;

	pipelineDesc.vertex.bufferCount = 0;
	pipelineDesc.vertex.buffers = nullptr;
	pipelineDesc.vertex.module = shaderModule;
	pipelineDesc.vertex.entryPoint.data = "vs_main";
	pipelineDesc.vertex.entryPoint.length = WGPU_STRLEN;
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

	wgpu::FragmentState fragmentState;
	fragmentState.module = shaderModule;
	fragmentState.entryPoint.data = "fs_main";
	fragmentState.entryPoint.length = WGPU_STRLEN;
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

	wgpu::BlendState blendState;
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;

	wgpu::ColorTargetState colorTarget;
	colorTarget.format = surfaceFormat;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All;

	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;
	pipelineDesc.fragment = &fragmentState;

	pipelineDesc.depthStencil = nullptr;
	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;
	pipelineDesc.layout = nullptr;

	pipeline = device.createRenderPipeline(pipelineDesc);

	shaderModule.release();
}

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

	instance = wgpu::createInstance();

	surface = glfwCreateWindowWGPUSurface(instance, window);
	if (!surface) {
		std::cerr << "Failed to create surface" << std::endl;
		glfwTerminate();
		return false;
	}

	std::cout << "Requesting adapter..." << std::endl;
	wgpu::RequestAdapterOptions adapterOptions = {};
	adapterOptions.nextInChain = nullptr;
	adapterOptions.compatibleSurface = surface;
	adapter = requestAdapterSync(instance, &adapterOptions);
	std::cout << "Got adapter" << std::endl;

	instance.release();

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

	// deviceLostCallbackInfo.callback is WGPUDeviceLostCallback — raw C signature
	deviceDesc.deviceLostCallbackInfo.mode = wgpu::CallbackMode::AllowSpontaneous;
	deviceDesc.deviceLostCallbackInfo.callback = [](WGPUDevice const*, WGPUDeviceLostReason reason, WGPUStringView message, void*, void*) {
		std::cout << "Device Lost: reason " << reason;
		if (message.data)
			std::cout << " (" << std::string(message.data, message.length) << ")";
		std::cout << std::endl;
	};

	deviceDesc.uncapturedErrorCallbackInfo.callback = [](WGPUDevice const*, WGPUErrorType type, WGPUStringView message, void*, void*) {
		std::cout << "Uncaptured device error: type " << type;
		if (message.data)
			std::cout << " (" << std::string(message.data, message.length) << ")";
		std::cout << std::endl;
	};

	device = requestDeviceSync(adapter, &deviceDesc);
	std::cout << "Got device" << std::endl;

	queue = device.getQueue();

	wgpu::SurfaceCapabilities surfaceCaps = {};
	surface.getCapabilities(adapter, &surfaceCaps);

	wgpu::SurfaceConfiguration surfaceConfig = {};
	surfaceConfig.nextInChain = nullptr;
	surfaceConfig.viewFormatCount = 0;
	surfaceConfig.viewFormats = nullptr;
	surfaceConfig.device = device;
	surfaceFormat = surfaceCaps.formats[0];
	surfaceConfig.format = surfaceFormat;
	surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
	surfaceConfig.width = 640;
	surfaceConfig.height = 480;
	surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
	surfaceConfig.alphaMode = wgpu::CompositeAlphaMode::Auto;

	surface.configure(surfaceConfig);

	adapter.release();

	InitializePipeline();

	PlayingWithBuffers();

	return true;
}

void Application::PlayingWithBuffers() {
	wgpu::BufferDescriptor bufferDesc = {};
	bufferDesc.label.data = "GPU Side Data Buffer";
	bufferDesc.label.length = WGPU_STRLEN;
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
	bufferDesc.size = 16;
	bufferDesc.mappedAtCreation = false;
	wgpu::Buffer buffer1 = device.createBuffer(bufferDesc);

	bufferDesc.label.data = "Output Buffer";
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
	wgpu::Buffer buffer2 = device.createBuffer(bufferDesc);

	std::vector<uint8_t> data(16);
	for (uint8_t i = 0; i < 16; ++i) { data[i] = i; }

	queue.writeBuffer(buffer1, 0, data.data(), data.size());

	wgpu::CommandEncoder encoder = device.createCommandEncoder();
	encoder.copyBufferToBuffer(buffer1, 0, buffer2, 0, 16);

	wgpu::CommandBuffer command = encoder.finish();
	encoder.release();
	queue.submit(1, &command);
	command.release();

	struct Context {
		bool ready = false;
		wgpu::Buffer buffer;
	};

	auto onBuffer2Mapped = [](WGPUMapAsyncStatus status, WGPUStringView message, void* pUserData1, void*) {
		Context* context = reinterpret_cast<Context*>(pUserData1);
		context->ready = true;
		std::cout << "Buffer 2 mapped with status " << status << " (" << std::string(message.data, message.length) << ")" << std::endl;
		if (status != WGPUMapAsyncStatus_Success) return;

		const uint8_t* buffer_data = static_cast<const uint8_t*>(context->buffer.getConstMappedRange(0, 16));

		std::cout << "bufferData = [";
		for (int i = 0; i < 16; ++i) {
			if (i > 0) std::cout << ", ";
			std::cout << static_cast<int>(buffer_data[i]);
		}
		std::cout << "]" << std::endl;

		context->buffer.unmap();
	};

	Context context;
	context.buffer = buffer2;

	WGPUBufferMapCallbackInfo callbackInfo = {};
	callbackInfo.nextInChain = nullptr;
	callbackInfo.callback = onBuffer2Mapped;
	callbackInfo.userdata1 = &context;
	callbackInfo.userdata2 = nullptr;
	callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;

	wgpuBufferMapAsync(buffer2, WGPUMapMode_Read, 0, 16, callbackInfo);

	while (!context.ready) {
#if defined(WEBGPU_BACKEND_DAWN)
		device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
		device.poll(false, nullptr);
#endif
	}

	buffer1.release();
	buffer2.release();
}

wgpu::Limits Application::GetRequiredLimits(wgpu::Adapter adapter) {
	wgpu::Limits supportedLimits = {};
	adapter.getLimits(&supportedLimits);
	wgpu::Limits requiredLimits = wgpu::Default;
	return requiredLimits;
}

void Application::InitializeBuffers() {
}

void Application::Terminate() {
	pipeline.release();
	surface.unconfigure();
	queue.release();
	surface.release();
	device.release();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::MainLoop() {
	glfwPollEvents();

	auto [surfaceTexture, targetView] = GetNextSurfaceViewData();
	if (!targetView) return;

	wgpu::CommandEncoderDescriptor encoderDesc = {};
	encoderDesc.nextInChain = nullptr;
	encoderDesc.label.data = "My command encoder";
	encoderDesc.label.length = WGPU_STRLEN;
	wgpu::CommandEncoder encoder = device.createCommandEncoder(encoderDesc);

	wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = targetView;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
	renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
	renderPassColorAttachment.clearValue = wgpu::Color{ 0.9, 0.1, 0.2, 1.0 };
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

	wgpu::RenderPassDescriptor renderPassDesc = {};
	renderPassDesc.nextInChain = nullptr;
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;
	renderPassDesc.depthStencilAttachment = nullptr;
	renderPassDesc.timestampWrites = nullptr;

	wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
	renderPass.setPipeline(pipeline);
	renderPass.draw(3, 1, 0, 0);
	renderPass.end();
	renderPass.release();

	wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.nextInChain = nullptr;
	cmdBufferDescriptor.label.data = "Command buffer";
	cmdBufferDescriptor.label.length = WGPU_STRLEN;
	wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
	encoder.release();

	queue.submit(1, &command);
	command.release();

	targetView.release();
	surface.present();

#if defined(WEBGPU_BACKEND_DAWN)
	device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
	device.poll(false, nullptr);
#endif
}

bool Application::IsRunning() {
	return !glfwWindowShouldClose(window);
}

std::pair<wgpu::SurfaceTexture, wgpu::TextureView> Application::GetNextSurfaceViewData() {
	wgpu::SurfaceTexture surfaceTexture = {};
	surface.getCurrentTexture(&surfaceTexture);

	if (surfaceTexture.status == wgpu::SurfaceGetCurrentTextureStatus::Error)
		return {surfaceTexture, nullptr};

	// surfaceTexture.texture is a raw WGPUTexture handle — wrap it
	wgpu::Texture texture = wgpu::Texture(surfaceTexture.texture);

	wgpu::TextureViewDescriptor viewDescriptor = {};
	viewDescriptor.nextInChain = nullptr;
	viewDescriptor.label.data = "Surface Texture View";
	viewDescriptor.label.length = WGPU_STRLEN;
	viewDescriptor.format = texture.getFormat();
	viewDescriptor.dimension = wgpu::TextureViewDimension::_2D;
	viewDescriptor.baseMipLevel = 0;
	viewDescriptor.mipLevelCount = 1;
	viewDescriptor.baseArrayLayer = 0;
	viewDescriptor.arrayLayerCount = 1;
	viewDescriptor.aspect = wgpu::TextureAspect::All;

	wgpu::TextureView targetView = texture.createView(viewDescriptor);

	return {surfaceTexture, targetView};
}