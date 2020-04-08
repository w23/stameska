uniform sampler2D testure;

float w(vec3 p) {
	return min(p.y, length(p-vec3(0.,1.,0.)) - (#float radius + 1.));
}
void main() {
	vec2 uv = gl_FragCoord.xy / #vec2 R * 2. - 1.; uv.x *= R.x / R.y;
	vec3 color = vec3(0.);
	vec3 O = vec3(0., 1., 4. + 2.);
	vec3 D = normalize(vec3(uv, -2.));
	float l = 0.;
	vec3 p;
	for (int i = 0; i < 100; ++i) {
		p = O + D * l;
		float d = w(p);
		l += d;
		if (d < .001)
			break;
	}
	color = vec3(1. / l) * texture2D(testure, p.xz).rgb;
	gl_FragColor = vec4(sqrt(color), 1.);
}
