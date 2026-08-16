#include "oe_internal.h"
#include <math.h>
#include <string.h>

typedef struct { int y, m, d, tai_utc; } leap_entry;
static const leap_entry leaps[] = {
    {1972,1,1,10},{1972,7,1,11},{1973,1,1,12},{1974,1,1,13},
    {1975,1,1,14},{1976,1,1,15},{1977,1,1,16},{1978,1,1,17},
    {1979,1,1,18},{1980,1,1,19},{1981,7,1,20},{1982,7,1,21},
    {1983,7,1,22},{1985,7,1,23},{1988,1,1,24},{1990,1,1,25},
    {1991,1,1,26},{1992,7,1,27},{1993,7,1,28},{1994,7,1,29},
    {1996,1,1,30},{1997,7,1,31},{1999,1,1,32},{2006,1,1,33},
    {2009,1,1,34},{2012,7,1,35},{2015,7,1,36},{2017,1,1,37}
};

double oe_norm_deg(double x) {
    x = fmod(x, 360.0); return x < 0.0 ? x + 360.0 : x;
}

static int is_leap(int y) { return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0); }
static int valid_date(int y, int m, int d) {
    static const int dm[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int n;
    if (m < 1 || m > 12) return 0;
    n = dm[m-1] + (m == 2 && is_leap(y)); return d >= 1 && d <= n;
}
static double calendar_jd(int y, int m, int d, int h, int min, double sec) {
    int a = (14 - m) / 12, yy = y + 4800 - a, mm = m + 12*a - 3;
    int64_t jdn = d + (153*mm+2)/5 + 365LL*yy + yy/4 - yy/100 + yy/400 - 32045;
    return (double)jdn - 0.5 + (h + (min + sec/60.0)/60.0)/24.0;
}
static double decimal_year(int y, int m, int d) {
    static const int before[] = {0,0,31,59,90,120,151,181,212,243,273,304,334};
    int doy = before[m] + d + (m > 2 && is_leap(y));
    return y + ((double)doy - 0.5) / (365.0 + is_leap(y));
}
static int date_cmp(int y,int m,int d,const leap_entry *e) {
    if (y != e->y) return y < e->y ? -1 : 1;
    if (m != e->m) return m < e->m ? -1 : 1;
    return d == e->d ? 0 : (d < e->d ? -1 : 1);
}

double oe_delta_t_seconds(double y) {
    double u, t;
    if (y < 1800.0) { u = (y - 1820.0)/100.0; return -20.0 + 32.0*u*u; }
    if (y < 1860.0) { t=y-1800.0; return 13.72-0.332447*t+0.0068612*t*t+0.0041116*t*t*t-0.00037436*pow(t,4)+0.0000121272*pow(t,5)-0.0000001699*pow(t,6)+0.000000000875*pow(t,7); }
    if (y < 1900.0) { t=y-1860.0; return 7.62+0.5737*t-0.251754*t*t+0.01680668*pow(t,3)-0.0004473624*pow(t,4)+pow(t,5)/233174.0; }
    if (y < 1920.0) { t=y-1900.0; return -2.79+1.494119*t-0.0598939*t*t+0.0061966*pow(t,3)-0.000197*pow(t,4); }
    if (y < 1941.0) { t=y-1920.0; return 21.20+0.84493*t-0.076100*t*t+0.0020936*pow(t,3); }
    if (y < 1961.0) { t=y-1950.0; return 29.07+0.407*t-t*t/233.0+pow(t,3)/2547.0; }
    if (y < 1986.0) { t=y-1975.0; return 45.45+1.067*t-t*t/260.0-pow(t,3)/718.0; }
    if (y < 2005.0) { t=y-2000.0; return 63.86+0.3345*t-0.060374*t*t+0.0017275*pow(t,3)+0.000651814*pow(t,4)+0.00002373599*pow(t,5); }
    if (y < 2050.0) { t=y-2000.0; return 62.92+0.32217*t+0.005589*t*t; }
    if (y < 2150.0) return -20.0+32.0*pow((y-1820.0)/100.0,2)-0.5628*(2150.0-y);
    u=(y-1820.0)/100.0; return -20.0+32.0*u*u;
}

oe_status oe_time_from_jd(double tt, double ut1, oe_time *out) {
    if (!out || !isfinite(tt) || !isfinite(ut1)) return OE_ERR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out)); out->struct_size=sizeof(*out); out->abi_version=OE_ABI_VERSION;
    out->jd_tt=tt; out->jd_ut1=ut1; out->quality=OE_TIME_EXACT; return OE_OK;
}

oe_status oe_time_from_utc(int y,int m,int d,int h,int min,double sec,double dut1,oe_time *out) {
    double jd, dy, dt; int tai=-1; size_t i; uint32_t q=0;
    if (!out || !valid_date(y,m,d) || h<0 || h>23 || min<0 || min>59 ||
        !isfinite(sec) || sec<0.0 || sec>=61.0 || y<1800 || y>2200)
        return OE_ERR_INVALID_ARGUMENT;
    jd=calendar_jd(y,m,d,h,min,sec); dy=decimal_year(y,m,d);
    for (i=0;i<sizeof(leaps)/sizeof(leaps[0]);++i)
        if (date_cmp(y,m,d,&leaps[i])>=0) tai=leaps[i].tai_utc;
    memset(out,0,sizeof(*out)); out->struct_size=sizeof(*out); out->abi_version=OE_ABI_VERSION;
    if (isfinite(dut1)) out->jd_ut1=jd+dut1/OE_DAY_S;
    else { out->jd_ut1=jd; q|=OE_TIME_DUT1_MODELED; }
    if (tai>=0) { out->jd_tt=jd+(tai+32.184)/OE_DAY_S; q|=OE_TIME_LEAP_SECONDS_KNOWN; if(y>2026) q|=OE_TIME_FUTURE_UTC; }
    else { dt=oe_delta_t_seconds(dy); out->jd_tt=out->jd_ut1+dt/OE_DAY_S; q|=OE_TIME_DELTA_T_MODELED; }
    out->quality=q; return OE_OK;
}

double oe_mean_obliquity_rad(double jd) {
    double t=(jd-OE_J2000)/36525.0;
    double a=84381.406-46.836769*t-0.0001831*t*t+0.00200340*t*t*t-5.76e-7*pow(t,4)-4.34e-8*pow(t,5);
    return a/3600.0*OE_D2R;
}

double oe_gmst_deg(double ut1,double tt) {
    double d=ut1-OE_J2000, t=(tt-OE_J2000)/36525.0;
    return oe_norm_deg(280.46061837+360.98564736629*d+0.000387933*t*t-t*t*t/38710000.0);
}
