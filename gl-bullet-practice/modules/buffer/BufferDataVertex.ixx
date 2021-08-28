module;

#include <gl/glew.h>
#include <vector>
#include <ranges>

export module BufferDataVertex;

export import IBufferData;

export namespace ownfos::opengl
{
    // Represents the layout of a vertex buffer object.
    // Note that single contiguous memory can hold multiple attribute per vertex like below:
    //
    //  std::vector<float>{e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, ...}
    //                     |--------|  |----|  |--------|  |----|
    //                        pos1      tex1      pos2      tex2
    //
    //  |       | count | offset |   type   | stride |
    //  |-------|-------|--------|----------|--------|
    //  |  pos  |   3   |   0    | GL_FLOAT |   20   |
    //  |-------|-------|--------|----------|--------|
    //  |  tex  |   2   |   12   | GL_FLOAT |   20   |
    //  |-------|-------|--------|----------|--------|
    //
    //  stride = 5 * sizeof(float) = 20 bytes
    struct VertexDataAttribute
    {
        int location; // The indices used in vertex shader's layout (e.g. 1 <= layout (location = 1) in vec3 normal;)
        int count; // # of elements to consume per vertex data (e.g. 3 <= vec3 type input)
        int type = GL_FLOAT; // The GLenum value corresponding to the type of each element (e.g. GL_FLOAT <= float)
        int offset = 0; // The byte offset of this attribute (e.g. 12 <= this data starts after 3 float type elements)
        int stride = 0;// # of bytes between each vertex data, where 0 means tight packing.
    };

    template<typename T>
    class BufferDataVertex : public IBufferData
    {
    public:
        BufferDataVertex(const std::vector<T>& data, const std::vector<VertexDataAttribute>& attributes, unsigned int usage = GL_STATIC_DRAW)
            : attributes(attributes)
        {
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(T) * data.size(), &data[0], usage);
        }

        void bind() const override
        {
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            for (const auto& attrib : attributes)
            {
                glVertexAttribPointer(attrib.location, attrib.count, attrib.type, false, attrib.stride, (void*)attrib.offset);
                glEnableVertexAttribArray(attrib.location);
            }
        }

        void update_data(int offset_index, std::span<T> new_data)
        {
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(T) * offset_index, new_data.size_bytes(), new_data.data());
        }

    private:
        std::vector<VertexDataAttribute> attributes;
        unsigned int buffer;
    };
} // namespace ownfos::opengl
