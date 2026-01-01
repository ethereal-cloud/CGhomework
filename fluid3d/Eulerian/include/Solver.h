/**
 * Solver.h: 3D欧拉流体求解器头文件
 * 定义流体仿真求解器类
 */

#pragma once
#ifndef __EULERIAN_3D_SOLVER_H__
#define __EULERIAN_3D_SOLVER_H__

#include "MACGrid3d.h"
#include "Configure.h"

namespace FluidSimulation
{
	namespace Eulerian3d
	{
		/**
		 * 求解器类
		 * 实现基于MAC网格的3D欧拉流体仿真算法
		 */
		class Solver
		{
		public:
			/**
			 * 构造函数
			 * @param grid MAC网格引用
			 */
			Solver(MACGrid3d &grid);

			/**
			 * 执行一步仿真计算
			 * 包含速度更新、压力求解等步骤
			 */
			void solve();

		protected:
			MACGrid3d &mGrid;  // MAC网格引用
		};
	}
}

#endif // !__EULERIAN_3D_SOLVER_H__
