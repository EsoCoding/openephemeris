#include "oe_internal.h"
#include <limits.h>
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
static oe_status eval_chebyshev_segment(const oe_spk *s,const oe_spk_segment *g,
                                        double et,oe_state *o) {
    double init,intlen,rsize_d,n_d,mid,rad,x,*r; int rsize,n,index,degree,j; int64_t a;
    if(!get_double(s,g->last_addr-3,&init)||!get_double(s,g->last_addr-2,&intlen)||
       !get_double(s,g->last_addr-1,&rsize_d)||!get_double(s,g->last_addr,&n_d))return OE_ERR_BAD_KERNEL;
    if(!isfinite(init)||!isfinite(intlen)||!isfinite(rsize_d)||!isfinite(n_d)||
       rsize_d<5.0||rsize_d>(double)INT_MAX||n_d<1.0||n_d>(double)INT_MAX)
        return OE_ERR_BAD_KERNEL;
    rsize=(int)rsize_d;n=(int)n_d;
    if(rsize_d!=(double)rsize||n_d!=(double)n||intlen<=0)return OE_ERR_BAD_KERNEL;
    index=(int)floor((et-init)/intlen);if(index==n&&et<=g->last_et)index=n-1;if(index<0||index>=n)return OE_ERR_NO_COVERAGE;
    a=g->first_addr+(int64_t)index*rsize;r=(double*)malloc((size_t)rsize*sizeof(double));if(!r)return OE_ERR_IO;
    for(j=0;j<rsize;j++)if(!get_double(s,a+j,&r[j])||!isfinite(r[j])){free(r);return OE_ERR_BAD_KERNEL;}
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

static oe_status eval_type21_segment(const oe_spk *s,const oe_spk_segment *g,
                                     double et,oe_state *o) {
    double maxdim_d,nrec_d,epoch,line[4*OE_SPK21_MAX_TERMS+11];
    double fc[OE_SPK21_MAX_TERMS+1]={1.0};
    double wc[OE_SPK21_MAX_TERMS+1]={0.0};
    double w[OE_SPK21_MAX_TERMS+3]={0.0};
    double delta,tp,sum;
    int maxdim,nrec,ndir,dlsize,low,high,index,j,i;
    int kqmax1,kq[3],mq2,ks,ks1,jx;
    int64_t epoch_base,record_address;
    if(!get_double(s,g->last_addr-1,&maxdim_d)||
       !get_double(s,g->last_addr,&nrec_d)) return OE_ERR_BAD_KERNEL;
    if(!isfinite(maxdim_d)||!isfinite(nrec_d)||maxdim_d<15.0||
       maxdim_d>(double)OE_SPK21_MAX_TERMS||nrec_d<1.0||
       nrec_d>(double)INT_MAX||
       nrec_d>(double)((g->last_addr-g->first_addr+1)/71))
        return OE_ERR_BAD_KERNEL;
    maxdim=(int)maxdim_d;nrec=(int)nrec_d;
    if(maxdim_d!=(double)maxdim||nrec_d!=(double)nrec) return OE_ERR_BAD_KERNEL;
    dlsize=4*maxdim+11;ndir=nrec/100;
    epoch_base=g->last_addr-ndir-2-nrec+1;
    if(epoch_base!=g->first_addr+(int64_t)nrec*dlsize) return OE_ERR_BAD_KERNEL;

    low=0;high=nrec;
    while(low<high){
        int middle=low+(high-low)/2;
        if(!get_double(s,epoch_base+middle,&epoch)||!isfinite(epoch)) return OE_ERR_BAD_KERNEL;
        if(epoch<et)low=middle+1;else high=middle;
    }
    index=low;
    if(index>=nrec) return OE_ERR_NO_COVERAGE;
    record_address=g->first_addr+(int64_t)index*dlsize;
    for(j=0;j<dlsize;j++)
        if(!get_double(s,record_address+j,&line[j])||!isfinite(line[j]))
            return OE_ERR_BAD_KERNEL;

    if(line[4*maxdim+7]<3.0||line[4*maxdim+7]>(double)(maxdim+1))
        return OE_ERR_BAD_KERNEL;
    kqmax1=(int)line[4*maxdim+7];
    if(line[4*maxdim+7]!=(double)kqmax1)return OE_ERR_BAD_KERNEL;
    for(i=0;i<3;i++)
        if(line[4*maxdim+8+i]<1.0||line[4*maxdim+8+i]>(double)maxdim)
            return OE_ERR_BAD_KERNEL;
        else {
            kq[i]=(int)line[4*maxdim+8+i];
            if(line[4*maxdim+8+i]!=(double)kq[i])return OE_ERR_BAD_KERNEL;
        }

    delta=et-line[0];tp=delta;mq2=kqmax1-2;ks=kqmax1-1;
    for(j=1;j<=mq2;j++){
        if(line[j]==0.0)return OE_ERR_BAD_KERNEL;
        fc[j]=tp/line[j];wc[j]=delta/line[j];tp=delta+line[j];
    }
    for(j=1;j<=kqmax1;j++)w[j]=1.0/(double)j;
    jx=0;ks1=ks-1;
    while(ks>=2){
        ++jx;
        for(j=1;j<=jx;j++)w[j+ks]=fc[j]*w[j+ks1]-wc[j]*w[j+ks];
        ks=ks1;--ks1;
    }
    for(i=0;i<3;i++){
        const double *dt=line+maxdim+7+i*maxdim;
        sum=0.0;
        for(j=kq[i];j>=1;j--)sum+=dt[j-1]*w[j+ks];
        ((double*)&o->p)[i]=line[maxdim+1+2*i]+delta*(line[maxdim+2+2*i]+delta*sum);
    }
    for(j=1;j<=jx;j++)w[j+ks]=fc[j]*w[j+ks1]-wc[j]*w[j+ks];
    --ks;
    for(i=0;i<3;i++){
        const double *dt=line+maxdim+7+i*maxdim;
        sum=0.0;
        for(j=kq[i];j>=1;j--)sum+=dt[j-1]*w[j+ks];
        ((double*)&o->v)[i]=line[maxdim+2+2*i]+delta*sum;
    }
    for(i=0;i<3;i++)
        if(!isfinite(((double*)&o->p)[i])||!isfinite(((double*)&o->v)[i]))
            return OE_ERR_NUMERIC;
    return OE_OK;
}

static oe_status eval_segment(const oe_spk *s,const oe_spk_segment *g,
                              double et,oe_state *o) {
    if(g->frame!=1)return OE_ERR_UNSUPPORTED_KERNEL;
    if(g->type==2||g->type==3)return eval_chebyshev_segment(s,g,et,o);
    if(g->type==21)return eval_type21_segment(s,g,et,o);
    return OE_ERR_UNSUPPORTED_KERNEL;
}

oe_status oe_spk_direct_state(const oe_spk *s,int target,int center,double et,
                              oe_state *o) {
    size_t i;
    if(!s||!o||!isfinite(et))return OE_ERR_INVALID_ARGUMENT;
    for(i=s->segment_count;i>0;i--){const oe_spk_segment *g=&s->segments[i-1];
        if(g->target==target&&g->center==center&&et>=g->first_et&&et<=g->last_et)
            return eval_segment(s,g,et,o);
    }
    return OE_ERR_NO_COVERAGE;
}

static oe_status relative_ssb(const oe_spk *s,int target,double et,
                              oe_state *o,int depth) {
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
    st=relative_ssb(s,target,et,&a,0);if(st!=OE_OK)return st;
    st=relative_ssb(s,center,et,&b,0);if(st!=OE_OK)return st;
    o->p.x=a.p.x-b.p.x;o->p.y=a.p.y-b.p.y;o->p.z=a.p.z-b.p.z;
    o->v.x=a.v.x-b.v.x;o->v.y=a.v.y-b.v.y;o->v.z=a.v.z-b.v.z;return OE_OK;
}
