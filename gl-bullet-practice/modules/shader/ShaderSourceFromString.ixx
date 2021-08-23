export module ShaderSourceFromString;

export import IShaderSource;

export namespace ownfos::opengl
{
    // Provide string value as source code
    class ShaderSourceFromString : public IShaderSource
    {
    public:
        ShaderSourceFromString(std::string_view shader_code)
            : shader_code(shader_code)
        {}

        std::string_view get_shader_code() const override
        {
            return shader_code;
        }

    private:
        std::string_view shader_code;
    };
} // namespace ownfos::opengl
