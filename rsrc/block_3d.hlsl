#pragma pack_matrix(row_major)



static float3 light_dir=float3(-0.4767312946227962, 0.6674238124719146, -0.5720775535473555);



cbuffer core:register(b0){
	matrix cm;
	matrix pm;
};



cbuffer extra:register(b1){
	matrix tm;
	matrix rm;
};



struct VS_OUT{
	float4 p:SV_POSITION;
	float4 c:COLOR;
	float4 n:NORMAL;
};



VS_OUT block_3d_vs(float4 p:POSITION,float4 c:COLOR,float4 n:NORMAL){
	VS_OUT o={
		mul(mul(mul(p,tm),cm),pm),
		c,
		mul(n,rm)
	};
	return o;
}



float map(float v,float aa,float ab,float ba,float bb){
	return (v-aa)/(ab-aa)*(bb-ba)+ba;
}



float4 block_3d_ps(VS_OUT vo):SV_TARGET{
	return vo.c*map(dot(vo.n.xyz,light_dir),-1,1,0.2,1);
}
