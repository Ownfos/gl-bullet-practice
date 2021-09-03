module;

#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

export module Ray;

export import Camera;
export import GLMAdapter;

using namespace ownfos::opengl;

constexpr float default_long_distance = 10000.0f;

export namespace ownfos::bullet
{
    struct Ray
    {
        btVector3 start_point;
        btVector3 end_point;

        // Returns the point between start and end of this ray,
        // where ratio of 0 means start point and 1 means end point.
        btVector3 get_lerped_point(float ratio) const
        {
            return start_point.lerp(end_point, ratio);
        }

        // Returns the normalized direction vector from start to end point
        btVector3 get_direction() const
        {
            return (end_point - start_point).normalized();
        }

        static Ray from_direction(const btVector3& start_point, const btVector3& direction, float distance = default_long_distance)
        {
            return {
                .start_point = start_point,
                .end_point = start_point + direction.normalized() * distance
            };
        }

        static Ray from_endpoints(const btVector3& start_point, const btVector3& end_point)
        {
            return {
                .start_point = start_point,
                .end_point = end_point
            };
        }

        static Ray from_screenpoint(const Camera& camera, const glm::vec2& normalized_screen_point, float distance = default_long_distance)
        {
            auto world_pos = glm2bt(camera.screen_to_world_point(normalized_screen_point));
            auto cam_origin = camera.transform.position;

            auto ray_direction = world_pos - cam_origin;

            return Ray::from_direction(world_pos, ray_direction, distance);
        }
    };
} // namespace ownfos::bullet
