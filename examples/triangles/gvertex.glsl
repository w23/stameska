#version 130
uniform vec2 R;
uniform float t;

mat3 lookat(vec3 pos, vec3 at, vec3 up) {
	vec3 dir = normalize(at - pos);
	vec3 right = normalize(cross(dir, up));
	up = normalize(cross(right, dir));
	return mat3(right, up, -dir);
}

const vec3 E = vec3(0., .01, 1.);

mat4 lookat4(vec3 pos, vec3 at, vec3 up) {
	mat4 tr = mat4(E.zxxx, E.xzxx, E.xxzx, vec4(-pos, 1.));
	return mat4(transpose(lookat(pos,at,up))) * tr;
}

mat4 persp(float n, float f, float hfov, float aspect) {
	float w = 2. * n * tan(hfov / 2.), h = w / aspect;
	return mat4(
		vec4(2.*n/w, 0., 0., 0.),
		vec4(0., 2.*n/h, 0., 0.),
		vec4(0., 0., (f+n)/(n-f), -1.),
		vec4(0., 0., (2.*f*n)/(n-f), 0.));
}

float tt = t*.1;
const float dt = 4.;
float tn = floor(t / dt);
float Tt = smoothstep(0., dt, mod(t, dt));
float t2 = tn + Tt;

const float dt2 = 16.;
float tn2 = floor(t / dt2);
float Tt2 = sqrt(smoothstep(0., dt2, mod(t, dt2)));
float t22 = tn2 + Tt2;

float hash(float f) { return fract(sin(f)*53285.32132); }
float hash(vec2 f) { return hash(dot(vec2(17.,119.), f)); }

vec3 CO =
	15.*(-.5+mix(
		vec3(hash(tn2), hash(tn2+1.), hash(tn2+2.)),
		vec3(hash(tn2+1.), hash(tn2+2.), hash(tn2+3.)), Tt2));
vec3 CA =
	3.*(-.5+mix(
		vec3(hash(tn2+10.), hash(tn2+11.), hash(tn2+12.)),
		vec3(hash(tn2+11.), hash(tn2+12.), hash(tn2+13.)), Tt2));
vec3 CU =
	normalize(vec3(0.,1.,0.)+.8*(-.5+mix(
		vec3(hash(tn2+20.), hash(tn2+21.), hash(tn2+22.)),
		vec3(hash(tn2+21.), hash(tn2+22.), hash(tn2+23.)), Tt2)));

mat4 mproj = persp(0.01, 100., 3.1415927/2., R.x/R.y);
mat3 mlat3 = lookat(CO, CA, CU);
mat4 mvp = mproj * lookat4(CO, CA, CU);

varying vec2 v_uv;
varying vec3 v_p;
varying vec3 v_n;

const int N = 32;
const int W = N, H = N;
const vec2 geom_size = vec2(30., 30.);

void main() {

	int row_id = gl_VertexID / (2*W + 4);
	int row_x = int(mod(gl_VertexID, 2*W + 4));
	if (row_x > 2*W + 2) row_x -= 1;
	if (row_x > 0) row_x -= 1;

	vec3 p = vec3(row_x / 2, mod(row_x, 2) + row_id, 0.);
	float r = 2.;
	const int next = 329;
	r += 3. * mix(
		hash(floor(p.xy)) * step(next - 2, mod(gl_VertexID + tn, next)),
		hash(floor(p.xy)) * step(next - 2, mod(gl_VertexID + tn + 1., next)), Tt);
	p.xy /= vec2(W, H);
	r += mix(
		hash(floor(p.y*4. + floor(tn))),
		hash(floor(p.y*4. + floor(tn + 1.))), Tt);
	v_uv = p.xy;

	const float PI = 3.1415926;
	p.z = (p.y - .5) * 3.;
	p.xy = r * vec2(cos(p.x * PI * 2.), sin(p.x * PI * 2.));

	/*
	p.xy -= .5;
	p.xy *= geom_size;
	p.z = sin(p.x) * sin(p.y);
	*/
	p = p.xzy;
	v_p = p;
	v_n = normalize(p);

	gl_Position =
		mvp *
		vec4(p, 1.);

	gl_PointSize = 10.;
}
