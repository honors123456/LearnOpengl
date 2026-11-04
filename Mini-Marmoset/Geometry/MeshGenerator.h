#pragma once

#include <glm/glm.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace marmoset {

// CPU data only. Use sizeof(Vertex) and offsetof() when configuring a VAO.
// CPU 顶点布局：位置、法线和 UV；GPU 上传和属性偏移由调用方处理。
struct Vertex {
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f};
    glm::vec2 uv{0.0f};
};

// Linear RGB; emissive may exceed 1 for HDR. No textures or GL handles.
// 线性空间的数值 PBR 材质，不包含纹理或 GL 句柄。
struct Material {
    glm::vec3 albedo{0.5f};
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    glm::vec3 emissive{0.0f};
};

// 顶点、32 位三角形索引和材质的 CPU 数据集合。
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    Material material;
};

// Right-handed coordinates, outward CCW triangles; Y is up except circle (XY).
// 右手坐标系网格生成器，三角形朝外且逆时针，通常以 Y 为上轴。
class MeshGenerator {
public:
    // 构造并校验材质；非法比例、颜色或非有限值会抛出异常。
    static Material material(glm::vec3 albedo, float metallic = 0.0f,
                             float roughness = 0.5f, float ao = 1.0f,
                             glm::vec3 emissive = glm::vec3(0.0f))
    {
        Material result{albedo, metallic, roughness, ao, emissive};
        validateMaterial(result);
        return result;
    }

    // 经纬网格球面，UV 接缝重复顶点，极点处跳过退化三角形。
    static MeshData sphere(float radius = 1.0f, std::uint32_t segments = 64,
                           std::uint32_t rings = 32, const Material &mat = {})
    {
        positive(radius); require(segments >= 3 && rings >= 2, "sphere needs >=3 segments and >=2 rings");
        MeshData mesh = gridMesh(segments, rings, mat);
        for (std::uint32_t y = 0; y <= rings; ++y) {
            const double v = static_cast<double>(y) / rings;
            const double phi = pi * v;
            for (std::uint32_t x = 0; x <= segments; ++x) {
                const double u = static_cast<double>(x) / segments;
                const double theta = 2.0 * pi * (x == segments ? 0.0 : u);
                glm::vec3 normal(static_cast<float>(std::sin(phi) * std::cos(theta)),
                                 static_cast<float>(std::cos(phi)),
                                 static_cast<float>(std::sin(phi) * std::sin(theta)));
                if (y == 0 || y == rings) normal = glm::vec3(0, y == 0 ? 1 : -1, 0);
                mesh.vertices.push_back({normal * radius, normal, {static_cast<float>(u), static_cast<float>(v)}});
            }
        }
        for (std::uint32_t y = 0; y < rings; ++y) {
            for (std::uint32_t x = 0; x < segments; ++x) {
                const auto a = y * (segments + 1) + x;
                const auto b = a + segments + 1;
                if (y > 0) triangle(mesh, a, a + 1, b);
                if (y + 1 < rings) triangle(mesh, a + 1, b + 1, b);
            }
        }
        return mesh;
    }

    // Filled disk in XY, facing +Z. Not a line-loop primitive.
    // XY 平面上朝向 +Z 的实心圆盘，以中心点和圆周组成三角扇。
    static MeshData circle(float radius = 1.0f, std::uint32_t segments = 64,
                           const Material &mat = {})
    {
        positive(radius); require(segments >= 3, "circle needs >=3 segments");
        MeshData mesh = makeMesh(mat);
        reserve(mesh, static_cast<std::uint64_t>(segments) + 2, static_cast<std::uint64_t>(segments) * 3);
        mesh.vertices.push_back({{0, 0, 0}, {0, 0, 1}, {0.5f, 0.5f}});
        for (std::uint32_t i = 0; i <= segments; ++i) {
            const double angle = i == segments ? 0.0 : 2.0 * pi * i / segments;
            const float x = static_cast<float>(std::cos(angle)), y = static_cast<float>(std::sin(angle));
            mesh.vertices.push_back({{radius * x, radius * y, 0}, {0, 0, 1}, {x * 0.5f + 0.5f, y * 0.5f + 0.5f}});
        }
        for (std::uint32_t i = 0; i < segments; ++i) triangle(mesh, 0, i + 1, i + 2);
        return mesh;
    }

    // 等边立方体复用长方体生成路径，size 为完整边长。
    static MeshData cube(float size = 1.0f, const Material &mat = {})
    {
        return box(glm::vec3(size), mat);
    }

    // Separate vertices on each face preserve hard normals and independent UVs.
    // 六个面独立创建顶点，保留硬法线和各面独立 UV。
    static MeshData box(glm::vec3 size, const Material &mat = {})
    {
        positive(size.x); positive(size.y); positive(size.z);
        MeshData mesh = makeMesh(mat);
        reserve(mesh, 24, 36);
        const glm::vec3 normals[] = {{1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};
        const glm::vec3 rights[] = {{0,0,-1}, {0,0,1}, {1,0,0}, {1,0,0}, {1,0,0}, {-1,0,0}};
        const glm::vec3 ups[] = {{0,1,0}, {0,1,0}, {0,0,-1}, {0,0,1}, {0,1,0}, {0,1,0}};
        const glm::vec2 uv[] = {{0,0}, {1,0}, {1,1}, {0,1}};
        for (std::uint32_t face = 0; face < 6; ++face) {
            const auto base = static_cast<std::uint32_t>(mesh.vertices.size());
            for (const auto &texcoord : uv) {
                const glm::vec3 p = normals[face] + (texcoord.x * 2 - 1) * rights[face] + (texcoord.y * 2 - 1) * ups[face];
                mesh.vertices.push_back({p * size * 0.5f, normals[face], texcoord});
            }
            triangle(mesh, base, base + 1, base + 2);
            triangle(mesh, base, base + 2, base + 3);
        }
        return mesh;
    }

    // 沿大圆和管截面采样圆环，两方向均重复 UV 接缝顶点。
    static MeshData torus(float majorRadius = 1.0f, float minorRadius = 0.25f,
                          std::uint32_t majorSegments = 64, std::uint32_t minorSegments = 24,
                          const Material &mat = {})
    {
        positive(majorRadius); positive(minorRadius);
        require(majorRadius > minorRadius && std::isfinite(majorRadius + minorRadius), "torus needs finite majorRadius > minorRadius");
        require(majorSegments >= 3 && minorSegments >= 3, "torus needs >=3 segments per axis");
        MeshData mesh = gridMesh(minorSegments, majorSegments, mat);
        for (std::uint32_t y = 0; y <= majorSegments; ++y) {
            const double u = static_cast<double>(y) / majorSegments;
            const double a = y == majorSegments ? 0.0 : 2.0 * pi * u;
            for (std::uint32_t x = 0; x <= minorSegments; ++x) {
                const double v = static_cast<double>(x) / minorSegments;
                const double b = x == minorSegments ? 0.0 : 2.0 * pi * v;
                const float ca = static_cast<float>(std::cos(a)), sa = static_cast<float>(std::sin(a));
                const float cb = static_cast<float>(std::cos(b)), sb = static_cast<float>(std::sin(b));
                const float r = majorRadius + minorRadius * cb;
                mesh.vertices.push_back({{r * ca, minorRadius * sb, r * sa}, {cb * ca, sb, cb * sa}, {static_cast<float>(u), static_cast<float>(v)}});
            }
        }
        for (std::uint32_t y = 0; y < majorSegments; ++y)
            for (std::uint32_t x = 0; x < minorSegments; ++x) {
                const auto a = y * (minorSegments + 1) + x, b = a + minorSegments + 1;
                triangle(mesh, a, a + 1, b); triangle(mesh, a + 1, b + 1, b);
            }
        return mesh;
    }

    // 上下半径相等的圆台；capped 控制是否生成端盖。
    static MeshData cylinder(float radius = 1.0f, float height = 2.0f,
                             std::uint32_t segments = 64, bool capped = true,
                             const Material &mat = {})
    {
        return frustum(radius, radius, height, segments, capped, mat);
    }

    // 顶部半径为 0 的圆台，侧面跳过顶端退化三角形。
    static MeshData cone(float radius = 1.0f, float height = 2.0f,
                         std::uint32_t segments = 64, bool capped = true,
                         const Material &mat = {})
    {
        return frustum(radius, 0.0f, height, segments, capped, mat);
    }

    // Height callback is evaluated once per vertex, in world-space X/Z units.
    // 对每个网格顶点调用一次高度函数，以世界空间 X/Z 取样；拒绝非有限高度。
    template<class HeightFunction>
    static MeshData terrain(float width, float depth, std::uint32_t xSegments,
                            std::uint32_t zSegments, HeightFunction height,
                            const Material &mat = {})
    {
        positive(width); positive(depth);
        require(xSegments >= 1 && zSegments >= 1, "terrain needs >=1 segment per axis");
        MeshData mesh = gridMesh(xSegments, zSegments, mat);
        for (std::uint32_t z = 0; z <= zSegments; ++z)
            for (std::uint32_t x = 0; x <= xSegments; ++x) {
                const float u = static_cast<float>(static_cast<double>(x) / xSegments);
                const float v = static_cast<float>(static_cast<double>(z) / zSegments);
                const float px = (u - 0.5f) * width, pz = (v - 0.5f) * depth;
                const double py = static_cast<double>(height(px, pz));
                require(std::isfinite(py) && std::abs(py) <= std::numeric_limits<float>::max(), "height must be a finite float");
                mesh.vertices.push_back({{px, static_cast<float>(py), pz}, {0,0,0}, {u,v}});
            }
        // 按单元连接三角形，并用未归一化叉积累计面积加权法线。
        std::vector<glm::dvec3> normals(mesh.vertices.size(), glm::dvec3(0.0));
        for (std::uint32_t z = 0; z < zSegments; ++z)
            for (std::uint32_t x = 0; x < xSegments; ++x) {
                const auto a = z * (xSegments + 1) + x, b = a + xSegments + 1;
                triangle(mesh, a, b, a + 1); triangle(mesh, a + 1, b, b + 1);
            }
        for (std::size_t i = 0; i < mesh.indices.size(); i += 3) {
            const auto a = mesh.indices[i], b = mesh.indices[i + 1], c = mesh.indices[i + 2];
            const glm::dvec3 n = glm::cross(glm::dvec3(mesh.vertices[b].position) - glm::dvec3(mesh.vertices[a].position),
                                          glm::dvec3(mesh.vertices[c].position) - glm::dvec3(mesh.vertices[a].position));
            require(glm::dot(n, n) > 0, "terrain dimensions lose precision at this resolution");
            normals[a] += n; normals[b] += n; normals[c] += n;
        }
        for (std::size_t i = 0; i < normals.size(); ++i) mesh.vertices[i].normal = glm::vec3(glm::normalize(normals[i]));
        return mesh;
    }

    // 通过零高度回调复用地形流程，保持相同的绕序和法线处理。
    static MeshData plane(float width = 2.0f, float depth = 2.0f,
                          std::uint32_t xSegments = 1, std::uint32_t zSegments = 1,
                          const Material &mat = {})
    {
        return terrain(width, depth, xSegments, zSegments, [](float, float) { return 0.0f; }, mat);
    }

private:
    static constexpr double pi = 3.14159265358979323846;

    // 统一条件检查，失败抛出 invalid_argument，不依赖 Qt 或 GL。
    static void require(bool condition, const char *message)
    {
        if (!condition) throw std::invalid_argument(message);
    }
    // 尺寸必须为有限正数，拒绝零、负数、NaN 和无穷大。
    static void positive(float value) { require(std::isfinite(value) && value > 0, "dimensions must be positive and finite"); }
    // 验证反照率、自发光和比例参数，避免无效材质进入网格。
    static void validateMaterial(const Material &mat)
    {
        for (int i = 0; i < 3; ++i) {
            require(std::isfinite(mat.albedo[i]) && mat.albedo[i] >= 0 && mat.albedo[i] <= 1, "albedo must be linear RGB in [0,1]");
            require(std::isfinite(mat.emissive[i]) && mat.emissive[i] >= 0, "emissive must be finite and nonnegative");
        }
        for (float value : {mat.metallic, mat.roughness, mat.ao})
            require(std::isfinite(value) && value >= 0 && value <= 1, "material scalars must be in [0,1]");
    }
    // 初始化空网格并绑定已校验的材质。
    static MeshData makeMesh(const Material &mat) { validateMaterial(mat); MeshData mesh; mesh.material = mat; return mesh; }
    // 用 64 位数量预检 32 位索引上限，再预分配容器容量。
    static void reserve(MeshData &mesh, std::uint64_t vertices, std::uint64_t indices)
    {
        if (vertices > std::numeric_limits<std::uint32_t>::max() || indices > std::numeric_limits<std::uint32_t>::max())
            throw std::length_error("mesh exceeds 32-bit vertex/index count");
        mesh.vertices.reserve(static_cast<std::size_t>(vertices));
        mesh.indices.reserve(static_cast<std::size_t>(indices));
    }
    // 为包含接缝行列的规则网格预留容量，不直接生成顶点。
    static MeshData gridMesh(std::uint32_t x, std::uint32_t y, const Material &mat)
    {
        MeshData mesh = makeMesh(mat);
        const std::uint64_t columns = static_cast<std::uint64_t>(x) + 1, rows = static_cast<std::uint64_t>(y) + 1;
        if (columns > std::numeric_limits<std::uint32_t>::max() / rows)
            throw std::length_error("mesh exceeds 32-bit vertex count");
        reserve(mesh, columns * rows, static_cast<std::uint64_t>(x) * y * 6);
        return mesh;
    }
    // 按输入顺序追加三角形索引，绕序由调用方决定。
    static void triangle(MeshData &mesh, std::uint32_t a, std::uint32_t b, std::uint32_t c)
    {
        mesh.indices.insert(mesh.indices.end(), {a,b,c});
    }
    // 端盖独立顶点形成硬边，上下端盖使用相反绕序。
    static void cap(MeshData &mesh, float radius, float y, std::uint32_t segments, bool top)
    {
        const auto base = static_cast<std::uint32_t>(mesh.vertices.size());
        const glm::vec3 normal(0, top ? 1 : -1, 0);
        mesh.vertices.push_back({{0,y,0}, normal, {0.5f,0.5f}});
        for (std::uint32_t i = 0; i <= segments; ++i) {
            const double angle = i == segments ? 0.0 : 2.0 * pi * i / segments;
            const float x = static_cast<float>(std::cos(angle)), z = static_cast<float>(std::sin(angle));
            mesh.vertices.push_back({{radius*x,y,radius*z}, normal, {x*0.5f+0.5f,z*0.5f+0.5f}});
        }
        for (std::uint32_t i = 0; i < segments; ++i) {
            if (top) triangle(mesh, base, base+i+2, base+i+1);
            else triangle(mesh, base, base+i+1, base+i+2);
        }
    }
    // 圆柱/圆锥共用侧面生成流程，根据半径变化求倾斜法线。
    static MeshData frustum(float bottom, float top, float height, std::uint32_t segments, bool capped, const Material &mat)
    {
        positive(bottom); positive(height); require(segments >= 3, "cylinder/cone needs >=3 segments");
        MeshData mesh = makeMesh(mat);
        const std::uint64_t n = segments;
        reserve(mesh, 2*(n+1) + (capped ? (top > 0 ? 2 : 1)*(n+2) : 0), n*(top > 0 ? 6 : 3) + (capped ? n*(top > 0 ? 6 : 3) : 0));
        // 半径差除以高度得到侧面斜率，参与侧面法线计算。
        const double slope = (static_cast<double>(bottom) - top) / height;
        for (std::uint32_t i = 0; i <= segments; ++i) {
            const double u = static_cast<double>(i) / segments;
            const double angle = i == segments ? 0.0 : 2.0*pi*u;
            const float x = static_cast<float>(std::cos(angle)), z = static_cast<float>(std::sin(angle));
            const glm::vec3 normal(glm::normalize(glm::dvec3(x, slope, z)));
            mesh.vertices.push_back({{bottom*x,-height*0.5f,bottom*z}, normal, {static_cast<float>(u),0}});
            mesh.vertices.push_back({{top*x,height*0.5f,top*z}, normal, {static_cast<float>(u),1}});
        }
        for (std::uint32_t i = 0; i < segments; ++i) {
            const auto a = i*2;
            triangle(mesh, a, a+1, a+2);
            if (top > 0) triangle(mesh, a+2, a+1, a+3);
        }
        if (capped) { cap(mesh, bottom, -height*0.5f, segments, false); if (top > 0) cap(mesh, top, height*0.5f, segments, true); }
        return mesh;
    }
};

} // namespace marmoset
