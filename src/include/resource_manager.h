#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__
#include <engine.h>
#include <renderer.h>
#define COBJMACROS
#include <d3d11_1.h>



#define ASSET_BASE_FILE_PATH "rsrc/mdl/"



typedef struct _BLOCK_MODEL* BlockModel;
typedef BlockModel AssetID;



struct _BLOCK_MODEL{
	uint32_t ib_l;
	ID3D11Buffer* ib;
	ID3D11Buffer* vb;
};



AssetID resource_manager_load_asset(uint8_t id);



void resource_manager_draw_asset(AssetID a,RawMatrix tm,RawMatrix rm);



#endif
