export module IBufferData;

export namespace ownfos::opengl
{
    class IBufferData
    {
    public:
        virtual void bind_to(unsigned int buffer, unsigned int usage) const = 0;
    };
} // namespace ownfos::opengl
