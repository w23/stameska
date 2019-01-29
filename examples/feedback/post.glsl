uniform sampler2D frame;

void main() {
	vec2 uv = gl_FragCoord.xy / #vec2 R;
	gl_FragColor = sqrt(texture2D(frame, uv));
	/*
	//uv.x *= #vec2 R.x / #vec2 R.y;
	uv.x += .01 * sin(floor(60.*uv.y) + #float t);
	vec2 cuv = uv - .5;
	float r = length(cuv);
	gl_FragColor = vec4(
		texture2D(frame, uv + cuv*r*.02).r,
		texture2D(frame, uv + cuv*r*.04).g,
		texture2D(frame, uv + cuv*r*.06).b, 1.);
	*/
}
