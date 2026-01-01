/**
 * MACGrid2d.h: 2D MAC网格头文件
 * 定义基于MAC（Marker-And-Cell）格式的2D网格数据结构
 */

#pragma once
#ifndef __MACGRID_2D_H__
#define __MACGRID_2D_H__

#include <windows.h>
#include <glm/glm.hpp>
#include "GridData2d.h"
#include <Logger.h>

namespace FluidSimulation
{
    namespace Eulerian2d
    {
        /**
         * MAC网格类
         * 存储流体的速度、密度、温度等物理量
         * MAC格式：速度分量存储在网格面中心，标量存储在网格单元中心
         */
        class MACGrid2d
        {
        public:
            MACGrid2d();
            ~MACGrid2d();
            MACGrid2d(const MACGrid2d &orig);
            MACGrid2d &operator=(const MACGrid2d &orig);

            void reset();

            glm::vec4 getRenderColor(int i, int j);
            glm::vec4 getRenderColor(const glm::vec2 &pt);

            // Setup
            void initialize();
            void createSolids();
            void updateSources();

            // advect
            glm::vec2 semiLagrangian(const glm::vec2 &pt, double dt);

            // get value
            glm::vec2 getVelocity(const glm::vec2 &pt);
            double getVelocityX(const glm::vec2 &pt);
            double getVelocityY(const glm::vec2 &pt);
            double getTemperature(const glm::vec2 &pt);
            double getDensity(const glm::vec2 &pt);

            enum Direction
            {
                X,
                Y
            };

            // get point
            glm::vec2 getCenter(int i, int j);
            glm::vec2 getLeft(int i, int j);
            glm::vec2 getRight(int i, int j);
            glm::vec2 getTop(int i, int j);
            glm::vec2 getBottom(int i, int j);

            void getCell(int index, int &i, int &j);
            int getIndex(int i, int j);
            bool isNeighbor(int i0, int j0, int i1, int j1);
            bool isValid(int i, int j, Direction d);

            int isSolidCell(int i, int j);              // Returns 1 if true, else otherwise
            int isSolidFace(int i, int j, Direction d); // Returns 1 if true, else otherwise

            bool inSolid(const glm::vec2 &pt);
            bool inSolid(const glm::vec2 &pt, int &i, int &j);

            bool intersects(const glm::vec2 &pt, const glm::vec2 &dir, int i, int j, double &time);
            int numSolidCells();

            // pressure
            double getPressureCoeffBetweenCells(int i0, int j0, int i1, int j1);
            
            // 散度
            double getDivergence(int i, int j);
            // 散度
            double checkDivergence(int i, int j);
            bool checkDivergence();

            // Boussinesq Force
            double getBoussinesqForce(const glm::vec2 &pt);

            float cellSize;             // 网格单元大小
            int dim[2];                 // 网格维度 [宽, 高]

            Glb::GridData2dX mU;        // X方向速度分量
            Glb::GridData2dY mV;        // Y方向速度分量
            Glb::CubicGridData2d mD;    // 密度场
            Glb::CubicGridData2d mT;    // 温度场

            Glb::GridData2d mSolid;     // 固体标记（1表示固体，0表示流体）
        };

/**
 * 遍历所有网格单元的宏
 * 用于快速遍历网格中的所有单元
 */
#define FOR_EACH_CELL                                                \
    for (int j = 0; j < Eulerian2dPara::theDim2d[MACGrid2d::Y]; j++) \
        for (int i = 0; i < Eulerian2dPara::theDim2d[MACGrid2d::X]; i++)

/**
 * 遍历所有网格线的宏
 * 用于遍历速度分量所在的网格线
 */
#define FOR_EACH_LINE                                                    \
    for (int j = 0; j < Eulerian2dPara::theDim2d[MACGrid2d::Y] + 1; j++) \
        for (int i = 0; i < Eulerian2dPara::theDim2d[MACGrid2d::X] + 1; i++)

    }
}

#endif