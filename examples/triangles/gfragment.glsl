uniform float t;
varying vec3 v_p;
varying vec3 v_n;
void main() {
	vec2 p = v_p.xz;
	p.y /= 10.;

	float tt = t * .1;
	vec3 n = normalize(vec3(cos(tt*.4), sin(tt*.3), -sin(tt*.5)));
	float f = dot(n, v_p);

	if (mod(f, 1.7) > 1.4) {
		discard;
	}

	vec3 color = vec3(0.);
	vec3 ld = normalize(vec3(1.));
	color = vec3(.5, .66, 1.) * (.1 + max(0., dot(v_n, ld)));

	gl_FragColor = vec4(sqrt(color), 1.);
}
