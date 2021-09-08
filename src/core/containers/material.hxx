#pragma once
#include <cstdint>

struct image;

struct material
{
    bool _doubleSided;

    uint32_t _baseColorTexture{UINT32_MAX};
};