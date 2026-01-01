#pragma once
#ifndef __GRID_DATA_2D_H__
#define __GRID_DATA_2D_H__

#pragma warning(disable: 4244 4267 4996)
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <glm/glm.hpp>

namespace Glb {

	using namespace boost::numeric;

	// 2D网格数据基类,用于存储和处理MAC网格上的标量场
	class GridData2d
	{
	public:
		GridData2d();
		GridData2d(const GridData2d& orig);
		virtual ~GridData2d();
		virtual GridData2d& operator=(const GridData2d& orig);

		// 用默认值初始化网格
		virtual void initialize(double dfltValue = 0.0);

		// 访问(i,j)位置上的可改变数据
		// 可以使用 GridData2d(i, j) = num 这样的形式进行赋值
		virtual double& operator()(int i, int j);

		// 给定坐标，返回插值得到的值
		// 对于超出范围的点，返回默认值
		virtual double interpolate(const glm::vec2& pt);	

		// 访问数据，将mData作为ublas向量进行操作
		ublas::vector<double>& data();

		// 给定世界坐标，返回该点所在的网格单元
		virtual void getCell(const glm::vec2& pt, int& i, int& j);

		virtual glm::vec2 worldToSelf(const glm::vec2& pt) const;
		double mDfltValue;				// 默认值，用于初始化数据
		glm::vec2 mMax;					// 二维空间中的最大坐标，表示网格的尺寸
		ublas::vector<double> mData;	    // 存储网格数据的一维数组
		float cellSize;                  // 网格单元大小
		int dim[2];                      // 网格维度
	};

	// X方向速度分量的网格数据类
	class GridData2dX : public GridData2d
	{
	public:
		GridData2dX();
		virtual ~GridData2dX();
		virtual void initialize(double dfltValue = 0.0);
		virtual double& operator()(int i, int j);
		virtual glm::vec2 worldToSelf(const glm::vec2& pt) const;
	};

	// Y方向速度分量的网格数据类
	class GridData2dY : public GridData2d
	{
	public:
		GridData2dY();
		virtual ~GridData2dY();
		virtual void initialize(double dfltValue = 0.0);
		virtual double& operator()(int i, int j);
		virtual glm::vec2 worldToSelf(const glm::vec2& pt) const;
	};

	// 使用三次插值的网格数据类
	class CubicGridData2d : public GridData2d
	{
	public:
		CubicGridData2d();
		CubicGridData2d(const CubicGridData2d& orig);
		virtual ~CubicGridData2d();
		virtual double interpolate(const glm::vec2& pt);

	protected:
		// 三次插值辅助函数
		double cubic(double q1, double q2, double q3, double q4, double t);
		double interpX(int i, int j, double fracty, double fractx);
		double interpY(int i, int j, double fracty);
	};
}

#endif