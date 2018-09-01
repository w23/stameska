/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef BLITTER_H_
# define BLITTER_H_

const char *blitter_glsl =
 "uniform sampler2D T;"
 "uniform int s;"
 "float i=float(s)/44096.;"
 "vec2 r=vec2(640.,480.),v=vec2(1920.,1080.);"
 "void main()"
 "{"
   "vec2 m=gl_FragCoord.xy-v*.5;"
   "vec3 x=vec3(0.);"
   "for(int f=0;f<80;++f)"
     "x+=texture2D(T,((m+.4*smoothstep(56.,68.,i)*step(i,192.)*(m-vec2(0.,-200.))*float(f-40)/80.)*max(r.x/v.x,r.y/v.y)+r*.5)/r).xyz;"
   "gl_FragColor=vec4(sqrt(x/80.),0.);"
 "}";

#endif // BLITTER_H_
