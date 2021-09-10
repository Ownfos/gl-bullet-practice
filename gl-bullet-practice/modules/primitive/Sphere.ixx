module;

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

export module Sphere;

export import PrimitiveModelData;

glm::vec3 point_in_sphere(float latitude, float longitude)
{
    auto point = glm::vec4(0, 0, -1, 1);
    
    // First, find a point in zy-plane that have appropriate y value.
    // Latitude of -90 degrees give -y axis, while +90 degrees give +y axis
    point = glm::rotate(glm::mat4(1.0f), latitude, { 1, 0, 0 }) * point;

    // Second, rotate the point along y axis so that it has appropriate x and z value
    point = glm::rotate(glm::mat4(1.0f), longitude, { 0, 1, 0 }) * point;

    // Convert homogeneous coordinate to cartesian coordinate
    return point / point.w;
}

void push_vec3(std::vector<float>& storage, const glm::vec3& vec)
{
    storage.push_back(vec.x);
    storage.push_back(vec.y);
    storage.push_back(vec.z);
}

struct SphereModel
{
    std::vector<float> position;
    std::vector<float> normal;
    std::vector<unsigned int> index;

    SphereModel(int num_vertices_per_half_circle, bool is_surface_smooth)
    {
        const float angle_delta = glm::radians(180.0f) / num_vertices_per_half_circle;

        // Half circle from 0 to 180 degrees which decide the y value
        for (int latitude_index = 0; latitude_index < num_vertices_per_half_circle; ++latitude_index)
        {
            auto current_latitude = angle_delta * latitude_index - glm::radians(90.0f);
            auto next_latitude = angle_delta * (latitude_index + 1) - glm::radians(90.0f);

            // Full circle on xz-plane
            for (int longitude_index = 0; longitude_index < num_vertices_per_half_circle * 2; ++longitude_index)
            {
                auto current_longitude = angle_delta * longitude_index;
                auto next_longitude = angle_delta * (longitude_index + 1);

                // Find four points on this patch of surface, which then becomes
                // a round wall of triangles around y-axis 
                auto p1 = point_in_sphere(current_latitude, current_longitude);
                auto p2 = point_in_sphere(current_latitude, next_longitude);
                auto p3 = point_in_sphere(next_latitude, current_longitude);
                auto p4 = point_in_sphere(next_latitude, next_longitude);

                // Create two faces in counter clockwise direction
                add_face(p1, p2, p4, is_surface_smooth);
                add_face(p4, p3, p1, is_surface_smooth);
            }
        }
    }

    void add_face(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, bool is_surface_smooth)
    {
        auto current_index = index.size();

        push_vec3(position, p1);
        push_vec3(position, p2);
        push_vec3(position, p3);

        // Normals for smooth shading
        if (is_surface_smooth)
        {
            push_vec3(normal, p1);
            push_vec3(normal, p2);
            push_vec3(normal, p3);
        }
        // Normals for flat shading (same normal for all vertices in this face)
        else
        {
            auto face_normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
            push_vec3(normal, face_normal);
            push_vec3(normal, face_normal);
            push_vec3(normal, face_normal);
        }

        index.push_back(current_index);
        index.push_back(current_index + 1);
        index.push_back(current_index + 2);
    }
};

export namespace ownfos::primitive
{
    PrimitiveModelData create_sphere_model_data(int num_vertices_per_half_circle, bool is_surface_smooth)
    {
        auto sphere = SphereModel(num_vertices_per_half_circle, is_surface_smooth);
        return PrimitiveModelData{
            .position = {sphere.position, {{.location = 0, .count = 3}}},
            .normal = {sphere.normal, {{.location = 1, .count = 3}}},
            .indices = {sphere.index}
        };
    }
}