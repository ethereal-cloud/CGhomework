/**
 * Configure.h: 系统配置文件
 * 定义系统的各种配置参数和全局变量
 */

#pragma once
#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include <iostream>
#include <string>
#include "Component.h"
#include <vector>
#include "glm/glm.hpp"

/**
 * 线性插值宏
 * @param a 起始值
 * @param b 结束值
 * @param t 插值参数 [0,1]
 */
#define LERP(a, b, t) (1 - t) * a + t *b

#ifndef __MINMAX_DEFINED
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// 渲染分辨率
extern int imageWidth;      // 渲染图像宽度
extern int imageHeight;     // 渲染图像高度

// 窗口尺寸
extern int windowWidth;     // 窗口宽度
extern int windowHeight;    // 窗口高度

// UI设置
extern float fontSize;      // 字体大小

// 仿真状态
extern bool simulating;     // 是否正在仿真

/**
 * 2D欧拉方法参数命名空间
 * 包含2D欧拉流体仿真的所有配置参数
 */
namespace Eulerian2dPara
{
    /**
     * 烟雾源结构体
     * 定义烟雾源的初始位置、速度、密度和温度
     */
    struct SourceSmoke {
        glm::ivec2 position = glm::ivec2(0);
        glm::vec2 velocity = glm::vec2(0.0f);
        float density = 0.0f;
        float temp = 0.0f;
    };

    extern int theDim2d[];
    extern std::vector<SourceSmoke> source;
    extern float theCellSize2d;
    extern bool addSolid;

    extern float dt;

    extern float contrast;
    extern int drawModel;
    extern int gridNum;


    extern float airDensity;
    extern float ambientTemp;
    extern float boussinesqAlpha;
    extern float boussinesqBeta;
    extern float vorticityConst;
}

/**
 * 3D欧拉方法参数命名空间
 * 包含3D欧拉流体仿真的所有配置参数
 */
namespace Eulerian3dPara
{
    /**
     * 烟雾源结构体
     * 定义烟雾源的初始位置、速度、密度和温度
     */
    struct SourceSmoke {
        glm::ivec3 position = glm::ivec3(0);
        glm::vec3 velocity = glm::vec3(0.0f);
        float density = 0.0f;
        float temp = 0.0f;
    };

    extern int theDim3d[];
    extern float theCellSize3d;
    extern std::vector<SourceSmoke> source;
    extern bool addSolid;

    extern float contrast;
    extern bool oneSheet;
    extern float distanceX;
    extern float distanceY;
    extern float distanceZ;
    extern bool xySheetsON;
    extern bool yzSheetsON;
    extern bool xzSheetsON;
    extern int xySheetsNum;
    extern int yzSheetsNum;
    extern int xzSheetsNum;
    extern int drawModel;
    extern int gridNumX;
    extern int gridNumY;
    extern int gridNumZ;

    extern float dt;

    extern float airDensity;
    extern float ambientTemp;
    extern float boussinesqAlpha;
    extern float boussinesqBeta;
    extern float vorticityConst;

}

/**
 * 2D拉格朗日方法参数命名空间
 * 包含2D拉格朗日流体仿真的所有配置参数
 */
namespace Lagrangian2dPara
{
    /**
     * 流体块结构体
     * 定义初始流体块的位置、大小、初始速度和粒子间距
     */
    struct FluidBlock {
        glm::vec2 lowerCorner = glm::vec2(0.0f, 0.0f);
        glm::vec2 upperCorner = glm::vec2(0.0f, 0.0f);
        glm::vec2 initVel = glm::vec2(0.0f, 0.0f);
        float particleSpace = 0.02f;
    };

    extern float scale;
    extern std::vector<FluidBlock> fluidBlocks;

    extern float dt;
    extern int substep;
    extern float maxVelocity;
    extern float velocityAttenuation;
    extern float eps;

    extern float supportRadius;
    extern float particleRadius;
    extern float particleDiameter;
    extern float gravityX;
    extern float gravityY;
    extern float density;
    extern float stiffness;
    extern float exponent;
    extern float viscosity;

    // ==================== 喷泉参数 ====================
    extern bool enableFountain;          // 是否启用喷泉模式
    extern glm::vec2 fountainPosition;   // 喷泉位置（未缩放坐标）
    extern glm::vec2 fountainVelocity;   // 喷射初速度
    extern float fountainSpread;         // 喷口宽度
    extern float emissionRate;           // 每秒发射粒子数
    extern int maxParticles;             // 粒子数量上限
}

/**
 * 3D拉格朗日方法参数命名空间
 * 包含3D拉格朗日流体仿真的所有配置参数
 */
namespace Lagrangian3dPara
{
    /**
     * 流体块结构体
     * 定义初始流体块的位置、大小、初始速度和粒子间距
     */
    struct FluidBlock {
        glm::vec3 lowerCorner = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upperCorner = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 initVel = glm::vec3(0.0f, 0.0f, 0.0f);
        float particleSpace = 0.02f;
    };

    extern float scale;
    extern std::vector<FluidBlock> fluidBlocks;


    extern float dt;
    extern int substep;
    extern float maxVelocity;
    extern float velocityAttenuation;
    extern float eps;

    extern float supportRadius;
    extern float particleRadius;
    extern float particleDiameter;

    extern float gravityX;
    extern float gravityY;
    extern float gravityZ;

    extern float density;
    extern float stiffness;
    extern float exponent;
    extern float viscosity;

    // ==================== 表面张力 ====================
    extern bool enableSurfaceTension;        // 是否启用表面张力
    extern float surfaceTension;             // 表面张力系数 σ (N/m)
    extern float surfaceThreshold;           // 表面检测阈值

    // ==================== 运动固体（搅拌棒） ====================
    extern bool enableStirrer;               // 是否启用搅拌棒
    extern glm::vec3 stirrerCenter;          // 基准中心（未缩放坐标）
    extern float stirrerOrbitRadius;         // 圆周半径（未缩放坐标）
    extern float stirrerFrequency;           // 运动频率（Hz）
    extern float stirrerRodRadius;           // 竖杆半径（未缩放坐标）
    extern float stirrerRodZMin;             // 竖杆底部高度（未缩放坐标）
    extern float stirrerRodZMax;             // 竖杆顶部高度（未缩放坐标）
    extern float stirrerBarLength;           // 底部横杠长度（未缩放坐标）
    extern float stirrerBarZ;                // 底部横杠高度（未缩放坐标）
    extern float stirrerTime;                // 仿真时间（秒）
}

// 资源路径
extern std::string shaderPath;      // 着色器文件路径
extern std::string picturePath;      // 图片文件路径

// 组件管理
extern std::vector<Glb::Component *> methodComponents;  // 所有方法组件的列表

#endif // !__CONFIGURE_H__
