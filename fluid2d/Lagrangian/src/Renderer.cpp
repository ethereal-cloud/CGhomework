#include "Lagrangian/include/Renderer.h"

#include <iostream>
#include <fstream>
#include "Configure.h"

// 这是渲染器的主要实现,涉及OpenGL函数的使用
// 在阅读此文件之前,请确保您对OpenGL有基本了解

namespace FluidSimulation
{

    namespace Lagrangian2d
    {

        Renderer::Renderer()
        {
        }

        // 初始化渲染器
        int32_t Renderer::init()
        {
            extern std::string shaderPath;

            // 加载并编译粒子着色器
            std::string particleVertShaderPath = shaderPath + "/DrawParticles2d.vert";
            std::string particleFragShaderPath = shaderPath + "/DrawParticles2d.frag";
            shader = new Glb::Shader();
            shader->buildFromFile(particleVertShaderPath, particleFragShaderPath);

            // 生成顶点数组对象(VAO)
            glGenVertexArrays(1, &VAO);
            // 生成位置的顶点缓冲对象(VBO)
            glGenBuffers(1, &positionVBO);
            // 生成密度的顶点缓冲对象(VBO)
            glGenBuffers(1, &densityVBO);

            // 生成帧缓冲对象(FBO)
            glGenFramebuffers(1, &FBO);
            // 绑定帧缓冲
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // 生成并设置纹理
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glBindTexture(GL_TEXTURE_2D, 0);

            // 将纹理附加到帧缓冲
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

            // 生成渲染缓冲对象(RBO)
            glGenRenderbuffers(1, &RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, imageWidth, imageHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // 将RBO附加到帧缓冲
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                Glb::Logger::getInstance().addLog("Error: Framebuffer is not complete!");
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 设置视口大小
            glViewport(0, 0, imageWidth, imageHeight);

            return 0;
        }

        // 绘制粒子系统
        void Renderer::draw(ParticleSystem2d& ps)
        {
            // 绑定VAO
            glBindVertexArray(VAO);

            // 绑定并更新位置VBO
            glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
            glBufferData(GL_ARRAY_BUFFER, ps.particles.size() * sizeof(ParticleInfo2d), ps.particles.data(), GL_STATIC_DRAW);

            // 设置位置属性
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo2d), (void*)offsetof(ParticleInfo2d, position));
            glEnableVertexAttribArray(0);

            // 设置密度属性
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo2d), (void*)offsetof(ParticleInfo2d, density));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);

            // 保存粒子数量
            particleNum = ps.particles.size();

            // 绑定帧缓冲开始渲染
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // 清空缓冲区
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 准备渲染
            glBindVertexArray(VAO);
            shader->use();
            shader->setFloat("scale", ps.scale);

            // 启用点精灵
            glEnable(GL_PROGRAM_POINT_SIZE);

            // 绘制所有粒子
            glDrawArrays(GL_POINTS, 0, particleNum);

            // 解绑帧缓冲
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // 获取渲染结果的纹理ID
        GLuint Renderer::getRenderedTexture()
        {
            return textureID;
        }
    }

}