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
	GLFWwindow* window;
	wgpu::Instance instance;
	wgpu::Device device;
	wgpu::Queue queue;
	wgpu::Surface surface;
	wgpu::Adapter adapter;
};

#endif //APPLICATION_H