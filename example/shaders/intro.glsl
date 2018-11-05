//#version 130
//uniform vec2 R;
//uniform float t;
uniform sampler2D T;
uniform int s;
float t = float(s)/88200.;

//const float INF = 10000.;
const vec3 E = vec3(0., .001, 1.);
const float PI = 3.141593;

//float t8 = t/8.;
//float t16 = t/16.;
//float t32 = t/32.;
//float t64 = t/64.;

float saturate(float f) { return min(1., max(0., f)); }

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
}
*/

/*
vec4 sN(vec2 c) { return texture2D(N, c); }
vec4 snoise24(vec2 c) { return texture2D(N, (c+.5)/textureSize(N,0)); }
*/

//float box2(vec2 p, vec2 s) { p = abs(p) - s; return max(p.x, p.y); }
float box3(vec3 p, vec3 s) { p = abs(p) - s; return max(p.z, max(p.x, p.y)); }

float fOpIntersectionChamfer(float a, float b, float r) {
	return max(max(a, b), (a + r + b)*sqrt(0.5));
}
float hexabox(vec3 p, vec3 s) {//, float rxz, float ry) {
	p = abs(p) - s;
	return fOpIntersectionChamfer(fOpIntersectionChamfer(p.x, p.z, 1./*rxz*/), p.y, 1./*ry*/);
}
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

float scenet = t/128., lt = fract(scenet);
int sdf_scene = 1 + int(floor(scenet));
float flr, walls;
mat3 rz2 = RZ(.2);
mat3 rz3 = RZ(.3);
mat3 rz5 = RZ(.5);
float w(vec3 p) {
	if (sdf_scene < 2) {
		flr = p.y;
		walls = -box3(p, vec3(4., 3., 10.));
		float balki = max(-walls, box3(vec3(p.xy-vec2(0., 3.), fract(p.z) - .5), vec3(10., .3, .1)));
		walls = max(walls, box3(p, vec3(5.1, 3.1, 10.1)));
		walls = max(walls, -box3(p+vec3(1.5, 1., 10.), vec3(2., 3., 1.)));
		walls = min(walls, balki);
	} else
	if (sdf_scene == 2) {
		flr = p.y;
		p.y -= 4.;
		vec3 p1 = vec3(mod(p.x, 9.) - 4.5, p.yz);
		walls = box3(p, vec3(50., 8., 16.));
		walls = max(walls, -hexabox(p1, vec3(4., 3., 15.)));
		walls = max(walls, -hexabox(p1, vec3(4., 10., 7.)));
		walls = max(walls, -hexabox(p1-vec3(0., 0., 14.), vec3(10., 4., 5.)));
		walls = min(walls, box3(p1 - vec3(0., 6., -7.), vec3(1.2)));
		walls = max(walls, -box3(p1 - vec3(0., 0., -12.), vec3(3., 10., 3.)));
		walls = max(walls, -box3(p1 - vec3(0., 0., -12.), vec3(6., 4., 3.)));
		//pipes = length(rep2(p1.yz, vec2(4.))) - 1.;
		//return min(flr, walls);//max(walls, -pipes));

		/*
		walls = box3(p1, vec3(10., 10., 1.));
		walls = max(walls, -hexabox(p1, vec3(8., 7., 2.)));
		walls = min(walls, 14. - abs(p.x));
		walls = max(walls, - p.z - 20.);
		return min(flr, walls);
		*/
	} else if (sdf_scene == 3) {
		flr = p.y; // if (flr < .1) flr += .2 * noise2(p.xz);
		walls = min(p.x + 4., 6. - p.y);
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
		walls = max(walls, p.x - 6.);
		walls = min(walls, p.z + 30.);
		flr = max(flr, p.x - 6.);
		//return min(flr, walls);
	} else if (sdf_scene == 4) {
		flr = p.y;
		walls = p.z + 30.; //max(box3(p, vec3(6.)), -box3(p, vec3(5.)));
		//walls = max(walls, -box3(p, vec3(1., 100., 1.)));

		p.x = abs(p.x);
		walls = min(walls, 10. - p.x);

// ceiling w/ grid
		walls = min(walls, - p.y + 24.8);
		//walls = max(walls, -box3(vec3(rep2(p.xz, vec2(1.,2.)), p.y).xzy+vec3(0., -25., 10.), vec3(1.)));
		walls = max(walls,
			-box3(
				vec3(
					rep2(p.xz, vec2(2.,4.)),
					p.y - 25.).xzy,
				vec3(.8, 1., 1.6)));
		walls = max(walls, p.y - 25.);

		float paths = box3(vec3(p.xy, mod(p.z, 10.) - 5.), vec3(100., 4.5, 1.8));

		p -= vec3(30., 0., -20.);
		p *= rz2;
		walls = min(walls, box3(p, vec3(20.)));
		p *= rz3;
		p -= vec3(5., 10., -7.);
		walls = min(walls, box3(p, vec3(20.)));
		p *= rz5;
		p -= vec3(7., 3., -10.);
		walls = min(walls, box3(p, vec3(20.)));

		walls = max(walls, -paths);
		//return min(flr, walls);
	} else {
		flr = p.y;
		flr = min(flr, box3(p, vec3(.3, .1, 30.)));
		walls = -box3(p, vec3(3., 4., 30.));
		walls = max(walls, box3(p, vec3(4.0, 4.4, 31.)));
		vec3 p1 = vec3(p.x - 3., p.y - 1.5, mod(p.z, 10.) - 5.);
		/*vec3 p1 = p;
		p1.x -= 3.;
		p1.y += .3 - 1.8;
		p1.z = mod(p.z, 10.) - 5.;
		*/
		walls = max(walls, -box3(p1, vec3(2., 1.2, 1.)));
		// interesting walls = max(walls, box3(p1, vec3(2.2, 1.4, 1.2)));
		//walls = max(walls, p.x - 4.4);

		walls = max(walls, -box3(vec3(p.x,
			p.y + 4. - (t-512.-64.)/16. - 2. * max(0., t-684.),
			mod(p.z, 1.)-.5),
			vec3(20., 4., .4)));
		//shiny = box3(p + vec3(0., 0., 10.), vec3(1.));
		//return min(flr, min(walls, shiny));
	}

	//if (flr < .1) flr += .2 * noise2(p.xz);// + .1 * noise2(p.xz*4.);
	return min(flr, walls);
}

//float wd(vec3 p) { return w(p); }// + .01 * noise3(p*16.); }

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
//int sdfcnt = 0;
float march(vec3 o, vec3 d, float l, float L, int steps) {
	//float kw = 1.2;
	for (int i = 0; i < steps; ++i) {
		//++sdfcnt;
		float dd = w(o + d * l);
		l += dd;// * kw;
		if (dd < 0.002 * l || l > L) break;
	}
	return l;
}

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

/*
float tex1(vec2 p) {
 return fbm(p * 16. + 8. * (vec2(fbm(p*4.), fbm(p*4.+96.)) - .5));
}
*/

vec3 rust(vec2 uv) {
	return mix(
		vec3(.467, .22, .1),
		vec3(.627, .35, .216),
			smoothstep(.2,.8,fbm(uv*3.)));
}

vec2 normalToUv(vec3 p, vec3 n) {
	vec3 t1 = cross(n, E.zxx), t2 = cross(n, E.xxz), tangent, binormal;
	if (dot(t1,t1) > dot(t2,t2)) tangent = t1; else tangent = t2;
	binormal = cross(n, tangent);
	return vec2(dot(p, binormal), dot(p, tangent)).xy;
}

void main() {
	vec2 R = vec2(640., 720.);
	vec2 uv = gl_FragCoord.xy / R * vec2(3.56,2.) - 1.;// uv.x *= 2. * R.x / R.y;
	//gl_FragColor = vec4(uv, 0., 1.); return;
	//gl_FragColor = vec4(concrete(uv*2.), 1.); return;
	//gl_FragColor = vec4(rust(uv), 1.); return;

	// rust1
	// vec3(.467, .22, .1)
	// vec3(.627, .35, .216)
	// moss1
	//vec3(.48, .51, .31)

	mat3 Dorient = RX(0.);
	vec3 O, D, D2, N;
	O = vec3(-.5+lt, 1.8, 5.);
	vec3 sundir = vec3(-2.,1.,-1.);
	vec3 total_color = vec3(0.);
	float seedhash = t;
	float alpha = .15;
	if (sdf_scene < 2) {
		//O.y = 1.;
		O.z = -3. + 3. * lt;
	} else if (sdf_scene == 2) {
		//O.y = 0.;
		O.z = 16.;
		O.x = 3. - 6.*lt;
		Dorient = RX(-.2);
	} else if (sdf_scene == 3) {
		sundir.z = -sundir.z;
		Dorient = RY(-.15);
	} else if (sdf_scene == 4) {
		sundir = vec3(1., 2., 1.);
		O = vec3(5.-6.*lt, 1.8, 9.);
		Dorient =  RY(-.3+.2*lt) * RX(-.3);
	} else {
		O = vec3(2., 1.8 + .1 * abs(sin(lt*60.)), 5. - lt * 10.);
		Dorient = RY(-.5);
		alpha = mix(alpha, .6, step(512.+64., t));
	}
	D = Dorient * normalize(vec3(uv, -2.));
	//vec3 D = normalize(vec3(uv, -2.));
	D2 = Dorient * normalize(vec3(uv+vec2(2.,1.)/R, -2.));

	sundir = normalize(sundir);
	//const int samples_per_pixel = 16;
	for (int s = 0; s < 16; ++s) {
		vec3 o = O;
		vec3 d = mix(D, D2, vec3(hash1(seedhash), hash1(seedhash+1.), hash1(seedhash+2.)));
		vec3 sample_color = vec3(0.);
		vec3 k = vec3(1.);
		for (int b = 0; b < 4; ++b) {
			float roughness = .9;
			float L = 50.;
			float l = 0.;
			for (int i = 0; i < 40; ++i) {
				float dd = w(o + d * l);
				l += dd;
				if (dd < 0.002 * l || l > L)
					break;
			}

			vec3 emissive = vec3(0.);
			vec3 albedo = vec3(.75, .75, .73);
			vec3 n = vec3(0., 1., 0.);
			//roughness = .9;//.04;

			if (l > L) {
				emissive = vec3(2.) + vec3(30., 20., 10.) * pow(max(0.,dot(d, sundir)),10.);//*dot(d,vec3(0.,1.,0.)));
				//emissive *= 10.;
				albedo = vec3(0.);
				//o = o + d * l;
			} else {
				o = o + d * l;
				n = wn(o);
				vec2 uv = normalToUv(o, n);
				o += n * .01;

				float mossmask;
				if (flr < walls) {
					//albedo = vec3(.75,.75,.73);
					roughness = 9.*smoothstep(.4,.45,(fbm(uv/2.)*.85+.15*fbm(uv*8.))) + .01;
					mossmask = smoothstep(.45, .6, fbm(uv/4.) + .5 * saturate(- 10. - o.z));
				} else {
					//albedo = concrete(uv);
					albedo *= .6+(.2+.2*smoothstep(.9, .99, sin(PI*fbm(uv*4.))))*fbm(uv*16.);
					vec2 wC = floor(uv/2.), wc = (uv/2. - wC);
					albedo *= 1. - .3 * hash2(wC) - .4 * step(.99,max(wc.x,wc.y));
					//albedo = mix(albedo, .6 * rust(uv), 1.-step(.5, max(wc.x, wc.y)));
					/*albedo = mix(albedo,
							.4*rust(uv)*(.3+.7*fbm(uv*vec2(16.,4.))),
							smoothstep(.5, .6, fbm(uv*vec2(1.,.2))));
							*/
					mossmask = smoothstep(.5, .7, fbm(uv/4.));
					//roughness = mix(roughness, .4, mossmask);
					//albedo = mix(albedo, vec3(.3, .44, .15)*(.3+.7*fbm(uv*16.)), mossmask);
				}

				albedo = mix(albedo,
					vec3(.3, .44, .15)*(.3+.7*fbm(uv*16.)),
					mossmask);

				//albedo = vec3(tex1(o.xz/4.));
				////roughness = smoothstep(.3, .6,tex1(o.xz/4.));
				//roughness = .01 + .8*smoothstep(.3, .6, fbm(o.xz));
				//emissive = 20. * step(1.-16.*fract(31./32.), o.z) * step(o.z, 2.-16.*fract(31./32.));
			}

			sample_color += k * emissive;
			k *= albedo.rgb;
			//if (s == 0 && b == 0) { d = sundir; k *= max(0., dot(n, sundir)); continue; }
			if (all(lessThan(k,vec3(.003)))) break;
			//gl_FragColor = vec4(n, 1.); return;

			//if (any(isnan(d))) { break; }

			//if (true) {
			//if (false) {
			if (hash1(seedhash+=d.x) < .03) {
				d = sundir;
				roughness = .02;
				//k *= max(0., dot(n, d)) / 3.;
			} else {
				d = reflect(d, n);
			}
				d = normalize(mix(d, (
					vec3(
								hash1(seedhash += d.x),
								hash1(seedhash += d.y),
								hash1(seedhash += d.z))
								- .5) * 2.,
						roughness));
				d *= sign(dot(n, d));
				//k *= max(0., dot(n, d)) / 3.;
			//}
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
	gl_FragColor = vec4(total_color/16., alpha); // /* * smoothstep(0., 4., t)*/, mix(alpha, 1., step(t,1.)));//.4);
	//gl_FragColor = vec4(vec3(mod(float(sdfcnt),500.)/500.), 1.);
}
