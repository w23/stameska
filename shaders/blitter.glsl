#version 130
// TOOL
//uniform vec2 RES;
// Release
uniform sampler2D F;
uniform int s;
float t = float(s)/44096.;

void main() {
vec2 RES = vec2(1920.,1080.);
	/*
	vec2 uv = (gl_FragCoord.xy / RES - .5) * 2.; uv.x *= RES.x / RES.y;
	gl_FragColor = sqrt(texture2D(F, uv*vec2(640.,480.)/RES+.5));
	//gl_FragColor = vec4(uv, 0., 1.);
	//
	*/

	vec2 uv2 = gl_FragCoord.xy - RES*.5;
	vec2 FR = textureSize(F,0);
	vec3 color = vec3(.0);
	vec2 c = vec2(0., -200.);//sin(t/32.), cos(t/32.)) * 200.;
	float l = smoothstep(56., 68., t) * step(t, 192.);
	for (int i = 0; i < 80; ++i) {
		vec2 p = uv2 + l * normalize(uv2-c) * float(i-30);
		color += texture2D(F, (p*max(FR.x/RES.x,FR.y/RES.y)+FR*.5)/FR).rgb;
	}
	gl_FragColor = vec4(sqrt(color/80.),.0);

	//gl_FragColor = vec4(uv2, 0., 1.);

/*
	vec2 ts = textureSize(F,0);
	vec2 uv = gl_FragCoord.xy / RES; //uv.x *= RES.x / RES.y;
	vec2 ks = RES / ts;
	float k = 1.;//min(ks.x, ks.y);// / 2.;
	gl_FragColor = sqrt(texture2D(F, uv * k));

	gl_FragColor = vec4(mod(gl_FragCoord.x, 4.)/3.);
*/
	//gl_FragColor = vec4(uv, 0., 1.);
	//gl_FragColor = sqrt(texture2D(F, uv-vec2(.0,.5)/textureSize(F,0)) * (.5 + .5 * mod(gl_FragCoord.x, 2.)));
	//gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/2./textureSize(F,0)));
	//gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/2./textureSize(F,0)));
	//gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/1./textureSize(F,0)));
	//gl_FragColor = sqrt(texture2D(F, gl_FragCoord.xy/2./textureSize(F,0)));
}
