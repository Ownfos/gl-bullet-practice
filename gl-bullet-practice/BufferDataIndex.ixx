module;

#include <gl/glew.h>

export module BufferDataIndex;

export import IBufferData;
export import std.core;

export namespace ownfos::opengl
{
    class BufferDataIndex : public IBufferData
    {
    public:
        BufferDataIndex() = default;
        BufferDataIndex(BufferDataIndex&&) = default;
        BufferDataIndex(const BufferDataIndex&) = delete;

        BufferDataIndex(std::vector<unsigned int>&& indices)
            : indices(std::move(indices))
        {}

        BufferDataIndex& operator=(BufferDataIndex&&) = default;
        BufferDataIndex& operator=(const BufferDataIndex&) = delete;

        ~BufferDataIndex() = default;

        void bind_to(unsigned int buffer, unsigned int usage) const override
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], usage);
        }

    private:
        std::vector<unsigned int> indices;
    };
} // namespace ownfos::opengl
