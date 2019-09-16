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
    void        Resize(const int width, const int height);
    void        Update(const float dt);

    void        DoUI();
    // LatLong
    void        ImportLatLong(const fs::path& path = fs::path());
    void        ExportLatLong(const fs::path& path = fs::path());
    // CubeCross
    void        ImportCubeCross(const fs::path& path = fs::path());
    void        ExportCubeCross(const fs::path& path = fs::path());
    // CubeFaces
    void        ImportCubeFaces(const Array<fs::path> paths = {});
    void        ExportCubeFaces(const fs::path& path = fs::path());

private:
    void*               mWindow;
    int                 mWidth;
    int                 mHeight;
    EnvironmentImage    mEnvImg;
};
