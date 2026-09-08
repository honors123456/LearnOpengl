// Shader 工具类：读取 GLSL 文件、编译 vertex/fragment shader 并链接 program。
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class Shader
{
public:
    // 创建普通 vertex + fragment program，或 vertex + geometry + fragment program。
    Shader(const char* vertexPath, const char* fragmentPath);
    Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void use() const;
    unsigned int id() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat3(const std::string& name, const glm::mat3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;
    void setVec2(const std::string& name, const glm::vec2& value)const;
    void setVec3(const std::string& name, const glm::vec3& value)const;
    void setVec4(const std::string& name, const glm::vec4& value)const;

private:
    // OpenGL program 对象句柄；uniform 设置均通过该 program 完成。
    unsigned int m_id = 0;

    static std::string readShaderFile(const char* filePath);
    static unsigned int compileShader(unsigned int shaderType, const char* source, const char* debugName);
    static void checkProgramLink(unsigned int program);
};
