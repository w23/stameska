uniform vec2 R;
uniform float t;

const vec3 E = vec3(0., .001, 1.);

float vmax(vec3 v) { return max(v.x, max(v.y, v.z)); }
float box3(vec3 p, vec3 s) { return vmax(abs(p) - s); }

mat3 RX(float a) { float c=cos(a),s=sin(a); return mat3(1., 0., 0., 0., c, s, 0., -s, c); }
mat3 RY(float a) { float c=cos(a),s=sin(a); return mat3(c, 0., s, 0., 1., 0., -s, 0., c); }

float texp = fract(t*1e-2);
mat3 brot = RY(t * 43e-3) * RX(t * 3e-2);
mat3 o1r = RX(2.1 + texp*.5);//t*1e-2);

float kwobj1 = 1.;
float kwobj2 = 1.;
float wgnd, wobj1, wobj2;
float w(vec3 p) {
	//return min(p.y, length(p) - 1.);

	wgnd = p.y;

	wobj1 = 1e6;

	vec3 pp = p;
	pp.y -= 1.;
	pp.z += 3.;
	float ws1 = box3(brot * pp, vec3(.6));//length(p) - 1.;
	float ws2 = length(pp - vec3(1.)) - 1.4;
	wobj2 = max(ws1, -ws2);
	wobj2 = length(pp) - 2.;
	wobj2 *= kwobj2;

	float k = 1.;
	for (int i = 0; i < 9; ++i) {
		p.xy = abs(p.yx);
		p.y -= texp * 5.3 * k;
		p *= o1r;
		k *= .9;
		//p.xz = abs(p.zy);
		p.z = abs(p.z);
		wobj1 = min(wobj1, box3(p, vec3(.3, .8, .1) * k));
	}
	wobj1 *= kwobj1;

	return min(min(wobj1, wobj2), wgnd);
}

vec3 wn(vec3 p) {
	return normalize(vec3(
		w(p + E.yxx),
		w(p + E.xyx),
		w(p + E.xxy)) - w(p));
}

float tr(vec3 o, vec3 d, float l, float L) {
	for (int i = 0; i < 50; ++i) {
		vec3 p = o + d * l;
		float d = w(p);
		l += d;
		if (d < .001 * l) break;
	}
	return l;
}

vec3 O, D, P, N;
vec4 mdiffuse, mspecular;

vec3 ldir(vec3 ld, vec3 lc, float L) {
	float shadowk = step(L, tr(P, ld, .01, L));
	vec3 lv = ld + D;
	return shadowk * mix(
		pow(max(dot(normalize(ld-D), N), 0.), mspecular.w) * mspecular.rgb,
		max(dot(ld, N), 0.) * mdiffuse.rgb, mdiffuse.w) * lc;
}

vec3 lpos(vec3 lp, vec3 lc) {
	vec3 ld = lp - P;
	return ldir(normalize(ld), lc, length(ld)) / dot(ld, ld);
}

void main() {
	vec2 uv = gl_FragCoord.xy / R - .5;
	uv.x *= R.x / R.y;

	O = vec3(0., 2., 9.);
	D = normalize(vec3(uv, -2.));
	D *= RX(.08);
	mat3 cr = RY(t*2e-2);
	O *= cr;
	D *= cr;

	vec3 col = vec3(0.);

	vec3 kc = vec3(1.);
	float d;
	for (int j = 0; j < 5; ++j) {
		const float L = 20.;
		float l = 0.;
		for (int i = 0; i < 100; ++i) {
			P = O + D * l;
			d = w(P);
			l += d;
			if (d < .001 * l) break;
		}

		if (l >= L)
			break;

		bool refr = d == wobj1;

		N = wn(P);

		mdiffuse = vec4(1., 1., 1., .01);
		mspecular = vec4(1., 1., 1., 1e3);

		if (wgnd < wobj1 && wgnd < wobj2) {
			mdiffuse.rgb = vec3(.1);
			mdiffuse.w = 1.;
		}

		vec3 c = vec3(.01, .02, .025) * mdiffuse.rgb;

		vec3 sd = normalize(vec3(.1, 1., .2));
		c += ldir(sd, vec3(1.), 10.);
		c += lpos(vec3(0., .1 + 1. * texp, 0.), 10. * vec3(1., .6, .1));
		//c += ldir(normalize(vec3(-.4, .9, .2)), vec3(.6, .8, 1.), 10.);

		col += kc * c;

		if (refr) {
			//col.r = 1.; break;
			kwobj1 *= -1.;
			O = P - N * .02;
			D = refract(D, N, .95);
			//O = P + D * .01;
		} else {
			D = reflect(D, N);
			O = P + D * .01;
			kc *= (1. - mdiffuse.w);
		}
	}

	col = sqrt(col);
	gl_FragColor = vec4(col, 0.);
}
