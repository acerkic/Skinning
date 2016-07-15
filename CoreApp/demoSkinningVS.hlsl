cbuffer SkinningTransforms
{
	matrix WorldMatrix;
	matrix ViewProjMatrix;
	matrix SkinMatrices[6];
	matrix SkinNormalMatrices[6];
};

cbuffer LightParameters
{
	float3 LightPositionWS;
	float4 LightColor;
};

struct VS_INPUT
{
	float3 position :	SV_POSITION;
	float4 normal	:	NORMAL;
	float4 tangent	:	TANGENT;
	float4 color	:	COLOR;
	float2 tex		:	TEXCOORD;
	uint4  bone		:	BONEIDS;
	float4 weights	:	BONEWEIGHTS;

};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float3 normal :NORMAL;
	float2 tex : TEXCOORD0;
	float3 light : LIGHT;
};


VS_OUTPUT main( in VS_INPUT input)
{
	VS_OUTPUT output;
	
	output.position =  (mul(float4(input.position, 1.0f), SkinMatrices[input.bone.x]) * input.weights.x);
	output.position += (mul(float4(input.position, 1.0f), SkinMatrices[input.bone.y]) * input.weights.y);
	output.position += (mul(float4(input.position, 1.0f), SkinMatrices[input.bone.z]) * input.weights.z);
	output.position += (mul(float4(input.position, 1.0f), SkinMatrices[input.bone.w]) * input.weights.w);

	//transform worl position with viewprojection Matrix
	output.position = mul(output.position, ViewProjMatrix);

	//calc world space normal
	output.normal =  (mul(input.normal, (float3x3) SkinNormalMatrices[input.bone.x]) * input.weights.x).xyz;
	output.normal += (mul(input.normal, (float3x3) SkinNormalMatrices[input.bone.y]) * input.weights.y).xyz;
	output.normal += (mul(input.normal, (float3x3) SkinNormalMatrices[input.bone.w]) * input.weights.z).xyz;
	
	//calculate the world space light vector
	output.light = LightPositionWS - output.position.xyz;
	
	//pass the texture coordinates
	output.tex = input.tex;
	
	return output;
}