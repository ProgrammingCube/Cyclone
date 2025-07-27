#version 330 core

// 'out' is the modern way to declare an output variable for the shader
out vec4 FragColor;

void main()
{
    // We write our final color to our output variable
    FragColor = vec4(0.2f, 0.6f, 1.0f, 1.0f); // A nice blue color
}