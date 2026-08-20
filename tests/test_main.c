#include "openephemeris/oe.h"
#include "oe_internal.h"
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
    static const int systems[] = {
        OE_HOUSE_PLACIDUS, OE_HOUSE_KOCH, OE_HOUSE_PORPHYRY,
        OE_HOUSE_REGIOMONTANUS, OE_HOUSE_CAMPANUS, OE_HOUSE_EQUAL,
        OE_HOUSE_WHOLE_SIGN, OE_HOUSE_ALCABITIUS, OE_HOUSE_TOPOCENTRIC,
        OE_HOUSE_MORINUS, OE_HOUSE_MERIDIAN, OE_HOUSE_VEHLOW,
        OE_HOUSE_EQUAL_MC
    };
    oe_house_result h;int i, s_idx;double hp;
    const double jd_ut=2451545.0;
    assert(oe_houses_at_jd(jd_ut,0.0,79.5394,OE_HOUSE_PLACIDUS,&h)==OE_OK);
    for(i=0;i<6;i++)assert(separation(h.cusps_deg[i+6],h.cusps_deg[i]+180)<1e-8);
    assert(separation(h.ascendant_deg,h.cusps_deg[0])<1e-12);
    assert(separation(h.midheaven_deg,h.cusps_deg[9])<1e-12);
    assert(oe_house_position_at_jd(jd_ut,0,79.5394,OE_HOUSE_PLACIDUS,h.armc_deg,0,&hp)==OE_OK);
    assert(fabs(hp-10)<1e-8);
    assert(oe_houses_at_jd(jd_ut,90,0,OE_HOUSE_PLACIDUS,&h)==OE_ERR_INVALID_ARGUMENT);
    for(i=-50;i<=50;i+=10){
        for(s_idx=0;s_idx<(int)(sizeof(systems)/sizeof(systems[0]));s_idx++){
            assert(oe_houses_at_jd(jd_ut,(double)i,5.0,systems[s_idx],&h)==OE_OK);
            assert(h.ascendant_deg>=0&&h.ascendant_deg<360);
            assert(h.midheaven_deg>=0&&h.midheaven_deg<360);
            for(int c=0;c<12;c++){
                assert(h.cusps_deg[c]>=0&&h.cusps_deg[c]<360);
            }
        }
    }
}
static void test_abi(void){
    FILE *file;unsigned char invalid[1024]={0};oe_ephemeris *e=(oe_ephemeris*)1;
    assert(strcmp(oe_version(),"0.4.0")==0);
    assert(strcmp(oe_status_string(OE_ERR_NO_COVERAGE),"no ephemeris coverage")==0);
    assert(oe_ephemeris_open(NULL,NULL,NULL)==OE_ERR_INVALID_ARGUMENT);
    file=fopen("oe-invalid.bsp","wb");assert(file);assert(fwrite(invalid,1,sizeof(invalid),file)==sizeof(invalid));assert(fclose(file)==0);
    assert(oe_ephemeris_open("oe-invalid.bsp",NULL,&e)==OE_ERR_BAD_KERNEL);assert(e==NULL);assert(remove("oe-invalid.bsp")==0);
}
static void test_de440_optional(void){
    const char *path=getenv("OE_TEST_KERNEL");oe_ephemeris *e;oe_position_result p;
    if(!path)return;assert(oe_ephemeris_open(path,NULL,&e)==OE_OK);
    assert(oe_position_at_jd(e,OE_SUN,2451545.0,&p)==OE_OK);
    /* JPL Horizons DE441, apparent geocentric ecliptic-of-date, 2000-01-01 12:00 UTC. */
    assert(separation(p.longitude_deg,280.3689092)<0.1/3600.0);
    assert(fabs(p.latitude_deg-0.0002381)<0.1/3600.0);oe_ephemeris_close(e);
}
static void test_spice_state_matrix_optional(void){
    static const double fixtures[][7]={
        {10,2.6499033677425094e7,-1.3275741733833946e8,-5.7556718470538191e7,29.794260070421970,5.0180523087861113,2.1753938028266693},
        {301,-2.9160838463343546e5,-2.6671683339423337e5,-7.6102487099902020e4,.64353138771903273,-.66608768409163044,-.30132570498227307},
        {1,7.0373073215713687e6,-1.9268538511181986e8,-8.7549491150369614e7,66.789251927701159,-3.5116229745961567,-6.2177273406405540},
        {2,-8.0957460374765486e7,-1.3967994611322212e8,-5.3870531424917541e7,31.176166099685414,-26.999766122896620,-12.316441670441332},
        {4,2.3454717431925747e8,-1.3254779816498069e8,-6.3085880783777490e7,30.956932507027226,28.936462009085030,13.114565702339517},
        {5,6.2506661727535403e8,2.7662893277821511e8,1.0333762865246880e8,21.884422381854677,15.201550124152199,6.7331128562947935},
        {6,9.8488415789714861e8,7.9095824074344790e8,2.8274414710236400e8,22.362238072480938,11.127228630143286,5.0183273572874452},
        {7,2.1854740534366508e9,-2.0036682200931020e9,-9.0752545331753361e8,34.431284306373570,9.2809064908884427,3.9768178548429898},
        {8,2.5415454622065272e9,-3.5705315214703999e9,-1.5272701762972105e9,34.260162120246754,7.9068586705521469,3.2468456171777111},
        {9,-1.4508326323316393e9,-4.3183365125613136e9,-9.1829670383106232e8,35.038411429733976,3.0656732168358389,-.015123085085968491}};
    const char *path=getenv("OE_TEST_KERNEL");oe_spk spk;oe_state state;size_t i,j;
    if(!path)return;assert(oe_spk_open(&spk,path)==OE_OK);
    for(i=0;i<sizeof(fixtures)/sizeof(fixtures[0]);i++){
        assert(oe_spk_state(&spk,(int)fixtures[i][0],399,0.0,&state)==OE_OK);
        for(j=0;j<3;j++)assert(fabs(((double*)&state.p)[j]-fixtures[i][j+1])<1e-5);
        for(j=0;j<3;j++)assert(fabs(((double*)&state.v)[j]-fixtures[i][j+4])<1e-10);
    }
    oe_spk_close(&spk);
}
static void test_simple_positions_optional(void){
    oe_ephemeris *e;oe_position_result position;oe_house_result houses;
    oe_status status;
    if(!getenv("OE_TEST_KERNEL"))return;
    status=oe_ephemeris_open_default(&e);assert(status==OE_OK);
    assert(oe_position_at_jd(e,OE_SUN,2451545.0,&position)==OE_OK);
    assert(oe_houses_at_jd(2451545.0,52.3676,4.9041,
                           OE_HOUSE_PLACIDUS,&houses)==OE_OK);
    if(getenv("OE_TEST_CHIRON"))assert(oe_position_at_jd(e,OE_CHIRON,2451545.0,&position)==OE_OK);
    else assert(oe_position_at_jd(e,OE_CHIRON,2451545.0,&position)==OE_ERR_NO_COVERAGE);
    oe_ephemeris_close(e);
}
static void test_chiron_type21_optional(void){
    const char *path=getenv("OE_TEST_CHIRON");oe_spk spk;oe_state state;
    if(!path)return;
    assert(oe_spk_open(&spk,path)==OE_OK);
    assert(oe_spk_direct_state(&spk,20002060,10,0.0,&state)==OE_OK);
    /* JPL Horizons JPL#171, heliocentric ICRF state at J2000 TDB, km and km/s. */
    assert(fabs(state.p.x-(-5.280202440571708e8))<1e-3);
    assert(fabs(state.p.y-(-1.297821534197968e9))<1e-3);
    assert(fabs(state.p.z-(-4.392050917157409e8))<1e-3);
    assert(fabs(state.v.x-8.607465369084641)<1e-9);
    assert(fabs(state.v.y-(-6.278987787980275))<1e-9);
    assert(fabs(state.v.z-(-1.429830165842763))<1e-9);
    assert(oe_spk_direct_state(&spk,20002060,10,
           (2378496.5-OE_J2000)*OE_DAY_S,&state)==OE_OK);
    assert(oe_spk_direct_state(&spk,20002060,10,
           (2524593.5-OE_J2000)*OE_DAY_S,&state)==OE_OK);
    assert(oe_spk_direct_state(&spk,20002060,10,
           (2378496.5-OE_J2000)*OE_DAY_S-1.0,&state)==OE_ERR_NO_COVERAGE);
    oe_spk_close(&spk);
}
static void test_astrology(void){
    oe_aspect_info asp;
    /* Signs */
    assert(strcmp(oe_sign_name(OE_SIGN_ARIES), "Aries") == 0);
    assert(strcmp(oe_sign_symbol(OE_SIGN_TAURUS), "♉") == 0);
    assert(oe_longitude_sign(0.0) == OE_SIGN_ARIES);
    assert(oe_longitude_sign(35.5) == OE_SIGN_TAURUS);
    assert(fabs(oe_longitude_in_sign(35.5) - 5.5) < 1e-12);
    assert(oe_sign_element(OE_SIGN_CANCER) == OE_ELEMENT_WATER);
    assert(oe_sign_modality(OE_SIGN_LEO) == OE_MODALITY_FIXED);

    /* Aspects */
    assert(strcmp(oe_aspect_name(OE_ASPECT_TRINE), "Trine") == 0);
    assert(strcmp(oe_aspect_symbol(OE_ASPECT_TRINE), "△") == 0);
    assert(fabs(oe_aspect_angle(OE_ASPECT_TRINE) - 120.0) < 1e-12);

    /* Exact Conjunction */
    assert(oe_aspect_calculate(10.0, 1.0, 10.0, 0.5, 5.0, &asp) == OE_OK);
    assert(asp.type == OE_ASPECT_CONJUNCTION);
    assert(asp.orb_deg < 1e-12);
    assert(asp.flags & OE_ASPECT_EXACT);

    /* Trine: 10° Aries and 131° Leo -> diff 121°, target 120°, orb 1° */
    assert(oe_aspect_calculate(10.0, 1.0, 131.0, 0.5, 5.0, &asp) == OE_OK);
    assert(asp.type == OE_ASPECT_TRINE);
    assert(fabs(asp.orb_deg - 1.0) < 1e-12);

    /* Part of Fortune: ASC = 30°, Moon = 120°, Sun = 45° -> Day: 30 + 120 - 45 = 105° */
    assert(fabs(oe_part_of_fortune(30.0, 45.0, 120.0, 0) - 105.0) < 1e-12);
    /* Night: 30 + 45 - 120 = -45 = 315° */
    assert(fabs(oe_part_of_fortune(30.0, 45.0, 120.0, 1) - 315.0) < 1e-12);
}
static void test_sidereal_and_stars(void){
    oe_time t; oe_sidereal_result s; oe_position_result p; oe_house_result h;
    const oe_fixed_star *star;
    assert(oe_time_from_jd(2451545.0,2451545.0,&t)==OE_OK);
    assert(oe_fixed_star_count() >= 10);
    star = oe_fixed_star_find("aldebaran"); assert(star != NULL);
    assert(oe_fixed_star_position_at_jd(star,2451545.0,&p)==OE_OK);
    assert(p.longitude_deg >= 0.0 && p.longitude_deg < 360.0);
    assert(isfinite(oe_ayanamsa_at_jd(2451545.0,OE_AYANAMSA_LAHIRI,0.0)));
    /* Sidereal and tropical positions are separated by the ayanamsa. */
    /* A kernel is needed for body positions, so exercise houses independently. */
    assert(oe_sidereal_houses_at_jd(2451545.0,52.0,5.0,OE_HOUSE_PLACIDUS,
                              OE_AYANAMSA_LAHIRI,0.0,&h)==OE_OK);
    assert(h.ascendant_deg >= 0.0 && h.ascendant_deg < 360.0);
    (void)s;
}
static void test_jd_wrappers_optional(void){
    const char *path=getenv("OE_TEST_KERNEL");
    oe_ephemeris *e;oe_position_result p;oe_house_result h;
    oe_aspect_info a;
    if(!path)return;
    assert(oe_ephemeris_open(path,NULL,&e)==OE_OK);
    assert(oe_position_at_jd(e,OE_SUN,2451545.0,&p)==OE_OK);
    assert(fabs(p.longitude_deg-280.3689092)<0.1/3600.0);
    assert(oe_houses_at_jd(2451545.0,52.3676,4.9041,
                           OE_HOUSE_WHOLE_SIGN,&h)==OE_OK);
    assert(oe_aspect_between_bodies_at_jd(e,OE_SUN,OE_MOON,
                                          2451545.0,
                                          10.0,&a)==OE_OK);
    oe_ephemeris_close(e);
}

int main(void){
    test_abi();
    test_time();
    test_houses();
    test_astrology();
    test_sidereal_and_stars();
    test_jd_wrappers_optional();
    test_spice_state_matrix_optional();
    test_de440_optional();
    test_chiron_type21_optional();
    test_simple_positions_optional();
    puts("OpenEphemeris tests passed");
    return 0;
}
