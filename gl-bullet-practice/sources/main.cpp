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

int main()
{
    try
    {
        auto init = GLFWInitializer();
        auto window = Window(600, 600, "hello, world!");

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
                .scale = {5, 1, 5}
            }
        });
        world.add_rigid_body(ground);

        // Place a mini cube above the ground.
        auto object = std::make_shared<RigidBody>(RigidBodyConfig{
            .shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 }),
            .mass = 1.0f,
            .transform = {
                .position = {4.5, 10, 0},
                .rotation = {{0, 0, 1}, glm::radians(45.1f)} // Rotate 45.1 degrees around +Z axis
            }
        });
        world.add_rigid_body(object);

        // Prepare camera for view and projection matrix
        auto camera = Camera{
            .transform = {
                .position = {-3, 10, -40}
            },
            .projection = Projection::perspective(
                glm::radians(60.0f),
                window.get_aspect_ratio(),
                0.1f,
                1000.0f
            )
        };

        window.register_key_handler([&](auto key, auto action) {
            // Camera movement
            if (key == GLFW_KEY_UP) camera.transform.position += {0, 0, 0.5f};
            if (key == GLFW_KEY_DOWN) camera.transform.position -= {0, 0, 0.5f};
            if (key == GLFW_KEY_LEFT) camera.transform.position += {0.5f, 0, 0};
            if (key == GLFW_KEY_RIGHT) camera.transform.position -= {0.5f, 0, 0};
            if (key == GLFW_KEY_Z) camera.transform.position += {0, 0.5f, 0};
            if (key == GLFW_KEY_X) camera.transform.position -= {0, 0.5f, 0};

            // Exit program when ESC is pressed.
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                window.set_close_flag();
            }
        });

        // Prevent rendered images from being stretched w.r.t. new window size.
        window.register_resize_handler([&](auto width, auto height) {
            //camera.projection = Projection::perspective(glm::radians(60.0f), window.get_aspect_ratio(), 0.1f, 1000.0f);
        });

        // Start the main loop that draws a square with transform applied.
        while (!window.should_close())
        {
            world.step_simulation(1.0f / 60.0f, 10);

            window.clear({ 1,1,1,1 });

            // Set up camera that follows the falling mini cube
            //camera.view = View::look_target(cam_pos, to_glm(object->get_world_transform().getOrigin()));
            camera.transform.look_at(object->get_world_transform().getOrigin());

            shader.use();
            shader.set_uniform("camera", camera.get_matrix());

            // Now we will draw ground and falling object with single cube model.
            // This works because the btCollisionShape used by these rigid bodies
            // is btBoxShape with half-width of 1, which is identical to the cube_model data we've generated.
            cube_model.use();

            // Draw the falling mini cube
            shader.set_uniform("world", object->get_world_transform_matrix());
            shader.set_uniform("color", glm::vec4{ 0.5f, 0.7f, 0.9f, 1.0f });
            draw_indexed(GL_TRIANGLES, 36, 0);

            // Draw the ground
            shader.set_uniform("world", ground->get_world_transform_matrix());
            shader.set_uniform("color", glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
            draw_indexed(GL_TRIANGLES, 36, 0);

            window.swap_buffer();

            if (window.get_key_state(GLFW_KEY_B) == GLFW_PRESS)
            {
                auto pos = window.get_cursor_pos();
                std::cout << fmt::format("{}, {}\n", pos.x, pos.y);
            }

            glfwPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
