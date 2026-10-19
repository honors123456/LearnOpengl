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
        Eigen::Vector4f glVertex = transform * Eigen::Vector4f(vert.x(),vert.y(),vert.z(),1.0f);

        //获取纹理坐标
        varying_uv[nidxvert] = model.uv(iface,nidxvert);

        return glVertex;
    }

    virtual bool fragment(Eigen::Vector3f bar, TGAColor &color) override
    {
        //根据归一化权重因子计算出精确的纹理坐标
        Eigen::Vector2f uv = varying_uv[0] * bar.x() + varying_uv[1] * bar.y() + varying_uv[2] * bar.z();

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
    Eigen::Vector2f varying_uv[3];

private:

};
