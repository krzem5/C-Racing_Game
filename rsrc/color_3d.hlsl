#pragma pack_matrix(row_major)



cbuffer core:register(b0){
	matrix cm;
	matrix pm;
};



struct VS_OUT{
	float4 p:SV_POSITION;
	float4 c:COLOR;
};



VS_OUT color_3d_vs(float4 p:POSITION,float4 c:COLOR){
	VS_OUT o={
		mul(mul(p,cm),pm),
		c
	};
	return o;
}



float4 color_3d_ps(float4 p:SV_POSITION,float4 c:COLOR):SV_Target{
	return c;
}
