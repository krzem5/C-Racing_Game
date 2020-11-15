#define COBJMACROS
#include <renderer.h>
#include <engine.h>
#include <windows.h>
#include <windowsx.h>
#include <unknwn.h>
#include <shellscalingapi.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"shcore.lib")



HWND renderer_w=NULL;
uint32_t renderer_ww=0;
uint32_t renderer_wh=0;
uint32_t renderer_wx=0;
uint32_t renderer_wy=0;
int16_t renderer_mx=0;
int16_t renderer_my=0;
uint16_t renderer_mf=0;
float renderer_wsf=0;
bool renderer_wf=false;
float renderer_cc[4]={0,0,0,255};
ID3D11Device* renderer_d3_d=NULL;
ID3D11Device1* renderer_d3_d1=NULL;
ID3D11DeviceContext* renderer_d3_dc=NULL;
ID3D11DeviceContext1* renderer_d3_dc1=NULL;
IDXGISwapChain* renderer_d3_sc=NULL;
IDXGISwapChain1* renderer_d3_sc1=NULL;
ID3D11RenderTargetView* renderer_d3_rt=NULL;
ID3D11Texture2D* renderer_d3_ds=NULL;
ID3D11DepthStencilView* renderer_d3_dsv=NULL;
ID3D11DepthStencilState* renderer_d3_dss=NULL;
ID3D11DepthStencilState* renderer_d3_ddss=NULL;
ID3D11BlendState* renderer_d3_bse=NULL;
ID3D11BlendState* renderer_d3_bsd=NULL;
ID3D11RasterizerState* renderer_d3_ar=NULL;



LRESULT CALLBACK _msg_cb(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
	switch (msg){
		case WM_KILLFOCUS:
			renderer_wf=false;
			renderer_mf=0;
			return 0;
		case WM_SETFOCUS:
			renderer_wf=true;
			return 0;
		case WM_KEYDOWN:
			return 0;
		case WM_MOUSEMOVE:
			renderer_mx=(int16_t)GET_X_LPARAM(lp);
			renderer_my=(int16_t)GET_Y_LPARAM(lp);
			return 0;
		case WM_LBUTTONDBLCLK:
			renderer_mf|=M_LEFT_DBL;
			return 0;
		case WM_LBUTTONDOWN:
			renderer_mf|=M_LEFT;
			return 0;
		case WM_LBUTTONUP:
			renderer_mf&=(~M_LEFT);
			return 0;
		case WM_MBUTTONDBLCLK:
			renderer_mf|=M_MIDDLE_DBL;
			return 0;
		case WM_MBUTTONDOWN:
			renderer_mf|=M_MIDDLE;
			return 0;
		case WM_MBUTTONUP:
			renderer_mf|=(~M_MIDDLE);
			return 0;
		case WM_RBUTTONDBLCLK:
			renderer_mf|=M_RIGHT_DBL;
			return 0;
		case WM_RBUTTONDOWN:
			renderer_mf|=M_RIGHT;
			return 0;
		case WM_RBUTTONUP:
			renderer_mf&=(~M_RIGHT);
			return 0;
		case WM_XBUTTONDBLCLK:
			if (GET_XBUTTON_WPARAM(wp)==0x1){
				renderer_mf=M_X1_DBL;
			}
			else{
				renderer_mf=M_X2_DBL;
			}
			return 0;
		case WM_XBUTTONDOWN:
			if (GET_XBUTTON_WPARAM(wp)==0x1){
				renderer_mf|=M_X1;
			}
			else{
				renderer_mf|=M_X2;
			}
			return 0;
		case WM_XBUTTONUP:
			if (GET_XBUTTON_WPARAM(wp)==0x1){
				renderer_mf&=(~M_X1);
			}
			else{
				renderer_mf&=(~M_X2);
			}
			return 0;
		case WM_SIZE:
			if (renderer_d3_dc!=NULL){
				RECT sz;
				GetClientRect(renderer_w,&sz);
				renderer_ww=sz.right;
				renderer_wh=sz.bottom;
				ID3D11DeviceContext_OMSetRenderTargets(renderer_d3_dc,0,0,0);
				ID3D11RenderTargetView_Release(renderer_d3_rt);
				HRESULT hr=IDXGISwapChain_ResizeBuffers(renderer_d3_sc,0,0,0,DXGI_FORMAT_UNKNOWN,0);
				ID3D11Texture2D* bb=NULL;
				hr=IDXGISwapChain_GetBuffer(renderer_d3_sc,0,&IID_ID3D11Texture2D,(void**)&bb);
				ID3D11Resource* bbr=NULL;
				ID3D11Texture2D_QueryInterface(bb,&IID_ID3D11Resource,(void**)&bbr);
				hr=ID3D11Device_CreateRenderTargetView(renderer_d3_d,bbr,NULL,&renderer_d3_rt);
				ID3D11Resource_Release(bbr);
				ID3D11Texture2D_Release(bb);
				ID3D11DeviceContext_OMSetRenderTargets(renderer_d3_dc,1,&renderer_d3_rt,NULL);
				D3D11_VIEWPORT vp={
					0,
					0,
					(float)renderer_ww,
					(float)renderer_wh,
					0,
					1
				};
				ID3D11DeviceContext_RSSetViewports(renderer_d3_dc,1,&vp);
			}
			return 0;
		case WM_MOVING:
			RECT* r=(RECT*)lp;
			renderer_wx=r->left;
			renderer_wy=r->top;
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			renderer_w=NULL;
			return 0;
	}
	return DefWindowProc(hwnd,msg,wp,lp);
}



Matrix identity_matrix(){
	Matrix o=malloc(sizeof(struct _MATRIX));
	o->_00=1;
	o->_01=0;
	o->_02=0;
	o->_03=0;
	o->_10=0;
	o->_11=1;
	o->_12=0;
	o->_13=0;
	o->_20=0;
	o->_21=0;
	o->_22=1;
	o->_23=0;
	o->_30=0;
	o->_31=0;
	o->_32=0;
	o->_33=1;
	return o;
}



Matrix y_rotation_matrix(float a){
	Matrix o=malloc(sizeof(struct _MATRIX));
	float s=sinf(a);
	float c=cosf(a);
	o->_00=c;
	o->_01=0;
	o->_02=-s;
	o->_03=0;
	o->_10=0;
	o->_11=1;
	o->_12=0;
	o->_13=0;
	o->_20=s;
	o->_21=0;
	o->_22=c;
	o->_23=0;
	o->_30=0;
	o->_31=0;
	o->_32=0;
	o->_33=1;
	return o;
}



Matrix perspective_fov_matrix(float fov,float a,float n,float f){
	Matrix o=malloc(sizeof(struct _MATRIX));
	float cs=cosf(fov/2)/sinf(fov/2);
	float r=f/(f-n);
	o->_00=cs/a;
	o->_01=0;
	o->_02=0;
	o->_03=0;
	o->_10=0;
	o->_11=cs;
	o->_12=0;
	o->_13=0;
	o->_20=0;
	o->_21=0;
	o->_22=r;
	o->_23=1;
	o->_30=0;
	o->_31=0;
	o->_32=-n*r;
	o->_33=0;
	return o;
}



Matrix ortographic_matrix(float t,float l,float b,float r,float n,float f){
	Matrix o=malloc(sizeof(struct _MATRIX));
	o->_00=2/(r-l)/renderer_aspect_ratio;
	o->_01=0;
	o->_02=0;
	o->_03=-(r+l)/(r-l);
	o->_10=0;
	o->_11=2/(t-b);
	o->_12=0;
	o->_13=-(t+b)/(t-b);
	o->_20=0;
	o->_21=0;
	o->_22=-2/(f-n);
	o->_23=-(f+n)/(f-n);
	o->_30=0;
	o->_31=0;
	o->_32=0;
	o->_33=1;
	return o;
}



Matrix look_at_matrix(float ex,float ey,float ez,float dx,float dy,float dz,float ux,float uy,float uz){
	float dm=sqrtf(dx*dx+dy*dy+dz*dz);
	dx/=dm;
	dy/=-dm;
	dz/=dm;
	ey=-ey;
	float xx=uy*dz-uz*dy;
	float xy=ux*dx-ux*dz;
	float xz=ux*dy-uy*dx;
	float xm=sqrtf(xx*xx+xy*xy+xz*xz);
	xx/=xm;
	xy/=xm;
	xz/=xm;
	float yx=xy*dz-xz*dy;
	float yy=xz*dx-xx*dz;
	float yz=xx*dy-xy*dx;
	Matrix o=malloc(sizeof(struct _MATRIX));
	o->_00=xx;
	o->_01=yx;
	o->_02=dx;
	o->_03=0;
	o->_10=-xy;
	o->_11=-yy;
	o->_12=-dy;
	o->_13=0;
	o->_20=xz;
	o->_21=yz;
	o->_22=dz;
	o->_23=0;
	o->_30=-(xx*ex+xy*ey+xz*ez);
	o->_31=-(yx*ex+yy*ey+yz*ez);
	o->_32=-(dx*ex+dy*ey+dz*ez);
	o->_33=1;
	return o;
}



ObjectBuffer create_object_buffer(){
	ObjectBuffer o=malloc(sizeof(struct _OBJECT_BUFFER));
	o->vll=0;
	o->vl=NULL;
	o->ill=0;
	o->il=NULL;
	o->st=3*sizeof(float);
	o->_vb=NULL;
	o->_ib=NULL;
	return o;
}



ObjectBuffer box_object_buffer(){
	ObjectBuffer o=malloc(sizeof(struct _OBJECT_BUFFER));
	o->vll=56;
	o->vl=malloc(56*sizeof(float));
	*o->vl=-1;
	*(o->vl+1)=1;
	*(o->vl+2)=-1;
	*(o->vl+3)=0;
	*(o->vl+4)=0;
	*(o->vl+5)=1;
	*(o->vl+6)=1;
	*(o->vl+7)=1;
	*(o->vl+8)=1;
	*(o->vl+9)=-1;
	*(o->vl+10)=0;
	*(o->vl+11)=1;
	*(o->vl+12)=0;
	*(o->vl+13)=1;
	*(o->vl+14)=1;
	*(o->vl+15)=1;
	*(o->vl+16)=1;
	*(o->vl+17)=0;
	*(o->vl+18)=1;
	*(o->vl+19)=1;
	*(o->vl+20)=1;
	*(o->vl+21)=-1;
	*(o->vl+22)=1;
	*(o->vl+23)=1;
	*(o->vl+24)=1;
	*(o->vl+25)=0;
	*(o->vl+26)=0;
	*(o->vl+27)=1;
	*(o->vl+28)=-1;
	*(o->vl+29)=-1;
	*(o->vl+30)=-1;
	*(o->vl+31)=1;
	*(o->vl+32)=0;
	*(o->vl+33)=1;
	*(o->vl+34)=1;
	*(o->vl+35)=1;
	*(o->vl+36)=-1;
	*(o->vl+37)=-1;
	*(o->vl+38)=1;
	*(o->vl+39)=1;
	*(o->vl+40)=0;
	*(o->vl+41)=1;
	*(o->vl+42)=1;
	*(o->vl+43)=-1;
	*(o->vl+44)=1;
	*(o->vl+45)=1;
	*(o->vl+46)=1;
	*(o->vl+47)=1;
	*(o->vl+48)=1;
	*(o->vl+49)=-1;
	*(o->vl+50)=-1;
	*(o->vl+51)=1;
	*(o->vl+52)=0;
	*(o->vl+53)=0;
	*(o->vl+54)=0;
	*(o->vl+55)=1;
	o->ill=36;
	o->il=malloc(36*sizeof(uint32_t));
	*o->il=3;
	*(o->il+1)=1;
	*(o->il+2)=0;
	*(o->il+3)=2;
	*(o->il+4)=1;
	*(o->il+5)=3;
	*(o->il+6)=0;
	*(o->il+7)=5;
	*(o->il+8)=4;
	*(o->il+9)=1;
	*(o->il+10)=5;
	*(o->il+11)=0;
	*(o->il+12)=3;
	*(o->il+13)=4;
	*(o->il+14)=7;
	*(o->il+15)=0;
	*(o->il+16)=4;
	*(o->il+17)=3;
	*(o->il+18)=1;
	*(o->il+19)=6;
	*(o->il+20)=5;
	*(o->il+21)=2;
	*(o->il+22)=6;
	*(o->il+23)=1;
	*(o->il+24)=2;
	*(o->il+25)=7;
	*(o->il+26)=6;
	*(o->il+27)=3;
	*(o->il+28)=7;
	*(o->il+29)=2;
	*(o->il+30)=6;
	*(o->il+31)=4;
	*(o->il+32)=5;
	*(o->il+33)=7;
	*(o->il+34)=4;
	*(o->il+35)=6;
	o->st=7*sizeof(float);
	o->_vb=NULL;
	o->_ib=NULL;
	update_object_buffer(o);
	return o;
}



void update_object_buffer(ObjectBuffer ob){
	D3D11_BUFFER_DESC bd={
		ob->vll*sizeof(float),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0,
		0
	};
	D3D11_SUBRESOURCE_DATA dt={
		ob->vl,
		0,
		0
	};
	HRESULT hr=ID3D11Device_CreateBuffer(renderer_d3_d,&bd,&dt,&ob->_vb);
	if (FAILED(hr)){
		printf("ERR4\n");
		return;
	}
	bd.Usage=D3D11_USAGE_DEFAULT;
	bd.ByteWidth=ob->ill*sizeof(uint16_t);
	bd.BindFlags=D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags=0;
	dt.pSysMem=ob->il;
	hr=ID3D11Device_CreateBuffer(renderer_d3_d,&bd,&dt,&ob->_ib);
	if (FAILED(hr)){
		printf("ERR5\n");
		return;
	}
}



void draw_object_buffer(ObjectBuffer ob){
	unsigned int off=0;
	ID3D11DeviceContext_IASetVertexBuffers(renderer_d3_dc,0,1,&ob->_vb,&ob->st,&off);
	ID3D11DeviceContext_IASetIndexBuffer(renderer_d3_dc,ob->_ib,DXGI_FORMAT_R16_UINT,0);
	ID3D11DeviceContext_IASetPrimitiveTopology(renderer_d3_dc,D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D11DeviceContext_DrawIndexed(renderer_d3_dc,ob->ill,0,0);
}



void init_renderer(void){
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	WNDCLASSEXW wc={
		sizeof(WNDCLASSEX),
		CS_DBLCLKS|CS_OWNDC|CS_HREDRAW|CS_VREDRAW,
		&_msg_cb,
		0,
		0,
		GetModuleHandle(0),
		0,
		0,
		0,
		0,
		L"Fischertechnic Instruction Builder",
		0
	};
	RegisterClassExW(&wc);
	assert(renderer_w==NULL);
	renderer_w=CreateWindowExW(wc.style,wc.lpszClassName,L"Fischertechnic Instruction Builder",WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,GetModuleHandle(0),NULL);
	SetWindowPos(renderer_w,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	SetWindowLongPtr(renderer_w,GWL_STYLE,WS_VISIBLE);
	SetWindowLongPtr(renderer_w,GWL_EXSTYLE,0);
	MONITORINFO mf={
		sizeof(mf)
	};
	GetMonitorInfo(MonitorFromWindow(renderer_w,MONITOR_DEFAULTTONEAREST),&mf);
	renderer_ww=min(1920,mf.rcMonitor.right-mf.rcMonitor.left);
	renderer_wh=min(1080,mf.rcMonitor.bottom-mf.rcMonitor.top);
	renderer_wx=mf.rcMonitor.left/2+mf.rcMonitor.right/2-renderer_ww/2;
	renderer_wy=mf.rcMonitor.top/2+mf.rcMonitor.bottom/2-renderer_wh/2;
	renderer_wsf=GetDpiForWindow(renderer_w)/96.0f;
	SetWindowPos(renderer_w,NULL,renderer_wx,renderer_wy,renderer_ww,renderer_wh,SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
	SetCapture(renderer_w);
	ShowWindow(renderer_w,SW_SHOW);
	UpdateWindow(renderer_w);
	SetCursor(LoadCursorW(NULL,IDC_APPSTARTING));
	LARGE_INTEGER tf;
	LARGE_INTEGER lt={0};
	QueryPerformanceFrequency(&tf);
	MSG msg={0};
	LARGE_INTEGER c;
	while (msg.message!=WM_QUIT&&renderer_w!=NULL){
		if (PeekMessage(&msg,renderer_w,0,0,PM_REMOVE)>0){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		QueryPerformanceCounter(&c);
		if (lt.QuadPart==0){
			D3D_FEATURE_LEVEL fl;
			D3D_FEATURE_LEVEL sfl[]={
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0
			};
			D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_HARDWARE,0,D3D11_CREATE_DEVICE_DEBUG,sfl,1,D3D11_SDK_VERSION,&renderer_d3_d,&fl,&renderer_d3_dc);
			IDXGIFactory1* dxgi_f=NULL;
			IDXGIDevice* dxgi_d=NULL;
			HRESULT hr=ID3D11Device_QueryInterface(renderer_d3_d,&IID_IDXGIDevice,(void**)&dxgi_d);
			if (SUCCEEDED(hr)){
				IDXGIAdapter* dxgi_a=NULL;
				hr=IDXGIDevice_GetAdapter(dxgi_d,&dxgi_a);
				if (SUCCEEDED(hr)){
					hr=IDXGIAdapter_GetParent(dxgi_a,&IID_IDXGIFactory1,(void**)&dxgi_f);
					IDXGIAdapter_Release(dxgi_a);
				}
				IDXGIDevice_Release(dxgi_d);
			}
			IDXGIFactory2* dxgi_f2=NULL;
			hr=IDXGIFactory1_QueryInterface(dxgi_f,&IID_IDXGIFactory2,(void**)&dxgi_f2);
			assert(hr==S_OK);
			if (dxgi_f2!=NULL){
				hr=ID3D11Device_QueryInterface(renderer_d3_d,&IID_ID3D11Device1,(void**)&renderer_d3_d1);
				if (SUCCEEDED(hr)){
					ID3D11DeviceContext_QueryInterface(renderer_d3_dc,&IID_ID3D11DeviceContext1,(void**)&renderer_d3_dc1);
				}
				DXGI_SWAP_CHAIN_DESC1 sc_d={
					renderer_ww,
					renderer_wh,
					DXGI_FORMAT_R8G8B8A8_UNORM,
					false,
					{
						1,
						0
					},
					DXGI_USAGE_RENDER_TARGET_OUTPUT,
					1,
					DXGI_SCALING_STRETCH,
					DXGI_SWAP_EFFECT_DISCARD,
					DXGI_ALPHA_MODE_UNSPECIFIED,
					0
				};
				IUnknown* du;
				ID3D11Device_QueryInterface(renderer_d3_d,&IID_IUnknown,(void**)&du);
				hr=IDXGIFactory2_CreateSwapChainForHwnd(dxgi_f2,du,renderer_w,&sc_d,NULL,NULL,&renderer_d3_sc1);
				IUnknown_Release(du);
				if (SUCCEEDED(hr)){
					hr=IDXGISwapChain1_QueryInterface(renderer_d3_sc1,&IID_IDXGISwapChain,(void**)&renderer_d3_sc);
					assert(!FAILED(hr));
				}
				IDXGIFactory2_Release(dxgi_f2);
			}
			else{
				DXGI_SWAP_CHAIN_DESC sc_d={
					{
						renderer_ww,
						renderer_wh,
						{
							60,
							1
						},
						DXGI_FORMAT_UNKNOWN,
						DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
						DXGI_MODE_SCALING_UNSPECIFIED
					},
					{
						1,
						0
					},
					DXGI_USAGE_RENDER_TARGET_OUTPUT,
					1,
					renderer_w,
					true,
					DXGI_SWAP_EFFECT_DISCARD,
					0
				};
				IUnknown* du;
				ID3D11Device_QueryInterface(renderer_d3_d,&IID_IUnknown,(void**)&du);
				IDXGIFactory2_CreateSwapChain(dxgi_f,du,&sc_d,&renderer_d3_sc);
				IUnknown_Release(du);
			}
			IDXGIFactory2_MakeWindowAssociation(dxgi_f,renderer_w,DXGI_MWA_NO_ALT_ENTER);
			IDXGIFactory2_Release(dxgi_f);
			ID3D11Texture2D* bb=NULL;
			assert(renderer_d3_sc!=NULL);
			hr=IDXGISwapChain_GetBuffer(renderer_d3_sc,0,&IID_ID3D11Texture2D,(void**)&bb);
			if (FAILED(hr)){
				printf("ERR6\n");
				break;
			}
			ID3D11Resource* bbr;
			ID3D11Texture2D_QueryInterface(bb,&IID_ID3D11Resource,(void**)&bbr);
			hr=ID3D11Device_CreateRenderTargetView(renderer_d3_d,bbr,NULL,&renderer_d3_rt);
			ID3D11Resource_Release(bbr);
			if (FAILED(hr)){
				printf("ERR7\n");
				break;
			}
			ID3D11Texture2D_Release(bb);
			D3D11_TEXTURE2D_DESC dd={
				renderer_ww,
				renderer_wh,
				1,
				1,
				DXGI_FORMAT_D24_UNORM_S8_UINT,
				{
					1,
					0
				},
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_DEPTH_STENCIL,
				0,
				0
			};
			hr=ID3D11Device_CreateTexture2D(renderer_d3_d,&dd,NULL,&renderer_d3_ds);
			if (FAILED(hr)){
				printf("Error creating Depth Stensil Texture\n");
				break;
			}
			D3D11_DEPTH_STENCIL_VIEW_DESC dsv={
				dd.Format,
				D3D11_DSV_DIMENSION_TEXTURE2D,
				0
			};
			dsv.Texture2D.MipSlice=0;
			ID3D11Resource* dsr;
			ID3D11Texture2D_QueryInterface(renderer_d3_ds,&IID_ID3D11Resource,(void**)&dsr);
			hr=ID3D11Device_CreateDepthStencilView(renderer_d3_d,dsr,&dsv,&renderer_d3_dsv);
			D3D11_DEPTH_STENCIL_DESC dss={
				true,
				D3D11_DEPTH_WRITE_MASK_ALL,
				D3D11_COMPARISON_LESS,
				true,
				0xff,
				0xff,
				{
					D3D11_STENCIL_OP_KEEP,
					D3D11_STENCIL_OP_INCR,
					D3D11_STENCIL_OP_KEEP,
					D3D11_COMPARISON_ALWAYS
				},
				{
					D3D11_STENCIL_OP_KEEP,
					D3D11_STENCIL_OP_DECR,
					D3D11_STENCIL_OP_KEEP,
					D3D11_COMPARISON_ALWAYS
				}
			};
			hr=ID3D11Device_CreateDepthStencilState(renderer_d3_d,&dss,&renderer_d3_dss);
			ID3D11DeviceContext_OMSetDepthStencilState(renderer_d3_dc,renderer_d3_dss,1);
			dss.DepthEnable=false;
			hr=ID3D11Device_CreateDepthStencilState(renderer_d3_d,&dss,&renderer_d3_ddss);
			ID3D11Resource_Release(dsr);
			if (FAILED(hr)){
				printf("ERR9\n");
				break;
			}
			D3D11_BLEND_DESC bsd={
				false,
				false,
				{
					true,
					D3D11_BLEND_SRC_ALPHA,
					D3D11_BLEND_INV_SRC_ALPHA,
					D3D11_BLEND_OP_ADD,
					D3D11_BLEND_ONE,
					D3D11_BLEND_ZERO,
					D3D11_BLEND_OP_ADD,
					D3D11_COLOR_WRITE_ENABLE_ALL
				}
			};
			hr=ID3D11Device_CreateBlendState(renderer_d3_d,&bsd,&renderer_d3_bse);
			bsd.RenderTarget[0].BlendEnable=false;
			hr=ID3D11Device_CreateBlendState(renderer_d3_d,&bsd,&renderer_d3_bsd);
			ID3D11DeviceContext_OMSetRenderTargets(renderer_d3_dc,1,&renderer_d3_rt,renderer_d3_dsv);
			D3D11_VIEWPORT vp={
				0,
				0,
				(float)renderer_ww,
				(float)renderer_wh,
				0,
				1
			};
			ID3D11DeviceContext_RSSetViewports(renderer_d3_dc,1,&vp);
			D3D11_RASTERIZER_DESC rd={
				D3D11_FILL_SOLID,
				D3D11_CULL_BACK,
				false,
				D3D11_DEFAULT_DEPTH_BIAS,
				D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
				D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
				true,
				false,
				true,
				false
			};
			hr=ID3D11Device_CreateRasterizerState(renderer_d3_d,&rd,&renderer_d3_ar);
			ID3D11DeviceContext_RSSetState(renderer_d3_dc,renderer_d3_ar);
			init_engine();
			update_engine(0);
		}
		else{
			ID3D11DeviceContext_ClearRenderTargetView(renderer_d3_dc,renderer_d3_rt,renderer_cc);
			ID3D11DeviceContext_ClearDepthStencilView(renderer_d3_dc,renderer_d3_dsv,D3D11_CLEAR_DEPTH,1.0f,0);
			update_engine((double)((c.QuadPart-lt.QuadPart)*1000000/tf.QuadPart));
		}
		if (renderer_w==NULL){
			break;
		}
		renderer_mf&=(~M_LEFT_DBL)&(~M_MIDDLE_DBL)&(~M_RIGHT_DBL)&(~M_X1_DBL)&(~M_X2_DBL);
		HRESULT r=IDXGISwapChain_Present(renderer_d3_sc,0,0);
		if (r!=S_OK&&r!=DXGI_STATUS_OCCLUDED){
			printf("%lx\n",r);
			assert(0);
		}
		lt=c;
	}
}



bool is_pressed(uint32_t k){
	return (GetKeyState(k)<0?true:false);
}



ID3D11VertexShader* load_vertex_shader(const BYTE* dt,size_t ln,D3D11_INPUT_ELEMENT_DESC* il,uint16_t ill,ID3D11InputLayout** o){
	ID3D11VertexShader* vo=NULL;
	HRESULT hr=ID3D11Device_CreateVertexShader(renderer_d3_d,dt,ln,NULL,&vo);
	if (FAILED(hr)){
		printf("Error creating VS\n");
		return NULL;
	}
	*o=NULL;
	hr=ID3D11Device_CreateInputLayout(renderer_d3_d,il,ill,dt,ln,o);
	if (FAILED(hr)){
		printf("Error creating VS Input Layout\n");
		assert(0);
		return NULL;
	}
	return vo;
}



ID3D11PixelShader* load_pixel_shader(const BYTE* dt,size_t ln){
	ID3D11PixelShader* o=NULL;
	HRESULT hr=ID3D11Device_CreatePixelShader(renderer_d3_d,dt,ln,NULL,&o);
	if (FAILED(hr)){
		printf("Error creating PS\n");
		return NULL;
	}
	return o;
}



ID3D11Buffer* create_constant_buffer(uint16_t l){
	if (l%16!=0){
		printf("%hu Bytes of Padding Required!",16-l%16);
		assert(0);
	}
	D3D11_BUFFER_DESC bd={
		l,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_CONSTANT_BUFFER,
		0,
		0,
		0
	};
	ID3D11Buffer* o=NULL;
	HRESULT hr=ID3D11Device_CreateBuffer(renderer_d3_d,&bd,NULL,&o);
	if (FAILED(hr)){
		printf("Error Creating Constant Buffer: %x\n",hr);
		return NULL;
	}
	return o;
}



void update_constant_buffer(ID3D11Buffer* cb,void* dt){
	ID3D11Resource* cbbr=NULL;
	ID3D11Buffer_QueryInterface(cb,&IID_ID3D11Resource,(void**)&cbbr);
	ID3D11DeviceContext_UpdateSubresource(renderer_d3_dc,cbbr,0,NULL,dt,0,0);
	ID3D11Resource_Release(cbbr);
}



void close_window(void){
	if (renderer_d3_d!=NULL){
		ID3D11Device_Release(renderer_d3_d);
		renderer_d3_d=NULL;
	}
	if (renderer_d3_d1!=NULL){
		ID3D11Device1_Release(renderer_d3_d1);
		renderer_d3_d1=NULL;
	}
	if (renderer_d3_dc!=NULL){
		ID3D11DeviceContext_Release(renderer_d3_dc);
		renderer_d3_dc=NULL;
	}
	if (renderer_d3_dc1!=NULL){
		ID3D11DeviceContext1_Release(renderer_d3_dc1);
		renderer_d3_dc1=NULL;
	}
	if (renderer_d3_sc!=NULL){
		IDXGISwapChain_Release(renderer_d3_sc);
		renderer_d3_sc=NULL;
	}
	if (renderer_d3_sc1!=NULL){
		IDXGISwapChain_Release(renderer_d3_sc1);
		renderer_d3_sc1=NULL;
	}
	if (renderer_d3_rt!=NULL){
		ID3D11RenderTargetView_Release(renderer_d3_rt);
		renderer_d3_rt=NULL;
	}
	if (renderer_d3_ds!=NULL){
		ID3D11Texture2D_Release(renderer_d3_ds);
		renderer_d3_ds=NULL;
	}
	if (renderer_d3_dsv!=NULL){
		ID3D11DepthStencilView_Release(renderer_d3_dsv);
		renderer_d3_dsv=NULL;
	}
	if (renderer_d3_dss!=NULL){
		ID3D11DepthStencilState_Release(renderer_d3_dss);
		renderer_d3_dss=NULL;
	}
	if (renderer_d3_ddss!=NULL){
		ID3D11DepthStencilState_Release(renderer_d3_ddss);
		renderer_d3_ddss=NULL;
	}
	if (renderer_d3_bse!=NULL){
		ID3D11BlendState_Release(renderer_d3_bse);
		renderer_d3_bse=NULL;
	}
	if (renderer_d3_bsd!=NULL){
		ID3D11BlendState_Release(renderer_d3_bsd);
		renderer_d3_bsd=NULL;
	}
	if (renderer_d3_ar!=NULL){
		ID3D11RasterizerState_Release(renderer_d3_ar);
		renderer_d3_ar=NULL;
	}
	if (renderer_w!=NULL){
		DestroyWindow(renderer_w);
		renderer_w=NULL;
	}
}
