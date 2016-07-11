struct VS_INPUT
{
	float3 position :	SV_POSITION;
	float4 normal	:	NORMAL;
	float4 tangent	:	TANGENT;
	float4  color	:	COLOR;
	float2 tex		:	TEXCOORD;
	uint4   bone		:	BONEIDS;
	float4 weights	:	BONEWEIGHTS;

};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float4 light :LIGHT;
	float2 tex : TEXCOORD0;
};



VS_OUTPUT main( in VS_INPUT input)
{
	VS_OUTPUT output;
	output.position = float4(input.position,1);
	output.light = float4(0, 0, 0, 0);
	output.tex = input.tex;

	output.normal = input.normal;

	return output;
}