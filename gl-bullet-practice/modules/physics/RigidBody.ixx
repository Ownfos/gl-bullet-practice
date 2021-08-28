module;

#include <bullet/btBulletDynamicsCommon.h>
#include <memory>

export module RigidBody;

export namespace ownfos::bullet
{
    class RigidBody
    {
    public:
        RigidBody(std::shared_ptr<btCollisionShape> shape, btScalar mass, btVector3 origin)
            : shape(shape)
        {
            auto inertia = btVector3{ 0, 0, 0 };

            if (mass > 0.0f)
            {
                shape->calculateLocalInertia(mass, inertia);
            }

            auto transform = btTransform();
            transform.setIdentity();
            transform.setOrigin(origin);

            motion_state = new btDefaultMotionState(transform);
            rigid_body = new btRigidBody({ mass, motion_state, shape.get(), inertia });
        }

        ~RigidBody()
        {
            if (motion_state != nullptr)
            {
                delete motion_state;
            }

            if (rigid_body != nullptr)
            {
                delete rigid_body;
            }
        }

        btRigidBody* get_body() const
        {
            return rigid_body;
        }

        btMotionState* get_motion_state() const
        {
            return motion_state;
        }

        btTransform get_world_transform() const
        {
            btTransform transform;
            motion_state->getWorldTransform(transform);
            return transform;
        }

    private:
        std::shared_ptr<btCollisionShape> shape;
        btRigidBody* rigid_body = nullptr;
        btMotionState* motion_state = nullptr;
    };
} // namespace ownfos::bullet
