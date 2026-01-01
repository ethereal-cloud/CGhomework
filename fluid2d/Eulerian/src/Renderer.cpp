#include "fluid2d/Eulerian/include/Renderer.h"
#include "stb_image.h"

namespace FluidSimulation
{
	namespace Eulerian2d
	{
		// 顶点数据:位置、纹理坐标和颜色
		float vertices[]{
			// position	//texcood   //color
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

		// 索引数据:定义两个三角形
		unsigned int indices[] = {
			0, 1, 2, // first triangle
			0, 2, 3	 // second triangle
		};

		// 构造函数:初始化OpenGL渲染资源
		Renderer::Renderer()
		{
			// 加载着色器
			extern std::string shaderPath;
			std::string vertShaderPath = shaderPath + "/DrawSmokeTexture2d.vert";
			std::string fragShaderPath = shaderPath + "/DrawSmokeTexture2d.frag";
			shader = new Glb::Shader();
			shader->buildFromFile(vertShaderPath, fragShaderPath);

			// 创建VAO、VBO和EBO
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			// 创建帧缓冲对象(FBO)
			glGenFramebuffers(1, &FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);

			// 创建并设置网格纹理
			glGenTextures(1, &textureGridID);
			glBindTexture(GL_TEXTURE_2D, textureGridID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			// 将纹理附加到帧缓冲
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureGridID, 0);

			// 创建并设置渲染缓冲对象(RBO)
			glGenRenderbuffers(1, &RBO);
			glBindRenderbuffer(GL_RENDERBUFFER, RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, imageWidth, imageHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			// 将RBO附加到帧缓冲
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				Glb::Logger::getInstance().addLog("Error: Framebuffer is not complete!");
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// 设置OpenGL状态
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glViewport(0, 0, imageWidth, imageHeight);

			loadTexture();
		}

		// 加载烟雾纹理
		void Renderer::loadTexture()
		{
			glGenTextures(1, &smokeTexture);
			glBindTexture(GL_TEXTURE_2D, smokeTexture);
			
			// 设置纹理参数
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// 加载纹理图像
			int width, height, nrChannels;
			unsigned char *data = stbi_load((picturePath + "/white.png").c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
			{
				Glb::Logger::getInstance().addLog("Error: Failed to load texture!");
			}
			stbi_image_free(data);
		}

		// 绘制MAC网格
		void Renderer::draw(MACGrid2d &mGrid)
		{
			// 像素模式绘制
			if (Eulerian2dPara::drawModel == 0)
			{
				std::vector<float> imageData;

				// 遍历每个像素
				for (int j = 1; j <= imageHeight; j++)
				{
					for (int i = 1; i <= imageWidth; i++)
					{
						float pt_x = i * mGrid.mD.mMax[0] / (imageWidth);
						float pt_y = j * mGrid.mD.mMax[1] / (imageHeight);
						glm::vec2 pt(pt_x, pt_y);
						
						// 如果是固体,设置为绿色
						if (mGrid.inSolid(pt)) {
							imageData.push_back(0);
							imageData.push_back(1);
							imageData.push_back(0);
						}
						else {
							// 否则根据密度设置颜色
							glm::vec4 color = mGrid.getRenderColor(pt);
							imageData.push_back(color.x * Eulerian2dPara::contrast);
							imageData.push_back(color.y * Eulerian2dPara::contrast);
							imageData.push_back(color.z * Eulerian2dPara::contrast);
						}
					}
				}

				// 创建并设置像素纹理
				glGenTextures(1, &texturePixelID);
				glBindTexture(GL_TEXTURE_2D, texturePixelID);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_FLOAT, imageData.data());
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			// 网格模式绘制
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, FBO);

				glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				float dt_x = mGrid.mU.mMax[0] / (Eulerian2dPara::gridNum);
				float dt_y = mGrid.mV.mMax[1] / (Eulerian2dPara::gridNum);

				// 遍历每个网格单元
				for (int j = Eulerian2dPara::gridNum; j >= 1; j--)
				{
					for (int i = Eulerian2dPara::gridNum; i >= 1; i--)
					{
						// 计算网格单元的顶点位置和密度值
						float pt_x = i * mGrid.mD.mMax[0] / (Eulerian2dPara::gridNum);
						float pt_y = j * mGrid.mD.mMax[1] / (Eulerian2dPara::gridNum);

						vertices[0] = pt_x - dt_x / 2;
						vertices[1] = pt_y - dt_y / 2;
						vertices[4] = mGrid.getDensity(glm::vec2(vertices[0], vertices[1]));

						vertices[5] = pt_x + dt_x / 2;
						vertices[6] = pt_y - dt_y / 2;
						vertices[9] = mGrid.getDensity(glm::vec2(vertices[5], vertices[6]));

						vertices[10] = pt_x + dt_x / 2;
						vertices[11] = pt_y + dt_y / 2;
						vertices[14] = mGrid.getDensity(glm::vec2(vertices[10], vertices[11]));

						vertices[15] = pt_x - dt_x / 2;
						vertices[16] = pt_y + dt_y / 2;
						vertices[19] = mGrid.getDensity(glm::vec2(vertices[15], vertices[16]));

						// 转换到NDC坐标系
						for (int k = 0; k <= 15; k += 5)
						{
							vertices[k] = (vertices[k] / mGrid.mU.mMax[0]) * 2 - 1;
								vertices[k + 1] = (vertices[k + 1] / mGrid.mV.mMax[1]) * 2 - 1;
						}

						// 设置顶点数据
						glBindVertexArray(VAO);
						glBindBuffer(GL_ARRAY_BUFFER, VBO);
						glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
						
						// 设置顶点属性
						glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
						glEnableVertexAttribArray(0);
						glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
						glEnableVertexAttribArray(1);
						glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(4 * sizeof(float)));
						glEnableVertexAttribArray(2);
						glBindVertexArray(0);

						// 绘制网格单元
						shader->use();
						glBindTexture(GL_TEXTURE_2D, smokeTexture);
						glUniform1i(glGetUniformLocation(shader->getId(), "mTexture"), 0);
						shader->setFloat("contrast", Eulerian2dPara::contrast);
						glBindVertexArray(VAO);
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					}
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}

		// 获取渲染结果的纹理ID
		GLuint Renderer::getTextureID()
		{
			if (Eulerian2dPara::drawModel == 0)
			{
				return texturePixelID;  // 像素模式
			}
			return textureGridID;  // 网格模式
		}
	}
}