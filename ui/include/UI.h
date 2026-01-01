/**
 * UI.h: 用户界面主类头文件
 * 定义UI系统的主入口类
 */

#pragma once
#ifndef __UI_H__
#define __UI_H__

#include "glad/glad.h"
#include "glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Manager.h"
#include "Logger.h"

namespace FluidSimulation 
{
	using namespace std;

	/**
	 * UI主类
	 * 负责初始化和管理整个UI系统，包括GLFW窗口和ImGui
	 */
	class UI {
	private:
	public:
		/**
		 * 构造函数
		 */
		UI();
		
		/**
		 * 运行UI主循环
		 * 初始化窗口、ImGui，并进入主渲染循环
		 */
		void run();
	};
}

#endif