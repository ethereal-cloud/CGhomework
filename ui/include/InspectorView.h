#pragma once
#ifndef __INSPECTOR_VIEW_H__
#define __INSPECTOR_VIEW_H__

// OpenGL相关头文件
#include "glad/glad.h"
#include "glfw3.h"

// ImGui相关头文件
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// 项目相关头文件
#include "Configure.h"
#include "Manager.h"
#include "Logger.h"

#include <iostream>
#include <string>

namespace FluidSimulation {
	// 检视器视图类
	// 负责显示和编辑当前流体模拟方法的参数
	class InspectorView {
	private:
		GLFWwindow* window;	// GLFW窗口
		ImVec2 pos;			// 视图位置

	public:
		int showID;			// 是否显示组件ID

		InspectorView();
		InspectorView(GLFWwindow* window);
		void display();		// 显示参数编辑面板
	};
}

#endif