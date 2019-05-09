#pragma once

class Model;
class Quad;
class Effect;
class Scene;
class ToneMapUtility
{

	int										RTWidth = 512;
	int										RTHeight = 512;
	D3D11_VIEWPORT							offScreenViewport;

	ID3D11ShaderResourceView				*offScreenSRV2 = nullptr;
	ID3D11RenderTargetView					*offScreenRTV2 = nullptr;
	ID3D11ShaderResourceView				*offScreenSRV = nullptr;
	ID3D11RenderTargetView					*offScreenRTV = nullptr;
	ID3D11DepthStencilView					*offScreenDSV = nullptr;
	ID3D11DeviceContext						*context = nullptr;
	ID3D11Device							*device = nullptr;
	Quad									*screenQuad = nullptr;
	ID3D11VertexShader						*screenQuadVS = nullptr;
	ID3D11PixelShader						*textureCopyPS = nullptr;
	Effect									*defaultEffect = nullptr;

public:
	ToneMapUtility(ID3D11Device *deviceIn, ID3D11DeviceContext *contextIn);
	HRESULT setupRenderTargets();
	ID3D11ShaderResourceView* computeMeanIntensity(Scene*scene);
	~ToneMapUtility();
};

