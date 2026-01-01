/**
 * ParticleSystem2d.h: 2D粒子系统头文件
 * 定义基于粒子的2D流体仿真数据结构
 */

#pragma once
#ifndef __PARTICAL_SYSTEM_2D_H__
#define __PARTICAL_SYSTEM_2D_H__

#include <vector>
#include <list>
#include <glm/glm.hpp>
#include "Global.h"

#include "Configure.h"

namespace FluidSimulation
{

    namespace Lagrangian2d
    {
        /**
         * 2D粒子信息结构体
         * 存储粒子的物理属性
         */
        struct ParticleInfo2d
        {
            alignas(8) glm::vec2 position;      // 位置
            alignas(8) glm::vec2 velocity;      // 速度
            alignas(8) glm::vec2 accleration;    // 加速度
            alignas(4) float density;            // 密度
            alignas(4) float pressure;          // 压力
            alignas(4) float pressDivDens2;      // 压力除以密度的平方
            alignas(4) uint32_t blockId;         // 所属流体块ID
        };

        /**
         * 2D粒子系统类
         * 管理所有粒子的物理属性和空间索引
         */
        class ParticleSystem2d
        {
        public:
            ParticleSystem2d();
            ~ParticleSystem2d();

            void setContainerSize(glm::vec2 containerCorner, glm::vec2 containerSize);
            int32_t addFluidBlock(glm::vec2 corner, glm::vec2 size, glm::vec2 v0, float particleSpace);
            uint32_t getBlockIdByPosition(glm::vec2 position);
            void updateBlockInfo();

            // ==================== 喷泉功能 ====================
            /**
             * 发射喷泉粒子
             * 在喷泉位置生成新粒子，赋予向上的初速度
             * @param dt 时间步长，用于计算本帧应发射的粒子数
             */
            void emitFountainParticles(float dt);

        public:
            // 粒子参数
            float supportRadius = Lagrangian2dPara::supportRadius;
            float supportRadius2 = supportRadius * supportRadius;
            float particleRadius = Lagrangian2dPara::particleRadius;
            float particleDiameter = Lagrangian2dPara::particleDiameter;
            float particleVolume = particleDiameter * particleDiameter;

            // 存储全部粒子信息
            std::vector<ParticleInfo2d> particles;

            // 容器参数
            glm::vec2 lowerBound = glm::vec2(FLT_MAX);
            glm::vec2 upperBound = glm::vec2(-FLT_MAX);
            glm::vec2 containerCenter = glm::vec2(0.0f);
            float scale = Lagrangian2dPara::scale;
            
            // Block结构（加速临近搜索）
            glm::uvec2 blockNum = glm::uvec2(0);
            glm::vec2 blockSize = glm::vec2(0.0f);
            std::vector<glm::uvec2> blockExtens;
            std::vector<int32_t> blockIdOffs;
        };
    }
}

#endif // !PARTICAL_SYSTEM_H
