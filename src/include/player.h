#ifndef __PLAYER_H__
#define __PLAYER_H__
#include <engine.h>
#include <ds4.h>
#include <resource_manager.h>



#define MAX_SPEED 10



typedef struct _PLAYER* Player;



struct _PLAYER{
	float x;
	float z;
	float vx;
	float vz;
	float r;
	bool t;
	union _PLAYER_DATA{
		struct _PLAYER_DATA_PLAYER{
			DS4Device d;
		} pl;
	} dt;
	AssetID _a;
};



Player init_player(float x,float z,float r,bool t);



Matrix update_player(Player p,float dt);



void draw_player(Player p);



#endif
