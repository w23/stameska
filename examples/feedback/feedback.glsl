uniform sampler2D prev;
void main() {
	vec2 uv = gl_FragCoord.xy / #vec2 R;
	vec2 auv = uv * 2. - 1.; auv.x *= R.x / R.y;
	vec3 color = vec3(step(length(auv-.5*vec2(sin(#float t*.1), cos(t*.1))), .1));

	uv += vec2(.01*sin(20.*uv.yx));
	color.r += .97 * texture2D(prev, uv).r;
	uv += vec2(.005*sin(21.*uv.yx));
	color.g += .97 * texture2D(prev, uv).g;
	uv += vec2(.0025*sin(22.*uv.yx));
	color.b += .97 * texture2D(prev, uv).b;

	gl_FragColor = vec4(color, 1.);
}
