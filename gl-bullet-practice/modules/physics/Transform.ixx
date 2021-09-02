module;

#include <bullet/btBulletDynamicsCommon.h>

export module Transform;

export namespace ownfos::bullet
{
    struct Transform
    {
        btVector3 position = { 0, 0, 0 };
        btQuaternion rotation = { 0, 0, 0, 1 };
        btVector3 scale = { 1, 1, 1 };
    };
} // namespace ownfos::bullet
