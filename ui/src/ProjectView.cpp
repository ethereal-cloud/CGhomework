/**
 * ProjectView.cpp: 项目视图实现文件
 * 显示系统日志信息
 */

#include "ProjectView.h"

namespace FluidSimulation {
	/**
	 * 默认构造函数
	 */
	ProjectView::ProjectView() {
		// 默认构造函数
	}

	/**
	 * 构造函数
	 * @param window GLFW窗口
	 */
	ProjectView::ProjectView(GLFWwindow* window) {
		// 保存窗口指针
		this->window = window;
	}

	/**
	 * 显示项目视图
	 * 显示所有系统日志消息
	 */
	void ProjectView::display() {
		// 创建项目视图窗口
		ImGui::Begin("Project", NULL, ImGuiWindowFlags_NoCollapse);

		// 创建可滚动区域
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		// 显示所有日志消息
		for (const auto& message : Glb::Logger::getInstance().getLog()) {
			// 设置日志文本颜色为白色
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::TextWrapped("%s", message.c_str());
			ImGui::PopStyleColor();
		}

		// 如果滚动到底部,自动显示最新日志
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
		ImGui::End();
	}
}