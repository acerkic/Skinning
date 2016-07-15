cbuffer MaterialVars : register (b0)
{
    float4 MaterialAmbient;
    float4 MaterialDiffuse;
    float4 MaterialSpecular;
    float4 MaterialEmissive;
    float MaterialSpecularPower;
};
       
cbuffer ObjectVars : register(b2)
{
    float4x4 LocalToWorld4x4;
    float4x4 LocalToProjected4x4;
    float4x4 WorldToLocal4x4;
    float4x4 WorldToView4x4;
    float4x4 UVTransform4x4;
    float3 EyePosition;
};

struct VSInput
{
	float4 pos : POSITION0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};
	       
 struct VSOutput
 {
     float4 pos : SV_POSITION;
     float4 diffuse : COLOR;
     float2 uv : TEXCOORD0;
     float3 worldNorm : TEXCOORD1;
     float3 worldPos : TEXCOORD2;
     float3 toEye : TEXCOORD3;
     float4 tangent : TEXCOORD4;
     float3 normal : TEXCOORD5;
 };
	       


 VSOutput main(VSInput input)
{
	 VSOutput result;
     
     float3 wp = mul(input.pos, LocalToWorld4x4).xyz;
     
     // Set output data.
     result.pos = mul(input.pos, LocalToProjected4x4);
     result.diffuse = input.color * MaterialDiffuse;
     result.uv = mul(float4(input.uv.x, input.uv.y, 0, 1), UVTransform4x4).xy;
     result.worldNorm = mul(input.normal, (float3x3)LocalToWorld4x4);
     result.worldPos = wp;
     result.toEye = EyePosition - wp;
     result.tangent = input.tangent;
     result.normal = input.normal;
     
     return result;
}