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
        BufferDataIndex(const std::vector<unsigned int>& indices, unsigned int usage = GL_STATIC_DRAW)
        {
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], usage);
        }

        void bind() const override
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        }

    private:
        unsigned int buffer;
    };
} // namespace ownfos::opengl
