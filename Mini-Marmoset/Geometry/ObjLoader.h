#pragma once

#include "MeshGenerator.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <locale>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

namespace marmoset {

// Polygonal OBJ reader. MTL names are retained, but no external files are loaded.
// 多边形 OBJ 的 CPU 解析入口，保留 MTL 名称但不加载外部材质或纹理。
class ObjLoader {
public:
    // 可选翻转 V 坐标，并为缺失法线按平滑组生成法线。
    struct Options {
        bool flipV = false;
        bool generateMissingNormals = true;
    };
    // 连续索引区间的对象/组/材质元数据，区间单位为索引个数。
    struct Part {
        std::string objectName;
        std::string groupName;
        std::string materialName;
        std::uint32_t firstIndex = 0;
        std::uint32_t indexCount = 0;
    };
    // 完整结果包含网格、分段及引用的材质库名称。
    struct Result {
        MeshData mesh;
        std::vector<Part> parts;
        std::vector<std::string> materialLibraries;
    };

    // 文件入口：打开失败抛出异常，成功后交给公共流解析器。
    static Result load(const std::filesystem::path &path) { return load(path, Options{}); }
    static Result load(const std::filesystem::path &path, const Options &options)
    {
        std::ifstream stream(path);
        if (!stream) throw std::runtime_error("Cannot open OBJ: " + path.u8string());
        return parse(stream, options);
    }
    // 内存文本入口，适合程序构造 OBJ 内容或离线验证。
    static Result fromString(std::string_view text) { return fromString(text, Options{}); }
    static Result fromString(std::string_view text, const Options &options)
    {
        std::istringstream stream{std::string(text)};
        return parse(stream, options);
    }
    // 流入口，每次解析创建独立 Parser，不共享模型间状态。
    static Result parse(std::istream &stream) { return parse(stream, Options{}); }
    static Result parse(std::istream &stream, const Options &options) { return Parser(options).run(stream); }

private:
    // 缺失属性使用哨兵；顶点去重同时考虑位置、UV、法线和平滑组。
    static constexpr std::size_t missing = std::numeric_limits<std::size_t>::max();
    struct Reference { std::size_t position = missing, uv = missing, normal = missing; };
    using NormalKey = std::pair<std::size_t, std::int64_t>;
    using VertexKey = std::tuple<std::size_t, std::size_t, std::size_t, std::int64_t>;

    // 保存当前解析的属性数组、去重映射、法线累加和对象/材质状态。
    class Parser {
    public:
        // 复制本次选项，其他缓存使用空初始状态。
        explicit Parser(Options options) : options_(options) {}

        // 逐行合并续行、清理 BOM 和注释，再解析语句；最终补全法线并返回结果。
        Result run(std::istream &stream)
        {
            std::string physical, logical;
            std::size_t physicalLine = 0;
            bool continuing = false;
            while (std::getline(stream, physical)) {
                ++physicalLine;
                if (!continuing) line_ = physicalLine;
                if (physicalLine == 1 && physical.compare(0, 3, "\xef\xbb\xbf") == 0) physical.erase(0, 3);
                const auto comment = physical.find('#');
                if (comment != std::string::npos) physical.erase(comment);
                physical = trim(physical);
                continuing = !physical.empty() && physical.back() == '\\';
                if (continuing) physical.pop_back();
                logical += physical;
                if (continuing) { logical += ' '; continue; }
                readLine(logical);
                logical.clear();
            }
            if (stream.bad() || (stream.fail() && !stream.eof())) fail("input stream read failed");
            if (continuing) fail("unfinished line continuation");
            if (result_.mesh.indices.empty()) fail("no polygon faces found");
            // 所有面处理完成后统一归一化累计法线，允许同平滑组跨 UV 接缝平滑。
            if (options_.generateMissingNormals) {
                for (const auto &entry : generated_) {
                    const glm::dvec3 n = accumulated_.at(entry.second);
                    if (!(glm::dot(n, n) > 0)) fail("generated smoothing normal cancels to zero");
                    result_.mesh.vertices[entry.first].normal = glm::vec3(glm::normalize(n));
                }
            }
            return std::move(result_);
        }

    private:
        // 工作缓存只属于本次解析，结果返回后随 Parser 一起释放。
        Options options_;
        Result result_;
        std::vector<glm::vec3> positions_, normals_;
        std::vector<glm::vec2> uvs_;
        std::map<VertexKey, std::uint32_t> vertices_;
        std::map<NormalKey, glm::dvec3> accumulated_;
        std::vector<std::pair<std::uint32_t, NormalKey>> generated_;
        std::string object_, group_, material_;
        std::size_t line_ = 0;
        std::int64_t smoothing_ = 0, face_ = 0;

        // 错误附带逻辑语句起始行号，方便定位 OBJ 格式或几何问题。
        [[noreturn]] void fail(const std::string &message) const
        {
            throw std::runtime_error("OBJ line " + std::to_string(line_) + ": " + message);
        }
        // 清除行首尾空白，兼容空行和 Windows 风格换行。
        static std::string trim(const std::string &value)
        {
            // 记录本面索引区间，相邻且对象、组、材质相同的面合并为一个 Part。
            const auto first = value.find_first_not_of(" \t\r\n");
            return first == std::string::npos ? std::string{} : value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
        }
        // 严格转换整个整数 token，允许正号，拒绝非法尾部字符。
        std::int64_t integer(std::string_view token) const
        {
            if (!token.empty() && token.front() == '+') token.remove_prefix(1);
            std::int64_t value = 0;
            const auto parsed = std::from_chars(token.data(), token.data() + token.size(), value);
            if (token.empty() || parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size()) fail("invalid integer index/group");
            return value;
        }
        // 将 OBJ 的正索引或相对末尾的负索引转为 0 起始索引，拒绝 0 和越界。
        std::size_t index(std::string_view token, std::size_t count) const
        {
            const auto value = integer(token);
            if (count > static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())) fail("too many source elements");
            const auto size = static_cast<std::int64_t>(count);
            if (value == 0 || value > size || value < -size) fail("index out of range: " + std::string(token));
            return static_cast<std::size_t>(value > 0 ? value - 1 : size + value);
        }
        // 支持 v、v/vt、v//vn、v/vt/vn 形式，分别校验各属性索引。
        Reference reference(const std::string &token) const
        {
            Reference ref;
            const auto first = token.find('/');
            if (first == std::string::npos) { ref.position = index(token, positions_.size()); return ref; }
            ref.position = index(std::string_view(token).substr(0, first), positions_.size());
            const auto second = token.find('/', first + 1);
            if (second == std::string::npos) {
                ref.uv = index(std::string_view(token).substr(first + 1), uvs_.size());
                return ref;
            }
            if (token.find('/', second + 1) != std::string::npos) fail("too many '/' separators in face reference");
            if (second > first + 1) ref.uv = index(std::string_view(token).substr(first + 1, second - first - 1), uvs_.size());
            ref.normal = index(std::string_view(token).substr(second + 1), normals_.size());
            return ref;
        }
        // 使用经典 locale 解析有限数值，不受系统小数点设置影响。
        std::vector<double> numbers(std::istringstream &line) const
        {
            std::vector<double> values;
            std::string token;
            while (line >> token) {
                std::istringstream scalar(token);
                scalar.imbue(std::locale::classic());
                double value = 0;
                if (!(scalar >> value) || !scalar.eof() || !std::isfinite(value)) fail("invalid/non-finite numeric value");
                values.push_back(value);
            }
            return values;
        }
        // 转换前检查 float 范围，避免溢出产生无穷大坐标。
        float asFloat(double value) const
        {
            if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<float>::max()) fail("value outside finite float range");
            return static_cast<float>(value);
        }
        // 分派几何、平滑组和材质元数据语句，不支持的 OBJ 扩展显式报错。
        void readLine(const std::string &text)
        {
            std::istringstream line(text);
            line.imbue(std::locale::classic());
            std::string command;
            if (!(line >> command)) return;
            if (command == "v") {
                const auto values = numbers(line);
                if (values.size() != 3 && values.size() != 4) fail("v needs x y z [w]; vertex-color extensions are unsupported");
                const double w = values.size() == 4 ? values[3] : 1.0;
                if (w == 0) fail("vertex w cannot be zero");
                positions_.emplace_back(asFloat(values[0]/w), asFloat(values[1]/w), asFloat(values[2]/w));
            } else if (command == "vt") {
                const auto values = numbers(line);
                if (values.empty() || values.size() > 3) fail("vt needs u [v [w]]");
                const double v = values.size() > 1 ? values[1] : 0.0;
                uvs_.emplace_back(asFloat(values[0]), asFloat(options_.flipV ? 1.0-v : v));
            } else if (command == "vn") {
                const auto values = numbers(line);
                if (values.size() != 3) fail("vn needs x y z");
                const glm::dvec3 n(values[0], values[1], values[2]);
                const double length = glm::length(n);
                if (!(length > 0) || !std::isfinite(length)) fail("invalid normal length");
                normals_.push_back(glm::vec3(n/length));
            } else if (command == "f") {
                std::vector<Reference> refs;
                std::string token;
                while (line >> token) refs.push_back(reference(token));
                addFace(refs);
            } else if (command == "s") {
                std::string token, extra;
                if (!(line >> token) || (line >> extra)) fail("s needs one smoothing group");
                smoothing_ = token == "off" || token == "0" ? 0 : token == "on" ? 1 : integer(token);
                if (smoothing_ < 0) fail("smoothing group cannot be negative");
            } else if (command == "o" || command == "g" || command == "usemtl") {
                std::string rest; std::getline(line, rest); rest = trim(rest);
                if (command == "o") object_ = rest;
                else if (command == "g") group_ = rest;
                else material_ = rest;
            } else if (command == "mtllib") {
                std::string name;
                bool found = false;
                while (line >> name) { result_.materialLibraries.push_back(name); found = true; }
                if (!found) fail("mtllib needs a library name");
            } else {
                fail("unsupported statement: " + command + " (only polygonal meshes are supported)");
            }
        }

        // 二维叉积用于判断投影面积、绕序和点在线段哪一侧。
        static double cross2(glm::dvec2 a, glm::dvec2 b) { return a.x*b.y - a.y*b.x; }
        // 结合面积和距离误差判断点是否位于线段上。
        static bool onSegment(glm::dvec2 a, glm::dvec2 b, glm::dvec2 p, double areaEps, double lengthEps)
        {
            return std::abs(cross2(b-a,p-a)) <= areaEps && p.x >= std::min(a.x,b.x)-lengthEps && p.x <= std::max(a.x,b.x)+lengthEps
                && p.y >= std::min(a.y,b.y)-lengthEps && p.y <= std::max(a.y,b.y)+lengthEps;
        }
        // 检测线段交叉或接触，拒绝自相交的多边形。
        static bool intersects(glm::dvec2 a, glm::dvec2 b, glm::dvec2 c, glm::dvec2 d, double eps, double lengthEps)
        {
            const double abC = cross2(b-a,c-a), abD = cross2(b-a,d-a), cdA = cross2(d-c,a-c), cdB = cross2(d-c,b-c);
            if (((abC > eps && abD < -eps) || (abC < -eps && abD > eps)) &&
                ((cdA > eps && cdB < -eps) || (cdA < -eps && cdB > eps))) return true;
            return onSegment(a,b,c,eps,lengthEps) || onSegment(a,b,d,eps,lengthEps) || onSegment(c,d,a,eps,lengthEps) || onSegment(c,d,b,eps,lengthEps);
        }

        // 验证面共面且非退化，投影到二维后用耳切法处理简单凹多边形。
        std::vector<std::array<std::size_t,3>> triangulate(const std::vector<Reference> &refs, glm::dvec3 &areaNormal) const
        {
            if (refs.size() < 3) fail("face needs at least 3 corners");
            // 以首点为局部原点并用双精度计算，减轻大坐标带来的消减误差。
            const glm::dvec3 origin(positions_[refs[0].position]);
            std::vector<glm::dvec3> points;
            glm::dvec3 lo(0), hi(0);
            for (const auto &ref : refs) {
                const glm::dvec3 p = glm::dvec3(positions_[ref.position]) - origin;
                points.push_back(p); lo = glm::min(lo,p); hi = glm::max(hi,p);
            }
            areaNormal = glm::dvec3(0);
            for (std::size_t i = 0; i < points.size(); ++i) areaNormal += glm::cross(points[i], points[(i+1)%points.size()]);
            const glm::dvec3 span = hi-lo;
            const double scale = std::max({span.x,span.y,span.z});
            // 几何容差随多边形尺度变化，分别约束面积误差和距离误差。
            const double eps = scale*scale*1e-12, lengthEps = scale*1e-12;
            const double normalLength = glm::length(areaNormal);
            if (!(normalLength > eps)) fail("degenerate polygon");
            const glm::dvec3 normal = areaNormal/normalLength;
            for (const auto &p : points) if (std::abs(glm::dot(p,normal)) > scale*1e-5) fail("non-planar polygon; triangulate before exporting");
            const glm::dvec3 an = glm::abs(normal);
            // 舍弃法线绝对值最大的轴，得到较稳定的二维投影。
            const int drop = an.x >= an.y && an.x >= an.z ? 0 : an.y >= an.z ? 1 : 2;
            std::vector<glm::dvec2> projected;
            for (const auto &p : points) projected.emplace_back(drop == 0 ? p.y : p.x, drop == 2 ? p.y : p.z);
            double area = 0;
            for (std::size_t i = 0; i < points.size(); ++i) area += cross2(projected[i],projected[(i+1)%points.size()]);
            const double sign = area > 0 ? 1 : -1;
            for (std::size_t i = 0; i < points.size(); ++i) {
                const std::size_t next = (i+1)%points.size();
                if (glm::length(projected[i]-projected[next]) <= lengthEps) fail("repeated polygon corner");
                for (std::size_t j = i+1; j < points.size(); ++j) {
                    const auto jnext = (j+1)%points.size();
                    if (j == next || jnext == i) continue;
                    if (intersects(projected[i],projected[next],projected[j],projected[jnext],eps,lengthEps)) fail("self-intersecting polygon");
                }
            }
            // 维护未裁出的角点环，逐次裁掉不包含其他角点的凸耳并保持原绕序。
            std::vector<std::size_t> remaining(points.size());
            std::iota(remaining.begin(), remaining.end(), std::size_t{0});
            std::vector<std::array<std::size_t,3>> triangles;
            while (remaining.size() > 3) {
                bool clipped = false;
                for (std::size_t i = 0; i < remaining.size(); ++i) {
                    const auto a = remaining[(i+remaining.size()-1)%remaining.size()], b = remaining[i], c = remaining[(i+1)%remaining.size()];
                    if (sign*cross2(projected[b]-projected[a],projected[c]-projected[b]) <= eps) continue;
                    bool occupied = false;
                    for (auto p : remaining) {
                        if (p == a || p == b || p == c) continue;
                        if (sign*cross2(projected[b]-projected[a],projected[p]-projected[a]) >= -eps &&
                            sign*cross2(projected[c]-projected[b],projected[p]-projected[b]) >= -eps &&
                            sign*cross2(projected[a]-projected[c],projected[p]-projected[c]) >= -eps) { occupied = true; break; }
                    }
                    if (occupied) continue;
                    triangles.push_back({a,b,c});
                    remaining.erase(remaining.begin()+static_cast<std::ptrdiff_t>(i));
                    clipped = true; break;
                }
                if (!clipped) fail("cannot triangulate polygon");
            }
            if (sign*cross2(projected[remaining[1]]-projected[remaining[0]],projected[remaining[2]]-projected[remaining[1]]) <= eps) fail("degenerate final triangle");
            triangles.push_back({remaining[0],remaining[1],remaining[2]});
            return triangles;
        }

        // 三角化、顶点去重、缺失法线累加与连续索引分段的统一入口。
        void addFace(const std::vector<Reference> &refs)
        {
            glm::dvec3 areaNormal;
            const auto triangles = triangulate(refs, areaNormal);
            if (face_ == std::numeric_limits<std::int64_t>::max()) fail("too many faces");
            ++face_;
            // 关闭平滑时使用每面独立负标识，避免相邻面共享生成法线。
            const auto smooth = smoothing_ > 0 ? smoothing_ : -face_;
            if (triangles.size() > (std::numeric_limits<std::uint32_t>::max()-result_.mesh.indices.size())/3) fail("too many triangle indices");
            std::vector<std::uint32_t> indices;
            for (const auto &ref : refs) {
                const NormalKey normalKey{ref.position, smooth};
                if (options_.generateMissingNormals) {
                    auto inserted = accumulated_.emplace(normalKey, glm::dvec3(0));
                    inserted.first->second += areaNormal;
                }
                // 去重键保留 UV/法线接缝；显式法线不再依赖平滑组标识。
                const VertexKey key{ref.position, ref.uv, ref.normal, ref.normal == missing ? smooth : 0};
                const auto found = vertices_.find(key);
                if (found != vertices_.end()) { indices.push_back(found->second); continue; }
                if (result_.mesh.vertices.size() >= std::numeric_limits<std::uint32_t>::max()) fail("too many output vertices");
                const auto vertexIndex = static_cast<std::uint32_t>(result_.mesh.vertices.size());
                result_.mesh.vertices.push_back({positions_[ref.position], ref.normal == missing ? glm::vec3(0) : normals_[ref.normal], ref.uv == missing ? glm::vec2(0) : uvs_[ref.uv]});
                vertices_.emplace(key, vertexIndex);
                if (ref.normal == missing && options_.generateMissingNormals) generated_.push_back({vertexIndex,normalKey});
                indices.push_back(vertexIndex);
            }
            const auto first = static_cast<std::uint32_t>(result_.mesh.indices.size());
            for (const auto &t : triangles) for (auto corner : t) result_.mesh.indices.push_back(indices[corner]);
            const auto count = static_cast<std::uint32_t>(result_.mesh.indices.size())-first;
            if (!result_.parts.empty() && result_.parts.back().objectName == object_ && result_.parts.back().groupName == group_ && result_.parts.back().materialName == material_) {
                result_.parts.back().indexCount += count;
            } else {
                result_.parts.push_back({object_, group_, material_, first, count});
            }
        }
    };
};

} // namespace marmoset
