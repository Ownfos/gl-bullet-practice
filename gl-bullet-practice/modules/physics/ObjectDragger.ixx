module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <optional>

export module ObjectDragger;

export import Window;
export import Camera;
export import LocalPoint;
export import DynamicsWorld;

using namespace ownfos::opengl;
using namespace ownfos::utility;

export namespace ownfos::bullet
{
    struct DraggingState
    {
        float last_raycast_distance;
        LocalPoint clicked_point;
        btVector3 drag_target_point = { 0, 0, 0 };
    };

    class ObjectDragger
    {
    public:
        ObjectDragger(Window& window, Camera& camera, DynamicsWorld& world)
        {
            window.register_mouse_button_handler([&](auto button, auto action, auto mods) {
                if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
                {
                    try_start_dragging(window, camera, world);
                }
                if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
                {
                    stop_dragging();
                }
                });
        }

        void try_drag_object(Window& window, Camera& camera, DynamicsWorld& world)
        {
            if (is_dragging() && window.get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                // Calculate the position in world space where we want the clicked point to reach
                auto screen_pos = window.get_normalized_cursor_pos();
                auto ray = Ray::from_screenpoint(camera, screen_pos);
                dragging_state->drag_target_point = ray.start_point + ray.get_direction() * dragging_state->last_raycast_distance;

                auto clicked_point_world = glm2bt(dragging_state->clicked_point.get_world_point());

                // A spring-like force towards the target point with damping
                constexpr float spring_coef = 30.0f;
                auto force = (dragging_state->drag_target_point - clicked_point_world) * spring_coef;

                constexpr float damping_coef = 5.0f;
                auto damping = -dragging_state->clicked_point.object->get_linear_velocity() * damping_coef;

                dragging_state->clicked_point.object->apply_force(force + damping, glm2bt(dragging_state->clicked_point.get_world_offset()));
            }
        }

        bool is_dragging() const
        {
            return dragging_state.has_value();
        }

        DraggingState get_dragging_state()
        {
            return dragging_state.value();
        }

    private:
        void try_start_dragging(Window& window, Camera& camera, DynamicsWorld& world)
        {
            // Coordinate conversion from normalized screen space to world space
            auto screen_pos = window.get_normalized_cursor_pos();
            auto world_pos = camera.screen_to_world_point(screen_pos);

            // Find the closest object
            auto ray = Ray::from_screenpoint(camera, screen_pos);
            auto result = world.raycast_closest(ray);

            if (result.has_value())
            {
                // Since raycast results are given in world coordinate,
                // the values become invalid after the object moves.
                //
                // In order to keep track of the same point on the model,
                // convert the clicked point to the model's local coordinate.
                auto world_to_local = glm::inverse(result->object->get_world_transform_matrix());
                auto clicked_point_local = apply_transform(world_to_local, bt2glm(result->position));

                // The object will now get dragged to a point in a sphere around camera position,
                // where the radius is the initial raycast distance.
                auto raycast_distance = ray.start_point.distance(result->position);

                // Save the current state and use it on main loop
                dragging_state = {
                    .last_raycast_distance = raycast_distance,
                    .clicked_point = LocalPoint{
                        .object = result->object,
                        .local_offset = clicked_point_local
                    }
                };
            }
        }

        void stop_dragging()
        {
            dragging_state = {};
        }

        std::optional<DraggingState> dragging_state;
    };
} // namespace ownfos::bullet
