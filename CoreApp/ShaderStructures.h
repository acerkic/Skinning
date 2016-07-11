#pragma once

#include <DirectXMath.h>
namespace DirectX
{
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