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
        auto ground_shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 });
        auto ground = std::make_shared<RigidBody>(ground_shape, 0, btVector3{ 0, -11, 0 }, btQuaternion{ 0,0,0, 1 }, btVector3{ 5, 1, 5 });
        world.add_rigid_body(ground);

        // Place a sphere above the ground.
        auto object_shape = std::make_shared<btBoxShape>(btVector3{ 1, 1, 1 });
        auto object = std::make_shared<RigidBody>(object_shape, 1.0f, btVector3{ 4.5, 10, 0 }, btQuaternion{ btVector3{0, 0, 1}, glm::radians(45.1f) }, btVector3{ 1, 1, 1 });
        world.add_rigid_body(object);

        auto cam_pos = glm::vec3{ 2, 5,-60 };

        window.register_key_handler([&](auto key, auto action) {
            // Camera movement
            if (key == GLFW_KEY_UP) cam_pos.z += 0.5f;
            if (key == GLFW_KEY_DOWN) cam_pos.z -= 0.5f;
            if (key == GLFW_KEY_LEFT) cam_pos.x += 0.5f;
            if (key == GLFW_KEY_RIGHT) cam_pos.x -= 0.5f;
            if (key == GLFW_KEY_Z) cam_pos.y += 0.5f;
            if (key == GLFW_KEY_X) cam_pos.y -= 0.5f;

            // Exit program when ESC is pressed.
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                window.set_close_flag();
            }
        });

        // Start the main loop that draws a square with transform applied.
        while (!window.should_close())
        {
            world.step_simulation(1.0f / 60.0f, 10);

            window.clear({ 1,1,1,1 });

            // Set up camera that follows the falling mini cube
            auto object_origin = object->get_world_transform().getOrigin();

            auto cam_target = glm::vec3{ object_origin.getX(), object_origin.getY(), object_origin.getZ() };
            auto cam_up = glm::vec3{ 0,1,0 };

            auto view = glm::lookAt(cam_pos, cam_target, cam_up);
            auto projection = glm::perspective(glm::radians(60.0f), window.get_aspect_ratio(), 0.1f, 1000.0f);

            shader.use();
            shader.set_uniform("view", view);
            shader.set_uniform("projection", projection);

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

            glfwPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
