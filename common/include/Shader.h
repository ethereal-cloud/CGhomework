#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

namespace Glb {
    // 着色器程序类,用于管理和使用OpenGL着色器
    class Shader {
    public:
        Shader();
        ~Shader();

        // 从文件构建着色器程序
        int32_t buildFromFile(std::string& vertPath, std::string& fragPath);
        // 从文件构建包含几何着色器的着色器程序
        int32_t buildFromFile(std::string& vertPath, std::string& fragPath, std::string& geomPath);
        
        // 使用/取消使用着色器程序
        void use();
        void unUse();
        
        // 获取着色器程序ID
        GLuint getId();

        // 设置uniform变量的各种方法
        void setBool(const std::string& name, bool value);
        void setInt(const std::string& name, int value);
        void setFloat(const std::string& name, float value);
        void setVec2(const std::string& name, const glm::vec2& value);
        void setVec2(const std::string& name, float x, float y);
        void setVec3(const std::string& name, const glm::vec3& value);
        void setVec3(const std::string& name, float x, float y, float z);
        void setVec4(const std::string& name, const glm::vec4& value);
        void setVec4(const std::string& name, float x, float y, float z, float w);
        void setMat2(const std::string& name, const glm::mat2& mat);
        void setMat3(const std::string& name, const glm::mat3& mat);
        void setMat4(const std::string& name, const glm::mat4& mat);

    private:
        GLuint mId = 0; // 着色器程序ID

    };
}

#endif
