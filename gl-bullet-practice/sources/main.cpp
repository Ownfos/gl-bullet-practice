#include <memory>
#include <iostream>
#include <vector>
#include <fmt/format.h>

import Scene1;
import Scene2;
import Scene3;
import Scene4;
import Scene5;

import Multiply;
import Add;
import Subtract;
import Value;

using namespace ownfos::math;

class TimeDependent : public Value
{
public:
    TimeDependent(std::string_view name, float val, std::shared_ptr<Value> time, std::shared_ptr<TimeDependent> time_derivative)
        : Value(val, name), time(time), time_derivative(time_derivative)
    {}

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

    GeneralizedCoordinate(std::string_view name, float position_value, float velocity_value, std::shared_ptr<Value> time)
    {
        acceleration = std::make_shared<TimeDependent>(fmt::format("d^2({})/dt^2", name), 0, time, nullptr);
        velocity = std::make_shared<TimeDependent>(fmt::format("d({})/dt", name), velocity_value, time, acceleration);
        position = std::make_shared<TimeDependent>(name, position_value, time, velocity);
    }
};

int main()
{
    //scene1::run();
    //scene2::run();
    //scene3::run();
    //scene4::run();
    //scene5::run();

    auto time = std::make_shared<Value>();

    // A system with 2 degrees of freedom (two linear position or one planar position)
    auto q1 = GeneralizedCoordinate("q1", 5, 3, time);
    auto q2 = GeneralizedCoordinate("q2", 1, 2, time);
    auto system = std::vector{&q1, &q2};

    // A circular constraint with radius = k
    auto k = std::make_shared<Value>(5);
    auto constraint = q1.position * q1.position + q2.position * q2.position - k * k;

    // First and second order time derivative of constraint
    auto cdot = constraint->total_derivative(time.get());
    auto cdotdot = cdot->total_derivative(time.get());

    // Alternative way to calculate second order time derivative of constraint using only positions
    std::shared_ptr<IExpression> cdotdotalt = std::make_shared<Value>(0);
    for (auto q : system)
    {
        cdotdotalt = cdotdotalt + cdot->total_derivative(q->position.get()) * q->velocity + cdot->total_derivative(q->velocity.get()) * q->acceleration;
    }

    // Gradient of constraint
    auto c1 = constraint->total_derivative(q1.position.get());
    auto c2 = constraint->total_derivative(q2.position.get());

    std::cout << fmt::format("d(constraint)/dt = {} = {}\n", cdot->to_string(), cdot->value());
    std::cout << fmt::format("d^2(constraint)/dt^2 = {} = {} (should be equal to {})\n", cdotdot->to_string(), cdotdot->value(), cdotdotalt->value());
    std::cout << fmt::format("d(constraint)/d(q1) = {} = {}\n", c1->to_string(), c1->value());
    std::cout << fmt::format("d(constraint)/d(q2) = {} = {}\n", c2->to_string(), c2->value());
}