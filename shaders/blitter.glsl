uniform vec2 RES;
uniform sampler2D F;

void main() {
	vec2 uv = (gl_FragCoord.xy - .5) / RES;
	gl_FragColor = sqrt(texture2D(F, uv) * (.5 + .5 * mod(gl_FragCoord.x, 2.)));
}
