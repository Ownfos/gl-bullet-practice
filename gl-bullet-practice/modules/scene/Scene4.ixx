module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <imgui.h>
#include <fmt/format.h>
#include <memory>
#include <vector>
#include <optional>
#include <exception>
#include <iostream>

export module Scene4;

import Window;
import SphereRenderer;
import CubeRenderer;
import LineRenderer;
import InteractiveCamera;

import DynamicsWorld;
import DefaultDynamicsWorldComponents;
import Spring;
import ObjectDragger;

import ImGuiHelper;

using namespace ownfos::opengl;
using namespace ownfos::imgui;
using namespace ownfos::bullet;
using namespace ownfos::utility;

struct RenderedSphere
{
    RigidBody* object;
    glm::vec4 color;
};

export namespace scene4
{
    // Test spring with difference tension and damping parameters
    void run()
    {
        try
        {
            auto init = GLFWInitializer();
            auto window = Window(1200, 800, "hello, world!");
            window.exit_when_pressed(GLFW_KEY_ESCAPE);

            auto imgui = ImGuiHelper(window);
            auto sphere_renderer = SphereRenderer();
            auto cube_renderer = CubeRenderer();
            auto line_renderer = LineRenderer();

            auto camera = InteractiveCamera(
                Camera{
                    .transform = {
                        .position = { 0, 0, 20 }
                    },
                    .projection = Projection::perspective(
                        glm::radians(45.0f),
                        window.get_aspect_ratio(),
                        0.1f,
                        1000.0f
                    )
                },
                window
            );

            // Create a new physics world without gravity
            auto world_components = std::make_unique<DefaultDynamicsWorldComponents>();
            auto world = DynamicsWorld(std::move(world_components));
            world.set_gravity({ 0, 0, 0 });

            // Prepare parameters controllable by ImGui sliders
            float linear_damping = 0.8f;
            float angular_damping = 0.8f;
            float spring_coef = 1.0f;
            float gravity = 0.0f;

            // Prepare ground
            auto ground = std::make_shared<RigidBody>(RigidBodyConfig{
                .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
                .transform = {
                    .position = {0, -2, 0},
                    .scale = {20, 0.2, 20}
                }
            });
            world.add_rigid_body(ground);

            // Prepare spheres
            std::vector<RenderedSphere> rendered_spheres;

            auto create_uniform_sphere = [&](const btVector3& position, const glm::vec4& color) {
                auto sphere = std::make_shared<RigidBody>(RigidBodyConfig{
                    .shape = std::make_shared<btSphereShape>(1.0f),
                    .mass = 0.2f,
                    .transform = {
                        .position = position,
                        .scale = {0.3f, 0.3f, 0.3f}
                    }
                });
                world.add_rigid_body(sphere);
                rendered_spheres.push_back({ .object = sphere.get(), .color = color });

                return sphere;
            };

            auto sphere_color = glm::vec4{ 0.2f, 0.8f, 0.4f, 1.0f };
            auto spheres = std::vector{
                // Spheres representing an end point of a cube
                create_uniform_sphere({ 1, 1, 1 }, sphere_color),
                create_uniform_sphere({ 1, -1, 1 }, sphere_color),
                create_uniform_sphere({ 1, 1, -1 }, sphere_color),
                create_uniform_sphere({ 1, -1, -1 }, sphere_color),
                create_uniform_sphere({ -1, 1, 1 }, sphere_color),
                create_uniform_sphere({ -1, -1, 1 }, sphere_color),
                create_uniform_sphere({ -1, 1, -1 }, sphere_color),
                create_uniform_sphere({ -1, -1, -1 }, sphere_color),
                create_uniform_sphere({ 0, 0, 0 }, sphere_color),

                // Tetrahedron with offset (2, 0, 0)
                create_uniform_sphere({ 3, 0, 0 }, sphere_color),
                create_uniform_sphere({ 2, 1, 0 }, sphere_color),
                create_uniform_sphere({ 2, 0, 1 }, sphere_color),
                create_uniform_sphere({ 3, 1, 1 }, sphere_color),
            };

            // Initialize springs
            auto springs = std::vector<Spring>();

            auto connect_spheres = [&](auto s1, auto s2){
                springs.push_back({ spring_coef, { spheres[s1].get(), {0, 0, 0} }, { spheres[s2].get(), {0, 0, 0} } });
            };

            // Connect first 9 spheres in every possible way
            for (int i = 0; i < 9; ++i)
            {
                for (int j = i + 1; j < 9; ++j)
                {
                    connect_spheres(i, j);
                }
            }

            // Connect next 4 spheres in every possible way
            for (int i = 9; i < 13; ++i)
            {
                for (int j = i + 1; j < 13; ++j)
                {
                    connect_spheres(i, j);
                }
            }

            auto object_dragger = ObjectDragger(window, camera, world);

            // Prevent rendered images from being stretched w.r.t. new window size.
            window.register_resize_handler([&](auto width, auto height) {
                camera.projection = Projection::perspective(glm::radians(60.0f), window.get_aspect_ratio(), 0.1f, 1000.0f);
            });

            // Start the main loop that draws a square with transform applied.
            while (!window.should_close())
            {
                glfwPollEvents();

                // Update world
                {
                    camera.update_transform(window);
                    object_dragger.try_drag_object(window, camera, world);

                    // Update gravity strength
                    world.set_gravity({ 0, -gravity, 0 });

                    // Update damping parameters
                    for (auto& sphere : spheres)
                    {
                        sphere->set_damping(linear_damping, angular_damping);
                    }

                    // Spring between two uniform cubes
                    for (auto& spring : springs)
                    {
                        spring.spring_coef = spring_coef;
                        spring.apply_force();
                    }

                    world.step_simulation(1.0f / 60.0f, 1000);
                }

                // Render
                {
                    window.clear({ 1,1,1,1 });

                    auto cam_mat = camera.get_matrix();

                    // Render ground
                    cube_renderer.render(cam_mat, ground->get_world_transform_matrix(), { 0.7, 0.7, 0.7, 1.0 });

                    // Render all rigid bodies
                    for (const auto& sphere : rendered_spheres)
                    {
                        sphere_renderer.render(cam_mat, sphere.object->get_world_transform_matrix(), sphere.color);
                    }

                    // Draw gizmos for spring connections
                    for (const auto& spring : springs)
                    {
                        line_renderer.render(cam_mat, spring.point1.get_world_point(), spring.point2.get_world_point(), { 0.0f, 1.0f, 1.0f, 1.0f });
                    }

                    // Draw gizmos for object drag positions (dragging point on object & target point to move)
                    if (object_dragger.is_dragging())
                    {
                        auto dragging_state = object_dragger.get_dragging_state();

                        auto gizmo_color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
                        auto start_point = dragging_state.clicked_point.get_world_point();
                        auto end_point = bt2glm(dragging_state.drag_target_point);

                        // Line between points
                        line_renderer.render(cam_mat, start_point, end_point, gizmo_color);

                        // First end
                        {
                            glm::mat4 transform(1.0f);
                            transform = glm::translate(transform, start_point);
                            transform = glm::scale(transform, { 0.1, 0.1, 0.1 });
                            sphere_renderer.render(cam_mat, transform, gizmo_color);
                        }

                        // Second end
                        {
                            glm::mat4 transform(1.0f);
                            transform = glm::translate(transform, end_point);
                            transform = glm::scale(transform, { 0.1, 0.1, 0.1 });
                            sphere_renderer.render(cam_mat, transform, gizmo_color);
                        }
                    }

                    // Visualize x, y, z axis
                    {
                        line_renderer.render(cam_mat, { 0, 0, 0 }, { 100, 0, 0 }, { 1, 0, 0, 1 });
                        line_renderer.render(cam_mat, { 0, 0, 0 }, { 0, 100, 0 }, { 0, 1, 0, 1 });
                        line_renderer.render(cam_mat, { 0, 0, 0 }, { 0, 0, 100 }, { 0, 0, 1, 1 });
                    }

                    // Show imformation about spring configurations
                    imgui.render([&] {
                        ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Once);
                        if (ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                        {
                            ImGui::Text("[Physics Parameters]");
                            ImGui::Separator();
                            ImGui::SliderFloat("Spring Coefficient", &spring_coef, 0.0f, 10.0f);
                            ImGui::SliderFloat("Linear Damping", &linear_damping, 0.0f, 1.0f);
                            ImGui::SliderFloat("Angular Damping", &angular_damping, 0.0f, 1.0f);
                            ImGui::SliderFloat("Gravity", &gravity, 0.0f, 10.0f);
                        }
                        ImGui::End();
                    });

                    window.swap_buffer();
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
} // namespace scene4
