#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <ctime>

import GLFWInitializer;
import Window;
import ShaderProgram;
import BufferPreset;

import Plane;
import Cube;

import DynamicsWorld;
import DefaultDynamicsWorldComponents;

import GLMAdapter;

import Camera;

using namespace ownfos::opengl;
using namespace ownfos::primitives;
using namespace ownfos::bullet;

// Custom format function bullet vector type
template<>
struct fmt::formatter<btVector3> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const btVector3& v, FormatContext& ctx)
    {
        auto str = fmt::format("({:5.2f}, {:5.2f}, {:5.2f})", v.x(), v.y(), v.z());
        return fmt::formatter<std::string_view>::format(str, ctx);
    }
};

int main()
{
    try
    {
        auto init = GLFWInitializer();
        auto window = Window(800, 600, "hello, world!");

        // Prepare shader.
        auto vs = ShaderSourceFromFile("resources/default_vs.txt");
        auto fs = ShaderSourceFromFile("resources/default_fs.txt");
        auto shader = ShaderProgram(&vs, &fs);

        // Prepare a cube model.
        auto cube = create_cube_model_data();
        auto cube_model = BufferPreset({ &cube.position, &cube.normal, &cube.indices });

        // Create a new physics world with downward gravity.
        auto world_components = std::make_unique<DefaultDynamicsWorldComponents>();
        auto world = DynamicsWorld(std::move(world_components));
        world.set_gravity({ 0, -10, 0 });

        // Place a box shaped ground below.
        // Note that the default mass value is 0, which means the object is static.
        auto ground = std::make_shared<RigidBody>(RigidBodyConfig{
            .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
            .transform = {
                .position = {0, -11, 0},
                .scale = {10, 0.2, 10}
            }
        });
        world.add_rigid_body(ground);

        // Place a mini cube above the ground.
        auto cube_object = std::make_shared<RigidBody>(RigidBodyConfig{
            .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
            .mass = 1.0f,
            .transform = {
                .position = {4.5, 10, 0},
                .rotation = {{0, 0, 1}, glm::radians(45.1f)}, // Rotate 45.1 degrees around +Z axis
                .scale = {1, 0.5, 0.5}
            }
        });
        world.add_rigid_body(cube_object);

        // Prepare camera for view and projection matrix
        auto cam_yaw = 0.0f;
        auto cam_pitch = 0.0f;
        auto camera = Camera{
            .transform = {
                .position = {0, 0, 40},
                .rotation = {cam_yaw, cam_pitch, 0.0f}
            },
            .projection = Projection::perspective(
                glm::radians(45.0f),
                window.get_aspect_ratio(),
                0.1f,
                1000.0f
            )
        };
        auto prev_cursor = window.get_normalized_cursor_pos();
        window.set_cursor_enabled(false);

        window.register_key_handler([&](auto key, auto action) {
            // Cursor activation
            if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) window.set_cursor_enabled(true);
            if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) window.set_cursor_enabled(false);

            // Exit program when ESC is pressed.
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                window.set_close_flag();
            }
        });

        // Prevent rendered images from being stretched w.r.t. new window size.
        window.register_resize_handler([&](auto width, auto height) {
            camera.projection = Projection::perspective(glm::radians(60.0f), window.get_aspect_ratio(), 0.1f, 1000.0f);
        });

        // Start the main loop that draws a square with transform applied.
        while (!window.should_close())
        {
            // Update world
            {
                // Control camera with mouse
                {
                    auto current_cursor = window.get_normalized_cursor_pos();

                    if (window.get_key_state(GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
                    {
                        // Camera rotation
                        constexpr float cam_rotation_speed = glm::radians(35.0f);
                        auto cursor_movement = current_cursor - prev_cursor;

                        cam_yaw -= cursor_movement.x * cam_rotation_speed;
                        cam_pitch += cursor_movement.y * cam_rotation_speed;

                        camera.transform.rotation = { cam_yaw, cam_pitch, 0.0f };

                        // Camera movement
                        constexpr float cam_movement_speed = 0.2f;
                        if (window.get_key_state(GLFW_KEY_W) == GLFW_PRESS) camera.transform.position += camera.transform.forward() * cam_movement_speed;
                        if (window.get_key_state(GLFW_KEY_S) == GLFW_PRESS) camera.transform.position -= camera.transform.forward() * cam_movement_speed;
                        if (window.get_key_state(GLFW_KEY_D) == GLFW_PRESS) camera.transform.position += camera.transform.right() * cam_movement_speed;
                        if (window.get_key_state(GLFW_KEY_A) == GLFW_PRESS) camera.transform.position -= camera.transform.right() * cam_movement_speed;
                        if (window.get_key_state(GLFW_KEY_SPACE) == GLFW_PRESS) camera.transform.position += {0, 0.2, 0} * cam_movement_speed;
                        if (window.get_key_state(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.transform.position -= {0, 0.2, 0} * cam_movement_speed;
                    }

                    prev_cursor = current_cursor;
                }

                // Test screen to world coordinate conversion, raycast, and force application
                {
                    // Coordinate conversion from normalized screen space to world space
                    auto screen_pos = window.get_normalized_cursor_pos();
                    auto world_pos = camera.screen_to_world_point(screen_pos);

                    // Find the closest object
                    auto ray = Ray::from_screenpoint(camera, screen_pos);
                    auto result = world.raycast_closest(ray);

                    if (result.has_value())
                    {
                        std::cout << fmt::format(
                            "[Raycast Hit Closest] world position: {} world normal: {}\n",
                            result->position,
                            result->normal
                        );

                        // Pull object
                        if (window.get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                        {
                            // Apply a force parallel to the normal vector of the selected point
                            auto force_point = result->position - result->object->get_transform().position;
                            auto force = result->normal * 10.0f;

                            result->object->apply_force(force, force_point);
                        }

                        // Push object
                        if (window.get_mouse_button_state(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                        {
                            // Apply a force parallel to the ray direction
                            auto force_point = result->position - result->object->get_transform().position;
                            auto force = ray.get_direction() * 10.0f;

                            result->object->apply_force(force, force_point);
                        }
                    }
                }

                world.step_simulation(1.0f / 60.0f, 10);
            }

            // Render
            {
                window.clear({ 1,1,1,1 });

                shader.use();
                shader.set_uniform("camera", camera.get_matrix());

                // Now we will draw ground and falling object with single cube model.
                // This works because the btCollisionShape used by these rigid bodies
                // is btBoxShape with half-width of 1, which is identical to the cube_model data we've generated.
                cube_model.use();

                // Draw the falling mini cube
                shader.set_uniform("world", cube_object->get_world_transform_matrix());
                shader.set_uniform("color", glm::vec4{ 0.5f, 0.7f, 0.9f, 1.0f });
                draw_indexed(GL_TRIANGLES, 36, 0);

                // Draw the ground
                shader.set_uniform("world", ground->get_world_transform_matrix());
                shader.set_uniform("color", glm::vec4{ 1.0f, 0.0f, 0.0f, 0.7f });
                draw_indexed(GL_TRIANGLES, 36, 0);

                window.swap_buffer();
            }

            glfwPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
