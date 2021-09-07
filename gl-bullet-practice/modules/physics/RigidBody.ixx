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

            // Save this pointer for future reference from btRigidBody to ownfos::bullet::RigidBody.
            // Used by ownfos::bullet::DynamicsWorld to get RigidBody instance from raycast result.
            rigid_body->setUserPointer(this);
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

        // Returns the mass of rigidbody.
        // If the object is static, return value is 0.
        btScalar get_mass() const
        {
            auto inv_mass = rigid_body->getInvMass();
            if (inv_mass == 0)
            {
                return 0;
            }
            else
            {
                return 1.0f / inv_mass;
            }
        }

        const btVector3& get_linear_velocity() const
        {
            return rigid_body->getLinearVelocity();
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

        void set_damping(btScalar linear, btScalar angular)
        {
            rigid_body->setDamping(linear, angular);
        }

        void apply_force(const btVector3& force, const btVector3& offset_from_center = { 0, 0, 0 })
        {
            rigid_body->activate(true);
            rigid_body->applyForce(force, offset_from_center);
        }

        void apply_impulse(const btVector3& impulse, const btVector3& offset_from_center = { 0, 0, 0 })
        {
            rigid_body->activate(true);
            rigid_body->applyImpulse(impulse, offset_from_center);
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
