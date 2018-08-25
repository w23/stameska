/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef INTRO_H_
# define INTRO_H_

const char *intro_glsl =
 "#version 130\n"
 "uniform vec2 RES;"
 "uniform float t;"
 "const vec3 v=vec3(0.,.001,1.);"
 "const float m=3.14159;"
 "float f=t/8.,l=t/16.,x=t/32.,y=t/64.;"
 "float n(float v)"
 "{"
   "return fract(sin(v)*43758.5);"
 "}"
 "float p(vec2 v)"
 "{"
   "return n(dot(v,vec2(129.47,37.56)));"
 "}"
 "float e(vec2 f)"
 "{"
   "vec2 m=floor(f);"
   "f=fract(f);"
   "return mix(mix(p(m),p(m+v.zx),f.x),mix(p(m+v.xz),p(m+v.zz),f.x),f.y);"
 "}"
 "float r(vec2 v)"
 "{"
   "return.5*e(v)+.25*e(v*2.3)+.125*e(v*4.1);"
 "}"
 "float e(vec3 v,vec3 x)"
 "{"
   "return v=abs(v)-x,max(v.x,max(v.y,v.z));"
 "}"
 "vec3 n(vec3 v,vec3 f)"
 "{"
   "return mod(v,f)-f*.5;"
 "}"
 "mat3 s(float v)"
 "{"
   "float f=sin(v),x=cos(v);"
   "return mat3(1.,0.,0.,0.,x,-f,0.,f,x);"
 "}"
 "mat3 h(float v)"
 "{"
   "float f=sin(v),x=cos(v);"
   "return mat3(x,0.,f,0.,1.,0,-f,0.,x);"
 "}"
 "float e(float v,float f,float x)"
 "{"
   "return max(x,min(v,f))-length(max(vec2(x-v,x-f),vec2(0)));"
 "}"
 "mat3 h(vec3 v,vec3 f,vec3 x)"
 "{"
   "vec3 m=normalize(v-f),y=normalize(cross(x,m));"
   "x=normalize(cross(m,y));"
   "return mat3(y,x,m);"
 "}"
 "float z=6.36e+06,i=6.38e+06,S=40.;"
 "vec3 k=vec3(0.,-z,0.),R=vec3(5.8e-06,1.35e-05,3.31e-05),d=vec3(2e-05),c=d*1.1,u;"
 "vec2 a(vec3 v)"
 "{"
   "float f=max(.1,length(v-k)-z);"
   "return vec2(exp(-f/8000.),exp(-f/1200.));"
 "}"
 "float a(vec3 v,vec3 f,float x)"
 "{"
   "vec3 m=v-k;"
   "float e=dot(m,f),y=dot(m,m)-x*x,i=e*e-y;"
   "if(i<0.)"
     "return-1.;"
   "i=sqrt(i);"
   "float n=-e-i,u=-e+i;"
   "return n>=0.?n:u;"
 "}"
 "vec2 a(vec3 v,vec3 f,float m,float x)"
 "{"
   "vec2 e=vec2(0.);"
   "m/=x;"
   "f*=m;"
   "for(float i=0.;i<x;++i)"
     "e+=a(v+f*i);"
   "return e*m;"
 "}"
 "vec3 e(vec3 v,vec3 f,float m,vec3 x)"
 "{"
   "vec2 e=vec2(0.);"
   "vec3 r,y;"
   "r=y=vec3(0.);"
   "float z=dot(f,u),n=8.;"
   "m/=n;"
   "f*=m;"
   "for(float k=0.;k<n;++k)"
     "{"
       "vec3 o=v+f*k;"
       "vec2 s=a(o)*m;"
       "e+=s;"
       "vec2 E=e+a(o,u,a(o,u,i),8.);"
       "vec3 g=exp(-R*E.x-c*E.y);"
       "r+=g*s.x;"
       "y+=g*s.y;"
     "}"
   "return x*exp(-R*e.x-c*e.y)+S*(1.+z*z)*(r*R*.0597+y*d*.0196/pow(1.58-1.52*z,1.5));"
 "}"
 "float g(vec3 f)"
 "{"
   "return f.y+=.2,min(length(f-v.zxx*1.4),length(f+v.zxx*1.4))-.8;"
 "}"
 "vec3 o=vec3(8.,4.,8.);"
 "float E=0.;"
 "float w(vec3 v)"
 "{"
   "v.z-=4.;"
   "float f=-e(v,o);"
   "if(t>768.)"
     "f=max(max(f,e(v,o+.3)),-e(n(v-u*o,vec3(7.,9.,2.)),E*vec3(3.,4.,.5)));"
   "return f;"
 "}"
 "mat3 b=s(y)*h(x);"
 "float C(vec3 v)"
 "{"
   "return e(b*(v-vec3((t-320.)*3./256.-1.5,-.2,0.)),vec3(.3));"
 "}"
 "float T(vec3 v)"
 "{"
   "float f=g(v);"
   "if(t>320.&&t<576.)"
     "f=e(f,C(v),.3);"
   "return min(w(v),f);"
 "}"
 "vec3 q(vec3 f)"
 "{"
   "return normalize(vec3(T(f+v.yxx)-T(f-v.yxx),T(f+v.xyx)-T(f-v.xyx),T(f+v.xxy)-T(f-v.xxy)));"
 "}"
 "float C(vec3 v,vec3 f,float x,float m,int y)"
 "{"
   "for(int i=0;i<y;++i)"
     "{"
       "float n=T(v+f*x);"
       "x+=n;"
       "if(n<.001*x||x>m)"
         "break;"
     "}"
   "return x;"
 "}"
 "void main()"
 "{"
   "vec2 f=gl_FragCoord.xy/RES*2.-1.;"
   "f.x*=2.*RES.x/RES.y;"
   "vec3 m=vec3(0.,1.5,3.9),x=v.xzx,y=vec3(0.),s=normalize(vec3(f,-2.));"
   "if(t<384.)"
     "{"
       "float z=smoothstep(64.,384.,t);"
       "y=mix(vec3(0.,.4,-4.),vec3(0.),z);"
       "m=mix(vec3(0.,.4,-2.),vec3(0.,1.5,3.9),z);"
     "}"
   "else"
     "{"
       "float z=smoothstep(432.,640.,t);"
       "m=mix(m,vec3(0.,-.2,5.9),z);"
       "float k=smoothstep(512.,704.,t);"
       "o=mix(o,vec3(2.2,3.,6.),k);"
       "E=mix(.38,1.,smoothstep(768.,896.,t));"
     "}"
   "s=h(m,y,x)*s;"
   "u=normalize(mix(vec3(0.,10.,0.),vec3(-.2,.01,-.3),t/1216.));"
   "float z=t;"
   "vec3 k=vec3(0.);"
   "for(int c=0;c<16;++c)"
     "{"
       "vec3 d=m,S=s,l=vec3(0.),R=vec3(1.);"
       "for(int b=0;b<4;++b)"
         "{"
           "float p=a(d,S,i);"
           "int T=0;"
           "float Z=S.y<-.0001?(-1.-d.y)/S.y:p;"
           "if(Z<p)"
             "p=Z,T=1;"
           "float Y=min(p,20.),X=C(d,S,0.,Y,32);"
           "if(X<Y)"
             "p=X,T=2;"
           "vec3 W=vec3(0.),V=vec3(.75,.75,.73),U=vec3(0.,1.,0.);"
           "float Q=.04;"
           "if(T==0)"
             "{"
               "W=e(d,S,p,vec3(0.));"
               "if(any(isnan(W)))"
                 "break;"
               "V=vec3(0.);"
               "d=d+S*p;"
             "}"
           "else"
             "{"
               "d=d+S*p;"
               "if(T==1)"
                 "Q=.01+.6*smoothstep(.4,.5,r(d.xz*2.));"
               "else"
                 " if(T==2)"
                   "{"
                     "U=q(d);"
                     "d+=U*.01;"
                     "if(g(d)>w(d))"
                       "{"
                         "Q=.8;"
                         "if(U.z>.9)"
                           "{"
                             "vec2 P=floor(d.xy*4.);"
                             "float O=mod(-t/2.+n(P.y)*64.,64.)-32.;"
                             "W=vec3(step(O,P.x)*step(P.x,O+1.+8.*n(P.y+32.))*step(P.x,(-t+734.)/2.));"
                           "}"
                       "}"
                   "}"
             "}"
           "l+=R*W;"
           "R*=V.xyz;"
           "if(all(lessThan(R,vec3(.001))))"
             "break;"
           "S=reflect(S,U);"
           "S=normalize(mix(S,(vec3(n(z+=S.x),n(z+=S.y),n(z+=S.z))-.5)*2.,Q));"
           "S*=sign(dot(U,S));"
         "}"
       "k+=l;"
     "}"
   "gl_FragColor=vec4(k/16.,.3);"
 "}";

#endif // INTRO_H_
