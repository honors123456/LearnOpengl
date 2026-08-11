#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture; // 离屏渲染存好的场景贴图

vec3 pseudoColor(float value)
{
    value = clamp(value, 0.0, 1.0);

    if (value < 0.25) {
        return mix(vec3(0.0, 0.0, 0.5), vec3(0.0, 1.0, 1.0), value / 0.25);
    } else if (value < 0.5) {
        return mix(vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), (value - 0.25) / 0.25);
    } else if (value < 0.75) {
        return mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), (value - 0.5) / 0.25);
    }

    return mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (value - 0.75) / 0.25);
}

void main() {
    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    // 先把原始颜色转成亮度，再把亮度映射成伪彩颜色。
    float gray = dot(col, vec3(0.2126, 0.7152, 0.0722));
    FragColor = vec4(pseudoColor(gray), 1.0);
}
