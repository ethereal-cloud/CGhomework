#include "fluid3d/Lagrangian/include/Renderer.h"
#include <vector>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace FluidSimulation
{
    namespace Lagrangian3d
    {
        namespace
        {
            constexpr float kPi = 3.14159265358979323846f;
        }

        void Renderer::init()
        {
            container = new Glb::Container();
            container->resetSize(1, 1, 1);
            container->init();

            // Build Shaders

            shader = new Glb::Shader();
            std::string drawColorVertPath = shaderPath + "/DrawParticles3d.vert";
            std::string drawColorFragPath = shaderPath + "/DrawParticles3d.frag";
            shader->buildFromFile(drawColorVertPath, drawColorFragPath);

            solidShader = new Glb::Shader();
            std::string solidVertPath = shaderPath + "/Solid3d.vert";
            std::string solidFragPath = shaderPath + "/Solid3d.frag";
            solidShader->buildFromFile(solidVertPath, solidFragPath);

            // Generate Frame Buffers
            // generate frame buffer object
            glGenFramebuffers(1, &FBO);
            // make it active
            // start fbo
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // generate textures
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glBindTexture(GL_TEXTURE_2D, 0);

      
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

            // generate render buffer object (RBO)
            glGenRenderbuffers(1, &RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, imageWidth, imageHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                Glb::Logger::getInstance().addLog("Error: Framebuffer is not complete!");
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glGenBuffers(1, &VBO);
            MakeVertexArrays();
            buildCylinderMesh();

            glEnable(GL_MULTISAMPLE);

            glViewport(0, 0, imageWidth, imageHeight);
        }

        void Renderer::MakeVertexArrays()
        {
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particle3d), (void *)offsetof(particle3d, position));
            glEnableVertexAttribArray(0); // location = 0
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(particle3d), (void *)offsetof(particle3d, density));
            glEnableVertexAttribArray(1); // location = 1
            glBindVertexArray(0);
        }

        void Renderer::buildCylinderMesh()
        {
            const int slices = 32;
            std::vector<glm::vec3> positions;
            std::vector<uint32_t> indices;

            positions.reserve((slices + 1) * 2 + 2);
            for (int i = 0; i <= slices; i++)
            {
                float u = static_cast<float>(i) / static_cast<float>(slices);
                float theta = u * kPi * 2.0f;
                float x = std::cos(theta);
                float y = std::sin(theta);
                positions.emplace_back(x, y, -0.5f);
                positions.emplace_back(x, y, 0.5f);
            }

            indices.reserve(slices * 12);
            for (int i = 0; i < slices; i++)
            {
                uint32_t base = i * 2;
                indices.push_back(base);
                indices.push_back(base + 1);
                indices.push_back(base + 2);
                indices.push_back(base + 1);
                indices.push_back(base + 3);
                indices.push_back(base + 2);
            }

            uint32_t bottomCenter = static_cast<uint32_t>(positions.size());
            positions.emplace_back(0.0f, 0.0f, -0.5f);
            uint32_t topCenter = static_cast<uint32_t>(positions.size());
            positions.emplace_back(0.0f, 0.0f, 0.5f);

            for (int i = 0; i < slices; i++)
            {
                uint32_t bottom0 = i * 2;
                uint32_t bottom1 = (i + 1) * 2;
                indices.push_back(bottomCenter);
                indices.push_back(bottom1);
                indices.push_back(bottom0);

                uint32_t top0 = i * 2 + 1;
                uint32_t top1 = (i + 1) * 2 + 1;
                indices.push_back(topCenter);
                indices.push_back(top0);
                indices.push_back(top1);
            }

            solidIndexCount = static_cast<uint32_t>(indices.size());

            glGenVertexArrays(1, &solidVAO);
            glGenBuffers(1, &solidVBO);
            glGenBuffers(1, &solidEBO);

            glBindVertexArray(solidVAO);
            glBindBuffer(GL_ARRAY_BUFFER, solidVBO);
            glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, solidEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
            glEnableVertexAttribArray(0);
            glBindVertexArray(0);
        }

        void Renderer::draw(ParticleSystem3d &ps)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, ps.particles.size() * sizeof(particle3d), ps.particles.data(), GL_DYNAMIC_COPY);
            particleNum = ps.particles.size();

            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            shader->use();
            shader->setMat4("view", Glb::Camera::getInstance().GetView());
            shader->setMat4("projection", Glb::Camera::getInstance().GetProjection());
            shader->setFloat("scale", ps.scale);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, particleNum);
            shader->unUse();

            if (Lagrangian3dPara::enableStirrer && solidIndexCount > 0)
            {
                float time = Lagrangian3dPara::stirrerTime;
                float omega = 2.0f * kPi * Lagrangian3dPara::stirrerFrequency;
                float theta = omega * time;
                float orbitRadius = Lagrangian3dPara::stirrerOrbitRadius * Lagrangian3dPara::scale;
                glm::vec3 baseCenter = Lagrangian3dPara::stirrerCenter * Lagrangian3dPara::scale;
                glm::vec3 center = baseCenter + glm::vec3(orbitRadius * std::cos(theta), orbitRadius * std::sin(theta), 0.0f);

                float rodRadius = Lagrangian3dPara::stirrerRodRadius * Lagrangian3dPara::scale;
                float rodZMin = Lagrangian3dPara::stirrerRodZMin * Lagrangian3dPara::scale;
                float rodZMax = Lagrangian3dPara::stirrerRodZMax * Lagrangian3dPara::scale;
                float barLength = Lagrangian3dPara::stirrerBarLength * Lagrangian3dPara::scale;
                float barZ = Lagrangian3dPara::stirrerBarZ * Lagrangian3dPara::scale;

                solidShader->use();
                solidShader->setMat4("view", Glb::Camera::getInstance().GetView());
                solidShader->setMat4("projection", Glb::Camera::getInstance().GetProjection());

                if (rodZMax > rodZMin && rodRadius > 0.0f)
                {
                    float rodLength = rodZMax - rodZMin;
                    glm::vec3 rodCenter(center.x, center.y, (rodZMin + rodZMax) * 0.5f);
                    glm::mat4 rodModel(1.0f);
                    rodModel = glm::translate(rodModel, rodCenter);
                    rodModel = glm::scale(rodModel, glm::vec3(rodRadius, rodRadius, rodLength));
                    solidShader->setMat4("model", rodModel);
                    glBindVertexArray(solidVAO);
                    glDrawElements(GL_TRIANGLES, solidIndexCount, GL_UNSIGNED_INT, 0);
                }

                if (barLength > 0.0f && rodRadius > 0.0f)
                {
                    glm::mat4 barModel(1.0f);
                    barModel = glm::translate(barModel, glm::vec3(center.x, center.y, barZ));
                    barModel = glm::rotate(barModel, theta, glm::vec3(0.0f, 0.0f, 1.0f));
                    barModel = glm::rotate(barModel, kPi * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
                    barModel = glm::scale(barModel, glm::vec3(rodRadius, rodRadius, barLength));
                    solidShader->setMat4("model", barModel);
                    glBindVertexArray(solidVAO);
                    glDrawElements(GL_TRIANGLES, solidIndexCount, GL_UNSIGNED_INT, 0);
                }

                solidShader->unUse();
            }

            container->draw();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        GLuint Renderer::getRenderedTexture()
        {
            return textureID;
        }
    }
}
