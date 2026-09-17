#include <cmath>
#include "tgaimage.h"
#include <utility>
#include "geometry.h"
#include <algorithm>
#include <iostream>

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

constexpr int width = 1024;
constexpr int height = 1024;


void line(int x0, int y0, int x1, int y1, TGAImage &framebuffer, TGAColor color)
{

    bool steep = std::abs( y1 - y0) > std::abs(x1 - x0);
    if(steep){
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if(x0 > x1){
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int t = x0; t <= x1; t++)
    {
        //a dot, return;
        if(x0 == x1) {
            framebuffer.set(x0, y0, color);
            return;
        }
        float k = static_cast<float>(t - x0) / (x1 - x0);
        int y = y0 + k * (y1 - y0);
        if(steep){
            framebuffer.set(y, t, color);
        }
        else{
            framebuffer.set(t, y, color);
        }
        
    }
}

bool insidetriangle(Point a, Point b, Point c, Point p, TGAColor& color, float& z){
    if((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x) == 0 ){
        return false;
    }
    // u = beta (B - A)(greem)
    // v = gamma (C - A)(blue)
    //alpha = 1 - u - v (A)(red)
    float u = static_cast<float>((p.x - a.x)*(c.y - a.y) - (p.y - a.y)*(c.x - a.x))  /  static_cast<float>((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x));
    float v = static_cast<float>((p.x - a.x)*(b.y - a.y) - (p.y - a.y)*(b.x - a.x))  /  static_cast<float>((c.x - a.x)*(b.y - a.y) - (c.y - a.y)*(b.x - a.x));
    if((u >= 0) && (v >= 0) && ((u + v) <= 1)){
        float alpha = 1 - u - v;
        color = alpha * red + u * green + v * blue;
        z = alpha * a.z + u * b.z + v * c.z;
        return true;
    }
    else{
        return false;
    }
}

void triangle(Point a, Point b, Point c, TGAImage& framebuffer, TGAColor color, std::vector<float>& zbuffer){
    int minx = static_cast<int>(
        std::floor(std::min({a.x, b.x, c.x}))
    );

    int maxx = static_cast<int>(
        std::ceil(std::max({a.x, b.x, c.x}))
    );

    int miny = static_cast<int>(
        std::floor(std::min({a.y, b.y, c.y}))
    );

    int maxy = static_cast<int>(
        std::ceil(std::max({a.y, b.y, c.y}))
    );

    minx = std::max(minx, 0);
    maxx = std::min(maxx, width - 1);

    miny = std::max(miny, 0);
    maxy = std::min(maxy, height - 1);

    Point p;

    for(int x = minx; x <= maxx; x++){
        for(int y = miny; y <= maxy; y++){
            p.x = x + 0.5f;
            p.y = y + 0.5f;
            float z;
            if(insidetriangle(a, b, c, p,color, z)){
                if( z < zbuffer[x + y * width]){
                    zbuffer[x + y * width] = z;
                    framebuffer.set(x, y, color);
                }
                
            }
        }
    }
}

int main(int argc, char **argv)
{

    TGAImage framebuffer(width, height, TGAImage::RGB);

    std::vector<float> zbuffer(width * height);

    std::fill(
        zbuffer.begin(),
        zbuffer.end(),
        std::numeric_limits<float>::infinity()
    );


    // framebuffer.set(ax, ay, blue);
    // framebuffer.set(bx, by, white);
    // framebuffer.set(cx, cy, yellow);

    // line(4, 5, 899, 1000 , framebuffer, red);

    // line(100, 100, 900, 100, framebuffer, red);     // 水平

    // line(500, 100, 500, 900, framebuffer, green);   // 垂直

    // line(100, 100, 900, 600, framebuffer, blue);    // 普通斜线

    // line(100, 100, 300, 900, framebuffer, yellow);  // 陡线

    // line(900, 100, 100, 800, framebuffer, white);   // 反方向+负斜率

    // line(600, 600, 600, 600, framebuffer, red);     // 水平
    

    // Point a = {100, 100, 0.2f};
    // Point b = {800, 200, 0.8f};
    // Point c = {400, 800, 0.5f};
    // line(a.x, a.y, b.x, b.y, framebuffer, red); 
    // line(b.x, b.y, c.x, c.y, framebuffer, red); 
    // line(c.x, c.y, a.x, a.y, framebuffer, red); 
    // triangle(a, b, c, framebuffer, green, zbuffer);

    // Point d = {500, 100, 0.3f};
    // Point e = {200, 600, 0.8f};
    // Point f = {600, 800, 0.5f};
    // line(d.x, d.y, e.x, e.y, framebuffer, red); 
    // line(e.x, e.y, f.x, f.y, framebuffer, red); 
    // line(f.x, f.y, d.x, d.y, framebuffer, red); 
    // triangle(d, e, f, framebuffer, red, zbuffer);

    // framebuffer.write_tga_file("framebuffer.tga");

    // ///////////////////////////////////////test for S R T
    // Vec4 p = {1, 0, 0, 1};

    // Mat4 S = Mat4::scale(2, 2, 2);
    // Mat4 R = Mat4::rotationZ(3.1415926f / 2.0f);
    // Mat4 T = Mat4::translation(10, 20, 30);

    // Mat4 M = T * R * S;

    // Vec4 result = M * p;
    // result.print();

    
    
    Vec3 A = {-1.0f, -1.0f, 0.0f};
    Vec3 B = { 1.0f, -1.0f, 0.0f};
    Vec3 C = { 0.0f,  1.0f, 0.0f};
////////////Model//////////
    Mat4 S = Mat4::scale(0.8f, 0.8f, 0.8f);
    Mat4 R = Mat4::rotationZ(3.1415926f / 4.0f);
    Mat4 T = Mat4::translation(0.5f, 0.0f, 0.0f);

    Mat4 M = T * R * S;

///////////////View Matrix////////////////////
    Vec3 eye    = {0, 0, 3};
    Vec3 target = {0, 0, 0};
    Vec3 up     = {0, 1, 0};

    Mat4 V = Mat4::lookAt(eye, target, up);

//////////Projection///////////////////////
    float fov = 90.0f * 3.1415926f / 180.0f;

    Mat4 P = Mat4::perspective(
        fov,
        1.0f,
        0.1f,
        100.0f
    );

/////////////////////MVP////////

    Mat4 MVP = P * V * M;

    Point ScrA = Mat4::transformPoint(A, MVP, width, height);
    Point ScrB = Mat4::transformPoint(B, MVP, width, height);
    Point ScrC = Mat4::transformPoint(C, MVP, width, height);

    triangle(ScrA, ScrB, ScrC, framebuffer, red, zbuffer);
    framebuffer.write_tga_file("framebuffer.tga");

    return 0;
}
