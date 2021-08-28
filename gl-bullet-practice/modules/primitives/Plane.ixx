module;

#include <gl/glew.h>
#include <vector>

export module Plane;

export import PrimitiveModelData;

std::vector<float> create_vertex_data()
{
    return {
        0, 0, 0, // left-bottom
        1, 0, 0, // right-bottom
        0, 1, 0, // left-top
        1, 1, 0  // right-top
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
    PrimitiveModelData create_plane_model_data()
    {
        return PrimitiveModelData{
            .position = {create_vertex_data(), {{.location = 0, .count = 3}}},
            .indices = {create_index_data()}
        };
    }
} // namespace ownfos::primitives::plane
