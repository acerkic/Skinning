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
	static const XMVECTORF32 lightPos = {0.0f, 3.0f, 5.0f, 0.f};

	XMFLOAT4 dir;
	DirectX::XMStoreFloat4(&dir, XMVector3Normalize(lightPos));

	//set light constants
	m_lightConstants.ActiveLights = 1;
	m_lightConstants.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_lightConstants.IsPointLight[0]=false;
	m_lightConstants.LightColor[0] = XMFLOAT4(.8f, .8f, .8f, 1.0f);
	m_lightConstants.LightDirection[0] = dir;
	m_lightConstants.LightSpecularIntensity[0].x = 3;

	m_graphics.UpdateLightConstants(m_lightConstants);

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
	m_graphics.Initialize(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext(), m_deviceResources->GetDeviceFeatureLevel());

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
		L"assets\\check.dds", //texture location
		m_meshModels, true, L"demoPS.cso").then([this]()  //When done loading file, reset the animation
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

	auto loadDomainShaderTask = DX::ReadDataAsync(L"demoDS.cso");
	auto loadHullShaderTask = DX::ReadDataAsync(L"demoHS.cso");

	loadDomainShaderTask.then([this](const std::vector<byte>& data) {

		m_deviceResources->GetD3DDevice()->CreateDomainShader(
			&data[0],
			data.size(),
			nullptr,
			m_domainShader.GetAddressOf()
		);

		if (m_domainShader)
			domain_loaded = true;
	});




	loadHullShaderTask.then([this](const std::vector<byte>& data) {

		m_deviceResources->GetD3DDevice()->CreateHullShader(
		&data[0],
		data.size(),
		nullptr,
		m_hullShader.GetAddressOf()
		);

		if (m_hullShader)
			hull_loaded = true;

	});

	//Init skinded Mesh rendrer
	auto initSkinMeshRenderer = m_skinnedMeshRenderer.InitializeAsync(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());

	(loadMesh && initSkinMeshRenderer).then([this]() {
		
		if (domain_loaded && hull_loaded)
		{
			m_loaded = true;
		}
	});

	//once tasks are done, the scene is ready to render. 
	
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

	XMMATRIX modelRotation = XMMatrixRotationX(-90); //XMMatrixRotationY(rotation);
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
			m_skinnedMeshRenderer.RenderSkinnedMesh(m_meshModels[i], m_graphics, modelRotation,m_domainShader.Get(), m_hullShader.Get());
		}
		else
		{
			m_meshModels[i]->Render(m_graphics, modelRotation);
		}
	}
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

}

// Notifies renderers that device resources may now be recreated.
void CoreAppMain::OnDeviceRestored()
{
	CreateWindowSizeDependentResources();
}
