
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
import VertexBuffer;

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
        test_bullet();

        auto init = GLFWInitializer();
        auto window = Window(600, 600, "hello, world!");
        window.register_key_handler([&](auto key, auto action)
            {
                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                {
                    window.set_close_flag();
                }
            });

        auto vs = ShaderSourceFromFile("resources/default_vs.txt");
        auto fs = ShaderSourceFromFile("resources/default_fs.txt");
        auto shader = ShaderProgram(&vs, &fs);

        auto color = std::vector<float>{
            1,0,1,1,
            1,0,0,1,
            0,1,0,1,
            0,0,1,1
        };
        auto color_data = BufferDataVertex(color, {{.location = 1, .count = 4}});

        auto data = plane::create_data();

        auto model = VertexBuffer({ &data.position, &data.indices, &color_data });

        float new_color[] = { 1,1,1,1 };
        color_data.update_data(4, new_color);

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
