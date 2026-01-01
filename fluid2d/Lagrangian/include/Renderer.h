#pragma once
#ifndef __LAGRANGIAN_2D_RENDERER_H__
#define __LAGRANGIAN_2D_RENDERER_H__

#include <glad/glad.h>
#include <glfw3.h>

#include <chrono>
#include <vector>
#include "Shader.h"
#include <glm/glm.hpp>

#include "ParticleSystem2d.h"
#include "Configure.h"
#include <Logger.h>

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        // 拉格朗日法流体渲染器类
        // 负责将粒子系统可视化
        class Renderer
        {
        public:
            Renderer();

            int32_t init();                                // 初始化渲染器
            void draw(ParticleSystem2d& ps);               // 绘制粒子系统
            GLuint getRenderedTexture();                   // 获取渲染结果的纹理ID

        private:
            Glb::Shader *shader = nullptr;                 // 着色器程序

            GLuint VAO = 0;                                // 顶点数组对象
            GLuint positionVBO = 0;                        // 位置顶点缓冲对象
            GLuint densityVBO = 0;                         // 密度顶点缓冲对象

            GLuint FBO = 0;                                // 帧缓冲对象
            GLuint textureID = 0;                          // 渲染纹理ID
            GLuint RBO = 0;                                // 渲染缓冲对象

            size_t particleNum = 0;                        // 粒子数量
        };
    }
}

#endif
