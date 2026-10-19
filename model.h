#include<iostream>
#include<cmath>
#include<vector>
#include<array>

#include "geometry.h"

#include "tgaimage.h"

struct FaceVertex{
    int vertexIndex;
    int texcoorIndex;
    int normalIndex;
};

struct Face
{
    std::array<FaceVertex,3> vertices;
};


class Model
{
private:
    std::vector<Vec3> positions;
    std::vector<Vec2> uvs;
    std::vector<Vec3> normalData;
    std::vector<Face> faceData;
    TGAImage normalMap;
    TGAImage diffuseMap;
public:
    const std::vector<Vec3>& vertices() const{
        return positions;
    }

    const std::vector<Vec2>& texcoords() const {
        return uvs;
    }

    const std::vector<Vec3>& normals() const {
        return normalData;
    }

    const std::vector<Face>& faces() const {
        return faceData;
    }

    bool load(const std::string& filename);

    const TGAImage& normalmap() const{
        return normalMap;
    }

    const TGAImage& diffusemap() const{
        return diffuseMap;
    }
};

struct RasterVertex {
    Point screen;  // 投影后的屏幕坐标
    Vec3 position; // 模型空间位置，用来算切线
    Vec2 uv;
    Vec3 normal;   // OBJ 的 vn
    float invW; // 1/clip.w
};


