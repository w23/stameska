#version 130
// TOOL
uniform vec2 R;
uniform sampler2D T;
//uniform int s;
//float t = float(s)/88200.;
//vec2 FR = vec2(640., 480.);//textureSize(T,0);
vec2 FR = textureSize(T,0);
//vec2 RES = vec2(1920.,1080.);

void main() {
	// Release
	//vec2 uv = (gl_FragCoord.xy / R - .5) * 2.; uv.x *= R.x / R.y;
	//gl_FragColor = sqrt(texture2D(F, uv*vec2(640.,480.)/R+.5)); return;
	//gl_FragColor = vec4(uv, 0., 1.); return;

	/* radial blur from midschool
	vec2 uv2 = gl_FragCoord.xy - RES*.5;
	vec3 color = vec3(.0);
	for (int i = 0; i < 80; ++i) {
		color += texture2D(T, ((uv2 + .4 * smoothstep(56., 68., t) * step(t, 192.) * (uv2-vec2(0., -200.)) * float(i-40) / 80.) * max(FR.x/RES.x,FR.y/RES.y)+FR*.5)/FR).rgb;
	}
	gl_FragColor = vec4(sqrt(color/80.),.0);
	*/
	//gl_FragColor = sqrt(texture2D(T, ((gl_FragCoord.xy - R*.5) * max(FR.x/R.x,FR.y/R.y)+FR*.5)/FR));

	vec2 uv = (gl_FragCoord.xy + .5 ) / R;
	//gl_FragColor = vec4(uv, 0., 1.);
	gl_FragColor = sqrt(texture2D(T, uv-vec2(.0,.5)/textureSize(T,0)) * (.5 + .5 * mod(gl_FragCoord.x, 2.)));

	//gl_FragColor = vec4(uv2, 0., 1.);

	//gl_FragColor = vec4(uv, 0., 1.);
	//gl_FragColor = sqrt(texture2D(F, uv-vec2(.0,.5)/textureSize(F,0)) * (.5 + .5 * mod(gl_FragCoord.x, 2.)));
	//gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/2./textureSize(F,0)));
	//gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/2./textureSize(F,0)));
	//gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/1./textureSize(F,0)));
	//gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/2./textureSize(F,0)));
}
