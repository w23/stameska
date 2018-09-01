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
   "vec2 r=vec2(1920.,1080.),y=gl_FragCoord.xy-r*.5,d=textureSize(F,0);"
   "vec3 x=vec3(0.);"
   "for(int v=0;v<80;++v)"
     "x+=texture2D(F,((y+.4*smoothstep(56.,68.,i)*step(i,192.)*(y-vec2(0.,-200.))*float(v-40)/80.)*max(d.x/r.x,d.y/r.y)+d*.5)/d).xyz;"
   "gl_FragColor=vec4(sqrt(x/80.),0.);"
 "}";

#endif // BLITTER_H_
