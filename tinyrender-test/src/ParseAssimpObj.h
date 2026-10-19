#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <Eigen/Dense>

//.obj文件，每行开头的前缀标识符代表了数据的类型
// v 顶点几何坐标行 v -0.0005 0.2447 0.1783
// vt 顶点纹理坐标行 vt 0.500 1.000
// vn 顶点法线行 vn 0.000 0.000 1.000
// f 面/多边形 f 1/1/1 2/2/2 3/3/3
// # 注释 # This is a comment

//三角形
struct FaceVertex
{
    int vertex_index;
    int texture_index;
    int normal_index;
};

inline Eigen::Vector3f barycentric(Eigen::Vector2f A, Eigen::Vector2f B, Eigen::Vector2f C, Eigen::Vector2f P)
{
    Eigen::Vector3f vx = { C.x() - A.x(), B.x() - A.x(), A.x() - P.x() };
    Eigen::Vector3f vy = { C.y() - A.y(), B.y() - A.y(), A.y() - P.y() };

    Eigen::Vector3f u = vx.cross(vy);

    if (std::abs(u.z()) < 1e-2) {
        return { -1.0f, 1.0f, 1.0f };
    }

    // 归一化解出权重
    float v = u.y() / u.z();
    float w = u.x() / u.z();
    float alpha = 1.0f - (v + w);

    return { alpha, v, w };
}

class ParseAssimp{
public:
    std::vector<Eigen::Vector3f> verts;       //顶点数据
    std::vector<Eigen::Vector2f> uvs;         //纹理数据
    std::vector<Eigen::Vector3f> normals;     //法线数据
    std::vector<std::vector<FaceVertex>> faces; //三角形面数据

    Eigen::Vector3f vert(int iface,int nVert) const
    {
        if (iface < 0 || static_cast<std::size_t>(iface) >= faces.size())
            throw std::out_of_range("face index out of range");

        const auto& face = faces.at(iface);
        if (nVert < 0 || static_cast<std::size_t>(nVert) >= face.size())
            throw std::out_of_range("vertex index in face out of range");

        const int index = face.at(nVert).vertex_index;
        if (index < 0 || static_cast<std::size_t>(index) >= verts.size())
            throw std::out_of_range("OBJ vertex index out of range");

        return verts[index];
    }

    Eigen::Vector2f uv(int iface, int nVert) const
    {
        const auto& vertex = faces.at(iface).at(nVert);
        return uvs.at(vertex.texture_index);
    }

    Eigen::Vector3f normal(int iface, int nVert) const
    {
        const auto& vertex = faces.at(iface).at(nVert);
        return normals.at(vertex.normal_index);
    }

    

    bool loadData(const std::string& fileName)
    {
        std::fstream fs(fileName);
        if(!fs.is_open())
        {
            std::cerr<<"Failed to open "<<fileName<<std::endl;
            return false;
        }

        std::string line;
        while(std::getline(fs,line))
        {
            if(line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if(prefix == "v")
            {
                Eigen::Vector3f vert;
                iss >> vert.x() >> vert.y() >> vert.z();
                verts.push_back(vert);
            }
            else if(prefix == "vt")
            {
                Eigen::Vector2f uv;
                iss >> uv.x() >> uv.y();
                uvs.push_back(uv);
            }
            else if(prefix == "vn")
            {
                Eigen::Vector3f normal;
                iss >> normal.x() >> normal.y() >> normal.z();
                normals.push_back(normal);
            }
            else if(prefix == "f")
            {
                std::vector<FaceVertex> face;
                std::string token;

                while(iss >> token)
                {
                    FaceVertex fv = {-1,-1,-1};
                    int v = -1, vt = -1, vn = -1;
                    if (sscanf(token.c_str(), "%d/%d/%d", &v, &vt, &vn) == 3) {
                        fv.vertex_index  = v - 1;
                        fv.texture_index = vt - 1;
                        fv.normal_index = vn - 1;
                    } 
                    else if (sscanf(token.c_str(), "%d//%d", &v, &vn) == 2) {
                        // 处理无纹理坐标的情况: v//vn
                        fv.vertex_index  = v - 1;
                        fv.normal_index = vn - 1;
                    } 
                    else if (sscanf(token.c_str(), "%d/%d", &v, &vt) == 2) {
                        // 处理无发线的情况: v/vt
                        fv.vertex_index  = v - 1;
                        fv.texture_index = vt - 1;
                    } 
                    else if (sscanf(token.c_str(), "%d", &v) == 1) {
                        // 纯顶点: v
                        fv.vertex_index = v - 1;
                    }
                    face.push_back(fv);
                }

                if (face.size() >= 3) {
                    for (size_t i = 1; i + 1 < face.size(); ++i) {
                        faces.push_back({ face[0], face[i], face[i + 1] });
                    }
                }
            }
        }

        return true;
    }
};


