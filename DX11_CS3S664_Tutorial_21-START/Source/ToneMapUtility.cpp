#include "stdafx.h"
#include "ToneMapUtility.h"
#include "Scene.h"
#include <Model.h>
#include <Quad.h>
#include <Effect.h>
#include <VertexStructures.h>


ToneMapUtility::ToneMapUtility(ID3D11Device *deviceIn, ID3D11DeviceContext *contextIn)
{
	device = deviceIn;
	context = contextIn;
	setupRenderTargets();
	char *tmpShaderBytecode = nullptr;

	ID3D11InputLayout	*screenQuadVSInputLayout = nullptr;
	SIZE_T shaderBytes = LoadShader("Shaders\\cso\\screen_quad_vs.cso", &tmpShaderBytecode);
	device->CreateVertexShader(tmpShaderBytecode, shaderBytes, NULL, &screenQuadVS);
	device->CreateInputLayout(basicVertexDesc, ARRAYSIZE(basicVertexDesc), tmpShaderBytecode, shaderBytes, &screenQuadVSInputLayout);
	screenQuad = new Quad(device, screenQuadVSInputLayout);
	//shaderBytes=LoadShader("Shaders\\cso\\per_pixel_lighting_vs.cso", &tmpShaderBytecode);
	//device->CreateVertexShader(tmpShaderBytecode, shaderBytes, NULL, &perPixelLightingVS);
	//shaderBytes = LoadShader("Shaders\\cso\\convolve_u_ps.cso", &tmpShaderBytecode);
	//device->CreatePixelShader(tmpShaderBytecode, shaderBytes, NULL, &horizontalBlurPS);
	//shaderBytes = LoadShader("Shaders\\cso\\convolve_v_ps.cso", &tmpShaderBytecode);
	//device->CreatePixelShader(tmpShaderBytecode, shaderBytes, NULL, &verticalBlurPS);
	//shaderBytes = LoadShader("Shaders\\cso\\emissive_ps.cso", &tmpShaderBytecode);
	//device->CreatePixelShader(tmpShaderBytecode, shaderBytes, NULL, &emissivePS);
	shaderBytes = LoadShader("Shaders\\cso\\tone_map_ps.cso", &tmpShaderBytecode);
	device->CreatePixelShader(tmpShaderBytecode, shaderBytes, NULL, &textureCopyPS);
	//shaderBytes = LoadShader("Shaders\\cso\\copy_depth_ps.cso", &tmpShaderBytecode);
	//device->CreatePixelShader(tmpShaderBytecode, shaderBytes, NULL, &depthCopyPS);
	
	defaultEffect = new Effect(device, "Shaders\\cso\\basic_colour_vs.cso", "Shaders\\cso\\basic_colour_ps.cso", basicVertexDesc, ARRAYSIZE(basicVertexDesc));


}
void swapTexturePointers(ID3D11ShaderResourceView*SRV1, ID3D11ShaderResourceView *SRV2, ID3D11RenderTargetView *RTV1, ID3D11RenderTargetView *RTV2)
{
	ID3D11ShaderResourceView				*tmpSRV = SRV1; SRV1 = SRV2; SRV2 = tmpSRV;
	ID3D11RenderTargetView					*tmpRTV = RTV1; RTV1 = RTV2; RTV2 = tmpRTV;
}
ID3D11ShaderResourceView* ToneMapUtility::computeMeanIntensity(Scene*scene)
{
	// Draw the Orb 
	if (scene) {
		swapTexturePointers(offScreenSRV, offScreenSRV2, offScreenRTV, offScreenRTV2);
		// Ensure default states
		FLOAT			blendFactor[] = { 1, 1, 1, 1 };
		context->OMSetDepthStencilState(defaultEffect->getDepthStencilState(), 0);
		context->OMSetBlendState(defaultEffect->getBlendState(), blendFactor, 0xFFFFFFFF);

		// Store Scene render target and depth buffer to restore when finished
		ID3D11RenderTargetView		* tempRT[1] = { 0 };
		ID3D11DepthStencilView		*tempDS = nullptr;
		context->OMGetRenderTargets(1, tempRT, &tempDS);

		// Clear off screen render targets
		FLOAT clearColorBlur[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->ClearRenderTargetView(offScreenRTV, clearColorBlur);
		context->ClearDepthStencilView(offScreenDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Store Scene viewport to put them back when finished
		D3D11_VIEWPORT 		currentVP;
		UINT nCurrentVP = 1;
		context->RSGetViewports(&nCurrentVP, &currentVP);
		// Set Viewport Offscreen 
		context->RSSetViewports(1, &offScreenViewport);
		
		//copy multisample depth stencil (depthSRV) to non-ms depth stencil (depthStencilViewOrb)
		context->OMSetRenderTargets(1, &offScreenRTV, offScreenDSV);

		context->PSSetShaderResources(7, 1, &offScreenSRV2);
		scene->renderSceneObjs();

		// Restore Scene viewport 
		context->RSSetViewports(1, &currentVP);

		// Create "nullSRV"  to release intermedVSRV shader resource view
		ID3D11ShaderResourceView	*nullSRV[1]; nullSRV[0] = NULL;
		context->PSSetShaderResources(7, 1, nullSRV);

		// Restore Scene depth buffer.
		context->OMSetRenderTargets(1, tempRT, tempDS);
		context->GenerateMips(offScreenSRV);
	/*	tempRT[0]->Release();
		tempDS->Release();*/
		//context->OMSetBlendState(defaultEffect->getBlendState(), blendFactor, 0xFFFFFFFF);
	}
	return offScreenSRV;
}




HRESULT ToneMapUtility::setupRenderTargets(){

	if (!device || !context)
		return E_FAIL;


	// Release as we might be resizing
	if (offScreenSRV)
		offScreenSRV->Release();
	if (offScreenRTV)
		offScreenRTV->Release();


	// Setup Render Targets

	HRESULT hr;

	// Setup viewport for offscreen rendering
	RTWidth = 512;
	RTHeight = 512;
	offScreenViewport.TopLeftX = 0;
	offScreenViewport.TopLeftY = 0;
	offScreenViewport.Width = RTWidth;
	offScreenViewport.Height = RTHeight;
	offScreenViewport.MinDepth = 0.0f;
	offScreenViewport.MaxDepth = 1.0f;

	// Setup texture description for offscreen rendering
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = RTWidth;
	texDesc.Height = RTHeight;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE ;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags =  D3D11_RESOURCE_MISC_GENERATE_MIPS;
	ID3D11Texture2D* tex = 0;

	// Setup shader resource view description for offscreen rendering.
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels =-1;
	viewDesc.Texture2D.MostDetailedMip = 0;
	// Setup render targetview  description for offscreen rendering.
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;


	hr = device->CreateTexture2D(&texDesc, 0, &tex);
	hr = device->CreateShaderResourceView(tex, &viewDesc, &offScreenSRV);
	hr = device->CreateRenderTargetView(tex, &rtvDesc, &offScreenRTV);
	tex->Release();
	hr = device->CreateTexture2D(&texDesc, 0, &tex);
	hr = device->CreateShaderResourceView(tex, &viewDesc, &offScreenSRV2);
	hr = device->CreateRenderTargetView(tex, &rtvDesc, &offScreenRTV2);
	tex->Release();
	// Setup the Depth Stencil buffer and Depth Stencil View (DSV)
	// Setup DepthStencilTexture description
	D3D11_TEXTURE2D_DESC		depthStencilDesc;
	depthStencilDesc.Width = RTWidth;
	depthStencilDesc.Height = RTHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1; // Multi-sample properties much match the above DXGI_SWAP_CHAIN_DESC structure
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// Create DepthStencilTexture
	if (SUCCEEDED(hr))
		hr = device->CreateTexture2D(&depthStencilDesc, 0, &tex);
	// Setup DepthStencilView description
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	// Create DepthStencilView
	if (SUCCEEDED(hr))
		hr = device->CreateDepthStencilView(tex, &descDSV, &offScreenDSV);
	// Cleanup--we only need the views.
	tex->Release();

}


ToneMapUtility::~ToneMapUtility()
{
}
