/**
 * Manager.cpp: UI管理器实现文件
 * 实现UI系统的管理和视图显示
 */

#include "Manager.h"

namespace FluidSimulation
{
    /**
     * 初始化管理器
     * 创建所有视图并注册流体模拟组件
     * @param window GLFW窗口
     */
    void Manager::init(GLFWwindow* window) {
        // 保存窗口指针
        this->window = window;

        // 创建各个视图
        inspectorView = new InspectorView(window);
        projectView =  new ProjectView(window);
        sceneView = new SceneView(window);

        // 创建并注册所有流体模拟组件
        int id = 0;
        methodComponents.push_back(new Lagrangian2d::Lagrangian2dComponent("Lagrangian 2d", id++));
        methodComponents.push_back(new Eulerian2d::Eulerian2dComponent("Eulerian 2d", id++));
        methodComponents.push_back(new Lagrangian3d::Lagrangian3dComponent("Lagrangian 3d", id++));
        methodComponents.push_back(new Eulerian3d::Eulerian3dComponent("Eulerian 3d", id++));
        // TODO(optional): 添加更多模拟方法
    }

    /**
     * 显示所有视图
     * 设置停靠空间并显示场景视图、检视器视图和项目视图
     */
	void Manager::displayViews() {
        // 设置停靠空间标志
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGui::DockSpaceOverViewport(nullptr, dockspace_flags);

        // 设置主题颜色
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        // 显示所有视图
        sceneView->display();
        inspectorView->display();
        projectView->display();

        // 弹出主题颜色
        ImGui::PopStyleColor(5);
	}

    /**
     * 显示工具栏
     * TODO: 实现工具栏功能
     */
    void Manager::displayToolBar() {
        // TODO: 实现工具栏
    }
}