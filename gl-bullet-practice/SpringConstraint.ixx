module;

#include <bullet/btBulletDynamicsCommon.h>

export module SpringConstraint;

export import IBulletConstraint;
export import LocalPoint;

using namespace ownfos::utility;

export namespace ownfos::bullet
{
    struct SpringParams
    {
        float stiffness;
        float damping;
    };

    class SpringConstraint : public IBulletConstraint
    {
    public:
        SpringConstraint(LocalPoint point1, LocalPoint point2, SpringParams linear_params, SpringParams angular_params, bool allow_collision = false)
            : allow_collision(allow_collision)
        {
            // Do not allow linked objects to freeze
            point1.object->get_body()->setActivationState(DISABLE_DEACTIVATION);
            point2.object->get_body()->setActivationState(DISABLE_DEACTIVATION);

            // Prepare local offsets which are supposed to be in same world position
            btTransform frameInA, frameInB;
            frameInA = btTransform::getIdentity();
            frameInA.setOrigin(glm2bt(point1.local_offset) * point1.object->get_scale());
            frameInB = btTransform::getIdentity();
            frameInB.setOrigin(glm2bt(point2.local_offset) * point2.object->get_scale());

            spring = new btGeneric6DofSpringConstraint(*point1.object->get_body(), *point2.object->get_body(), frameInA, frameInB, false);

            // Ignore linear limits
            spring->setLinearUpperLimit({ 0,0,0 });
            spring->setLinearLowerLimit({ 1,1,1 });

            // Ignore angular limits
            spring->setAngularUpperLimit({ 0,0,0 });
            spring->setAngularLowerLimit({ 1,1,1 });

            // Enable spring on all axis (linear xyz and angular xyz)
            for (int i = 0; i < 6; ++i)
            {
                spring->enableSpring(i, true);
            }
            spring->setEquilibriumPoint();

            // Configure stiffness and damping
            set_params_linear(linear_params);
            set_params_angular(angular_params);
        }

        ~SpringConstraint()
        {
            delete spring;
        }

        // Set stiffness and damping regarding linear motion
        void set_params_linear(SpringParams params)
        {
            for (int i = 0; i < 3; ++i)
            {
                spring->setStiffness(i, params.stiffness);
                spring->setDamping(i, params.damping);
            }
        }

        // Set stiffness and damping regarding angular motion
        void set_params_angular(SpringParams params)
        {
            for (int i = 3; i < 6; ++i)
            {
                spring->setStiffness(i, params.stiffness);
                spring->setDamping(i, params.damping);
            }
        }

        btTypedConstraint* get_constraint() const override
        {
            return spring;
        }

        bool is_linked_object_collision_allowed() const override
        {
            return allow_collision;
        }

    private:
        bool allow_collision;
        btGeneric6DofSpringConstraint* spring;
    };
} // namespace ownfos::bullet
