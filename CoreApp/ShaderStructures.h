#pragma once

#include <DirectXMath.h>
namespace DirectX
{

	struct SkinningMeshTransforms
	{
		DirectX::XMFLOAT4X4 WorldMatrix;
		DirectX::XMFLOAT4X4 WorldViewProjMatrix;
		DirectX::XMFLOAT4X4 SkinMatrices[6];
		DirectX::XMFLOAT4X4 SkinNormalMatrices[6];
	};


	struct StaticMeshTransforms
	{
		DirectX::XMFLOAT4X4 WorldMatrix;
		DirectX::XMFLOAT4X4 WorldViewProjMatrix;

	};

	struct LightParameters
	{

		DirectX::XMFLOAT3 LightPositionWS;
		DirectX::XMFLOAT4 LightColor;
	};
}