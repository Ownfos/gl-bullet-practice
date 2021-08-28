export module IBuffer;

export namespace ownfos::opengl
{
    class IBuffer
    {
    public:
        virtual void bind() const = 0;
    };
} // namespace ownfos::opengl
