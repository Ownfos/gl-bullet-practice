module;

#include <gl/glew.h>
#include <vector>

export module Cube;

export import PrimitiveModelData;

std::vector<float> create_vertex_data()
{
    return {
        // lower plane
        0, 0, 0,
        1, 0, 0,
        0, 1, 0,
        1, 1, 0,

        // upper plane
        0, 0, 1,
        1, 0, 1,
        0, 1, 1,
        1, 1, 1
    };
}

std::vector<unsigned int> create_index_data()
{
    return {
        // lower plane
        0, 1, 2,
        1, 3, 2,
        
        // upper plane
        4, 5, 6,
        5, 7, 6
    };
}

namespace ownfos::primitives
{
    export PrimitiveModelData create_cube_model_data()
    {
        return PrimitiveModelData{
            .position = {create_vertex_data(), {{.location = 0, .count = 3}}},
            .indices = {create_index_data()}
        };
    }
} // namespace ownfos::primitives::cube
