#pragma once

#include <cstddef>

struct TGAImage;
struct TGAColor;

// Call on the render thread with the application's OpenGL context current.
// Clear (or destroy) the image before destroying that context.
class ImageView {
public:
    ImageView();
    ~ImageView();
    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;

    // Tightly packed RGBA8, top row first. Copies pixels into a GPU texture.
    // Returns false for invalid input or a failed texture upload.
    bool SetImage(const unsigned char* rgba, std::size_t byte_count, int width, int height);
    bool SetImage(const TGAImage& image);
    void Draw() const;
    void Clear();
private:
    unsigned int texture_ = 0;
    int width_ = 0;
    int height_ = 0;

};
