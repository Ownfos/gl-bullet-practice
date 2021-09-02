module;

#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

export module GLMAdapter;

export namespace ownfos::bullet
{
    glm::vec3 to_glm(btVector3 vector)
    {
        return { vector.x(), vector.y(), vector.z() };
    }
} // namespace ownfos::bullet
