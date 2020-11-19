std::string strVertexShader_copy("\
#version 330\n\
out vec4 gl_Position;\n\
layout(location = 0) in vec4 position;\n\
void main()\n\
{\n\
    gl_Position.x = position.x/2.0;\n\
    gl_Position.y = position.y/2.0;\n\
    gl_Position.z = position.z;\n\
    gl_Position.w = position.w;\n\
}\n\
");