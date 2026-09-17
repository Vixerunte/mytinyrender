#pragma once
#include <iostream>
#include <cmath>

struct Point
{
    float x;
    float y;
    float z;

    void print(){
        std::cout << x << ' ' << y << ' ' << z << std::endl;
    }
};

struct Vec3 {
    float x;
    float y;
    float z;

    Vec3 operator+ (const Vec3& p) const{
        return{
            x + p.x,
            y + p.y,
            z + p.z
        };
    }

    Vec3 operator- (const Vec3& p) const{
        return{
            x - p.x,
            y - p.y,
            z - p.z
        };
    }

    Vec3 operator* (const float p) const{
        return{
            x * p,
            y * p,
            z * p
        };
    }

    float dot (const Vec3& p) const{
        return{
            x * p.x +
            y * p.y +
            z * p.z
        };
    }

    Vec3 cross (const Vec3& p) const{
        return{
            y * p.z - z * p.y,
            z * p.x - x * p.z,
            x * p.y - y * p.x
        };
    }

    float length() const {
        return std::sqrt(
            x*x + y*y + z*z
        );
    }


    Vec3 normalize() const {
        float len = length();
        if (len == 0.0f) {
            return {0, 0, 0};
        }
        return{
            x / len,
            y / len,
            z / len
        };
    }    

};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;

    void print() const {
        std::cout << x << ' '
                << y << ' '
                << z << ' '
                << w << std::endl;
    }
};

struct Mat4 {
    float m[4][4];

    static Mat4 identity() {
        Mat4 result = {};
        result.m[0][0] = 1.0f;
        result.m[1][1] = 1.0f;
        result.m[2][2] = 1.0f;
        result.m[3][3] = 1.0f;
        return result;
    }

    static Mat4 translation(float tx, float ty, float tz) {
        Mat4 result = Mat4::identity();

        result.m[0][3] = tx;
        result.m[1][3] = ty;
        result.m[2][3] = tz;

        return result;
    }

    static Mat4 scale(float sx, float sy, float sz) {
        Mat4 result = {};

        result.m[0][0] = sx;
        result.m[1][1] = sy;
        result.m[2][2] = sz;
        result.m[3][3] = 1.0f;

        return result;
    }

    static Mat4 rotationZ(float angle) {
        Mat4 result = Mat4::identity();

        float c = std::cos(angle);
        float s = std::sin(angle);

        result.m[0][0] = c;
        result.m[0][1] = -s;

        result.m[1][0] = s;
        result.m[1][1] = c;

        return result;
    }

    Vec4 operator* (const Vec4& p) const{
        return{
            m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3] * p.w,
            m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3] * p.w,
            m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3] * p.w,
            m[3][0] * p.x + m[3][1] * p.y + m[3][2] * p.z + m[3][3] * p.w
        };
    }


    Mat4 operator*(const Mat4& other) const {
        Mat4 result = {};

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }

        return result;
    }

    ///////////View Matrix ////////////“从相机看来物体在哪里”
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 forward = (target - eye).normalize();
        Vec3 right = forward.cross(up).normalize();
        Vec3 cameraUp = right.cross(forward);

        Mat4 result = Mat4::identity();

        result.m[0][0] = right.x;
        result.m[0][1] = right.y;
        result.m[0][2] = right.z;
        result.m[0][3] = -right.dot(eye);

        result.m[1][0] = cameraUp.x;
        result.m[1][1] = cameraUp.y;
        result.m[1][2] = cameraUp.z;
        result.m[1][3] = -cameraUp.dot(eye);

        result.m[2][0] = -forward.x;
        result.m[2][1] = -forward.y;
        result.m[2][2] = -forward.z;
        result.m[2][3] = forward.dot(eye);

        return result;
    }

    static Mat4 perspective(float fov, float aspect, float near, float far) {
        Mat4 result = {};

        float f = 1.0f / std::tan(fov / 2.0f);

        result.m[0][0] = f / aspect;
        result.m[1][1] = f;

        result.m[2][2] = (far + near) / (near - far);
        result.m[2][3] = (2.0f * far * near) / (near - far);

        result.m[3][2] = -1.0f;

        return result;
    }

    static Point viewport(const Vec4& ndc, int width, int height) {
        return {
            (ndc.x + 1.0f) * 0.5f * (width - 1),
            (ndc.y + 1.0f) * 0.5f * (height - 1),
            ndc.z
        };
    }

    static Point transformPoint(const Vec3& position,const Mat4& mvp, int width, int height) {
        Vec4 p = {
            position.x,
            position.y,
            position.z,
            1.0f
        };

        Vec4 clip = mvp * p;

        Vec4 ndc = {
            clip.x / clip.w,
            clip.y / clip.w,
            clip.z / clip.w,
            1.0f
        };

        return viewport(ndc,width,height);
    }
};