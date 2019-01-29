/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef INTRO_H_
# define INTRO_H_

const char *intro_glsl =
 "uniform sampler2D T;"
 "uniform int s;"
 "float v=float(s)/88200.;"
 "const vec3 m=vec3(0.,.001,1.);"
 "const float i=3.14159;"
 "float x(float v)"
 "{"
   "return min(1.,max(0.,v));"
 "}"
 "float f(float v)"
 "{"
   "return fract(sin(v)*43758.5);"
 "}"
 "float n(vec2 k)"
 "{"
   "return f(dot(k,vec2(129.47,37.56)));"
 "}"
 "float t(vec2 v)"
 "{"
   "vec2 k=floor(v);"
   "v=fract(v);"
   "v*=v*(3.-2.*v);"
   "return mix(mix(n(k),n(k+m.zx),v.x),mix(n(k+m.xz),n(k+m.zz),v.x),v.y);"
 "}"
 "float d(vec2 v)"
 "{"
   "return.5*t(v)+.25*t(v*2.3)+.125*t(v*4.1);"
 "}"
 "float d(vec3 k,vec3 v)"
 "{"
   "return k=abs(k)-v,max(k.z,max(k.x,k.y));"
 "}"
 "float d(float k,float v,float m)"
 "{"
   "return max(max(k,v),(k+m+v)*sqrt(.5));"
 "}"
 "float f(vec3 k,vec3 v)"
 "{"
   "return k=abs(k)-v,d(d(k.x,k.z,1.),k.y,1.);"
 "}"
 "vec3 n(vec3 k,vec3 v)"
 "{"
   "return mod(k,v)-v*.5;"
 "}"
 "vec2 t(vec2 k,vec2 v)"
 "{"
   "return mod(k,v)-v*.5;"
 "}"
 "mat3 r(float v)"
 "{"
   "float k=sin(v),z=cos(v);"
   "return mat3(1.,0.,0.,0.,z,-k,0.,k,z);"
 "}"
 "mat3 p(float v)"
 "{"
   "float k=sin(v),z=cos(v);"
   "return mat3(z,0.,k,0.,1.,0,-k,0.,z);"
 "}"
 "mat3 c(float v)"
 "{"
   "float k=sin(v),z=cos(v);"
   "return mat3(z,k,0.,-k,z,0.,0.,0.,1.);"
 "}"
 "float z=v/128.,y=fract(z);"
 "int a=1+int(floor(z));"
 "float e,k;"
 "mat3 g=c(.2),o=c(.3),l=c(.5);"
 "float w(vec3 m)"
 "{"
   "if(a<2)"
     "{"
       "e=m.y;"
       "k=-d(m,vec3(4.,3.,10.));"
       "float z=max(-k,d(vec3(m.xy-vec2(0.,3.),fract(m.z)-.5),vec3(10.,.3,.1)));"
       "k=max(k,d(m,vec3(5.1,3.1,10.1)));"
       "k=max(k,-d(m+vec3(1.5,1.,10.),vec3(2.,3.,1.)));"
       "k=min(k,z);"
     "}"
   "else"
     " if(a==2)"
       "{"
         "e=m.y;"
         "m.y-=4.;"
         "vec3 z=vec3(mod(m.x,9.)-4.5,m.yz);"
         "k=d(m,vec3(50.,8.,16.));"
         "k=max(k,-f(z,vec3(4.,3.,15.)));"
         "k=max(k,-f(z,vec3(4.,10.,7.)));"
         "k=max(k,-f(z-vec3(0.,0.,14.),vec3(10.,4.,5.)));"
         "k=min(k,d(z-vec3(0.,6.,-7.),vec3(1.2)));"
         "k=max(k,-d(z-vec3(0.,0.,-12.),vec3(3.,10.,3.)));"
         "k=max(k,-d(z-vec3(0.,0.,-12.),vec3(6.,4.,3.)));"
       "}"
     "else"
       " if(a==3)"
         "{"
           "e=m.y;"
           "k=min(m.x+4.,6.-m.y);"
           "vec3 z=m;"
           "z.z=mod(m.z,10.)-5.;"
           "float i=d(z,vec3(1.,10.,1.));"
           "z.x-=4.;"
           "z.y+=-1.5;"
           "float x=d(z,vec3(.5,10.,.5));"
           "k=min(k,x);"
           "k=min(k,d(m-vec3(5.,0.,0.),vec3(1.,1.,100.)));"
           "k=max(k,-min(i,6.4-m.y));"
           "k=max(k,m.x-6.);"
           "k=min(k,m.z+30.);"
           "e=max(e,m.x-6.);"
         "}"
       "else"
         " if(a==4)"
           "{"
             "e=m.y;"
             "k=m.z+30.;"
             "m.x=abs(m.x);"
             "k=min(k,10.-m.x);"
             "k=min(k,-m.y+24.8);"
             "k=max(k,-d(vec3(t(m.xz,vec2(2.,4.)),m.y-25.).xzy,vec3(.8,1.,1.6)));"
             "k=max(k,m.y-25.);"
             "float z=d(vec3(m.xy,mod(m.z,10.)-5.),vec3(100.,4.5,1.8));"
             "m-=vec3(30.,0.,-20.);"
             "m*=g;"
             "k=min(k,d(m,vec3(20.)));"
             "m*=o;"
             "m-=vec3(5.,10.,-7.);"
             "k=min(k,d(m,vec3(20.)));"
             "m*=l;"
             "m-=vec3(7.,3.,-10.);"
             "k=min(k,d(m,vec3(20.)));"
             "k=max(k,-z);"
           "}"
         "else"
           "{"
             "e=m.y;"
             "e=min(e,d(m,vec3(.3,.1,30.)));"
             "k=-d(m,vec3(3.,4.,30.));"
             "k=max(k,d(m,vec3(4.,4.4,31.)));"
             "vec3 z=vec3(m.x-3.,m.y-1.5,mod(m.z,10.)-5.);"
             "k=max(k,-d(z,vec3(2.,1.2,1.)));"
             "k=max(k,-d(vec3(m.x,m.y+4.-(v-512.-64.)/16.-2.*max(0.,v-684.),mod(m.z,1.)-.5),vec3(20.,4.,.4)));"
           "}"
   "return min(e,k);"
 "}"
 "vec3 u(vec3 v)"
 "{"
   "return normalize(vec3(w(v+m.yxx)-w(v-m.yxx),w(v+m.xyx)-w(v-m.xyx),w(v+m.xxy)-w(v-m.xxy)));"
 "}"
 "float c(vec3 k,vec3 m,float v,float z,int y)"
 "{"
   "for(int i=0;i<y;++i)"
     "{"
       "float a=w(k+m*v);"
       "v+=a;"
       "if(a<.002*v||v>z)"
         "break;"
     "}"
   "return v;"
 "}"
 "vec3 b(vec2 v)"
 "{"
   "return mix(vec3(.467,.22,.1),vec3(.627,.35,.216),smoothstep(.2,.8,d(v*3.)));"
 "}"
 "vec2 b(vec3 k,vec3 v)"
 "{"
   "vec3 z=cross(v,m.zxx),y=cross(v,m.xxz),x,f;"
   "if(dot(z,z)>dot(y,y))"
     "x=z;"
   "else"
     " x=y;"
   "f=cross(v,x);"
   "return vec2(dot(k,f),dot(k,x)).xy;"
 "}"
 "void main()"
 "{"
   "vec2 z=vec2(640.,720.),m=gl_FragCoord.xy/z*vec2(3.56,2.)-1.;"
   "mat3 c=r(0.);"
   "vec3 g,l,o,t;"
   "g=vec3(-.5+y,1.8,5.);"
   "vec3 h=vec3(-2.,1.,-1.),C=vec3(0.);"
   "float D=v,q=.15;"
   "if(a<2)"
     "g.z=-3.+3.*y;"
   "else"
     " if(a==2)"
       "g.z=16.,g.x=3.-6.*y,c=r(-.2);"
     "else"
       " if(a==3)"
         "h.z=-h.z,c=p(-.15);"
       "else"
         " if(a==4)"
           "h=vec3(1.,2.,1.),g=vec3(5.-6.*y,1.8,9.),c=p(-.3+.2*y)*r(-.3);"
         "else"
           " g=vec3(2.,1.8+.1*abs(sin(y*60.)),5.-y*10.),c=p(-.5),q=mix(q,.6,step(576.,v));"
   "l=c*normalize(vec3(m,-2.));"
   "o=c*normalize(vec3(m+vec2(2.,1.)/z,-2.));"
   "h=normalize(h);"
   "for(int Z=0;Z<16;++Z)"
     "{"
       "vec3 Y=g,X=mix(l,o,vec3(f(D),f(D+1.),f(D+2.))),W=vec3(0.),V=vec3(1.);"
       "for(int U=0;U<4;++U)"
         "{"
           "float S=.9,R=50.,Q=0.;"
           "for(int P=0;P<40;++P)"
             "{"
               "float O=w(Y+X*Q);"
               "Q+=O;"
               "if(O<.002*Q||Q>R)"
                 "break;"
             "}"
           "vec3 O=vec3(0.),P=vec3(.75,.75,.73),M=vec3(0.,1.,0.);"
           "if(Q>R)"
             "O=vec3(2.)+vec3(30.,20.,10.)*pow(max(0.,dot(X,h)),10.),P=vec3(0.);"
           "else"
             "{"
               "Y=Y+X*Q;"
               "M=u(Y);"
               "vec2 L=b(Y,M);"
               "Y+=M*.01;"
               "float K;"
               "if(e<k)"
                 "S=9.*smoothstep(.4,.45,d(L/2.)*.85+.15*d(L*8.))+.01,K=smoothstep(.45,.6,d(L/4.)+.5*x(-10.-Y.z));"
               "else"
                 "{"
                   "P*=.6+(.2+.2*smoothstep(.9,.99,sin(i*d(L*4.))))*d(L*16.);"
                   "vec2 J=floor(L/2.),I=L/2.-J;"
                   "P*=1.-.3*n(J)-.4*step(.99,max(I.x,I.y));"
                   "K=smoothstep(.5,.7,d(L/4.));"
                 "}"
               "P=mix(P,vec3(.3,.44,.15)*(.3+.7*d(L*16.)),K);"
             "}"
           "W+=V*O;"
           "V*=P.xyz;"
           "if(all(lessThan(V,vec3(.003))))"
             "break;"
           "if(f(D+=X.x)<.03)"
             "X=h,S=.02;"
           "else"
             " X=reflect(X,M);"
           "X=normalize(mix(X,(vec3(f(D+=X.x),f(D+=X.y),f(D+=X.z))-.5)*2.,S));"
           "X*=sign(dot(M,X));"
         "}"
       "C+=W;"
     "}"
   "gl_FragColor=vec4(C/16.,q);"
 "}";

#endif // INTRO_H_