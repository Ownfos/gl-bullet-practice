module;

#include <memory>
#include <fmt/format.h>

export module Subtract;

export import Value;

export namespace ownfos::math
{
    class Subtract : public IExpression
    {
    public:
        Subtract(std::shared_ptr<IExpression> lhs, std::shared_ptr<IExpression> rhs)
            : lhs(lhs), rhs(rhs)
        {}

        virtual float value() const override
        {
            return lhs->value() - rhs->value();
        }

        virtual std::shared_ptr<IExpression> total_derivative(IExpression* expression) const override
        {
            return std::make_shared<Subtract>(lhs->total_derivative(expression), rhs->total_derivative(expression));
        }

        virtual std::shared_ptr<IExpression> partial_derivative(IExpression* expression) const override
        {
            auto result = (lhs.get() == expression) - (rhs.get() == expression);
            return std::make_shared<Value>(result);
        }

        virtual std::string to_string() const override
        {
            return fmt::format("({} - {})", lhs->to_string(), rhs->to_string());
        }

    private:
        std::shared_ptr<IExpression> lhs;
        std::shared_ptr<IExpression> rhs;
    };

    std::shared_ptr<Subtract> operator-(std::shared_ptr<IExpression> lhs, std::shared_ptr<IExpression> rhs)
    {
        return std::make_shared<Subtract>(lhs, rhs);
    }
} // namespace ownfos::math
