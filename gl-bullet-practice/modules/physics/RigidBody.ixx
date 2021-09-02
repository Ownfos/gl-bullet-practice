module;

#include <bullet/btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

export module RigidBody;

export import Transform;

export namespace ownfos::bullet
{
    struct RigidBodyConfig
    {
        std::shared_ptr<btCollisionShape> shape;
        btScalar mass = 0;
        Transform transform = {};
    };

    class RigidBody
    {
    public:
        RigidBody(const RigidBodyConfig& config)
            : shape(config.shape), scale(config.transform.scale)
        {
            auto inertia = btVector3{ 0, 0, 0 };

            if (config.mass > 0.0f)
            {
                shape->calculateLocalInertia(config.mass, inertia);
            }

            // Set position and rotation
            auto bullet_transform = btTransform();
            bullet_transform.setIdentity();
            bullet_transform.setOrigin(config.transform.position);
            bullet_transform.setRotation(config.transform.rotation);

            // Set scale
            shape->setLocalScaling(scale);

            motion_state = new btDefaultMotionState(bullet_transform);
            rigid_body = new btRigidBody({ config.mass, motion_state, shape.get(), inertia });
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

        glm::mat4 get_world_transform_matrix() const
        {
            glm::mat4 transform;

            // Get position and rotation matrix
            get_world_transform().getOpenGLMatrix(glm::value_ptr(transform));

            // Apply scale, which is not included in the btTransform.
            // Note that object scaling is handled by btCollisionShape.
            transform = glm::scale(transform, { scale.getX(), scale.getY(), scale.getZ() });

            return transform;
        }

    private:
        // Keep reference to the pointer so that shape doesn't get destroyed while rigid body is alive
        std::shared_ptr<btCollisionShape> shape;

        // Keep the initial scale value, so that we can make scaling affect get_world_transform_matrix()
        btVector3 scale;

        btRigidBody* rigid_body = nullptr;
        btMotionState* motion_state = nullptr;
    };
} // namespace ownfos::bullet
