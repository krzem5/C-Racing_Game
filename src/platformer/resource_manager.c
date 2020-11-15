#include <resource_manager.h>
#include <engine.h>
#include <renderer.h>
#include <stdio.h>



struct CBufferLayout{
	RawMatrix tm;
	RawMatrix rm;
};



struct _ASSET_LIST_NODE{
	struct _ASSET_LIST_NODE* n;
	uint8_t id;
	AssetID dt;
} _head={
	NULL,
	UINT8_MAX,
	NULL
};
ID3D11Buffer* a_cb=NULL;



AssetID resource_manager_load_asset(uint8_t id){
	struct _ASSET_LIST_NODE* n=&_head;
	while (n->n!=NULL){
		if (n->id==id){
			return n->dt;
		}
		n=n->n;
	}
	if (n->id!=UINT8_MAX){
		n->n=malloc(sizeof(struct _ASSET_LIST_NODE));
		n=n->n;
		n->n=NULL;
	}
	n->id=id;
	n->dt=malloc(sizeof(struct _BLOCK_MODEL));
	size_t ln=strlen(ASSET_BASE_FILE_PATH);
	char* fp=malloc((ln+7)*sizeof(char));
	for (size_t i=0;i<ln;i++){
		*(fp+i)=*(ASSET_BASE_FILE_PATH+i);
	}
	*(fp+ln)=(id>>4)+(id>=160?39:0)+48;
	*(fp+ln+1)=(id&0x0f)+((id&0x0f)>=10?39:0)+48;
	*(fp+ln+2)='.';
	*(fp+ln+3)='m';
	*(fp+ln+4)='d';
	*(fp+ln+5)='l';
	*(fp+ln+6)=0;
	FILE* f=NULL;
	errno_t e=fopen_s(&f,fp,"rb");
	free(fp);
	assert(e==0);
	fseek(f,0,SEEK_END);
	size_t sz=ftell(f);
	rewind(f);
	n->dt->ib_l=(fgetc(f)&0xff)|((fgetc(f)&0xff)<<8)|((fgetc(f)&0xff)<<16)|((fgetc(f)&0xff)<<24);
	uint16_t* il=malloc(n->dt->ib_l*3*sizeof(uint16_t));
	assert(fread(il,sizeof(uint16_t),n->dt->ib_l*3,f)==n->dt->ib_l*3);
	sz=(sz-n->dt->ib_l*6-4)/sizeof(float);
	float* vl=malloc(sz*sizeof(float));
	assert(fread(vl,sizeof(float),sz,f)==sz);
	fclose(f);
	D3D11_BUFFER_DESC bd={
		(uint32_t)(n->dt->ib_l*3*sizeof(uint16_t)),
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
	HRESULT hr=ID3D11Device_CreateBuffer(renderer_d3_d,&bd,&dt,&(n->dt->ib));
	free(il);
	bd.ByteWidth=(uint32_t)(sz*sizeof(float));
	bd.BindFlags=D3D11_BIND_VERTEX_BUFFER;
	dt.pSysMem=vl;
	hr=ID3D11Device_CreateBuffer(renderer_d3_d,&bd,&dt,&(n->dt->vb));
	free(vl);
	return n->dt;
}



void resource_manager_draw_asset(AssetID a,RawMatrix tm,RawMatrix rm){
	assert(a!=NULL);
	if (a_cb==NULL){
		a_cb=create_constant_buffer(sizeof(struct CBufferLayout));
	}
	struct CBufferLayout cb_dt={
		tm,
		rm
	};
	update_constant_buffer(a_cb,(void*)&cb_dt);
	ID3D11DeviceContext_VSSetConstantBuffers(renderer_d3_dc,1,1,&a_cb);
	ID3D11DeviceContext_PSSetConstantBuffers(renderer_d3_dc,1,1,&a_cb);
	unsigned int off=0;
	unsigned int st=9*sizeof(float);
	ID3D11DeviceContext_IASetPrimitiveTopology(renderer_d3_dc,D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D11DeviceContext_IASetVertexBuffers(renderer_d3_dc,0,1,&a->vb,&st,&off);
	ID3D11DeviceContext_IASetIndexBuffer(renderer_d3_dc,a->ib,DXGI_FORMAT_R16_UINT,0);
	ID3D11DeviceContext_DrawIndexed(renderer_d3_dc,a->ib_l*3,0,0);
}
