R"===(
struct Vertex2Fragment {
    vec3 Normal;
};

#ifdef VERTEX_SHADER

uniform mat4 gProj;
uniform mat4 gModel;

out Vertex2Fragment v2f;

const vec3 CubeVertices[36] = vec3[](
    /* +X */
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    /* -X */
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0, -1.0, -1.0),
    /* +Y */
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    /* -Y */
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    /* +Z */
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    /* -Z */
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0)
);

void main() {
    vec4 vPos = vec4(CubeVertices[gl_VertexID], 1.0);

    v2f.Normal = vPos.xyz;

    gl_Position = (gProj * gModel) * vPos;
}

#endif

#ifdef FRAGMENT_SHADER

uniform samplerCube tCubeMap;

in Vertex2Fragment v2f;

out vec4 Target0;

void main() {
    vec3 normal = normalize(v2f.Normal);
    vec3 cubeColour = texture(tCubeMap, normal).xyz;

    Target0 = vec4(cubeColour, 1.0);
}

#endif
)==="
