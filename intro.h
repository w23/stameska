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
 "float f=t/8.,s=t/16.,x=t/32.,y=t/64.;"
 "float p(float v)"
 "{"
   "return fract(sin(v)*43758.5);"
 "}"
 "float n(vec2 v)"
 "{"
   "return p(dot(v,vec2(129.47,37.56)));"
 "}"
 "vec3 e(float v)"
 "{"
   "return vec3(p(v),p(v+17.),p(v+32.));"
 "}"
 "float r(vec2 f)"
 "{"
   "vec2 m=floor(f);"
   "f=fract(f);"
   "return mix(mix(n(m),n(m+v.zx),f.x),mix(n(m+v.xz),n(m+v.zz),f.x),f.y);"
 "}"
 "float i(vec2 v)"
 "{"
   "return.5*r(v)+.25*r(v*2.3)+.125*r(v*4.1);"
 "}"
 "float e(vec3 v,vec3 x)"
 "{"
   "return v=abs(v)-x,max(v.x,max(v.y,v.z));"
 "}"
 "vec3 i(vec3 v,vec3 f)"
 "{"
   "return mod(v,f)-f*.5;"
 "}"
 "mat3 d(float v)"
 "{"
   "float f=sin(v),x=cos(v);"
   "return mat3(1.,0.,0.,0.,x,-f,0.,f,x);"
 "}"
 "mat3 h(float v)"
 "{"
   "float f=sin(v),x=cos(v);"
   "return mat3(x,0.,f,0.,1.,0,-f,0.,x);"
 "}"
 "float d(float v,float f,float x)"
 "{"
   "return max(x,min(v,f))-length(max(vec2(x-v,x-f),vec2(0)));"
 "}"
 "mat3 e(vec3 v,vec3 f,vec3 x)"
 "{"
   "vec3 m=normalize(v-f),y=normalize(cross(x,m));"
   "x=normalize(cross(m,y));"
   "return mat3(y,x,m);"
 "}"
 "float z=6.36e+06,l=6.38e+06,S=40.;"
 "vec3 k=vec3(0.,-z,0.),R=vec3(5.8e-06,1.35e-05,3.31e-05),c=vec3(2e-05),o=c*1.1,u;"
 "vec2 a(vec3 v)"
 "{"
   "float f=max(.1,length(v-k)-z);"
   "return vec2(exp(-f/8000.),exp(-f/1200.));"
 "}"
 "float a(vec3 v,vec3 f,float x)"
 "{"
   "vec3 m=v-k;"
   "float s=dot(m,f),y=dot(m,m)-x*x,i=s*s-y;"
   "if(i<0.)"
     "return-1.;"
   "i=sqrt(i);"
   "float z=-s-i,l=-s+i;"
   "return z>=0.?z:l;"
 "}"
 "vec2 a(vec3 v,vec3 f,float m,float x)"
 "{"
   "vec2 z=vec2(0.);"
   "m/=x;"
   "f*=m;"
   "for(float i=0.;i<x;++i)"
     "z+=a(v+f*i);"
   "return z*m;"
 "}"
 "vec3 d(vec3 v,vec3 f,float m,vec3 x)"
 "{"
   "vec2 i=vec2(0.);"
   "vec3 z,s;"
   "z=s=vec3(0.);"
   "float y=dot(f,u),n=8.;"
   "m/=n;"
   "f*=m;"
   "for(float r=0.;r<n;++r)"
     "{"
       "vec3 E=v+f*r;"
       "vec2 d=a(E)*m;"
       "i+=d;"
       "vec2 p=i+a(E,u,a(E,u,l),8.);"
       "vec3 b=exp(-R*p.x-o*p.y);"
       "z+=b*d.x;"
       "s+=b*d.y;"
     "}"
   "return x*exp(-R*i.x-o*i.y)+S*(1.+y*y)*(z*R*.0597+s*c*.0196/pow(1.58-1.52*y,1.5));"
 "}"
 "float g(vec3 f)"
 "{"
   "return f.y+=.2,min(length(f-v.zxx*1.4),length(f+v.zxx*1.4))-.8;"
 "}"
 "vec3 E=vec3(8.,4.,8.);"
 "float b=0.;"
 "float w(vec3 v)"
 "{"
   "v.z-=4.;"
   "float f=-e(v,E);"
   "if(t>768.)"
     "f=max(max(f,e(v,E+.3)),-e(i(v-u*E,vec3(7.,9.,2.)),b*vec3(3.,4.,.5)));"
   "return f;"
 "}"
 "mat3 C=d(y)*h(x);"
 "float T(vec3 v)"
 "{"
   "return e(C*(v-vec3((t-320.)*3./256.-1.5,-.2,0.)),vec3(.3));"
 "}"
 "float q(vec3 v)"
 "{"
   "float f=g(v);"
   "if(t>320.&&t<576.)"
     "f=d(f,T(v),.3);"
   "return min(w(v),f);"
 "}"
 "vec3 Z(vec3 f)"
 "{"
   "return normalize(vec3(q(f+v.yxx)-q(f-v.yxx),q(f+v.xyx)-q(f-v.xyx),q(f+v.xxy)-q(f-v.xxy)));"
 "}"
 "float T(vec3 v,vec3 f,float x,float m,int y)"
 "{"
   "for(int i=0;i<y;++i)"
     "{"
       "float s=q(v+f*x);"
       "x+=s;"
       "if(s<.001*x||x>m)"
         "break;"
     "}"
   "return x;"
 "}"
 "void main()"
 "{"
   "vec2 f=gl_FragCoord.xy/RES*2.-1.;"
   "f.x*=2.*RES.x/RES.y;"
   "vec3 m=vec3(0.,1.5,3.9),x=v.xzx,r=vec3(0.),s=normalize(vec3(f,-2.));"
   "if(t<384.)"
     "{"
       "float y=smoothstep(64.,384.,t);"
       "r=mix(vec3(0.,.4,-4.),vec3(0.),y);"
       "m=mix(vec3(0.,.4,-2.),vec3(0.,1.5,3.9),y);"
     "}"
   "else"
     "{"
       "float y=smoothstep(432.,640.,t);"
       "m=mix(m,vec3(0.,-.2,5.9),y);"
       "float z=smoothstep(512.,704.,t);"
       "E=mix(E,vec3(2.4,3.,6.),z);"
       "b=mix(.25,1.,smoothstep(768.,832.,t));"
       "if(t>832.)"
         "{"
           "float n=smoothstep(864.,992.,t);"
           "r=mix(vec3(-1.4,-.2,0.),vec3(-1.3,-.5,-.4),n);"
           "m=mix(vec3(-1.1,.2,1.2),vec3(-.9,.2,.8),n);"
           "if(t>960.)"
             "r=mix(vec3(-1.4,3.2,0.),vec3(-1.4,0.,0.),smoothstep(1088.,1216.,t)),m=vec3(1.,.1,6.);"
         "}"
     "}"
   "s=e(m,r,x)*s;"
   "u=normalize(mix(vec3(0.,1.,0.),vec3(-.2,.01,-.3),smoothstep(768.,1216.,t)));"
   "float z=t;"
   "vec3 y=vec3(0.);"
   "for(int n=0;n<16;++n)"
     "{"
       "vec3 c=m,S=s,k=vec3(0.),R=vec3(1.);"
       "for(int C=0;C<4;++C)"
         "{"
           "float q=a(c,S,l);"
           "int o=0;"
           "float h=S.y<-.0001?(-1.-c.y)/S.y:q;"
           "if(h<q)"
             "q=h,o=1;"
           "float Y=min(q,20.),X=T(c,S,0.,Y,40);"
           "if(X<Y)"
             "q=X,o=2;"
           "vec3 W=vec3(0.),V=vec3(.75,.75,.73),U=vec3(0.,1.,0.);"
           "float Q=.04;"
           "if(o==0)"
             "{"
               "W=d(c,S,q,vec3(0.));"
               "if(any(isnan(W)))"
                 "break;"
               "V=vec3(0.);"
               "c=c+S*q;"
             "}"
           "else"
             "{"
               "c=c+S*q;"
               "if(o==1)"
                 "Q=.01+.6*smoothstep(.4,.5,i(c.xz*2.));"
               "else"
                 " if(o==2)"
                   "{"
                     "U=Z(c);"
                     "c+=U*.01;"
                     "if(g(c)>w(c))"
                       "{"
                         "Q=.4;"
                         "if(U.z>.9)"
                           "{"
                             "vec2 P=floor(c.xy*4.);"
                             "float O=mod(-t/2.+p(P.y)*64.,64.)-32.;"
                             "W=vec3(3.*step(O,P.x)*step(P.x,O+1.+16.*p(P.y+32.))*step(P.x,(-t+718.)/2.));"
                           "}"
                       "}"
                     "else"
                       " Q=.01+.1*smoothstep(.4,.5,.5*(i(U.yx*16.)+i(U.xz*16.)));"
                   "}"
             "}"
           "k+=R*W;"
           "R*=V.xyz;"
           "if(all(lessThan(R,vec3(.001))))"
             "break;"
           "S=reflect(S,U);"
           "S=normalize(mix(S,(vec3(p(z+=S.x),p(z+=S.y),p(z+=S.z))-.5)*2.,Q));"
           "S*=sign(dot(U,S));"
         "}"
       "y+=k;"
     "}"
   "gl_FragColor=vec4(y/16.,.3);"
 "}";

#endif // INTRO_H_
