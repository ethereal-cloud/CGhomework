#pragma once
#ifndef __PARTICAL_SYSTEM_3D_H__
#define __PARTICAL_SYSTEM_3D_H__

#include <glm/glm.hpp>
#include <vector>
#include "Configure.h"

namespace FluidSimulation
{
    namespace Lagrangian3d
    {
        // 三维粒子信息结构体
        struct particle3d
        {
            alignas(16) glm::vec3 position;      // 位置
            alignas(16) glm::vec3 velocity;      // 速度
            alignas(16) glm::vec3 accleration;   // 加速度
            alignas(4) float_t density;          // 密度
            alignas(4) float_t pressure;         // 压力
            alignas(4) float_t pressDivDens2;    // 压力除以密度平方
            alignas(4) uint32_t blockId;         // 所在块的ID
        };

        // 粒子系统类
        // 管理三维流体粒子和空间划分
        class ParticleSystem3d
        {
        public:
            ParticleSystem3d();
            ~ParticleSystem3d();

            // 容器和流体块操作
            void setContainerSize(glm::vec3 corner, glm::vec3 size);  // 设置容器大小
            int32_t addFluidBlock(glm::vec3 corner, glm::vec3 size, glm::vec3 v0, float particleSpace);  // 添加流体块
            
            // 空间划分相关
            uint32_t getBlockIdByPosition(glm::vec3 position);  // 根据位置获取块ID
            void updateBlockInfo();                             // 更新块信息

        public:
            // 粒子参数
            float supportRadius = Lagrangian3dPara::supportRadius;     // 支持半径
            float supportRadius2 = supportRadius * supportRadius;       // 支持半径平方
            float particleRadius = Lagrangian3dPara::particleRadius;   // 粒子半径
            float particleDiameter = Lagrangian3dPara::particleDiameter;  // 粒子直径
            float particleVolume = std::pow(particleDiameter, 3);      // 粒子体积

            // 粒子数据
            std::vector<particle3d> particles;  // 粒子数组

            // 容器参数
            glm::vec3 lowerBound = glm::vec3(FLT_MAX);     // 容器下边界
            glm::vec3 upperBound = glm::vec3(-FLT_MAX);    // 容器上边界
            glm::vec3 containerCenter = glm::vec3(0.0f);   // 容器中心
            float scale = Lagrangian3dPara::scale;         // 缩放系数

            // 空间划分参数
            glm::uvec3 blockNum = glm::uvec3(0);          // 块数量
            glm::vec3 blockSize = glm::vec3(0.0f);        // 块大小
            std::vector<glm::uvec2> blockExtens;          // 块范围
            std::vector<int32_t> blockIdOffs;             // 块偏移
        };
    }
}

#endif