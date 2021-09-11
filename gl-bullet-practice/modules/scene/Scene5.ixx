module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <iostream>
#include <exception>

export module Scene5;

import Window;
import InteractiveCamera;
import CubeRenderer;
import LineRenderer;

import DynamicsWorld;
import DefaultDynamicsWorldComponents;
import ObjectDragger;
import SpringConstraint;

using namespace ownfos::opengl;
using namespace ownfos::bullet;
using namespace ownfos::utility;

std::shared_ptr<RigidBody> create_uniform_cube(const btVector3& position)
{
    return std::make_shared<RigidBody>(RigidBodyConfig{
        .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
        .mass = 1.0f,
        .transform = {
            .position = position,
            .scale = {0.5, 0.5, 0.5}
        }
    });
}

void draw_local_axis(LineRenderer& renderer, const glm::mat4& cam_mat, RigidBody* object)
{
    auto origin = object->get_world_transform().getOrigin();
    auto transform = Transform{
        .rotation = object->get_world_transform().getRotation()
    };
    renderer.render(cam_mat, bt2glm(origin), bt2glm(origin + transform.right()), { 1,0,0,1 });
    renderer.render(cam_mat, bt2glm(origin), bt2glm(origin + transform.up()), { 0,1,0,1 });
    renderer.render(cam_mat, bt2glm(origin), bt2glm(origin - transform.forward()), { 0,0,1,1 }); // Note: forward == -z direction
}

export namespace scene5
{
    // Test Bullet's built-in spring constraint
    void run()
    {
        try
        {
            auto init = GLFWInitializer();
            auto window = Window(1200, 800, "hello, world!", { 3, 3 });
            window.exit_when_pressed(GLFW_KEY_ESCAPE);

            auto cube_renderer = CubeRenderer();
            auto line_renderer = LineRenderer();

            // Create a new physics world without gravity
            auto world_components = std::make_unique<DefaultDynamicsWorldComponents>();
            auto world = DynamicsWorld(std::move(world_components));
            world.set_gravity({ 0, -9.8, 0 });

            // Prepare ground
            auto ground = std::make_shared<RigidBody>(RigidBodyConfig{
                .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
                .transform = {
                    .position = {0, -2, 0},
                    .scale = {20, 0.2, 20}
                }
            });
            world.add_rigid_body(ground);

            auto cube1 = create_uniform_cube({ -2, 0, 0 });
            auto cube2 = create_uniform_cube({ 2, 0, 0 });

            world.add_rigid_body(cube1);
            world.add_rigid_body(cube2);

            auto point1 = LocalPoint{ cube1.get(), { 1, 0, 0 } };
            auto point2 = LocalPoint{ cube2.get(), { -1, 0, 0 } };
            auto linear_constraint = SpringParams{ .stiffness = 10.0f, .damping = 0.3f };
            auto angular_constraint = SpringParams{ .stiffness = 3.0f, .damping = 0.2f };
            auto spring = SpringConstraint(point1, point2, linear_constraint, angular_constraint);

            world.add_constraint(&spring);

            auto camera = InteractiveCamera(
                Camera{
                    .transform = {
                        .position = {0, 0, 10}
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

            auto object_dragger = ObjectDragger(window, camera, world);

            while (!window.should_close())
            {
                glfwPollEvents();

                camera.update_transform(window);
                object_dragger.try_drag_object(window, camera, world);

                world.step_simulation(1.0f / 60.0f, 1000);

                window.clear({ 1,1,1,1 });

                auto cam_mat = camera.get_matrix();
                cube_renderer.render(cam_mat, ground->get_world_transform_matrix(), { 1, 0.5, 1, 1 });
                cube_renderer.render(cam_mat, cube1->get_world_transform_matrix(), { 1, 1, 0, 1 });
                cube_renderer.render(cam_mat, cube2->get_world_transform_matrix(), { 0, 1, 1, 1 });
                draw_local_axis(line_renderer, cam_mat, cube1.get());
                draw_local_axis(line_renderer, cam_mat, cube2.get());
                line_renderer.render(cam_mat, {0, 0, 0}, { 1,0,0 }, { 1,0,0,1 });
                line_renderer.render(cam_mat, {0, 0, 0}, { 0,1,0 }, { 0,1,0,1 });
                line_renderer.render(cam_mat, {0, 0, 0}, { 0,0,1 }, { 0,0,1,1 });

                window.swap_buffer();
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
} // namespace scene5
