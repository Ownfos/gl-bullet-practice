module;

#include <bullet/btBulletDynamicsCommon.h>
#include <memory>
#include <vector>

export module DynamicsWorld;

export import IDynamicsWorldComponents;
export import RigidBody;

export namespace ownfos::bullet
{
    class DynamicsWorld
    {
    public:
        DynamicsWorld(std::unique_ptr<IDynamicsWorldComponents>&& components)
            : components(std::move(components))
            , world(new btDiscreteDynamicsWorld(
                this->components->get_dispatcher(),
                this->components->get_broadphase(),
                this->components->get_constraint_solver(),
                this->components->get_collision_configuration()
            ))
        {}

        ~DynamicsWorld()
        {
            for (const auto& rigid_body : rigid_bodies)
            {
                world->removeCollisionObject(rigid_body->get_body());
            }

            if (world != nullptr)
            {
                delete world;
            }
        }

        void set_gravity(const btVector3& gravity)
        {
            world->setGravity(gravity);
        }

        void add_rigid_body(std::shared_ptr<RigidBody> rigid_body)
        {
            world->addRigidBody(rigid_body->get_body());

            // Keep the reference to the RigidBody pointer so that it stays alive
            // until all collision objects are removed from the world at destructor.
            // If the btRigidBody instance gets deallocated before we remove it,
            // world->removeCollisionObject(...) will cause an invalid access error.
            rigid_bodies.push_back(rigid_body);
        }

        void step_simulation(btScalar time_step, int max_substep)
        {
            world->stepSimulation(time_step, max_substep);
        }

        const std::vector<std::shared_ptr<RigidBody>>& get_rigid_bodies()
        {
            return rigid_bodies;
        }

    private:
        // The instances used to create a physics world, such as solver, broadphase, etc....
        std::unique_ptr<IDynamicsWorldComponents> components;

        // Reference to all rigid bodies added to this physics world.
        // This assures that all btRigidBody instances are not deleted until we remove them from this world.
        std::vector<std::shared_ptr<RigidBody>> rigid_bodies;

        // The physics world where simulation happens.
        btDynamicsWorld* world = nullptr;
    };
} // namespace ownfos::bullet
