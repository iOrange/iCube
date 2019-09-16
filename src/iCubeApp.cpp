#include "iCubeApp.h"

#include "imgui.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "nfd.h"

iCubeApp::iCubeApp()
    : mWindow(nullptr)
    , mWidth(1280)
    , mHeight(720)
{

}
iCubeApp::~iCubeApp() {

}

bool iCubeApp::Initialize() {
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    mWindow = glfwCreateWindow(mWidth, mHeight, "iCube", nullptr, nullptr);

    if (mWindow == nullptr) {
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(scast<GLFWwindow*>(mWindow), this);

    glfwMakeContextCurrent(scast<GLFWwindow*>(mWindow));
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        return false;
    }

    glfwSwapInterval(1); // enable v-sync

    glViewport(0, 0, mWidth, mHeight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    //glBindSampler(0, 0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // disable "imgui.ini"
    ImGui::GetStyle().WindowRounding = 0.0f; // disable windows rounding

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(scast<GLFWwindow*>(mWindow), true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

    glfwSetWindowSizeCallback(scast<GLFWwindow*>(mWindow), [](GLFWwindow* wnd, int width, int height) {
        iCubeApp* _this = scast<iCubeApp*>(glfwGetWindowUserPointer(wnd));
        if (_this) {
            _this->Resize(width, height);
        }
    });

    return true;
}

void iCubeApp::Loop() {
    double lastTimerValue = glfwGetTime();

    while(GL_FALSE == glfwWindowShouldClose(scast<GLFWwindow*>(mWindow))) {
        glfwPollEvents();

        const double currentTimerValue = glfwGetTime();
        const double dt = currentTimerValue - lastTimerValue;
        lastTimerValue = currentTimerValue;

        this->Update(scast<float>(dt));

        glClearColor(0.412f, 0.796f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        this->DoUI();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(scast<GLFWwindow*>(mWindow));
    }
}

void iCubeApp::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(scast<GLFWwindow*>(mWindow));
    glfwTerminate();

    mWindow = nullptr;
}


void iCubeApp::Resize(const int width, const int height) {
    mWidth = width;
    mHeight = height;

    glViewport(0, 0, mWidth, mHeight);
}

void iCubeApp::Update(const float dt) {
    // nothing here yet
}


void iCubeApp::DoUI() {
    const ImGuiWindowFlags kPanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    const float kHalfScreenW = scast<float>(mWidth) * 0.5f;
    const float kHalfScreenH = scast<float>(mHeight) * 0.5f;

    // Viewer panel [LT]
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(kHalfScreenW, kHalfScreenH));
    ImGui::Begin("Viewer:", nullptr, kPanelFlags); {
        ImGui::Text("%.1f FPS (%.3f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    } ImGui::End();

    // LatLong panel [RT]
    ImGui::SetNextWindowPos(ImVec2(kHalfScreenW, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(kHalfScreenW, kHalfScreenH));
    ImGui::Begin("LatLong:", nullptr, kPanelFlags); {
        const ImTextureID texture = rcast<ImTextureID>(scast<size_t>(mEnvImg.GetTextureLatLong()));
        if (texture) {
            ImVec2 wndMin = ImGui::GetWindowContentRegionMin();
            ImVec2 wndMax = ImGui::GetWindowContentRegionMax();

            ImVec2 imgSize = ImVec2(wndMax.x - wndMin.x, wndMax.y - wndMin.y);

            ImGui::PushID("LatLongImg"); {
                ImGui::Image(texture, imgSize);
            } ImGui::PopID();

            ImGui::SetCursorPos(wndMin);
        }

        if (ImGui::Button("Import##LatLong")) {
            this->ImportLatLong();
        }
        if (ImGui::Button("Export##LatLong")) {
            this->ExportLatLong();
        }
    } ImGui::End();

    // Cube Cross panel [LB]
    ImGui::SetNextWindowPos(ImVec2(0.0f, kHalfScreenH));
    ImGui::SetNextWindowSize(ImVec2(kHalfScreenW, kHalfScreenH));
    ImGui::Begin("Cube Cross:", nullptr, kPanelFlags); {
        const ImTextureID texture = rcast<ImTextureID>(scast<size_t>(mEnvImg.GetTextureCubeCross()));
        if (texture) {
            ImVec2 wndMin = ImGui::GetWindowContentRegionMin();
            ImVec2 wndMax = ImGui::GetWindowContentRegionMax();

            ImVec2 imgSize = ImVec2(wndMax.x - wndMin.x, wndMax.y - wndMin.y);

            ImGui::PushID("CubeCrossImg"); {
                ImGui::Image(texture, imgSize);
            } ImGui::PopID();

            ImGui::SetCursorPos(wndMin);
        }

        if (ImGui::Button("Import##CubeCross")) {
            this->ImportCubeCross();
        }
        if (ImGui::Button("Export##CubeCross")) {
            this->ExportCubeCross();
        }
    } ImGui::End();

    // Cube Faces panel [RB]
    ImGui::SetNextWindowPos(ImVec2(kHalfScreenW, kHalfScreenH));
    ImGui::SetNextWindowSize(ImVec2(kHalfScreenW, kHalfScreenH));
    ImGui::Begin("Cube Faces:", nullptr, kPanelFlags); {
        if (ImGui::Button("Import##CubeFaces")) {
            this->ImportCubeFaces();
        }
        if (ImGui::Button("Export##CubeFaces")) {
            this->ExportCubeFaces();
        }
    } ImGui::End();
}

nfdchar_t* kSupportedFilesExtensions = "bmp,jpg,tga,png,hdr";

// LatLong
void iCubeApp::ImportLatLong(const fs::path& path) {
    fs::path srcPath = path;
    if (srcPath.empty()) {
        nfdchar_t* outPath = nullptr;
        if (NFD_OKAY == NFD_OpenDialog(kSupportedFilesExtensions, nullptr, &outPath)) {
            srcPath = outPath;
        }
    }

    if (!srcPath.empty()) {
        mEnvImg.LoadLatLong(srcPath);
    }
}

void iCubeApp::ExportLatLong(const fs::path& path) {
    fs::path dstPath = path;
    if (dstPath.empty()) {
        nfdchar_t* outPath = nullptr;
        if (NFD_OKAY == NFD_SaveDialog(kSupportedFilesExtensions, nullptr, &outPath)) {
            dstPath = outPath;
        }
    }

    if (!dstPath.empty()) {

    }
}

// CubeCross
void iCubeApp::ImportCubeCross(const fs::path& path) {
    fs::path srcPath = path;
    if (srcPath.empty()) {
        nfdchar_t* outPath = nullptr;
        if (NFD_OKAY == NFD_OpenDialog(kSupportedFilesExtensions, nullptr, &outPath)) {
            srcPath = outPath;
        }
    }

    if (!srcPath.empty()) {
        mEnvImg.LoadCubeCross(srcPath);
    }
}

void iCubeApp::ExportCubeCross(const fs::path& path) {
    fs::path dstPath = path;
    if (dstPath.empty()) {
        nfdchar_t* outPath = nullptr;
        if (NFD_OKAY == NFD_SaveDialog(kSupportedFilesExtensions, nullptr, &outPath)) {
            dstPath = outPath;
        }
    }

    if (!dstPath.empty()) {

    }
}

// CubeFaces
void iCubeApp::ImportCubeFaces(const Array<fs::path> paths) {
    Array<fs::path> srcPaths = paths;
    if (srcPaths.empty()) {
        nfdpathset_t outPaths = {};
        if (NFD_OKAY == NFD_OpenDialogMultiple(kSupportedFilesExtensions, nullptr, &outPaths)) {
            srcPaths.reserve(outPaths.count);
            for (size_t i = 0; i < outPaths.count; ++i) {
                srcPaths.push_back(NFD_PathSet_GetPath(&outPaths, i));
            }

            NFD_PathSet_Free(&outPaths);
        }
    }

    if (!srcPaths.empty()) {
        mEnvImg.LoadCubeFaces(srcPaths);
    }
}

void iCubeApp::ExportCubeFaces(const fs::path& path) {
    fs::path dstPath = path;
    if (dstPath.empty()) {
        nfdchar_t* outPath = nullptr;
        if (NFD_OKAY == NFD_SaveDialog(kSupportedFilesExtensions, nullptr, &outPath)) {
            dstPath = outPath;
        }
    }

    if (!dstPath.empty()) {

    }
}
