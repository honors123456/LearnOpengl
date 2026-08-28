#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    const std::string vertexCode = readShaderFile(vertexPath);
    const std::string fragmentCode = readShaderFile(fragmentPath);

    // 先分别编译顶点着色器和片段着色器。
    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), vertexPath);
    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), fragmentPath);

    // 再把两个着色器链接成一个可使用的着色器程序。
    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShader);
    glAttachShader(m_id, fragmentShader);
    glLinkProgram(m_id);
    checkProgramLink(m_id);

    // 链接完成后，单独的 shader 对象已经不需要保留。
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath)
{
    const std::string vertexCode = readShaderFile(vertexPath);
    const std::string geometryCode = readShaderFile(geometryPath);
    const std::string fragmentCode = readShaderFile(fragmentPath);

    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), vertexPath);
    const unsigned int geometryShader = compileShader(GL_GEOMETRY_SHADER, geometryCode.c_str(), geometryPath);
    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), fragmentPath);

    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShader);
    glAttachShader(m_id, geometryShader);
    glAttachShader(m_id, fragmentShader);
    glLinkProgram(m_id);
    checkProgramLink(m_id);

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    if(m_id != 0)
    {
        glDeleteProgram(m_id);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_id(other.m_id)
{
    other.m_id = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if(this != &other)
    {
        if(m_id != 0)
        {
            glDeleteProgram(m_id);
        }

        m_id = other.m_id;
        other.m_id = 0;
    }

    return *this;
}

void Shader::use() const
{
    glUseProgram(m_id);
}

unsigned int Shader::id() const
{
    return m_id;
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) const
{
    glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

std::string Shader::readShaderFile(const char* filePath)
{
    std::ifstream file(filePath);
    if(!file.is_open())
    {
        throw std::runtime_error(std::string("Failed to open shader file: ") + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::compileShader(unsigned int shaderType, const char* source, const char* debugName)
{
    const unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char infoLog[1024] = {};
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Failed to compile shader: " << debugName << std::endl;
        std::cerr << infoLog << std::endl;
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Failed to compile shader: ") + debugName);
    }

    return shader;
}

void Shader::checkProgramLink(unsigned int program)
{
    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        char infoLog[1024] = {};
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "Failed to link shader program" << std::endl;
        std::cerr << infoLog << std::endl;
        glDeleteProgram(program);
        throw std::runtime_error("Failed to link shader program");
    }
}
