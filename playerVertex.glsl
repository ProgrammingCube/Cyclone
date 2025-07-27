#version 330 core

// 'in' is the modern replacement for 'attribute'
// We use a standard naming convention: 'a' for attribute
in vec3 aPos;

// These are the matrices we will get from our C code
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    // The final position is the result of transforming the object's local
    // position (aPos) by the Model, View, and Projection matrices.
    // The multiplication order is important: P * V * M
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);
}