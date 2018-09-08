/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef BLITTER_H_
# define BLITTER_H_

const char *blitter_glsl =
 "uniform sampler2D T;"
 "void main()"
 "{"
   "gl_FragColor=sqrt(texture2D(T,(gl_FragCoord.xy+.5)/vec2(1280.,720.)-vec2(0.,.5)/vec2(640.,720.))*(.5+.5*mod(gl_FragCoord.x,2.)));"
 "}";

#endif // BLITTER_H_
