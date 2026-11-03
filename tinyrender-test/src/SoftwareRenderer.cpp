#include "SoftwareRenderer.h"
#include "Shader.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>

TGAImage SoftwareRenderer::creator()
{
    constexpr int width  = 256;
    constexpr int height = 256;

    TGAImage framebuffer(width,height,TGAImage::RGB);

    return framebuffer;
}

SoftwareRenderer::SoftwareRenderer()
{
    const float radius = 2.0f;
    const float angle = -30.0f * 3.14159265f / 180.0f;

    camera.x() = radius * std::sin(angle);
    camera.z() = radius * std::cos(angle);

    model_ = Eigen::Matrix4f::Identity();
    view_ = lookAt(camera,world_center,world_up);
    projection_ = perspective(1.0472f, 1.0f, 0.1f, 100.0f);
}

void SoftwareRenderer::drawModel(const ParseAssimp& model, IShader& shader, TGAImage& framebuffer, std::vector<float>& zbuffer)
{
    // Each face must finish vertex preparation and rasterization before the next face.
    for (int iface = 0; iface < static_cast<int>(model.faces.size()); ++iface) {
        Eigen::Vector4f clip_coords[3];
        for (int j = 0; j < 3; ++j)
            clip_coords[j] = shader.vertex(iface, j);
        triangle(clip_coords, shader, zbuffer, framebuffer);
    }
}

// ------------------------- 1.直线算法 -------------------------------------
void SoftwareRenderer::line(int ax, int ay, int bx, int by, TGAImage &framebuffer, const TGAColor &color)
{
    //Bresham算法
    const std::int64_t dx = std::abs(static_cast<std::int64_t>(bx) - ax);
    const std::int64_t dy = -std::abs(static_cast<std::int64_t>(by) - ay);
    const int sx = ax < bx ? 1 : -1;
    const int sy = ay < by ? 1 : -1;
    std::int64_t error = dx + dy;

    while (true) {
        framebuffer.set(ax, ay, color);
        if (ax == bx && ay == by)
            break;

        const std::int64_t twice_error = 2 * error;

        if (twice_error >= dy) {
            error += dy;
            ax += sx;
        }

        if (twice_error <= dx) {
            error += dx;
            ay += sy;
        }
    }
}

// ---------------------------- 2.三角形光栅化 & -----------------------------
void SoftwareRenderer::triangle(Eigen::Vector2f pts[3], TGAImage &framebuffer, const TGAColor &color)
{
    //光栅化填充，包围盒算法
    //计算包围盒
    float min_x = std::min({pts[0].x(), pts[1].x(), pts[2].x()});
    float max_x = std::max({pts[0].x(), pts[1].x(), pts[2].x()});
    float min_y = std::min({pts[0].y(), pts[1].y(), pts[2].y()});
    float max_y = std::max({pts[0].y(), pts[1].y(), pts[2].y()});

    //把包围盒的数据限制到图片区域内
    int bbox_min_x = std::max(0, (int)std::floor(min_x));
    int bbox_max_x = std::min(framebuffer.width() - 1, (int)std::ceil(max_x));
    int bbox_min_y = std::max(0, (int)std::floor(min_y));
    int bbox_max_y = std::min(framebuffer.height() - 1, (int)std::ceil(max_y));

    //遍历包围盒内的每个像素
    for(int x = bbox_min_x; x<= bbox_max_x ; x++)
    {
        for(int y = bbox_min_y; y<= bbox_max_y ; y++)
        {
            //P就是包围盒内的每个像素的几何中心坐标
            Eigen::Vector2f P = {(float)x+0.5f,(float)y+0.5f};
            //点P在三角形内的归一化权重系数
            Eigen::Vector3f bc_screen = barycentric(pts[0],pts[1],pts[2],P);

            if(bc_screen.x() >= 0 && bc_screen.y() >= 0 && bc_screen.z() >= 0)
            {
                framebuffer.set(x,y,color);
            }
        }
    }
}
//--------------------------- 3.深度测试
void SoftwareRenderer::triangle(Eigen::Vector3f pts[3], std::vector<float> &zBuffer, TGAImage &framebuffer, const TGAColor &color)
{
    //计算包围盒
    float min_x = std::min({pts[0].x(), pts[1].x(), pts[2].x()});
    float max_x = std::max({pts[0].x(), pts[1].x(), pts[2].x()});
    float min_y = std::min({pts[0].y(), pts[1].y(), pts[2].y()});
    float max_y = std::max({pts[0].y(), pts[1].y(), pts[2].y()});

    int bbox_min_x = std::max(0,(int)std::floor(min_x));
    int bbox_max_x = std::min(framebuffer.width() - 1 ,(int)std::ceil(max_x));
    int bbox_min_y = std::max(0,(int)std::floor(min_y));
    int bbox_max_y = std::min(framebuffer.height() - 1,(int)std::ceil(max_y));

    //遍历并填充三角形
    for(int i=bbox_min_x;i<=bbox_max_x;i++)
    {
        for(int j=bbox_min_y;j<=bbox_max_y;j++)
        {
            //取出2D平面的三角形顶点坐标
            Eigen::Vector2f pts2d[3] = {
                {pts[0].x(),pts[0].y()},
                {pts[1].x(),pts[1].y()},
                {pts[2].x(),pts[2].y()}
            };

            //计算像素的理论中心坐标
            Eigen::Vector2f P = {(float)i+0.5f,(float)j+0.5f};

            //判定点是否在三角形内部，计算归一化权重因子
            Eigen::Vector3f bc_screen = barycentric(pts2d[0], pts2d[1], pts2d[2], P);
            
            if(bc_screen.x() < 0 || bc_screen.y() < 0 || bc_screen.z() < 0)
                continue;

            //计算点P的深度值z
            float P_z = bc_screen.x() * pts[0].z() + bc_screen.y() * pts[1].z() + bc_screen.z() * pts[2].z();

            //比较深度深度信息，只绘制近的像素
            //perspective() 使用的是 OpenGL 传统深度映射：近裁剪面映射到 NDC 的 -1，远裁剪面映射到 +1
            int idx = i + j * framebuffer.width();
            if(P_z < zBuffer[idx])
            {
                zBuffer[idx] = P_z;
                framebuffer.set(i,j,color);
            }
        }
    }
}

void SoftwareRenderer::triangle(Eigen::Vector4f pts[3], IShader &shader, std::vector<float> &zBuffer, TGAImage &framebuffer)
{
    const int width = framebuffer.width();
    const int height = framebuffer.height();
    if (width <= 0 || height <= 0 ||
        zBuffer.size() < static_cast<std::size_t>(width) * height)
        return;

    //计算裁剪空间到屏幕空间的坐标
    Eigen::Vector3f screen[3];
    Eigen::Vector2f screen_xy[3];
    float screen_w[3];
    for (int k = 0; k < 3; ++k) {
        if (!pts[k].allFinite() || pts[k].w() <= 1e-6f)
            return;

        screen_w[k] = pts[k].w();

        //ndc坐标
        const float ndc_x = pts[k].x() / pts[k].w();
        const float ndc_y = pts[k].y() / pts[k].w();
        const float ndc_z = pts[k].z() / pts[k].w();

        //ndc转换为图片空间坐标
        screen[k] = {
            (ndc_x + 1.0f) * 0.5f * (width - 1),
            (1.0f - ndc_y) * 0.5f * (height - 1),
            ndc_z
        };

        if (!screen[k].allFinite())
            return;

        //图片xy坐标
        screen_xy[k] = screen[k].head<2>();
    }

    //计算包围盒
    const float min_x = std::max(0.0f, std::min({screen[0].x(), screen[1].x(), screen[2].x()}));
    const float max_x = std::min(float(width - 1), std::max({screen[0].x(), screen[1].x(), screen[2].x()}));
    const float min_y = std::max(0.0f, std::min({screen[0].y(), screen[1].y(), screen[2].y()}));
    const float max_y = std::min(float(height - 1), std::max({screen[0].y(), screen[1].y(), screen[2].y()}));
    
    if (min_x > max_x || min_y > max_y)
        return;

    int bbox_min_x = std::max(0,(int)std::floor(min_x));
    int bbox_max_x = std::min(framebuffer.width() - 1 ,(int)std::ceil(max_x));
    int bbox_min_y = std::max(0,(int)std::floor(min_y));
    int bbox_max_y = std::min(framebuffer.height() - 1,(int)std::ceil(max_y));

    for(int x=bbox_min_x;x<=bbox_max_x;x++)
    {
        for(int y=bbox_min_y;y<=bbox_max_y;y++)
        {
            //计算像素的理论中心坐标
            Eigen::Vector2f P = {(float)x+0.5f,(float)y+0.5f};

            //计算每个像素归一化权重因子，屏幕重心权重 bar
            const Eigen::Vector3f bar = barycentric(screen_xy[0], screen_xy[1], screen_xy[2], P);
            if (bar.minCoeff() < 0.0f)
                continue;

            //计算每个像素对应纹理的权重因子
            //已经有屏幕重心权重 bar，但透视投影会改变比例，直接用 bar 插值 UV 可能导致纹理失真
            //屏幕权重因子为什么可以除以裁剪空间的w，得到屏幕重心权重？
            Eigen::Vector3f weights(
                bar.x() / screen_w[0],
                bar.y() / screen_w[1],
                bar.z() / screen_w[2]
            );

            const float sum = weights.sum();
            if (!weights.allFinite() || !std::isfinite(sum) || sum <= 0.0f)
                continue;
            weights /= sum;

            //计算每个像素点的深度信息
            const float depth = bar.x() * screen[0].z() + bar.y() * screen[1].z() + bar.z() * screen[2].z();
            int idx = x + y * framebuffer.width();
            if (!std::isfinite(depth) || depth < -1.0f || depth > 1.0f || depth >= zBuffer[idx])
               continue; 

            //获取颜色
            TGAColor color;
            if (shader.fragment(weights, color)) {
                framebuffer.set(x, y, color);
                zBuffer[idx] = depth;
            }
        }
    }
}

// ---------------------------- 4.视图空间转换 -------------------------
//世界空间转为视觉空间
Eigen::Matrix4f SoftwareRenderer::lookAt(Eigen::Vector3f camera, Eigen::Vector3f center, Eigen::Vector3f up)
{
    if (!camera.allFinite() || !center.allFinite() || !up.allFinite() ||
        (camera - center).norm() == 0.0f || up.norm() == 0.0f)
        std::cerr << "lookAt requires finite, nondegenerate camera vectors" << std::endl;

    Eigen::Vector3f z = (camera - center).normalized();
    Eigen::Vector3f x = up.normalized().cross(z);
    if (x.norm() <= 1e-6f)
        std::cerr << "lookAt up must not be parallel to the view direction" <<std::endl;

    x.normalize();

    Eigen::Vector3f y = z.cross(x);

    Eigen::Matrix4f res = Eigen::Matrix4f::Identity();

    res.block<1, 3>(0, 0) = x.transpose();
    res.block<1, 3>(1, 0) = y.transpose();
    res.block<1, 3>(2, 0) = z.transpose();

    res(0, 3) = -x.dot(camera);
    res(1, 3) = -y.dot(camera);
    res(2, 3) = -z.dot(camera);

    return res;
}

//视觉空间转换到 OpenGL 裁剪空间；透视除法由后续 GPU 流程执行。
Eigen::Matrix4f SoftwareRenderer::perspective(float fovYRadians, float aspect, float nearPlane, float farPlane)
{
    constexpr float pi = 3.14159265358979323846f;
    if (!std::isfinite(fovYRadians) || !std::isfinite(aspect) || !std::isfinite(nearPlane) || !std::isfinite(farPlane) || fovYRadians <= 0.0f || fovYRadians >= pi || aspect <= 0.0f ||
    nearPlane <= 0.0f || farPlane <= nearPlane)
        throw std::invalid_argument("perspective requires 0 < fov < pi, aspect > 0, and 0 < near < far");

    const float half_fov_tangent = std::tan(fovYRadians * 0.5f);
    Eigen::Matrix4f projection = Eigen::Matrix4f::Zero();
    projection(0, 0) = 1.0f / (aspect * half_fov_tangent);
    projection(1, 1) = 1.0f / half_fov_tangent;
    projection(2, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
    projection(2, 3) = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    projection(3, 2) = -1.0f;
    return projection;
}

Eigen::Matrix4f SoftwareRenderer::orthographic(float left, float right, float bottom, float top,
                                             float nearPlane, float farPlane)
{
    if (!std::isfinite(left) || !std::isfinite(right) || !std::isfinite(bottom) ||
        !std::isfinite(top) || !std::isfinite(nearPlane) || !std::isfinite(farPlane) ||
        left >= right || bottom >= top || nearPlane < 0.0f || farPlane <= nearPlane)
        throw std::invalid_argument("orthographic requires left < right, bottom < top, and 0 <= near < far");

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    projection(0, 0) = 2.0f / (right - left);
    projection(1, 1) = 2.0f / (top - bottom);
    projection(2, 2) = -2.0f / (farPlane - nearPlane);
    projection(0, 3) = -(right + left) / (right - left);
    projection(1, 3) = -(top + bottom) / (top - bottom);
    projection(2, 3) = -(farPlane + nearPlane) / (farPlane - nearPlane);
    return projection;
}

void SoftwareRenderer::setCameraPos(const Eigen::Vector3f &cameraPos)
{
    camera = cameraPos;
}

void SoftwareRenderer::setWorldCenter(const Eigen::Vector3f &center)
{
    world_center = center;
}

void SoftwareRenderer::setWorldUp(const Eigen::Vector3f &up)
{
    world_up = up;
}

void SoftwareRenderer::setLightDir(const Eigen::Vector3f &lightDir)
{
    light_dir_ = lightDir;
}

void SoftwareRenderer::setModelMatrix(const Eigen::Matrix4f &model)
{
    model_ = model;
}

void SoftwareRenderer::setViewMatrix(const Eigen::Matrix4f &view)
{
    view_ = view;
}

void SoftwareRenderer::setProjectionMatrix(const Eigen::Matrix4f &projection)
{
    projection_ = projection;
}

Eigen::Vector3f SoftwareRenderer::getCameraPos()
{
    return camera;
}

Eigen::Vector3f SoftwareRenderer::getWorldCenter()
{
    return world_center;
}

Eigen::Vector3f SoftwareRenderer::getWorldUp()
{
    return world_up;
}

Eigen::Vector3f SoftwareRenderer::getLightDir()
{
    return light_dir_;
}

Eigen::Matrix4f SoftwareRenderer::getModelMatrix()
{
    return model_;
}

Eigen::Matrix4f SoftwareRenderer::getViewMatrix()
{
    return view_;
}

Eigen::Matrix4f SoftwareRenderer::getProjectionMatrix()
{
    return projection_;
}
