/**
 * Renderer.h: 2D欧拉流体渲染器头文件
 * 负责将MAC网格数据渲染到纹理
 */

#pragma once
#ifndef __EULERIAN_2D_RENDERER_H__
#define __EULERIAN_2D_RENDERER_H__

#include "MACGrid2d.h"
#include <glad/glad.h>
#include <glfw3.h>
#include "Configure.h"
#include "glm/glm.hpp"
#include "Shader.h"
#include <Logger.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // !STB_IMAGE_IMPLEMENTATION

namespace FluidSimulation {
	namespace Eulerian2d {
		/**
		 * 渲染器类
		 * 将MAC网格的密度、温度等数据渲染到纹理
		 */
		class Renderer {
		public:
			/**
			 * 构造函数
			 * 初始化OpenGL资源和着色器
			 */
			Renderer();

			/**
			 * 绘制网格数据到纹理
			 * @param mGrid MAC网格引用
			 */
			void draw(MACGrid2d& mGrid);

			/**
			 * 加载纹理
			 */
			void loadTexture();

			/**
			 * 获取渲染结果的纹理ID
			 * @return OpenGL纹理ID
			 */
			GLuint getTextureID();

		private:
			Glb::Shader* shader;      // 着色器程序
			
			float* data;              // 纹理数据缓冲区

			GLuint VAO = 0;           // 顶点数组对象
			GLuint VBO = 0;           // 顶点缓冲对象
			GLuint EBO = 0;           // 索引缓冲对象
			GLuint FBO = 0;           // 帧缓冲对象
			GLuint RBO = 0;           // 渲染缓冲对象

			GLuint textureGridID = 0;  // 网格纹理ID
			GLuint texturePixelID = 0; // 像素纹理ID

			GLuint smokeTexture = 0;    // 烟雾纹理ID
		};
	}
}

#endif // !__EULER_RENDERER_H__
