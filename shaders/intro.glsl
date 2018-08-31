#version 130
uniform vec2 RES;
uniform float t;
//uniform sampler2D N;

//const float INF = 10000.;
const vec3 E = vec3(0., .001, 1.);
const float PI = 3.141593;

float t8 = t/8.;
float t16 = t/16.;
float t32 = t/32.;
float t64 = t/64.;

float hash1(float v) { return fract(sin(v) * 43758.5453); }
float hash2(vec2 v) { return hash1(dot(v, vec2(129.47, 37.56))); }
//float hash3(vec3 v) { return hash1(dot(v, vec3(17.343,129.47, 37.56))); }
//vec3 hash13(float v) { return vec3(hash1(v), hash1(v+17.), hash1(v+32.)); }
//float hash3(vec3 v) { return hash1(dot(v, vec3(129.47, 37.56, 1.))); }
//float noise1(float v) { float V = floor(v); v = fract(v); return mix(hash1(V), hash1(V+1.), v); }
float noise2(vec2 v) {
  vec2 V = floor(v); v = fract(v);
  return mix(
    mix(hash2(V), hash2(V+E.zx), v.x),
    mix(hash2(V+E.xz), hash2(V+E.zz), v.x), v.y);
}
float fbm(vec2 f) {
  return
    .5 * noise2(f)
    + .25 * noise2(f*2.3)
    + .125 * noise2(f*4.1)
    //+ .0625 * noise2(f*7.2)
  ;
}
/*
float noise3(vec3 v) {
  vec3 V = floor(v); v = fract(v);
  return mix(
			mix(
				mix(hash3(V+E.xxx), hash3(V+E.zxx), v.x),
				mix(hash3(V+E.xzx), hash3(V+E.zzx), v.x), v.y),
			mix(
				mix(hash3(V+E.xxz), hash3(V+E.zxz), v.x),
				mix(hash3(V+E.xzz), hash3(V+E.zzz), v.x), v.y), v.z);
}*/

/*
vec4 sN(vec2 c) { return texture2D(N, c); }
vec4 snoise24(vec2 c) { return texture2D(N, (c+.5)/textureSize(N,0)); }
*/

float box2(vec2 p, vec2 s) { p = abs(p) - s; return max(p.x, p.y); }
float box3(vec3 p, vec3 s) { p = abs(p) - s; return max(p.x, max(p.y, p.z)); }

//float ball(vec3 p, float r) { return length(p) - r; }

float pModPolar(inout vec2 p, float repetitions) {
	float angle = 2*PI/repetitions;
	float a = atan(p.y, p.x) + angle/2.;
	float r = length(p);
	float c = floor(a/angle);
	a = mod(a,angle) - angle/2.;
	p = vec2(cos(a), sin(a))*r;
	// For an odd number of repetitions, fix cell index of the cell in -x direction
	// (cell index would be e.g. -5 and 5 in the two halves of the cell):
	if (abs(c) >= (repetitions/2)) c = abs(c);
	return c;
}


vec3 rep3(vec3 p, vec3 r) { return mod(p,r) - r*.5; }
vec2 rep2(vec2 p, vec2 r) { return mod(p,r) - r*.5; }
float ring(vec3 p, float r, float R, float t) {
	return max(abs(p.y)-t, max(length(p) - R, r - length(p)));
}

//float vmax2(vec2 p) { return max(p.x, p.y); }
//float vmax3(vec3 v) { return max(v.x, max(v.y, v.z)); }
//float box(vec3 p, vec3 s) { return vmax3(abs(p) - s);}
float tor(vec3 p, vec2 s) { return length(vec2(length(p.xz) - s.x, p.y)) - s.y; }

mat3 RX(float a){ float s=sin(a),c=cos(a); return mat3(1.,0.,0.,0.,c,-s,0.,s,c); }
mat3 RY(float a){	float s=sin(a),c=cos(a); return mat3(c,0.,s,0.,1.,0,-s,0.,c); }
mat3 RZ(float a){ float s=sin(a),c=cos(a); return mat3(c,s,0.,-s,c,0.,0.,0.,1.); }

/*
float raySphere(vec3 o, vec3 d, vec3 c, float r, float L) {
	vec3 l = c - o;
	float tca = dot(l, d);
	if (tca < 0.) return L;
	float d2 = dot(l, l) - tca * tca;
	float r2 = r * r;
	if (d2 < 0. || d2 > r2) return L;
	return tca - sqrt(r2 - d2);
}
*/

float metamin(float a, float b, float r) {
		return max(r, min (a, b)) - length(max(vec2(r - a,r - b), vec2(0)));
}

float sqr(vec3 p) { return dot(p, p); }

/*
mat3 lookat(vec3 o, vec3 at, vec3 up) {
	vec3 z = normalize(o - at);
	vec3 x = normalize(cross(up, z));
	up = normalize(cross(z, x));
	return mat3(x, up, z);
}
*/

/*
float fOpIntersectionRound(float a, float b, float r) {
	vec2 u = max(vec2(r + a,r + b), vec2(0));
	return min(-r, max (a, b)) + length(u);
}
*/

float box3w(vec3 p, vec3 s) {
	return max(box3(p,s), -min(min(
		box3(p,s-vec3(-.1, .1, .1)),
		box3(p,s-vec3(.1, -.1, .1))),
		box3(p,s-vec3(.1, .1, -.1)))
		);
}

int sdf_scene = 2;
float w(vec3 p) {
	if (sdf_scene == 1) {
		return min(
			box3w(RZ(t/10.)*RY(t/20.)*(p+1.5*E.zxx*sin(t32)), vec3(1.)),
			box3w(RZ(t/17.)*RY(t/23.)*(p-1.5*E.zxx*sin(t32)), vec3(1.)));
	} else
	if (sdf_scene == 2) {
		return max(
			box3(RZ(t/10.)*RY(t/20.)*(p+1.5*E.zxx*sin(t32)), vec3(.9)),
			box3(RZ(t/17.)*RY(t/23.)*(p-1.5*E.zxx*sin(t32)), vec3(.9)));
	} else
	if (sdf_scene == 3) {
		return length(p) - 1.;
	} else
	if (sdf_scene == 4) {
		float d = 0.;
		for (int i = 0; i < 4; ++i) {
			float a = float(i)*4.;
			vec3 b = p - 1.5*vec3(cos(t16 + a), sin(t8 + a*3.), cos(t32 + a));
			d += .5 / dot(b,b);
		}
		return 1. - d;
	}
	return box3(RZ(t/10.)*RY(t/20.)*p, vec3(1.));
}

vec3 wn(vec3 p) {
	return normalize(vec3(
	//* gives NaNs a lot
		w(p + E.yxx),
		w(p + E.xyx),
		w(p + E.xxy)) - w(p));
	/*/
		w(p + E.yxx) - w(p - E.yxx),
		w(p + E.xyx) - w(p - E.xyx),
		w(p + E.xxy) - w(p - E.xxy)));*/
}

float pixel_size = 1. / RES.x;
float march(vec3 o, vec3 d, float l, float L, int steps) {
	for (int i = 0; i < steps; ++i) {
		float dd = w(o + d * l);
		l += dd;
		if (dd < pixel_size * l || l > L) break;
	}
	return l;
}

float circle(vec2 uv, float r, float R, float n, float a) {
	float uvr = length(uv);
	a += atan(uv.x, uv.y) / PI;
	return step(r, uvr) * step(uvr, R) * step(.5, mod(a * n, 2.));
}

vec3 O, D;

vec3 ambient = vec3(.01);
vec4 drawSDFScene(int index) {
	sdf_scene = index;
	float l = march(O, D, 0., 10., 64);
	if (l < 10.) {
		vec3 p = O+D*l;
		vec3 n = wn(p);
		//n = wn(RZ(-t/16.)*floor(RZ(t/16.)*p*4.));
		//n = wn(sign(p)*floor(p*4.)/4.);
		n = wn((floor(p*4.)+.5)/4.);
		vec3 diffuse = vec3(.9, .3, .1);
		vec3 ld = -D;//normalize(vec3(1.));
		return vec4(ambient + diffuse * max(0., dot(n, ld)), 1.);
	}
	return vec4(0.);
}

void main() {
	//vec2 uv = gl_FragCoord.xy / RES;
	vec2 uv = (gl_FragCoord.xy / RES * 2. - 1.); uv.x *= RES.x / RES.y;

	//gl_FragColor = vec4(mod(gl_FragCoord.x, 2.)); return;
	//gl_FragColor = vec4(1., 0., 0., 1.); return;
	//gl_FragColor = vec4(hash1(uv.x), 0., 0., 1.); return;
	//gl_FragColor = vec4(uv, 0., 1.); return;
	//gl_FragColor = vec4(snoise24((floor(gl_FragCoord.xy) + .5)/textureSize(N, 0).xy).xyz, 1.); return;

	O = vec3(0., 0., 5.);
	D = normalize(vec3(uv, -2.));
	vec3 color = vec3(0.);

	vec2 tuv = uv + vec2(0., -t/64.);
	color = .2 * vec3(fbm(tuv * 16. + 8. * (vec2(fbm(tuv*4.), fbm(tuv*4.+96.)) - .5)));

	vec4 sc5 = drawSDFScene(4);
	color = mix(color, sc5.rgb, sc5.a);

	/*
	vec3 join = drawSDFScene(2);
	color = mix(color, join, step(.001, join.r));
	color += drawSDFScene(1).r * .3;
	*/

	color += vec3(.5) * circle(uv, .4, .45, 8., t16);
	color += vec3(.5) * circle(uv, .5, .55, 64., -t32);
	color += vec3(.5) * circle(uv, .6, .65, 32., t64);

	gl_FragColor = vec4(color, 1.);
}
