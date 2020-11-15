#include <engine.h>
#include <ds4.h>
#include <windows.h>
#include <setupapi.h>
#include <winioctl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment(lib,"setupapi.lib")



struct _HIDD_ATTRIBUTES{
	unsigned long sz;
	uint16_t vid;
	uint16_t pid;
	uint16_t _;
};
struct _HIDP_CAPS{
	uint32_t _;
	uint16_t il;
	uint16_t ol;
	uint64_t __[7];
};
typedef int (__stdcall *_HidD_GetAttributes_f)(HANDLE d,struct _HIDD_ATTRIBUTES* a);
typedef int (__stdcall *_HidD_GetPreparsedData_f)(HANDLE h,void* dt);
typedef int (__stdcall *_HidD_FreePreparsedData_f)(void* dt);
typedef NTSTATUS (__stdcall *_HidP_GetCaps_f)(void* dt,struct _HIDP_CAPS *c);
typedef int (__stdcall *_HidD_SetNumInputBuffers_f)(HANDLE h,unsigned long n_bf);
static _HidD_GetAttributes_f _HidD_GetAttributes;
static _HidD_GetPreparsedData_f _HidD_GetPreparsedData;
static _HidD_FreePreparsedData_f _HidD_FreePreparsedData;
static _HidP_GetCaps_f _HidP_GetCaps;
static _HidD_SetNumInputBuffers_f _HidD_SetNumInputBuffers;
bool _ds4_i=false;
HMODULE _hlib_h=NULL;



void DS4_init(void){
	if (_ds4_i==false){
		_hlib_h=LoadLibraryA("hid.dll");
		assert(_hlib_h!=NULL);
#define RESOLVE(x) _##x=(_##x##_f)GetProcAddress(_hlib_h,#x);assert(_##x!=NULL);
		RESOLVE(HidD_GetAttributes);
		RESOLVE(HidD_GetPreparsedData);
		RESOLVE(HidD_FreePreparsedData);
		RESOLVE(HidP_GetCaps);
		RESOLVE(HidD_SetNumInputBuffers);
#undef RESOLVE
		_ds4_i=true;
	}
}



void DS4_cleanup(void){
	if (_hlib_h!=NULL){
		FreeLibrary(_hlib_h);
	}
	_hlib_h=NULL;
	_ds4_i=false;
}



DS4DeviceList DS4_find_all(void){
	DS4DeviceList o=NULL;
	DS4DeviceList c=NULL;
	GUID i_guid={
		0x4d1e55b2,
		0xf16f,
		0x11cf,
		{
			0x88,
			0xcb,
			0x00,
			0x11,
			0x11,
			0x00,
			0x00,
			0x30
		}
	};
	DS4_init();
	SP_DEVINFO_DATA d_dt={
		sizeof(SP_DEVINFO_DATA),
		{
			0,
			0,
			0,
			0
		},
		0,
		0
	};
	SP_DEVICE_INTERFACE_DATA d_idt={
		sizeof(SP_DEVICE_INTERFACE_DATA),
		{
			0,
			0,
			0,
			0
		},
		0,
		0
	};
	HDEVINFO d_il=SetupDiGetClassDevsA(&i_guid,NULL,NULL,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
	SP_DEVICE_INTERFACE_DETAIL_DATA_A* d_ddt=NULL;
	int r;
	uint32_t i=0;
	while (true){
		HANDLE fh=INVALID_HANDLE_VALUE;
		uint32_t sz=0;
		struct _HIDD_ATTRIBUTES d_a;
		r=SetupDiEnumDeviceInterfaces(d_il,NULL,&i_guid,i,&d_idt);
		if (r==0){
			break;
		}
		r=SetupDiGetDeviceInterfaceDetailA(d_il,&d_idt,NULL,0,&sz,NULL);
		d_ddt=malloc(sz);
		d_ddt->cbSize=sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
		r=SetupDiGetDeviceInterfaceDetailA(d_il,&d_idt,d_ddt,sz,NULL,NULL);
		if (r!=0){
			uint32_t j=0;
			while (true){
				char d_nm[256];
				r=SetupDiEnumDeviceInfo(d_il,j,&d_dt);
				if (r==0){
					j=UINT32_MAX;
					break;
				}
				r=SetupDiGetDeviceRegistryPropertyA(d_il,&d_dt,SPDRP_CLASS,NULL,(unsigned char*)d_nm,sizeof(d_nm),NULL);
				if (r==0){
					j=UINT32_MAX;
					break;
				}
				if (strcmp(d_nm,"HIDClass")==0){
					r=SetupDiGetDeviceRegistryPropertyA(d_il,&d_dt,SPDRP_DRIVER,NULL,(unsigned char*)d_nm,sizeof(d_nm),NULL);
					if (r!=0){
						break;
					}
				}
				j++;
			}
			if (j!=UINT32_MAX){
				fh=CreateFileA(d_ddt->DevicePath,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
				if (fh!=INVALID_HANDLE_VALUE){
					d_a.sz=sizeof(struct _HIDD_ATTRIBUTES);
					_HidD_GetAttributes(fh,&d_a);
					if (d_a.vid==0x054c&&(d_a.pid==0x05c4||d_a.pid==0x09cc)){
						DS4DeviceList tmp=malloc(sizeof(struct _DS4_DEVICE_LIST));
						tmp->p=malloc(1);
						size_t ln=0;
						for (size_t k=0;true;k++){
							ln++;
							tmp->p=realloc(tmp->p,ln+1);
							*(tmp->p+ln-1)=*(d_ddt->DevicePath+k);
							if (*(d_ddt->DevicePath+k)==0){
								break;
							}
						}
						if (o==NULL){
							o=tmp;
							c=tmp;
							tmp->n=NULL;
						}
						else{
							c->n=tmp;
							c=tmp;
						}
					}
					CloseHandle(fh);
				}
			}
		}
		free(d_ddt);
		i++;
	}
	SetupDiDestroyDeviceInfoList(d_il);
	return o;
}



void DS4_free_list(DS4DeviceList l){
	assert(l!=NULL);
	assert(l->p!=NULL);
	free(l->p);
	if (l->n!=NULL){
		DS4_free_list(l->n);
	}
	free(l);
}



DS4Device DS4_connect(char* p){
	assert(p!=NULL);
	DS4_init();
	DS4Device o=malloc(sizeof(struct _DS4_DEVICE));
	o->fc=0;
	o->dt=0;
	o->btn=0;
	o->l2=0;
	o->r2=0;
	o->lx=0;
	o->ly=0;
	o->rx=0;
	o->ry=0;
	o->bt=0;
	o->r=0;
	o->g=0;
	o->b=0;
	o->fr=0;
	o->sr=0;
	o->fon=0;
	o->foff=0;
	o->_fh=CreateFileA(p,(GENERIC_WRITE|GENERIC_READ),FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
	o->_ib=NULL;
	o->_ob=NULL;
	o->_f=0;
	o->_uc=UINT64_MAX;
	o->_o.hEvent=CreateEvent(NULL,false,false,NULL);
	o->_lt.QuadPart=0;
	o->_tf.QuadPart=0;
	if (o->_fh==INVALID_HANDLE_VALUE){
		CloseHandle(o->_o.hEvent);
		free(o);
		return NULL;
	}
	int r=_HidD_SetNumInputBuffers(o->_fh,64);
	if (r==0){
		CloseHandle(o->_o.hEvent);
		CloseHandle(o->_fh);
		free(o);
		return NULL;
	}
	void* d_pdt=NULL;
	r=_HidD_GetPreparsedData(o->_fh,&d_pdt);
	if (r==0){
		CloseHandle(o->_o.hEvent);
		CloseHandle(o->_fh);
		free(o);
		return NULL;
	}
	struct _HIDP_CAPS d_c;
	uint32_t nt_r=_HidP_GetCaps(d_pdt,&d_c);
	_HidD_FreePreparsedData(d_pdt);
	if (nt_r!=0x110000){
		CloseHandle(o->_o.hEvent);
		CloseHandle(o->_fh);
		free(o);
		return NULL;
	}
	assert(d_c.il==64);
	assert(d_c.ol==32);
	o->_ib=malloc(64);
	for (uint16_t i=0;i<64;i++){
		*(o->_ib+i)=0;
	}
	o->_ob=malloc(32);
	for (uint16_t i=0;i<32;i++){
		*(o->_ib+i)=0;
	}
	QueryPerformanceFrequency(&o->_tf);
	DS4_update(o);
	return o;
}



void DS4_update(DS4Device d){
	uint64_t uc=((uint64_t)d->r<<56)|((uint64_t)d->g<<48)|((uint64_t)d->b<<40)|((uint64_t)d->fr<<32)|((uint64_t)d->sr<<24)|((uint64_t)d->r<<16)|((uint64_t)d->r<<8);
	if (d->_uc!=uc){
		d->_uc=uc;
		*d->_ob=0x05;
		*(d->_ob+1)=0x7f;
		*(d->_ob+2)=0x04;
		*(d->_ob+4)=d->fr;
		*(d->_ob+5)=d->sr;
		*(d->_ob+6)=d->r;
		*(d->_ob+7)=d->g;
		*(d->_ob+8)=d->b;
		*(d->_ob+9)=d->fon;
		*(d->_ob+10)=d->foff;
		OVERLAPPED oe;
		memset(&oe,0,sizeof(OVERLAPPED));
		int r=WriteFile(d->_fh,d->_ob,32,NULL,&oe);
		if (r==0){
			if (GetLastError()!=ERROR_IO_PENDING){
				printf("ERROR: %lx\n",GetLastError());
				assert(0);
				return;
			}
		}
		uint32_t br=0;
		r=GetOverlappedResult(d->_fh,&oe,&br,true);
		if (r==0){
			assert(0);
			return;
		}
	}
	uint32_t br=0;
	if ((d->_f&1)==0){
		d->_f|=1;
		memset(d->_ib,0,64);
		ResetEvent(d->_o.hEvent);
		memset(&d->_o,0,sizeof(OVERLAPPED));
		int r=ReadFile(d->_fh,d->_ib,64,NULL,&d->_o);
		if (r==0){
			if (GetLastError()!=ERROR_IO_PENDING){
				CancelIo(d->_fh);
				d->_f&=~1;
				assert(0);
				return;
			}
		}
	}
	int r=GetOverlappedResult(d->_fh,&d->_o,&br,true);
	d->_f&=~1;
	if (r!=0&&br>0){
		d->fc=(uint8_t)(*(d->_ib+7))>>2;
		uint8_t dp=(*(d->_ib+5)&0x0f);
		d->btn=((dp==0||dp==1||dp==7)?d->btn|DS4_BUTTON_UP:d->btn&(~DS4_BUTTON_UP));
		d->btn=((dp>=3&&dp<=5)?d->btn|DS4_BUTTON_DOWN:d->btn&(~DS4_BUTTON_DOWN));
		d->btn=((dp>=5&&dp<=7)?d->btn|DS4_BUTTON_LEFT:d->btn&(~DS4_BUTTON_LEFT));
		d->btn=((dp>=1&&dp<=3)?d->btn|DS4_BUTTON_RIGHT:d->btn&(~DS4_BUTTON_RIGHT));
		d->btn=(((*(d->_ib+6))&1)!=0?d->btn|DS4_BUTTON_L1:d->btn&(~DS4_BUTTON_L1));
		d->btn=(((*(d->_ib+6))&2)!=0?d->btn|DS4_BUTTON_R1:d->btn&(~DS4_BUTTON_R1));
		d->btn=(((*(d->_ib+6))&4)!=0?d->btn|DS4_BUTTON_L2:d->btn&(~DS4_BUTTON_L2));
		d->btn=(((*(d->_ib+6))&8)!=0?d->btn|DS4_BUTTON_R2:d->btn&(~DS4_BUTTON_R2));
		d->btn=(((*(d->_ib+6))&64)!=0?d->btn|DS4_BUTTON_L3:d->btn&(~DS4_BUTTON_L3));
		d->btn=(((*(d->_ib+6))&128)!=0?d->btn|DS4_BUTTON_R3:d->btn&(~DS4_BUTTON_R3));
		d->btn=(((*(d->_ib+5))&32)!=0?d->btn|DS4_BUTTON_CROSS:d->btn&(~DS4_BUTTON_CROSS));
		d->btn=(((*(d->_ib+5))&64)!=0?d->btn|DS4_BUTTON_CIRCLE:d->btn&(~DS4_BUTTON_CIRCLE));
		d->btn=(((*(d->_ib+5))&16)!=0?d->btn|DS4_BUTTON_SQURARE:d->btn&(~DS4_BUTTON_SQURARE));
		d->btn=(((*(d->_ib+5))&128)!=0?d->btn|DS4_BUTTON_TRIANGLE:d->btn&(~DS4_BUTTON_TRIANGLE));
		d->btn=(((*(d->_ib+6))&32)!=0?d->btn|DS4_BUTTON_OPTIONS:d->btn&(~DS4_BUTTON_OPTIONS));
		d->btn=(((*(d->_ib+6))&16)!=0?d->btn|DS4_BUTTON_SHARE:d->btn&(~DS4_BUTTON_SHARE));
		d->btn=(((*(d->_ib+7))&1)!=0?d->btn|DS4_BUTTON_PS:d->btn&(~DS4_BUTTON_PS));
		d->btn=(((*(d->_ib+7))&2)!=0?d->btn|DS4_BUTTON_TOUCHPAD:d->btn&(~DS4_BUTTON_TOUCHPAD));
		d->l2=*(d->_ib+8);
		d->r2=*(d->_ib+9);
		d->lx=-128+*(d->_ib+1);
		d->ly=127-*(d->_ib+2);
		d->rx=-128+*(d->_ib+3);
		d->ry=127-*(d->_ib+4);
		d->bt=(uint8_t)(((*(d->_ib+30))&16)*15.9375);
	}
}



void DS4_hsl(DS4Device d,uint8_t h,uint8_t s,uint8_t l){
	if (s==0){
		d->r=l;
		d->g=l;
		d->b=l;
	}
	else{
		uint8_t r=h/43;
		uint8_t rm=(h-(r*43))*6;
		uint8_t p=(l*(255-s))>>8;
		uint8_t q=(l*(255-((s*rm)>>8)))>>8;
		uint8_t t=(l*(255-((s*(255-rm))>>8)))>>8;
		switch (r){
			case 0:
				d->r=l;
				d->g=t;
				d->b=p;
				break;
			case 1:
				d->r=q;
				d->g=l;
				d->b=p;
				break;
			case 2:
				d->r=p;
				d->g=l;
				d->b=t;
				break;
			case 3:
				d->r=p;
				d->g=q;
				d->b=l;
				break;
			case 4:
				d->r=t;
				d->g=p;
				d->b=l;
				break;
			default:
				d->r=l;
				d->g=p;
				d->b=q;
				break;
		}
	}
}



void DS4_close(DS4Device d){
	CancelIo(d->_fh);
	if (d->_ib!=NULL){
		free(d->_ib);
	}
	if (d->_ob!=NULL){
		free(d->_ob);
	}
	free(d);
}
