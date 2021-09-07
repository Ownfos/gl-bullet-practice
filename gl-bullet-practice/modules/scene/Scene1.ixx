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

import GLFWInitializer;
import Window;

import CubeRenderer;
import LineRenderer;

import DynamicsWorld;
import DefaultDynamicsWorldComponents;

import GLMAdapter;

import InteractiveCamera;

import ImGuiHelper;

using namespace ownfos::opengl;
using namespace ownfos::imgui;
using namespace ownfos::bullet;

// Custom format function bullet vector type
template<>
struct fmt::formatter<btVector3> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const btVector3& v, FormatContext& ctx)
    {
        auto str = fmt::format("({:6.2f}, {:6.2f}, {:6.2f})", v.x(), v.y(), v.z());
        return fmt::formatter<std::string_view>::format(str, ctx);
    }
};

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

std::shared_ptr<RigidBody> create_voxel(const btVector3& position)
{
    return std::make_shared<RigidBody>(RigidBodyConfig{
            .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
            .mass = 1.0f,
            .transform = {
                .position = position
            }
        });
}

struct LocalPoint
{
    RigidBody* object;
    glm::vec3 local_offset;

    glm::vec3 get_origin() const
    {
        return bt2glm(object->get_world_transform().getOrigin());
    }

    glm::vec3 get_local_point() const
    {
        return local_offset;
    }

    glm::vec3 get_world_point() const
    {
        return apply_transform(object->get_world_transform_matrix(), local_offset);
    }

    glm::vec3 get_world_offset() const
    {
        return get_world_point() - get_origin();
    }
};

struct Spring
{
    float coef;
    float relaxed_length;
    LocalPoint point1;
    LocalPoint point2;

    Spring(float coef, const LocalPoint& point1, const LocalPoint& point2)
        : coef(coef), point1(point1), point2(point2)
    {
        relaxed_length = glm::distance(point1.get_world_point(), point2.get_world_point());
    }

    Spring(float coef, float relaxed_length, const LocalPoint& point1, const LocalPoint& point2)
        : coef(coef), relaxed_length(relaxed_length), point1(point1), point2(point2)
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
            auto force = glm2bt(p1_to_p2 * displacement * coef);

            point1.object->apply_force(force, glm2bt(point1.get_world_offset()));
            point2.object->apply_force(-force, glm2bt(point2.get_world_offset()));
        }
    }
};

struct DraggingState
{
    float last_raycast_distance;
    LocalPoint clicked_point;
    btVector3 drag_target_point = { 0, 0, 0 };
};

struct RenderedCube
{
    RigidBody* object;
    glm::vec4 color;
};

export namespace scene1
{
    void run()
    {
        try
        {
            auto init = GLFWInitializer();
            auto window = Window(1200, 800, "hello, world!");

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

            auto spring_end_1 = create_voxel({ 0, 0, 0 });
            auto spring_end_2 = create_voxel({ 3, 0, 0 });
            world.add_rigid_body(spring_end_1);
            world.add_rigid_body(spring_end_2);
            rendered_cubes.push_back({ .object = spring_end_1.get(), .color = {0.2f, 0.8f, 0.4f, 1.0f} });
            rendered_cubes.push_back({ .object = spring_end_2.get(), .color = {0.2f, 0.8f, 0.4f, 1.0f} });
            spring_end_1->set_damping(0.4f, 0.8f);
            spring_end_2->set_damping(0.4f, 0.8f);

            // Initialize springs between two cubes
            constexpr float coef = 3.0f;
            auto point1 = spring_end_1.get();
            auto point2 = spring_end_2.get();
            auto springs = std::vector<Spring>{
                // Connect center
                Spring(coef, { point1, {0, 0, 0}   }, { point2, {0, 0, 0}    }),

                // Connect four points on X plane
                Spring(coef, { point1, {1, 1, -1}  }, { point2, {-1, 1, -1}  }),
                Spring(coef, { point1, {1, -1, -1} }, { point2, {-1, -1, -1} }),
                Spring(coef, { point1, {1, 1, 1}   }, { point2, {-1, 1, 1}   }),
                Spring(coef, { point1, {1, -1, 1}  }, { point2, {-1, -1, 1}  }),

                // Cross eight points
                Spring(coef, { point1, {1, -1, -1} }, { point2, {-1, 1, 1}   }),
                Spring(coef, { point1, {1, 1, -1}  }, { point2, {-1, -1, 1}  }),
                Spring(coef, { point1, {1, -1, 1}  }, { point2, {-1, 1, -1}  }),
                Spring(coef, { point1, {1, 1, 1}   }, { point2, {-1, -1, -1} }),
            };

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
                window.get_normalized_cursor_pos()
            );
            window.set_cursor_enabled(false);

            // Variables used to drag an object around the camera with mouse cursor
            std::optional<DraggingState> dragging_state;

            window.register_key_handler([&](auto key, auto action) {
                // Cursor activation
                if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
                {
                    window.set_cursor_enabled(true);
                }
                if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
                {
                    window.set_cursor_enabled(false);

                    // Prevent camera from rotating drastically due to cursor movement while camera update is disabled.
                    // Note that we do not rotate nor translate camera while left alt key is pressed,
                    // making the interactive camera think the cursor is staying at the position when we just pressed the key.
                    camera.update_cursor_pos(window);
                }

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

                // Stop dragging object
                if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
                {
                    dragging_state = {};
                }
            });

            // Prevent rendered images from being stretched w.r.t. new window size.
            window.register_resize_handler([&](auto width, auto height) {
                camera.projection = Projection::perspective(glm::radians(60.0f), window.get_aspect_ratio(), 0.1f, 1000.0f);
            });

            // FPS counter
            auto frame_record = FPSRecord(60);

            // VSync statewith ImGui checkbox.
            // We will call glfwSwapInterval only when current checkbox value mismatches the current state.
            glfwSwapInterval(1);
            bool vsync_checkbox_state = true;
            bool current_vsync_state = true;

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
                    {
                        if (window.get_key_state(GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
                        {
                            camera.update_transform(window);
                        }
                    }

                    // Drag clicked object
                    {
                        if (dragging_state.has_value() && window.get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
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

                    // Spring between two uniform cubes
                    {
                        for (const auto& spring : springs)
                        {
                            spring.apply_force();
                        }
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
                    if (dragging_state.has_value())
                    {
                        auto gizmo_color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
                        auto start_point = dragging_state->clicked_point.get_world_point();
                        auto end_point = bt2glm(dragging_state->drag_target_point);
                        
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
                            if (dragging_state.has_value())
                            {
                                ImGui::Text(fmt::format("Clicked point (local): {}", glm2bt(dragging_state->clicked_point.get_local_point())).c_str());
                                ImGui::Text(fmt::format("Clicked point (world): {}", glm2bt(dragging_state->clicked_point.get_world_point())).c_str());
                                ImGui::Text(fmt::format("Drag target point: {}", dragging_state->drag_target_point).c_str());
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
