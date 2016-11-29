uniform sampler2D blue;
uniform sampler2D star;

varying vec3 vertex;

void main()
{
    vec3 V = normalize(vertex);
    vec3 L = normalize(gl_LightSource[0].position.xyz);

    // Compute the proximity of this fragment to the sun.

    float vl = dot(V, L);

    // Look up the sky color and glow colors.

    vec4 Kc = texture2D(blue, vec2((L.y + 1.0) / 2.0, V.y));
    vec4 Kg = texture2D(star,  vec2((L.y + 1.0) / 2.0, V.y));

    // Combine the color and glow giving the pixel value.

    gl_FragColor = mix(Kc, Kg, 0.5);
}
