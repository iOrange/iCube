#include "EnvironmentImage.h"

#define STBI_NO_PSD
#define STBI_NO_GIF
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_STATIC
#include "stb_image_write.h"


// Vertical cross layout
//
//      |+Y|
//   |-X|+Z|+X|
//      |-Y|
//      |-Z|
//
static const size_t sVerticalCrossOffsetsX[EnvironmentImage::kNumCubeFaces] = {
    /*+X*/2, /*-X*/0, /*+Y*/1, /*-Y*/1, /*+Z*/1, /*-Z*/1,
};
static const size_t sVerticalCrossOffsetsY[EnvironmentImage::kNumCubeFaces] = {
    /*+X*/1, /*-X*/1, /*+Y*/0, /*-Y*/2, /*+Z*/1, /*-Z*/3,
};

static const vec3 sFaceUvVectors[EnvironmentImage::kNumCubeFaces][3] = {
    { // +x face
        {  0.0f,  0.0f, -1.0f }, // u -> -z
        {  0.0f, -1.0f,  0.0f }, // v -> -y
        {  1.0f,  0.0f,  0.0f }, // +x face
    },
    { // -x face
        {  0.0f,  0.0f,  1.0f }, // u -> +z
        {  0.0f, -1.0f,  0.0f }, // v -> -y
        { -1.0f,  0.0f,  0.0f }, // -x face
    },
    { // +y face
        {  1.0f,  0.0f,  0.0f }, // u -> +x
        {  0.0f,  0.0f,  1.0f }, // v -> +z
        {  0.0f,  1.0f,  0.0f }, // +y face
    },
    { // -y face
        {  1.0f,  0.0f,  0.0f }, // u -> +x
        {  0.0f,  0.0f, -1.0f }, // v -> -z
        {  0.0f, -1.0f,  0.0f }, // -y face
    },
    { // +z face
        {  1.0f,  0.0f,  0.0f }, // u -> +x
        {  0.0f, -1.0f,  0.0f }, // v -> -y
        {  0.0f,  0.0f,  1.0f }, // +z face
    },
    { // -z face
        { -1.0f,  0.0f,  0.0f }, // u -> -x
        {  0.0f, -1.0f,  0.0f }, // v -> -y
        {  0.0f,  0.0f, -1.0f }, // -z face
    }
};

static const String sFacesFilenameSuffixes[EnvironmentImage::kNumCubeFaces] = {
    "_px", "_nx", "_py", "_ny", "_pz", "_nz"
};


EnvironmentImage::EnvironmentImage()
    : mTextureLatLong(0)
    , mTextureCubeCross(0)
    , mTextureCubeMap(0)
{
}
EnvironmentImage::~EnvironmentImage() {
    this->Free();
}

bool EnvironmentImage::LoadLatLong(const fs::path& path) {
    bool result = false;

    this->Free();

    if (this->LoadImage2D(path, mLatLong)) {
        this->LatLongToCubeFaces();
        this->CubeFacesToCubeCross();

        this->CreateOpenGLTextures();

        result = true;
    }

    return result;
}

bool EnvironmentImage::LoadCubeCross(const fs::path& path) {
    bool result = false;

    this->Free();

    if (this->LoadImage2D(path, mCubeCross)) {
        this->CubeCrossToCubeFaces();
        this->CubeFacesToLatLong();

        this->CreateOpenGLTextures();

        result = true;
    }

    return result;
}

bool EnvironmentImage::LoadCubeFaces(const Array<fs::path>& paths) {
    this->Free();
    return false;
}

bool EnvironmentImage::SaveLatLong(const fs::path& path) {
    if (mLatLong.data.empty()) {
        return false;
    } else {
        return this->SaveImage2D(path, mLatLong);
    }
}

bool EnvironmentImage::SaveCubeCross(const fs::path& path) {
    if (mCubeCross.data.empty()) {
        return false;
    } else {
        return this->SaveImage2D(path, mCubeCross);
    }
}

bool EnvironmentImage::SaveCubeFaces(const fs::path& path) {
    bool result = false;

    if (!mCubeFaces.empty()) {
        fs::path rootFolder = path.parent_path();
        fs::path fileName = path.stem();
        String extension = path.extension().u8string();

        for (size_t i = 0; i < EnvironmentImage::kNumCubeFaces; ++i) {
            const String& suffix = sFacesFilenameSuffixes[i];

            fs::path facePath = rootFolder / fileName;
            facePath += suffix + extension;

            result = this->SaveImage2D(facePath, mCubeFaces[i]);
            if (!result) {
                break;
            }
        }
    }

    return result;
}

void EnvironmentImage::Free() {
    mLatLong = {};
    mCubeCross = {};
    mCubeFaces = {};

    if (mTextureLatLong) {
        glDeleteTextures(1, &mTextureLatLong);
        mTextureLatLong = 0;
    }
    if (mTextureCubeCross) {
        glDeleteTextures(1, &mTextureCubeCross);
        mTextureCubeCross = 0;
    }
    if (mTextureCubeMap) {
        glDeleteTextures(1, &mTextureCubeMap);
        mTextureCubeMap = 0;
    }
}

bool EnvironmentImage::IsEmpty() const {
    return mCubeFaces.empty();
}

GLuint EnvironmentImage::GetTextureLatLong() const {
    return mTextureLatLong;
}

GLuint EnvironmentImage::GetTextureCubeCross() const {
    return mTextureCubeCross;
}

GLuint EnvironmentImage::GetTextureCubeMap() const {
    return mTextureCubeMap;
}

size_t EnvironmentImage::GetLatLongWidth() const {
    return mLatLong.width;
}

size_t EnvironmentImage::GetLatLongHeight() const {
    return mLatLong.height;
}

size_t EnvironmentImage::GetCubeCrossWidth() const {
    return mCubeCross.width;
}

size_t EnvironmentImage::GetCubeCrossHeight() const {
    return mCubeCross.height;
}

size_t EnvironmentImage::GetCubeFaceWidth() const {
    return mCubeFaces.empty() ? size_t(0) : mCubeFaces.front().width;
}

size_t EnvironmentImage::GetCubeFaceHeight() const {
    return mCubeFaces.empty() ? size_t(0) : mCubeFaces.front().height;
}

vec3 EnvironmentImage::SampleCube(const vec3& dir) const {
    const vec3 absVec(std::fabsf(dir.x), std::fabsf(dir.y), std::fabsf(dir.z));

    const float maxAxis = Maximum(Maximum(absVec.x, absVec.y), absVec.z);

    // Get face id (max component == face vector).
    CubeFace face;
    if (maxAxis == absVec.x) {
        face = (dir.x >= 0.0f) ? CubeFace::PosX : CubeFace::NegX;
    } else if (maxAxis == absVec.y) {
        face = (dir.y >= 0.0f) ? CubeFace::PosY : CubeFace::NegY;
    } else /* if (maxAxis == absVec.z) */ {
        face = (dir.z >= 0.0f) ? CubeFace::PosZ : CubeFace::NegZ;
    }

    vec3 faceVec = dir / maxAxis;

    // Project other two components to face uv basis.
    const float u = (Dot(sFaceUvVectors[scast<size_t>(face)][0], faceVec) + 1.0f) * 0.5f;
    const float v = (Dot(sFaceUvVectors[scast<size_t>(face)][1], faceVec) + 1.0f) * 0.5f;

    return this->SampleImage2D(mCubeFaces[scast<size_t>(face)], u, v);
}

vec3 EnvironmentImage::SampleLatLong(const vec3& dir) const {
    const vec2 uv = DirToLatLong(dir);
    return this->SampleImage2D(mLatLong, uv.x, uv.y);
}


bool EnvironmentImage::LoadImage2D(const fs::path& path, Image2D& img) {
    bool result = false;

    const String pathUtf8 = path.u8string();

    int width, height, comp;
    float* imgData = stbi_loadf(pathUtf8.c_str(), &width, &height, &comp, STBI_rgb);

    if (imgData != nullptr) {
        img.width = scast<size_t>(width);
        img.height = scast<size_t>(height);

        img.data.resize(width * height);
        memcpy(img.data.data(), imgData, width * height * sizeof(vec3));

        stbi_image_free(imgData);

        result = true;
    }

    return result;
}

bool EnvironmentImage::SaveImage2D(const fs::path& path, Image2D& img) {
    bool result = false;

    const String pathUtf8 = path.u8string();
    const String extension = path.extension().u8string();

    int stbiRet = 0;
    if (extension == ".hdr") {
        stbiRet = stbi_write_hdr(pathUtf8.c_str(), scast<int>(img.width), scast<int>(img.height), STBI_rgb, rcast<const float*>(img.data.data()));
    } else {
        const size_t numPixels = img.data.size();
        Array<uint8_t> ldrPixels(numPixels * 3);

        for (size_t i = 0; i < numPixels; ++i) {
            const vec3& hdrPixel = img.data[i];

            ldrPixels[i * 3 + 0] = scast<uint8_t>(scast<uint32_t>(hdrPixel.x * 255.0f) & 0xFF);
            ldrPixels[i * 3 + 1] = scast<uint8_t>(scast<uint32_t>(hdrPixel.y * 255.0f) & 0xFF);
            ldrPixels[i * 3 + 2] = scast<uint8_t>(scast<uint32_t>(hdrPixel.z * 255.0f) & 0xFF);
        }

        if (extension == ".bmp") {
            stbiRet = stbi_write_bmp(pathUtf8.c_str(), scast<int>(img.width), scast<int>(img.height), STBI_rgb, ldrPixels.data());
        } else if (extension == ".jpg") {
            stbiRet = stbi_write_jpg(pathUtf8.c_str(), scast<int>(img.width), scast<int>(img.height), STBI_rgb, ldrPixels.data(), 95);
        } else if (extension == ".tga") {
            stbiRet = stbi_write_tga(pathUtf8.c_str(), scast<int>(img.width), scast<int>(img.height), STBI_rgb, ldrPixels.data());
        } else if (extension == ".png") {
            stbiRet = stbi_write_png(pathUtf8.c_str(), scast<int>(img.width), scast<int>(img.height), STBI_rgb, ldrPixels.data(), 0);
        }
    }

    result = (stbiRet != 0);

    return result;
}

vec3 EnvironmentImage::SampleImage2D(const Image2D& img, const float u, const float v) const {
#if 0
    // Nearest
    const size_t ix = scast<size_t>(img.width * u) % img.width;
    const size_t iy = scast<size_t>(img.height * v) % img.height;
    return img.data[ix + iy * img.width];
#else
    // Bilinear
    float m;
    float mu = std::modf(u, &m);
    float mv = std::modf(v, &m);

    const float fu = mu * scast<float>(img.width);
    const float fv = mv * scast<float>(img.height);

    const size_t u00 = scast<size_t>(fu);
    const size_t u01 = (u00 + 1) % img.width;
    const size_t u10 = u00;
    const size_t u11 = u01;
    const size_t v00 = scast<size_t>(fv);
    const size_t v01 = v00;
    const size_t v10 = (v00 + 1) % img.height;
    const size_t v11 = v10;

    const float ku = fu - u00;
    const float kv = fv - v00;
    const float kuv = ku * kv;

    const float k00 = 1 - ku - kv + kuv;
    const float k01 = ku - kuv;
    const float k10 = kv - kuv;
    const float k11 = kuv;

    const vec3& color00 = img.data[u00 + v00 * img.width];
    const vec3& color01 = img.data[u01 + v01 * img.width];
    const vec3& color10 = img.data[u10 + v10 * img.width];
    const vec3& color11 = img.data[u11 + v11 * img.width];

    // Interpolating result
    vec3 ret = color00 * k00 + color01 * k01 + color10 * k10 + color11 * k11;
    return ret;
#endif
}

void EnvironmentImage::LatLongToCubeFaces() {
    const size_t latLongWidth = mLatLong.width;
    const size_t latLongHeight = mLatLong.height;

    const size_t faceWidth = latLongWidth / 4;
    const size_t faceHeight = latLongHeight / 2;

    const float invHalfFaceWidth = 2.0f / scast<float>(faceWidth - 1);
    const float invHalfFaceHeight = 2.0f / scast<float>(faceHeight - 1);

    mCubeFaces.resize(kNumCubeFaces);
    const vec3* crossData = mCubeCross.data.data();
    for (size_t i = 0; i < kNumCubeFaces; ++i) {
        Image2D& faceImg = mCubeFaces[i];
        faceImg.width = faceWidth;
        faceImg.height = faceHeight;
        faceImg.data.resize(faceWidth * faceHeight);
        vec3* cubeFacePtr = faceImg.data.data();

        for (size_t y = 0; y < faceHeight; ++y) {
            const float fv = (scast<float>(y) * invHalfFaceHeight) - 1.0f;

            for (size_t x = 0; x < faceWidth; ++x, ++cubeFacePtr) {
                const float fu = (scast<float>(x) * invHalfFaceWidth) - 1.0f;

                const vec3 dir = Normalize(sFaceUvVectors[i][0] * fu + sFaceUvVectors[i][1] * fv + sFaceUvVectors[i][2]);

                *cubeFacePtr = this->SampleLatLong(dir);
            }
        }
    }
}

void EnvironmentImage::CubeFacesToCubeCross() {
    if (!mCubeFaces.empty()) {
        const size_t faceWidth = mCubeFaces.front().width;
        const size_t faceHeight = mCubeFaces.front().height;

        const size_t crossWidth = faceWidth * 3;
        const size_t crossHeight = faceHeight * 4;

        mCubeCross.width = crossWidth;
        mCubeCross.height = crossHeight;
        mCubeCross.data.resize(crossWidth * crossHeight);

        vec3* crossData = mCubeCross.data.data();
        for (size_t i = 0; i < kNumCubeFaces; ++i) {
            const size_t crossFaceOffX = sVerticalCrossOffsetsX[i];
            const size_t crossFaceOffY = sVerticalCrossOffsetsY[i];

            const vec3* srcFacePtr = mCubeFaces[i].data.data();
            vec3* crossFacePtr = crossData + (crossFaceOffY * faceHeight * crossWidth) + (crossFaceOffX * faceWidth);

            if (i < scast<size_t>(CubeFace::NegZ)) {
                for (size_t line = 0; line < faceHeight; ++line) {
                    memcpy(crossFacePtr, srcFacePtr, faceWidth * sizeof(vec3));

                    srcFacePtr += faceWidth;
                    crossFacePtr += crossWidth;
                }
            } else { // back face needs to be rotated 180
                crossFacePtr += ((faceHeight - 1) * crossWidth) + faceWidth - 1;
                for (size_t y = 0; y < faceHeight; ++y) {
                    for (size_t x = 0; x < faceWidth; ++x, ++srcFacePtr, --crossFacePtr) {
                        *crossFacePtr = *srcFacePtr;
                    }
                    crossFacePtr -= (crossWidth - faceWidth);
                }
            }
        }
    }
}

void EnvironmentImage::CubeCrossToCubeFaces() {
    const size_t crossWidth = mCubeCross.width;
    const size_t crossHeight = mCubeCross.height;

    const size_t faceWidth = crossWidth / 3;
    const size_t faceHeight = crossHeight / 4;

    mCubeFaces.resize(kNumCubeFaces);
    const vec3* crossData = mCubeCross.data.data();
    for (size_t i = 0; i < kNumCubeFaces; ++i) {
        const size_t crossFaceOffX = sVerticalCrossOffsetsX[i];
        const size_t crossFaceOffY = sVerticalCrossOffsetsY[i];
        const vec3* crossFacePtr = crossData + (crossFaceOffY * faceHeight * crossWidth) + (crossFaceOffX * faceWidth);

        Image2D& faceImg = mCubeFaces[i];
        faceImg.width = faceWidth;
        faceImg.height = faceHeight;
        faceImg.data.resize(faceWidth * faceHeight);
        vec3* cubeFacePtr = faceImg.data.data();

        if (i < scast<size_t>(CubeFace::NegZ)) {
            for (size_t line = 0; line < faceHeight; ++line) {
                memcpy(cubeFacePtr, crossFacePtr, faceWidth * sizeof(vec3));

                cubeFacePtr += faceWidth;
                crossFacePtr += crossWidth;
            }
        } else { // back face needs to be rotated 180
            crossFacePtr += ((faceHeight - 1) * crossWidth) + faceWidth - 1;
            for (size_t y = 0; y < faceHeight; ++y) {
                for (size_t x = 0; x < faceWidth; ++x, ++cubeFacePtr, --crossFacePtr) {
                    *cubeFacePtr = *crossFacePtr;
                }
                crossFacePtr -= (crossWidth - faceWidth);
            }
        }
    }
}

void EnvironmentImage::CubeFacesToLatLong() {
    if (!mCubeFaces.empty()) {
        const size_t faceWidth = mCubeFaces.front().width;
        const size_t faceHeight = mCubeFaces.front().height;

        const size_t latLongWidth = faceWidth * 4;
        const size_t latLongHeight = faceHeight * 2;

        mLatLong.width = latLongWidth;
        mLatLong.height = latLongHeight;
        mLatLong.data.resize(latLongWidth * latLongHeight);

        const float invLatLongWidth = 1.0f / scast<float>(latLongWidth - 1);
        const float invLatLongHeight = 1.0f / scast<float>(latLongHeight - 1);

        vec3* latLongData = mLatLong.data.data();
        for (size_t y = 0; y < latLongHeight; ++y) {
            const float fv = scast<float>(y) * invLatLongHeight;

            for (size_t x = 0; x < latLongWidth; ++x, ++latLongData) {
                const float fu = scast<float>(x) * invLatLongWidth;
                const vec3 dir = LatLongToDir(vec2(fu, fv));

                *latLongData = this->SampleCube(dir);
            }
        }
    }
}

void EnvironmentImage::CreateOpenGLTextures() {
    GLenum target = GL_TEXTURE_2D;

#define SETUP_SAMPLER                                               \
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      \
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);      \
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);   \
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);   \
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);   \
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);              \
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);


    if (!mLatLong.data.empty()) {
        glGenTextures(1, &mTextureLatLong);
        glBindTexture(target, mTextureLatLong);
        SETUP_SAMPLER
        glTexImage2D(target, 0, GL_RGB32F, scast<GLsizei>(mLatLong.width), scast<GLsizei>(mLatLong.height), 0, GL_RGB, GL_FLOAT, mLatLong.data.data());
    }

    if (!mCubeCross.data.empty()) {
        glGenTextures(1, &mTextureCubeCross);
        glBindTexture(target, mTextureCubeCross);
        SETUP_SAMPLER
        glTexImage2D(target, 0, GL_RGB32F, scast<GLsizei>(mCubeCross.width), scast<GLsizei>(mCubeCross.height), 0, GL_RGB, GL_FLOAT, mCubeCross.data.data());
    }

    if (!mCubeFaces.empty()) {
        target = GL_TEXTURE_CUBE_MAP;

        glGenTextures(1, &mTextureCubeMap);
        glBindTexture(target, mTextureCubeMap);
        SETUP_SAMPLER

        const GLsizei faceWidth = scast<GLsizei>(mCubeFaces.front().width);
        const GLsizei faceHeight = scast<GLsizei>(mCubeFaces.front().height);

        for (size_t i = 0; i < kNumCubeFaces; ++i) {
            const Image2D& faceImg = mCubeFaces[i];

            const GLuint face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + scast<GLuint>(i);
            glTexImage2D(face, 0, GL_RGB32F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, faceImg.data.data());
        }
    }
}
