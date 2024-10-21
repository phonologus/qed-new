#ifndef UINCLUDED
#define UINCLUDED

#define u_incr(p) u_incrp(&(p))
#define u_decr(p) u_decrp(&(p))
#define u_postincr(p) u_postincrp(&(p))
#define u_postdecr(p) u_postdecrp(&(p))

#define u_load(p,c,q) u_loadp(&(p),c,&(q))
#define u_loadc(p,c,q) u_loadcp(&(p),c,&(q))
#define u_loadf(p,c,f) u_loadfp(&(p),c,f)
#define u_loadcf(p,c,f) u_loadcfp(&(p),c,f)

#endif
