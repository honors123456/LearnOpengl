#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImageView.h"
#include "SoftwareRenderer.h"
#include "Shader.h"
#include <limits>
#include <vector>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include "tgaimage.h"

#include <cstdio>

int main()
{
    glfwSetErrorCallback([](int code, const char* description) {
        std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
    });
    if (!glfwInit())
        return 1;

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    const char* glsl_version = "#version 150";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    const char* glsl_version = "#version 130";
#endif
    GLFWwindow* window = glfwCreateWindow(800, 800, "TinyRender - Dear ImGui", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    ImageView image;
    SoftwareRenderer renderer;
    TGAImage tga(1024, 1024, TGAImage::RGB);
    std::vector<float> zbuffer(1024 * 1024, std::numeric_limits<float>::infinity());

    //head数据
    ParseAssimp head;
    TGAImage headDiffuse, headNormal;
    bool headLoaded = head.loadData("../obj/african_head/african_head.obj") && 
        headDiffuse.read_tga_file("../obj/african_head/african_head_diffuse.tga") &&
        headNormal.read_tga_file("../obj/african_head/african_head_nm.tga");

    if(!headLoaded)
    {
        std::fprintf(stderr, "Failed to load head model or textures.\n");
        return -1;
    }

    //eye数据
    ParseAssimp eyes;
    TGAImage eyesDiffuse, eyesNormal;
    bool eyeLoaded = eyes.loadData("../obj/african_head/african_head_eye_inner.obj") &&
        eyesDiffuse.read_tga_file("../obj/african_head/african_head_eye_inner_diffuse.tga") &&
        eyesNormal.read_tga_file("../obj/african_head/african_head_eye_inner_nm.tga");

    if(!eyeLoaded)
    {
        std::fprintf(stderr, "Failed to load eye model or textures.\n");
        return -1;
    }

    //Pass 1:得到深度信息
    //离屏缓存buffer
    Eigen::Vector3f light_dir(1.0f,1.0f,1.0f);
    Eigen::Vector3f cameraPos(0.0f,0.0f,3.0f);
    Eigen::Vector3f center(0.0f,0.0f,0.0f);
    Eigen::Vector3f up(0.0f,1.0f,0.0f);
    std::vector<float> shadowBuffer(1024 * 1024, std::numeric_limits<float>::max());
    TGAImage shadowFrame(1024, 1024, TGAImage::RGB);

    //MVP
    Eigen::Matrix4f lightModel = Eigen::Matrix4f::Identity();
    Eigen::Vector3f lightPos = center + light_dir.normalized() * 10.0f;
    Eigen::Matrix4f lightView = renderer.lookAt(lightPos,center,up);
    Eigen::Matrix4f lightProjection = renderer.orthographic(-1.5f, 1.5f, -1.5f, 1.5f, 0.1f, 10.0f);
    Eigen::Matrix4f lightTransform = lightProjection * lightView * lightModel;

    Eigen::Matrix4f cameraTransform = renderer.transform();

    DepthShader headDepthShader(head,lightTransform);
    renderer.drawModel(head,headDepthShader,shadowFrame,shadowBuffer);

    DepthShader eyeDepthShader(eyes,lightTransform);
    renderer.drawModel(eyes,eyeDepthShader,shadowFrame,shadowBuffer);

    //Pass 2:利用深度信息正常渲染物体
    ShadowShader headShadowShader(head,light_dir,cameraTransform,lightTransform,shadowBuffer,1024,1024,headDiffuse,headNormal);
    renderer.drawModel(head,headShadowShader,tga,zbuffer);

    ShadowShader eyeShadowShader(eyes,light_dir,cameraTransform,lightTransform,shadowBuffer,1024,1024,eyesDiffuse,eyesNormal);
    renderer.drawModel(eyes,eyeShadowShader,tga,zbuffer);
    
    image.SetImage(tga);
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            glfwWaitEventsTimeout(0.1);
            continue;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        image.Draw();

        ImGui::Render();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.10f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    image.Clear();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
