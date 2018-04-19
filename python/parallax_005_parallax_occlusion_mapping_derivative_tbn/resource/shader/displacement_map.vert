#version 400

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNV;
layout (location = 2) in vec3 inCol;
layout (location = 3) in vec2 inUV;

out TVertexData
{
    vec3  pos;
    vec3  nv;
    vec3  col;
    vec2  uv;
    float clip;
} out_data;

uniform mat4 u_projectionMat44;
uniform mat4 u_viewMat44;
uniform mat4 u_modelMat44;
uniform vec4 u_clipPlane;

void main()
{
    vec3 modelNV   = mat3( u_modelMat44 ) * normalize( inNV );
    out_data.nv    = mat3( u_viewMat44 ) * modelNV;
    out_data.col   = inCol;
    out_data.uv    = inUV;
    vec4 worldPos  = u_modelMat44 * vec4( inPos, 1.0 );
    vec4 viewPos   = u_viewMat44 * worldPos;
    out_data.pos   = viewPos.xyz / viewPos.w;
    gl_Position    = u_projectionMat44 * viewPos;
    vec4 clipPlane = vec4(normalize(u_clipPlane.xyz), u_clipPlane.w);
    out_data.clip  = dot(worldPos, clipPlane);
}