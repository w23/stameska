#version 130
uniform vec2 R;
//uniform float t;
uniform sampler2D T;
uniform int s;
float t = float(s)/88200.;
//vec2 R = vec2(640., 480.);

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
	v *= v * (3. - 2. * v);
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
float box3(vec3 p, vec3 s) { p = abs(p) - s; return max(p.z, max(p.x, p.y)); }

//float ball(vec3 p, float r) { return length(p) - r; }

/*
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
*/


vec3 rep3(vec3 p, vec3 r) { return mod(p,r) - r*.5; }
vec2 rep2(vec2 p, vec2 r) { return mod(p,r) - r*.5; }
/*
float ring(vec3 p, float r, float R, float t) {
	return max(abs(p.y)-t, max(length(p) - R, r - length(p)));
}
*/

//float vmax2(vec2 p) { return max(p.x, p.y); }
//float vmax3(vec3 v) { return max(v.x, max(v.y, v.z)); }
//float box(vec3 p, vec3 s) { return vmax3(abs(p) - s);}
//float tor(vec3 p, vec2 s) { return length(vec2(length(p.xz) - s.x, p.y)) - s.y; }

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

/*
float metamin(float a, float b, float r) {
		return max(r, min (a, b)) - length(max(vec2(r - a,r - b), vec2(0)));
}
*/

//float sqr(vec3 p) { return dot(p, p); }

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

int sdf_scene = 7;
float flr;
float w(vec3 p) {
	if (sdf_scene == 3) {
		//float flr = p.
		float walls = -box3(p-vec3(0.,1.8,0.), vec3(3., 1.8, 30.));
		vec3 p1 = p;
		p1.z = mod(p.z, 10.) - 5.;
		p1.x -= 3.;
		p1.y += .3 - 1.8;
		walls = max(walls, -box3(p1, vec3(2., 1.2, 1.)));
		walls = min(walls, box3(p, vec3(.3, .1, 30.)));
		walls = max(walls, p.x - 4.4);
		return walls;
	} else if (sdf_scene == 1) {
		flr = p.y; //if (flr < .1) flr += .2 * noise2(p.xz);
		float walls = min(p.x + 4., 6. - p.y);
		vec3 p1 = p;
		p1.z = mod(p.z, 10.) - 5.;
		float holes = box3(p1, vec3(1., 10., 1.));
		p1.x -= 4.;
		p1.y += .3 - 1.8;
		float columns = box3(p1, vec3(.5, 10., .5));
		walls = min(walls, columns);
		//walls = min(walls, p.z + 30.);
		walls = min(walls, box3(p-vec3(5., 0., 0.), vec3(1., 1., 100.)));
		walls = max(walls, -min(holes, 6.4 - p.y));
		return min(max(min(flr, walls), p.x - 6.), p.z + 30.);
	} else if (sdf_scene == 7) {
		flr = p.y;
		float walls = p.z + 30.; //max(box3(p, vec3(6.)), -box3(p, vec3(5.)));
		//walls = max(walls, -box3(p, vec3(1., 100., 1.)));

		p.x = abs(p.x);
		walls = min(walls, 10. - p.x);
		walls = min(walls, box3(p-vec3(14.,12.,-20.), vec3(8., 4., 10.)));
		return min(flr, walls);
	}
	return min(p.y + 1., length(p) - 1.);
}

vec3 wn(vec3 p) {
	return normalize(vec3(
	/* gives NaNs a lot
		w(p + E.yxx),
		w(p + E.xyx),
		w(p + E.xxy)) - w(p));
	*/
		w(p + E.yxx) - w(p - E.yxx),
		w(p + E.xyx) - w(p - E.xyx),
		w(p + E.xxy) - w(p - E.xxy)));
}

//float pixel_size = 1. / R.x;
float march(vec3 o, vec3 d, float l, float L, int steps) {
	for (int i = 0; i < steps; ++i) {
		float dd = w(o + d * l);
		l += dd;
		if (dd < 0.002 * l || l > L) break;
	}
	return l;
}

vec3 O, D, N;

/*
float shine, kd;

float dirlight(vec3 dir, float shine, float kd) {
	return mix(
		max(0., dot(N, dir)) / 3.,
		pow(max(0., dot(N, normalize(dir - D))), shine) * (shine + 8.) / 24.,
		kd
	);
}
*/

float tex1(vec2 p) {
 return fbm(p * 16. + 8. * (vec2(fbm(p*4.), fbm(p*4.+96.)) - .5));
}
/*
vec3 tex2(vec2 p) {
 return mix(vec3(.9,.4,.7), vec3(.2, .7,.9), tex1(p));
}
*/
/*
vec3 tex3(vec2 p) {
	float c, f = t64;
	vec2 s1,s2;
	s1=4.*vec2(sin(-f),cos(-f*4.));
	s2=7.*vec2(sin(f*3.),cos(f*7.));
	c=sin(3.*p.x+s1.x)*sin(4.*p.y+s1.y)
	+sin(7.*p.x+s2.x)*sin(2.*p.y+s2.y);
	return vec3(1.-c);//sin(2.*(c+sqrt(c/4.))),c/4.+.5,log2(c)+exp(c));
}*/

/*
float shadow(vec3 o, vec3 d, float L, int steps) {
	float l = march(o, d, .01, L, steps);
	return step(L, l);
}

vec3 drawSDFScene(vec3 sample_color, vec2 uv) {
	//O = vec3(-.5, 1.8, 5.);
	//D = normalize(vec3(uv, -2.));
	vec3 k = vec3(1.);
	for (int i = 0; i < 1; ++i) {
		float l = march(O, D, 0., 40., 80);
		if (l > 40.)
			break;
		if (i == 0)
			sample_color = vec3(0.);
		vec3 p = O+D*l;
		vec3 emissive = vec3(0.);
		vec3 diffuse = vec3(1.);
		N = wn(p);
		shine = 100.;
		kd = 0.;

		//return N;

		vec2 uv = p.xz;
		//emissive = .1 * step(.9,mod(p,1.));
		//diffuse = mod(p,1.);
		//diffuse = vec3(tex1(uv));
		//diffuse = N;
		*/

		/*
		if (sdf_scene == 1) {
			diffuse = mix(vec3(1.,.2,.3), vec3(0.), step(d1, d2));
			//kd = mix(.5, 0., step(d1, d2));
			emissive = vec3(step(d1, d2));
		} else
		if (sdf_scene == 2) {
			diffuse = mix(vec3(1.,.2,.3), vec3(tex1(p.xz/6.)), step(d1, d2));
		} else
		if (sdf_scene == 4 || sdf_scene == 0) {
			diffuse = vec3(1.,.2,.3);
			N = wn((floor(p*4.)+.5)/4.);
		} else
		if (sdf_scene == 5) {
			diffuse = mix(vec3(1.,.2,.3), vec3(1.), step(d1, d2));
			//diffuse = mix(vec3(.9, .4, .1), vec3(.1,.2,.9), step(d1, d2));
		} else
		if (sdf_scene == 6) {
			diffuse = mix(vec3(1.,.2,.3), vec3(1.), tex1(tuv/4.));
			kd = .7;
			shine = 80.;
			//diffuse = mix(vec3(.9,.4,.7), vec3(.2, .7,.9), tex1(tuv/4.));
		}
		*/
		//n = wn(RZ(-t/16.)*floor(RZ(t/16.)*p*4.));
		//n = wn(sign(p)*floor(p*4.)/4.);
		//vec3 ld = -D;//normalize(vec3(1.));

/*
		vec3 ld;
		for (int j = 0; j < 3; ++j) {
			float z = float(j) * 10.;
			ld = vec3(4., 3., -5. - z) - p;
			sample_color += k * diffuse * 10.
				* shadow(p, normalize(ld), length(ld), 40)
				* dirlight(normalize(ld), shine, kd)/dot(ld,ld);
			ld = vec3(-1., 2., -5. - z) - p;
			sample_color += k * diffuse * 1.
				* shadow(p, normalize(ld), length(ld), 40)
				* dirlight(normalize(ld), shine, kd)/dot(ld,ld);
		}

		sample_color += k * emissive;
		//if (sdf_scene != 2) break;

		O = p + N * .01;
		D = reflect(D,N);
		k *= diffuse * kd;
	}
	return sample_color;
}
 */

void main() {
	vec2 uv = (gl_FragCoord.xy / R * 2. - 1.); uv.x *= 2. * R.x / R.y;
	//gl_FragColor = vec4(uv, 0., 1.); return;

	O = vec3(-.5+.5*sin(t/16.), 1.8, 5.);
	D = normalize(vec3(uv, -2.));
	//vec3 D = normalize(vec3(uv, -2.));
	vec3 D2 = normalize(vec3(uv+vec2(2.,1.)/R, -2.));

	//gl_FragColor = vec4(drawSDFScene(vec3(0.), uv), 1.); return;

	const int samples_per_pixel = 16;
	float seedhash = t;
	vec3 total_color = vec3(0.);
	for (int s = 0; s < samples_per_pixel; ++s) {
		vec3 o = O;
		vec3 d = mix(D, D2, vec3(hash1(seedhash), hash1(seedhash+1.), hash1(seedhash+2.)));

		vec3 sample_color = vec3(0.);
		vec3 k = vec3(1.);
		for (int b = 0; b < 3; ++b) {
			//if (any(isnan(d))) { lolnan = true; break; }
			float l = 60.;//escape(o, d, Ra);
			//if (isnan(lsky)) break;
			//if (isnan(lsky)) lolnan = true;
			int mat = 0;
			//if (isnan(l)) break;

			float L = min(l, 50.);
			float ldf = march(o, d, 0., L, 40);
			//if (isnan(l)) {lolnan = true; break; }
			if (ldf < L) {
				l = ldf;
				mat = 2;
			}

			vec3 emissive = vec3(0.);
			vec3 albedo = vec3(.75, .75, .73);
			vec3 n = vec3(0., 1., 0.);
			float roughness = .04;
			roughness = .9;//.04;

			if (mat == 0) {
				emissive = vec3(1.5);//*dot(d,vec3(0.,1.,0.)));
				albedo = vec3(0.);
				o = o + d * l;
			} else {
				o = o + d * l;
				n = wn(o);
				o += n * .01;
				//albedo = vec3(tex1(o.xz/4.));
				////roughness = smoothstep(.3, .6,tex1(o.xz/4.));
				//roughness = .01 + .8*smoothstep(.3, .6, fbm(o.xz));
			}

			//vec3 fog = vec3(1.);//vec3(1. * exp(-l/8.));
			//vec3 fog = vec3(1.);//vec3(exp(-max(0., length(o)-4.)/2.));
			/*
			sample_color += fog * k * em;
			k *= al * fog;
			*/
			//k *= fog;

			sample_color += k * emissive;
			k *= albedo.rgb;
			//if (s == 0 && b == 0) { d = sundir; k *= max(0., dot(n, sundir)); continue; }
			if (all(lessThan(k,vec3(.001)))) break;

			//gl_FragColor = vec4(n, 1.); return;

			d = reflect(d, n);
			//if (any(isnan(d))) { break; }
			/*
			seed.x += 1. / textureSize(N,0).x;
			vec4 rvect = texture2D(N, seed);//snoise24(vec2(seed,.5));
			d = normalize(mix(d, (rvect.xyz - .5) * 2., roughness));
			*/

			//gl_FragColor = vec4(d, 1.); return;
			//gl_FragColor = vec4(hash1(uv.x), 0., 0., 1.); return;
			//gl_FragColor = vec4(hash1(t), 0., 0., 1.); return;
	/*		vec3 rvect =
				vec3(
							hash1(seedhash += d.x),
							hash1(seedhash += d.y),
							hash1(seedhash += d.z))
				;*/

			//gl_FragColor = vec4(rvect + 1., 1.); return;

			d = normalize(mix(d, (
				vec3(
							hash1(seedhash += d.x),
							hash1(seedhash += d.y),
							hash1(seedhash += d.z))
							- .5) * 2.,
					roughness));

			d *= sign(dot(n, d));
			//float s = sign(dot(n, d)); if (s < .0) { gl_FragColor = vec4(1., 0., 0., 1.); return; }
		}

		//if (lolnan) { gl_FragColor = vec4(0., 1000., 0., 1.); return; }

		total_color += sample_color;
	}

	//if (lolnan) { gl_FragColor = vec4(0., 0., 1000., 1.); return; }

	//total_color /= float(samples_per_pixel);

	//gl_FragColor = vec4(total_color /* * smoothstep(0., 4., t)*/, mix(.1, 1., step(t,1.)));//.4);
	//float alpha = .15;
	//alpha = .3;
	//alpha = 1.;
	gl_FragColor = vec4(total_color/float(samples_per_pixel), .12); // /* * smoothstep(0., 4., t)*/, mix(alpha, 1., step(t,1.)));//.4);
}
