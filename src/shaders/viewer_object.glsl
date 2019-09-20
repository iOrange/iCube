R"===(
struct Vertex2Fragment {
    vec3 Normal;
};

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 inPos;

uniform mat4 gProj;
uniform mat4 gModel;

out Vertex2Fragment v2f;

void main() {
    vec4 vPos = vec4(inPos, 1.0);

    v2f.Normal = (gModel * vec4(normalize(vPos.xyz), 0.0)).xyz;

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
