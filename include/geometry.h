#pragma once

#include <array>
#include <cmath>

namespace geo {

// TODO: Implement Matrix operations

struct vec2 {
  float x, y;

  vec2 operator+(const vec2 &other) const {
    return vec2{x + other.x, y + other.y};
  }
  vec2 operator-(const vec2 &other) const {
    return vec2{x - other.x, y - other.y};
  }
  vec2 operator*(const float &scalar) const {
    return vec2{x * scalar, y * scalar};
  }
  vec2 operator*(const vec2 &other) const {
    return vec2{x * other.x + y * other.y};
  }
};

struct vec3 {
  float x, y, z;

  vec3 operator+(const vec3 &other) const {
    return vec3{x + other.x, y + other.y, z + other.z};
  }
  vec3 operator-(const vec3 &other) const {
    return vec3{x - other.x, y - other.y, z - other.z};
  }
  vec3 operator*(const float &scalar) const {
    return vec3{x * scalar, y * scalar, z * scalar};
  }
  float operator*(const vec3 &other) const {
    return x * other.x + y * other.y + z * other.z;
  }
};

struct vec4 {
  float x, y, z, w{1.0f};

  vec4 operator+(const vec4 &other) const {
    return vec4{x + other.x, y + other.y, z + other.z, w + other.w};
  }
  vec4 operator-(const vec4 &other) const {
    return vec4{x - other.x, y - other.y, z - other.z, w - other.w};
  }
  vec4 operator*(const float &scalar) const {
    return vec4{x * scalar, y * scalar, z * scalar, w * scalar};
  }
  vec4 operator*(const vec4 &other) const {
    return vec4{x * other.x + y * other.y + z * other.z + w * other.w};
  }
};

template <typename T, std::size_t Cols, std::size_t Rows> class matrix {
public:
  std::array<std::array<T, Cols>, Rows> data{};
};

struct mat2 {
  float data[2][2];

  mat2 operator*(const mat2 &other) const {
    mat2 result{};
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        result.data[i][j] = 0.0f;
        for (int k = 0; k < 2; k++) {
          result.data[i][j] += data[i][k] * other.data[k][j];
        }
      }
    }
    return result;
  }

  mat2 operator*(const float &scalar) const {
    mat2 result{data[0][0] * scalar, data[0][1] * scalar, data[1][0] * scalar,
                data[1][1] * scalar};
    return result;
  }

  vec2 operator*(const vec2 &other) const {
    vec2 result{data[0][0] * other.x + data[0][1] * other.y,
                data[1][0] * other.x + data[1][1] * other.y};
    return result;
  }
};

struct mat3 {
  // std::array
  float data[3][3];

  mat3 operator*(const mat3 &other) const {
    mat3 result{};
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        result.data[i][j] = 0.0f;
        for (int k = 0; k < 3; k++) {
          result.data[i][j] += data[i][k] * other.data[k][j];
        }
      }
    }
    return result;
  }

  vec3 operator*(const vec3 &other) const {
    vec3 result{
        data[0][0] * other.x + data[0][1] * other.y + data[0][2] * other.z,
        data[1][0] * other.x + data[1][1] * other.y + data[1][2] * other.z,
        data[2][0] * other.x + data[2][1] * other.y + data[2][2] * other.z};
    return result;
  }
};

struct mat4 {

  float data[4][4];
  mat4 operator*(const mat4 &other) const {
    mat4 result{};
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        result.data[i][j] = 0.0f;
        for (int k = 0; k < 4; k++) {
          result.data[i][j] += data[i][k] * other.data[k][j];
        }
      }
    }
    return result;
  }
  vec4 operator*(const vec4 &other) const {
    vec4 result{data[0][0] * other.x + data[0][1] * other.y +
                    data[0][2] * other.z + data[0][3] * other.w,
                data[1][0] * other.x + data[1][1] * other.y +
                    data[1][2] * other.z + data[1][3] * other.w,
                data[2][0] * other.x + data[2][1] * other.y +
                    data[2][2] * other.z + data[2][3] * other.w,
                data[3][0] * other.x + data[3][1] * other.y +
                    data[3][2] * other.z + data[3][3] * other.w};
    return result;
  }
};

// --- Free-function vector utilities ----------------------------------------

inline float dot(const vec3 &a, const vec3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 cross(const vec3 &a, const vec3 &b) {
  return vec3{a.y * b.z - a.z * b.y,
              a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x};
}

inline float length(const vec3 &v) {
  return std::sqrt(dot(v, v));
}

inline vec3 normalize(const vec3 &v) {
  const float len = length(v);
  return vec3{v.x / len, v.y / len, v.z / len};
}

}; // namespace geo
