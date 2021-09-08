module;

#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

export module GLMAdapter;

export namespace ownfos::utility
{
    glm::vec3 bt2glm(const btVector3& vector)
    {
        return { vector.x(), vector.y(), vector.z() };
    }

    btVector3 glm2bt(const glm::vec3& vector)
    {
        return { vector.x, vector.y, vector.z };
    }
} // namespace ownfos::utility
