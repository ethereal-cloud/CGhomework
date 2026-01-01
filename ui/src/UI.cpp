/**
 * UI.cpp: 用户界面主类实现文件
 * 实现UI系统的初始化和主循环
 */

#include "UI.h"

namespace FluidSimulation {

    /**
     * 构造函数
     */
    UI::UI() {
        // 构造函数,无需初始化
    }

    /**
     * 运行UI主循环
     * 初始化窗口、ImGui，并进入主渲染循环
     */
    void UI::run() {
        // 初始化ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // 启用停靠功能
        (void)io;
        // 加载控制台字体
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", fontSize);
        
        // 初始化GLFW
        if (!glfwInit())
        {
            Glb::Logger::getInstance().addLog("GLFW initialization failed.");
            return;
        }
        // 设置OpenGL版本和配置
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // 创建窗口
        GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Fluid Simulation System", NULL, NULL);
        if (!window) {
            Glb::Logger::getInstance().addLog("Fail to create window.");
            glfwTerminate();
            return;
        }
        // 设置为当前上下文
        glfwMakeContextCurrent(window);

        // 初始化GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            Glb::Logger::getInstance().addLog("GLAD initialization failed.");
            return;
        }
        Glb::Logger::getInstance().addLog("GLFW and GLAD initialization succeeded.");

        // 初始化ImGui的OpenGL后端
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui_ImplOpenGL3_CreateDeviceObjects();

        // 初始化UI管理器
        Manager::getInstance().init(window);
        Glb::Logger::getInstance().addLog("Start Main Render Loop...");

        // 主渲染循环
        while (!glfwWindowShouldClose(window)) {
            // 处理事件
            glfwPollEvents();

            // 清除缓冲
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // 开始新一帧
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 显示所有视图
            Manager::getInstance().displayViews();

            // 渲染ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // 交换缓冲
            glfwSwapBuffers(window);
        }

        // 清理资源
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}