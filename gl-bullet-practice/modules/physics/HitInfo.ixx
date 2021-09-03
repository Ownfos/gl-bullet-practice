module;

#include <bullet/btBulletDynamicsCommon.h>

export module HitInfo;

export import RigidBody;

export namespace ownfos::bullet
{
    struct HitInfo
    {
        RigidBody* object;
        btVector3 position;
        btVector3 normal;
    };
} // namespace ownfos::bullet
