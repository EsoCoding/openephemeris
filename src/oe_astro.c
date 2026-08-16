#include "oe_internal.h"
#include "erfa.h"
#include <math.h>
#include <string.h>

static double length(oe_vec3 a){return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);}
static oe_vec3 sub(oe_vec3 a,oe_vec3 b){oe_vec3 r={a.x-b.x,a.y-b.y,a.z-b.z};return r;}
static oe_vec3 scale(oe_vec3 a,double s){oe_vec3 r={a.x*s,a.y*s,a.z*s};return r;}

static oe_status raw_apparent(const oe_ephemeris *e,int target,double jd,
                              double *lon,double *lat,double *dist) {
    const oe_spk *kernel=&e->planetary; double et=(jd-OE_J2000)*OE_DAY_S;
    oe_state earth,targ,sun;oe_vec3 r,u;double lt=0,rm[3][3],q[3],ec[3],dpsi,deps;
    double v[3],bm1,rs;int i;oe_status st;
    if(target==2002060)kernel=&e->chiron;
    st=oe_spk_state(&e->planetary,399,0,et,&earth);if(st!=OE_OK)return st;
    st=oe_spk_state(&e->planetary,10,0,et,&sun);if(st!=OE_OK)return st;
    for(i=0;i<4;i++){
        st=oe_spk_state(kernel,target,0,et-lt,&targ);if(st!=OE_OK)return st;
        r=sub(targ.p,earth.p);lt=length(r)/OE_C_KM_S;
    }
    *dist=length(r)/OE_AU_KM;u=scale(r,1.0/length(r));
    /* Solar deflection followed by annual aberration, using ERFA's IAU model. */
    q[0]=u.x;q[1]=u.y;q[2]=u.z;
    {double eh[3]={earth.p.x-sun.p.x,earth.p.y-sun.p.y,earth.p.z-sun.p.z};
     double em=sqrt(eh[0]*eh[0]+eh[1]*eh[1]+eh[2]*eh[2]);
     eh[0]/=em;eh[1]/=em;eh[2]/=em;eraLdsun(q,eh,em/OE_AU_KM,q);rs=em/OE_AU_KM;}
    v[0]=earth.v.x/OE_C_KM_S;v[1]=earth.v.y/OE_C_KM_S;v[2]=earth.v.z/OE_C_KM_S;
    bm1=sqrt(fmax(0.0,1.0-(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])));eraAb(q,v,rs,bm1,q);
    eraEcm06(2400000.5,jd-2400000.5,rm);eraRxp(rm,q,ec);
    eraNut06a(2400000.5,jd-2400000.5,&dpsi,&deps);
    *lon=oe_norm_deg(atan2(ec[1],ec[0])*OE_R2D+dpsi*OE_R2D);
    *lat=atan2(ec[2],hypot(ec[0],ec[1]))*OE_R2D;return OE_OK;
}

oe_status oe_apparent_position(const oe_ephemeris *e,int target,const oe_time *time,
                               oe_position_result *out) {
    double lon,lat,d,lp,lm,bp,bm,dp,dm,step=1e-3;oe_status s;
    s=raw_apparent(e,target,time->jd_tt,&lon,&lat,&d);if(s!=OE_OK)return s;
    s=raw_apparent(e,target,time->jd_tt+step,&lp,&bp,&dp);if(s!=OE_OK)return s;
    s=raw_apparent(e,target,time->jd_tt-step,&lm,&bm,&dm);if(s!=OE_OK)return s;
    out->longitude_deg=lon;out->latitude_deg=lat;out->distance_au=d;
    out->longitude_speed_deg_per_day=remainder(lp-lm,360.0)/(2*step);
    out->latitude_speed_deg_per_day=(bp-bm)/(2*step);
    out->distance_speed_au_per_day=(dp-dm)/(2*step);return OE_OK;
}
