#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <fmt/format.h>
#include <imgui.h>
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

import ImGuiHelper;

using namespace ownfos::opengl;
using namespace ownfos::imgui;
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

        auto imgui = ImGuiHelper(window);

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
        world.set_gravity({ 0, -9.8, 0 });

        // Place a box shaped ground below.
        // Note that the default mass value is 0, which means the object is static.
        auto ground = std::make_shared<RigidBody>(RigidBodyConfig{
            .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
            .transform = {
                .position = {0, -11, 0},
                .scale = {20, 0.2, 20}
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

        // Place more cubes!
        std::vector<std::shared_ptr<RigidBody>> more_cubes;
        for (int pos = 0; pos < 5; ++pos)
        {
            auto cube_object2 = std::make_shared<RigidBody>(RigidBodyConfig{
                .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
                .mass = 1.0f,
                .transform = {
                    .position = {4.5f, pos * 2.0f, 5.0f},
                    .rotation = {{0, 0, 1}, glm::radians(45.1f)}, // Rotate 45.1 degrees around +Z axis
                    .scale = {1, 0.5, 0.5}
                }
            });
            world.add_rigid_body(cube_object2);
            more_cubes.push_back(cube_object2);
        }

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

        // Variables used to drag an object around the camera with mouse cursor
        float last_raycast_distance;
        glm::vec3 clicked_point_local;
        btVector3 clicked_point_world;
        btVector3 drag_target_point;
        RigidBody* dragging_object = nullptr;

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

        window.register_mouse_button_handler([&](auto button, auto action, auto mods) {
            // Check if there's an object to drag with mouse
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                // Coordinate conversion from normalized screen space to world space
                auto screen_pos = window.get_normalized_cursor_pos();
                auto world_pos = camera.screen_to_world_point(screen_pos);

                // Find the closest object
                auto ray = Ray::from_screenpoint(camera, screen_pos);
                auto result = world.raycast_closest(ray);

                if (result.has_value())
                {
                    // This object will be referenced in the main loop to apply force
                    dragging_object = result->object;

                    // Since raycast results are given in world coordinate,
                    // the values become invalid after the object moves.
                    //
                    // In order to keep track of the same point on the model,
                    // convert the clicked point to the model's local coordinate.
                    auto world_to_local = glm::inverse(result->object->get_world_transform_matrix());
                    clicked_point_local = apply_transform(world_to_local, bt2glm(result->position));

                    // The object will now get dragged to a point in a sphere around camera position,
                    // where the radius is the initial raycast distance.
                    last_raycast_distance = ray.start_point.distance(result->position);
                }
            }

            // Stop dragging object
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
            {
                dragging_object = nullptr;
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

                // Drag clicked object
                {
                    if (dragging_object != nullptr && window.get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                    {
                        // Calculate the position in world space where we want the clicked point to reach
                        auto screen_pos = window.get_normalized_cursor_pos();
                        auto ray = Ray::from_screenpoint(camera, screen_pos);
                        drag_target_point = ray.start_point + ray.get_direction() * last_raycast_distance;

                        // Calculate the world space coordinate of the clicked point
                        clicked_point_world = glm2bt(apply_transform(dragging_object->get_world_transform_matrix(), clicked_point_local));

                        // The relative coordinate of the point in world space where we want to apply force
                        auto object_pos = dragging_object->get_world_transform().getOrigin();
                        auto local_offset = clicked_point_world - object_pos;

                        // A spring-like force towards the target point with damping
                        constexpr float spring_coef = 30.0f;
                        auto force = (drag_target_point - clicked_point_world) * spring_coef;

                        constexpr float damping_coef = 5.0f;
                        auto damping = -dragging_object->get_linear_velocity() * damping_coef;

                        dragging_object->apply_force(force + damping, local_offset);
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

                // Draw other cubes
                shader.set_uniform("color", glm::vec4{ 0.7f, 0.2f, 0.5f, 1.0f });
                for (const auto& cube : more_cubes)
                {
                    shader.set_uniform("world", cube->get_world_transform_matrix());
                    draw_indexed(GL_TRIANGLES, 36, 0);
                }

                // Draw the ground
                shader.set_uniform("world", ground->get_world_transform_matrix());
                shader.set_uniform("color", glm::vec4{ 1.0f, 0.0f, 0.0f, 0.7f });
                draw_indexed(GL_TRIANGLES, 36, 0);


                // Draw gizmos for object drag positions (dragging point on object & target point to move)
                if (dragging_object != nullptr)
                {
                    shader.set_uniform("color", glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });
                    {
                        glm::mat4 transform(1.0f);
                        transform = glm::translate(transform, bt2glm(clicked_point_world));
                        transform = glm::scale(transform, { 0.1, 0.1, 0.1 });

                        shader.set_uniform("world", transform);
                        draw_indexed(GL_TRIANGLES, 36, 0);
                    }

                    {
                        glm::mat4 transform(1.0f);
                        transform = glm::translate(transform, bt2glm(drag_target_point));
                        transform = glm::scale(transform, { 0.1, 0.1, 0.1 });

                        shader.set_uniform("world", transform);
                        draw_indexed(GL_TRIANGLES, 36, 0);
                    }
                }

                // Show imformation about current object dragging state
                imgui.render([&] {
                    ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Once);
                    ImGui::Begin("Dragging Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                    if (dragging_object != nullptr)
                    {
                        ImGui::Text(fmt::format("Clicked point (local): {}", glm2bt(clicked_point_local)).c_str());
                        ImGui::Text(fmt::format("Clicked point (world): {}", clicked_point_world).c_str());
                        ImGui::Text(fmt::format("Drag target point: {}", drag_target_point).c_str());
                    }
                    else
                    {
                        ImGui::Text("No object is selected");
                    }
                    ImGui::End();
                });

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
