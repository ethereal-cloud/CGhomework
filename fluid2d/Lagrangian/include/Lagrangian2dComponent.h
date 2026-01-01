#pragma once
#ifndef __LAGRANGIAN_2D_COMPONENT_H__
#define __LAGRANGIAN_2D_COMPONENT_H__

#include "Renderer.h"
#include "Solver.h"
#include "ParticleSystem2d.h"

#include "Component.h"
#include "Configure.h"
#include "Logger.h"

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        // 拉格朗日法流体模拟组件类
        // 管理粒子系统、求解器和渲染器
        class Lagrangian2dComponent : public Glb::Component
        {
        public:
            Renderer *renderer;        // 渲染器
            Solver *solver;            // 求解器
            ParticleSystem2d *ps;      // 粒子系统

            Lagrangian2dComponent(char *description, int id)
            {
                this->description = description;
                this->id = id;
                renderer = NULL;
                solver = NULL;
                ps = NULL;
            }

            virtual void shutDown();           // 关闭组件,释放资源
            virtual void init();               // 初始化组件
            virtual void simulate();           // 执行一步模拟
            virtual GLuint getRenderedTexture();  // 获取渲染结果
        };
    }
}

#endif