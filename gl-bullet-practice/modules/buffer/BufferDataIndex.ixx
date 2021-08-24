module;

#include <gl/glew.h>
#include <vector>

export module BufferDataIndex;

export import IBufferData;

export namespace ownfos::opengl
{
    class BufferDataIndex : public IBufferData
    {
    public:
        BufferDataIndex(std::vector<unsigned int>&& indices)
            : indices(std::move(indices))
        {}

        void bind_to(unsigned int buffer, unsigned int usage) const override
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], usage);
        }

    private:
        std::vector<unsigned int> indices;
    };
} // namespace ownfos::opengl
