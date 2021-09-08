module;

#include <fmt/format.h>
#include <glm/glm.hpp>
#include <bullet/LinearMath/btVector3.h>

export module FormatMathTypes;

export namespace ownfos::utility
{
    std::string format(const btVector3& v)
    {
        return fmt::format("({:6.2f}, {:6.2f}, {:6.2f})", v.x(), v.y(), v.z());
    }

    std::string format(const glm::vec3& v)
    {
        return fmt::format("({:6.2f}, {:6.2f}, {:6.2f})", v.x, v.y, v.z);
    }

    std::string format(const glm::vec4& v)
    {
        return fmt::format("({:6.2f}, {:6.2f}, {:6.2f}, {:6.2f})", v.x, v.y, v.z, v.w);
    }
} // namespace ownfos::utility
