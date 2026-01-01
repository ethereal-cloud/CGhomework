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

            sphereShader = new Glb::Shader();
            std::string solidVertPath = shaderPath + "/Solid3d.vert";
            std::string solidFragPath = shaderPath + "/Solid3d.frag";
            sphereShader->buildFromFile(solidVertPath, solidFragPath);

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
            buildSphereMesh();

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

        void Renderer::buildSphereMesh()
        {
            const int stacks = 16;
            const int slices = 32;
            std::vector<glm::vec3> positions;
            std::vector<uint32_t> indices;

            positions.reserve((stacks + 1) * (slices + 1));
            for (int i = 0; i <= stacks; i++)
            {
                float v = static_cast<float>(i) / static_cast<float>(stacks);
                float phi = v * kPi;
                float sinPhi = std::sin(phi);
                float cosPhi = std::cos(phi);

                for (int j = 0; j <= slices; j++)
                {
                    float u = static_cast<float>(j) / static_cast<float>(slices);
                    float theta = u * kPi * 2.0f;
                    float sinTheta = std::sin(theta);
                    float cosTheta = std::cos(theta);

                    positions.emplace_back(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta);
                }
            }

            indices.reserve(stacks * slices * 6);
            for (int i = 0; i < stacks; i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    uint32_t first = i * (slices + 1) + j;
                    uint32_t second = first + slices + 1;
                    indices.push_back(first);
                    indices.push_back(second);
                    indices.push_back(first + 1);
                    indices.push_back(second);
                    indices.push_back(second + 1);
                    indices.push_back(first + 1);
                }
            }

            sphereIndexCount = static_cast<uint32_t>(indices.size());

            glGenVertexArrays(1, &sphereVAO);
            glGenBuffers(1, &sphereVBO);
            glGenBuffers(1, &sphereEBO);

            glBindVertexArray(sphereVAO);
            glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
            glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
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

            shader->use();
            shader->setMat4("view", Glb::Camera::getInstance().GetView());
            shader->setMat4("projection", Glb::Camera::getInstance().GetProjection());
            shader->setFloat("scale", ps.scale);

            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, particleNum);
            shader->unUse();

            if (Lagrangian3dPara::enableMovingSphere && sphereIndexCount > 0)
            {
                float time = Lagrangian3dPara::movingSphereTime;
                float omega = 2.0f * kPi * Lagrangian3dPara::movingSphereFrequency;
                float phase = std::sin(omega * time);
                glm::vec3 baseCenter = Lagrangian3dPara::movingSphereCenter * Lagrangian3dPara::scale;
                glm::vec3 amplitude = Lagrangian3dPara::movingSphereAmplitude * Lagrangian3dPara::scale;
                glm::vec3 center = baseCenter + amplitude * phase;
                float radius = Lagrangian3dPara::movingSphereRadius * Lagrangian3dPara::scale;

                glm::mat4 model(1.0f);
                model = glm::translate(model, center);
                model = glm::scale(model, glm::vec3(radius));

                sphereShader->use();
                sphereShader->setMat4("view", Glb::Camera::getInstance().GetView());
                sphereShader->setMat4("projection", Glb::Camera::getInstance().GetProjection());
                sphereShader->setMat4("model", model);

                glBindVertexArray(sphereVAO);
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
                sphereShader->unUse();
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
