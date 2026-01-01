/**
 * Eulerian3dComponent.cpp: 3D欧拉流体组件实现文件
 * 实现组件的初始化、仿真和渲染功能
 */

#include "Eulerian3dComponent.h"

namespace FluidSimulation {
    namespace Eulerian3d {
        /**
         * 关闭组件，释放资源
         */
        void Eulerian3dComponent::shutDown() {
            // 释放所有组件资源
            delete renderer, solver, grid;
            renderer = NULL;
            solver = NULL;
            grid = NULL;
        }

        /**
         * 初始化组件
         * 创建MAC网格、渲染器和求解器
         */
        void Eulerian3dComponent::init() {
            // 如果组件已存在则先释放
            if (renderer != NULL || solver != NULL || grid != NULL) {
                shutDown();
            }

            // 重置计时器
            Glb::Timer::getInstance().clear();

            // 创建MAC网格
            grid = new MACGrid3d();

            // 记录网格创建日志
            Glb::Logger::getInstance().addLog("3d MAC gird created. dimension: " + std::to_string(Eulerian3dPara::theDim3d[0]) + "x"
                + std::to_string(Eulerian3dPara::theDim3d[1]) + "x"
                + std::to_string(Eulerian3dPara::theDim3d[2]) + ". cell size:"
                + std::to_string(Eulerian3dPara::theCellSize3d).substr(0, 3));

            // 创建渲染器和求解器
            renderer = new Renderer(*grid);
            solver = new Solver(*grid);
        }

        void Eulerian3dComponent::simulate() {
            // 更新烟雾源并求解一步
            grid->updateSources();
            solver->solve();
        }

        GLuint Eulerian3dComponent::getRenderedTexture()
        {
            // 绘制场景并返回渲染结果
            renderer->draw();          
            return renderer->getTextureID();
        }
    }
}