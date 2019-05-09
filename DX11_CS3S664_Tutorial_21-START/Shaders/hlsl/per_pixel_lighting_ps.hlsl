
//
// Model a simple light
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


//-----------------------------------------------------------------
// Structures and resources
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------
struct Light{
	float4				Vec; // w=1: Vec represents position, w=0: Vec  represents direction.
	float4				Ambient;
	float4				Diffuse;
	float4				Specular;
	float4				Attenuation;// x=constant,y=linear,z=quadratic,w=cutOff
};
cbuffer modelCBuffer : register(b0) {

	float4x4			worldMatrix;
	float4x4			worldITMatrix; // Correctly transform normals to world space
};
cbuffer cameraCbuffer : register(b1) {
	float4x4			viewMatrix;
	float4x4			projMatrix;
	float4				eyePos;
}
cbuffer lightCBuffer : register(b2) {
	Light light[2];
};


//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------

// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {

	// Vertex in world coords
	float3				posW			: POSITION;
	// Normal in world coords
	float3				normalW			: NORMAL;
	float4				matDiffuse		: DIFFUSE; // a represents alpha.
	float4				matSpecular		: SPECULAR; // a represents specular power. 
	float2				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};


//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
Texture2D diffuseTexture : register(t0);
Texture2D alphaTexture : register(t1);
Texture2D toneMap : register(t7);
SamplerState linearSampler : register(s0);
SamplerState anisoSampler : register(s1);

//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket v) { 

	FragmentOutputPacket outputFragment;
	bool useTexture = true;
	bool useAlphaTexture = true;

	float3 N = normalize(v.normalW);
		float4 baseColour = v.matDiffuse;
	if (useTexture)
		baseColour = baseColour * diffuseTexture.Sample(anisoSampler, v.texCoord);
	
	float alpha = 1;
	if (useAlphaTexture)
		alpha = alphaTexture.Sample(anisoSampler, v.texCoord).r;

	if (alpha<0.5)clip(-1);
		
	float3 colour = 0;
	for (int i = 0; i < 1; i++)
	{
		
		
		float att=1.0;
		//Initialise returned colour to ambient component
		colour+= baseColour.xyz* light[i].Ambient;
		// Calculate the lambertian term (essentially the brightness of the surface point based on the dot product of the normal vector with the vector pointing from v to the light source's location)
		float3 lightDir = -light[i].Vec.xyz; // Directional light
		if (light[i].Vec.w == 1.0)// Positional light
		{

			lightDir = light[i].Vec.xyz - v.posW; 
			float dist = length(lightDir);
			float cutOff = light[i].Attenuation.w;
			if (dist > cutOff)
				continue;
			else
			{
				float con = light[i].Attenuation.x; float lin = light[i].Attenuation.y; float quad = light[i].Attenuation.z; 
				att = 1.0 / (con + lin*dist + quad*dist*dist);
			}
				
		}
		lightDir = normalize(lightDir);
		// Add diffuse light if relevant (otherwise we end up just returning the ambient light colour)
		// Add Code Here (Add diffuse light calculation)
		colour += att*(max(dot(lightDir, N), 0.0f) *baseColour.xyz * light[i].Diffuse);

		// Calc specular light
		float specPower = max(v.matSpecular.a*1000.0, 1.0f);

		float3 eyeDir = normalize(eyePos - v.posW);
		float3 R = reflect(-lightDir, N);

		// Add Code Here (Specular Factor calculation)	
		float specFactor = pow(max(dot(R, eyeDir), 0.0f), specPower);
		colour += att*specFactor * v.matSpecular.xyz * light[i].Specular;
		
	}


	float targetIntensity = 0.5;
	float3 tempColour = toneMap.SampleLevel(linearSampler, float2(0.5, 0.5), 9.0).xyz;
	float meanIntensity = (tempColour.r + tempColour.g + tempColour.b) / 3.0;
	float3 scaledColour = colour * (targetIntensity / (meanIntensity));

	outputFragment.fragmentColour = float4(scaledColour, 1.0);//
	return outputFragment;

}
