#include <engine.h>
#include <renderer.h>
#include <ui.h>
#include <level_renderer.h>
#include <ds4.h>
#include <block_3d_vertex.h>
#include <block_3d_pixel.h>
#include <stdio.h>



typedef struct _CAMERA* Camera;



struct _CAMERA{
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
	float zm;
	float ms;
	float rs;
	bool enabled;
	uint16_t _f;
	int16_t _rx;
	int16_t _ry;
	int16_t _px;
	int16_t _py;
	float _ox;
	float _oy;
	float _oz;
};



Camera create_camera(float ms,float rs,float x,float y,float z,float rx,float ry,float rz,float zm){
	Camera o=malloc(sizeof(struct _CAMERA));
	o->x=x;
	o->y=y;
	o->z=z;
	o->rx=(rx-90)*PI_DIV_180;
	o->ry=ry*PI_DIV_180;
	o->rz=rz*PI_DIV_180;
	o->zm=zm;
	o->ms=ms;
	o->rs=rs;
	o->enabled=false;
	o->_f=0;
	o->_rx=0;
	o->_ry=0;
	o->_px=0;
	o->_py=0;
	o->_ox=0;
	o->_oy=0;
	o->_oz=0;
	return o;
}



Matrix update_camera(Camera c,float dt){
	if (c->enabled==true){
		bool rc=false;
		if ((renderer_mf&M_RIGHT)!=0){
			rc=true;
			if ((c->_f&M_RIGHT)==0){
				c->_f|=M_RIGHT;
				c->_rx=renderer_mx;
				c->_ry=renderer_my;
			}
			c->rx-=(c->_ry-renderer_my)*c->rs*dt*PI_DIV_180;
			c->ry-=(c->_rx-renderer_mx)*c->rs*dt*PI_DIV_180;
			if (c->rx>PI_DIV_TWO-EPSILON){
				c->rx=PI_DIV_TWO-EPSILON;
			}
			else if (c->rx<-PI_DIV_TWO+EPSILON){
				c->rx=-PI_DIV_TWO+EPSILON;
			}
			c->_rx=renderer_mx;
			c->_ry=renderer_my;
		}
		else{
			c->_f&=(~M_RIGHT);
		}
		float dx=cosf(c->ry)*c->ms*dt;
		float dz=sinf(c->ry)*c->ms*dt;
		if (is_pressed(0x57)){
			rc=true;
			c->_ox+=dx;
			c->_oz+=dz;
		}
		if (is_pressed(0x53)){
			rc=true;
			c->_ox-=dx;
			c->_oz-=dz;
		}
		if (is_pressed(0x41)){
			rc=true;
			c->_ox+=dz;
			c->_oz-=dx;
		}
		if (is_pressed(0x44)){
			rc=true;
			c->_ox-=dz;
			c->_oz+=dx;
		}
		if (is_pressed(VK_SPACE)){
			rc=true;
			c->_oy+=c->ms*dt;
		}
		if (is_pressed(VK_LSHIFT)){
			rc=true;
			c->_oy-=c->ms*dt;
		}
		if (rc==true){
			c->x=c->_ox+c->zm*sinf(-c->rx+PI_DIV_TWO)*cosf(PI-c->ry);
			c->y=c->_oy+c->zm*cosf(-c->rx+PI_DIV_TWO);
			c->z=c->_oz+c->zm*sinf(-c->rx+PI_DIV_TWO)*sinf(PI-c->ry);
		}
	}
	return look_at_matrix(c->x,c->y,c->z,sinf(c->rx-PI_DIV_TWO)*cosf(c->ry),cosf(c->rx-PI_DIV_TWO),sinf(c->rx-PI_DIV_TWO)*sinf(c->ry),0,1,0);
}



struct CBufferLayout{
	RawMatrix cm;
	RawMatrix pm;
};



ID3D11VertexShader* b_vs=NULL;
ID3D11PixelShader* b_ps=NULL;
ID3D11InputLayout* b_vl=NULL;
Camera c;
RawMatrix pm;
ID3D11Buffer* cb=NULL;
Level l=NULL;



void init_engine(void){
	D3D11_INPUT_ELEMENT_DESC b_il[]={
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
		},
		{
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
	b_vs=load_vertex_shader(g_block_3d_vs,sizeof(g_block_3d_vs),b_il,sizeof(b_il)/sizeof(D3D11_INPUT_ELEMENT_DESC),&b_vl);
	b_ps=load_pixel_shader(g_block_3d_ps,sizeof(g_block_3d_ps));
	cb=create_constant_buffer(sizeof(struct CBufferLayout));
	*renderer_cc=1;
	*(renderer_cc+1)=1;
	*(renderer_cc+2)=1;
	*(renderer_cc+3)=1;
	c=create_camera(10,50,0,0,0,90,0,0,1);
	c->enabled=true;
	float n=EPSILON;
	float f=1000;
	pm=raw_matrix(1/renderer_aspect_ratio,0,0,0,0,1,0,0,0,0,-2/(f-n),-(f+n)/(f-n),0,0,0,1);
	l=create_level(load_level("level1"));
	init_ui();
}



void update_engine(double dt){
	static float t=0;
	t+=(float)(dt*1e-6);
	update_ui(dt);
	if (renderer_w==NULL){
		return;
	}
	Matrix cm=update_camera(c,(float)(dt*1e-6));free(update_level(l,dt));
	// Matrix cm=update_level(l,dt);
	if (cm==NULL){
		return;
	}
	struct CBufferLayout cb_dt={
		as_raw_matrix(cm),
		pm
	};
	update_constant_buffer(cb,(void*)&cb_dt);
	ID3D11DeviceContext_VSSetConstantBuffers(renderer_d3_dc,0,1,&cb);
	ID3D11DeviceContext_PSSetConstantBuffers(renderer_d3_dc,0,1,&cb);
	ID3D11DeviceContext_VSSetShader(renderer_d3_dc,b_vs,NULL,0);
	ID3D11DeviceContext_PSSetShader(renderer_d3_dc,b_ps,NULL,0);
	ID3D11DeviceContext_IASetInputLayout(renderer_d3_dc,b_vl);
	draw_level(l);
	draw_ui();
	free(cm);
}
