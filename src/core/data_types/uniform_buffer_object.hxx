#pragma once
#include "math/mat4.hxx"

struct UniformBufferObject 
{
    UniformBufferObject() 
    : _model(mat4::makeIdentity())
    , _view(mat4::makeIdentity())
    , _projection(mat4::makeIdentity())
    {
    }

    mat4 _model;
    mat4 _view;
    mat4 _projection;
};