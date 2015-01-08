/******************************
* PS_DirLight.hlsl
* Written by <Your name here>
******************************/

#include "LightHelper.hlsli"
 
// matches ShaderManager::cbMaterial
cbuffer cbMaterial : register(b0)
{
	Material gMaterial;		// the material
		bool gUseTexture;	// whether we use a texture or just colour
	bool pad1,pad2,pad3;	// padding (ignore)
}; 
// matches ShaderMananger::cbLights
// Note: it MUST be register(b1), so its at a different index to the above cbMaterial
cbuffer cbLights : register(b1)
{
	PointLight gPointLights[3];
	DirectionalLight gDirLights[3];
	float3 gEyePosW;
	float pad0;	// needs one extra float to keep it to 4x floats
};

Texture2D gDiffuseTexture: register(t0);
SamplerState gSamplerState;

float4 main(VertexOut pin) : SV_Target
{
	// the normal 
	float3 normal=normalize(pin.NormalW);
	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	float3 position = pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye); 
	// Normalize.
	toEye /= distToEye;

	//
	// Lighting.
	//

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.  
	[unroll]
	for(int i = 0; i < 3; ++i)
	{
		float4 A, D, S;
		
		ComputeDirectionalLight(gMaterial,gDirLights[i],normal,toEye, 
			A, D, S);

		ambient += A;
		diffuse += D;
		spec    += S;
	}

	for(int i = 0; i < 3; ++i)
	{
		float4 A, D, S;
		
		ComputePointLight(gMaterial,gPointLights[i],position,normal,toEye, 
			A, D, S);

		ambient += A;
		diffuse += D;
		spec    += S;
	}

	float4 litColor = clamp(ambient + diffuse + spec,0,1);

	// take alpha from diffuse material (just in case of transparency)
	litColor.a = gMaterial.Diffuse.a;

		
	if (gUseTexture)
		litColor *= gDiffuseTexture.Sample( gSamplerState, pin.Tex );

	return litColor;
}
