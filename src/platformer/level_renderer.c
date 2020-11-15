#include <level_renderer.h>
#include <engine.h>
#include <resource_manager.h>
#include <renderer.h>
#include <player.h>
#include <stdlib.h>
#include <stdio.h>
#include <color_3d_vertex.h>
#include <color_3d_pixel.h>



struct CBufferLayout{
	RawMatrix wm;
};



ID3D11Buffer* l_cb=NULL;
ID3D11Buffer* TMP_ib;
ID3D11Buffer* TMP_vb;
uint32_t TMP_ill;
ID3D11VertexShader* TMP_vs=NULL;
ID3D11PixelShader* TMP_ps=NULL;
ID3D11InputLayout* TMP_vl=NULL;



void _next_track(float* x,float* z,uint8_t *r,float a,float b){
	*r=(*r+4)%4;
	assert(*r<4);
	switch (*r){
		case 0:
			(*x)+=b;
			(*z)+=a;
			return;
		case 1:
			(*x)+=a;
			(*z)-=b;
			return;
		case 2:
			(*x)-=b;
			(*z)-=a;
			return;
		case 3:
			(*x)-=a;
			(*z)+=b;
			return;
	}
}



void _gen_turn_lines(uint64_t* tlll,struct _LEVEL_TRACK_LINE** tll,float x,float z,uint8_t r){
	(*tlll)+=3;
	*tll=realloc(*tll,(*tlll)*sizeof(struct _LEVEL_TRACK_LINE));
	_next_track(&x,&z,&r,0.5f,-0.1f);
	(*tll+*tlll-3)->x=x;
	(*tll+*tlll-3)->z=z;
	_next_track(&x,&z,&r,0.5f,-0.23f);
	(*tll+*tlll-2)->x=x;
	(*tll+*tlll-2)->z=z;
	_next_track(&x,&z,&r,0.35f,-0.5f);
	(*tll+*tlll-1)->x=x;
	(*tll+*tlll-1)->z=z;
}



LevelLayout load_level(const char* nm){
	size_t ln=0;
	while (*(nm+ln)!=0){
		ln++;
	}
	assert(ln!=0);
	size_t lnb=strlen(LEVEL_BASE_FILE_PATH);
	char* fp=malloc((ln+lnb+5)*sizeof(char));
	for (size_t i=0;i<lnb;i++){
		*(fp+i)=*(LEVEL_BASE_FILE_PATH+i);
	}
	for (size_t i=0;i<ln;i++){
		*(fp+i+lnb)=*(nm+i);
	}
	*(fp+lnb+ln)='.';
	*(fp+lnb+ln+1)='l';
	*(fp+lnb+ln+2)='v';
	*(fp+lnb+ln+3)='l';
	*(fp+lnb+ln+4)=0;
	FILE* f=NULL;
	errno_t e=fopen_s(&f,fp,"rb");
	free(fp);
	if (e==ENOENT){
		printf("File '%s' doesn't extist!\n",fp);
		return NULL;
	}
	uint32_t fm=(fgetc(f)&0xff)|((fgetc(f)&0xff)<<8)|((fgetc(f)&0xff)<<16)|((fgetc(f)&0xff)<<24);
	assert(fm==LEVEL_FILE_MAGIC);
	LevelLayout o=malloc(sizeof(struct _LEVEL_LAYOUT));
	o->nml=fgetc(f)&0xff;
	assert(o->nml<=20);
	o->nm=malloc((o->nml+1)*sizeof(char));
	for (size_t i=0;i<o->nml;i++){
		*(o->nm+i)=(fgetc(f)&0xff);
	}
	*(o->nm+o->nml)=0;
	o->sx=(fgetc(f)&0xff)|((fgetc(f)&0xff)<<8);
	assert(o->sx<LEVEL_MAX_WIDTH);
	o->sz=(fgetc(f)&0xff)|((fgetc(f)&0xff)<<8);
	assert(o->sz<LEVEL_MAX_WIDTH);
	o->sr=(fgetc(f)&0xff);
	assert(o->sr<4);
	o->tl=(fgetc(f)&0xff)|((fgetc(f)&0xff)<<8)|((fgetc(f)&0xff)<<16)|((fgetc(f)&0xff)<<24);
	o->t=malloc(o->tl*sizeof(uint8_t));
	assert(fread(o->t,sizeof(uint8_t),o->tl,f)==o->tl);
	fclose(f);
	return o;
}



Level create_level(LevelLayout ll){
	assert(ll!=NULL);
	Level o=malloc(sizeof(struct _LEVEL));
	o->ll=ll;
	o->p=init_player(ll->sx,ll->sz,ll->sr*PI_DIV_TWO,true);
	o->tl=ll->tl+2;
	o->t=malloc(ll->tl*sizeof(struct _LEVEL_TRACK_PIECE));
	o->tlll=3;
	o->tll=malloc(o->tlll*sizeof(struct _LEVEL_TRACK_LINE));
	float x=ll->sx;
	float z=ll->sz;
	uint8_t r=ll->sr;
	o->tll->x=x;
	o->tll->z=z;
	o->t->a=resource_manager_load_asset(TRACK_ID_START_POS);
	_next_track(&x,&z,&r,1,0);
	o->t->x=x;
	o->t->z=z;
	o->t->r=r;
	_next_track(&x,&z,&r,1,0);
	(o->tll+1)->x=x;
	(o->tll+1)->z=z;
	(o->t+1)->a=resource_manager_load_asset(TRACK_ID_START);
	_next_track(&x,&z,&r,1,0);
	(o->t+1)->x=x;
	(o->t+1)->z=z;
	(o->t+1)->r=r;
	_next_track(&x,&z,&r,1,0);
	(o->tll+2)->x=x;
	(o->tll+2)->z=z;
	for (uint32_t i=2;i<o->tl;i++){
		float tx=x;
		float tz=z;
		switch (*(ll->t+i-2)){
			default:
				assert(0);
				return NULL;
			case TRACK_TYPE_FWD:
				_next_track(&x,&z,&r,1,0);
				(o->t+i)->a=resource_manager_load_asset(TRACK_ID_FWD);
				(o->t+i)->x=x;
				(o->t+i)->z=z;
				(o->t+i)->r=r;
				_next_track(&x,&z,&r,1,0);
				break;
			case TRACK_TYPE_TURN_LS:
				assert(0);
				break;
			case TRACK_TYPE_TURN_LM:
				assert(0);
				break;
			case TRACK_TYPE_TURN_LL:
				assert(0);
				break;
			case TRACK_TYPE_TURN_RS:
				assert(0);
				break;
			case TRACK_TYPE_TURN_RM:
				o->tlll+=3;
				o->tll=realloc(o->tll,o->tlll*sizeof(struct _LEVEL_TRACK_LINE));
				_next_track(&tx,&tz,&r,0.5f,-0.1f);
				(o->tll+o->tlll-3)->x=tx;
				(o->tll+o->tlll-3)->z=tz;
				_next_track(&tx,&tz,&r,0.5f,-0.23f);
				(o->tll+o->tlll-2)->x=tx;
				(o->tll+o->tlll-2)->z=tz;
				_next_track(&tx,&tz,&r,0.35f,-0.5f);
				(o->tll+o->tlll-1)->x=tx;
				(o->tll+o->tlll-1)->z=tz;
				(o->t+i)->a=resource_manager_load_asset(TRACK_ID_TURN_RM);
				_next_track(&x,&z,&r,1,-0.5f);
				(o->t+i)->x=x;
				(o->t+i)->z=z;
				(o->t+i)->r=r;
				r--;
				_next_track(&x,&z,&r,1,0.5f);
				break;
			case TRACK_TYPE_TURN_RL:
				o->tlll+=3;
				o->tll=realloc(o->tll,o->tlll*sizeof(struct _LEVEL_TRACK_LINE));
				_next_track(&tx,&tz,&r,1,-0.2f);
				(o->tll+o->tlll-3)->x=tx;
				(o->tll+o->tlll-3)->z=tz;
				_next_track(&tx,&tz,&r,0.65f,-0.4f);
				(o->tll+o->tlll-2)->x=tx;
				(o->tll+o->tlll-2)->z=tz;
				_next_track(&tx,&tz,&r,0.63f,-0.85f);
				(o->tll+o->tlll-1)->x=tx;
				(o->tll+o->tlll-1)->z=tz;
				(o->t+i)->a=resource_manager_load_asset(TRACK_ID_TURN_RL);
				_next_track(&x,&z,&r,1.5f,-1);
				(o->t+i)->x=x;
				(o->t+i)->z=z;
				(o->t+i)->r=r;
				r--;
				_next_track(&x,&z,&r,1.5f,1);
				break;
		}
		o->tlll++;
		o->tll=realloc(o->tll,o->tlll*sizeof(struct _LEVEL_TRACK_LINE));
		(o->tll+o->tlll-1)->x=x;
		(o->tll+o->tlll-1)->z=z;
	}
	/*********************************/
	printf("TLLL: %llu\n",o->tlll);
	D3D11_INPUT_ELEMENT_DESC id[]={
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
	TMP_vs=load_vertex_shader(g_color_3d_vs,sizeof(g_color_3d_vs),id,sizeof(id)/sizeof(D3D11_INPUT_ELEMENT_DESC),&TMP_vl);
	TMP_ps=load_pixel_shader(g_color_3d_ps,sizeof(g_color_3d_ps));
	TMP_ill=(uint32_t)((o->tlll-1)*2);
	uint16_t* il=malloc(TMP_ill*sizeof(uint16_t));
	float* vl=malloc(o->tlll*9*sizeof(float));
	for (uint32_t i=0;i<o->tlll-1;i++){
		*(il+i*2)=i;
		*(il+i*2+1)=i+1;
	}
	for (size_t i=0;i<o->tlll;i++){
		*(vl+i*9)=(o->tll+i)->x;
		*(vl+i*9+1)=0.02f;
		*(vl+i*9+2)=(o->tll+i)->z;
		*(vl+i*9+3)=1;
		*(vl+i*9+4)=0;
		*(vl+i*9+5)=0;
		*(vl+i*9+6)=0;
		*(vl+i*9+7)=1;
		*(vl+i*9+8)=0;
	}
	D3D11_BUFFER_DESC bd={
		(uint32_t)(TMP_ill*sizeof(uint16_t)),
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_INDEX_BUFFER,
		0,
		0,
		0
	};
	D3D11_SUBRESOURCE_DATA dt={
		il,
		0,
		0
	};
	HRESULT hr=ID3D11Device_CreateBuffer(renderer_d3_d,&bd,&dt,&TMP_ib);
	free(il);
	bd.ByteWidth=(uint32_t)(o->tlll*9*sizeof(float));
	bd.BindFlags=D3D11_BIND_VERTEX_BUFFER;
	dt.pSysMem=vl;
	hr=ID3D11Device_CreateBuffer(renderer_d3_d,&bd,&dt,&TMP_vb);
	free(vl);
	return o;
}



Matrix update_level(Level l,double dt){
	assert(l!=NULL);
	return update_player(l->p,(float)(dt*1e-6));
}



void draw_level(Level l){
	assert(l!=NULL);
	for (uint32_t i=0;i<l->tl;i++){
		float s=((l->t+i)->r==0?0:((l->t+i)->r==1?1:((l->t+i)->r==2?0:-1.0f)));
		float c=((l->t+i)->r==0?1:((l->t+i)->r==1?0:((l->t+i)->r==2?-1.0f:0)));
		resource_manager_draw_asset((l->t+i)->a,raw_matrix(c,0,-s,0,0,1,0,0,s,0,c,0,(l->t+i)->x,0,(l->t+i)->z,1),raw_matrix(c,0,-s,0,0,1,0,0,s,0,c,0,0,0,0,1));
	}
	draw_player(l->p);
	/******************************************************/
	ID3D11DeviceContext_VSSetShader(renderer_d3_dc,TMP_vs,NULL,0);
	ID3D11DeviceContext_PSSetShader(renderer_d3_dc,TMP_ps,NULL,0);
	ID3D11DeviceContext_IASetInputLayout(renderer_d3_dc,TMP_vl);
	unsigned int off=0;
	unsigned int st=9*sizeof(float);
	ID3D11DeviceContext_IASetPrimitiveTopology(renderer_d3_dc,D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	ID3D11DeviceContext_IASetVertexBuffers(renderer_d3_dc,0,1,&TMP_vb,&st,&off);
	ID3D11DeviceContext_IASetIndexBuffer(renderer_d3_dc,TMP_ib,DXGI_FORMAT_R16_UINT,0);
	ID3D11DeviceContext_DrawIndexed(renderer_d3_dc,TMP_ill,0,0);
}
