module;

#include <bullet/btBulletDynamicsCommon.h>
#include <memory>
#include <vector>
#include <optional>
#include <unordered_map>

export module DynamicsWorld;

export import IDynamicsWorldComponents;
export import IBulletConstraint;
export import RigidBody;
export import Ray;
export import HitInfo;

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

        void add_constraint(IBulletConstraint* constraint)
        {
            world->addConstraint(constraint->get_constraint(), constraint->is_linked_object_collision_allowed());
        }

        void step_simulation(btScalar time_step, int max_substep)
        {
            world->stepSimulation(time_step, max_substep);
        }

        const std::vector<std::shared_ptr<RigidBody>>& get_rigid_bodies()
        {
            return rigid_bodies;
        }

        std::optional<std::vector<HitInfo>> raycast_all(const Ray& ray)
        {
            btCollisionWorld::AllHitsRayResultCallback result(ray.start_point, ray.end_point);
            world->rayTest(ray.start_point, ray.end_point, result);

            if (!result.hasHit())
            {
                return {};
            }

            std::vector<HitInfo> hit_info;

            for (int i = 0; i < result.m_hitFractions.size(); ++i)
            {
                hit_info.push_back({
                    // RigidBody sets user pointer to its own 'this' pointer, so we can guarantee that this cast is valid.
                    .object = static_cast<RigidBody*>(result.m_collisionObjects[i]->getUserPointer()),

                    // Each hit point is calculated as interpolation of start and end point of the given ray.
                    .position = ray.get_lerped_point(result.m_hitFractions[i]),
                    .normal = result.m_hitNormalWorld[i]
                });
            }

            return hit_info;
        }

        std::optional<HitInfo> raycast_closest(const Ray& ray) const
        {
            btCollisionWorld::ClosestRayResultCallback result(ray.start_point, ray.end_point);
            world->rayTest(ray.start_point, ray.end_point, result);

            if (!result.hasHit())
            {
                return {};
            }
            else
            {
                return HitInfo{
                    // RigidBody sets user pointer to its own 'this' pointer, so we can guarantee that this cast is valid.
                    .object = static_cast<RigidBody*>(result.m_collisionObject->getUserPointer()),

                    .position = result.m_hitPointWorld,
                    .normal = result.m_hitNormalWorld
                };
            }
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
