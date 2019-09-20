#pragma once
#include "mycommon.h"
#include "mymath.h"

#include "glad/glad.h"


class EnvironmentImage {
public:
    enum class CubeFace : size_t {
        PosX = 0,
        NegX = 1,
        PosY = 2,
        NegY = 3,
        PosZ = 4,
        NegZ = 5
    };

    static const size_t kNumCubeFaces = 6;

private:
    struct Image2D {
        size_t      width;
        size_t      height;
        Array<vec3> data;
    };

public:
    EnvironmentImage();
    ~EnvironmentImage();

    bool    LoadLatLong(const fs::path& path);
    bool    LoadCubeCross(const fs::path& path);
    bool    LoadCubeFaces(const Array<fs::path>& paths);

    bool    SaveLatLong(const fs::path& path);
    bool    SaveCubeCross(const fs::path& path);
    bool    SaveCubeFaces(const fs::path& path);

    void    Free();
    bool    IsEmpty() const;

    GLuint  GetTextureLatLong() const;
    GLuint  GetTextureCubeCross() const;
    GLuint  GetTextureCubeMap() const;

    size_t  GetLatLongWidth() const;
    size_t  GetLatLongHeight() const;
    size_t  GetCubeCrossWidth() const;
    size_t  GetCubeCrossHeight() const;
    size_t  GetCubeFaceWidth() const;
    size_t  GetCubeFaceHeight() const;

    vec3    SampleCube(const vec3& dir) const;
    vec3    SampleLatLong(const vec3& dir) const;

private:
    bool    LoadImage2D(const fs::path& path, Image2D& img);
    bool    SaveImage2D(const fs::path& path, Image2D& img);

    vec3    SampleImage2D(const Image2D& img, const float u, const float v) const;

    void    LatLongToCubeFaces();
    void    CubeFacesToCubeCross();
    void    CubeCrossToCubeFaces();
    void    CubeFacesToLatLong();

    void    CreateOpenGLTextures();

private:
    Image2D         mLatLong;
    Image2D         mCubeCross;
    Array<Image2D>  mCubeFaces;

    GLuint          mTextureLatLong;
    GLuint          mTextureCubeCross;
    GLuint          mTextureCubeMap;
};
