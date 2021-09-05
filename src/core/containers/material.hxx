#pragma once
#include <cstdint>

struct image;

struct material
{
    bool _doubleSided;

    image* _baseColorTexture{nullptr};
};