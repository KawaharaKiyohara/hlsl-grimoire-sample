#include "stdafx.h"
#include "Tonemap.h"

namespace {
	/// <summary>
	/// 近傍16ピクセルをサンプリングするためのUVオフセットを計算する。
	/// </summary>
	/// <param name="dwWidth">元のテクスチャの幅</param>
	/// <param name="dwHeight">元のテクスチャの高さ</param>
	/// <param name="avSampleOffsets">UVオフセットの記憶先。</param>
	/// <returns></returns>
	void GetSampleOffsets4x4(DWORD dwWidth, DWORD dwHeight, Vector4 avSampleOffsets[])
	{
		// 1テクセルオフセット
		float tU = 1.0f / dwWidth;
		float tV = 1.0f / dwHeight;

		int index = 0;
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				avSampleOffsets[index].x = (x - 1.5f) * tU;
				avSampleOffsets[index].y = (y - 1.5f) * tV;

				index++;
			}
		}
	}
	/// <summary>
	/// 近接9ピクセルをサンプリングするためのUVオフセットを計算する。
	/// </summary>
	/// <param name="dwWidth"></param>
	/// <param name="dwHeight"></param>
	/// <param name="avSampleOffsets"></param>
	void GetSampleOffset3x3(DWORD dwWidth, DWORD dwHeight, Vector4 avSampleOffsets[])
	{
		// 1テクセルオフセット
		float tU = 1.0f / dwWidth;
		float tV = 1.0f / dwHeight;

		int index = 0;
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {
				avSampleOffsets[index].x = (x - 1.0f) * tU;
				avSampleOffsets[index].y = (y - 1.0f) * tV;

				index++;
			}
		}
	}
}


void Tonemap::InitRenderTargets(RenderTarget& mainRenderTarget)
{
	// 平均値を計算する処理のレンダリングターゲットのサイズ。
	static const int calcAVGRtSize[enNumCalcAvgStep] = {
		1024,		// enCalcAvg_0
		256,		// enCalcAvg_1
		64,			// enCalcAvg_2
		16,			// enCalcAvg_3
		4,			// enCalcAvg_4
		1			// enCalcAvg_5
	};
	// 平均輝度計算用のレンダリングターゲットを作成。
	for (int i = 0; i < enNumCalcAvgStep; i++) {
		m_calcAvgRt[i].Create(
			calcAVGRtSize[i],
			calcAVGRtSize[i],
			1,
			1,
			DXGI_FORMAT_R16_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
	}
	// 最終的にトーンマップで使用する平均輝度を書き込むレンダリングターゲットを作成
	for (auto& avgRt : m_avgRt) {
		avgRt.Create(
			1, 1, 1, 1, DXGI_FORMAT_R16_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
	}
	// トーンマップ用のスプライトを作成
	m_finalRt.Create(
		mainRenderTarget.GetWidth(),
		mainRenderTarget.GetHeight(),
		1,
		1,
		mainRenderTarget.GetColorBufferFormat(),
		DXGI_FORMAT_UNKNOWN
	);
}

void Tonemap::InitSprites(RenderTarget& mainRenderTarget)
{
	for (int procStep = 0; procStep < enNumCalcAvgStep; procStep++)
	{
		SpriteInitData initData;
		initData.m_width = m_calcAvgRt[procStep].GetWidth();
		initData.m_height = m_calcAvgRt[procStep].GetHeight();
		initData.m_colorBufferFormat[0] = m_calcAvgRt[procStep].GetColorBufferFormat();
		initData.m_expandConstantBuffer = m_sampleUVOffsetArray;
		initData.m_expandConstantBufferSize = sizeof(m_sampleUVOffsetArray);
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";

		if (procStep == enCalcAvgStep_0) {
			// 自然対数を底とする対数の平均をとるスプライトを初期化
			initData.m_psEntryPoinFunc = "PSCalcLuminanceLogAvarage";
			initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
			m_calcAvgSprites[procStep].Init(initData);
		}
		else if (procStep == enCalcAvgStep_5) {
			// exp関数を用いて自然対数を底とする対数から平均輝度に復元するためのスプライトを初期化。
			initData.m_psEntryPoinFunc = "PSCalcLuminanceExpAvarage";
			initData.m_textures[0] = &m_calcAvgRt[procStep - 1].GetRenderTargetTexture();
			m_calcAvgSprites[procStep].Init(initData);
		}
		else {
			// 平均をとるスプライトを初期化。
			initData.m_psEntryPoinFunc = "PSCalcLuminanceAvarage";
			initData.m_textures[0] = &m_calcAvgRt[procStep - 1].GetRenderTargetTexture();
			m_calcAvgSprites[procStep].Init(initData);
		}
	}
	// 明暗順応
	{
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = m_calcAvgRt[enCalcAvgStep_5].GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSCalcAdaptedLuminance";
		initData.m_expandConstantBuffer = &m_tonemapParam;
		initData.m_expandConstantBufferSize = sizeof(m_tonemapParam);
		initData.m_textures[0] = &m_calcAvgRt[enCalcAvgStep_5].GetRenderTargetTexture();
		initData.m_textures[1] = &m_avgRt[0].GetRenderTargetTexture();
		initData.m_textures[2] = &m_avgRt[1].GetRenderTargetTexture();

		m_calcAdapteredLuminanceSprite.Init(initData);
	}
	// 平均輝度を使ってトーンマップを行うためのスプライトを初期化。
	{
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSFinal";
		initData.m_expandConstantBuffer = &m_tonemapParam;
		initData.m_expandConstantBufferSize = sizeof(m_tonemapParam);
		initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
		initData.m_textures[1] = &m_avgRt[0].GetRenderTargetTexture();
		initData.m_textures[2] = &m_avgRt[1].GetRenderTargetTexture();

		m_finalSprite.Init(initData);
	}
	// トーンマップされた絵をメインレンダリングターゲットにコピーするためのスプライトを初期化。
	{
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/preset/sprite.fx";
		initData.m_textures[0] = &m_finalRt.GetRenderTargetTexture();
		m_copyMainRtSprite.Init(initData);
	}
}

void Tonemap::ExecuteCalcAvg(RenderContext& rc)
{
	for (int procStep = 0; procStep < enNumCalcAvgStep; procStep++) {
		// レンダリングターゲットとして利用できるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_calcAvgRt[procStep]);
		// レンダリングターゲットを設定
		rc.SetRenderTargetAndViewport(m_calcAvgRt[procStep]);
		rc.ClearRenderTargetView(m_calcAvgRt[procStep]);
		GetSampleOffsets4x4(
			m_calcAvgSprites[procStep].GetTextureWidth(0),
			m_calcAvgSprites[procStep].GetTextureHeight(0),
			m_sampleUVOffsetArray
		);
		m_calcAvgSprites[procStep].Draw(rc);

		// レンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(m_calcAvgRt[procStep]);
	}
}

void Tonemap::ExecuteTonemap(RenderContext& rc)
{
	// レンダリングターゲットを設定
	rc.WaitUntilToPossibleSetRenderTarget(m_finalRt);
	rc.SetRenderTargetAndViewport(m_finalRt);
	// 最終合成。
	m_finalSprite.Draw(rc);
	// レンダリングターゲットへの書き込み終了待ち
	rc.WaitUntilFinishDrawingToRenderTarget(m_finalRt);
}

void Tonemap::ExecuteCopyResultToMainRenderTarget(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// 最終合成された絵をメインレンダリングターゲットにコピーする。
	rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
	// レンダリングターゲットを設定
	rc.SetRenderTargetAndViewport(mainRenderTarget);
	m_copyMainRtSprite.Draw(rc);
	// レンダリングターゲットへの書き込み終了待ち
	rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
}
void Tonemap::ExecuteLuminanceAdapter(RenderContext& rc)
{
	m_tonemapParam.currentAvgTexNo = m_currentAvgRt;

	rc.WaitUntilToPossibleSetRenderTarget(m_avgRt[m_currentAvgRt]);
	// レンダリングターゲットを設定
	rc.SetRenderTargetAndViewport(m_avgRt[m_currentAvgRt]);
	m_calcAdapteredLuminanceSprite.Draw(rc);
	// レンダリングターゲットへの書き込み終了待ち
	rc.WaitUntilFinishDrawingToRenderTarget(m_avgRt[m_currentAvgRt]);
}

void Tonemap::Init(RenderTarget& mainRenderTarget)
{
	m_tonemapParam.midddleGray = 0.4f;
	m_tonemapParam.deltaTime = 1.0f / 60.0f;

	// 1. 各種レンダリングターゲットを初期化する。
	InitRenderTargets(mainRenderTarget);

	// 2. 各種スプライトを初期化する。
	InitSprites(mainRenderTarget);
}
void Tonemap::Execute(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// 1. シーンの平均輝度計算。
	ExecuteCalcAvg( rc);

	// 2. 明暗順応。
	ExecuteLuminanceAdapter(rc);

	// 3. シーンの平均輝度を使ってトーンマップ。
	ExecuteTonemap(rc);
	
	// 4. トーンマップした結果の画像をメインレンダリングターゲットにコピーする。
	ExecuteCopyResultToMainRenderTarget(rc, mainRenderTarget);

	// 5. 平均値を計算するレンダリングターゲットの番号を入れ替える。
	m_currentAvgRt = 1 ^ m_currentAvgRt;
}