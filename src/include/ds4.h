#ifndef __DS4_H__
#define __DS4_H__
#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <signal.h>



typedef struct _DS4_DEVICE_LIST* DS4DeviceList;
typedef struct _DS4_DEVICE* DS4Device;



enum DS4_BUTTON{
	DS4_BUTTON_UP=0x00001,
	DS4_BUTTON_DOWN=0x00002,
	DS4_BUTTON_LEFT=0x00004,
	DS4_BUTTON_RIGHT=0x00008,
	DS4_BUTTON_L1=0x00010,
	DS4_BUTTON_R1=0x00020,
	DS4_BUTTON_L2=0x00040,
	DS4_BUTTON_R2=0x00080,
	DS4_BUTTON_L3=0x00100,
	DS4_BUTTON_R3=0x00200,
	DS4_BUTTON_CROSS=0x00400,
	DS4_BUTTON_CIRCLE=0x00800,
	DS4_BUTTON_SQURARE=0x01000,
	DS4_BUTTON_TRIANGLE=0x02000,
	DS4_BUTTON_OPTIONS=0x04000,
	DS4_BUTTON_SHARE=0x08000,
	DS4_BUTTON_PS=0x10000,
	DS4_BUTTON_TOUCHPAD=0x20000
};



struct _DS4_DEVICE{
	uint8_t fc;
	uint32_t dt;
	uint32_t btn;
	uint8_t l2;
	uint8_t r2;
	int8_t lx;
	int8_t ly;
	int8_t rx;
	int8_t ry;
	uint8_t bt;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t fr;
	uint8_t sr;
	uint8_t fon;
	uint8_t foff;
	HANDLE _fh;
	char* _ib;
	char* _ob;
	uint8_t _f;
	uint64_t _uc;
	OVERLAPPED _o;
	LARGE_INTEGER _lt;
	LARGE_INTEGER _tf;
};



struct _DS4_DEVICE_LIST{
	char* p;
	struct _DS4_DEVICE_LIST* n;
};



void DS4_init(void);



void DS4_cleanup(void);



DS4DeviceList DS4_find_all(void);



void DS4_free_list(DS4DeviceList l);



DS4Device DS4_connect(char* p);



void DS4_update(DS4Device d);



void DS4_hsl(DS4Device d,uint8_t h,uint8_t s,uint8_t l);



void DS4_close(DS4Device d);



#endif
