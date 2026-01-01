/**
 * Solver.cpp: 2D欧拉流体求解器实现文件
 * 实现基于 MAC 网格的烟雾/流体仿真算法
 *
 * 欧拉方法概述：
 * ==============
 * 欧拉方法将空间划分为固定网格，在网格上存储和更新物理量。
 * 本实现使用 MAC（Marker-And-Cell）网格格式：
 *   - 速度分量存储在网格面中心（交错网格）
 *   - 标量（密度、温度）存储在网格单元中心
 *
 * 主要步骤（Stable Fluids 算法）：
 * 1. 更新源（Source）：在指定位置注入烟雾/热量
 * 2. 添加外力（Forces）：Boussinesq 浮力使热空气上升
 * 3. 平流（Advection）：将物理量沿速度场移动
 * 4. 投影（Projection）：使速度场无散（∇·u = 0）
 */

#include "Eulerian/include/Solver.h"
#include "Configure.h"
#include <vector>
#include <cmath>

namespace FluidSimulation
{
    namespace Eulerian2d
    {
        /**
         * 构造函数，初始化求解器并重置网格
         * @param grid MAC网格引用
         */
        Solver::Solver(MACGrid2d &grid) : mGrid(grid)
        {
            mGrid.reset();
        }

        // ==================== 主求解函数 ====================

        /**
         * 求解流体方程 - 欧拉方法的主函数
         *
         * 实现 Stable Fluids 算法（Jos Stam, 1999）：
         * 这是一种无条件稳定的流体求解方法，即使使用较大的时间步长也不会爆炸。
         *
         * 每个时间步执行以下操作：
         * 1. 更新源：在指定位置持续注入烟雾和热量
         * 2. 添加外力：计算 Boussinesq 浮力
         * 3. 平流：使用半拉格朗日方法移动物理量
         * 4. 投影：求解压力泊松方程，使速度场无散
         */
        void Solver::solve()
        {
            // 临时空实现用于调试
            // 如果这样能运行，说明问题在下面的算法实现中
            return;

            double dt = Eulerian2dPara::dt;

            // 步骤 1: 更新源 - 在指定位置注入烟雾和热量
            mGrid.updateSources();

            // 步骤 2: 添加外力 - Boussinesq 浮力
            addExternalForces(dt);

            // 步骤 3: 平流 - 移动速度、密度、温度
            advect(dt);

            // 步骤 4: 投影 - 使速度场无散
            project(dt);
        }

        // ==================== 平流（Advection） ====================

        /**
         * 平流步骤 - 将物理量沿速度场移动
         *
         * 使用半拉格朗日方法（Semi-Lagrangian Method）：
         * 对于每个网格点，回溯找到 dt 时间前的位置，然后从那里插值获取值。
         *
         * 优点：无条件稳定，即使 CFL 数大于 1 也不会爆炸
         * 缺点：会引入数值耗散，使细节逐渐模糊
         *
         * @param dt 时间步长
         */
        void Solver::advect(double dt)
        {
            // 创建临时网格存储平流后的结果
            Glb::GridData2dX newU;
            Glb::GridData2dY newV;
            Glb::CubicGridData2d newD;
            Glb::CubicGridData2d newT;

            newU.initialize(0.0);
            newV.initialize(0.0);
            newD.initialize(0.0);
            newT.initialize(Eulerian2dPara::ambientTemp);

            double cellSize = mGrid.cellSize;
            int dimX = Eulerian2dPara::theDim2d[0];
            int dimY = Eulerian2dPara::theDim2d[1];

            // -------------------- 平流 X 方向速度 --------------------
            // X 速度存储在网格左边界面上（i, j+0.5 的位置）
            for (int j = 0; j < dimY; j++)
            {
                for (int i = 0; i < dimX + 1; i++)
                {
                    // 跳过固体边界
                    if (mGrid.isSolidFace(i, j, MACGrid2d::X))
                    {
                        newU(i, j) = 0.0;
                        continue;
                    }

                    // 获取当前面中心的世界坐标
                    glm::vec2 pos = mGrid.getLeft(i, j);

                    // 回溯：找到 dt 时间前的位置
                    glm::vec2 oldPos = mGrid.semiLagrangian(pos, dt);

                    // 从旧位置插值获取速度
                    newU(i, j) = mGrid.getVelocityX(oldPos);
                }
            }

            // -------------------- 平流 Y 方向速度 --------------------
            // Y 速度存储在网格下边界面上（i+0.5, j 的位置）
            for (int j = 0; j < dimY + 1; j++)
            {
                for (int i = 0; i < dimX; i++)
                {
                    // 跳过固体边界
                    if (mGrid.isSolidFace(i, j, MACGrid2d::Y))
                    {
                        newV(i, j) = 0.0;
                        continue;
                    }

                    // 获取当前面中心的世界坐标
                    glm::vec2 pos = mGrid.getBottom(i, j);

                    // 回溯
                    glm::vec2 oldPos = mGrid.semiLagrangian(pos, dt);

                    // 从旧位置插值获取速度
                    newV(i, j) = mGrid.getVelocityY(oldPos);
                }
            }

            // -------------------- 平流密度和温度 --------------------
            // 密度和温度存储在网格单元中心
            FOR_EACH_CELL
            {
                // 获取单元中心的世界坐标
                glm::vec2 pos = mGrid.getCenter(i, j);

                // 回溯
                glm::vec2 oldPos = mGrid.semiLagrangian(pos, dt);

                // 从旧位置插值获取密度和温度
                newD(i, j) = mGrid.getDensity(oldPos);
                newT(i, j) = mGrid.getTemperature(oldPos);
            }

            // 将结果复制回原网格
            mGrid.mU = newU;
            mGrid.mV = newV;
            mGrid.mD = newD;
            mGrid.mT = newT;
        }

        // ==================== 外力（External Forces） ====================

        /**
         * 添加外力 - Boussinesq 浮力
         *
         * Boussinesq 近似：
         *   F_y = -α × ρ_smoke + β × (T - T_ambient)
         *
         * 其中：
         *   - α (boussinesqAlpha): 烟雾密度系数，密度大则下沉
         *   - β (boussinesqBeta): 温度系数，温度高则上升
         *   - ρ_smoke: 烟雾密度
         *   - T: 当前温度
         *   - T_ambient: 环境温度
         *
         * 这个力只影响 Y 方向速度（垂直方向）
         *
         * @param dt 时间步长
         */
        void Solver::addExternalForces(double dt)
        {
            int dimX = Eulerian2dPara::theDim2d[0];
            int dimY = Eulerian2dPara::theDim2d[1];

            // 遍历所有 Y 方向速度分量（在网格的水平边上）
            for (int j = 0; j < dimY + 1; j++)
            {
                for (int i = 0; i < dimX; i++)
                {
                    // 跳过固体边界
                    if (mGrid.isSolidFace(i, j, MACGrid2d::Y))
                        continue;

                    // 获取当前面中心位置
                    glm::vec2 pos = mGrid.getBottom(i, j);

                    // 计算 Boussinesq 浮力
                    double force = mGrid.getBoussinesqForce(pos);

                    // 更新 Y 方向速度：v += F × dt
                    mGrid.mV(i, j) += force * dt;
                }
            }
        }

        // ==================== 投影（Projection） ====================

        /**
         * 压力投影 - 使速度场无散
         *
         * 不可压缩流体的连续性方程要求：∇·u = 0（速度场散度为零）
         *
         * 投影步骤：
         * 1. 计算当前速度场的散度
         * 2. 求解压力泊松方程：∇²p = ∇·u / dt
         * 3. 用压力梯度修正速度：u_new = u - dt × ∇p
         *
         * 压力泊松方程使用 Gauss-Seidel 迭代法求解（无需外部库）
         *
         * @param dt 时间步长
         */
        void Solver::project(double dt)
        {
            int dimX = Eulerian2dPara::theDim2d[0];
            int dimY = Eulerian2dPara::theDim2d[1];
            int numCells = dimX * dimY;
            double cellSize = mGrid.cellSize;

            // -------------------- 初始化压力场和散度场 --------------------
            // 使用 2D 数组存储压力和散度
            std::vector<std::vector<double>> pressure(dimX, std::vector<double>(dimY, 0.0));
            std::vector<std::vector<double>> divergence(dimX, std::vector<double>(dimY, 0.0));

            // 计算每个单元的散度
            FOR_EACH_CELL
            {
                if (!mGrid.isSolidCell(i, j))
                {
                    divergence[i][j] = mGrid.getDivergence(i, j);
                }
            }

            // -------------------- Gauss-Seidel 迭代求解压力泊松方程 --------------------
            // 泊松方程：∇²p = ∇·u / dt
            // 离散形式：(p_{i+1,j} + p_{i-1,j} + p_{i,j+1} + p_{i,j-1} - 4*p_{i,j}) / h² = div / dt
            //
            // Gauss-Seidel 更新公式：
            // p_{i,j} = (p_{i+1,j} + p_{i-1,j} + p_{i,j+1} + p_{i,j-1} - h * div / dt) / numNeighbors

            const int maxIterations = 100;  // 最大迭代次数
            const double tolerance = 1e-6;  // 收敛容差

            for (int iter = 0; iter < maxIterations; iter++)
            {
                double maxError = 0.0;

                FOR_EACH_CELL
                {
                    // 跳过固体单元
                    if (mGrid.isSolidCell(i, j))
                    {
                        pressure[i][j] = 0.0;
                        continue;
                    }

                    // 计算邻居压力之和，并统计有效邻居数量
                    double sumP = 0.0;
                    double numNeighbors = 0.0;

                    // 左邻居
                    if (i > 0 && !mGrid.isSolidCell(i - 1, j))
                    {
                        sumP += pressure[i - 1][j];
                        numNeighbors += 1.0;
                    }
                    else if (i == 0)
                    {
                        // 开放边界：使用当前压力值（Neumann边界条件）
                        numNeighbors += 1.0;
                    }

                    // 右邻居
                    if (i < dimX - 1 && !mGrid.isSolidCell(i + 1, j))
                    {
                        sumP += pressure[i + 1][j];
                        numNeighbors += 1.0;
                    }
                    else if (i == dimX - 1)
                    {
                        numNeighbors += 1.0;
                    }

                    // 下邻居
                    if (j > 0 && !mGrid.isSolidCell(i, j - 1))
                    {
                        sumP += pressure[i][j - 1];
                        numNeighbors += 1.0;
                    }
                    else if (j == 0)
                    {
                        numNeighbors += 1.0;
                    }

                    // 上邻居
                    if (j < dimY - 1 && !mGrid.isSolidCell(i, j + 1))
                    {
                        sumP += pressure[i][j + 1];
                        numNeighbors += 1.0;
                    }
                    else if (j == dimY - 1)
                    {
                        numNeighbors += 1.0;
                    }

                    // 计算新的压力值
                    if (numNeighbors > 0)
                    {
                        double rhs = -divergence[i][j] * cellSize / dt;
                        double newP = (sumP - rhs) / numNeighbors;

                        // 计算误差
                        double error = fabs(newP - pressure[i][j]);
                        if (error > maxError) maxError = error;

                        pressure[i][j] = newP;
                    }
                }

                // 检查收敛
                if (maxError < tolerance)
                {
                    break;
                }
            }

            // -------------------- 用压力梯度修正速度 --------------------
            // u_new = u - dt × ∂p/∂x
            // v_new = v - dt × ∂p/∂y

            // 修正 X 方向速度
            for (int j = 0; j < dimY; j++)
            {
                for (int i = 0; i < dimX + 1; i++)
                {
                    // 跳过固体边界
                    if (mGrid.isSolidFace(i, j, MACGrid2d::X))
                    {
                        mGrid.mU(i, j) = 0.0;
                        continue;
                    }

                    // 计算压力梯度
                    double pLeft = (i > 0) ? pressure[i - 1][j] : 0.0;
                    double pRight = (i < dimX) ? pressure[i][j] : 0.0;

                    // 边界处理
                    if (i == 0) pLeft = pRight;
                    if (i == dimX) pRight = pLeft;

                    double gradP = (pRight - pLeft) / cellSize;

                    // 更新速度
                    mGrid.mU(i, j) -= dt * gradP;
                }
            }

            // 修正 Y 方向速度
            for (int j = 0; j < dimY + 1; j++)
            {
                for (int i = 0; i < dimX; i++)
                {
                    // 跳过固体边界
                    if (mGrid.isSolidFace(i, j, MACGrid2d::Y))
                    {
                        mGrid.mV(i, j) = 0.0;
                        continue;
                    }

                    // 计算压力梯度
                    double pBottom = (j > 0) ? pressure[i][j - 1] : 0.0;
                    double pTop = (j < dimY) ? pressure[i][j] : 0.0;

                    // 边界处理
                    if (j == 0) pBottom = pTop;
                    if (j == dimY) pTop = pBottom;

                    double gradP = (pTop - pBottom) / cellSize;

                    // 更新速度
                    mGrid.mV(i, j) -= dt * gradP;
                }
            }
        }
    }
}
