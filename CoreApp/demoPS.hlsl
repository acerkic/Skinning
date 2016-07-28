//Texture2D ColorTexture : register(t0);
//SamplerState LinearSampler : register(s0);
//
//
//struct PS_INPUT
//{
//	float4 position : SV_POSITION;
//	float4 diffuse : COLOR;
//	float2 tex : TEXCOORD0;
//	float3 worldNorm : TEXCOORD1;
//	float3 worldPos : TEXCOORD2;
//	float3 toEye : TEXCOORD3;
//	float4 tangent : TEXCOORD4;
//	float3 normal : TEXCOORD5;
//	float3 light : LIGHT;
//
//};

Texture2D Texture1 : register(t0);
Texture2D Texture2 : register(t1);
Texture2D Texture3 : register(t2);
Texture2D Texture4 : register(t3);
Texture2D Texture5 : register(t4);
Texture2D Texture6 : register(t5);
Texture2D Texture7 : register(t6);
Texture2D Texture8 : register(t7);

TextureCube CubeTexture1 : register(t8);
TextureCube CubeTexture2 : register(t9);
TextureCube CubeTexture3 : register(t10);
TextureCube CubeTexture4 : register(t11);
TextureCube CubeTexture5 : register(t12);
TextureCube CubeTexture6 : register(t13);
TextureCube CubeTexture7 : register(t14);
TextureCube CubeTexture8 : register(t15);

SamplerState TexSampler : register(s0);

cbuffer MaterialVars : register (b0)
{
	float4 MaterialAmbient;
	float4 MaterialDiffuse;
	float4 MaterialSpecular;
	float4 MaterialEmissive;
	float MaterialSpecularPower;
};

cbuffer LightVars : register (b1)
{
	float4 AmbientLight;
	float4 LightColor[4];
	float4 LightAttenuation[4];
	float3 LightDirection[4];
	float LightSpecularIntensity[4];
	uint IsPointLight[4];
	uint ActiveLights;
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

cbuffer MiscVars : register(b3)
{
	float ViewportWidth;
	float ViewportHeight;
	float Time;
};

struct A2V
{
	float4 pos : POSITION0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};

struct V2P
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

struct P2F
{
	float4 fragment : SV_Target;
};


float4 main(in V2P input) : SV_TARGET
{
	//normalize world space and light vectors
	float3 n = normalize(input.normal);
	float3 l = normalize(LightDirection[0]);

	//calculate the amount of light reaching this fragment
	float4 Illumination = max(dot(n, l), 0) + .2;

	//determine the color properties of the texture 
	float4 SurfaceColor = Texture1.Sample(TexSampler, input.uv);
	
	return float4(SurfaceColor * Illumination );
}