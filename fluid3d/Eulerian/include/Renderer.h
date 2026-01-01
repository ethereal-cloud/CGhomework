#pragma once
#ifndef __EULERIAN_3D_RENDERER_H__
#define __EULERIAN_3D_RENDERER_H__

#include "glm/glm.hpp"
#include <glad/glad.h>
#include <glfw3.h>
#include "Shader.h"
#include "Container.h"
#include "MACGrid3d.h"
#include "Camera.h"
#include "Configure.h"
#include <Logger.h>

namespace FluidSimulation {
	namespace Eulerian3d {
		// 欧拉法流体渲染器类
		// 负责将三维MAC网格中的流体数据可视化
		class Renderer {
		public:
			Renderer(MACGrid3d& grid);

			void loadTexture();                   // 加载纹理
			void resetVertices(float x, float y, float z);  // 重置顶点数据
			void draw();                          // 绘制整个场景
			void drawOneSheet();                  // 绘制单层切片
			void drawOneSheetXY();                // 绘制XY平面切片
			void drawOneSheetYZ();                // 绘制YZ平面切片
			void drawOneSheetXZ();                // 绘制XZ平面切片
			void drawXYSheets();                  // 绘制多层XY切片
			void drawYZSheets();                  // 绘制多层YZ切片
			void drawXZSheets();                  // 绘制多层XZ切片

			GLuint getTextureID();                // 获取渲染结果的纹理ID

		private:
			GLuint textureID = 0;                 // 渲染纹理ID
			GLuint smokeTexture = 0;              // 烟雾纹理ID

			float eps = 0.05f;                    // 边界偏移量

			// 不同平面的顶点数组对象
			GLuint VAO_XY;                        // XY平面VAO
			GLuint VBO_XY;                        // XY平面VBO
			GLuint VAO_YZ;                        // YZ平面VAO
			GLuint VBO_YZ;                        // YZ平面VBO
			GLuint VAO_XZ;                        // XZ平面VAO
			GLuint VBO_XZ;                        // XZ平面VBO

			// 通用渲染对象
			GLuint VAO = 0;                       // 顶点数组对象
			GLuint VBO = 0;                       // 顶点缓冲对象
			GLuint EBO = 0;                       // 索引缓冲对象
			GLuint FBO = 0;                       // 帧缓冲对象
			GLuint RBO = 0;                       // 渲染缓冲对象

			// 像素分辨率
			int pixelX = (float)Eulerian3dPara::theDim3d[0] / Eulerian3dPara::theDim3d[2] * 200;
			int pixelY = 200;
			int pixelZ = 200;

			// 顶点数据
			float* dataXY;                        // XY平面顶点数据
			float* dataYZ;                        // YZ平面顶点数据
			float* dataXZ;                        // XZ平面顶点数据

			// 着色器和容器
			Glb::Shader* pixelShader;             // 像素着色器
			Glb::Shader* gridShader;              // 网格着色器
			Glb::Container* container;            // 容器对象

			MACGrid3d& mGrid;                     // MAC网格引用
		};
	}
}

#endif
