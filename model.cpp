#include"model.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

bool Model::load(const std::string& filename){
    std::ifstream file(filename);
    if(!file.is_open()){
        std::cerr << "can't open this OBJ file: "
                <<filename << std::endl;
        return false;
    }

    std::string normalPath = filename.substr(0, filename.find_last_of('.')) + "_nm_tangent.tga";
    if(!normalMap.read_tga_file(normalPath)){
        std::cerr << "can't load normal map:" << normalPath << std::endl; 
        return false;
    }

    std::string diffusePath = filename.substr(0, filename.find_last_of('.')) + "_diffuse.tga";
    if(!diffuseMap.read_tga_file(diffusePath)){
        std::cerr << "can't load normal map:" << normalPath << std::endl; 
        return false;
    }

    positions.clear();
    uvs.clear();
    normalData.clear();
    faceData.clear();

    std::string line;

    while(std::getline(file, line)){
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream stream(line);
        std::string type;
        stream >> type;

        if (type == "v") {
            Vec3 position{};
            stream >> position.x
                   >> position.y
                   >> position.z;

            positions.push_back(position);
        }
        else if (type == "vt") {
            Vec2 uv{};
            stream >> uv.x
                   >> uv.y;

            uvs.push_back({uv.x, 1.0f - uv.y});
        }
        else if (type == "vn") {
            Vec3 normal{};
            stream >> normal.x
                   >> normal.y
                   >> normal.z;

            normalData.push_back(normal);
        }
        else if(type == "f"){
            Face face{};
            for(int i = 0; i < 3; i++){
                std::string token;
                stream >> token;

                int vertexIndex;
                int texcoordIndex;
                int normalIndex;

                int count = std::sscanf(
                        token.c_str(),
                        "%d/%d/%d",
                        &vertexIndex,
                        &texcoordIndex,
                        &normalIndex
                );

                if (count != 3) {
                        std::cerr << "不支持的面格式: "
                                << token << std::endl;
                        return false;
                }

                face.vertices[i] = {
                    vertexIndex - 1,
                    texcoordIndex - 1,
                    normalIndex - 1
                };

                
            }
            faceData.push_back(face);
        }

        
    }
    std::cout << "load finished: "
            << positions.size() << " vertices, "
            << uvs.size() << "  UV, "
            << normalData.size() << " normal, "
            << faceData.size() << " triangle"
            << std::endl;

    return true;

}