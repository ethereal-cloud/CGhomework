/**
 * Solver.cpp: 2D拉格朗日流体求解器实现文件
 * 实现基于SPH (Smoothed Particle Hydrodynamics) 的2D流体仿真算法
 *
 * SPH 算法概述：
 * ==============
 * SPH 是一种拉格朗日粒子方法，将流体离散为一系列粒子。
 * 每个粒子携带质量、位置、速度等物理量。
 * 通过核函数对邻居粒子的贡献进行加权求和，来估计连续物理场。
 *
 * 主要步骤：
 * 1. 密度计算：使用 Poly6 核函数
 * 2. 压力计算：使用 Tait 状态方程
 * 3. 加速度计算：压力梯度(Spiky核) + 粘性力(Viscosity核) + 重力
 * 4. 时间积分：半隐式欧拉方法
 * 5. 边界处理：反弹碰撞
 */

#include "Lagrangian/include/Solver.h"
#include "Global.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// 数学常量
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        /**
         * 构造函数，保存对粒子系统的引用
         * @param ps 粒子系统引用
         */
        Solver::Solver(ParticleSystem2d &ps) : mPs(ps)
        {
        }

        // ==================== SPH 核函数实现 ====================

        /**
         * Poly6 核函数 - 用于密度计算 (2D 版本)
         *
         * 数学公式 (2D)：
         *   W(r,h) = 4/(πh⁸) × (h²-r²)³   当 0 ≤ r ≤ h
         *   W(r,h) = 0                     当 r > h
         *
         * 注意：3D 版本的系数是 315/(64πh⁹)，2D 不同！
         *
         * 为什么用 Poly6？
         * - 计算高效：输入 r² 可避免开方
         * - 在 r=0 处有最大值，适合密度计算
         * - 光滑且连续
         *
         * @param r2 两粒子距离的平方 |r_i - r_j|²
         * @return 核函数值 W(r,h)
         */
        float Solver::poly6(float r2)
        {
            float h = mPs.supportRadius;
            float h2 = h * h;

            // 超出支持半径，返回0
            if (r2 >= h2) return 0.0f;

            // 计算 (h² - r²)³
            float diff = h2 - r2;
            float diff3 = diff * diff * diff;

            // 2D 系数: 4 / (π × h⁸)
            float h8 = h2 * h2 * h2 * h2;  // h⁸
            float coeff = 4.0f / (M_PI * h8);

            return coeff * diff3;
        }

        /**
         * Spiky 核函数的梯度 - 用于压力梯度计算 (2D 版本)
         *
         * 数学公式 (2D)：
         *   ∇W(r,h) = -30/(πh⁵) × (h-|r|)² × (r/|r|)   当 0 < r ≤ h
         *   ∇W(r,h) = 0                                 当 r > h 或 r = 0
         *
         * 注意：3D 版本的系数是 -45/(πh⁶)，2D 不同！
         *
         * 为什么用 Spiky 而不是 Poly6 的梯度？
         * - Poly6 的梯度在 r→0 时趋于0，导致粒子靠近时排斥力消失
         * - Spiky 的梯度在 r→0 时趋于无穷，能产生足够的排斥力防止粒子聚集
         *
         * @param r 从粒子j指向粒子i的向量 (r_i - r_j)
         * @return 核函数梯度向量 ∇W
         */
        glm::vec2 Solver::spikyGrad(glm::vec2 r)
        {
            float h = mPs.supportRadius;
            float rLen = glm::length(r);

            // 超出支持半径或距离为0，返回零向量
            if (rLen >= h || rLen < 1e-6f) return glm::vec2(0.0f);

            // 计算 (h - |r|)²
            float diff = h - rLen;
            float diff2 = diff * diff;

            // 2D 系数: -30 / (π × h⁵)
            float h5 = h * h * h * h * h;
            float coeff = -30.0f / (M_PI * h5);

            // 方向：从 j 指向 i 的单位向量
            glm::vec2 direction = r / rLen;

            return coeff * diff2 * direction;
        }

        /**
         * 粘性核函数的拉普拉斯 - 用于粘性力计算 (2D 版本)
         *
         * 数学公式 (2D)：
         *   ∇²W(r,h) = 40/(πh⁵) × (h-r)   当 0 ≤ r ≤ h
         *   ∇²W(r,h) = 0                   当 r > h
         *
         * 注意：3D 版本的系数是 45/(πh⁶)，2D 不同！
         *
         * 为什么用这个核？
         * - 粘性力公式需要速度场的拉普拉斯
         * - 这个核的拉普拉斯总是正的，保证粘性力的耗散性质
         *
         * @param r 两粒子之间的距离 |r_i - r_j|
         * @return 核函数拉普拉斯值 ∇²W
         */
        float Solver::viscosityLaplacian(float r)
        {
            float h = mPs.supportRadius;

            // 超出支持半径，返回0
            if (r >= h) return 0.0f;

            // 2D 系数: 40 / (π × h⁵)
            float h5 = h * h * h * h * h;
            float coeff = 40.0f / (M_PI * h5);

            return coeff * (h - r);
        }

        // ==================== 主求解函数 ====================

        /**
         * 求解流体方程 - SPH 算法的主函数
         *
         * 每个时间步执行以下操作：
         * 0. [新增] 发射喷泉粒子（如果启用）
         * 1. 计算每个粒子的密度（邻居贡献求和）
         * 2. 根据密度计算压力（状态方程）
         * 3. 计算加速度（压力梯度 + 粘性力 + 重力）
         * 4. 更新速度和位置（时间积分）
         * 5. 处理边界碰撞
         * 6. 更新粒子的空间索引
         */
        void Solver::solve()
        {
            // 获取参数
            float h = mPs.supportRadius;
            float h2 = h * h;
            float rho0 = Lagrangian2dPara::density;           // 参考密度 (1000 kg/m³)
            float stiffness = Lagrangian2dPara::stiffness;    // 刚度系数
            float exponent = Lagrangian2dPara::exponent;      // 压力指数
            float viscosity = Lagrangian2dPara::viscosity;    // 粘度系数
            float dt = Lagrangian2dPara::dt;                  // 时间步长
            glm::vec2 gravity(Lagrangian2dPara::gravityX,
                             -Lagrangian2dPara::gravityY);    // 重力（Y轴向下为负）

            // ==================== 步骤 0: 发射喷泉粒子 ====================
            //
            // 如果启用喷泉模式，在每个时间步开始时发射新粒子
            // 新粒子会被赋予向上的初速度，然后参与后续的SPH计算
            //
            mPs.emitFountainParticles(dt);

            // 粒子质量：假设初始时每个粒子占据 particleDiameter² 的面积，密度为 rho0
            float particleMass = rho0 * mPs.particleVolume;

            int numParticles = mPs.particles.size();

            // ==================== 步骤 1: 计算密度 ====================
            //
            // SPH 密度估计公式：
            //   ρ_i = Σ_j m_j × W(|r_i - r_j|, h)
            //
            // 即：粒子 i 的密度 = 所有邻居粒子 j 的质量 × 核函数值 的总和
            //
            for (int i = 0; i < numParticles; i++)
            {
                ParticleInfo2d &pi = mPs.particles[i];
                float density = 0.0f;

                // 遍历相邻的 9 个块（包括自己所在的块）
                for (int offset : mPs.blockIdOffs)
                {
                    int neighborBlockId = pi.blockId + offset;

                    // 检查块 ID 是否有效
                    if (neighborBlockId < 0 ||
                        neighborBlockId >= mPs.blockExtens.size())
                        continue;

                    // 获取该块中粒子的范围 [start, end)
                    glm::uvec2 extent = mPs.blockExtens[neighborBlockId];

                    // 遍历该块中的所有粒子
                    for (uint32_t j = extent.x; j < extent.y; j++)
                    {
                        ParticleInfo2d &pj = mPs.particles[j];

                        // 计算距离的平方
                        glm::vec2 r = pi.position - pj.position;
                        float r2 = glm::dot(r, r);

                        // 在支持半径内才计算
                        if (r2 < h2)
                        {
                            density += particleMass * poly6(r2);
                        }
                    }
                }

                // 保存密度（设置下限防止除零）
                // 注意：用括号包裹 std::max 防止与 Configure.h 中的 max 宏冲突
                pi.density = (std::max)(density, rho0 * 0.01f);
            }

            // ==================== 步骤 2: 计算压力 ====================
            //
            // 使用 Tait 状态方程（适用于弱可压缩流体）：
            //   p = stiffness × ((ρ/ρ₀)^exponent - 1)
            //
            // - 当 ρ > ρ₀ 时，p > 0，产生排斥力
            // - 当 ρ < ρ₀ 时，p < 0，产生吸引力
            // - exponent 通常取 7（水）
            //
            for (int i = 0; i < numParticles; i++)
            {
                ParticleInfo2d &pi = mPs.particles[i];

                // Tait 状态方程
                float rhoRatio = pi.density / rho0;
                pi.pressure = stiffness * (std::pow(rhoRatio, exponent) - 1.0f);

                // 预计算 pressure / density²，用于压力梯度计算
                pi.pressDivDens2 = pi.pressure / (pi.density * pi.density);
            }

            // ==================== 步骤 3: 计算加速度 ====================
            //
            // 加速度由三部分组成：
            // a = a_pressure + a_viscosity + a_gravity
            //
            // 压力加速度（Navier-Stokes 方程的压力项）：
            //   a_pressure = -∇p/ρ = -Σ_j m_j × (p_i/ρ_i² + p_j/ρ_j²) × ∇W(r_i - r_j)
            //
            // 粘性加速度（Navier-Stokes 方程的粘性项）：
            //   a_viscosity = μ∇²v/ρ = μ × Σ_j m_j × (v_j - v_i)/ρ_j × ∇²W(|r_i - r_j|)
            //
            // 重力加速度：
            //   a_gravity = g
            //
            for (int i = 0; i < numParticles; i++)
            {
                ParticleInfo2d &pi = mPs.particles[i];
                glm::vec2 pressureAcc(0.0f);
                glm::vec2 viscosityAcc(0.0f);

                // 遍历相邻块
                for (int offset : mPs.blockIdOffs)
                {
                    int neighborBlockId = pi.blockId + offset;

                    if (neighborBlockId < 0 ||
                        neighborBlockId >= mPs.blockExtens.size())
                        continue;

                    glm::uvec2 extent = mPs.blockExtens[neighborBlockId];

                    for (uint32_t j = extent.x; j < extent.y; j++)
                    {
                        // 跳过自己
                        if (i == j) continue;

                        ParticleInfo2d &pj = mPs.particles[j];

                        glm::vec2 r = pi.position - pj.position;
                        float rLen = glm::length(r);

                        if (rLen < h && rLen > 1e-6f)
                        {
                            // 压力梯度力
                            // 使用对称形式保证动量守恒：(p_i/ρ_i² + p_j/ρ_j²)
                            glm::vec2 gradW = spikyGrad(r);
                            float pressureTerm = pi.pressDivDens2 + pj.pressDivDens2;
                            pressureAcc -= particleMass * pressureTerm * gradW;

                            // 粘性力
                            float lapW = viscosityLaplacian(rLen);
                            glm::vec2 velDiff = pj.velocity - pi.velocity;
                            viscosityAcc += particleMass * velDiff / pj.density * lapW;
                        }
                    }
                }

                // 合成总加速度
                pi.accleration = pressureAcc + viscosity * viscosityAcc + gravity;
            }

            // ==================== 步骤 4: 时间积分 ====================
            //
            // 使用半隐式欧拉方法（Symplectic Euler）：
            //   v_new = v_old + a × dt    （先更新速度）
            //   x_new = x_old + v_new × dt （用新速度更新位置）
            //
            // 半隐式欧拉比显式欧拉更稳定，能量守恒性更好
            //
            float maxVel = Lagrangian2dPara::maxVelocity;

            for (int i = 0; i < numParticles; i++)
            {
                ParticleInfo2d &pi = mPs.particles[i];

                // 更新速度
                pi.velocity += pi.accleration * dt;

                // 限制最大速度（防止数值爆炸）
                float velMag = glm::length(pi.velocity);
                if (velMag > maxVel)
                {
                    pi.velocity = pi.velocity / velMag * maxVel;
                }

                // 更新位置
                pi.position += pi.velocity * dt;
            }

            // ==================== 步骤 5: 边界碰撞处理 ====================
            //
            // 当粒子越过边界时：
            // 1. 将位置钳制到边界内
            // 2. 反转并衰减该方向的速度（模拟能量损失）
            //
            float eps = Lagrangian2dPara::eps;
            float attenuation = Lagrangian2dPara::velocityAttenuation;

            for (int i = 0; i < numParticles; i++)
            {
                ParticleInfo2d &pi = mPs.particles[i];

                // 左边界
                if (pi.position.x < mPs.lowerBound.x + eps)
                {
                    pi.position.x = mPs.lowerBound.x + eps;
                    pi.velocity.x *= -attenuation;
                }
                // 右边界
                if (pi.position.x > mPs.upperBound.x - eps)
                {
                    pi.position.x = mPs.upperBound.x - eps;
                    pi.velocity.x *= -attenuation;
                }
                // 下边界
                if (pi.position.y < mPs.lowerBound.y + eps)
                {
                    pi.position.y = mPs.lowerBound.y + eps;
                    pi.velocity.y *= -attenuation;
                }
                // 上边界
                if (pi.position.y > mPs.upperBound.y - eps)
                {
                    pi.position.y = mPs.upperBound.y - eps;
                    pi.velocity.y *= -attenuation;
                }
            }

            // ==================== 步骤 6: 更新空间索引 ====================
            //
            // 粒子位置改变后，需要重新计算它们所属的块
            // （updateBlockInfo 会在下一帧 simulate() 开始时调用，这里只更新 blockId）
            //
            for (int i = 0; i < numParticles; i++)
            {
                mPs.particles[i].blockId = mPs.getBlockIdByPosition(mPs.particles[i].position);
            }
        }
    }
}
