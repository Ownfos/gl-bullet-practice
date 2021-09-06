module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module Camera;

export import Transform;
export import GLMAdapter;

using namespace ownfos::bullet;

export namespace ownfos::opengl
{
    struct Range
    {
        float start;
        float end;
    };

    class Projection
    {
    public:
        static Projection perspective(float fovy, float aspect_ratio, float near, float far)
        {
            return { glm::perspective(fovy, aspect_ratio, near, far), near };
        }

        static Projection orthographic(Range x_axis, Range y_axis, Range z_axis)
        {
            return { glm::ortho(x_axis.start, x_axis.end, y_axis.start, y_axis.end, z_axis.start, z_axis.end), z_axis.start };
        }

        const glm::mat4& get_matrix() const
        {
            return matrix;
        }

        float get_near_plane_distance() const
        {
            return near_plane_distance;
        }

    private:
        Projection(glm::mat4 matrix, float near_plane_distance)
            : matrix(matrix), near_plane_distance(near_plane_distance)
        {}

        glm::mat4 matrix;
        float near_plane_distance;
    };

    struct Camera
    {
        Transform transform;
        Projection projection;

        glm::mat4 get_matrix() const
        {
            auto pos = bt2glm(transform.position);
            auto forward = bt2glm(transform.forward());
            auto view = glm::lookAt(pos, pos + forward, { 0, 1, 0 });

            return projection.get_matrix() * view;
        }

        glm::vec3 screen_to_world_point(const glm::vec2& normalized_screen_point) const
        {
            auto inverse_transform = glm::inverse(get_matrix());
            auto camera_near_plane_point = glm::vec3(normalized_screen_point, projection.get_near_plane_distance());

            auto world_point = apply_transform(inverse_transform, camera_near_plane_point);

            return world_point;
        }
    };
} // namespace ownfos::opengl
