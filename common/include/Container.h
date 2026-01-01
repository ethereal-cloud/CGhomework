#pragma once
#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <glad/glad.h>
#include <glfw3.h>
#include "Shader.h"
#include "Configure.h"
#include "Camera.h"

namespace Glb {
	class Container {
	public:
		void resetSize(float x, float y, float z);  // 重置容器大小
		void draw();                                // 绘制容器
		void init();                                // 初始化容器

	private:
		Shader* shader = NULL;                      // 着色器程序

		float x = 1.0f;                            // 容器x轴长度
		float y = 1.0f;                            // 容器y轴长度 
		float z = 1.0f;                            // 容器z轴长度

		GLuint VBO = 0;                            // 顶点缓冲对象
		GLuint VAO = 0;                            // 顶点数组对象

	};
}
#endif