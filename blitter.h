/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef BLITTER_H_
# define BLITTER_H_

const char *blitter_glsl =
 "#version 130\n"
 "uniform vec2 RES;"
 "uniform sampler2D F;"
 "void main()"
 "{"
   "vec2 r=textureSize(F,0),g=gl_FragCoord.xy/RES,o=RES/r;"
   "float v=1.;"
   "gl_FragColor=sqrt(texture2D(F,g*v));"
   "gl_FragColor=vec4(mod(gl_FragCoord.x,4.)/3.);"
   "gl_FragColor=sqrt(texture2D(F,gl_FragCoord.xy/textureSize(F,0)));"
 "}";

#endif // BLITTER_H_
