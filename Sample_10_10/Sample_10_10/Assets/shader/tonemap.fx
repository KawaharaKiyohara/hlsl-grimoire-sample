/*!
 * @brief	トーンマップ。
 */

// 頂点シェーダーへの入力構造体体。
struct VSInput{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};
// ピクセルシェーダーへの入力構造体。
struct PSInput{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

//トーンマップの共通定数バッファ。
cbuffer cbTonemapCommon : register(b1){
	float deltaTime;
	float middleGray;
    int currentAvgTexNo;
}

/*!
 * @brief	頂点シェーダー。
 */
PSInput VSMain(VSInput In) 
{
	PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}



////////////////////////////////////////////////////////
// 輝度の対数平均を求める。
////////////////////////////////////////////////////////

static const float3 LUMINANCE_VECTOR  = float3(0.2125f, 0.7154f, 0.0721f);
Texture2D<float4> sceneTexture : register(t0);	//シーンテクスチャ。
sampler Sampler : register(s0);

static const int    MAX_SAMPLES            = 16;    // Maximum texture grabs

/*!
 * @brief	定数バッファ。
 */
cbuffer cbCalcLuminanceAvg : register(b0){
	float4 g_avSampleOffsets[MAX_SAMPLES];
};

////////////////////////////////////////////////////////
// ここから輝度の平均を求める処理。
////////////////////////////////////////////////////////

/*!
 * @brief 輝度の自然対数の平均を求めるピクセルシェーダー
 * @detail ネイピア数を底とする輝度の自然対数平均を求めます。</br>
 *         C++側のenCalcAvgStep_0の処理に該当します。
 */
float4 PSCalcLuminanceLogAvarage(PSInput In) : SV_Target0
{
    float3 LUMINANCE_VECTOR  = float3(0.2125f, 0.7154f, 0.0721f);   // 輝度抽出用のベクトル。
    float  fLogLumSum = 0.0f;                                       // 輝度の自然対数の合計を記憶する変数。
    
    // 9テクセルサンプリングする。
    for(int i = 0; i < 9; i++)
    {
        float3 color = max( sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[i].xy), 0.001f );
        float v = dot( LUMINANCE_VECTOR, color.xyz );
        // ネイピア数を底とする輝度の自然対数を加算する。
        fLogLumSum += log(v);
    }
    // 9で除算して平均を求める。
    fLogLumSum /= 9;

    return float4(fLogLumSum, fLogLumSum, fLogLumSum, 1.0f);
}
/*!
 * @brief 16テクセルの平均輝度計算を求めるピクセルシェーダー
 * @detail 処理するテクセルの近傍16テクセルをサンプリングして、</br>
 *         平均値を出力します。</br>
 *         C++側のenCalcAvgStep_0 ～ enCalcAvgStep_4に該当します。
 */
float4 PSCalcLuminanceAvarage(PSInput In) : SV_Target0
{
	float fResampleSum = 0.0f; 
    
    // 16テクセルサンプリングして平均を求める。
    for(int iSample = 0; iSample < 16; iSample++)
    {
        fResampleSum += sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[iSample].xy);
    }
    
    fResampleSum /= 16;

    return float4(fResampleSum, fResampleSum, fResampleSum, 1.0f);
}

/*!
 *@brief	輝度の自然対数から輝度に復元するピクセルシェーダーのエントリーポイント
 *@detail   平均輝度計算の最後の処理です。近傍16テクセルをサンプリングして、</br>
 *          平均値を計算したあとで、exp()関数を利用して自然対数から輝度に復元します。</br>
 *          C++側のenCalcAvgStep_5に該当します。
 */
float4 PSCalcLuminanceExpAvarage( PSInput In ) : SV_Target0
{
	float fResampleSum = 0.0f;
    
    for(int iSample = 0; iSample < 16; iSample++)
    {
        fResampleSum += sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[iSample]);
    }
    
    // exp()を利用して復元する。
    fResampleSum = exp(fResampleSum/16);
    
    return float4(fResampleSum, fResampleSum, fResampleSum, 1.0f);
}

////////////////////////////////////////////////////////
// 輝度の平均を求める処理終わり。
////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// ここから明暗順応
/////////////////////////////////////////////////////////

Texture2D<float4> lumAvgTexture : register(t0);		        //平均輝度
Texture2D<float4> lastLumAvgTextureArray[2] : register(t1);	//１フレーム前の平均輝度の配列

/*!
 *@brief	明暗順応のための平均輝度の適合させるピクセルシェーダー。
 */
float4 PSCalcAdaptedLuminance( PSInput In ) : SV_Target0
{
	float fAdaptedLum;
    
    if( currentAvgTexNo == 0){
        fAdaptedLum = lastLumAvgTextureArray[1].Sample(Sampler, float2( 0.5f, 0.5f));
    }else{
        fAdaptedLum = lastLumAvgTextureArray[0].Sample(Sampler, float2( 0.5f, 0.5f));
    } 
    float fCurrentLum = lumAvgTexture.Sample(Sampler, float2(0.5f, 0.5f));
    
    float fNewAdaptation = fAdaptedLum + (fCurrentLum - fAdaptedLum) * ( 1 - pow( 0.98f, 60 * deltaTime ) );
    return float4(fNewAdaptation, fNewAdaptation, fNewAdaptation, 1.0f);
}



/*!
 *@brief	平均輝度からトーンマップを行うピクセルシェーダー。
 */
float4 PSFinal( PSInput In) : SV_Target0
{
	// シーンのカラーをサンプリングする。
	float4 sceneColor = sceneTexture.Sample(Sampler, In.uv );

	float avgLum = 0.0f;
    if( currentAvgTexNo == 0){
        avgLum = lastLumAvgTextureArray[0].Sample(Sampler, float2( 0.5f, 0.5f)).r;
    }else{
        avgLum = lastLumAvgTextureArray[1].Sample(Sampler, float2( 0.5f, 0.5f)).r;
    }
    // 露光値を計算する。
    // 平均輝度を0.18にするためのスケール値を求める。
    float k = ( 0.18f / ( max(avgLum, 0.001f )));
    // スケールをこのピクセルの輝度に掛け算する。
    sceneColor.xyz *= k;
    
    // Reinhard関数。今回のreinhardは、輝度2.0以上はあえて白飛びするようにしている。
    float luminanceLimit = 2.0f;
    sceneColor.xyz = ( sceneColor.xyz / (sceneColor.xyz + 1.0f) ) * (1 + sceneColor.xyz / ( luminanceLimit * luminanceLimit ));

    // ガンマ補正
    sceneColor.xyz = pow( max( sceneColor.xyz, 0.0001f), 1.0f / 2.2f );
    
	return sceneColor;
}