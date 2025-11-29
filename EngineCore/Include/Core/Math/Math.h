#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

namespace Yamen {
namespace Core {

using namespace DirectX;

// Forward declarations
struct vec2;
struct vec3;
struct vec4;
struct mat4;
struct quat;

// Vector2 Wrapper
struct vec2 : public XMFLOAT2 {
  vec2() : XMFLOAT2(0.f, 0.f) {}
  vec2(float x, float y) : XMFLOAT2(x, y) {}
  vec2(float s) : XMFLOAT2(s, s) {}
  vec2(const XMFLOAT2 &v) : XMFLOAT2(v) {}

  vec2 operator+(const vec2 &rhs) const { return vec2(x + rhs.x, y + rhs.y); }
  vec2 operator-(const vec2 &rhs) const { return vec2(x - rhs.x, y - rhs.y); }
  vec2 operator*(float s) const { return vec2(x * s, y * s); }
  vec2 operator/(float s) const { return vec2(x / s, y / s); }

  vec2 &operator+=(const vec2 &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  vec2 &operator-=(const vec2 &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }
  vec2 &operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }

  float &operator[](int i) { return reinterpret_cast<float *>(this)[i]; }
  const float &operator[](int i) const {
    return reinterpret_cast<const float *>(this)[i];
  }

  operator float *() { return &x; }
  operator const float *() const { return &x; }
};

// Vector3 Wrapper
struct vec3 : public XMFLOAT3 {
  vec3() : XMFLOAT3(0.f, 0.f, 0.f) {}
  vec3(float x, float y, float z) : XMFLOAT3(x, y, z) {}
  vec3(float s) : XMFLOAT3(s, s, s) {}
  vec3(const XMFLOAT3 &v) : XMFLOAT3(v) {}
  vec3(const XMVECTOR &v) { XMStoreFloat3(this, v); }
  vec3(const float *p) : XMFLOAT3(p) {}

  vec3 operator+(const vec3 &rhs) const {
    XMVECTOR v1 = XMLoadFloat3(this);
    XMVECTOR v2 = XMLoadFloat3(&rhs);
    return vec3(XMVectorAdd(v1, v2));
  }
  vec3 operator-(const vec3 &rhs) const {
    XMVECTOR v1 = XMLoadFloat3(this);
    XMVECTOR v2 = XMLoadFloat3(&rhs);
    return vec3(XMVectorSubtract(v1, v2));
  }
  vec3 operator*(float s) const {
    XMVECTOR v1 = XMLoadFloat3(this);
    return vec3(XMVectorScale(v1, s));
  }
  vec3 operator*(const vec3 &rhs) const {
    XMVECTOR v1 = XMLoadFloat3(this);
    XMVECTOR v2 = XMLoadFloat3(&rhs);
    return vec3(XMVectorMultiply(v1, v2));
  }
  vec3 operator/(float s) const {
    XMVECTOR v1 = XMLoadFloat3(this);
    return vec3(XMVectorScale(v1, 1.0f / s));
  }
  vec3 operator-() const { return vec3(-x, -y, -z); }

  vec3 &operator+=(const vec3 &rhs) {
    *this = *this + rhs;
    return *this;
  }
  vec3 &operator-=(const vec3 &rhs) {
    *this = *this - rhs;
    return *this;
  }
  vec3 &operator*=(float s) {
    *this = *this * s;
    return *this;
  }
  vec3 &operator/=(float s) {
    *this = *this / s;
    return *this;
  }

  bool operator==(const vec3 &rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }
  bool operator!=(const vec3 &rhs) const { return !(*this == rhs); }

  float &operator[](int i) { return reinterpret_cast<float *>(this)[i]; }
  const float &operator[](int i) const {
    return reinterpret_cast<const float *>(this)[i];
  }

  operator float *() { return &x; }
  operator const float *() const { return &x; }
};

// Vector4 Wrapper
struct vec4 : public XMFLOAT4 {
  vec4() : XMFLOAT4(0.f, 0.f, 0.f, 0.f) {}
  vec4(float x, float y, float z, float w) : XMFLOAT4(x, y, z, w) {}
  vec4(float s) : XMFLOAT4(s, s, s, s) {}
  vec4(const vec3 &v, float w) : XMFLOAT4(v.x, v.y, v.z, w) {}
  vec4(const XMFLOAT4 &v) : XMFLOAT4(v) {}
  vec4(const XMVECTOR &v) { XMStoreFloat4(this, v); }
  vec4(const float *p) : XMFLOAT4(p) {}

  vec4 operator+(const vec4 &rhs) const {
    XMVECTOR v1 = XMLoadFloat4(this);
    XMVECTOR v2 = XMLoadFloat4(&rhs);
    return vec4(XMVectorAdd(v1, v2));
  }
  vec4 operator*(float s) const {
    XMVECTOR v1 = XMLoadFloat4(this);
    return vec4(XMVectorScale(v1, s));
  }

  float &operator[](int i) { return reinterpret_cast<float *>(this)[i]; }
  const float &operator[](int i) const {
    return reinterpret_cast<const float *>(this)[i];
  }

  operator float *() { return &x; }
  operator const float *() const { return &x; }
};

// Quaternion Wrapper
struct quat : public XMFLOAT4 {
  quat() : XMFLOAT4(0.f, 0.f, 0.f, 1.f) {}
  quat(float x, float y, float z, float w) : XMFLOAT4(x, y, z, w) {}
  quat(const XMFLOAT4 &v) : XMFLOAT4(v) {}
  quat(const XMVECTOR &v) { XMStoreFloat4(this, v); }

  // Construct from Euler angles (pitch, yaw, roll)
  quat(const vec3 &euler) {
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
    XMStoreFloat4(this, q);
  }

  quat operator*(const quat &rhs) const {
    XMVECTOR q1 = XMLoadFloat4(this);
    XMVECTOR q2 = XMLoadFloat4(&rhs);
    return quat(XMQuaternionMultiply(q1, q2));
  }

  quat operator-() const { return quat(-x, -y, -z, -w); }

  operator float *() { return &x; }
  operator const float *() const { return &x; }
};

// Matrix4 Wrapper
struct mat4 : public XMFLOAT4X4 {
  mat4() { XMStoreFloat4x4(this, XMMatrixIdentity()); }
  mat4(const XMFLOAT4X4 &m) : XMFLOAT4X4(m) {}
  mat4(const XMMATRIX &m) { XMStoreFloat4x4(this, m); }
  mat4(float val) {
    if (val == 1.0f)
      XMStoreFloat4x4(this, XMMatrixIdentity());
    else {
      XMStoreFloat4x4(this, XMMatrixIdentity());
      _11 = _22 = _33 = _44 = val;
    }
  }
  // Construct from rows
  mat4(const vec4 &r0, const vec4 &r1, const vec4 &r2, const vec4 &r3) {
    m[0][0] = r0.x;
    m[0][1] = r0.y;
    m[0][2] = r0.z;
    m[0][3] = r0.w;
    m[1][0] = r1.x;
    m[1][1] = r1.y;
    m[1][2] = r1.z;
    m[1][3] = r1.w;
    m[2][0] = r2.x;
    m[2][1] = r2.y;
    m[2][2] = r2.z;
    m[2][3] = r2.w;
    m[3][0] = r3.x;
    m[3][1] = r3.y;
    m[3][2] = r3.z;
    m[3][3] = r3.w;
    m[3][0] = r3.x;
    m[3][1] = r3.y;
    m[3][2] = r3.z;
    m[3][3] = r3.w;
  }
  // Construct from floats
  mat4(float m00, float m01, float m02, float m03, float m10, float m11,
       float m12, float m13, float m20, float m21, float m22, float m23,
       float m30, float m31, float m32, float m33) {
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[0][3] = m03;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[1][3] = m13;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
    m[2][3] = m23;
    m[3][0] = m30;
    m[3][1] = m31;
    m[3][2] = m32;
    m[3][3] = m33;
  }

  mat4 operator*(const mat4 &rhs) const {
    XMMATRIX m1 = XMLoadFloat4x4(this);
    XMMATRIX m2 = XMLoadFloat4x4(&rhs);
    return mat4(XMMatrixMultiply(m1, m2));
  }

  vec4 operator*(const vec4 &rhs) const {
    XMVECTOR v = XMLoadFloat4(&rhs);
    XMMATRIX m = XMLoadFloat4x4(this);
    return vec4(XMVector4Transform(v, m));
  }

  // Access operator for rows
  float *operator[](int row) { return m[row]; }
  const float *operator[](int row) const { return m[row]; }

  operator float *() { return &m[0][0]; }
  operator const float *() const { return &m[0][0]; }
};

// Math Utils
namespace Math {
constexpr float PI = XM_PI;

inline float Radians(float degrees) { return XMConvertToRadians(degrees); }
inline float Degrees(float radians) { return XMConvertToDegrees(radians); }

inline vec3 Normalize(const vec3 &v) {
  return vec3(XMVector3Normalize(XMLoadFloat3(&v)));
}

inline float Dot(const vec3 &a, const vec3 &b) {
  return XMVectorGetX(XMVector3Dot(XMLoadFloat3(&a), XMLoadFloat3(&b)));
}

inline float Dot(const quat &a, const quat &b) {
  return XMVectorGetX(XMQuaternionDot(XMLoadFloat4(&a), XMLoadFloat4(&b)));
}

inline vec3 Cross(const vec3 &a, const vec3 &b) {
  return vec3(XMVector3Cross(XMLoadFloat3(&a), XMLoadFloat3(&b)));
}

inline float Length(const vec3 &v) {
  return XMVectorGetX(XMVector3Length(XMLoadFloat3(&v)));
}

inline float LengthSq(const vec3 &v) {
  return XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&v)));
}

inline mat4 Translate(const mat4 &m, const vec3 &v) {
  XMMATRIX mat = XMLoadFloat4x4(&m);
  XMMATRIX trans = XMMatrixTranslation(v.x, v.y, v.z);
  return mat4(XMMatrixMultiply(trans, mat)); // Apply translation
}

inline mat4 Translate(const vec3 &v) {
  return mat4(XMMatrixTranslation(v.x, v.y, v.z));
}

inline mat4 Rotate(const mat4 &m, float angle, const vec3 &axis) {
  XMMATRIX mat = XMLoadFloat4x4(&m);
  XMMATRIX rot = XMMatrixRotationAxis(XMLoadFloat3(&axis), angle);
  return mat4(XMMatrixMultiply(rot, mat));
}

inline mat4 Scale(const mat4 &m, const vec3 &v) {
  XMMATRIX mat = XMLoadFloat4x4(&m);
  XMMATRIX scale = XMMatrixScaling(v.x, v.y, v.z);
  return mat4(XMMatrixMultiply(scale, mat));
}

inline mat4 Perspective(float fov, float aspect, float nearPlane,
                        float farPlane) {
  return mat4(XMMatrixPerspectiveFovLH(fov, aspect, nearPlane,
                                       farPlane)); // Left Handed for DX
}

inline mat4 Ortho(float left, float right, float bottom, float top,
                  float nearPlane, float farPlane) {
  return mat4(XMMatrixOrthographicOffCenterLH(left, right, bottom, top,
                                              nearPlane, farPlane));
}

inline mat4 Inverse(const mat4 &m) {
  XMVECTOR det;
  return mat4(XMMatrixInverse(&det, XMLoadFloat4x4(&m)));
}

inline mat4 Transpose(const mat4 &m) {
  return mat4(XMMatrixTranspose(XMLoadFloat4x4(&m)));
}

inline float Determinant(const mat4 &m) {
  return XMVectorGetX(XMMatrixDeterminant(XMLoadFloat4x4(&m)));
}

inline mat4 ToMat4(const quat &q) {
  return mat4(XMMatrixRotationQuaternion(XMLoadFloat4(&q)));
}

inline quat ToQuat(const mat4 &m) {
  return quat(XMQuaternionRotationMatrix(XMLoadFloat4x4(&m)));
}

inline vec3 ToEulerAngles(const quat &q) {
  // Convert quaternion to euler angles (Pitch, Yaw, Roll)
  float pitch, yaw, roll;
  // Extract from rotation matrix is often easier/safer
  XMMATRIX m = XMMatrixRotationQuaternion(XMLoadFloat4(&q));
  // Decompose? No, just manual extraction or use a helper if available.
  // DirectXMath doesn't have direct Quat->Euler.
  // Let's use a standard formula for now.

  float x = q.x, y = q.y, z = q.z, w = q.w;

  // Roll (x-axis rotation)
  float sinr_cosp = 2 * (w * x + y * z);
  float cosr_cosp = 1 - 2 * (x * x + y * y);
  roll = std::atan2(sinr_cosp, cosr_cosp);

  // Pitch (y-axis rotation)
  float sinp = 2 * (w * y - z * x);
  if (std::abs(sinp) >= 1)
    pitch = std::copysign(XM_PI / 2, sinp); // use 90 degrees if out of range
  else
    pitch = std::asin(sinp);

  // Yaw (z-axis rotation)
  float siny_cosp = 2 * (w * z + x * y);
  float cosy_cosp = 1 - 2 * (y * y + z * z);
  yaw = std::atan2(siny_cosp, cosy_cosp);

  return vec3(roll, pitch, yaw); // Order might need verification
}

inline quat AngleAxis(float angle, const vec3 &axis) {
  return quat(XMQuaternionRotationAxis(XMLoadFloat3(&axis), angle));
}

inline vec3 Rotate(const quat &q, const vec3 &v) {
  return vec3(XMVector3Rotate(XMLoadFloat3(&v), XMLoadFloat4(&q)));
}

inline quat Slerp(const quat &a, const quat &b, float t) {
  return quat(XMQuaternionSlerp(XMLoadFloat4(&a), XMLoadFloat4(&b), t));
}

inline vec3 Lerp(const vec3 &a, const vec3 &b, float t) {
  return vec3(XMVectorLerp(XMLoadFloat3(&a), XMLoadFloat3(&b), t));
}

inline float Clamp(float v, float min, float max) {
  return std::max(min, std::min(v, max));
}

inline vec3 Clamp(const vec3 &v, float min, float max) {
  return vec3(Clamp(v.x, min, max), Clamp(v.y, min, max), Clamp(v.z, min, max));
}

inline vec3 Clamp(const vec3 &v, const vec3 &min, const vec3 &max) {
  return vec3(Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y),
              Clamp(v.z, min.z, max.z));
}

// Full LookAt matrix (Left-Handed, +Z forward — standard DirectX style)
inline mat4 LookAtLH(const vec3 &eye, const vec3 &target,
                     const vec3 &up = vec3(0, 1, 0)) {
  return mat4(XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target),
                               XMLoadFloat3(&up)));
}

// One-liner version returning a quaternion (most useful for characters/turrets)
inline quat LookAtRotation(const vec3 &from, const vec3 &to,
                           const vec3 &up = vec3(0, 1, 0)) {
  return ToQuat(LookAtLH(from, to, up));
}

// Extra convenience: directly get forward → target direction (normalized)
inline vec3 LookDirection(const vec3 &from, const vec3 &to) {
  return Normalize(to - from);
}

template <typename T> inline T Max(T a, T b) { return std::max(a, b); }
} // namespace Math

// Axis Aligned Bounding Box
struct AABB {
  vec3 Min;
  vec3 Max;

  AABB() : Min(FLT_MAX), Max(-FLT_MAX) {}
  AABB(const vec3 &min, const vec3 &max) : Min(min), Max(max) {}

  vec3 GetCenter() const { return (Min + Max) * 0.5f; }
  vec3 GetExtents() const { return (Max - Min) * 0.5f; }
  vec3 GetSize() const { return Max - Min; }

  bool Contains(const vec3 &point) const {
    return point.x >= Min.x && point.x <= Max.x && point.y >= Min.y &&
           point.y <= Max.y && point.z >= Min.z && point.z <= Max.z;
  }

  bool Intersects(const AABB &other) const {
    return (Min.x <= other.Max.x && Max.x >= other.Min.x) &&
           (Min.y <= other.Max.y && Max.y >= other.Min.y) &&
           (Min.z <= other.Max.z && Max.z >= other.Min.z);
  }
};

} // namespace Core
} // namespace Yamen
