#include <cmath>
#include "tgaimage.h"
#include <utility>
#include "geometry.h"
#include <algorithm>
#include <iostream>

#include "model.h"

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

constexpr int width = 1024;
constexpr int height = 1024;

constexpr int shadowWidth = 1024;
constexpr int shadowHeight = 1024;


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

bool insidetriangle(Point a, Point b, Point c, Point p, 
    float& z,
    Vec3& bary)
    {
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
            bary = {alpha, u, v};
            z = alpha * a.z + u * b.z + v * c.z;
            return true;
        }
        else{
            return false;
        }
    }

// 输入的三个顶点必须已经用 lightMVP 投影到 Shadow Map 的像素坐标。
// 这里只做覆盖测试和最近深度记录，不计算颜色。
void shadowTriangle(
    const Point& a,
    const Point& b,
    const Point& c,
    std::vector<float>& shadowMap)
{
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
    maxx = std::min(maxx, shadowWidth - 1);
    miny = std::max(miny, 0);
    maxy = std::min(maxy, shadowHeight - 1);

    if (minx > maxx || miny > maxy) {
        return;
    }

    Point p{};
    for (int x = minx; x <= maxx; ++x) {
        for (int y = miny; y <= maxy; ++y) {
            p.x = x + 0.5f;
            p.y = y + 0.5f;

            float depth;
            Vec3 bary;
            if (!insidetriangle(a, b, c, p, depth, bary)) {
                continue;
            }

            // 正交投影下 lightNdc.z 在 [-1, 1]；范围外的点不属于光源视锥。
            if (depth < -1.0f || depth > 1.0f) {
                continue;
            }

            const int index = x + y * shadowWidth;
            if (depth < shadowMap[index]) {
                shadowMap[index] = depth;
            }
        }
    }
}

TGAColor shadeFragment(
    const Vec2& uv,
    const Vec3& normal,
    const Vec3& tangent,
    float handedness,
    const Vec3& lightDir,
    const Vec3& viewDir,
    const TGAImage& normalMap,
    const TGAImage& diffuseMap,
    float visibility)
{
    const int normalX = std::clamp(
        static_cast<int>(uv.x * normalMap.width()),
        0, normalMap.width() - 1
    );
    const int normalY = std::clamp(
        static_cast<int>(uv.y * normalMap.height()),
        0, normalMap.height() - 1
    );

    const TGAColor normalTexel = normalMap.get(normalX, normalY);
    Vec3 tangentNormal = {
        normalTexel[2] / 127.5f - 1.0f, // R -> X
        normalTexel[1] / 127.5f - 1.0f, // G -> Y
        normalTexel[0] / 127.5f - 1.0f  // B -> Z
    };
    tangentNormal = tangentNormal.normalize();

    const Vec3 T = (tangent - normal * normal.dot(tangent)).normalize();
    const Vec3 B = normal.cross(T) * handedness;
    const Vec3 mappedNormal = (
        T * tangentNormal.x +
        B * tangentNormal.y +
        normal * tangentNormal.z
    ).normalize();



    const int diffuseX = std::clamp(
        static_cast<int>(uv.x * diffuseMap.width()),
        0, diffuseMap.width() - 1
    );
    const int diffuseY = std::clamp(
        static_cast<int>(uv.y * diffuseMap.height()),
        0, diffuseMap.height() - 1
    );
    const TGAColor albedo = diffuseMap.get(diffuseX, diffuseY);

    const Vec3 H = (viewDir + lightDir).normalize();

    const float diffuse = std::max(0.0f, mappedNormal.dot(lightDir));
    const float specular = std::pow(std::max(0.0f, mappedNormal.dot(H)), 32.0f); //shinness = 32.0f
    // 加白色镜面高光，并限制通道范围，避免 uint8_t 溢出
    //处于阴影中的像素会失去这盏光带来的漫反射和高光，但仍保留 ambient，所以不会完全漆黑
    const float specularStrength = 0.25f * specular * visibility;
    
    const float ambient = 0.15f;
    TGAColor color = albedo * (ambient + 0.85f * diffuse * visibility); //intensity = 0.15f + 0.85f * diffuse;
    
    for (int i = 0; i < 3; ++i) {
        color[i] = static_cast<uint8_t>(std::clamp(
            static_cast<float>(color[i]) + 255.0f * specularStrength,
            0.0f,
            255.0f
        ));
    }
    
    return color;
}

void triangle(
    const RasterVertex& a,
    const RasterVertex& b,
    const RasterVertex& c, 
    TGAImage& framebuffer,
    std::vector<float>& zbuffer,
    std::vector<float>& shadowMap,
    const TGAImage& normalMap,
    const TGAImage& diffuseMap,
    const Vec3& cameraPosition,
    Vec3 lightDir,
    const Mat4& lightMVP){
    int minx = static_cast<int>(
        std::floor(std::min({a.screen.x, b.screen.x, c.screen.x}))
    );

    int maxx = static_cast<int>(
        std::ceil(std::max({a.screen.x, b.screen.x, c.screen.x}))
    );

    int miny = static_cast<int>(
        std::floor(std::min({a.screen.y, b.screen.y, c.screen.y}))
    );

    int maxy = static_cast<int>(
        std::ceil(std::max({a.screen.y, b.screen.y, c.screen.y}))
    );

    minx = std::max(minx, 0);
    maxx = std::min(maxx, width - 1);

    miny = std::max(miny, 0);
    maxy = std::min(maxy, height - 1);

    Vec3 edge1 = b.position - a.position;
    Vec3 edge2 = c.position - a.position;
    
    //得到这两条边对应的 UV 变化量
    float du1 = b.uv.x - a.uv.x;
    float dv1 = b.uv.y - a.uv.y;
    float du2 = c.uv.x - a.uv.x;
    float dv2 = c.uv.y - a.uv.y;

    
    //det 表示 UV 三角形的面积相关量。若它接近零，UV 三角形退化，无法可靠计算 T/B
    float det = du1 * dv2 - dv1 * du2;
    if (std::abs(det) < 1e-8f) {
        return; // 或跳过这个三角形
    }

    float invDet = 1.0f / det;

    //切线方向 tangent 和副切线方向 bitangent
    Vec3 tangent =
        (edge1 * dv2 - edge2 * dv1) * invDet;

    Vec3 bitangent =
        (edge2 * du1 - edge1 * du2) * invDet;

    

    Vec3 geometricNormal = edge1.cross(edge2).normalize();
    float handedness =
        geometricNormal.cross(tangent).dot(bitangent) < 0 ? -1.0f : 1.0f;

    



    Point p;

    for(int x = minx; x <= maxx; x++){
        for(int y = miny; y <= maxy; y++){
            p.x = x + 0.5f;
            p.y = y + 0.5f;
            float z;
            Vec3 bary;
            if(insidetriangle(a.screen, b.screen, c.screen, p, z, bary)){
                //三角形每个像素
                if( z < zbuffer[x + y * width]){
                    
                   


                    const float denom =
                        bary.x * a.invW +
                        bary.y * b.invW +
                        bary.z * c.invW;

                    if (std::abs(denom) < 1e-8f) {
                        continue;
                    }

                    //归一化后的 correctedBary
                    const Vec3 correctedBary = {
                        bary.x * a.invW / denom,
                        bary.y * b.invW / denom,
                        bary.z * c.invW / denom
                    };
                    const Vec3 fragmentPosition =
                        a.position * correctedBary.x +
                        b.position * correctedBary.y +
                        c.position * correctedBary.z;

                    const Point lightScreen = Mat4::transformPoint(
                        fragmentPosition,
                        lightMVP,
                        shadowWidth,
                        shadowHeight
                    );
                    float visibility = 1.0f;
                    if (lightScreen.x >= 0.0f &&
                        lightScreen.x < shadowWidth &&
                        lightScreen.y >= 0.0f &&
                        lightScreen.y < shadowHeight &&
                        lightScreen.z >= -1.0f &&
                        lightScreen.z <= 1.0f)
                    {
                        const float sx = static_cast<int> (lightScreen.x);
                        const int sy = static_cast<int>(lightScreen.y);

                        const float closestDepth = shadowMap[sx + sy * shadowWidth];
                        const float bias = 0.001f;
                        if(closestDepth + bias < lightScreen.z){
                            visibility = 0.0f;
                        }

                    }

                    Vec2 uv = {
                        correctedBary.x * a.uv.x + correctedBary.y * b.uv.x + correctedBary.z * c.uv.x,
                        correctedBary.x * a.uv.y + correctedBary.y * b.uv.y + correctedBary.z * c.uv.y
                    };
                    const Vec3 normal = (
                        a.normal * correctedBary.x +
                        b.normal * correctedBary.y +
                        c.normal * correctedBary.z
                    ).normalize();

                    const Vec3 viewDir =
                        (cameraPosition - fragmentPosition).normalize();


                    const TGAColor color = shadeFragment(
                        uv, normal, tangent, handedness, lightDir,viewDir,
                        normalMap, diffuseMap,visibility
                    );
                    framebuffer.set(x, y, color);
                    zbuffer[x + y * width] = z;


                    //framebuffer.set(x, y, normalMap.get(tx, ty));
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

    std::vector<float> shadowMap(shadowWidth * shadowHeight);
    std::fill(
        shadowMap.begin(),
        shadowMap.end(),
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
    // Mat4 S = Mat4::scale(0.8f, 0.8f, 0.8f);
    // Mat4 R = Mat4::rotationZ(3.1415926f / 4.0f);
    // Mat4 T = Mat4::translation(0.5f, 0.0f, 0.0f);

    // Mat4 M = T * R * S;

    Mat4 M = Mat4::identity();

///////////////View Matrix////////////////////
    Vec3 eye    = {0, 1, 3};
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

    const Vec3 lightDir = Vec3{1, 1, 1}.normalize();
    const Vec3 lightTarget = target; // 这里复用场景/相机目标点
    const Vec3 lightEye = lightTarget + lightDir * 3.0f;
    const Vec3 lightUp = up;
    Mat4 lightView = Mat4::lookAt(lightEye, lightTarget, lightUp);
    Mat4 lightProjection =
        Mat4::orthographic(-2, 2, -2, 2, 0.1f, 10.0f);

    Mat4 lightMVP = lightProjection * lightView * M;

//     Point ScrA = Mat4::transformPoint(A, MVP, width, height);
//     Point ScrB = Mat4::transformPoint(B, MVP, width, height);
//     Point ScrC = Mat4::transformPoint(C, MVP, width, height);

//     triangle(ScrA, ScrB, ScrC, framebuffer, red, zbuffer);
//     framebuffer.write_tga_file("framebuffer.tga");

    Model model;

    if (!model.load("obj/african_head/african_head.obj")) {
        return 1;
    }

    for (const Face& face : model.faces()) {
        Point lightScreen[3];

        for (int i = 0; i < 3; ++i) {
            int vertexIndex = face.vertices[i].vertexIndex;

            lightScreen[i] = Mat4::transformPoint(
                model.vertices()[vertexIndex],
                lightMVP,
                shadowWidth,
                shadowHeight
            );
        }

        shadowTriangle(
            lightScreen[0],
            lightScreen[1],
            lightScreen[2],
            shadowMap
        );
    }
        

    // TGAImage shadowPreview(
    //     shadowWidth,
    //     shadowHeight,
    //     TGAImage::RGB
    // );

    // for (int y = 0; y < shadowHeight; ++y) {
    //     for (int x = 0; x < shadowWidth; ++x) {
    //         const float depth = shadowMap[x + y * shadowWidth];

    //         std::uint8_t gray = 255; // infinity：此处没有几何体，显示白色

    //         if (std::isfinite(depth)) {
    //             // 正交投影的 NDC 深度约在 [-1, 1]，映射到 [0, 255]
    //             const float depth01 = std::clamp(
    //                 (depth + 1.0f) * 0.5f,
    //                 0.0f,
    //                 1.0f
    //             );

    //             gray = static_cast<std::uint8_t>(depth01 * 255.0f);
    //         }

    //         // RGB 三通道写相同灰度值，兼容 VS Code 的 TGA 预览插件。
    //         shadowPreview.set(
    //             x, y,
    //             TGAColor{gray, gray, gray, 255, 3}
    //         );
    //     }
    // }

    // shadowPreview.write_tga_file("shadow_map.tga", true, false);
    
    for(const Face& face : model.faces()){
        RasterVertex rasterVertex[3];
        
        

        for (int i = 0; i < 3; ++i) {
            int vertexIndex = face.vertices[i].vertexIndex;
            int texcoordIndex = face.vertices[i].texcoorIndex;
            int normalIndex = face.vertices[i].normalIndex;

            rasterVertex[i].position = model.vertices()[vertexIndex];
            const Vec3 position = rasterVertex[i].position;
            const Vec4 clip = MVP * Vec4{
                position.x,
                position.y,
                position.z,
                1.0f
            };
            rasterVertex[i].invW = 1.0f/clip.w;

            // const Vec4 ndc = {
            //     clip.x / clip.w,
            //     clip.y / clip.w,
            //     clip.z / clip.w,
            //     1.0f
            // };
            // rasterVertex[i].screen = Mat4::viewport(ndc, width, height);


            rasterVertex[i].screen = Mat4::transformPoint(
                model.vertices()[vertexIndex],
                MVP,
                width,
                height
            );

            rasterVertex[i].normal = model.normals()[normalIndex].normalize();
            rasterVertex[i].uv = model.texcoords()[texcoordIndex];

        }

        triangle(
            rasterVertex[0],
            rasterVertex[1],
            rasterVertex[2],
            framebuffer,
            zbuffer,
            shadowMap,
            model.normalmap(),
            model.diffusemap(),
            eye,
            lightDir,
            lightMVP
        );

        
    }
    framebuffer.write_tga_file("framebuffer.tga");



    return 0;
}
