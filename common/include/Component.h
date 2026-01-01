#pragma once
#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <glfw3.h>

// 基类，用于组织流体/粒子系统和渲染组件

namespace Glb {
	class Component {
	public:
		int id;                  // 组件ID
		char* description;       // 组件描述

		Component() {
			id = -1;
			this->description = NULL;
		}
		Component(char* description) {
			id = -1;
			this->description = description;
		}

		virtual void shutDown() = 0;     // 关闭组件
		virtual void init() = 0;         // 初始化组件
		virtual void simulate() = 0;     // 模拟计算
		virtual GLuint getRenderedTexture() = 0;  // 获取渲染的纹理
	};
}

#endif