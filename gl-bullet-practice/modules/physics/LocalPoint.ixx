module;

#include <glm/glm.hpp>

export module LocalPoint;

export import GLMAdapter;
export import RigidBody;

using namespace ownfos::utility;

export namespace ownfos::bullet
{
    struct LocalPoint
    {
        RigidBody* object;
        glm::vec3 local_offset;

        glm::vec3 get_origin() const
        {
            return bt2glm(object->get_world_transform().getOrigin());
        }

        glm::vec3 get_local_point() const
        {
            return local_offset;
        }

        glm::vec3 get_world_point() const
        {
            return apply_transform(object->get_world_transform_matrix(), local_offset);
        }

        glm::vec3 get_world_offset() const
        {
            return get_world_point() - get_origin();
        }
    };
} // namespace ownfos::bullet
