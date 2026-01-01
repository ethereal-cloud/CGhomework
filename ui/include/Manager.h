#pragma once
#ifndef __MANAGER_H__
#define __MANAGER_H__

// OpenGL相关头文件
#include "glad/glad.h"
#include "glfw3.h"

// ImGui相关头文件
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// UI视图头文件
#include "SceneView.h"
#include "InspectorView.h"
#include "ProjectView.h"

// 流体模拟组件头文件
#include "Lagrangian2dComponent.h"
#include "Eulerian2dComponent.h"
#include "Lagrangian3dComponent.h"
#include "Eulerian3dComponent.h"

#include <vector>

namespace FluidSimulation
{
	class SceneView;
	class InspectorView;
	class ProjectView;

	// UI管理器类(单例模式)
	// 负责管理所有UI视图和流体模拟组件
	class Manager {
	public:
		// 获取单例实例
		static Manager& getInstance() {
			static Manager instance;
			return instance;
		}

		void init(GLFWwindow* window);      // 初始化管理器
		void displayViews();                // 显示所有视图
		void displayToolBar();              // 显示工具栏

		// Getter/Setter方法
		SceneView* getSceneView() const { return sceneView; };
		InspectorView* getInspectorView() const { return inspectorView; };
		ProjectView* getProjectView() const { return projectView; };
		GLFWwindow* getWindow() const { return window; };
		Glb::Component* getMethod() const { return currentMethod; };
		void setMethod(Glb::Component* method) { currentMethod = method; };

	private:
		// 私有构造函数(单例模式)
		Manager() {
			window = NULL;
			sceneView = NULL;
			inspectorView = NULL;
			projectView = NULL;
			currentMethod = NULL;
		};

		// 禁止拷贝和赋值(单例模式)
		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;
		
		GLFWwindow* window;                 // GLFW窗口
		SceneView* sceneView;               // 场景视图
		InspectorView* inspectorView;       // 检视器视图
		ProjectView* projectView;           // 项目视图

		Glb::Component* currentMethod;      // 当前选择的模拟方法
	};
}

#endif