/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef BLITTER_H_
# define BLITTER_H_

const char *blitter_glsl =
 "#version 130\n"
 "uniform sampler2D F;"
 "uniform int s;"
 "float i=float(s)/44096.;"
 "void main()"
 "{"
   "vec2 r=vec2(1920.,1080.),v=gl_FragCoord.xy-r*.5,y=textureSize(F,0);"
   "vec3 f=vec3(0.);"
   "vec2 m=vec2(0.,-200.);"
   "float e=smoothstep(56.,68.,i)*step(i,192.);"
   "for(int u=0;u<80;++u)"
     "{"
       "vec2 o=v+e*normalize(v-m)*float(u-30);"
       "f+=texture2D(F,(o*max(y.x/r.x,y.y/r.y)+y*.5)/y).xyz;"
     "}"
   "gl_FragColor=vec4(sqrt(f/80.),0.);"
 "}";

#endif // BLITTER_H_
