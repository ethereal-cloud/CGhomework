#include "ParticleSystem2d.h"
#include <iostream>
#include "Global.h"
#include <unordered_set>

namespace FluidSimulation
{

    namespace Lagrangian2d
    {
        ParticleSystem2d::ParticleSystem2d()
        {
        }

        ParticleSystem2d::~ParticleSystem2d()
        {
        }

        // 设置流体容器的大小
        void ParticleSystem2d::setContainerSize(glm::vec2 lower = glm::vec2(-1.0f, -1.0f), glm::vec2 upper = glm::vec2(1.0f, 1.0f))
        {
            // 应用缩放
            lower *=  Lagrangian2dPara::scale;
            upper *= Lagrangian2dPara::scale;

            // 设置边界,考虑支持半径和粒子直径
            lowerBound = lower - supportRadius + particleDiameter;
            upperBound = upper + supportRadius - particleDiameter;
            containerCenter = (lowerBound + upperBound) / 2.0f;

            glm::vec2 size = upperBound - lowerBound;

            // 计算块的数量和大小
            blockNum.x = floor(size.x / supportRadius);
            blockNum.y = floor(size.y / supportRadius);
            blockSize = glm::vec2(size.x / blockNum.x, size.y / blockNum.y);

            // 初始化块偏移数组
            blockIdOffs.resize(9);
            int p = 0;
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    blockIdOffs[p] = blockNum.x * j + i;
                    p++;
                }
            }

            // 清空粒子数组
            particles.clear();
        }

        // 添加流体块
        int ParticleSystem2d::addFluidBlock(glm::vec2 lowerCorner, glm::vec2 upperCorner, glm::vec2 v0, float particleSpace)
        {
            // 应用缩放
            lowerCorner *= Lagrangian2dPara::scale;
            upperCorner *= Lagrangian2dPara::scale;

            glm::vec2 size = upperCorner - lowerCorner;

            // 检查边界
            if (lowerCorner.x < lowerBound.x ||
                lowerCorner.y < lowerBound.y ||
                upperCorner.x > upperBound.x ||
                upperCorner.y > upperBound.y)
            {
                return 0;
            }

            // 计算粒子数量
            glm::uvec2 particleNum = glm::uvec2(size.x / particleSpace, size.y / particleSpace);
            std::vector<ParticleInfo2d> tempParticles(particleNum.x * particleNum.y);

            // 随机生成器,用于粒子位置的扰动
            Glb::RandomGenerator rand;
            int p = 0;
            // 在流体块中生成粒子
            for (int idX = 0; idX < particleNum.x; idX++)
            {
                for (int idY = 0; idY < particleNum.y; idY++)
                {
                    // 添加随机扰动避免规则排列
                    float x = (idX + rand.GetUniformRandom()) * particleSpace;
                    float y = (idY + rand.GetUniformRandom()) * particleSpace;

                    // 设置粒子属性
                    tempParticles[p].position = lowerCorner + glm::vec2(x, y);
                    tempParticles[p].blockId = getBlockIdByPosition(tempParticles[p].position);
                    tempParticles[p].density = Lagrangian2dPara::density;
                    tempParticles[p].velocity = v0;
                    p++;
                }
            }

            // 将新粒子添加到系统中
            particles.insert(particles.end(), tempParticles.begin(), tempParticles.end());
            return particles.size();
        }

        // 根据位置获取块ID
        uint32_t ParticleSystem2d::getBlockIdByPosition(glm::vec2 position)
        {
            // 检查边界
            if (position.x < lowerBound.x ||
                position.y < lowerBound.y ||
                position.x > upperBound.x ||
                position.y > upperBound.y)
            {
                return -1;
            }

            // 计算块索引
            glm::vec2 deltePos = position - lowerBound;
            uint32_t c = floor(deltePos.x / blockSize.x);
            uint32_t r = floor(deltePos.y / blockSize.y);
            return r * blockNum.x + c;
        }

        // 更新块信息
        void ParticleSystem2d::updateBlockInfo()
        {
            // 根据块ID对粒子进行排序
            std::sort(particles.begin(), particles.end(),
                      [=](ParticleInfo2d &first, ParticleInfo2d &second)
                      {
                          return first.blockId < second.blockId;
                      });

            // 更新每个块中粒子的范围
            blockExtens = std::vector<glm::uvec2>(blockNum.x * blockNum.y, glm::uvec2(0, 0));
            int curBlockId = 0;
            int left = 0;
            int right;
            for (right = 0; right < particles.size(); right++)
            {
                if (particles[right].blockId != curBlockId)
                {
                    blockExtens[curBlockId] = glm::uvec2(left, right);
                    left = right;
                    curBlockId = particles[right].blockId;
                }
            }
            blockExtens[curBlockId] = glm::uvec2(left, right);
        }

        // ==================== 喷泉功能实现 ====================

        /**
         * 发射喷泉粒子
         *
         * 实现原理：
         * 1. 根据发射率和时间步长计算本帧应发射的粒子数
         * 2. 使用累加器处理小数部分，确保发射率准确
         * 3. 在喷口位置（加随机扰动）生成新粒子
         * 4. 为粒子赋予向上的初速度（加随机扰动模拟喷射散开）
         * 5. 检查粒子数量上限，防止内存爆炸
         */
        void ParticleSystem2d::emitFountainParticles(float dt)
        {
            // 检查是否启用喷泉
            if (!Lagrangian2dPara::enableFountain) return;

            // 检查粒子数量上限
            if (particles.size() >= Lagrangian2dPara::maxParticles) return;

            // 计算本帧应发射的粒子数
            // 使用静态累加器处理小数部分
            static float emissionAccumulator = 0.0f;
            emissionAccumulator += Lagrangian2dPara::emissionRate * dt;

            int numToEmit = (int)emissionAccumulator;
            emissionAccumulator -= numToEmit;

            // 限制单帧最大发射数，防止卡顿
            numToEmit = (std::min)(numToEmit, 50);

            // 限制总粒子数
            int remainingCapacity = Lagrangian2dPara::maxParticles - (int)particles.size();
            numToEmit = (std::min)(numToEmit, remainingCapacity);

            if (numToEmit <= 0) return;

            // 获取喷泉参数（应用缩放）
            glm::vec2 fountainPos = Lagrangian2dPara::fountainPosition * Lagrangian2dPara::scale;
            glm::vec2 fountainVel = Lagrangian2dPara::fountainVelocity;
            float spread = Lagrangian2dPara::fountainSpread * Lagrangian2dPara::scale;

            // 随机数生成器
            Glb::RandomGenerator rand;

            // 发射粒子
            for (int i = 0; i < numToEmit; i++)
            {
                ParticleInfo2d newParticle;

                // 位置：在喷口宽度范围内随机分布
                float offsetX = (rand.GetUniformRandom() - 0.5f) * spread;
                newParticle.position = fountainPos + glm::vec2(offsetX, 0.0f);

                // 检查位置是否在容器内
                if (newParticle.position.x < lowerBound.x ||
                    newParticle.position.x > upperBound.x ||
                    newParticle.position.y < lowerBound.y ||
                    newParticle.position.y > upperBound.y)
                {
                    continue;  // 跳过越界粒子
                }

                // 速度：向上喷射，加少量随机扰动形成扇形
                float velVariation = 0.1f;  // 速度变化幅度
                float vx = fountainVel.x + (rand.GetUniformRandom() - 0.5f) * velVariation;
                float vy = fountainVel.y * (0.9f + rand.GetUniformRandom() * 0.2f);
                newParticle.velocity = glm::vec2(vx, vy);

                // 初始化其他属性
                newParticle.accleration = glm::vec2(0.0f);
                newParticle.density = Lagrangian2dPara::density;
                newParticle.pressure = 0.0f;
                newParticle.pressDivDens2 = 0.0f;
                newParticle.blockId = getBlockIdByPosition(newParticle.position);

                // 添加到粒子系统
                particles.push_back(newParticle);
            }
        }
    }
}
