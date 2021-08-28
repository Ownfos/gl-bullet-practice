export module IBufferData;

export namespace ownfos::opengl
{
    class IBufferData
    {
    public:
        virtual void bind() const = 0;
    };
} // namespace ownfos::opengl
