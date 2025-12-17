//
// Created by jonalama on 12/14/25.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include "GLFW/glfw3.h"
#include <webgpu/webgpu_cpp.h>

class Application {
public:
	bool Initialize();
	void Terminate();
	void MainLoop();
	bool IsRunning();
private:
	std::pair<wgpu::SurfaceTexture, wgpu::TextureView> GetNextSurfaceViewData();
	void InitializePipeline();
	wgpu::Limits GetRequiredLimits(wgpu::Adapter adapter);
	void InitializeBuffers();
	void PlayingWithBuffers();
private:
	GLFWwindow* window;
	wgpu::Instance instance;
	wgpu::Device device;
	wgpu::Queue queue;
	wgpu::Surface surface;
	wgpu::Adapter adapter;
	wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::Undefined;
	wgpu::RenderPipeline pipeline;
};

#endif //APPLICATION_H