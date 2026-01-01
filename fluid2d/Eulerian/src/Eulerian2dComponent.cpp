/**
 * Eulerian2dComponent.cpp: 2D欧拉流体组件实现文件
 * 实现组件的初始化、仿真和渲染功能
 */

#include "Eulerian2dComponent.h"

namespace FluidSimulation {
    namespace Eulerian2d {

        /**
         * 关闭组件，释放资源
         */
        void Eulerian2dComponent::shutDown() {
            delete renderer, solver, grid;
            renderer = NULL;
            solver = NULL;
            grid = NULL;
        }

        // 初始化组件
        void Eulerian2dComponent::init() {
            // 如果已经初始化过,先释放资源
            if (renderer != NULL || solver != NULL || grid != NULL) {
                shutDown();
            }

            // 清空计时器
            Glb::Timer::getInstance().clear();

            // 创建MAC网格
            grid = new MACGrid2d();

            // 记录网格创建日志
            Glb::Logger::getInstance().addLog("2d MAC gird created. dimension: " + std::to_string(Eulerian2dPara::theDim2d[0]) + "x"
                + std::to_string(Eulerian2dPara::theDim2d[1]) + ". cell size:" + std::to_string(Eulerian2dPara::theCellSize2d).substr(0,3));

            // 创建渲染器和求解器
            renderer = new Renderer();
            solver = new Solver(*grid);
        }

        // 执行一步模拟
        void Eulerian2dComponent::simulate() {
            // 更新烟雾源
            grid->updateSources();
            // 求解流体方程
            solver->solve();
        }

        // 获取渲染结果的纹理ID
        GLuint Eulerian2dComponent::getRenderedTexture()
        {
            // 绘制网格
            renderer->draw(*grid);
            // 返回渲染的纹理
            return renderer->getTextureID();
        }
    }
}