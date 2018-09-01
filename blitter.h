/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef BLITTER_H_
# define BLITTER_H_

const char *blitter_glsl =
 "uniform sampler2D T;"
 "uniform int s;"
 "float i=float(s)/44096.;"
 "void main()"
 "{"
   "vec2 r=vec2(1920.,1080.),y=vec2(640.,480.),o=gl_FragCoord.xy-r*.5;"
   "vec3 x=vec3(0.);"
   "for(int v=0;v<80;++v)"
     "x+=texture2D(T,((o+.4*smoothstep(56.,68.,i)*step(i,192.)*(o-vec2(0.,-200.))*float(v-40)/80.)*max(y.x/r.x,y.y/r.y)+y*.5)/y).xyz;"
   "gl_FragColor=vec4(sqrt(x/80.),0.);"
 "}";

#endif // BLITTER_H_
