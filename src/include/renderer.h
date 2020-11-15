#ifndef __RENDERER_H__
#define __RENDERER_H__
#define COBJMACROS
#include <engine.h>
#include <windows.h>
#include <d3d11_1.h>



#define TWO_PI ((float)6.283185307f)
#define PI ((float)3.141592654f)
#define PI_DIV_TWO ((float)1.570796327f)
#define PI_DIV_180 ((float)0.017453293f)
#define EPSILON ((float)0.0000001f)
#define M_LEFT 0x001
#define M_LEFT_DBL 0x002
#define M_MIDDLE 0x004
#define M_MIDDLE_DBL 0x008
#define M_RIGHT 0x010
#define M_RIGHT_DBL 0x020
#define M_X1 0x040
#define M_X1_DBL 0x080
#define M_X2 0x100
#define M_X2_DBL 0x200
#define FRAME_RATE 60
#define raw_vector(x,y,z,w) ((RawVector){(x),(y),(z),(w)})
#define as_raw_vector(v) (*((RawVector*)(v)))
#define raw_identity_matrix() ((RawMatrix){1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1})
#define raw_matrix(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) ((RawMatrix){(a),(b),(c),(d),(e),(f),(g),(h),(i),(j),(k),(l),(m),(n),(o),(p)})
#define as_raw_matrix(m) (*((RawMatrix*)(m)))
#define free_matrix(m) ((m)!=NULL?(free((m)),NULL):NULL)
#define renderer_aspect_ratio (renderer_ww/(float)renderer_wh)



typedef struct _VECTOR* Vector;
typedef struct _VECTOR RawVector;
typedef struct _MATRIX* Matrix;
typedef struct _MATRIX RawMatrix;
typedef struct _OBJECT_BUFFER* ObjectBuffer;



struct _VECTOR{
	float x;
	float y;
	float z;
	float w;
};



struct _MATRIX{
	float _00;
	float _01;
	float _02;
	float _03;
	float _10;
	float _11;
	float _12;
	float _13;
	float _20;
	float _21;
	float _22;
	float _23;
	float _30;
	float _31;
	float _32;
	float _33;
};



struct _OBJECT_BUFFER{
	uint32_t vll;
	float* vl;
	uint32_t ill;
	uint16_t* il;
	uint32_t st;
	ID3D11Buffer* _vb;
	ID3D11Buffer* _ib;
};



extern HWND renderer_w;
extern uint32_t renderer_ww;
extern uint32_t renderer_wh;
extern uint32_t renderer_wx;
extern uint32_t renderer_wy;
extern int16_t renderer_mx;
extern int16_t renderer_my;
extern uint16_t renderer_mf;
extern float renderer_wsf;
extern bool renderer_wf;
extern float renderer_cc[4];
extern ID3D11Device* renderer_d3_d;
extern ID3D11Device1* renderer_d3_d1;
extern ID3D11DeviceContext* renderer_d3_dc;
extern ID3D11DeviceContext1* renderer_d3_dc1;
extern IDXGISwapChain* renderer_d3_sc;
extern IDXGISwapChain1* renderer_d3_sc1;
extern ID3D11RenderTargetView* renderer_d3_rt;
extern ID3D11Texture2D* renderer_d3_ds;
extern ID3D11DepthStencilView* renderer_d3_sv;
extern ID3D11DepthStencilState* renderer_d3_dss;
extern ID3D11DepthStencilState* renderer_d3_ddss;
extern ID3D11BlendState* renderer_d3_bse;
extern ID3D11BlendState* renderer_d3_bsd;



Matrix identity_matrix();



Matrix y_rotation_matrix(float a);



Matrix perspective_fov_matrix(float fov,float a,float n,float f);



Matrix ortographic_matrix(float t,float l,float b,float r,float n,float f);



Matrix look_at_matrix(float ex,float ey,float ez,float dx,float dy,float dz,float ux,float uy,float uz);



ObjectBuffer create_object_buffer();



ObjectBuffer box_object_buffer();



void update_object_buffer(ObjectBuffer ob);



void draw_object_buffer(ObjectBuffer ob);



void init_renderer(void);



bool is_pressed(uint32_t k);



ID3D11VertexShader* load_vertex_shader(const BYTE* dt,size_t ln,D3D11_INPUT_ELEMENT_DESC* il,uint16_t ill,ID3D11InputLayout** o);



ID3D11PixelShader* load_pixel_shader(const BYTE* dt,size_t ln);



ID3D11Buffer* create_constant_buffer(uint16_t l);



void update_constant_buffer(ID3D11Buffer* cb,void* dt);



void use_vertex_shader(ID3D11VertexShader* vs);



void use_pixel_shader(ID3D11PixelShader* ps);



void close_window(void);



#endif
