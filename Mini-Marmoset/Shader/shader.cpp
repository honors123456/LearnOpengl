#include "Shader.h"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>

namespace {
// Shader 的所有操作（包括析构）都需要当前的、可共享的 OpenGL 上下文。
QOpenGLExtraFunctions *currentGlFunctions()
{
    auto *context = QOpenGLContext::currentContext();
    if (!context)
        qFatal("Shader requires a current OpenGL context");
    return context->extraFunctions();
}
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    const std::string vertexCode = readShaderFile(vertexPath);
    const std::string fragmentCode = readShaderFile(fragmentPath);
    // 任何一步失败都只打印错误并返回无效对象（id() == 0），
    // 由调用方检查 id() 之后再使用。
    if (vertexCode.empty() || fragmentCode.empty())
        return;

    // 先分别编译顶点着色器和片段着色器
    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), vertexPath);
    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), fragmentPath);
    if (vertexShader == 0 || fragmentShader == 0)
    {
        if (vertexShader != 0)
            currentGlFunctions()->glDeleteShader(vertexShader);
        if (fragmentShader != 0)
            currentGlFunctions()->glDeleteShader(fragmentShader);
        return;
    }

    // 再把两个着色器对象链接成一个可直接使用的着色器程序
    auto *gl = currentGlFunctions();
    const unsigned int program = gl->glCreateProgram();
    gl->glAttachShader(program, vertexShader);
    gl->glAttachShader(program, fragmentShader);
    gl->glLinkProgram(program);
    if (!checkProgramLink(program))
    {
        gl->glDeleteProgram(program);
        gl->glDeleteShader(vertexShader);
        gl->glDeleteShader(fragmentShader);
        return;
    }
    m_id = program;

    // 链接完成后，着色器对象已经不再需要了
    currentGlFunctions()->glDeleteShader(vertexShader);
    currentGlFunctions()->glDeleteShader(fragmentShader);
}

Shader::Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath)
{
    const std::string vertexCode = readShaderFile(vertexPath);
    const std::string geometryCode = readShaderFile(geometryPath);
    const std::string fragmentCode = readShaderFile(fragmentPath);
    if (vertexCode.empty() || geometryCode.empty() || fragmentCode.empty())
        return;

    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), vertexPath);
    const unsigned int geometryShader = compileShader(GL_GEOMETRY_SHADER, geometryCode.c_str(), geometryPath);
    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), fragmentPath);
    if (vertexShader == 0 || geometryShader == 0 || fragmentShader == 0)
    {
        if (vertexShader != 0)
            currentGlFunctions()->glDeleteShader(vertexShader);
        if (geometryShader != 0)
            currentGlFunctions()->glDeleteShader(geometryShader);
        if (fragmentShader != 0)
            currentGlFunctions()->glDeleteShader(fragmentShader);
        return;
    }

    auto *gl = currentGlFunctions();
    const unsigned int program = gl->glCreateProgram();
    gl->glAttachShader(program, vertexShader);
    gl->glAttachShader(program, geometryShader);
    gl->glAttachShader(program, fragmentShader);
    gl->glLinkProgram(program);
    if (!checkProgramLink(program))
    {
        gl->glDeleteProgram(program);
        gl->glDeleteShader(vertexShader);
        gl->glDeleteShader(geometryShader);
        gl->glDeleteShader(fragmentShader);
        return;
    }
    m_id = program;

    currentGlFunctions()->glDeleteShader(vertexShader);
    currentGlFunctions()->glDeleteShader(geometryShader);
    currentGlFunctions()->glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    if(m_id != 0)
    {
        currentGlFunctions()->glDeleteProgram(m_id);
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
            currentGlFunctions()->glDeleteProgram(m_id);
        }

        m_id = other.m_id;
        other.m_id = 0;
    }

    return *this;
}

void Shader::use() const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUseProgram(m_id);
}

unsigned int Shader::id() const
{
    return m_id;
}

void Shader::setBool(const std::string& name, bool value) const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUniform1i(currentGlFunctions()->glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUniform1i(currentGlFunctions()->glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUniform1f(currentGlFunctions()->glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUniformMatrix3fv(currentGlFunctions()->glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUniformMatrix4fv(currentGlFunctions()->glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUniform2fv(currentGlFunctions()->glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUniform3fv(currentGlFunctions()->glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    if(m_id == 0)
        return;
    currentGlFunctions()->glUniform4fv(currentGlFunctions()->glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

std::string Shader::readShaderFile(const char* filePath)
{
    std::ifstream file(filePath);
    if(!file.is_open())
    {
        qWarning("Failed to open shader file: %s", filePath);
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::compileShader(unsigned int shaderType, const char* source, const char* debugName)
{
    const unsigned int shader = currentGlFunctions()->glCreateShader(shaderType);
    currentGlFunctions()->glShaderSource(shader, 1, &source, nullptr);
    currentGlFunctions()->glCompileShader(shader);

    int success = 0;
    currentGlFunctions()->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        char infoLog[1024] = {};
        currentGlFunctions()->glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        qWarning("Failed to compile shader: %s\n%s", debugName, infoLog);
        currentGlFunctions()->glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::checkProgramLink(unsigned int program)
{
    int success = 0;
    currentGlFunctions()->glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        char infoLog[1024] = {};
        currentGlFunctions()->glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        qWarning("Failed to link shader program:\n%s", infoLog);
        return false;
    }
    return true;
}
