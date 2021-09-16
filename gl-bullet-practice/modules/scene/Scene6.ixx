module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bullet/LinearMath/btVector3.h>
#include <fmt/format.h>
#include <imgui.h>
#include <memory>
#include <vector>
#include <iostream>
#include <exception>

export module Scene6;

import Window;
import InteractiveCamera;
import SphereRenderer;
import LineRenderer;
import ImGuiHelper;

import Multiply;
import Add;
import Subtract;
import Value;

using namespace ownfos::opengl;
using namespace ownfos::imgui;
using namespace ownfos::math;

class TimeDependent : public Value
{
public:
    TimeDependent(std::string_view name, float val, std::shared_ptr<Value> time, std::shared_ptr<TimeDependent> time_derivative)
        : Value(val, name), time(time), time_derivative(time_derivative)
    {}

    TimeDependent& operator=(float val)
    {
        Value::operator=(val);
        return *this;
    }

    virtual std::shared_ptr<IExpression> total_derivative(IExpression* expression) const override
    {
        return time.get() == expression ? time_derivative : Value::total_derivative(expression);
    }

private:
    std::shared_ptr<Value> time;
    std::shared_ptr<IExpression> time_derivative;
};

struct GeneralizedCoordinate
{
    std::shared_ptr<TimeDependent> position;
    std::shared_ptr<TimeDependent> velocity;
    std::shared_ptr<TimeDependent> acceleration;
    float inverse_mass;

    GeneralizedCoordinate(std::string_view name, float position_value, float velocity_value, float mass, std::shared_ptr<Value> time)
    {
        acceleration = std::make_shared<TimeDependent>(fmt::format("d^2({})/dt^2", name), 0, time, nullptr);
        velocity = std::make_shared<TimeDependent>(fmt::format("d({})/dt", name), velocity_value, time, acceleration);
        position = std::make_shared<TimeDependent>(name, position_value, time, velocity);
        inverse_mass = mass == 0.0f ? 0.0f : 1.0f / mass;
    }
};

export namespace scene6
{
    // Test lagrangian mechanics with constraint (single particle on xy-plane with gravity, where radius should be constant)
    void run()
    {
        try
        {
            auto init = GLFWInitializer();
            auto window = Window(1200, 800, "hello, world!", { 3, 3 });
            window.exit_when_pressed(GLFW_KEY_ESCAPE);

            auto imgui = ImGuiHelper(window);

            auto sphere_renderer = SphereRenderer();
            auto line_renderer = LineRenderer();

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

            auto time = std::make_shared<Value>();

            auto desired_radius = 2.0f;
            auto gravity = 0.0f;

            // A system with 2 degrees of freedom (two linear position or one planar position)
            auto q1 = GeneralizedCoordinate("q1", desired_radius, 0, 10, time);
            auto q2 = GeneralizedCoordinate("q2", 0, 2, 10, time);
            auto system = std::vector{ &q1, &q2 };

            // A circular constraint with radius = l
            auto l = std::make_shared<Value>(desired_radius);
            auto squared_radius = q1.position * q1.position + q2.position * q2.position;
            auto constraint = squared_radius;
            auto constraint_target = l * l;

            // Spring configuration for constraint modification.
            // Since current constraint value might not be the desired value,
            // apply a spring-like force that will keep constraint value near the desired one.
            auto constraint_coef = 0.0f;
            auto constraint_damping = 0.0f;

            // Gradient of constraint (elements of Jacobian matrix)
            auto j1 = constraint->total_derivative(q1.position.get());
            auto j2 = constraint->total_derivative(q2.position.get());

            // First order time derivative of Jacobian
            auto j1dot = j1->total_derivative(time.get());
            auto j2dot = j2->total_derivative(time.get());

            auto constraint_velocity = constraint->total_derivative(time.get());

            while (!window.should_close())
            {
                // Update world
                {
                    constexpr float time_step = 1.0f / 60.0f;
                    constexpr int num_iter = 1000;
                    for (int i = 0; i < num_iter; ++i)
                    {
                        // How to calculate lambda for single constraint:
                        // acceleration = (external force + constraint force) * (1/m)
                        // where constraint force = lambda * d(constraint)/dq = lambda * j
                        // d^2(constraint)/dt^2 = sum(dj/dt * dq/dt + j * d^2q/dt^2) = C
                        // 
                        // Note: C should be 0 (keep it static) if current constraint value is valid,
                        //       but we know that it doesn't always happen to be so.
                        //       Think of constraint as a particle connected to a spring
                        //       with damping where relaxation point is desired constraint value.
                        // 
                        // The above equation is a linear equation w.r.t. unknown variable lambda "A + lambda * B = 0" where A and B are some constants.
                        // Therefore, lambda = (C - A) / B (if B != 0)
                        auto q1extforce = 0;
                        auto q2extforce = -gravity / q2.inverse_mass; // Gravity

                        auto A = j1dot->value() * q1.velocity->value() + j2dot->value() * q2.velocity->value() + j1->value() * q1extforce * q1.inverse_mass + j2->value() * q2extforce * q2.inverse_mass;
                        auto B = j1->value() * j1->value() * q1.inverse_mass + j2->value() * j2->value() * q2.inverse_mass;
                        auto C = (constraint_target->value() - constraint->value()) * constraint_coef - constraint_velocity->value() * constraint_damping;
                        auto lambda = (C - A) / B;

                        // Ignore divide by zero
                        if (glm::abs(B) < 0.0001f)
                        {
                            lambda = 0.0f;
                        }

                        *q1.acceleration.get() = (q1extforce + lambda * j1->value()) * q1.inverse_mass;
                        *q2.acceleration.get() = (q2extforce + lambda * j2->value()) * q2.inverse_mass;

                        // Euler integration
                        constexpr float delta_time = time_step / num_iter;
                        for (auto q : system)
                        {
                            *q->position.get() = q->position->value() + q->velocity->value() * delta_time;
                            *q->velocity.get() = q->velocity->value() + q->acceleration->value() * delta_time;
                        }
                    }
                }

                // Render
                {
                    glfwPollEvents();

                    camera.update_transform(window);

                    window.clear({ 1,1,1,1 });

                    auto cam_mat = camera.get_matrix();

                    // Axis
                    line_renderer.render(cam_mat, {0, 0, 0}, { 1,0,0 }, { 1,0,0,1 });
                    line_renderer.render(cam_mat, {0, 0, 0}, { 0,1,0 }, { 0,1,0,1 });
                    line_renderer.render(cam_mat, {0, 0, 0}, { 0,0,1 }, { 0,0,1,1 });

                    // Rotating object
                    auto object_pos = glm::vec3{ q1.position->value(), q2.position->value(), 0 };
                    sphere_renderer.render(cam_mat, glm::scale(glm::translate(glm::mat4(1.0f), object_pos), {0.5, 0.5, 0.5}), {0.0f,0.5f,0.5f,1.0f});

                    // Constraint force (net force - gravity)
                    auto constraint_force = glm::vec3{ q1.acceleration->value(), q2.acceleration->value() + gravity, 0 };
                    line_renderer.render(cam_mat, object_pos, object_pos + glm::normalize(constraint_force), { 1,0,1,1 });

                    // Velocity
                    auto velocity = glm::vec3{ q1.velocity->value(), q2.velocity->value(), 0 };
                    line_renderer.render(cam_mat, object_pos, object_pos + glm::normalize(velocity), { 0,1,1,1 });

                    // Current constraint values
                    imgui.render([&] {
                        ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Once);
                        if (ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                        {
                            ImGui::Text(fmt::format("constraint = {:<10.6f} (desired value = {:<10.6f})", constraint->value(), constraint_target->value()).c_str());
                            ImGui::Separator();
                            ImGui::SliderFloat("desired constraint value", &desired_radius, 1.0f, 3.0f);
                            ImGui::SliderFloat("constraint spring coef", &constraint_coef, 0.0f, 10.0f);
                            ImGui::SliderFloat("constraint spring damping", &constraint_damping, 0.0f, 1.0f);
                            ImGui::SliderFloat("gravity", &gravity, 0.0f, 10.0f);
                            *l.get() = desired_radius;
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
} // namespace scene2
