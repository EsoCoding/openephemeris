#include "oe_internal.h"
#include "erfa.h"
#include <math.h>

static double mean_node(double jd){double t=(jd-OE_J2000)/36525.0;return oe_norm_deg(125.04455501-1934.1361849*t+0.0020762*t*t+pow(t,3)/467410.0-pow(t,4)/60616000.0);}
static double mean_lilith(double jd){double t=(jd-OE_J2000)/36525.0;return oe_norm_deg(83.35324312+4069.01363525*t-0.0103217*t*t-pow(t,3)/80053.0+pow(t,4)/18999000.0+180.0);}

static oe_status osculating(const oe_ephemeris *e,double jd,double *node,double *apogee){
    oe_state s;double et=(jd-OE_J2000)*OE_DAY_S,rm[3][3],p[3],v[3],pe[3],ve[3];
    double h[3],ev[3],r,mu=403503.2357,vdotr;oe_status st=oe_spk_state(&e->planetary,301,399,et,&s);
    if(st!=OE_OK)return st;
    p[0]=s.p.x;p[1]=s.p.y;p[2]=s.p.z;v[0]=s.v.x;v[1]=s.v.y;v[2]=s.v.z;
    eraEcm06(2400000.5,jd-2400000.5,rm);eraRxp(rm,p,pe);eraRxp(rm,v,ve);
    h[0]=pe[1]*ve[2]-pe[2]*ve[1];h[1]=pe[2]*ve[0]-pe[0]*ve[2];h[2]=pe[0]*ve[1]-pe[1]*ve[0];
    *node=oe_norm_deg(atan2(h[0],-h[1])*OE_R2D);
    r=sqrt(pe[0]*pe[0]+pe[1]*pe[1]+pe[2]*pe[2]);vdotr=pe[0]*ve[0]+pe[1]*ve[1]+pe[2]*ve[2];
    ev[0]=((ve[1]*h[2]-ve[2]*h[1])/mu)-pe[0]/r;
    ev[1]=((ve[2]*h[0]-ve[0]*h[2])/mu)-pe[1]/r;
    ev[2]=((ve[0]*h[1]-ve[1]*h[0])/mu)-pe[2]/r;(void)vdotr;
    *apogee=oe_norm_deg(atan2(-ev[1],-ev[0])*OE_R2D);return OE_OK;
}
static oe_status point_lon(const oe_ephemeris *e,oe_body b,double jd,double *lon){
    double n,a;oe_status s;
    switch(b){case OE_MEAN_NODE:*lon=mean_node(jd);return OE_OK;case OE_MEAN_SOUTH_NODE:*lon=oe_norm_deg(mean_node(jd)+180);return OE_OK;
    case OE_MEAN_LILITH:*lon=mean_lilith(jd);return OE_OK;default:break;}
    s=osculating(e,jd,&n,&a);if(s!=OE_OK)return s;
    if(b==OE_TRUE_NODE)*lon=n;else if(b==OE_TRUE_SOUTH_NODE)*lon=oe_norm_deg(n+180);else if(b==OE_TRUE_LILITH)*lon=a;else return OE_ERR_INVALID_ARGUMENT;return OE_OK;
}
oe_status oe_lunar_point(const oe_ephemeris *e,oe_body b,const oe_time *t,oe_position_result *o){
    double x,p,m,step=1e-3;oe_status s=point_lon(e,b,t->jd_tt,&x);if(s!=OE_OK)return s;
    s=point_lon(e,b,t->jd_tt+step,&p);if(s!=OE_OK)return s;s=point_lon(e,b,t->jd_tt-step,&m);if(s!=OE_OK)return s;
    o->longitude_deg=x;o->latitude_deg=0;o->distance_au=0;o->longitude_speed_deg_per_day=remainder(p-m,360.0)/(2*step);return OE_OK;
}
