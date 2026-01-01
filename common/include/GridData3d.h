#pragma once
#ifndef __GRID_DATA_3D_H__
#define __GRID_DATA_3D_H__

#pragma warning(disable: 4244 4267 4996)
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <glm/glm.hpp>

namespace Glb {

	using namespace boost::numeric;

	// 3D网格数据基类,用于存储和处理MAC网格上的标量场
	class GridData3d
	{
	public:
		GridData3d();
		GridData3d(const GridData3d& orig);
		virtual ~GridData3d();
		virtual GridData3d& operator=(const GridData3d& orig);

		// 用默认值初始化网格
		virtual void initialize(double dfltValue = 0.0);

		// 访问(i,j,k)位置上的可改变数据
		virtual double& operator()(int i, int j, int k);

		// 给定坐标，返回插值得到的值
		virtual double interpolate(const glm::vec3& pt);

		ublas::vector<double>& data();

		// 给定世界坐标，返回该点所在的网格单元
		virtual void getCell(const glm::vec3& pt, int& i, int& j, int& k);

		glm::vec3 mMax;

	protected:
		virtual glm::vec3 worldToSelf(const glm::vec3& pt) const;
		double mDfltValue;				// 默认值，用于初始化数据
		ublas::vector<double> mData;	    // 存储网格数据的一维数组
		float cellSize;                  // 网格单元大小
		int dim[3];                      // 网格维度
	};

	// X方向速度分量的网格数据类
	class GridData3dX : public GridData3d
	{
	public:
		GridData3dX();
		virtual ~GridData3dX();
		virtual void initialize(double dfltValue = 0.0);
		virtual double& operator()(int i, int j, int k);
		virtual glm::vec3 worldToSelf(const glm::vec3& pt) const;
	};

	// Y方向速度分量的网格数据类
	class GridData3dY : public GridData3d
	{
	public:
		GridData3dY();
		virtual ~GridData3dY();
		virtual void initialize(double dfltValue = 0.0);
		virtual double& operator()(int i, int j, int k);
		virtual glm::vec3 worldToSelf(const glm::vec3& pt) const;
	};

	// Z方向速度分量的网格数据类
	class GridData3dZ : public GridData3d
	{
	public:
		GridData3dZ();
		virtual ~GridData3dZ();
		virtual void initialize(double dfltValue = 0.0);
		virtual double& operator()(int i, int j, int k);
		virtual glm::vec3 worldToSelf(const glm::vec3& pt) const;
	};

	// 使用三次插值的网格数据类
	class CubicGridData3d : public GridData3d
	{
	public:
		CubicGridData3d();
		CubicGridData3d(const CubicGridData3d& orig);
		virtual ~CubicGridData3d();
		virtual double interpolate(const glm::vec3& pt);

	protected:
		// 三次插值辅助函数
		double cubic(double q1, double q2, double q3, double q4, double t);
		double interpX(int i, int j, int k, double fracty, double fractx);
		double interpY(int i, int j, int k, double fracty);
	};
}

#endif