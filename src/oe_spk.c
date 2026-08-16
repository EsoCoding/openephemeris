#include "oe_internal.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t u32(const unsigned char *p,int le) {
    if(le) return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
    return (uint32_t)p[3]|((uint32_t)p[2]<<8)|((uint32_t)p[1]<<16)|((uint32_t)p[0]<<24);
}
static double f64(const unsigned char *p,int le) {
    unsigned char b[8]; double x; size_t i;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    const int host_le=0;
#else
    const int host_le=1;
#endif
    if(le==host_le) memcpy(b,p,8); else for(i=0;i<8;i++) b[i]=p[7-i];
    memcpy(&x,b,8); return x;
}
static int get_double(const oe_spk *s,int64_t address,double *x) {
    size_t off;
    if(address<1) return 0;
    off=(size_t)(address-1)*8;
    if(off>s->size || s->size-off<8) return 0;
    *x=f64(s->data+off,s->little_endian); return 1;
}

oe_status oe_spk_open(oe_spk *s,const char *path) {
    FILE *f; long n; int nd,ni,forward; unsigned char *rec; size_t count=0;
    if(!s||!path) return OE_ERR_INVALID_ARGUMENT;
    memset(s,0,sizeof(*s));
    f=fopen(path,"rb"); if(!f) return OE_ERR_IO;
    if(fseek(f,0,SEEK_END)!=0||(n=ftell(f))<1024||fseek(f,0,SEEK_SET)!=0){fclose(f);return OE_ERR_BAD_KERNEL;}
    s->data=(unsigned char*)malloc((size_t)n); if(!s->data){fclose(f);return OE_ERR_IO;}
    s->size=(size_t)n; if(fread(s->data,1,s->size,f)!=s->size){fclose(f);oe_spk_close(s);return OE_ERR_IO;} fclose(f);
    if(memcmp(s->data,"DAF/SPK ",8)!=0){oe_spk_close(s);return OE_ERR_BAD_KERNEL;}
    if(memcmp(s->data+88,"LTL-IEEE",8)==0)s->little_endian=1;
    else if(memcmp(s->data+88,"BIG-IEEE",8)==0)s->little_endian=0;
    else {oe_spk_close(s);return OE_ERR_UNSUPPORTED_KERNEL;}
    nd=(int)u32(s->data+8,s->little_endian); ni=(int)u32(s->data+12,s->little_endian);
    forward=(int)u32(s->data+76,s->little_endian);
    if(nd!=2||ni!=6||forward<2){oe_spk_close(s);return OE_ERR_BAD_KERNEL;}
    while(forward!=0) {
        int next,nsum,i; size_t off=(size_t)(forward-1)*1024; int ss=nd+(ni+1)/2;
        if(off>s->size||s->size-off<1024){oe_spk_close(s);return OE_ERR_BAD_KERNEL;}
        rec=s->data+off; next=(int)f64(rec,s->little_endian); nsum=(int)f64(rec+16,s->little_endian);
        if(nsum<0||nsum>(128-3)/ss){oe_spk_close(s);return OE_ERR_BAD_KERNEL;}
        for(i=0;i<nsum;i++) {
            const unsigned char *p=rec+24+(size_t)i*ss*8; oe_spk_segment *g;
            if(count>=OE_MAX_SEGMENTS){oe_spk_close(s);return OE_ERR_UNSUPPORTED_KERNEL;}
            g=&s->segments[count++]; g->first_et=f64(p,s->little_endian); g->last_et=f64(p+8,s->little_endian);
            g->target=(int32_t)u32(p+16,s->little_endian); g->center=(int32_t)u32(p+20,s->little_endian);
            g->frame=(int32_t)u32(p+24,s->little_endian); g->type=(int32_t)u32(p+28,s->little_endian);
            g->first_addr=(int32_t)u32(p+32,s->little_endian); g->last_addr=(int32_t)u32(p+36,s->little_endian);
            if(g->first_addr<1||g->last_addr<g->first_addr||(uint64_t)g->last_addr*8>s->size){oe_spk_close(s);return OE_ERR_BAD_KERNEL;}
        }
        forward=next;
    }
    s->segment_count=count; return count?OE_OK:OE_ERR_BAD_KERNEL;
}

void oe_spk_close(oe_spk *s){if(s){free(s->data);memset(s,0,sizeof(*s));}}

static void cheb(const double *c,int degree,double x,double scale,double *value,double *deriv) {
    double b1=0,b2=0,d1=0,d2=0; int k;
    for(k=degree;k>=1;k--){double b=2*x*b1-b2+c[k];double d=2*x*d1-d2+2*b1; b2=b1;b1=b;d2=d1;d1=d;}
    *value=x*b1-b2+c[0]; *deriv=(x*d1-d2+b1)*scale;
}
static oe_status eval_segment(const oe_spk *s,const oe_spk_segment *g,double et,oe_state *o) {
    double init,intlen,rsize_d,n_d,mid,rad,x,*r; int rsize,n,index,degree,j; int64_t a;
    if(g->type!=2&&g->type!=3) return OE_ERR_UNSUPPORTED_KERNEL;
    if(!get_double(s,g->last_addr-3,&init)||!get_double(s,g->last_addr-2,&intlen)||
       !get_double(s,g->last_addr-1,&rsize_d)||!get_double(s,g->last_addr,&n_d))return OE_ERR_BAD_KERNEL;
    rsize=(int)rsize_d;n=(int)n_d;if(rsize<5||n<1||intlen<=0)return OE_ERR_BAD_KERNEL;
    index=(int)floor((et-init)/intlen);if(index==n&&et<=g->last_et)index=n-1;if(index<0||index>=n)return OE_ERR_NO_COVERAGE;
    a=g->first_addr+(int64_t)index*rsize;r=(double*)malloc((size_t)rsize*sizeof(double));if(!r)return OE_ERR_IO;
    for(j=0;j<rsize;j++)if(!get_double(s,a+j,&r[j])){free(r);return OE_ERR_BAD_KERNEL;}
    mid=r[0];rad=r[1];x=(et-mid)/rad;if(fabs(x)>1.0000000001){free(r);return OE_ERR_BAD_KERNEL;}
    degree=(rsize-2)/(g->type==2?3:6)-1;
    if(degree<0||(2+(g->type==2?3:6)*(degree+1))!=rsize){free(r);return OE_ERR_BAD_KERNEL;}
    for(j=0;j<3;j++){
        double val,vel;cheb(r+2+j*(degree+1),degree,x,1.0/rad,&val,&vel);
        ((double*)&o->p)[j]=val;
        if(g->type==2)((double*)&o->v)[j]=vel;
        else {double ignored;cheb(r+2+(j+3)*(degree+1),degree,x,1.0/rad,&val,&ignored);((double*)&o->v)[j]=val;}
    }
    free(r);return OE_OK;
}
static oe_status relative_ssb(const oe_spk *s,int target,double et,oe_state *o,int depth) {
    size_t i; oe_state local,parent; oe_status st;
    if(target==0){memset(o,0,sizeof(*o));return OE_OK;} if(depth>8)return OE_ERR_BAD_KERNEL;
    for(i=s->segment_count;i>0;i--){const oe_spk_segment *g=&s->segments[i-1];
        if(g->target==target&&et>=g->first_et&&et<=g->last_et){
            st=eval_segment(s,g,et,&local);if(st!=OE_OK)return st;
            st=relative_ssb(s,g->center,et,&parent,depth+1);if(st!=OE_OK)return st;
            o->p.x=local.p.x+parent.p.x;o->p.y=local.p.y+parent.p.y;o->p.z=local.p.z+parent.p.z;
            o->v.x=local.v.x+parent.v.x;o->v.y=local.v.y+parent.v.y;o->v.z=local.v.z+parent.v.z;return OE_OK;
        }}return OE_ERR_NO_COVERAGE;
}
oe_status oe_spk_state(const oe_spk *s,int target,int center,double et,oe_state *o) {
    oe_state a,b;oe_status st;if(!s||!o||!isfinite(et))return OE_ERR_INVALID_ARGUMENT;
    st=relative_ssb(s,target,et,&a,0);if(st!=OE_OK)return st;st=relative_ssb(s,center,et,&b,0);if(st!=OE_OK)return st;
    o->p.x=a.p.x-b.p.x;o->p.y=a.p.y-b.p.y;o->p.z=a.p.z-b.p.z;o->v.x=a.v.x-b.v.x;o->v.y=a.v.y-b.v.y;o->v.z=a.v.z-b.v.z;return OE_OK;
}
