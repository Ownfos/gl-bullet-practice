module;

#include <memory>
#include <fmt/format.h>

export module Value;

export import IExpression;

export namespace ownfos::math
{
    class Value : public IExpression
    {
    public:
        Value(float val = 0, std::string_view name = "")
            : val(val), name(name)
        {}

        Value& operator=(float val)
        {
            this->val = val;

            return *this;
        }

        virtual float value() const override
        {
            return val;
        }

        virtual std::shared_ptr<IExpression> total_derivative(IExpression* expression) const override
        {
            return expression == this ? std::make_shared<Value>(1) : std::make_shared<Value>(0);
        }

        virtual std::shared_ptr<IExpression> partial_derivative(IExpression* expression) const override
        {
            return expression == this ? std::make_shared<Value>(1) : std::make_shared<Value>(0);
        }

        virtual std::string to_string() const override
        {
            return name.empty() ? fmt::format("{}", val) : name;
        }

    private:
        float val;
        std::string name;
    };
} // namespace ownfos::math
