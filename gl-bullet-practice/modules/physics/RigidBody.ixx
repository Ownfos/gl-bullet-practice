module;

#include <bullet/btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

export module RigidBody;

export namespace ownfos::bullet
{
    class RigidBody
    {
    public:
        RigidBody(std::shared_ptr<btCollisionShape> shape, btScalar mass, btVector3 position, btVector3 scale)
            : shape(shape), scale(scale)
        {
            auto inertia = btVector3{ 0, 0, 0 };

            if (mass > 0.0f)
            {
                shape->calculateLocalInertia(mass, inertia);
            }

            // Set position and rotation
            auto transform = btTransform();
            transform.setIdentity();
            transform.setOrigin(position);

            // Set scale
            shape->setLocalScaling(scale);

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
        std::shared_ptr<btCollisionShape> shape;
        btVector3 scale;
        btRigidBody* rigid_body = nullptr;
        btMotionState* motion_state = nullptr;
    };
} // namespace ownfos::bullet
