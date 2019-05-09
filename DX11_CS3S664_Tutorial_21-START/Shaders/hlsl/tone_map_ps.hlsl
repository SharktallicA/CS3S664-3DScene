
// Basic texture copy pixel shader


// input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct fragmentInputPacket {

	float2				texCoord	: TEXCOORD;
	float4				posH		: SV_POSITION;
};


struct fragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};

//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
Texture2D copyTexture : register(t0);
Texture2D toneTexture : register(t1);
SamplerState linearSampler : register(s0);

fragmentOutputPacket main(fragmentInputPacket inputFragment) {

	fragmentOutputPacket outputFragment;

	float4 colour = copyTexture.Sample(linearSampler, inputFragment.texCoord);
	float4 tone = toneTexture.Sample(linearSampler, float2(0.5, 0.5));
	float meanIntensity = (tone.r + tone.g + tone.b) / 3.0;
	outputFragment.fragmentColour = float4(colour.rg, meanIntensity , 1.0);
	return outputFragment;
}
