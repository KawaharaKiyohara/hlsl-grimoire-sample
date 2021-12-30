/*!
 * @brief ディズニーベースの物理ベースシェーダ
 */

///////////////////////////////////////////////////
// 定数
///////////////////////////////////////////////////
static const int NUM_DIRECTIONAL_LIGHT = 4; // ディレクションライトの本数
static const float PI = 3.1415926f;         // π

///////////////////////////////////////////////////
// 構造体
///////////////////////////////////////////////////

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// ディレクションライト
struct DirectionalLight
{
    float3 direction;   // ライトの方向
    float4 color;       // ライトの色
};


// ライト用の定数バッファー
cbuffer LightCb : register(b1)
{
    DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT];
    float3 eyePos;          // カメラの視点
    float specPow;          // スペキュラの絞り
    float3 ambientLight;    // 環境光
    Matrix mLvp;            // ライトビュープロジェクション行列
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;      // モデルの頂点座標
    float3 normal : NORMAL;     // 法線
    float3 tangent  : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv : TEXCOORD0;      // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;         // 法線
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv : TEXCOORD0;          // uv座標
    float3 worldPos : TEXCOORD1;    // ワールド空間でのピクセルの座標
    float4 posInLvp : TEXCOORD2;    // ライトビュープロジェクション空間での座標。
};

///////////////////////////////////////////////////
// グローバル変数
///////////////////////////////////////////////////

// step-1 アルベドマップ、法線マップ、スペキュラマップにアクセスするための変数を追加
Texture2D<float4> g_albedo : register(t0);      // アルベドマップ
Texture2D<float4> g_normalMap : register(t1);   // 法線マップ
Texture2D<float4> g_specularMap : register(t2); // スペキュラマップ。rgbにスペキュラカラー、aに金属度
Texture2D<float4> g_shadowMap : register(t10);  // シャドウマップ

// サンプラーステート
sampler g_sampler : register(s0);

///////////////////////////////////////////////////
// 関数
///////////////////////////////////////////////////

float3 GetNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
    float3 binSpaceNormal = g_normalMap.SampleLevel (g_sampler, uv, 0.0f).xyz;
    binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;

    float3 newNormal = tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;

    return newNormal;
}


/// <summary>
/// 頂点シェーダー
/// <summary>
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn = (SPSIn)0;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.posInLvp = mul(mLvp, float4( psIn.worldPos, 1.0f));

    psIn.normal = normalize(mul(mWorld, vsIn.normal));
    psIn.tangent = normalize(mul(mWorld, vsIn.tangent));
    psIn.biNormal = normalize(mul(mWorld, vsIn.biNormal));
    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// ピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{   
    // 法線を計算
    float3 normal = GetNormal(psIn.normal, psIn.tangent, psIn.biNormal, psIn.uv);
    // アルベドカラー（拡散反射光）
    float4 albedoColor = g_albedo.Sample(g_sampler, psIn.uv); 

    float2 shadowMapUV = psIn.posInLvp.xy / psIn.posInLvp.w;
    shadowMapUV = shadowMapUV * float2( 0.5f, -0.5f ) + 0.5f ;

    int startLigNo = 0;
    float z = psIn.posInLvp.z / psIn.posInLvp.w;
    if( 
        shadowMapUV.x >= 0.0f 
        && shadowMapUV.x <= 1.0f 
        && shadowMapUV.y >= 0.0f 
        && shadowMapUV.y <= 1.0f
        && z > 0.0f 
        && z < 1.0f
    ){
        float zInShadowMap = g_shadowMap.Sample(g_sampler, shadowMapUV);
        if( zInShadowMap < z - 0.001f){
            // 影になっている
            startLigNo = 1;
        }
    }
    
    float3 lig = 0;
    for(int ligNo = startLigNo; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    {
        
        // 正規化Lambert拡散反射を求める
        float NdotL = saturate(dot(normal, -directionalLight[ligNo].direction));
        float3 lambertDiffuse = directionalLight[ligNo].color * NdotL;
        // 最終的な拡散反射光を計算する
        lig += albedoColor * lambertDiffuse;

    }

    // 環境光による底上げ
    lig += ambientLight * albedoColor;

    float4 finalColor = 1.0f;
    finalColor.xyz = lig;
    return finalColor;
}
