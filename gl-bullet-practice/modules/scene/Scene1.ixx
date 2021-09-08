module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>
#include <iostream>
#include <memory>
#include <vector>
#include <numeric>
#include <ctime>

export module Scene1;

import Window;

import CubeRenderer;
import LineRenderer;

import DynamicsWorld;
import DefaultDynamicsWorldComponents;

import InteractiveCamera;
import Spring;
import ObjectDragger;

import ImGuiHelper;

import FormatMathTypes;

using namespace ownfos::opengl;
using namespace ownfos::imgui;
using namespace ownfos::bullet;
using namespace ownfos::utility;

class Timer
{
public:
    Timer()
    {
        mark_start();
    }

    void mark_start()
    {
        start = clock();
    }

    bool check_duration(clock_t threshold)
    {
        clock_t current = clock();
        return (current - start > threshold);
    }

private:
    clock_t start;
};

class FPSRecord
{
public:
    FPSRecord(int num_records)
        : frames(num_records, 0)
        , frame_num(num_records, 0)
    {
        std::iota(frame_num.begin(), frame_num.end(), 0);
    }

    void on_frame_end()
    {
        frame_count++;
        if (timer.check_duration(CLOCKS_PER_SEC))
        {
            timer.mark_start();
            record(frame_count);
            frame_count = 0;
        }
    }

    void record(int value)
    {
        for (int i = 0; i < frames.size() - 1; ++i)
        {
            frames[i] = frames[i + 1];
        }
        frames[frames.size() - 1] = value;
    }

    void reset()
    {
        for (auto& e : frames)
        {
            e = 0;
        }

        timer.mark_start();
        frame_count = 0;
    }

    void plot_line() const
    {
        ImPlot::PlotLine("FPS Record", frame_num.data(), frames.data(), frames.size());
    }

    int get_last_fps() const
    {
        return frames.back();
    }

private:
    std::vector<int> frames;
    std::vector<int> frame_num;

    Timer timer;
    int frame_count = 0;
};

struct RenderedCube
{
    RigidBody* object;
    glm::vec4 color;
};

export namespace scene1
{
    // Test multiple cube spawning, FPS plot, and object dragging
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

            // Create a new physics world with downward gravity.
            auto world_components = std::make_unique<DefaultDynamicsWorldComponents>();
            auto world = DynamicsWorld(std::move(world_components));
            world.set_gravity({ 0, -9.8, 0 });

            std::vector<RenderedCube> rendered_cubes;

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
            rendered_cubes.push_back({ .object = ground.get(), .color = {1.0f, 0.0f, 0.0f, 1.0f} });

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
            rendered_cubes.push_back({ .object = cube_object.get(), .color = {0.5f, 0.7f, 0.9f, 1.0f} });

            // Place more cubes!
            for (int pos = 0; pos < 5; ++pos)
            {
                auto more_cube = std::make_shared<RigidBody>(RigidBodyConfig{
                    .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
                    .mass = 1.0f,
                    .transform = {
                        .position = {4.5f, pos * 2.0f, 5.0f},
                        .rotation = {{0, 0, 1}, glm::radians(45.1f)}, // Rotate 45.1 degrees around +Z axis
                        .scale = {1, 0.5, 0.5}
                    }
                });
                world.add_rigid_body(more_cube);
                rendered_cubes.push_back({ .object = more_cube.get(), .color = {0.7f, 0.2f, 0.5f, 1.0f} });
            }

            // Prepare camera for view and projection matrix
            auto camera = InteractiveCamera(
                Camera{
                    .transform = {
                        .position = {0, 0, 40}
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

            // Prevent rendered images from being stretched w.r.t. new window size.
            window.register_resize_handler([&](auto width, auto height) {
                camera.projection = Projection::perspective(glm::radians(60.0f), window.get_aspect_ratio(), 0.1f, 1000.0f);
            });

            // FPS counter
            auto frame_record = FPSRecord(60);

            // VSync state with ImGui checkbox.
            // We will call glfwSwapInterval only when current checkbox value mismatches the current state.
            glfwSwapInterval(1);
            bool vsync_checkbox_state = true;
            bool current_vsync_state = true;

            auto object_dragger = ObjectDragger(window, camera, world);

            // Start the main loop that draws a square with transform applied.
            while (!window.should_close())
            {
                // Toggle VSync on/off if user clicked the checkbox
                if (current_vsync_state != vsync_checkbox_state)
                {
                    glfwSwapInterval(vsync_checkbox_state);
                    current_vsync_state = vsync_checkbox_state;
                    frame_record.reset();
                }

                // Update world
                {
                    // Control camera with mouse
                    camera.update_transform(window);

                    // Drag clicked object
                    object_dragger.try_drag_object(window, camera, world);

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

                    // Show imformation about current object dragging state
                    imgui.render([&] {
                        ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Once);
                        if (ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                        {
                            if (object_dragger.is_dragging())
                            {
                                auto dragging_state = object_dragger.get_dragging_state();

                                ImGui::Text(fmt::format("Clicked point (local): {}", format(dragging_state.clicked_point.get_local_point())).c_str());
                                ImGui::Text(fmt::format("Clicked point (world): {}", format(dragging_state.clicked_point.get_world_point())).c_str());
                                ImGui::Text(fmt::format("Drag target point: {}", format(dragging_state.drag_target_point)).c_str());
                            }

                            ImGui::Separator();

                            ImGui::Checkbox("VSync", &vsync_checkbox_state);

                            ImGui::Separator();

                            if (ImGui::CollapsingHeader("FPS"))
                            {
                                if (ImPlot::BeginPlot("FPS", "", "Frames/sec", ImVec2(500, 0), 0, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit))
                                {
                                    frame_record.plot_line();
                                    ImPlot::EndPlot();
                                }
                                ImGui::Text(fmt::format("Frames per second: {}", frame_record.get_last_fps()).c_str());
                            }
                        }
                        ImGui::End();
                     });

                    window.swap_buffer();
                }

                glfwPollEvents();

                frame_record.on_frame_end();
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
} // namespace scene1
