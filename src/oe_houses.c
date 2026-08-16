#include "oe_internal.h"
#include "erfa.h"
#include <math.h>
#include <string.h>

static double normpi(double x){x=remainder(x,2*OE_PI);return x<=-OE_PI?x+2*OE_PI:x;}
static void equatorial(double lambda,double eps,double *ra,double *dec){
    *ra=atan2(sin(lambda)*cos(eps),cos(lambda));if(*ra<0)*ra+=2*OE_PI;
    *dec=asin(sin(lambda)*sin(eps));
}
static int semi_arc(double phi,double dec,double *s){double q=-tan(phi)*tan(dec);if(q<=-1||q>=1)return 0;*s=acos(q);return 1;}
static double true_sidereal(const oe_time *t){return eraGst06a(2400000.5,t->jd_ut1-2400000.5,2400000.5,t->jd_tt-2400000.5);}

typedef struct cusp_problem{double theta,phi,eps,fraction;int nocturnal;} cusp_problem;
static int cusp_f(double lambda,const cusp_problem *p,double *f){
    double ra,dec,s,n,target,h;equatorial(lambda,p->eps,&ra,&dec);if(!semi_arc(p->phi,dec,&s))return 0;n=OE_PI-s;
    target=p->nocturnal?(-s-p->fraction*n):(-p->fraction*s);h=normpi(p->theta-ra);
    while(h-target>OE_PI)h-=2*OE_PI;
    while(h-target<-OE_PI)h+=2*OE_PI;
    *f=h-target;return 1;
}
static int solve_cusp(double lo,double hi,const cusp_problem *p,double *root){
    double fl,fh,fm,mid,x,previous;int i,found=0;
    previous=lo;if(!cusp_f(previous,p,&fl))return 0;
    for(i=1;i<=256;i++){
        x=lo+(hi-lo)*i/256.0;
        if(!cusp_f(x,p,&fh))return 0;
        if(fl==0||fl*fh<=0){lo=previous;hi=x;found=1;break;}
        previous=x;fl=fh;
    }
    if(!found)return 0;
    for(i=0;i<80;i++){mid=(lo+hi)/2;if(!cusp_f(mid,p,&fm))return 0;if(fabs(fm)<1e-14){*root=mid;return 1;}if(fl*fm<=0){hi=mid;fh=fm;}else{lo=mid;fl=fm;}}
    *root=(lo+hi)/2;(void)fh;return 1;
}
oe_status oe_placidus_houses(const oe_time *t,double latd,double lond,oe_house_result *o){
    double phi,eps,theta,mc,asc,ic,desc,r;int k;cusp_problem p;
    if(!t||!o||t->struct_size<sizeof(*t)||t->abi_version!=OE_ABI_VERSION||!isfinite(latd)||!isfinite(lond)||fabs(latd)>=90||fabs(lond)>180)return OE_ERR_INVALID_ARGUMENT;
    phi=latd*OE_D2R;eps=eraObl06(2400000.5,t->jd_tt-2400000.5);theta=true_sidereal(t)+lond*OE_D2R;theta=fmod(theta,2*OE_PI);if(theta<0)theta+=2*OE_PI;
    mc=atan2(sin(theta),cos(theta)*cos(eps));if(mc<0)mc+=2*OE_PI;
    asc=atan2(-cos(theta),sin(eps)*tan(phi)+cos(eps)*sin(theta))+OE_PI;asc=fmod(asc,2*OE_PI);if(asc<0)asc+=2*OE_PI;
    while(asc<=mc)asc+=2*OE_PI;
    while(asc-mc>OE_PI)asc-=2*OE_PI;
    if(asc<=mc)asc+=2*OE_PI;
    ic=mc+OE_PI;desc=asc+OE_PI;
    memset(o,0,sizeof(*o));o->struct_size=sizeof(*o);o->abi_version=OE_ABI_VERSION;
    o->cusps_deg[9]=oe_norm_deg(mc*OE_R2D);o->cusps_deg[0]=oe_norm_deg(asc*OE_R2D);
    o->cusps_deg[3]=oe_norm_deg(ic*OE_R2D);o->cusps_deg[6]=oe_norm_deg(desc*OE_R2D);
    p.theta=theta;p.phi=phi;p.eps=eps;p.nocturnal=0;
    for(k=1;k<=2;k++){p.fraction=k/3.0;if(!solve_cusp(mc,asc,&p,&r))return OE_ERR_HOUSES_UNDEFINED;o->cusps_deg[9+k]=oe_norm_deg(r*OE_R2D);o->cusps_deg[3+k]=oe_norm_deg(r*OE_R2D+180);}
    p.nocturnal=1;
    for(k=1;k<=2;k++){p.fraction=k/3.0;if(!solve_cusp(asc,ic,&p,&r))return OE_ERR_HOUSES_UNDEFINED;o->cusps_deg[k]=oe_norm_deg(r*OE_R2D);o->cusps_deg[k+6]=oe_norm_deg(r*OE_R2D+180);}
    o->ascendant_deg=oe_norm_deg(asc*OE_R2D);o->midheaven_deg=oe_norm_deg(mc*OE_R2D);o->armc_deg=oe_norm_deg(theta*OE_R2D);return OE_OK;
}

oe_status oe_placidus_house_position(const oe_time *t,double latd,double lond,double rad,double decd,double *out){
    double phi,dec,theta,h,s,n,x;
    if(!t||!out||!isfinite(latd)||!isfinite(lond)||!isfinite(rad)||!isfinite(decd)||fabs(latd)>=90||fabs(decd)>90||fabs(lond)>180)return OE_ERR_INVALID_ARGUMENT;
    phi=latd*OE_D2R;dec=decd*OE_D2R;theta=true_sidereal(t)+lond*OE_D2R;h=normpi(theta-rad*OE_D2R);if(!semi_arc(phi,dec,&s))return OE_ERR_HOUSES_UNDEFINED;n=OE_PI-s;
    if(h>=0&&h<=s)x=10.0-3.0*h/s;
    else if(h>s)x=7.0-3.0*(h-s)/n;
    else if(h>=-s)x=10.0+3.0*(-h)/s;
    else x=4.0-3.0*(h+OE_PI)/n;
    while(x>=13)x-=12;
    while(x<1)x+=12;
    *out=x;return OE_OK;
}
