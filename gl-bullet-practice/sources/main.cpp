
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

using namespace ownfos::opengl;
using namespace ownfos::primitives;
using namespace ownfos::bullet;

// hello world example from Bullet Physics github, refactored with custom wrapper classes.
void test_bullet()
{
    // Create a new physics world with downward gravity.
    auto world_components = std::make_unique<DefaultDynamicsWorldComponents>();
    auto world = DynamicsWorld(std::move(world_components));
    world.set_gravity({ 0, -10, 0 });

    // Place a box shaped ground far below.
    auto ground_shape = std::make_shared<btBoxShape>(btVector3{ 50, 50, 50 });
    auto ground = std::make_shared<RigidBody>(ground_shape, 0, btVector3{ 0, -56, 0 });
    world.add_rigid_body(ground);

    // Place a sphere above the ground.
    auto object_shape = std::make_shared<btSphereShape>(1.0f);
    auto object = std::make_shared<RigidBody>(object_shape, 1.0f, btVector3{ 2, 10, 0 });
    world.add_rigid_body(object);

    // Simulate and print the updated positions for each object.
    for (int i = 0; i < 150; ++i)
    {
        world.step_simulation(1.0f / 60.0f, 10);

        std::cout << fmt::format("Iteration {}\n", i);
        for (const auto& rigid_body : world.get_rigid_bodies())
        {
            auto origin = rigid_body->get_world_transform().getOrigin();

            std::cout << fmt::format("world pos object {:.6f}, {:.6f}, {:.6f}\n", origin.getX(), origin.getY(), origin.getZ());
        }
    }

    std::cout << "end\n";
}

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

        // Prepare a plane model with color.
        // You can combine multiple independent buffers containing index or vertex data
        // to create a single BufferPreset, which corresponds to a VAO of OpenGL.
        auto plane = create_plane_model_data();
        auto color_data = std::vector<float>{
            1,0,1,1,
            1,0,0,1,
            0,1,0,1,
            0,0,1,1
        };
        auto color_buffer = VertexBuffer(color_data, {{.location = 1, .count = 4}});
        auto model = BufferPreset({ &plane.position, &plane.indices, &color_buffer });

        window.register_key_handler([&](auto key, auto action) {
            // Switch the color of the green vertex (the 4 floats starting from the 8th element of the buffer).
            // 
            // You can change the content of an existing VertexBuffer at runtime
            // by providing an index in the buffer to start writing and a span containing new data.
            // Be careful not to exceed write beyond existing buffer's range.
            // It won't automatically resize the buffer for you.
            //
            // If you want to resize the buffer and use completely different data,
            // call set_new_data() with the new data and its usage.
            if (key == GLFW_KEY_A && action == GLFW_PRESS)
            {
                float new_color[] = { 1,1,1,1 };
                color_buffer.update_data(8, new_color);
            }
            if (key == GLFW_KEY_S && action == GLFW_PRESS)
            {
                auto color_data = std::vector<float>{
                    1,0,1,1,
                    1,0,0,1,
                    0,1,0,1,
                    0,0,1,1
                };
                color_buffer.set_new_data(color_data, GL_STATIC_DRAW);
            }

            // Run hello world example from Bullet Physics github.
            if (key == GLFW_KEY_Q && action == GLFW_PRESS)
            {
                test_bullet();
            }

            // Exit program when ESC is pressed.
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                window.set_close_flag();
            }
        });

        // Start the main loop that draws a square with transform applied.
        while (!window.should_close())
        {
            window.clear({ 0,1,1,1 });

            auto transform = glm::mat4(1.0f);
            transform = glm::translate(transform, { 0.5, 0, 0 });
            transform = glm::rotate(transform, glm::radians(60.0f), {0, 0, 1});
            transform = glm::scale(transform, { 0.5, 0.5, 0 });

            shader.use();
            shader.set_uniform("color_multiplier", glm::vec4{ 0.5f, 0.7f, 0.9f, 1.0f });
            shader.set_uniform("transform", transform);
            model.use();
            draw_indexed(GL_TRIANGLES, 6, 0);

            window.swap_buffer();

            glfwPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
