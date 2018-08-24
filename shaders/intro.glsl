#version 130
uniform vec2 RES;
uniform float t;
uniform sampler2D N;

//const float INF = 10000.;
const vec3 E = vec3(0., .01, 1.);
const float PI = 3.141593;

float hash1(float v) { return fract(sin(v) * 43758.5453); }
float hash2(vec2 v) { return hash1(dot(v, vec2(129.47, 37.56))); }
float hash3(vec3 v) { return hash1(dot(v, vec3(129.47, 37.56, 1.))); }
float noise1(float v) {
  float V = floor(v); v = fract(v);
  return mix(hash1(V), hash1(V+1.), v);
}
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

vec4 snoise24(vec2 c) { return texture2D(N, c); }

//float box2(vec2 p, vec2 s) { p = abs(p) - s; return max(p.x, p.y); }
//float box3(vec3 p, vec3 s) { p = abs(p) - s; return max(p.x, max(p.y, p.z)); }

float ball(vec3 p, float r) { return length(p) - r; }
vec3 rep3(vec3 p, vec3 r) { return mod(p,r) - r*.5; }
vec2 rep2(vec2 p, vec2 r) { return mod(p,r) - r*.5; }
float ring(vec3 p, float r, float R, float t) {
	return max(abs(p.y)-t, max(length(p) - R, r - length(p)));
}
//float vmax2(vec2 p) { return max(p.x, p.y); }
float vmax3(vec3 v) { return max(v.x, max(v.y, v.z)); }
float box(vec3 p, vec3 s) { return vmax3(abs(p) - s);}
float tor(vec3 p, vec2 s) { return length(vec2(length(p.xz) - s.x, p.y)) - s.y; }

mat3 RX(float a){ float s=sin(a),c=cos(a); return mat3(1.,0.,0.,0.,c,-s,0.,s,c); }
mat3 RY(float a){	float s=sin(a),c=cos(a); return mat3(c,0.,s,0.,1.,0,-s,0.,c); }
mat3 RZ(float a){ float s=sin(a),c=cos(a); return mat3(c,s,0.,-s,c,0.,0.,0.,1.); }

float raySphere(vec3 o, vec3 d, vec3 c, float r, float L) {
	vec3 l = c - o;
	float tca = dot(l, d);
	if (tca < 0.) return L;
	float d2 = dot(l, l) - tca * tca;
	float r2 = r * r;
	if (d2 < 0. || d2 > r2) return L;
	return tca - sqrt(r2 - d2);
}

float ornd(float a, float b, float r) {
		vec2 u = max(vec2(r - a,r - b), vec2(0));
			return max(r, min (a, b)) - length(u);
}

float sqr(vec3 p) { return dot(p, p); }

mat3 lookat(vec3 o, vec3 at, vec3 up) {
	vec3 z = normalize(o - at);
	vec3 x = normalize(cross(up, z));
	up = normalize(cross(z, x));
	return mat3(x, up, z);
}

/*const*/ float R0 = 6360e3;
/*const*/ float Ra = 6380e3;
// /*const*/ float low = 1e3, hi = 25e2, border = 5e2;
///*const*/ float Hr = 8e3, Hm = 12e2;
float I = 40.;

/*const*/ vec3 C = vec3(0., -R0, 0.);
/*const*/ vec3 bR = vec3(58e-7, 135e-7, 331e-7);
/*const*/ vec3 bMs = vec3(2e-5);
/*const*/ vec3 bMe = bMs * 1.1;

vec3 sundir;// = normalize(vec3(.5,.7,-1.));

//bool lolnan = false;

vec2 densitiesRM(vec3 p) {
	//float h = max(0., length(p - C) - R0);
	//if (any(isnan(p))) lolnan = true;
	float h = length(p - C) - R0;
	//if (isnan(h)) lolnan = true;
	vec2 retRM = vec2(exp(-h/8e3), exp(-h/12e2));
	//if (any(isnan(retRM))) lolnan = true;
	return retRM;
}

float escape(vec3 p, vec3 d, float R) {
	vec3 v = p - C;
	float b = dot(v, d);
	float c = dot(v, v) - R*R;
	float det = b * b - c;
	if (det < 0.) return -1.;
	det = sqrt(det);
	float t1 = -b - det, t2 = -b + det;
	return (t1 >= 0.) ? t1 : t2;
}

vec2 scatterDirectImpl(vec3 o, vec3 d, float L, float steps) {
	vec2 depthRMs = vec2(0.);
	L /= steps; d *= L;
	for (float i = 0.; i < steps; ++i)
		depthRMs += densitiesRM(o + d * i);
	return depthRMs * L;
}

/*
vec3 Lin(vec3 o, vec3 d, float L) {
	vec2 depthRMs = scatterDirectImpl(o, d, L, 32.);
	return exp(- bR * depthRMs.x - bMe * depthRMs.y);
}
*/

vec2 totalDepthRM;
vec3 LiR, LiM;

void scatterImpl(vec3 o, vec3 d, float L, float steps) {
	L /= steps; d *= L;
	for (float i = 0.; i < steps; ++i) {
		vec3 p = o + d * i;
		vec2 dRM = densitiesRM(p) * L;
		totalDepthRM += dRM;

		vec2 depthRMsum = totalDepthRM;
		float l = escape(p, sundir, Ra);
		depthRMsum += scatterDirectImpl(p, sundir, l, 8.);

		vec3 A = exp(-bR * depthRMsum.x - bMe * depthRMsum.y);
		LiR += A * dRM.x;
		LiM += A * dRM.y;
	}
}

vec3 scatter(vec3 o, vec3 d, float L, vec3 Lo) {
	totalDepthRM = vec2(0.);
	LiR = LiM = vec3(0.);
	scatterImpl(o, d, L, 8.);
	float mu = dot(d, sundir);

	return Lo * exp(-bR * totalDepthRM.x - bMe * totalDepthRM.y)
		+ I * (1. + mu * mu) * (
			LiR * bR * .0597 +
			LiM * bMs * .0196 / pow(1.58 - 1.52 * mu, 1.5));
}

float w(vec3 p) {
	//return length(p) - 1.;
	//return box(RY(t/8.)*(p-vec3(0., 1., 0.)), vec3(.5));

	float room = max(box(p, vec3(4.)), -box(p/*-vec3(0., 0., 1.)*/, vec3(3.9)));
	//float windows = box(rep3(p+vec3(.4,.0,0.), vec3(1.)), vec3(.3, 1., .2));
	//room = min(room, max(box(p, vec3(2.)), windows));
	//float windows = box(p-vec3(4., 0., 0.), vec3(1.));

	float windows = box(p-sundir * 5., vec2(mix(.1, 2., t/64.), 5.).xyx);
	//float windows = box(p-sundir * 5., vec3(mix(.1, 2., t/64.)*2.));

	//float windows = box(p-vec3(0., 1., 0.)* 5., vec3(1.2));
	//float windows = length(p-sundir * 5.) - 2.;
	room = max(room, -windows);

	return room;
}

vec3 wn(vec3 p) {
	return normalize(vec3(
	/* gives NaNs a lot
		w(p + E.yxx),
		w(p + E.xyx),
		w(p + E.xxy)) - w(p));*/
	w(p + E.yxx) - w(p - E.yxx),
		w(p + E.xyx) - w(p - E.xyx),
		w(p + E.xxy) - w(p - E.xxy)));
}

float march(vec3 o, vec3 d, float l, float L, int steps) {
	for (int i = 0; i < steps; ++i) {
		float dd = w(o + d * l);
		l += dd;//max(.01, d);
		if (dd < .001 * l || l > L) break;
	}
	return l;
}

void main() {
	//vec2 uv = gl_FragCoord.xy / RES;
	vec2 uv = (gl_FragCoord.xy / RES * 2. - 1.); uv.x *= 2. * RES.x / RES.y;

	//gl_FragColor = vec4(snoise24((floor(gl_FragCoord.xy) + .5)/textureSize(N, 0).xy).xyz, 1.); return;

	vec3 O = vec3(0., 1.5, 3.8);// + smoothstep(8., 0., t)*2.);
	vec3 up = E.xzx;
	vec3 at = vec3(0., 1., 0.);//2.*sin(t/9.), 0.);
	vec3 D = normalize(vec3(uv, -2.));
	D = lookat(O, at, up) * D;

	//sundir = normalize(vec3(.05, .04, -.1));
	//sundir = normalize(vec3(.5*sin(t/16.),.9 * (1. + sin(t/12.)),-1.*cos(t/16.)));
	//sundir = normalize(vec3(.05, .04 + .0 * .3 * (1. + cos(t/12.)), -.1));
	//sundir = normalize(vec3(.05*cos(t/17.), .07, -.1*sin(t/18.)));
	sundir = vec3(0., 1., 0.);

	//gl_FragColor = vec4(mod(scatter(O, D, escape(O, D, Ra), vec3(0.)), 2.), 1.); return;

	const int samples = 16;
	const int bounces = 4;
	float seedhash = t;
	vec2 seed = (gl_FragCoord.xy * vec2(17., 23.) + floor(t*943.)) / textureSize(N,0);
	vec3 total_color = vec3(0.);
	for (int s = 0; s < samples; ++s) {
		//seed.y += 1./textureSize(N,0).y;
		vec3 o = O;
		vec3 d = D;

		vec3 color = vec3(0.);
		vec3 k = vec3(1.);
		for (int b = 0; b < bounces; ++b) {
			//if (any(isnan(d))) { lolnan = true; break; }
			float lsky = escape(o, d, Ra);
			//if (isnan(lsky)) break;
			//if (isnan(lsky)) lolnan = true;
			int mat = 0;
			float l = lsky;
			//if (isnan(l)) break;

			float pXZ = d.y < -.0001 ? (0. - o.y) / d.y : l;
			if (pXZ < l) {
				l = pXZ;
				mat = 1;
			}

			float lsph = raySphere(o, d, vec3(0., 0., 0.), 1., lsky);
			if (lsph < l) {
				l = lsph;
				mat = 3;
			}

			float ldf = march(o, d, 0., 20., 32);
			//if (isnan(l)) {lolnan = true; break; }
			if (ldf < 10. && ldf < l) {
				l = ldf;
				mat = 2;
			}

			vec3 emissive = vec3(0.);
			vec3 albedo = vec3(.75, .75, .73);
			vec3 n = vec3(0., 1., 0.);
			float roughness = .04;

			if (mat == 0) {
				//emissive = d.y < 0. ? vec3(1., 0., 0.) : vec3(.1);
				//if (any(isnan(d))) lolnan = true;
				//if (isnan(l)) lolnan = true;
				emissive = scatter(o, d, l, vec3(0.));
				//if (lolnan) { gl_FragColor = vec4(1000., 0., 0., 1.); return; }
				//emissive = vec3(1.);
				albedo = vec3(0.);
				o = o + d * l;
			} else {
				o = o + d * l;
				if (mat == 1) {
					float puddle = smoothstep(.4, .5, fbm(o.xz*2.));
					roughness = .02 + .2 * puddle;
					//roughness = 0.;
				} else if (mat == 2) {
					//emissive = vec3(1.);
					roughness = .8;
					n = wn(o);
					//if (any(isnan(n))) { lolnan = true; break; }
					o += n * .01;
					float ep = -4. + 8. * mod(t/64., 1.);
					//emissive = vec3(10. * step(ep, o.z) * step(o.z, ep + .1));
				} else if (mat == 3) {
					n = normalize(o);
				}
			}

			//vec3 fog = vec3(1.);//vec3(1. * exp(-l/8.));
			vec3 fog = vec3(1.);//vec3(exp(-max(0., length(o)-4.)/2.));
			/*
			color += fog * k * em;
			k *= al * fog;
			*/

			k *= fog;
			color += k * emissive;
			k *= albedo.rgb;
			//if (s == 0 && b == 0) { d = sundir; k *= max(0., dot(n, sundir)); continue; }
			if (all(lessThan(k,vec3(.001))))
				break;

			d = reflect(d, n);
			//if (any(isnan(d))) { break; }
			/*
			seed.x += 1. / textureSize(N,0).x;
			vec4 rvect = texture2D(N, seed);//snoise24(vec2(seed,.5));
			d = normalize(mix(d, (rvect.xyz - .5) * 2., roughness));
			*/

			d = normalize(mix(d, (vec3(
							hash1(seedhash += d.x),
							hash1(seedhash += d.y),
							hash1(seedhash += d.z)) - .5) * 2.,
					roughness));

			d *= sign(dot(n, d));
			//float s = sign(dot(n, d)); if (s < .0) { gl_FragColor = vec4(1., 0., 0., 1.); return; }
		}

		//if (lolnan) { gl_FragColor = vec4(0., 1000., 0., 1.); return; }

		total_color += color;
	}

	//if (lolnan) { gl_FragColor = vec4(0., 0., 1000., 1.); return; }

	total_color /= float(samples);

	gl_FragColor = vec4(total_color /* * smoothstep(0., 4., t)*/, mix(.1, 1., step(t,1.)));//.4);
}
