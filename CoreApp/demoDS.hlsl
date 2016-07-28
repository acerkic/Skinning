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
	float4 position  : SV_POSITION;
	float3 normal : NORMAL;
	float3 light : LIGHT;
	float2 tex : TEXCOORD0;

};

Texture2D       ColorTexture : register(t0);
Texture2D       HeightTexture : register(t1);
SamplerState    LinearSampler : register(s0);


cbuffer ObjectVars : register(b2)
{
	float4x4 LocalToWorld4x4;
	float4x4 LocalToProjected4x4;
	float4x4 WorldToLocal4x4;
	float4x4 WorldToView4x4;
	float4x4 UVTransform4x4;
	float3 EyePosition;
};


[domain("tri")]
DS_OUTPUT main(
	const OutputPatch<HS_POINT_OUTPUT,3> TriPatch, float3 Coords : SVDomainLocation, HS_PATCH_OUTPUT input)
{
	DS_OUTPUT output;

	//interpolate world space  //first patch was at 6 not 0 ... not sure why
	float4 vWorldPos = Coords.x * TriPatch[0].position + 
		Coords.y * TriPatch[1].position +
		Coords.z * TriPatch[2].position;

	//calc the interpolated normal
	output.normal = Coords.x * TriPatch[0].normal +
		Coords.y * TriPatch[1].normal +
		Coords.z * TriPatch[2].normal;

	//norm the vector for displacement
	output.normal = normalize(output.normal);

	//interpolate the texture coordinates
	output.tex = Coords.x * TriPatch[0].tex +
		Coords.y * TriPatch[1].tex +
		Coords.z * TriPatch[2].tex;

	//calculate the interpolated world space light vector
	output.light = Coords.x * TriPatch[0].light +
		Coords.y * TriPatch[1].light +
		Coords.z * TriPatch[2].light;

	
	//calc MIP level to fectch the nromal from
	float fHeightMapMipLevel =
		clamp((distance(vWorldPos.xyz, float3(-20.0f, 25.0f, 10.0f)) - 100.0f)/100.0f, 0.0f, 3.0f); ///float3 is supposed to be a vEye position
	
	//sample the height map to know how much to displace
	float texHeight = HeightTexture.SampleLevel(LinearSampler, output.tex, fHeightMapMipLevel);
	
	//do the displacement. the fscale param determones max world space offset that can 
	//be applied to the surface. The displacement is performed along the interpolated
	//vertex normal vector
	const float fscale = 0.5f;
	vWorldPos.xyz = vWorldPos.xyz + output.normal * texHeight.r * fscale;

	//transform world position with the viewprojection matrix

	output.position = mul(vWorldPos, LocalToProjected4x4);

	return output;
}
