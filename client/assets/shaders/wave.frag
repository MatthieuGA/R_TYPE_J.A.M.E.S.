// wave.frag
uniform sampler2D texture;  // la texture SFML (sprite, etc.)
uniform float time;         // temps en secondes
uniform float amplitude;    // amplitude de la vague (en UV)
uniform float frequency;    // fréquence de la vague
uniform float speed;        // vitesse de déplacement

void main()
{
    // Coordonnées de la texture envoyées par SFML
    vec2 uv = gl_TexCoord[0].xy;

    // Calcul de la vague : on décale l'UV.x en fonction de y et du temps
    float wave = sin((uv.y * frequency) + time * speed) * amplitude;
    uv.x += wave;

    // Récupération de la couleur de la texture
    vec4 color = texture2D(texture, uv);

    // Multiplication par la couleur du vertex SFML (tinting)
    gl_FragColor = color * gl_Color;
}
