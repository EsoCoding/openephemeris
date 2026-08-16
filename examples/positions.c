#include "openephemeris/oe.h"
#include <stdio.h>

int main(int argc,char **argv){
    static const char *names[]={"Sun","Moon","Mercury","Venus","Mars","Jupiter","Saturn","Uranus","Neptune","Pluto","Mean node","True node","South mean node","South true node","Mean Lilith","True Lilith"};
    oe_ephemeris *e=NULL;oe_time t;oe_position_result p;oe_status s;int body;
    if(argc!=2){fprintf(stderr,"usage: %s /path/to/de440.bsp\n",argv[0]);return 2;}
    s=oe_ephemeris_open(argv[1],NULL,&e);if(s!=OE_OK){fprintf(stderr,"kernel: %s\n",oe_status_string(s));return 1;}
    oe_time_from_utc(2000,1,1,12,0,0.0,0.0,&t);
    for(body=OE_SUN;body<=OE_TRUE_LILITH;body++){
        s=oe_position(e,(oe_body)body,&t,&p);
        if(s!=OE_OK){fprintf(stderr,"%s: %s\n",names[body],oe_status_string(s));oe_ephemeris_close(e);return 1;}
        printf("%-16s %13.9f lon %12.9f lat %12.9f deg/day\n",names[body],p.longitude_deg,p.latitude_deg,p.longitude_speed_deg_per_day);
    }
    oe_ephemeris_close(e);return 0;
}
