#include "openephemeris/oe.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static double separation(double a,double b){return fabs(remainder(a-b,360.0));}
static void test_time(void){
    oe_time t;assert(oe_time_from_utc(2000,1,1,12,0,0.0,0.0,&t)==OE_OK);
    assert(fabs(t.jd_tt-2451545.0007428704)<1e-9);assert(fabs(t.jd_ut1-2451545.0)<1e-12);
    assert(t.quality&OE_TIME_LEAP_SECONDS_KNOWN);
    assert(oe_time_from_utc(1900,2,29,0,0,0,NAN,&t)==OE_ERR_INVALID_ARGUMENT);
    assert(oe_time_from_utc(2000,2,29,0,0,0,NAN,&t)==OE_OK);
}
static void test_houses(void){
    oe_time t;oe_house_result h;int i;double hp;
    assert(oe_time_from_jd(2451545.0,2451545.0,&t)==OE_OK);
    assert(oe_placidus_houses(&t,0.0,79.5394,&h)==OE_OK);
    for(i=0;i<6;i++)assert(separation(h.cusps_deg[i+6],h.cusps_deg[i]+180)<1e-8);
    assert(separation(h.ascendant_deg,h.cusps_deg[0])<1e-12);
    assert(separation(h.midheaven_deg,h.cusps_deg[9])<1e-12);
    assert(oe_placidus_house_position(&t,0,79.5394,h.armc_deg,0,&hp)==OE_OK);
    assert(fabs(hp-10)<1e-8);
    assert(oe_placidus_houses(&t,90,0,&h)==OE_ERR_INVALID_ARGUMENT);
    for(i=-50;i<=50;i+=10){
        assert(oe_placidus_houses(&t,(double)i,5.0,&h)==OE_OK);
        assert(h.ascendant_deg>=0&&h.ascendant_deg<360);
    }
}
static void test_abi(void){
    FILE *file;unsigned char invalid[1024]={0};oe_ephemeris *e=(oe_ephemeris*)1;
    assert(strcmp(oe_version(),"0.1.0")==0);
    assert(strcmp(oe_status_string(OE_ERR_NO_COVERAGE),"no ephemeris coverage")==0);
    assert(oe_ephemeris_open(NULL,NULL,NULL)==OE_ERR_INVALID_ARGUMENT);
    file=fopen("oe-invalid.bsp","wb");assert(file);assert(fwrite(invalid,1,sizeof(invalid),file)==sizeof(invalid));assert(fclose(file)==0);
    assert(oe_ephemeris_open("oe-invalid.bsp",NULL,&e)==OE_ERR_BAD_KERNEL);assert(e==NULL);assert(remove("oe-invalid.bsp")==0);
}
static void test_de440_optional(void){
    const char *path=getenv("OE_TEST_KERNEL");oe_ephemeris *e;oe_time t;oe_position_result p;
    if(!path)return;assert(oe_ephemeris_open(path,NULL,&e)==OE_OK);
    assert(oe_time_from_utc(2000,1,1,12,0,0,0,&t)==OE_OK);assert(oe_position(e,OE_SUN,&t,&p)==OE_OK);
    /* JPL Horizons DE441, apparent geocentric ecliptic-of-date, 2000-01-01 12:00 UTC. */
    assert(separation(p.longitude_deg,280.3689092)<0.1/3600.0);
    assert(fabs(p.latitude_deg-0.0002381)<0.1/3600.0);oe_ephemeris_close(e);
}
int main(void){test_abi();test_time();test_houses();test_de440_optional();puts("OpenEphemeris tests passed");return 0;}
