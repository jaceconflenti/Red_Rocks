uniform sampler2D blue;
uniform sampler2D star;
uniform sampler2D cloud;
uniform float mix1;

varying vec3 vertex;

void main()
{
    vec3 V = normalize(vertex);
    vec3 L = normalize(gl_LightSource[0].position.xyz);

    // Look up the sky color and glow colors.

    vec4 Kb = texture2D(blue, vec2((L.y + 1.0) / 2.0, V.y));
    vec4 Kc = texture2D(cloud, vec2(V.x, V.y));
    vec4 Ks = texture2D(star,  vec2(V.x, V.y));

    // Combine the color and glow giving the pixel value.
    vec4 Km = mix(Kb, Kc, 0.5);

    gl_FragColor = mix(Km, Ks, mix1);
}
