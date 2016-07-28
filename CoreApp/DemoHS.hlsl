struct VS_OUTPUT
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float3 light : LIGHT;
	float2 tex : TEXCOORD0;

};

struct HS_POINT_OUTPUT
{
	float4 position: SV_Position;
	float3 normal : NORMAL;
	float3 light : LIGHT;
	float2 tex : TEXCOORD0;
};


struct HS_PATCH_OUTPUT
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;

};

struct DS_OUTPUT
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float3 light : LIGHT;
	float2 tex : TEXCOORD0;


};

HS_PATCH_OUTPUT HSPATCH(InputPatch<VS_OUTPUT,3> ip, uint PatchID : SV_PrimitiveID)
{
	HS_PATCH_OUTPUT output;

	const float factor = 16.0f;

	output.Edges[0] = factor;
	output.Edges[1] = factor;
	output.Edges[2] = factor;

	output.Inside = factor;
	return output;
	
}


[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPATCH")]
HS_POINT_OUTPUT main(
	InputPatch<VS_OUTPUT, 3> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_POINT_OUTPUT output;

	output.position = ip[i].position;
	output.normal = ip[i].normal;
	output.light = ip[i].light;
	output.tex = ip[i].tex;


	return output;
}
