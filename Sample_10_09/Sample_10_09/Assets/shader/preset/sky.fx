///////////////////////////////////////
// 空用シェーダー。
///////////////////////////////////////

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;      // モデルの頂点座標
    float3 normal : NORMAL;     // 法線
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;         // 法線
};

TextureCube<float4> g_skyCubeMap : register(t0);      // スカイキューブマップ

sampler g_sampler : register(s0);

// 頂点シェーダー。
SPSIn VSMain( SVSIn vsIn )
{
	SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
	psIn.normal = normalize(mul(mWorld, vsIn.normal));
	return psIn;
}

// ピクセルシェーダー
float4 PSMain(SPSIn psIn) : SV_Target0
{
	float4 albedoColor;
	float3 normal = normalize(psIn.normal);
	albedoColor = g_skyCubeMap.Sample(g_sampler, normal * -1.0f);
	albedoColor.xyz *= 1.2f;
	return albedoColor ;
}