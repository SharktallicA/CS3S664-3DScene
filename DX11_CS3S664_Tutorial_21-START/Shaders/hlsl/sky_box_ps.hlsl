
//
// Model a simple light
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


//-----------------------------------------------------------------
// Structures and resources
//-----------------------------------------------------------------


//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
TextureCube cubeMap : register(t0);
Texture2D toneMap : register(t7);
SamplerState linearSampler : register(s0);


//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------

// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {


	float3				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};


//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket v) { 

	FragmentOutputPacket outputFragment;
	float3 colour = cubeMap.Sample(linearSampler, v.texCoord).xyz;
	float3 tempColour = toneMap.SampleLevel(linearSampler, float2(0.5,0.5), 9.0).xyz;
	float meanIntensity = (tempColour.r + tempColour.g + tempColour.b) / 3.0;
	if (meanIntensity >= 9.5)

	{
		colour *= 0.9;
			//float3 fullColour = colour;
			//	float3 scaledColour = colour * (0.5 / (meanIntensity));
			//	colour = lerp(scaledColour, fullColour, 0.5);
	}
	else
		colour *= 1.1;
		
		outputFragment.fragmentColour = float4(colour, 1.0);//


	return outputFragment;

}
