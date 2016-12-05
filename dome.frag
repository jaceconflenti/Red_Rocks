uniform sampler2D blue;
uniform sampler2D star;
uniform sampler2D glow;
uniform float mix;
uniform float mix2;

varying vec3 vertex;

void main()
{
    vec3 V = normalize(vertex);
    vec3 L = normalize(gl_LightSource[0].position.xyz);

    // Compute the proximity of this fragment to the sun.

    float vl = dot(V, L);

    // Look up the sky color and glow colors.

    vec4 Kc = texture2D(blue, vec2((L.y + 1.0) / 2.0, V.y));
    vec4 Kg = texture2D(star,  vec2(V.x, V.y));
    vec4 Ko = texture2D(glow, vec2((L.y + 1.0) / 2.0, vl));

    // Combine the color and glow giving the pixel value.

    vec4 Km = mix(Kc, Kg, mix);

    gl_FragColor = mix(Km, Ko, mix2);
}
