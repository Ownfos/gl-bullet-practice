module;

#include <memory>
#include <string>

export module IExpression;

export namespace ownfos::math
{
    class IExpression
    {
    public:
        virtual ~IExpression() = default;

        virtual float value() const = 0;
        virtual std::shared_ptr<IExpression> total_derivative(IExpression* expression) const = 0;
        virtual std::shared_ptr<IExpression> partial_derivative(IExpression* expression) const = 0;
        virtual std::string to_string() const = 0;
    };
} // namespace ownfos::math
