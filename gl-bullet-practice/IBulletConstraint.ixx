module;

#include <bullet/btBulletDynamicsCommon.h>

export module IBulletConstraint;

export namespace ownfos::bullet
{
    class IBulletConstraint
    {
    public:
        virtual ~IBulletConstraint() = default;

        virtual btTypedConstraint* get_constraint() const = 0;
        virtual bool is_linked_object_collision_allowed() const = 0;
    };
} // namespace ownfos::bullet
