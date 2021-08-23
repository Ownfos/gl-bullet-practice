export module PrimitiveModelData;

export import BufferDataVertex;
export import BufferDataIndex;

export namespace ownfos::primitives
{
    struct PrimitiveModelData
    {
        opengl::BufferDataVertex<float> position;
        opengl::BufferDataIndex indices;
    };
} // namespace ownfos::opengl
