std::string strVertexShader_copy("\
#version 330\n\
out vec4 gl_Position;\n\
layout(location = 0) in vec4 position;\n\
void main()\n\
{\n\
    gl_Position = position;\n\
}\n\
");