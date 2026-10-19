#include "ImageView.h"
#include "tgaimage.h"
#include "imgui.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <algorithm>
#include <limits>
#include <vector>

ImageView::ImageView() = default;

ImageView::~ImageView()
{
    Clear();
}

bool ImageView::SetImage(const TGAImage& image)
{
    const int width = image.width();
    const int height = image.height();
    if (width <= 0 || height <= 0)
        return false;
    GLint max_size = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
    if (width > max_size || height > max_size)
        return false;
    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    if (w > std::numeric_limits<std::size_t>::max() / 4 / h)
        return false;
    std::vector<unsigned char> rgba(w * h * 4);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const TGAColor color = image.get(x, y);
            const std::size_t offset = (static_cast<std::size_t>(y) * w + x) * 4;
            rgba[offset] = color.bytespp == TGAImage::GRAYSCALE ? color[0] : color[2];
            rgba[offset + 1] = color.bytespp == TGAImage::GRAYSCALE ? color[0] : color[1];
            rgba[offset + 2] = color[0];
            rgba[offset + 3] = color.bytespp == TGAImage::RGBA ? color[3] : 255;
        }
    }
    return SetImage(rgba.data(), rgba.size(), width, height);
}

bool ImageView::SetImage(const unsigned char* rgba, std::size_t byte_count, int width, int height)
{
    if (!rgba || width <= 0 || height <= 0)
        return false;
    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    if (w > std::numeric_limits<std::size_t>::max() / 4 / h || byte_count < w * h * 4)
        return false;
    GLint max_size = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
    if (width > max_size || height > max_size)
        return false;

    GLint previous_texture = 0;
    GLint alignment = 0;
    GLint row_length = 0;
    GLint skip_rows = 0;
    GLint skip_pixels = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &row_length);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skip_rows);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skip_pixels);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    constexpr GLint clamp_to_edge = 0x812F;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp_to_edge);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp_to_edge);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    if (texture != 0)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    GLint uploaded_width = 0;
    GLint uploaded_height = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &uploaded_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &uploaded_height);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skip_rows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skip_pixels);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous_texture));
    if (texture == 0 || uploaded_width != width || uploaded_height != height) {
        if (texture != 0)
            glDeleteTextures(1, &texture);
        return false;
    }
    Clear();
    texture_ = texture;
    width_ = width;
    height_ = height;
    return true;
}

void ImageView::Draw() const
{
    if (texture_ == 0)
        return;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (viewport->Size.x <= 0 || viewport->Size.y <= 0)
        return;
    const float scale = std::min(viewport->Size.x / static_cast<float>(width_),
                                 viewport->Size.y / static_cast<float>(height_));
    const ImVec2 size(width_ * scale, height_ * scale);
    const ImVec2 top_left(viewport->Pos.x + (viewport->Size.x - size.x) * 0.5f,
                          viewport->Pos.y + (viewport->Size.y - size.y) * 0.5f);
    ImGui::GetBackgroundDrawList()->AddImage(static_cast<ImTextureID>(texture_), top_left,
                                           ImVec2(top_left.x + size.x, top_left.y + size.y));
}

void ImageView::Clear()
{
    if (texture_ != 0)
        glDeleteTextures(1, &texture_);
    texture_ = 0;
    width_ = 0;
    height_ = 0;
}
