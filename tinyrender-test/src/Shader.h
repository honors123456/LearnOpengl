#pragma once

#include <Eigen/Eigen>
#include <iostream>
#include <algorithm>
#include "tgaimage.h"
#include "ParseAssimpObj.h"

struct IShader
{
    virtual ~IShader() {

    }

    //顶点着色器：输入当前面片索引，iface和当前面的顶点索引
    //返回经过 MVP 变换后的 4D 齐次裁剪坐标
    // Call for vertices 0, 1, 2 of one face, then rasterize that face immediately.
    virtual Eigen::Vector4f vertex(int iface,int nidxVert) = 0;

    //片段着色器：输入当前像素的重心坐标权重，输出最终颜色
    // bar contains perspective-correct weights. true = draw; false = discard.
    virtual bool fragment(Eigen::Vector3f bar,TGAColor &color) = 0;
};


class TexShader : public IShader
{
public:
    // Model and textures must outlive this shader. Loading belongs to the caller.
    TexShader(const ParseAssimp& m, const Eigen::Vector3f& l,
              const Eigen::Matrix4f& t, const TGAImage& diffuse, const TGAImage& normal)
        : model(m), diffuseMap(diffuse), normalMap(normal), light_dir(l), transform(t)
    {
    }

    ~TexShader()
    {

    }

    virtual Eigen::Vector4f vertex(int iface,int nidxvert) override
    {
        //获取顶点数据
        Eigen::Vector3f vert = model.vert(iface,nidxvert);

        //模型空间坐标转换为裁剪空间坐标
        Eigen::Vector4f glVertex = transform * Eigen::Vector4f(vert.x(),vert.y(),vert.z(),1.0f);

        //获取纹理坐标
        triangle_texture_pos[nidxvert] = model.uv(iface,nidxvert);

        return glVertex;
    }

    virtual bool fragment(Eigen::Vector3f bar, TGAColor &color) override
    {
        //根据归一化权重因子计算出精确的纹理坐标
        Eigen::Vector2f uv = triangle_texture_pos[0] * bar.x() + triangle_texture_pos[1] * bar.y() + triangle_texture_pos[2] * bar.z();

        //贴图采样
        //漫反射贴图是平面的基础颜色
        //法线贴图不是颜色，是每个像素的法向量
        if (!uv.allFinite())
            return false;
            
        const float u = std::clamp(uv.x(), 0.0f, 1.0f);
        const float v = 1.0f - std::clamp(uv.y(), 0.0f, 1.0f);
        // OBJ 的 V 原点在底部，解码后的图片原点在顶部；端点限制到有效像素。
        const int diffuse_x = std::min(static_cast<int>(u * diffuseMap.width()), diffuseMap.width() - 1);
        const int diffuse_y = std::min(static_cast<int>(v * diffuseMap.height()), diffuseMap.height() - 1);
        const int normal_x = std::min(static_cast<int>(u * normalMap.width()), normalMap.width() - 1);
        const int normal_y = std::min(static_cast<int>(v * normalMap.height()), normalMap.height() - 1);
        TGAColor diffuseColor = diffuseMap.get(diffuse_x, diffuse_y);
        TGAColor normalColor = normalMap.get(normal_x, normal_y);

        //[-1,1]全部映射到[0,255],反向得到正确的法线向量
        Eigen::Vector3f normal(
            normalColor[2] / 255.0f * 2 - 1.0f,
            normalColor[1] / 255.0f * 2 - 1.0f,
            normalColor[0] / 255.0f * 2 - 1.0f
        );

        normal.normalize();

        //计算漫反射光照因子
        float diffuse = std::max(0.0f,normal.dot(light_dir.normalized()));

        color.bgra[0] = diffuseColor.bgra[0] * diffuse;
        color.bgra[1] = diffuseColor.bgra[1] * diffuse;
        color.bgra[2] = diffuseColor.bgra[2] * diffuse;
        color.bgra[3] = 255;

        return true;
    }

public:
    //模型网格解析数据
    const ParseAssimp& model;
    //漫反射贴图
    const TGAImage& diffuseMap;
    //法线贴图
    const TGAImage& normalMap;
    //光源
    Eigen::Vector3f light_dir;
    //变换矩阵
    Eigen::Matrix4f transform;
    //用于顶点到片元的属性传递
    Eigen::Vector2f triangle_texture_pos[3];

private:

};

class DepthShader : public IShader
{
public:
    DepthShader(const ParseAssimp& m, const Eigen::Matrix4f& t)
        :model(m),light_transform(t)
    {

    }

    ~DepthShader()
    {

    }

    virtual Eigen::Vector4f vertex(int iface,int nidxvert) override
    {
        //获取三角形顶点的局部坐标
        Eigen::Vector3f v = model.vert(iface,nidxvert);

        //获取三角形顶点的裁剪坐标
        Eigen::Vector4f gl_vertex = light_transform * Eigen::Vector4f(v.x(),v.y(),v.z(),1.0f);

        //获取三角形顶点的ndc坐标
        triangle_ndc_pos[nidxvert] = gl_vertex.head<3>() / gl_vertex.w();

        return gl_vertex;
    }

    virtual bool fragment(Eigen::Vector3f bar, TGAColor &color) override
    {
        color = TGAColor{};
        return true;
    }


public:
    //模型网格解析数据
    const ParseAssimp& model;
    //光线空间变换矩阵
    Eigen::Matrix4f light_transform;
    //用于顶点到片元的属性传递，片元的ndc坐标
    Eigen::Vector3f triangle_ndc_pos[3];
};

class ShadowShader : public IShader
{
public:
    ShadowShader(
        const ParseAssimp& m,
        const Eigen::Vector3f& light_dir,
        const Eigen::Matrix4f& camera_transform,
        const Eigen::Matrix4f& light_transform,
        const std::vector<float>& sbuf,
        int w,int h,
        const TGAImage& diffuseMap,
        const TGAImage& normalMap
    )
        :model(m),
        light_dir(light_dir.normalized()),
        camera_transform(camera_transform),
        light_transform(light_transform),
        shadow_buffer(sbuf),
        width(w),
        height(h),
        diffuseMap(diffuseMap),
        normalMap(normalMap)
    {

    }

    ~ShadowShader()
    {

    }

    virtual Eigen::Vector4f vertex(int iface,int nidxvert) override
    {
        //获取纹理坐标
        triangle_texture_pos[nidxvert] = model.uv(iface,nidxvert);

        //顶点的局部坐标
        Eigen::Vector3f v = model.vert(iface,nidxvert);

        //顶点的世界坐标,这个项目默认模型在世界原点，model矩阵就是单位矩阵
        Eigen::Matrix3f m = Eigen::Matrix3f::Identity();
        triangle_world_pos[nidxvert] = m * Eigen::Vector3f(v.x(),v.y(),v.z());

        //返回裁剪空间坐标
        return camera_transform * Eigen::Vector4f(v.x(),v.y(),v.z(),1.0f);
    }

    //两个视角来比较得到阴影位置，摄影机视角和光源视角
    virtual bool fragment(Eigen::Vector3f bar, TGAColor &color) override
    {
        //插值纹理坐标和空间坐标
        Eigen::Vector2f uv = triangle_texture_pos[0] * bar.x() + triangle_texture_pos[1] * bar.y() + triangle_texture_pos[2] * bar.z();
        Eigen::Vector3f world_pos = triangle_world_pos[0] * bar.x() + triangle_world_pos[1] * bar.y() + triangle_world_pos[2] * bar.z();

        //光源视角下的ndc坐标
        Eigen::Vector4f light_pos = light_transform * Eigen::Vector4f(world_pos.x(),world_pos.y(),world_pos.z(),1.0f);
        Eigen::Vector3f light_ndc_pos = light_pos.head<3>() / light_pos.w();

        //阴影判断
        float shadow = 1.0f;
        int lx = static_cast<int>(
            (light_ndc_pos.x() + 1.0f) * 0.5f * (width - 1)
        );

        int ly = static_cast<int>(
            (1.0f-light_ndc_pos.y()) * 0.5f * (height - 1)
        );

        if(lx >=0 && lx<width && ly>=0 && ly<height)
        {
            int index = lx + ly * width;
            float current_depth = light_ndc_pos.z();

            constexpr float bias = 0.002f;
            //如果当前片元的深度比光源视角下的最近深度还要深，就说明被遮蔽了
            if(shadow_buffer[index] + bias < current_depth)
            {
                shadow = 0.15f;//处于阴影中，只保留环境光
            }
        }

        //材质采样与法线贴图
        const float u = std::clamp(uv.x(),0.0f,1.0f);
        const float v = 1.0f - std::clamp(uv.y(),0.0f,1.0f);

        int dx = std::min(static_cast<int>(u * diffuseMap.width()),diffuseMap.width() - 1);
        int dy = std::min(static_cast<int>(v * diffuseMap.height()),diffuseMap.height() - 1);
        TGAColor diffuseColor = diffuseMap.get(dx,dy);

        int nx = std::min(static_cast<int>(u * normalMap.width()),normalMap.width() -1);
        int ny = std::min(static_cast<int>(v * normalMap.height()),normalMap.height() -1);
        TGAColor normalColor = normalMap.get(nx,ny);

        //这里只是使用TGAColor来存储法线，用于微表面贴图
        Eigen::Vector3f N(
            normalColor[2] / 255.0f * 2.0f - 1.0f,
            normalColor[1] / 255.0f * 2.0f - 1.0f,
            normalColor[0] / 255.0f * 2.0f - 1.0f
        );
        N.normalize();

        //计算光照和阴影衰减因子
        float diffuse = std::max(0.0f,N.dot(light_dir));
        float ambient = 0.1f;
        float light_intensity = ambient + diffuse * shadow;

        //贴图
        for(int i=0;i<3;i++)
        {
            float c = diffuseColor.bgra[i] * light_intensity;
            color.bgra[i] = static_cast<std::uint8_t>(std::clamp(c, 0.0f, 255.0f));
        }
        color.bgra[3] = 255;
        return true;
    }

public:
    const ParseAssimp& model;
    Eigen::Vector3f light_dir;
    Eigen::Matrix4f camera_transform;
    Eigen::Matrix4f light_transform;
    const std::vector<float>& shadow_buffer;
    int width,height;
    const TGAImage& diffuseMap;
    const TGAImage& normalMap;
    Eigen::Vector2f triangle_texture_pos[3];
    Eigen::Vector3f triangle_world_pos[3];
};
