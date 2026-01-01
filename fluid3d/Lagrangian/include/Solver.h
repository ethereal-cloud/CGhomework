/**
 * Solver.h: 3D拉格朗日流体求解器头文件
 * 定义基于SPH的3D流体仿真求解器类
 */

#pragma once
#ifndef __LAGRANGIAN_3D_SOLVER_H__
#define __LAGRANGIAN_3D_SOLVER_H__

#include "ParticleSystem3d.h"
#include "Global.h"
#include "Configure.h"
#include <iostream>

namespace FluidSimulation
{
    namespace Lagrangian3d
    {
        /**
         * 拉格朗日法流体求解器类
         * 负责求解基于粒子的三维流体动力学方程
         */
        class Solver
        {
        public:
            Solver(ParticleSystem3d &ps);

            void solve();                         // 求解流体方程

        private:
            ParticleSystem3d &mPs;               // 粒子系统引用

            // ==================== SPH 核函数 (3D 版本) ====================

            /**
             * Poly6 核函数 - 用于密度计算
             * W(r,h) = 315/(64πh⁹) × (h²-r²)³  当 0 ≤ r ≤ h
             * @param r2 两粒子距离的平方 |r|²
             * @return 核函数值
             */
            float poly6(float r2);

            /**
             * Spiky 核函数的梯度 - 用于压力梯度计算
             * ∇W(r,h) = -45/(πh⁶) × (h-r)² × (r/|r|)  当 0 < r ≤ h
             * @param r 从粒子j指向粒子i的向量 (ri - rj)
             * @return 核函数梯度向量
             */
            glm::vec3 spikyGrad(glm::vec3 r);

            /**
             * 粘性核函数的拉普拉斯 - 用于粘性力计算
             * ∇²W(r,h) = 45/(πh⁶) × (h-r)  当 0 ≤ r ≤ h
             * @param r 两粒子之间的距离 |r|
             * @return 核函数拉普拉斯值
             */
            float viscosityLaplacian(float r);
        };
    }
}

#endif
