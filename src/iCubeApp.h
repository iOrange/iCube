#pragma once
#include "mycommon.h"
#include "EnvironmentImage.h"

class iCubeApp {
public:
    iCubeApp();
    ~iCubeApp();

    bool        Initialize();
    void        Loop();
    void        Shutdown();

private:
    void        OnResize(const int width, const int height);
    void        OnSetCursorPos(const float x, const float y);
    void        OnMouseButton(const int button, const int action, const int mods);
    void        OnUpdate(const float dt);

    void        DoUI();
    void        DrawCubeFaces(const vec4& clipRect);
    void        DrawPreviewPanel(const vec4& clipRect);

    // LatLong
    void        ImportLatLong(const fs::path& path = fs::path());
    void        ExportLatLong(const fs::path& path = fs::path());
    // CubeCross
    void        ImportCubeCross(const fs::path& path = fs::path());
    void        ExportCubeCross(const fs::path& path = fs::path());
    // CubeFaces
    void        ImportCubeFaces(const Array<fs::path> paths = {});
    void        ExportCubeFaces(const fs::path& path = fs::path());

    // render stuff
    void        PrepareRenderer();
    void        GenerateRoundedCube(const size_t resolution, const float power);

private:
    void*               mWindow;
    int                 mWidth;
    int                 mHeight;
    EnvironmentImage    mEnvImg;

    vec4                mViewerPanelBounds;
    vec4                mCubeFacesPanelBounds;
    vec2                mLastMPos;

    // cube faces draw
    GLuint              mDrawFacesShader;
    GLuint              mJunkVAO;
    bool                mCubeFacesMouseDown;
    vec2                mCubeFacesRotation;

    // viewer draw
    GLuint              mViewerObjectShader;
    GLuint              mViewerVAO;
    GLuint              mViewerVB;
    GLuint              mViewerIB;
    size_t              mViewerNumIndices;
    bool                mViewerMouseDown;
    vec2                mViewerRotation;
};
