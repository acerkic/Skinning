#pragma once

#include "StepTimer.h"
#include "Common\DeviceResources.h"
#include "SimpleMath.h"
//#include "Model.h"
//#include "Effects.h"
//#include "CommonStates.h"
//#include "ShaderStructures.h"
#include "common\VSD3DStarter.h"
#include "Common\Animation.h"



// Renders Direct2D and 3D content on the screen.
namespace CoreApp
{
	class CoreAppMain : public DX::IDeviceNotify
	{
	public:
		CoreAppMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~CoreAppMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();
		void CreateDeviceDependentResources();
		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		
		// Rendering loop timer.
		DX::StepTimer m_timer;
		float rotation;
		/*std::unique_ptr<DirectX::Model> model;
		std::unique_ptr<DirectX::IEffectFactory> factory;
		std::unique_ptr<DirectX::CommonStates> states;*/
		bool psDone, vsDone;

		DirectX::SimpleMath::Matrix view;
		DirectX::SimpleMath::Matrix proj;
		DirectX::SimpleMath::Matrix world;

		/*Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
		DirectX::StaticMeshTransforms m_constantBufferData;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_SkinnedconstantBuffer;
		DirectX::SkinningMeshTransforms m_SkinnedconstantBufferData;
	
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_LightConstantBuffer;
		DirectX::LightParameters m_lightConstantBufferData;

		
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;*/
		float m_x;
		//Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
		//Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
		//Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;



		///vsd3d stuff
		SkinnedMeshRenderer m_skinnedMeshRenderer;
		std::vector<VSD3DStarter::Mesh*> m_meshModels;
		std::vector<float> m_time;

		VSD3DStarter::Graphics m_graphics;
		VSD3DStarter::LightConstants m_lightConstants;
		VSD3DStarter::MiscConstants m_miscConstants;

		bool m_loaded;
		
	};
}