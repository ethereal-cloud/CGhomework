#include "Container.h"

namespace Glb {

	// 定义容器的顶点数据
	GLfloat vertices[] = {
		// 前面四个顶点
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// 后面四个顶点
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// 连接前后面的边的顶点
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,
	};

	void Container::resetSize(float x, float y, float z)
	{
		// 重置容器大小
		this->x = x;
		this->y = y;
		this->z = z;
		// 更新顶点数据以反映新的尺寸
		for (int i = 0; i < 48; i += 3) {
			if (vertices[i] != 0) {
				vertices[i] = x;
			}
			if (vertices[i + 1] != 0) {
				vertices[i + 1] = y;
			}
			if (vertices[i + 2] != 0) {
				vertices[i + 2] = z;
			}
		}
	}

	void Container::draw()
	{
		// 绑定顶点数组对象
		glBindVertexArray(VAO);
		
		// 设置线宽
		glLineWidth(2.0f);
		// 使用着色器程序
		shader->use();
		// 设置观察和投影矩阵
		shader->setMat4("view", Glb::Camera::getInstance().GetView());
		shader->setMat4("projection", Glb::Camera::getInstance().GetProjection());

		// 绘制前后面的边框
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glDrawArrays(GL_LINE_LOOP, 4, 4);

		// 绘制连接前后面的边
		for (int i = 8; i < 16; i += 2) {
			glDrawArrays(GL_LINES, i, 2);
		}

		// 解绑顶点数组对象
		glBindVertexArray(0);
	}

	void Container::init() {
		// 加载并编译着色器
		std::string vertShaderPath = shaderPath + "/Line.vert";
		std::string fragShaderPath = shaderPath + "/Line.frag";
		shader = new Shader();
		shader->buildFromFile(vertShaderPath, fragShaderPath);

		// 创建并设置顶点缓冲对象
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// 设置顶点属性指针
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}
}