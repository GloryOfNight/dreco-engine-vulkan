#pragma once
#include <stdint.h>

struct mat3x4 
{
    mat3x4();
    
private:
    int32_t _mat[3][4];
};

mat3x4 operator*(const mat3x4& first, const mat3x4& second);