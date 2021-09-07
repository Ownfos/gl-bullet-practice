module;

#include <bullet/LinearMath/btVector3.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

export module InteractiveCamera;

export import Window;
export import Camera;

export namespace ownfos::opengl
{
    class InteractiveCamera : public Camera
    {
    public:
        InteractiveCamera(Camera&& camera, const glm::vec2& initial_cursor_pos)
            : Camera(std::move(camera))
            , prev_cursor_pos(initial_cursor_pos)
        {
            auto initial_rotation = camera.transform.euler_rotation();
            yaw = initial_rotation.yaw;
            pitch = initial_rotation.pitch;
        }

        // Check mouse cursor movement and keyboard input to rotate and translate camera like FPS shooter
        void update_transform(Window& window)
        {
            auto current_cursor_pos = window.get_normalized_cursor_pos();

            // Rotation
            constexpr float cam_rotation_speed = glm::radians(35.0f);
            auto cursor_movement = current_cursor_pos - prev_cursor_pos;

            yaw -= cursor_movement.x * cam_rotation_speed;
            pitch += cursor_movement.y * cam_rotation_speed;

            // Prevent camera from exceeding 90 degrees up/down,
            // which creates a wierd behavior as if we've rotate 180 degrees around Y-axis.
            pitch = glm::clamp(pitch, glm::radians(-89.5f), glm::radians(89.5f));

            transform.rotation = { yaw, pitch, 0.0f };

            // Translation
            constexpr float cam_movement_speed = 0.2f;
            if (window.get_key_state(GLFW_KEY_W) == GLFW_PRESS) transform.position += transform.forward() * cam_movement_speed;
            if (window.get_key_state(GLFW_KEY_S) == GLFW_PRESS) transform.position -= transform.forward() * cam_movement_speed;
            if (window.get_key_state(GLFW_KEY_D) == GLFW_PRESS) transform.position += transform.right() * cam_movement_speed;
            if (window.get_key_state(GLFW_KEY_A) == GLFW_PRESS) transform.position -= transform.right() * cam_movement_speed;
            if (window.get_key_state(GLFW_KEY_SPACE) == GLFW_PRESS) transform.position += btVector3{ 0, 0.4, 0 } * cam_movement_speed;
            if (window.get_key_state(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) transform.position -= btVector3{ 0, 0.4, 0 } * cam_movement_speed;

            prev_cursor_pos = current_cursor_pos;
        }

        // Update the recorded last cursor position to current value.
        // 
        // This prevents camera rotation from quantum-jumping in cases where
        // update_transform() is not called for several frames while cursor is still moving.
        // Note that prev_cursor_pos only gets updated when update_transform() is invoked,
        // making the difference between current cursor position and prev_cursor_pos quite large.
        // 
        // If you wish to pause camera movement temporarily, make sure to
        // call this function when you restart updating the camera again.
        void update_cursor_pos(Window& window)
        {
            prev_cursor_pos = window.get_normalized_cursor_pos();;
        }

    private:
        glm::vec2 prev_cursor_pos;
        float yaw;
        float pitch;
    };
} // namespace ownfos::opengl
