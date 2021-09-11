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

export module Scene3;

import Window;
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

struct RenderedCube
{
    RigidBody* object;
    glm::vec4 color;
};

export namespace scene3
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
            float linear_damping = 0.4f;
            float angular_damping = 0.8f;
            float spring_coef = 3.0f;

            // Prepares cubes with several springs attached
            std::vector<RenderedCube> rendered_cubes;

            auto create_uniform_cube = [&](const btVector3& position, const glm::vec4& color) {
                auto cube = std::make_shared<RigidBody>(RigidBodyConfig{
                    .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
                    .mass = 1.0f,
                    .transform = {
                        .position = position
                    }
                });
                world.add_rigid_body(cube);
                rendered_cubes.push_back({ .object = cube.get(), .color = color });

                return cube;
            };
            auto spring_end_1 = create_uniform_cube({ 0, 0, -2 }, { 0.2f, 0.8f, 0.4f, 1.0f });
            auto spring_end_2 = create_uniform_cube({ 3, 0, -2 }, { 0.8f, 0.2f, 0.4f, 1.0f });

            // Initialize springs between two cubes
            auto point1 = spring_end_1.get();
            auto point2 = spring_end_2.get();
            auto springs = std::vector<Spring>{
                // Connect center
                Spring(spring_coef, { point1, {0, 0, 0}   }, { point2, {0, 0, 0}    }),

                // Connect four points on X plane
                Spring(spring_coef, { point1, {1, 1, -1}  }, { point2, {-1, 1, -1}  }),
                Spring(spring_coef, { point1, {1, -1, -1} }, { point2, {-1, -1, -1} }),
                Spring(spring_coef, { point1, {1, 1, 1}   }, { point2, {-1, 1, 1}   }),
                Spring(spring_coef, { point1, {1, -1, 1}  }, { point2, {-1, -1, 1}  }),

                // Cross eight points
                Spring(spring_coef, { point1, {1, -1, -1} }, { point2, {-1, 1, 1}   }),
                Spring(spring_coef, { point1, {1, 1, -1}  }, { point2, {-1, -1, 1}  }),
                Spring(spring_coef, { point1, {1, -1, 1}  }, { point2, {-1, 1, -1}  }),
                Spring(spring_coef, { point1, {1, 1, 1}   }, { point2, {-1, -1, -1} }),
            };

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

                    // Update damping parameters
                    spring_end_1->set_damping(linear_damping, angular_damping);
                    spring_end_2->set_damping(linear_damping, angular_damping);

                    // Spring between two uniform cubes
                    for (auto& spring : springs)
                    {
                        spring.spring_coef = spring_coef;
                        spring.apply_force();
                    }

                    world.step_simulation(1.0f / 60.0f, 100);
                }

                // Render
                {
                    window.clear({ 1,1,1,1 });

                    auto cam_mat = camera.get_matrix();

                    // Render all rigid bodies
                    for (const auto& cube : rendered_cubes)
                    {
                        cube_renderer.render(cam_mat, cube.object->get_world_transform_matrix(), cube.color);
                    }

                    // Draw gizmos for spring connections
                    for (const auto& spring : springs)
                    {
                        line_renderer.render(cam_mat, spring.point1.get_world_point(), spring.point2.get_world_point(), { 0.0f, 0.0f, 1.0f, 1.0f });
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
                            cube_renderer.render(cam_mat, transform, gizmo_color);
                        }

                        // Second end
                        {
                            glm::mat4 transform(1.0f);
                            transform = glm::translate(transform, end_point);
                            transform = glm::scale(transform, { 0.1, 0.1, 0.1 });
                            cube_renderer.render(cam_mat, transform, gizmo_color);
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

                            ImGui::NewLine();

                            auto rotation = camera.transform.euler_rotation();
                            ImGui::Text("[Camera Transform]");
                            ImGui::Separator();
                            ImGui::Text(fmt::format("Yaw   (Y-axis): {}", glm::degrees(rotation.yaw)).c_str());
                            ImGui::Text(fmt::format("Pitch (X-axis): {}", glm::degrees(rotation.pitch)).c_str());
                            ImGui::Text(fmt::format("Roll  (Z-axis): {}", glm::degrees(rotation.roll)).c_str());
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
} // namespace scene3
