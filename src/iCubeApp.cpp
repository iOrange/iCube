#include "iCubeApp.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "nfd.h"

#include <iostream>

void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam) {
    if (GL_DEBUG_TYPE_ERROR == type) {
        fprintf(stdout, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
        assert(false);
    }
}

iCubeApp::iCubeApp()
    : mWindow(nullptr)
    , mWidth(1280)
    , mHeight(720)
    , mViewerPanelBounds(0.0f)
    , mCubeFacesPanelBounds(0.0f)
    , mLastMPos(0.0f)
    , mDrawFacesShader(GL_NONE)
    , mJunkVAO(GL_NONE)
    , mCubeFacesMouseDown(false)
    , mCubeFacesRotation(0.0f)
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

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    glViewport(0, 0, mWidth, mHeight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindSampler(0, 0);

    this->PrepareRenderer();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // disable "imgui.ini"
    ImGui::GetStyle().WindowRounding = 0.0f; // disable windows rounding

    glfwSetWindowSizeCallback(scast<GLFWwindow*>(mWindow), [](GLFWwindow* wnd, int width, int height) {
        iCubeApp* _this = scast<iCubeApp*>(glfwGetWindowUserPointer(wnd));
        if (_this) {
            _this->OnResize(width, height);
        }
    });
    glfwSetCursorPosCallback(scast<GLFWwindow*>(mWindow), [](GLFWwindow* wnd, double x, double y) {
        iCubeApp* _this = scast<iCubeApp*>(glfwGetWindowUserPointer(wnd));
        if (_this) {
            _this->OnSetCursorPos(scast<float>(x), scast<float>(y));
        }
    });
    glfwSetMouseButtonCallback(scast<GLFWwindow*>(mWindow), [](GLFWwindow* wnd, int button, int action, int mods) {
        iCubeApp* _this = scast<iCubeApp*>(glfwGetWindowUserPointer(wnd));
        if (_this) {
            _this->OnMouseButton(button, action, mods);
        }
    });

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(scast<GLFWwindow*>(mWindow), true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

    return true;
}

void iCubeApp::Loop() {
    double lastTimerValue = glfwGetTime();

    while(GL_FALSE == glfwWindowShouldClose(scast<GLFWwindow*>(mWindow))) {
        glfwPollEvents();

        const double currentTimerValue = glfwGetTime();
        const double dt = currentTimerValue - lastTimerValue;
        lastTimerValue = currentTimerValue;

        this->OnUpdate(scast<float>(dt));

        glClearColor(0.412f, 0.796f, 1.0f, 1.0f);
        glClearDepthf(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    if (mDrawFacesShader != GL_NONE) {
        glDeleteProgram(mDrawFacesShader);
        mDrawFacesShader = GL_NONE;
    }
    if (mJunkVAO != GL_NONE) {
        glDeleteVertexArrays(1, &mJunkVAO);
        mJunkVAO = GL_NONE;
    }

    glfwDestroyWindow(scast<GLFWwindow*>(mWindow));
    glfwTerminate();

    mWindow = nullptr;
}


void iCubeApp::OnResize(const int width, const int height) {
    mWidth = width;
    mHeight = height;

    glViewport(0, 0, mWidth, mHeight);
}

void iCubeApp::OnSetCursorPos(const float x, const float y) {
    vec2 curMPos(x, y);
    vec2 mouseMove = curMPos - mLastMPos;
    mLastMPos = curMPos;

    if (mCubeFacesMouseDown) {
        mCubeFacesRotation.x = WrapAngle(mCubeFacesRotation.x + mouseMove.x * 0.5f);
        mCubeFacesRotation.y = WrapAngle(mCubeFacesRotation.y + mouseMove.y * 0.5f);
    }
}

void iCubeApp::OnMouseButton(const int button, const int action, const int mods) {
    if (0 == button && GLFW_PRESS == action) {
        if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
            if (mLastMPos.x > mCubeFacesPanelBounds.x &&
                mLastMPos.x < mCubeFacesPanelBounds.z &&
                mLastMPos.y > mCubeFacesPanelBounds.y &&
                mLastMPos.y < mCubeFacesPanelBounds.w) {
                mCubeFacesMouseDown = true;
            }
        }
    } else if (0 == button && GLFW_RELEASE == action) {
        mCubeFacesMouseDown = false;
    }

}

void iCubeApp::OnUpdate(const float dt) {
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
        if (!mEnvImg.IsEmpty()) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            window->DrawList->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
                const ImVec2& displPos = ImGui::GetDrawData()->DisplayPos;
                const ImVec2& displScale = ImGui::GetDrawData()->FramebufferScale;

                vec4 clipRect((cmd->ClipRect.x - displPos.x) * displScale.x,
                              (cmd->ClipRect.y - displPos.y) * displScale.y,
                              (cmd->ClipRect.z - displPos.x) * displScale.x,
                              (cmd->ClipRect.w - displPos.y) * displScale.y);

                iCubeApp* _this = scast<iCubeApp*>(cmd->UserCallbackData);
                _this->DrawPreviewPanel(clipRect);
            }, this);

            // reset state
            window->DrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
        }

        ImGui::Text("%.1f FPS (%.3f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    } ImGui::End();

    // LatLong panel [RT]
    ImGui::SetNextWindowPos(ImVec2(kHalfScreenW, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(kHalfScreenW, kHalfScreenH));
    ImGui::Begin("LatLong:", nullptr, kPanelFlags); {
        const ImTextureID texture = rcast<ImTextureID>(scast<size_t>(mEnvImg.GetTextureLatLong()));
        if (texture) {
            const float textureWidth = scast<float>(mEnvImg.GetLatLongWidth());
            const float textureHeight = scast<float>(mEnvImg.GetLatLongHeight());
            const float textureRatio = textureWidth / textureHeight;

            ImVec2 wndMin = ImGui::GetWindowContentRegionMin();
            ImVec2 wndMax = ImGui::GetWindowContentRegionMax();
            ImVec2 wndSize = ImVec2(wndMax.x - wndMin.x, wndMax.y - wndMin.y);
            const float wndRatio = wndSize.x / wndSize.y;

            const float scale = (wndRatio >= textureRatio) ? wndSize.y / textureHeight : wndSize.x / textureWidth;
            ImVec2 imgSize = ImVec2(textureWidth * scale, textureHeight * scale);
            ImVec2 imgPos = ImVec2((wndSize.x - imgSize.x) * 0.5f + wndMin.x, (wndSize.y - imgSize.y) * 0.5f + wndMin.y);

            ImGui::SetCursorPos(imgPos);
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
            const float textureWidth = scast<float>(mEnvImg.GetCubeCrossWidth());
            const float textureHeight = scast<float>(mEnvImg.GetCubeCrossHeight());
            const float textureRatio = textureWidth / textureHeight;

            ImVec2 wndMin = ImGui::GetWindowContentRegionMin();
            ImVec2 wndMax = ImGui::GetWindowContentRegionMax();
            ImVec2 wndSize = ImVec2(wndMax.x - wndMin.x, wndMax.y - wndMin.y);
            const float wndRatio = wndSize.x / wndSize.y;

            const float scale = (wndRatio >= textureRatio) ? wndSize.y / textureHeight : wndSize.x / textureWidth;
            ImVec2 imgSize = ImVec2(textureWidth * scale, textureHeight * scale);
            ImVec2 imgPos = ImVec2((wndSize.x - imgSize.x) * 0.5f + wndMin.x, (wndSize.y - imgSize.y) * 0.5f + wndMin.y);

            ImGui::SetCursorPos(imgPos);
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
        const ImTextureID texture = rcast<ImTextureID>(scast<size_t>(mEnvImg.GetTextureCubeMap()));
        if (texture) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            window->DrawList->AddCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
                const ImVec2& displPos = ImGui::GetDrawData()->DisplayPos;
                const ImVec2& displScale = ImGui::GetDrawData()->FramebufferScale;

                vec4 clipRect((cmd->ClipRect.x - displPos.x) * displScale.x,
                              (cmd->ClipRect.y - displPos.y) * displScale.y,
                              (cmd->ClipRect.z - displPos.x) * displScale.x,
                              (cmd->ClipRect.w - displPos.y) * displScale.y);

                iCubeApp* _this = scast<iCubeApp*>(cmd->UserCallbackData);
                _this->DrawCubeFaces(clipRect);
            }, this);

            // reset state
            window->DrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
        }

        if (ImGui::Button("Import##CubeFaces")) {
            this->ImportCubeFaces();
        }
        if (ImGui::Button("Export##CubeFaces")) {
            this->ExportCubeFaces();
        }
    } ImGui::End();
}

void iCubeApp::DrawCubeFaces(const vec4& clipRect) {
    mCubeFacesPanelBounds = clipRect;

    const float wndLeftTopX = clipRect.x;
    const float wndLeftTopY = clipRect.w;
    const float wndWidth = clipRect.z - clipRect.x;
    const float wndHeight = clipRect.w - clipRect.y;

    glViewport(scast<GLint>(wndLeftTopX), mHeight - scast<GLint>(wndLeftTopY), scast<GLsizei>(wndWidth), scast<GLsizei>(wndHeight));
    //glScissor(scast<GLint>(wndLeftTopX), mHeight - scast<GLint>(wndLeftTopY), scast<GLsizei>(wndWidth), scast<GLsizei>(wndHeight));

    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(mDrawFacesShader);

    const GLint locProj = glGetUniformLocation(mDrawFacesShader, "gProj");
    const GLint locModel = glGetUniformLocation(mDrawFacesShader, "gModel");
    const GLint locTCube = glGetUniformLocation(mDrawFacesShader, "tCubeMap");

    mat4 model = glm::rotate(mat4(1.0f), Deg2Rad(mCubeFacesRotation.x), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, Deg2Rad(mCubeFacesRotation.y), vec3(1.0f, 0.0f, 0.0f));
    model[3] = vec4(0.0f, 0.0f, -3.5f, 1.0f);

    mat4 proj = MatPerspective(Deg2Rad(60.0f), wndWidth / wndHeight, 0.1f, 15.0f);

    if (locProj >= 0) {
        glUniformMatrix4fv(locProj, 1, GL_FALSE, MatToPtr(proj));
    }
    if (locModel >= 0) {
        glUniformMatrix4fv(locModel, 1, GL_FALSE, MatToPtr(model));
    }
    if (locTCube >= 0) {
        glUniform1i(locTCube, 0);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvImg.GetTextureCubeMap());

    // we don't provide any geometry - it'll be generated via vertex shader
    glBindVertexArray(mJunkVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void iCubeApp::DrawPreviewPanel(const vec4& clipRect) {
    mViewerPanelBounds = clipRect;
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


const char gCubeFacesShaderCode[] = {
#include "shaders/simple_cube.glsl"
};

static GLuint CompileGLSLShader(const String& source, const bool isVertex) {
    GLuint shader = glCreateShader(isVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    if (shader != GL_NONE) {
        GLint status = 0;
        const char* shaderSrc[4];
        shaderSrc[0] = "#version 430 core\n";
        shaderSrc[1] = isVertex ? "#define VERTEX_SHADER\n\n" : "#define FRAGMENT_SHADER\nprecision highp float;\n\n";
        shaderSrc[2] = "#line 0\n";
        shaderSrc[3] = source.c_str();

        glShaderSource(shader, 4, shaderSrc, 0);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        // we grab the log anyway
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            String log(infoLen, '\0');
            glGetShaderInfoLog(shader, infoLen, nullptr, log.data());
            std::cout << log << std::endl;
        }

        if (!status) {
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}


GLuint CreateGLSLProgram(const String& shaderSource) {
    GLuint program = GL_NONE;
    if (!shaderSource.empty()) {
        const GLuint vertexShader = CompileGLSLShader(shaderSource, true);
        const GLuint fragmentShader = CompileGLSLShader(shaderSource, false);

        if (vertexShader && fragmentShader) {
            program = glCreateProgram();
            if (program != GL_NONE) {
                glAttachShader(program, vertexShader);
                glAttachShader(program, fragmentShader);

                glLinkProgram(program);

                // we don't need our shaders anymore
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);

                // we grab the log anyway
                GLint infoLen = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen > 1) {
                    String log(infoLen, '\0');
                    glGetProgramInfoLog(program, infoLen, nullptr, log.data());
                    std::cout << log << std::endl;
                }

                GLint status = 0;
                glGetProgramiv(program, GL_LINK_STATUS, &status);
                if (!status) {
                    glDeleteProgram(program);
                    program = GL_NONE;
                }
            } else {
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);
            }
        }
    }
    return program;
}

void iCubeApp::PrepareRenderer() {
    mDrawFacesShader = CreateGLSLProgram(gCubeFacesShaderCode);

    glGenVertexArrays(1, &mJunkVAO);
}
