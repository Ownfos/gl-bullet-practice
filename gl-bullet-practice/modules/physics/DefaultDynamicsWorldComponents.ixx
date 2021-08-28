module;

#include <bullet/btBulletDynamicsCommon.h>

export module DefaultDynamicsWorldComponents;

export import IDynamicsWorldComponents;

export namespace ownfos::bullet
{
    class DefaultDynamicsWorldComponents : public IDynamicsWorldComponents
    {
    public:
        DefaultDynamicsWorldComponents()
            : collision_configuration(new btDefaultCollisionConfiguration())
            , collision_dispatcher(new btCollisionDispatcher(collision_configuration))
            , broadphase(new btDbvtBroadphase())
            , solver(new btSequentialImpulseConstraintSolver())
        {}

        ~DefaultDynamicsWorldComponents()
        {
            if (solver != nullptr)
            {
                delete solver;
            }

            if (broadphase != nullptr)
            {
                delete broadphase;
            }

            if (collision_dispatcher != nullptr)
            {
                delete collision_dispatcher;
            }

            if (collision_configuration != nullptr)
            {
                delete collision_configuration;
            }
        }

        btCollisionConfiguration* get_collision_configuration() const override
        {
            return collision_configuration;
        }

        btDispatcher* get_dispatcher() const override
        {
            return collision_dispatcher;
        }

        btBroadphaseInterface* get_broadphase() const override
        {
            return broadphase;
        }

        btConstraintSolver* get_constraint_solver() const override
        {
            return solver;
        }

    private:
        btCollisionConfiguration* collision_configuration = nullptr;
        btDispatcher* collision_dispatcher = nullptr;
        btBroadphaseInterface* broadphase = nullptr;
        btConstraintSolver* solver = nullptr;
    };
} // namespace ownfos::bullet
