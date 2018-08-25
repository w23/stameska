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
   "vec2 r=(gl_FragCoord.xy+.5)/RES;"
   "gl_FragColor=sqrt(texture2D(F,r-vec2(0.,.5)/textureSize(F,0))*(.5+.5*mod(gl_FragCoord.x,2.)));"
 "}";

#endif // BLITTER_H_
