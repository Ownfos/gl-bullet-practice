
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <iostream>
#include <memory>
#include <fmt/format.h>

import GLFWInitializer;
import Window;
import ShaderProgram;
import VertexBuffer;

import Plane;
import Cube;

namespace ownfos::bullet
{
    class IDynamicsWorldComponents
    {
    public:
        virtual btCollisionConfiguration* get_collision_configuration() const = 0;
        virtual btDispatcher* get_dispatcher() const = 0;
        virtual btBroadphaseInterface* get_broadphase() const = 0;
        virtual btConstraintSolver* get_constraint_solver() const = 0;
    };

    class DefaultDynamicsWorldComponents : public IDynamicsWorldComponents
    {
    public:
        DefaultDynamicsWorldComponents()
            : collision_configuration(new btDefaultCollisionConfiguration())
            , collision_dispatcher(new btCollisionDispatcher(collision_configuration))
            , broadphase(new btDbvtBroadphase())
            , solver(new btSequentialImpulseConstraintSolver())
        {}

        ~DefaultDynamicsWorldComponents()
        {
            if (collision_configuration != nullptr)
            {
                delete collision_configuration;
            }

            if (collision_dispatcher != nullptr)
            {
                delete collision_dispatcher;
            }

            if (broadphase != nullptr)
            {
                delete broadphase;
            }

            if (solver != nullptr)
            {
                delete solver;
            }
        }

        btCollisionConfiguration* get_collision_configuration() const override
        {
            return collision_configuration;
        }

        btDispatcher* get_dispatcher() const override
        {
            return collision_dispatcher;
        }

        btBroadphaseInterface* get_broadphase() const override
        {
            return broadphase;
        }

        btConstraintSolver* get_constraint_solver() const override
        {
            return solver;
        }

    private:
        btCollisionConfiguration* collision_configuration = nullptr;
        btDispatcher* collision_dispatcher = nullptr;
        btBroadphaseInterface* broadphase = nullptr;
        btConstraintSolver* solver = nullptr;
    };

    class RigidBody
    {
    public:
        RigidBody(std::shared_ptr<btCollisionShape> shape, btScalar mass, btVector3 origin)
            : shape(shape)
        {
            auto inertia = btVector3{ 0, 0, 0 };

            if (mass > 0.0f)
            {
                shape->calculateLocalInertia(mass, inertia);
            }

            auto transform = btTransform();
            transform.setIdentity();
            transform.setOrigin(origin);

            motion_state = new btDefaultMotionState(transform);
            rigid_body = new btRigidBody({ mass, motion_state, shape.get(), inertia });
        }

        ~RigidBody()
        {
            if (rigid_body != nullptr)
            {
                delete rigid_body;
            }

            if (motion_state != nullptr)
            {
                delete motion_state;
            }
        }

        btRigidBody* get_body() const
        {
            return rigid_body;
        }

    private:
        std::shared_ptr<btCollisionShape> shape;
        btRigidBody* rigid_body = nullptr;
        btMotionState* motion_state = nullptr;
    };

    class DynamicsWorld
    {
    public:
        DynamicsWorld(std::unique_ptr<IDynamicsWorldComponents> components)
            : components(std::move(components))
            , world(new btDiscreteDynamicsWorld(
                this->components->get_dispatcher(),
                this->components->get_broadphase(),
                this->components->get_constraint_solver(),
                this->components->get_collision_configuration()
            ))
        {}

        ~DynamicsWorld()
        {
            if (world != nullptr)
            {
                delete world;
            }
        }

        void set_gravity(const btVector3& gravity)
        {
            world->setGravity(gravity);
        }

        void add_rigid_body(const RigidBody& rigidBody)
        {
            world->addRigidBody(rigidBody.get_body());
        }

        void step_simulation(btScalar time_step, int max_substep)
        {
            world->stepSimulation(time_step, max_substep);
        }

        btCollisionObjectArray& get_collision_objects()
        {
            return world->getCollisionObjectArray();
        }

    private:
        std::unique_ptr<IDynamicsWorldComponents> components;
        btDynamicsWorld* world = nullptr;
    };
}


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
    auto ground = RigidBody(ground_shape, 0, { 0, -56, 0 });
    world.add_rigid_body(ground);

    // Place a sphere above the ground.
    auto object_shape = std::make_shared<btSphereShape>(1.0f);
    auto object = RigidBody(object_shape, 1.0f, { 2, 10, 0 });
    world.add_rigid_body(object);

    // Simulate and print the updated positions for each object.
    for (int i = 0; i < 150; ++i)
    {
        world.step_simulation(1.0f / 60.0f, 10);

        auto& col_objects = world.get_collision_objects();
        for (int j = col_objects.size() - 1; j >= 0; --j)
        {
            auto object = col_objects[j];
            auto body = btRigidBody::upcast(object);
            btTransform trans;
            if (body && body->getMotionState())
            {
                body->getMotionState()->getWorldTransform(trans);
            }
			else
			{
				trans = object->getWorldTransform();
			}

            auto origin = trans.getOrigin();
            std::cout << fmt::format("world pos object {0}: {1:.6f}, {2:.6f}, {3:.6f}\n", j, origin.getX(), origin.getY(), origin.getZ());
        }
    }
}

int main()
{
    try
    {
        test_bullet();

        auto init = GLFWInitializer();
        auto window = Window(600, 400, "hello, world!");
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
        auto color_data = BufferDataVertex(std::move(color), {{.location = 1, .count = 4}});

        auto data = plane::create_data();

        auto model = VertexBuffer({ &data.position, &data.indices, &color_data }, GL_STATIC_DRAW);



        while (!window.should_close())
        {
            window.clear({ 0,1,1,1 });

            shader.use();
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
