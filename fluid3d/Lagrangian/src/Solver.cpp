/**
 * Solver.cpp: 3D拉格朗日流体求解器实现文件
 * 实现基于SPH (Smoothed Particle Hydrodynamics) 的3D流体仿真算法
 *
 * SPH 算法概述：
 * ==============
 * SPH 是一种拉格朗日粒子方法，将流体离散为一系列粒子。
 * 每个粒子携带质量、位置、速度等物理量。
 * 通过核函数对邻居粒子的贡献进行加权求和，来估计连续物理场。
 *
 * 3D 与 2D 版本的区别：
 * - 向量从 vec2 变为 vec3
 * - 邻居块从 9 个变为 27 个
 * - 核函数系数不同（详见各核函数注释）
 *
 * 主要步骤：
 * 1. 密度计算：使用 Poly6 核函数
 * 1b. 颜色场计算：用于表面张力（可选）
 * 2. 压力计算：使用 Tait 状态方程
 * 3. 加速度计算：压力梯度(Spiky核) + 粘性力(Viscosity核) + 重力 + 表面张力（可选）
 * 4. 时间积分：半隐式欧拉方法
 * 5. 边界处理：反弹碰撞
 *
 * 表面张力实现：
 * 基于 Müller et al. (2003) 的 SPH 表面张力模型
 * 通过颜色场计算曲率，在流体表面施加最小化表面积的力
 */

#include "fluid3d/Lagrangian/include/Solver.h"
#include "Global.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// OpenMP支持
#ifdef _OPENMP
#include <omp.h>
#endif

// 数学常量
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace FluidSimulation
{
    namespace Lagrangian3d
    {
        namespace
        {
            void resolveCylinderCollision(particle3d &pi,
                                          const glm::vec3 &a,
                                          const glm::vec3 &b,
                                          float radius,
                                          const glm::vec3 &bodyVelocity,
                                          float attenuation)
            {
                glm::vec3 ab = b - a;
                float abLen2 = glm::dot(ab, ab);
                if (abLen2 < 1e-8f)
                {
                    return;
                }

                float t = glm::dot(pi.position - a, ab) / abLen2;
                if (t < 0.0f)
                {
                    t = 0.0f;
                }
                else if (t > 1.0f)
                {
                    t = 1.0f;
                }

                glm::vec3 closest = a + ab * t;
                glm::vec3 delta = pi.position - closest;
                float dist2 = glm::dot(delta, delta);
                float radius2 = radius * radius;
                if (dist2 >= radius2)
                {
                    return;
                }

                float dist = std::sqrt(dist2);
                glm::vec3 normal = (dist > 1e-6f) ? (delta / dist) : glm::vec3(0.0f, 1.0f, 0.0f);
                pi.position = closest + normal * radius;

                glm::vec3 relVel = pi.velocity - bodyVelocity;
                float vn = glm::dot(relVel, normal);
                if (vn < 0.0f)
                {
                    relVel -= (1.0f + attenuation) * vn * normal;
                }
                pi.velocity = relVel + bodyVelocity;
            }
        }

        /**
         * 构造函数，保存对粒子系统的引用
         * @param ps 粒子系统引用
         */
        Solver::Solver(ParticleSystem3d &ps) : mPs(ps)
        {
        }

        // ==================== SPH 核函数实现 (3D 版本) ====================

        /**
         * Poly6 核函数 - 用于密度计算 (3D 版本)
         *
         * 数学公式 (3D)：
         *   W(r,h) = 315/(64πh⁹) × (h²-r²)³   当 0 ≤ r ≤ h
         *   W(r,h) = 0                         当 r > h
         *
         * 注意：2D 版本的系数是 4/(πh⁸)，3D 不同！
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

            // 3D 系数: 315 / (64 × π × h⁹)
            float h9 = h2 * h2 * h2 * h2 * h;  // h⁹
            float coeff = 315.0f / (64.0f * M_PI * h9);

            return coeff * diff3;
        }

        /**
         * Spiky 核函数的梯度 - 用于压力梯度计算 (3D 版本)
         *
         * 数学公式 (3D)：
         *   ∇W(r,h) = -45/(πh⁶) × (h-|r|)² × (r/|r|)   当 0 < r ≤ h
         *   ∇W(r,h) = 0                                 当 r > h 或 r = 0
         *
         * 注意：2D 版本的系数是 -30/(πh⁵)，3D 不同！
         *
         * 为什么用 Spiky 而不是 Poly6 的梯度？
         * - Poly6 的梯度在 r→0 时趋于0，导致粒子靠近时排斥力消失
         * - Spiky 的梯度在 r→0 时趋于无穷，能产生足够的排斥力防止粒子聚集
         *
         * @param r 从粒子j指向粒子i的向量 (r_i - r_j)
         * @return 核函数梯度向量 ∇W
         */
        glm::vec3 Solver::spikyGrad(glm::vec3 r)
        {
            float h = mPs.supportRadius;
            float rLen = glm::length(r);

            // 超出支持半径或距离为0，返回零向量
            if (rLen >= h || rLen < 1e-6f) return glm::vec3(0.0f);

            // 计算 (h - |r|)²
            float diff = h - rLen;
            float diff2 = diff * diff;

            // 3D 系数: -45 / (π × h⁶)
            float h6 = h * h * h * h * h * h;
            float coeff = -45.0f / (M_PI * h6);

            // 方向：从 j 指向 i 的单位向量
            glm::vec3 direction = r / rLen;

            return coeff * diff2 * direction;
        }

        /**
         * 粘性核函数的拉普拉斯 - 用于粘性力计算 (3D 版本)
         *
         * 数学公式 (3D)：
         *   ∇²W(r,h) = 45/(πh⁶) × (h-r)   当 0 ≤ r ≤ h
         *   ∇²W(r,h) = 0                   当 r > h
         *
         * 注意：2D 版本的系数是 40/(πh⁵)，3D 不同！
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

            // 3D 系数: 45 / (π × h⁶)
            float h6 = h * h * h * h * h * h;
            float coeff = 45.0f / (M_PI * h6);

            return coeff * (h - r);
        }

        // ==================== 主求解函数 ====================

        /**
         * 求解流体方程 - SPH 算法的主函数
         *
         * 每个时间步执行以下操作：
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
            float rho0 = Lagrangian3dPara::density;           // 参考密度 (1000 kg/m³)
            float stiffness = Lagrangian3dPara::stiffness;    // 刚度系数
            float exponent = Lagrangian3dPara::exponent;      // 压力指数
            float viscosity = Lagrangian3dPara::viscosity;    // 粘度系数
            float dt = Lagrangian3dPara::dt;                  // 时间步长
            glm::vec3 gravity(Lagrangian3dPara::gravityX,
                             Lagrangian3dPara::gravityY,
                             Lagrangian3dPara::gravityZ);     // 重力

            // 搅拌棒参数
            Lagrangian3dPara::stirrerTime += dt;
            float stirrerTime = Lagrangian3dPara::stirrerTime;
            bool useStirrer = Lagrangian3dPara::enableStirrer;
            glm::vec3 stirrerCenter(0.0f);
            glm::vec3 stirrerVelocity(0.0f);
            glm::vec3 barAxis(1.0f, 0.0f, 0.0f);
            float rodRadius = 0.0f;
            float rodZMin = 0.0f;
            float rodZMax = 0.0f;
            float barLength = 0.0f;
            float barZ = 0.0f;
            if (useStirrer)
            {
                float omega = 2.0f * static_cast<float>(M_PI) * Lagrangian3dPara::stirrerFrequency;
                float theta = omega * stirrerTime;
                float orbitRadius = Lagrangian3dPara::stirrerOrbitRadius * Lagrangian3dPara::scale;
                glm::vec3 baseCenter = Lagrangian3dPara::stirrerCenter * Lagrangian3dPara::scale;
                stirrerCenter = baseCenter + glm::vec3(orbitRadius * std::cos(theta), orbitRadius * std::sin(theta), 0.0f);
                stirrerVelocity = glm::vec3(-orbitRadius * omega * std::sin(theta),
                                            orbitRadius * omega * std::cos(theta),
                                            0.0f);

                barAxis = glm::vec3(std::cos(theta), std::sin(theta), 0.0f);
                rodRadius = Lagrangian3dPara::stirrerRodRadius * Lagrangian3dPara::scale;
                rodZMin = Lagrangian3dPara::stirrerRodZMin * Lagrangian3dPara::scale;
                rodZMax = Lagrangian3dPara::stirrerRodZMax * Lagrangian3dPara::scale;
                barLength = Lagrangian3dPara::stirrerBarLength * Lagrangian3dPara::scale;
                barZ = Lagrangian3dPara::stirrerBarZ * Lagrangian3dPara::scale;
            }

            // 粒子质量：假设初始时每个粒子占据 particleDiameter³ 的体积，密度为 rho0
            float particleMass = rho0 * mPs.particleVolume;

            int numParticles = mPs.particles.size();

            // ==================== 步骤 1: 计算密度 ====================
            //
            // SPH 密度估计公式：
            //   ρ_i = Σ_j m_j × W(|r_i - r_j|, h)
            //
            // 即：粒子 i 的密度 = 所有邻居粒子 j 的质量 × 核函数值 的总和
            //
            #pragma omp parallel for
            for (int i = 0; i < numParticles; i++)
            {
                particle3d &pi = mPs.particles[i];
                float density = 0.0f;

                // 遍历相邻的 27 个块（3D: 3×3×3，包括自己所在的块）
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
                        particle3d &pj = mPs.particles[j];

                        // 计算距离的平方
                        glm::vec3 r = pi.position - pj.position;
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

            // ==================== 步骤 1b: 计算颜色场（表面张力用） ====================
            //
            // 表面张力需要识别流体表面，通过颜色场来标记：
            //   C_i = Σ_j (m_j / ρ_j) × W(r_i - r_j)
            // 颜色场梯度：∇C_i = Σ_j (m_j / ρ_j) × ∇W(r_i - r_j)
            // 颜色场拉普拉斯：∇²C_i = Σ_j (m_j / ρ_j) × ∇²W(|r_i - r_j|)
            //
            bool computeSurfaceTension = Lagrangian3dPara::enableSurfaceTension;
            
            if (computeSurfaceTension)
            {
                #pragma omp parallel for
                for (int i = 0; i < numParticles; i++)
                {
                    particle3d &pi = mPs.particles[i];
                    glm::vec3 colorGrad(0.0f);
                    float colorLap = 0.0f;

                    for (int offset : mPs.blockIdOffs)
                    {
                        int neighborBlockId = pi.blockId + offset;
                        if (neighborBlockId < 0 || neighborBlockId >= mPs.blockExtens.size())
                            continue;

                        glm::uvec2 extent = mPs.blockExtens[neighborBlockId];
                        for (uint32_t j = extent.x; j < extent.y; j++)
                        {
                            if (i == j) continue;
                            particle3d &pj = mPs.particles[j];
                            glm::vec3 r = pi.position - pj.position;
                            float rLen = glm::length(r);

                            if (rLen < h && rLen > 1e-6f)
                            {
                                float weight = particleMass / pj.density;
                                colorGrad += weight * spikyGrad(r);
                                colorLap += weight * viscosityLaplacian(rLen);
                            }
                        }
                    }
                    pi.colorGradient = colorGrad;
                    pi.colorLaplacian = colorLap;
                }
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
            #pragma omp parallel for
            for (int i = 0; i < numParticles; i++)
            {
                particle3d &pi = mPs.particles[i];

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
            #pragma omp parallel for
            for (int i = 0; i < numParticles; i++)
            {
                particle3d &pi = mPs.particles[i];
                glm::vec3 pressureAcc(0.0f);
                glm::vec3 viscosityAcc(0.0f);

                // 遍历相邻块（27个）
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

                        particle3d &pj = mPs.particles[j];

                        glm::vec3 r = pi.position - pj.position;
                        float rLen = glm::length(r);

                        if (rLen < h && rLen > 1e-6f)
                        {
                            // 压力梯度力
                            // 使用对称形式保证动量守恒：(p_i/ρ_i² + p_j/ρ_j²)
                            glm::vec3 gradW = spikyGrad(r);
                            float pressureTerm = pi.pressDivDens2 + pj.pressDivDens2;
                            pressureAcc -= particleMass * pressureTerm * gradW;

                            // 粘性力
                            float lapW = viscosityLaplacian(rLen);
                            glm::vec3 velDiff = pj.velocity - pi.velocity;
                            viscosityAcc += particleMass * velDiff / pj.density * lapW;
                        }
                    }
                }

                // 合成总加速度：压力 + 粘性 + 重力
                pi.accleration = pressureAcc + viscosity * viscosityAcc + gravity;

                // 表面张力加速度（如果启用）
                if (computeSurfaceTension)
                {
                    float gradLen = glm::length(pi.colorGradient);
                    float threshold = Lagrangian3dPara::surfaceThreshold;
                    
                    if (gradLen > threshold)
                    {
                        glm::vec3 normal = pi.colorGradient / gradLen;
                        float curvature = -pi.colorLaplacian / gradLen;
                        float sigma = Lagrangian3dPara::surfaceTension;
                        glm::vec3 surfaceTensionForce = -sigma * curvature * normal;
                        pi.accleration += surfaceTensionForce / pi.density;
                    }
                }
            }

            // ==================== 步骤 4: 时间积分 ====================
            //
            // 使用半隐式欧拉方法（Symplectic Euler）：
            //   v_new = v_old + a × dt    （先更新速度）
            //   x_new = x_old + v_new × dt （用新速度更新位置）
            //
            // 半隐式欧拉比显式欧拉更稳定，能量守恒性更好
            //
            float maxVel = Lagrangian3dPara::maxVelocity;

            #pragma omp parallel for
            for (int i = 0; i < numParticles; i++)
            {
                particle3d &pi = mPs.particles[i];

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
            float eps = Lagrangian3dPara::eps;
            float attenuation = Lagrangian3dPara::velocityAttenuation;

            #pragma omp parallel for
            for (int i = 0; i < numParticles; i++)
            {
                particle3d &pi = mPs.particles[i];

                // 搅拌棒碰撞
                if (useStirrer && rodRadius > 0.0f)
                {
                    float combinedRadius = rodRadius + mPs.particleRadius * 0.05f;
                    if (rodZMax > rodZMin)
                    {
                        glm::vec3 a(stirrerCenter.x, stirrerCenter.y, rodZMin);
                        glm::vec3 b(stirrerCenter.x, stirrerCenter.y, rodZMax);
                        resolveCylinderCollision(pi, a, b, combinedRadius, stirrerVelocity, attenuation);
                    }
                    if (barLength > 0.0f)
                    {
                        glm::vec3 barCenter(stirrerCenter.x, stirrerCenter.y, barZ);
                        glm::vec3 halfAxis = barAxis * (barLength * 0.5f);
                        glm::vec3 a = barCenter - halfAxis;
                        glm::vec3 b = barCenter + halfAxis;
                        resolveCylinderCollision(pi, a, b, combinedRadius, stirrerVelocity, attenuation);
                    }
                }

                // X 方向边界
                if (pi.position.x < mPs.lowerBound.x + eps)
                {
                    pi.position.x = mPs.lowerBound.x + eps;
                    pi.velocity.x *= -attenuation;
                }
                if (pi.position.x > mPs.upperBound.x - eps)
                {
                    pi.position.x = mPs.upperBound.x - eps;
                    pi.velocity.x *= -attenuation;
                }

                // Y 方向边界
                if (pi.position.y < mPs.lowerBound.y + eps)
                {
                    pi.position.y = mPs.lowerBound.y + eps;
                    pi.velocity.y *= -attenuation;
                }
                if (pi.position.y > mPs.upperBound.y - eps)
                {
                    pi.position.y = mPs.upperBound.y - eps;
                    pi.velocity.y *= -attenuation;
                }

                // Z 方向边界（3D 新增）
                if (pi.position.z < mPs.lowerBound.z + eps)
                {
                    pi.position.z = mPs.lowerBound.z + eps;
                    pi.velocity.z *= -attenuation;
                }
                if (pi.position.z > mPs.upperBound.z - eps)
                {
                    pi.position.z = mPs.upperBound.z - eps;
                    pi.velocity.z *= -attenuation;
                }
            }

            // ==================== 步骤 6: 更新空间索引 ====================
            //
            // 粒子位置改变后，需要重新计算它们所属的块
            // （updateBlockInfo 会在下一帧 simulate() 开始时调用，这里只更新 blockId）
            //
            #pragma omp parallel for
            for (int i = 0; i < numParticles; i++)
            {
                mPs.particles[i].blockId = mPs.getBlockIdByPosition(mPs.particles[i].position);
            }
        }
    }
}
