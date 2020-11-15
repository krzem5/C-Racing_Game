#ifndef __LEVEL_RENDERER_H__
#define __LEVEL_RENDERER_H__
#include <engine.h>
#include <resource_manager.h>
#include <player.h>



#define LEVEL_BASE_FILE_PATH "rsrc/levels/"
#define LEVEL_FILE_MAGIC 0x204c764c
#define LEVEL_MAX_WIDTH 4096
#define TRACK_ID_START_POS 0x5d
#define TRACK_ID_START 0x5c
#define TRACK_ID_FWD 0x63
#define TRACK_ID_TURN_LS 0x42
#define TRACK_ID_TURN_LM 0x34
#define TRACK_ID_TURN_LL 0x37
#define TRACK_ID_TURN_RS 0x42
#define TRACK_ID_TURN_RM 0x34
#define TRACK_ID_TURN_RL 0x37



typedef struct _LEVEL_LAYOUT* LevelLayout;
typedef struct _LEVEL* Level;



enum TRACK_TYPE{
	TRACK_TYPE_FWD=0x00,
	TRACK_TYPE_TURN_LS=0x01,
	TRACK_TYPE_TURN_LM=0x02,
	TRACK_TYPE_TURN_LL=0x03,
	TRACK_TYPE_TURN_RS=0x04,
	TRACK_TYPE_TURN_RM=0x05,
	TRACK_TYPE_TURN_RL=0x06
};



struct _LEVEL_TRACK_PIECE{
	AssetID a;
	float x;
	float z;
	uint8_t r;

};



struct _LEVEL_LAYOUT{
	uint8_t nml;
	char* nm;
	uint16_t sx;
	uint16_t sz;
	uint8_t sr;
	uint32_t tl;
	uint8_t* t;
};



struct _LEVEL_TRACK_LINE{
	float x;
	float z;
};



struct _LEVEL{
	LevelLayout ll;
	Player p;
	uint32_t tl;
	struct _LEVEL_TRACK_PIECE* t;
	uint64_t tlll;
	struct _LEVEL_TRACK_LINE* tll;
};



LevelLayout load_level(const char* nm);



Level create_level(LevelLayout ll);



Matrix update_level(Level l,double dt);



void draw_level(Level l);



#endif
