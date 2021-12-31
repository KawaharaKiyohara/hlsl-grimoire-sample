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


Texture2D<float4> sceneTexture : register(t0);	//シーンテクスチャ。
sampler Sampler : register(s0);

static const int    MAX_SAMPLES            = 16;    // Maximum texture grabs

/*!
 * @brief	定数バッファ。
 */
cbuffer cbCalcLuminanceAvg : register(b0){
	float4 g_avSampleOffsets[MAX_SAMPLES];
};


/*!
 *@brief ACESトーンマッパー
 */
float ACESFilm(float x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

////////////////////////////////////////////////////////
// RGB->HSV, HSV->RGBへの色空間変換関連の関数集。
////////////////////////////////////////////////////////

/*!
 * @brief RGB系からHSV系に変換する。
 */
float3 Rgb2Hsv(float3 c)
{
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
/*!
 * @brief RGB系からHSVのV(輝度)を求める。
 */
float Rgb2V( float3 rgb)
{
    return max(rgb.r, max(rgb.g, rgb.b));
}
/*!
 * @brief HSV系からRGB系に変換する。
 */
float3 Hsv2Rgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

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
    // step-10 輝度の対数平均を求める。
    float  fLogLumSum = 0.0f;  // 輝度の自然対数の合計を記憶する変数。
    
    // 9テクセルサンプリングする。
    for(int i = 0; i < 9; i++)
    {
        // シーンのカラーをサンプリング。
        float3 color = max( sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[i].xy), 0.001f );
        // RGBか輝度を求める。
        float v = Rgb2V(color);
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
    // step-11 輝度の対数平均の平均を求める。
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
    // step-12 対数平均の計算と復元。
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

Texture2D<float4> lumAvgTexture : register(t1);		        //平均輝度が記憶されているテクスチャ。

/*!
 *@brief	平均輝度からトーンマップを行うピクセルシェーダー。
 */
float4 PSFinal( PSInput In) : SV_Target0
{
    // step-13 シーンのカラーから輝度を計算する。
    // シーンのカラーをサンプリングする。
	float4 sceneColor = sceneTexture.Sample(Sampler, In.uv );
    // シーンのカラーをRGB系からHSV系に変換する。
    float3 hsv = Rgb2Hsv(sceneColor);

    // step-14 平均輝度を使って、このピクセルの輝度をスケールダウンする。
    // 平均輝度をサンプリングする。
	float avgLum = lumAvgTexture.Sample(Sampler, float2( 0.5f, 0.5f)).r;
    // 平均輝度を0.18にするためのスケール値を求める。
    float k = ( 0.18f / ( max(avgLum, 0.001f )));
    // スケール値を使って、輝度をスケールダウン。
    hsv.z *= k;

    // step-15 輝度の変化を線形から非線形にする。
    // ACESトーンマッパーを使って輝度の変化を非線形にする。
    hsv.z = ACESFilm(hsv.z);

    // step-16 RGBに戻す。
    // HSV系からRGB系に戻す。
    sceneColor.xyz = Hsv2Rgb(hsv);
    
    // step-17 ガンマ補正。
    sceneColor.xyz = pow( max( sceneColor.xyz, 0.0001f), 1.0f / 2.2f );
    
	return sceneColor;
}