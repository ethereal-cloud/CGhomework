#pragma once
#ifndef __EULERIAN_2D_COMPONENT_H__
#define __EULERIAN_2D_COMPONENT_H__

#include "Renderer.h"
#include "Solver.h"
#include "MACGrid2d.h"

#include "Component.h"
#include "Configure.h"
#include "Logger.h"

namespace FluidSimulation {
    namespace Eulerian2d {
        // 欧拉法流体模拟组件类
        // 管理MAC网格、求解器和渲染器
        class Eulerian2dComponent : public Glb::Component {
        public:
            Renderer* renderer;    // 渲染器
            Solver* solver;        // 求解器
            MACGrid2d* grid;      // MAC网格

            Eulerian2dComponent(char* description, int id) {
                this->description = description;
                this->id = id;
                renderer = NULL;
                solver = NULL;
                grid = NULL;
            }

            virtual void shutDown();       // 关闭组件,释放资源
            virtual void init();           // 初始化组件
            virtual void simulate();       // 执行一步模拟
            virtual GLuint getRenderedTexture();  // 获取渲染结果
        };
    }
}

#endif