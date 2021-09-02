module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module Camera;

export import Transform;
export import GLMAdapter;

using namespace ownfos::bullet;

export namespace ownfos::opengl
{
    struct View
    {
        glm::mat4 matrix;

        static View look_target(glm::vec3 cam_pos, glm::vec3 target_pos, glm::vec3 cam_up = { 0, 1, 0 })
        {
            return { glm::lookAt(cam_pos, target_pos, cam_up) };
        }

        static View look_direction(glm::vec3 cam_pos, glm::vec3 cam_forward, glm::vec3 cam_up = { 0, 1, 0 })
        {
            return { glm::lookAt(cam_pos, cam_pos + cam_forward, cam_up) };
        }
    };

    struct Projection
    {
        glm::mat4 matrix;

        static Projection perspective(float fovy, float aspect_ratio, float near, float far)
        {
            return { glm::perspective(fovy, aspect_ratio, near, far) };
        }
    };

    struct Camera
    {
        Transform transform;
        Projection projection;

        glm::mat4 get_matrix() const
        {
            auto pos = to_glm(transform.position);
            auto forward = to_glm(transform.forward());
            auto up = to_glm(transform.up());
            auto view = glm::lookAt(pos, pos + forward, up);

            return projection.matrix * view;
        }
    };
} // namespace ownfos::opengl
