module;

#include <bullet/btBulletDynamicsCommon.h>

export module IDynamicsWorldComponents;

export namespace ownfos::bullet
{
    class IDynamicsWorldComponents
    {
    public:
        virtual ~IDynamicsWorldComponents() = default;

        virtual btCollisionConfiguration* get_collision_configuration() const = 0;
        virtual btDispatcher* get_dispatcher() const = 0;
        virtual btBroadphaseInterface* get_broadphase() const = 0;
        virtual btConstraintSolver* get_constraint_solver() const = 0;
    };
} // namespace ownfos::bullet
