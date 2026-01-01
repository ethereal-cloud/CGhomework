#pragma once
#ifndef __LAGRANGIAN_3D_RENDERER_H__
#define __LAGRANGIAN_3D_RENDERER_H__

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "glfw3.h"
#include "Shader.h"
#include "Container.h"
#include "Camera.h"
#include "Global.h"
#include "Configure.h"
#include <Logger.h>
#include "ParticleSystem3d.h"

namespace FluidSimulation
{
    namespace Lagrangian3d
    {
        // 拉格朗日法流体渲染器类
        // 负责将三维粒子系统可视化
        class Renderer
        {
        public:
            Renderer(){};

            void init();                          // 初始化渲染器
            GLuint getRenderedTexture();          // 获取渲染结果的纹理ID
            void draw(ParticleSystem3d &ps);      // 绘制粒子系统

        private:
            void MakeVertexArrays();              // 创建顶点数组
            void buildSphereMesh();               // 生成球体网格

        private:
            // 着色器和容器
            Glb::Shader *shader = nullptr;        // 着色器程序
            Glb::Shader *sphereShader = nullptr;  // 球体着色器
            Glb::Container *container = nullptr;   // 容器对象

            // OpenGL渲染对象
            GLuint FBO = 0;                       // 帧缓冲对象
            GLuint RBO = 0;                       // 渲染缓冲对象
            GLuint VAO = 0;                       // 顶点数组对象
            GLuint VBO = 0;                       // 顶点缓冲对象
            GLuint sphereVAO = 0;                 // 球体VAO
            GLuint sphereVBO = 0;                 // 球体VBO
            GLuint sphereEBO = 0;                 // 球体EBO
            GLuint textureID = 0;                 // 渲染纹理ID

            int32_t particleNum = 0;              // 粒子数量
            uint32_t sphereIndexCount = 0;        // 球体索引数量
        };
    }
}

#endif
