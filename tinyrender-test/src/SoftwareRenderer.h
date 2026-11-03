#pragma once
#include <Eigen/Dense>
#include <vector>
#include "tgaimage.h"
#include "ParseAssimpObj.h"

struct IShader;

// CPU rendering only; no OpenGL context is needed.
class SoftwareRenderer {
public:
    SoftwareRenderer();
    // Shader must refer to this model. Existing color/depth buffers are preserved.
    void drawModel(const ParseAssimp& model, IShader& shader,
                   TGAImage& framebuffer, std::vector<float>& zbuffer);
    Eigen::Matrix4f transform() const { return projection_ * view_ * model_; }
    TGAImage creator();
    void line(int ax, int ay, int bx, int by, TGAImage& framebuffer, const TGAColor& color);
    void triangle(Eigen::Vector2f pts[3],TGAImage& framebuffer,const TGAColor& color);
    void triangle(Eigen::Vector3f pts[3],std::vector<float>& zBuffer,TGAImage& framebuffer,const TGAColor& color);
    void triangle(Eigen::Vector4f pts[3], IShader& shader,std::vector<float>& zBuffer, TGAImage& framebuffer);

    //MVP
    // Right-handed world-to-view matrix for column vectors; forward is -Z.
    // Caller must provide finite, nondegenerate camera vectors.
    Eigen::Matrix4f lookAt(Eigen::Vector3f camera,Eigen::Vector3f center,Eigen::Vector3f up);
    // OpenGL perspective projection; fovYRadians is the vertical field of view.
    Eigen::Matrix4f perspective(float fovYRadians, float aspect, float nearPlane, float farPlane);
    // OpenGL orthographic projection; near/far are distances along view-space -Z.
    Eigen::Matrix4f orthographic(float left, float right, float bottom, float top,
                               float nearPlane, float farPlane);

    //setter
    void setCameraPos(const Eigen::Vector3f& cameraPos);
    void setWorldCenter(const Eigen::Vector3f& center);
    void setWorldUp(const Eigen::Vector3f& up);
    void setLightDir(const Eigen::Vector3f& lightDir);

    void setModelMatrix(const Eigen::Matrix4f& model);
    void setViewMatrix(const Eigen::Matrix4f& view);
    void setProjectionMatrix(const Eigen::Matrix4f& projection);

    //getter
    Eigen::Vector3f getCameraPos();
    Eigen::Vector3f getWorldCenter();
    Eigen::Vector3f getWorldUp();
    Eigen::Vector3f getLightDir();

    Eigen::Matrix4f getModelMatrix();
    Eigen::Matrix4f getViewMatrix();
    Eigen::Matrix4f getProjectionMatrix();
private:
    Eigen::Vector3f camera{0,0,2};
    Eigen::Vector3f world_center{0,0,0};
    Eigen::Vector3f world_up{0,1,0};

    Eigen::Matrix4f model_;
    Eigen::Matrix4f view_;
    Eigen::Matrix4f projection_;
    Eigen::Vector3f light_dir_;
};
