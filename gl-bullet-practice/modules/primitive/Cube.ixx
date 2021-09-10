module;

#include <gl/glew.h>
#include <vector>

export module Cube;

export import PrimitiveModelData;

std::vector<float> create_vertex_data()
{
    return {
        // back
        -1, -1, -1,
         1, -1, -1,
        -1,  1, -1,
         1,  1, -1,

        // front
        -1, -1,  1,
         1, -1,  1,
        -1,  1,  1,
         1,  1,  1,

        // left
        -1, -1, -1,
        -1,  1, -1,
        -1, -1,  1,
        -1,  1,  1,

        // right
         1, -1, -1,
         1,  1, -1,
         1, -1,  1,
         1,  1,  1,

        // down
        -1, -1, -1,
         1, -1, -1,
        -1, -1,  1,
         1, -1,  1,

        // up
        -1,  1, -1,
         1,  1, -1,
        -1,  1,  1,
         1,  1,  1,
    };
}

std::vector<float> create_normal_data()
{
    return {
        // back
         0,  0, -1,
         0,  0, -1,
         0,  0, -1,
         0,  0, -1,

        // front
         0,  0,  1,
         0,  0,  1,
         0,  0,  1,
         0,  0,  1,

        // left
        -1,  0,  0,
        -1,  0,  0,
        -1,  0,  0,
        -1,  0,  0,

        // right
         1,  0,  0,
         1,  0,  0,
         1,  0,  0,
         1,  0,  0,

        // down
         0, -1,  0,
         0, -1,  0,
         0, -1,  0,
         0, -1,  0,

        // up
         0,  1,  0,
         0,  1,  0,
         0,  1,  0,
         0,  1,  0,
    };
}

std::vector<unsigned int> create_index_data()
{
    return {
        // back
        0, 1, 2,
        1, 3, 2,
        
        // front
        4, 5, 6,
        5, 7, 6,

        // left
        8, 9, 10,
        9, 11, 10,

        // right
        12, 13, 14,
        13, 15, 14,

        // down
        16, 17, 18,
        17, 19, 18,

        // up
        20, 21, 22,
        21, 23, 22,
    };
}

export namespace ownfos::primitive
{
    // Make a cube where each edge ranges from -1 to +1
    PrimitiveModelData create_cube_model_data()
    {
        return PrimitiveModelData{
            .position = {create_vertex_data(), {{.location = 0, .count = 3}}},
            .normal = {create_normal_data(), {{.location = 1, .count = 3}}},
            .indices = {create_index_data()}
        };
    }
} // namespace ownfos::primitive
