
#include "stdafx.h"
#include <BaseModel.h>

BaseModel::BaseModel(ID3D11Device *device, Effect *_effect, Material *_materials[], int _numMaterials, ID3D11ShaderResourceView **_textures, int _numTextures) {

	if (_textures != nullptr && _numTextures == 0)_numTextures = 1;
	if (_materials != nullptr && _numMaterials == 0)_numMaterials = 1;
	
	if (_numTextures != 0) setTextures(_textures, _numTextures);
	if (_numMaterials != 0) setMaterials(_materials, _numMaterials);
	effect = _effect;
	inputLayout = effect->getVSInputLayout();
	createDefaultLinearSampler(device);
}

BaseModel::BaseModel(ID3D11Device *device, ID3D11InputLayout *_inputLayout, Material *_materials[], int _numMaterials, ID3D11ShaderResourceView **_textures, int _numTextures) {
	
	if (_textures != nullptr && _numTextures == 0)_numTextures = 1;
	if (_materials != nullptr && _numMaterials == 0)_numMaterials = 1;

	if (_numTextures != 0) setTextures(_textures, _numTextures);
	if (_numMaterials != 0) setMaterials(_materials, _numMaterials);
	inputLayout = _inputLayout;
	createDefaultLinearSampler(device);
}

void BaseModel::createDefaultLinearSampler(ID3D11Device *device){
	
	// If textures are used a sampler is required for the pixel shader to sample the texture
	D3D11_SAMPLER_DESC linearDesc;

	ZeroMemory(&linearDesc, sizeof(D3D11_SAMPLER_DESC));
	linearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	linearDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	linearDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	linearDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	linearDesc.MinLOD = 0.0f;
	linearDesc.MaxLOD = 0.0f;
	linearDesc.MipLODBias = 0.0f;
	linearDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

	device->CreateSamplerState(&linearDesc, &sampler);
}

void BaseModel::setTextures(ID3D11ShaderResourceView *_textures[], int _numTextures){
	numTextures = _numTextures;
	for (int i = 0; i < numTextures; i++)
		textures[i] = _textures[i];
};


void BaseModel::setMaterials(Material *_materials[], int _numMaterials) {
	numMaterials = _numMaterials;
	materials[0] = _materials[0];
	for (int i = 0; i < numMaterials; i++)
		materials[i] = _materials[i];
};


BaseModel::~BaseModel() {

	if (vertexBuffer)
		vertexBuffer->Release();

	if (indexBuffer)
		indexBuffer->Release();

	if (inputLayout)
		inputLayout->Release();
}
