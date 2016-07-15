#include "pch.h"
#include "CoreAppMain.h"

#include "DirectXHelper.h"
#include "DDSTextureLoader.h"


using namespace CoreApp;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace VSD3DStarter;

// Loads and initializes application assets when the application is loaded.
CoreAppMain::CoreAppMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);
	
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
	
	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

CoreAppMain::~CoreAppMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);

}

// Updates application state when the window size changes (e.g. device orientation change)
void CoreAppMain::CreateWindowSizeDependentResources() 
{
	//set viewport size and update the constant buffer
	m_miscConstants.ViewportHeight = m_deviceResources->GetScreenViewport().Height;
	m_miscConstants.ViewportWidth = m_deviceResources->GetScreenViewport().Width;
	m_graphics.UpdateMiscConstants(m_miscConstants);

	//Set camera orientation
	m_graphics.GetCamera().SetOrientationMatrix(m_deviceResources->GetOrientationTransform3D());

	Size outputSize = m_deviceResources->GetOutputSize();

	//set the camera parameters for the scene
	m_graphics.GetCamera().SetViewport((UINT)outputSize.Width, (UINT)outputSize.Height);
	m_graphics.GetCamera().SetPosition(XMFLOAT3(0.0f, 6.0f, -18.0f));
	m_graphics.GetCamera().SetLookAt(XMFLOAT3(0.0f, 0.0f, 0.0f));

	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	if (aspectRatio < 1.0f)
	{
		//portrait view
		m_graphics.GetCamera().SetUpVector(XMFLOAT3(1.0f, 0.0f, 0.0f));
		fovAngleY = 120.0f * XM_PI / 180.0f;
	}
	else
	{
		//landscape
		m_graphics.GetCamera().SetUpVector(XMFLOAT3(0.0f, 1.0f, 0.0f));
	}
		
	//set projection
	m_graphics.GetCamera().SetProjection(fovAngleY, aspectRatio, 1.0f, 1000.0f);


	//create light for the scene
	static const XMVECTORF32 lightPos = {5.0f, 5.0f, -2.5f, 0.f};

	XMFLOAT4 dir;
	DirectX::XMStoreFloat4(&dir, XMVector3Normalize(lightPos));

	//set light constants
	m_lightConstants.ActiveLights = 1;
	m_lightConstants.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_lightConstants.IsPointLight[0]=false;
	m_lightConstants.LightColor[0] = XMFLOAT4(.8f, .8f, .8f, 1.0f);
	m_lightConstants.LightDirection[0] = dir;
	m_lightConstants.LightSpecularIntensity[0].x = 2;

	m_graphics.UpdateLightConstants(m_lightConstants);

//	Size size = m_deviceResources->GetOutputSize();
//
//	float aspectRatio = size.Width / size.Height;
//	float fovAngleY = 70.0f * XM_PI / 180.0f;
//	if (aspectRatio < 1.0)
//	{
//		fovAngleY = 2.0f;
//	}
//	XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovRH(fovAngleY,aspectRatio, 0.1f, 1000);
//	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
//	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
//	proj = projectionMatrix * orientationMatrix;
//
//	world = Matrix::Identity * Matrix::CreateScale(.02f, .02f, .02f) * Matrix::CreateRotationY(rotation) * Matrix::CreateTranslation(0, -10, -30.f);
//	
//	/*ZeroMemory(&m_SkinnedconstantBufferData, sizeof(m_SkinnedconstantBufferData));
//	XMStoreFloat4x4(&m_SkinnedconstantBufferData.WorldMatrix,world);
//*/
//	static const XMVECTORF32 eye = {0.0f, 0.7f, 1.5f, 0.0f};
//	static const XMVECTORF32 at = {0.0f, -.1f, 0.0f, 0.0f};
//	static const XMVECTORF32 up = {0.0f, 1.0f, 0.0f, 0.0f};
//
//	view = XMMatrixLookAtRH(eye, at, up);
	
//	XMStoreFloat4x4(&m_SkinnedconstantBufferData.WorldViewProjMatrix, XMMatrixTranspose(world * view * proj));
}

void CoreAppMain::CreateDeviceDependentResources()
{
	///init graphics
	//Sets Device, Context, Feature set
	//Creates constants for : Material, light, object, and misc
	//Creates sampler
	//creates standard input layout
	//creates standard vertex shader from file VSD3dshadervs
	//Creates a new "empty" texture so that pixel shader works
	m_graphics.Initialize(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext(),m_deviceResources->GetDeviceFeatureLevel());

	//Create rasterizer state for no culling
	CD3D11_RASTERIZER_DESC d3dRas(D3D11_DEFAULT);
	d3dRas.CullMode = D3D11_CULL_NONE;
	d3dRas.MultisampleEnable = true;
	d3dRas.AntialiasedLineEnable = true;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> p3dRastState;
	//create Rasterizer state
	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&d3dRas, &p3dRastState);
	//set rasterizer state
	m_deviceResources->GetD3DDeviceContext()->RSSetState(p3dRastState.Get());

	//load mesh

	auto loadMesh = Mesh::LoadFromFileAsync(
	m_graphics,
	L"bendy.cmo",
	L"", //shader location
	L"", //texture location
		m_meshModels).then([this]()  //When done loading file, reset the animation
	{
		for (Mesh* m : m_meshModels)
		{
			if (m->BoneInfoCollection().empty() == false) //are there bones?
			{
				auto animState = new AnimationState();
				animState->m_boneWorldTransforms.resize(m->BoneInfoCollection().size());

				m->Tag = animState;
			}
		}
		
		//each mesh has a its own "Time" used to control the glow effect?
		m_time.clear();
	
		for (size_t i = 0; i < m_meshModels.size(); i++)
		{
			m_time.push_back(0.0f);
			
		}
	});


	//Init skinded Mesh rendrer
	auto initSkinMeshRenderer = m_skinnedMeshRenderer.InitializeAsync(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());


	//once tasks are done, the scene is ready to render. 
	(loadMesh && initSkinMeshRenderer).then([this]() {m_loaded = true; });
	

	//factory = std::make_unique<EffectFactory>(m_deviceResources->GetD3DDevice());

	//model = Model::CreateFromCMO(m_deviceResources->GetD3DDevice(), L"bendy.cmo", *factory);
	//DirectX::CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\check.dds", NULL, m_texture.GetAddressOf());
	//
	//auto loadVSTask = DX::ReadDataAsync(L"demoSkinningVS.cso");
	//auto loadPSTask = DX::ReadDataAsync(L"demoPS.cso");
	//
	//auto createVSTask = loadVSTask.then([this](const std::vector<byte>& filedata)
	//{
	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateVertexShader(
	//			&filedata[0],
	//			filedata.size(),
	//			nullptr,
	//			&m_vertexShader
	//		)
	//	);
	//	
	//	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	//	{
	//		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "COLOR",       0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "BONEIDS",	 0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "BONEWEIGHTS", 0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	};


	//	//Mesh no skinning info
	//	/*static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	//	{
	//		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "COLOR",       0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	//	};*/

	//	DX::ThrowIfFailed(
	//		m_deviceResources->GetD3DDevice()->CreateInputLayout(
	//			vertexDesc,
	//			ARRAYSIZE(vertexDesc),
	//			&filedata[0],
	//			filedata.size(),
	//			&m_inputLayout)
	//		);
	//	vsDone = true;
	//});

	//auto createPSTask = loadPSTask.then([this](const std::vector<byte>& filedata)
	//	{
	//		DX::ThrowIfFailed(
	//			m_deviceResources->GetD3DDevice()->CreatePixelShader(
	//				&filedata[0],
	//				filedata.size(),
	//				nullptr,
	//				&m_pixelShader
	//			)		
	//		);

	//	

	//		psDone = true;
	//	});

	////Skinned constant buffer creation
	//CD3D11_BUFFER_DESC SkinnedconstantBufferDesc(sizeof(SkinningMeshTransforms), D3D11_BIND_CONSTANT_BUFFER);

	//DX::ThrowIfFailed(
	//	m_deviceResources->GetD3DDevice()->CreateBuffer(
	//		&SkinnedconstantBufferDesc,
	//		nullptr,
	//		&m_SkinnedconstantBuffer
	//	)
	//);

	//if (m_SkinnedconstantBuffer.Get() == nullptr)
	//{
	//	int shit = 0;
	//}

	////Create light buffer
	//D3D11_BUFFER_DESC lightBufferDesc;
	//ZeroMemory(&lightBufferDesc, sizeof(lightBufferDesc));
	//lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//lightBufferDesc.ByteWidth = sizeof(LightParameters) + (16 * 2) - (sizeof(LightParameters));
	//lightBufferDesc.CPUAccessFlags = 0;
	//
	//m_lightConstantBufferData = {};
	//m_lightConstantBufferData.LightColor = DirectX::SimpleMath::Vector4(1, 1, 1,1);
	//m_lightConstantBufferData.LightPositionWS = DirectX::SimpleMath::Vector3( 0.0f, 0.0f, -30.0f);

	//m_deviceResources->GetD3DDevice()->CreateBuffer(&lightBufferDesc, nullptr, m_LightConstantBuffer.GetAddressOf());

	rotation = 0.0;
	
}


// Updates the application state once per frame.
void CoreAppMain::Update() 
{
	// Update scene objects.
	m_timer.Tick([this]()
	{
		// TODO: Replace this with your app's content update functions.
	
		float timeDelta = static_cast<float>(m_timer.GetElapsedSeconds());
		rotation += 1.f * timeDelta;
		m_skinnedMeshRenderer.UpdateAnimation(m_timer.GetElapsedSeconds(),m_meshModels, L"Armature|test");

		//Update time for glow effect
		for (float &time : m_time)
		{
			time = std::max<float>(0.0f, time - timeDelta);
		}
	});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool CoreAppMain::Render() 
{
	if (!m_loaded)
		return true;

	auto context = m_deviceResources->GetD3DDeviceContext();

	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	
	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::DarkSlateGray);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);



	XMMATRIX modelRotation = XMMatrixRotationY(rotation);
	//set render targets
	auto rtv = m_deviceResources->GetBackBufferRenderTargetView();
	auto dsv = m_deviceResources->GetDepthStencilView();

	ID3D11RenderTargetView *const targets[1] = {rtv};
	context->OMSetRenderTargets(1, targets, dsv);

	for (UINT i = 0; i < m_meshModels.size(); i++)
	{

		//update the time shader variable for the objects in the scene
		m_miscConstants.Time = m_time[i];
		m_graphics.UpdateMiscConstants(m_miscConstants);
	

		if (m_meshModels[i]->Tag != nullptr)
		{
			m_skinnedMeshRenderer.RenderSkinnedMesh(m_meshModels[i], m_graphics, modelRotation);
		}
		else
		{
			m_meshModels[i]->Render(m_graphics, modelRotation);
		
		}
	
	}



	// Don't try to render anything before the first Update.
	//if (m_timer.GetFrameCount() == 0)
	//{
	//	return false;
	//}

	//if (!psDone && !vsDone)
	//	return true;

	//world = Matrix::Identity * Matrix::CreateScale(.02f, .02f, .02f) * Matrix::CreateRotationY(rotation) * Matrix::CreateTranslation(0, -10, -20.f);
	////XMStoreFloat4x4(&m_constantBufferData.WorldViewProjMatrix, XMMatrixTranspose(world * view * proj));
	////XMStoreFloat4x4(&m_constantBufferData.WorldMatrix, world);
	////
	//auto context = m_deviceResources->GetD3DDeviceContext();

	//// Reset the viewport to target the whole screen.
	//auto viewport = m_deviceResources->GetScreenViewport();
	//context->RSSetViewports(1, &viewport);

	//// Reset render targets to the screen.
	//ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	//context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	//// Clear the back buffer and depth stencil view.
	//context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	//context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//

	//Set blend state 
	//context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);
	////set the DepthStencil state
	//context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
	//context->RSSetState(m_rasterizerState.Get());

	//if (m_SkinnedconstantBuffer.Get() == nullptr)
	//{
	//	int shit = 0;
	//
	//}

	
	////Use Model to render using the DXTool Kit 
	//states=std::unique_ptr<CommonStates>(new CommonStates(m_deviceResources->GetD3DDevice()));
	//CommonStates sts(m_deviceResources->GetD3DDevice());
	//for (auto i = model->meshes.cbegin(); i != model->meshes.cend(); ++i)
	//{
	//	ModelMesh* mesh = i->get();
	//	for (auto part = mesh->meshParts.cbegin(); part != mesh->meshParts.cend(); ++part)
	//	{
	//
	//		ModelMeshPart * meshPart = (*part).get();
	//		auto effect = dynamic_cast<IEffectSkinning*>(meshPart->effect.get());
	//		effect->SetBoneTransforms(&XMMatrixTranslation(m_x, 0.0f, 0.0f), 1);
	//	}
	//
	//}

	//model->Draw(context, sts, world, view, proj);
	//return true;


	///Render meshes
	//for (auto i = model->meshes.cbegin(); i != model->meshes.cend(); ++i)
	//{
	//	ID3D11DepthStencilState* depthStencilState;
	//	ID3D11BlendState* blendState;
	//	
	//	//Every Mesh
	//	//Prep for rendering
	//	D3D11_BLEND_DESC desc = {};

	//	desc.RenderTarget[0].BlendEnable = D3D11_BLEND_ONE;
	//	desc.RenderTarget[0].SrcBlend = desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	//	desc.RenderTarget[0].DestBlend = desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	//	desc.RenderTarget[0].BlendOp = desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	//	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//	m_deviceResources->GetD3DDevice()->CreateBlendState(&desc, &blendState);
	//	
	//	//Set blend state 
	//	context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
	//	
	//	D3D11_DEPTH_STENCIL_DESC Depthdesc = {};
	//	ZeroMemory(&Depthdesc, sizeof(Depthdesc));

	//	Depthdesc.DepthEnable = true;
	//	Depthdesc.DepthWriteMask =  D3D11_DEPTH_WRITE_MASK_ALL;
	//	Depthdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	//	Depthdesc.StencilEnable = false;
	//	Depthdesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	//	Depthdesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	//	Depthdesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	//	Depthdesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//	Depthdesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//	Depthdesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	//
	//	m_deviceResources->GetD3DDevice()->CreateDepthStencilState(&Depthdesc, &depthStencilState);
	//	
	//	//set the DepthStencil state
	//	context->OMSetDepthStencilState(depthStencilState, 0);
	//	///end of prep for Mesh rendering

	//	ID3D11RasterizerState *rast;
	//	D3D11_RASTERIZER_DESC cullDesc = {};

	//	cullDesc.CullMode = D3D11_CULL_BACK;
	//	cullDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	//	cullDesc.DepthClipEnable = true;
	//	cullDesc.MultisampleEnable = true;

	//	m_deviceResources->GetD3DDevice()->CreateRasterizerState(&cullDesc, &rast);
	//	
	//	//Set Rasterizer State
	//	context->RSSetState(rast);
	//		
	//	D3D11_SAMPLER_DESC samplerDesc = {};

	//	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	//	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//	samplerDesc.MipLODBias = 0.0f;
	//	samplerDesc.MaxAnisotropy = (m_deviceResources->GetD3DDevice()->GetFeatureLevel() > D3D_FEATURE_LEVEL_9_1) ? 16 : 2;
	//	samplerDesc.MinLOD = 0;
	//	samplerDesc.MaxLOD = FLT_MAX;
	//	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

	//	m_deviceResources->GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_sampler);
	//	 
	//	 //sampler needs to be passed as an array
	//	 ID3D11SamplerState* samplerStates[2];
	//	 samplerStates[0] = m_sampler.Get();

	//	 //Set sampler state.
	//	 context->PSSetSamplers(0, 1, samplerStates);

	// ModelMesh* mesh = i->get();
	//	 
	//	 for (int boneIndex = 0; boneIndex < 6; ++boneIndex)
	//	 {
	//		 m_SkinnedconstantBufferData.SkinMatrices[boneIndex] = mesh->animationClips[L"Armature|PoseLib"].Keyframes.at(0).Transform;
	//		 m_SkinnedconstantBufferData.SkinNormalMatrices[boneIndex] = mesh->animationClips[L"Armature|PoseLib"].Keyframes.at(0).Transform;
	//	 }
	//		 
	//	 context->UpdateSubresource1(
	//		 m_SkinnedconstantBuffer.Get(),
	//		 0,
	//		 NULL,
	//		 &m_SkinnedconstantBufferData,
	//		 0,
	//		 0,
	//		 0
	//	 );


	//	for (auto part = mesh->meshParts.cbegin(); part != mesh->meshParts.cend(); ++part)
	//	{

	//		//Every MeshPart
	//		ModelMeshPart * meshPart = (*part).get();

	//		context->IASetInputLayout(m_inputLayout.Get());

	//		context->IASetVertexBuffers(
	//			meshPart->startIndex,
	//			1,
	//			meshPart->vertexBuffer.GetAddressOf(),
	//			&meshPart->vertexStride,
	//			&meshPart->vertexOffset
	//		);

	//		context->IASetIndexBuffer(
	//			meshPart->indexBuffer.Get(),
	//			meshPart->indexFormat,
	//			meshPart->vertexOffset
	//		);
	//	 
	//	
	//		
	//		context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
	//	
	//		context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	//		context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	//	
	//		context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, 0);
	//		context->PSSetConstantBuffers1(0, 1, m_LightConstantBuffer.GetAddressOf(), nullptr, 0);

	//		context->IASetPrimitiveTopology(meshPart->primitiveType);
	//		context->DrawIndexed(meshPart->indexCount, 0, 0);
	//	}
	//}

	return true;
}


// Notifies renderers that device resources need to be released.
void CoreAppMain::OnDeviceLost()
{
	for (Mesh * m : m_meshModels)
	{
		if (m != nullptr)
		{
			AnimationState* animState = (AnimationState*)m->Tag;
			if (animState != nullptr)
			{
				m->Tag = nullptr;
				delete animState;
			}
		}
		delete m;
	}

	m_meshModels.clear();
	m_loaded = false;

	/*model.reset();
	factory.reset();
	states.reset();
	m_constantBuffer.Reset();

	m_SkinnedconstantBuffer.Reset();

	m_LightConstantBuffer.Reset();

	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_sampler.Reset();
	m_texture.Reset();

	m_depthStencilState.Reset();
	m_blendState.Reset();
	m_rasterizerState.Reset();*/
}

// Notifies renderers that device resources may now be recreated.
void CoreAppMain::OnDeviceRestored()
{
	
	CreateWindowSizeDependentResources();
}
