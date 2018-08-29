#version 130
uniform vec2 RES;
uniform sampler2D F;

void main() {
	//vec2 uv = (gl_FragCoord.xy + .5 ) / RES;
	//gl_FragColor = vec4(uv, 0., 1.);
	//gl_FragColor = sqrt(texture2D(F, uv-vec2(.0,.5)/textureSize(F,0)) * (.5 + .5 * mod(gl_FragCoord.x, 2.)));
	gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/2./textureSize(F,0)));
}
