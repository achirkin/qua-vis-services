#version 450
#extension GL_ARB_tessellation_shader : enable

layout(location = 0) in vec3 vPosition[];
layout(location = 1) in vec3 vColor[];

layout (vertices = 3) out;
out vec3 tcPosition[];
out vec3 tcColor[];

#define ID gl_InvocationID

void main()
{
    tcPosition[ID] = vPosition[ID];
    tcColor[ID] = vColor[ID];

    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = 1;
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1;
        gl_TessLevelOuter[2] = 1;
    }
}
