#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture; // 离屏渲染存好的场景贴图

void main() {
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // 效果 1：反色滤镜（Inversion）
    //FragColor = vec4(1.0 - col, 1.0);
    
    // 效果 2：黑白电影滤镜（Grayscale）
    float average = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
    FragColor = vec4(vec3(average), 1.0);
}