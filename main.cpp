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

bool insidetriangle(Point a, Point b, Point c, Point p, TGAColor& color){
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
        // std::cout << alpha << "alpha " << u << "u " << v << " v" 
        //     << int(color[0]) << 'blue'
        //     << int(color[1]) << "green"
        //     << int(color[2]) << "red" << std::endl;
        return true;
    }
    else{
        return false;
    }
}

void triangle(Point a, Point b, Point c, TGAImage& framebuffer, TGAColor color){
    int minx = std::min({a.x, b.x, c.x});
    int maxx = std::max({a.x, b.x, c.x});
    int miny = std::min({a.y, b.y, c.y});
    int maxy = std::max({a.y, b.y, c.y});

    Point p;

    for(int x = minx; x <= maxx; x++){
        for(int y = miny; y <= maxy; y++){
            p.x = x;
            p.y = y;
            if(insidetriangle(a, b, c, p,color)){
                framebuffer.set(p.x, p.y, color);
            }
        }
    }
}

int main(int argc, char **argv)
{
    constexpr int width = 1024;
    constexpr int height = 1024;
    TGAImage framebuffer(width, height, TGAImage::RGB);


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
    

    Point a = {100, 100};
    Point b = {800, 200};
    Point c = {400, 800};
    line(a.x, a.y, b.x, b.y, framebuffer, red); 
    line(b.x, b.y, c.x, c.y, framebuffer, red); 
    line(c.x, c.y, a.x, a.y, framebuffer, red); 
    triangle(a, b, c, framebuffer, green);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
