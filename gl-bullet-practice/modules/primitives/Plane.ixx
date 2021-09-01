module;

#include <gl/glew.h>
#include <vector>

export module Plane;

export import PrimitiveModelData;

std::vector<float> create_vertex_data()
{
    return {
        -1, -1, 0, // left-bottom
         1, -1, 0, // right-bottom
        -1,  1, 0, // left-top
         1,  1, 0  // right-top
    };
}

std::vector<float> create_normal_data()
{
    return {
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    };
}

std::vector<unsigned int> create_index_data()
{
    return {
        0, 1, 2,
        1, 3, 2
    };
}

export namespace ownfos::primitives
{
    // Make a quad with +Z axis as a normal vector
    PrimitiveModelData create_plane_model_data()
    {
        return PrimitiveModelData{
            .position = {create_vertex_data(), {{.location = 0, .count = 3}}},
            .normal = {create_normal_data(), {{.location = 1, .count = 3}}},
            .indices = {create_index_data()}
        };
    }
} // namespace ownfos::primitives::plane
