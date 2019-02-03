uniform sampler2D prev;
void main() {
	vec2 uv = gl_FragCoord.xy / #vec2 R;
	vec2 auv = uv * 2. - 1.; auv.x *= R.x / R.y;
	vec3 color = vec3(step(length(auv-.5*vec2(sin(#float t*.1), cos(t*.1))), .1));

	vec2 cuv;
	cuv = (uv - .5 - .001 * normalize(uv-.5) + vec2(.001*sin(20.*uv.yx))) + .5;
	color.r += .97 * texture2D(prev, cuv).r;
	cuv = (uv - .5 - .0011 * normalize(uv-.5) + vec2(.0011*sin(21.*uv.yx))) + .5;
	color.g += .97 * texture2D(prev, cuv).g;
	cuv = (uv - .5 - .0009 * normalize(uv-.5) + vec2(.0012*sin(22.*uv.yx))) + .5;
	color.b += .97 * texture2D(prev, cuv).b;

	gl_FragColor = vec4(color, 1.);
}
