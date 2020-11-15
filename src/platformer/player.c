#include <player.h>
#include <engine.h>
#include <ds4.h>
#include <resource_manager.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>



float map(float v,float aa,float ab,float ba,float bb){
	return (v-aa)/(ab-aa)*(bb-ba)+ba;
}



Player init_player(float x,float z,float r,bool t){
	Player o=malloc(sizeof(struct _PLAYER));
	o->x=x;
	o->z=z;
	o->vx=0;
	o->vz=0;
	o->r=r;
	o->t=t;
	if (t==true){
		DS4DeviceList l=DS4_find_all();
		assert(l!=NULL);
		o->dt.pl.d=DS4_connect(l->p);
		DS4_free_list(l);
		o->dt.pl.d->r=190;
		o->dt.pl.d->g=30;
		o->dt.pl.d->b=10;
	}
	else{
		assert(!"AI init() Not Implemented!");
		return NULL;
	}
	o->_a=resource_manager_load_asset(/**/0x2a/**/);
	return o;
}



Matrix update_player(Player p,float dt){
	#define JOYSTICK_BUFFOR 25
	#define JOYSTICK_TURN_BUFFOR 50
	#define FWD_SPEED 5
	#define BACK_SPEED 0.5f
	#define TURN_SPEED 1.2f
	#define SPEED_LERP_PROP 0.005f
	#define SPEED_LERP_PROP_FWD 0.02f
	#define SPEED_LERP_PROP_BACK 0.05f
	#define SPEED_TO_ROT_LERP 40
	#define CAMERA_X_OFFSET 2.2f
	#define CAMERA_Y_OFFSET 2
	#define CAMERA_Z_OFFSET 2.2f
	#define CAMERA_R_OFFSET (PI*0.8f)
	#define CAMERA_RX_ANGLE (-0.8f)
	#define MIN_BACK_SPEED 0.5f
	#define BACK_AUTO_BRAKE_SPEED 0.01f
	assert(p!=NULL);
	float s=sinf(p->r);
	float c=cosf(p->r);
	float ax=0;
	float az=0;
	float slp=SPEED_LERP_PROP;
	float nr=0;
	if (p->t==true){
		DS4_update(p->dt.pl.d);
		if (p->dt.pl.d->lx<=-JOYSTICK_TURN_BUFFOR||p->dt.pl.d->lx>=JOYSTICK_TURN_BUFFOR){
			nr-=(p->dt.pl.d->lx>0?1:-1)*TURN_SPEED;
		}
		if (p->dt.pl.d->ly>=JOYSTICK_BUFFOR){
			float v=map(p->dt.pl.d->ly,0,127,0,FWD_SPEED);
			ax+=s*v;
			az+=c*v;
			slp=SPEED_LERP_PROP_FWD;
		}
		else if (p->dt.pl.d->ly<=-JOYSTICK_BUFFOR){
			float d=p->vx*p->vx+p->vz*p->vz;
			if (d>MIN_BACK_SPEED*MIN_BACK_SPEED){
				ax+=(p->vx<0?1:-1)*BACK_AUTO_BRAKE_SPEED;
				az+=(p->vz<0?1:-1)*BACK_AUTO_BRAKE_SPEED;
			}
			else{
				flooat v=map(p->dt.pl.d->ly,0,-127,0,-BACK_SPEED);
				ax+=s*v;
				az+=c*v;
				slp=SPEED_LERP_PROP_BACK;
			}
		}
		else{
			nr=0;
		}
	}
	else{
		assert(!"AI update() Not Implemented!");
		return NULL;
	}
	nr+=p->r;
	float d=ax*ax+az*az;
	if (d>MAX_SPEED*MAX_SPEED){
		d=sqrtf(d);
		ax=ax/d*MAX_SPEED;
		az=az/d*MAX_SPEED;
	}
	p->vx=p->vx+slp*(ax-p->vx);
	p->vz=p->vz+slp*(az-p->vz);
	p->r=p->r+slp*SPEED_TO_ROT_LERP*(nr-p->r)*dt;
	p->x+=p->vx*dt;
	p->z+=p->vz*dt;
	s=-sinf(-p->r-CAMERA_R_OFFSET);
	c=-cosf(-p->r-CAMERA_R_OFFSET);
	return look_at_matrix(p->x+CAMERA_X_OFFSET*cosf(-p->r-CAMERA_R_OFFSET),CAMERA_Y_OFFSET,p->z+CAMERA_Z_OFFSET*sinf(-p->r-CAMERA_R_OFFSET),sinf(CAMERA_RX_ANGLE)*c,cosf(CAMERA_RX_ANGLE),sinf(CAMERA_RX_ANGLE)*s,0,1,0);
}



void draw_player(Player p){
	float s=-sinf(p->r);
	float c=-cosf(p->r);
	resource_manager_draw_asset(p->_a,raw_matrix(c,0,-s,0,0,1,0,0,s,0,c,0,p->x,0,p->z,1),raw_matrix(c,0,-s,0,0,1,0,0,s,0,c,0,0,0,0,1));
}
