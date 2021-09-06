module;

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/LinearMath/btQuaternion.h>
#include <glm/glm.hpp>

export module Transform;

export namespace ownfos::bullet
{
    // Apply linear transformation to the given vector and return the result.
    glm::vec3 apply_transform(const glm::mat4& transform, const glm::vec3& vector)
    {
        auto homogeneous = transform * glm::vec4(vector, 1.0f);
        auto cartesian = homogeneous / homogeneous.w;

        return cartesian;
    }

    struct Transform
    {
        btVector3 position = { 0, 0, 0 };
        btQuaternion rotation = { 0, 0, 0, 1 };
        btVector3 scale = { 1, 1, 1 };

        void look_at(const btVector3& target)
        {
            auto forward = btVector3{ 0, 0, -1 };
            auto look_direction = (target - position).normalized();

            auto rotation_axis = forward.cross(look_direction);
            auto rotation_angle = glm::acos(forward.dot(look_direction));

            rotation = btQuaternion{ rotation_axis, rotation_angle };
        }

        btVector3 forward() const
        {
            return quatRotate(rotation, { 0, 0, -1 });
        }

        btVector3 up() const
        {
            return quatRotate(rotation, { 0, 1, 0 });
        }

        btVector3 right() const
        {
            return quatRotate(rotation, { 1, 0, 0 });
        }
    };
} // namespace ownfos::bullet
