#pragma once

#pragma warning(push)
#pragma warning(disable : 4201) // C4201: nonstandard extension used: nameless struct/union
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#pragma warning(pop)

using vec2 = glm::highp_vec2;
using vec3 = glm::highp_vec3;
using vec4 = glm::highp_vec4;
using mat4 = glm::highp_mat4;
using quat = glm::highp_quat;

static const float MM_Pi = 3.14159265358979323846f;
static const float MM_InvPi = 1.0f / MM_Pi;
static const float MM_TwoPi = MM_Pi * 2.0f;
static const float MM_HalfPi = MM_Pi * 0.5f;
static const float MM_Epsilon = 1.192092896e-07f;
static const float MM_OneMinusEpsilon = 0.9999999403953552f;

static const mat4 MatZero = mat4(0.0f);
static const mat4 MatIdentity = mat4(1.0f);


inline float Rad2Deg(const float rad) {
    return rad * (180.0f / MM_Pi);
}

inline float Deg2Rad(const float deg) {
    return deg * (MM_Pi / 180.0f);
}

inline float WrapAngle(const float angle) {
    float result = angle;
    while (result > 360.0f) {
        result -= 360.0f;
    }
    while (result < 0.0f) {
        result += 360.0f;
    }
    return result;
}

template <typename T>
inline T Lerp(const T& a, const T& b, const float t) {
    //return a + (b - a) * t;
    return (a * (1.0f - t)) + (b * t);
}

inline float Sin(const float x) {
    return std::sinf(x);
}

inline float Cos(const float x) {
    return std::cosf(x);
}

inline float Sqrt(const float x) {
    return std::sqrtf(x);
}

inline int32_t Floori(const float x) {
    return static_cast<int32_t>(floorf(x));
}


template <typename T>
inline float Length(const T& v) {
    return glm::length(v);
}

inline float Dot(const vec2& a, const vec2& b) {
    return glm::dot(a, b);
}
inline float Dot(const vec3& a, const vec3& b) {
    return glm::dot(a, b);
}
inline float Dot(const vec4& a, const vec4& b) {
    return glm::dot(a, b);
}

inline vec3 Cross(const vec3& a, const vec3& b) {
    return glm::cross(a, b);
}

inline vec3 Normalize(const vec3& v) {
    return glm::normalize(v);
}
inline quat Normalize(const quat& q) {
    return glm::normalize(q);
}

inline quat QuatAngleAxis(const float angleRad, const vec3& axis) {
    return glm::angleAxis(angleRad, axis);
}
inline vec3 QuatRotate(const quat& q, const vec3& v) {
    return glm::rotate(q, v);
}
inline quat QuatSlerp(const quat& a, const quat& b, const float t) {
    return glm::slerp(a, b, t);
}

inline mat4 MatRotate(const float angle, const float x, const float y, const float z) {
    return glm::rotate(angle, vec3(x, y, z));
}
inline mat4 MatTranslate(const mat4& m, const vec3& v) {
    return glm::translate(m, v);
}
inline mat4 MatOrtho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
    return glm::orthoRH(left, right, bottom, top, zNear, zFar);
}
inline mat4 MatPerspective(const float fovy, const float aspect, const float zNear, const float zFar) {
    return glm::perspectiveRH(fovy, aspect, zNear, zFar);
}
inline mat4 MatLookAt(const vec3& eye, const vec3& center, const vec3& up) {
    return glm::lookAtRH(eye, center, up);
}
inline mat4 MatInverse(const mat4& m) {
    return glm::inverse(m);
}
inline mat4 MatFromQuat(const quat& q) {
    return glm::toMat4(q);
}
inline mat4 MatFromEuler(const vec3& euler) {
    return glm::eulerAngleZYX(euler.z, euler.y, euler.x);
}
inline const float* MatToPtr(const mat4& m) {
    return reinterpret_cast<const float*>(&m);
}


inline vec2 DirToLatLong(const vec3& dir) {
    const float phi = std::atan2f(dir.x, dir.z);
    const float theta = std::acosf(dir.y);

    vec2 result((MM_Pi + phi) * (0.5f / MM_Pi), theta * MM_InvPi);
    return result;
}

inline vec3 LatLongToDir(const vec2& latLong) {
    const float phi = latLong.x * MM_TwoPi;
    const float theta = latLong.y * MM_Pi;

    vec3 result(-std::sinf(theta) * std::sinf(phi),
                 std::cosf(theta),
                -std::sinf(theta) * std::cosf(phi));
    return result;
}
