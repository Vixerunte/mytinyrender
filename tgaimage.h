#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push,1)
struct TGAHeader {
    std::uint8_t  idlength = 0;
    std::uint8_t  colormaptype = 0;
    std::uint8_t  datatypecode = 0;
    std::uint16_t colormaporigin = 0;
    std::uint16_t colormaplength = 0;
    std::uint8_t  colormapdepth = 0;
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint8_t  bitsperpixel = 0;
    std::uint8_t  imagedescriptor = 0;
};
#pragma pack(pop)

struct TGAColor {
    std::uint8_t bgra[4] = {0,0,0,0};
    std::uint8_t bytespp = 4;
    std::uint8_t& operator[](const int i) { return bgra[i]; }
    const std::uint8_t& operator[](const int i) const { return bgra[i]; }
    TGAColor operator* (float k) const {
        TGAColor p;
        p.bgra[0] = static_cast<std::uint8_t>(bgra[0] * k);;
        p.bgra[1] = static_cast<std::uint8_t>(bgra[1] * k);;
        p.bgra[2] = static_cast<std::uint8_t>(bgra[2] * k);;
        p.bgra[3] = static_cast<std::uint8_t>(bgra[3] * k);;

        return p;
    }

    TGAColor operator+ (const TGAColor& color) const {
        return{
            static_cast<std::uint8_t>(this->bgra[0] + color.bgra[0]),
            static_cast<std::uint8_t>(this->bgra[1] + color.bgra[1]),
            static_cast<std::uint8_t>(this->bgra[2] + color.bgra[2]),
            static_cast<std::uint8_t>(this->bgra[3] + color.bgra[3])
        };
    }
};

inline TGAColor operator* (float k, const TGAColor& color) {
        return color * k;
    }

struct TGAImage {
    enum Format { GRAYSCALE=1, RGB=3, RGBA=4 };
    TGAImage() = default;
    TGAImage(const int w, const int h, const int bpp);
    bool  read_tga_file(const std::string filename);
    bool write_tga_file(const std::string filename, const bool vflip=true, const bool rle=true) const;
    void flip_horizontally();
    void flip_vertically();
    TGAColor get(const int x, const int y) const;
    void set(const int x, const int y, const TGAColor &c);
    int width()  const;
    int height() const;
private:
    bool   load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out) const;
    int w = 0, h = 0;
    std::uint8_t bpp = 0;
    std::vector<std::uint8_t> data = {};
};

