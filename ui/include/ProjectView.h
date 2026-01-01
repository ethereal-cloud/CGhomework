#pragma once
#ifndef __PROJECT_VIEW_H__
#define __PROJECT_VIEW_H__

// OpenGL相关头文件
#include "glad/glad.h"
#include "glfw3.h"

// ImGui相关头文件
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Manager.h"

namespace FluidSimulation {
	// 项目视图类
	// 负责显示系统日志和调试信息
	class ProjectView {
	private:
		GLFWwindow* window;	// GLFW窗口
		ImVec2 pos;			// 视图位置

	public:
		ProjectView();
		ProjectView(GLFWwindow* window);
		void display();		// 显示日志信息
	};
}

#endif