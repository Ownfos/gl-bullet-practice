module;

#include <memory>
#include <fmt/format.h>

export module Multiply;

export import Add;

export namespace ownfos::math
{
    class Multiply : public IExpression
    {
    public:
        Multiply(std::shared_ptr<IExpression> lhs, std::shared_ptr<IExpression> rhs)
            : lhs(lhs), rhs(rhs)
        {}

        virtual float value() const override
        {
            return lhs->value() * rhs->value();
        }

        virtual std::shared_ptr<IExpression> total_derivative(IExpression* expression) const override
        {
            return std::make_shared<Multiply>(lhs->total_derivative(expression), rhs) + std::make_shared<Multiply>(rhs->total_derivative(expression), lhs);
        }

        virtual std::shared_ptr<IExpression> partial_derivative(IExpression* expression) const override
        {
            if (lhs.get() == expression && rhs.get() == expression)
            {
                return std::make_shared<Multiply>(lhs, std::make_shared<Value>(2));
            }
            else if (lhs.get() == expression)
            {
                return rhs;
            }
            else if (rhs.get() == expression)
            {
                return lhs;
            }
            else
            {
                return std::make_shared<Value>(0);
            }
        }

        virtual std::string to_string() const override
        {
            return fmt::format("({} * {})", lhs->to_string(), rhs->to_string());
        }

    private:
        std::shared_ptr<IExpression> lhs;
        std::shared_ptr<IExpression> rhs;
    };

    std::shared_ptr<Multiply> operator*(std::shared_ptr<IExpression> lhs, std::shared_ptr<IExpression> rhs)
    {
        return std::make_shared<Multiply>(lhs, rhs);
    }
} // namespace ownfos::math
