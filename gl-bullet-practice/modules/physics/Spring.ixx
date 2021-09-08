module;

#include <glm/glm.hpp>
#include <bullet/LinearMath/btVector3.h>

export module Spring;

export import LocalPoint;

using namespace ownfos::utility;

export namespace ownfos::bullet
{
    struct Spring
    {
        float spring_coef;
        float relaxed_length;
        LocalPoint point1;
        LocalPoint point2;

        Spring(float spring_coef, const LocalPoint& point1, const LocalPoint& point2)
            : spring_coef(spring_coef), point1(point1), point2(point2)
        {
            relaxed_length = glm::distance(point1.get_world_point(), point2.get_world_point());
        }

        Spring(float spring_coef, float relaxed_length, const LocalPoint& point1, const LocalPoint& point2)
            : spring_coef(spring_coef), relaxed_length(relaxed_length), point1(point1), point2(point2)
        {}

        void apply_force() const
        {
            auto p1_world = point1.get_world_point();
            auto p2_world = point2.get_world_point();
            auto current_length = glm::distance(p1_world, p2_world);

            if (current_length > 0.00001f)
            {
                auto p1_to_p2 = glm::normalize(p2_world - p1_world);

                // Spring force is proportional to the extra displacement from the relaxed state
                auto displacement = current_length - relaxed_length;
                auto force = glm2bt(p1_to_p2 * displacement * spring_coef);

                point1.object->apply_force(force, glm2bt(point1.get_world_offset()));
                point2.object->apply_force(-force, glm2bt(point2.get_world_offset()));
            }
        }
    };
} // namespace ownfos::bullet
