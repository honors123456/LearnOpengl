#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 默认相机参数设定
const float YAW         = -90.0f; // 初始偏航角指向 -Z 轴
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class Camera {
public:
    // 相机属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // 欧拉角（Euler Angles）
    float Yaw;
    float Pitch;

    // 交互参数
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH);

    // 获取用于传递给着色器的 View 矩阵
    glm::mat4 GetViewMatrix() const;

    // 处理键盘输入 (WASD)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // 处理鼠标移动 (俯仰角 Pitch 与偏航角 Yaw)
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // 处理鼠标滚轮缩放 (FOV 调整)
    void ProcessMouseScroll(float yoffset);

private:
    // 根据当前 Pitch / Yaw 角度重新计算相机方向向量
    void updateCameraVectors();
};
