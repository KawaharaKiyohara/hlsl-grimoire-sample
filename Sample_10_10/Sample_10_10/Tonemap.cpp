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

void Tonemap::Init(RenderTarget& mainRenderTarget)
{
	m_tonemapParam.midddleGray = 0.4f;
	m_tonemapParam.deltaTime = 1.0f / 60.0f;

	// 平均輝度計算用のレンダリングターゲットを作成。
	for (int i = 0; i < enNumCalcAvgSprite; i++) {
		int rtSize = 1 << (2 * (enNumCalcAvgSprite - i - 1));
		m_calcAvgRt[i].Create(
			rtSize,
			rtSize,
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
	// 最終合成用のスプライトを作成
	m_finalRt.Create(
		mainRenderTarget.GetWidth(),
		mainRenderTarget.GetHeight(),
		1,
		1,
		mainRenderTarget.GetColorBufferFormat(),
		DXGI_FORMAT_UNKNOWN
	);
	// 自然対数を底とする対数の平均をとるスプライトを初期化
	int curRtNo = 0;
	{
		SpriteInitData initData;
		initData.m_width = m_calcAvgRt[curRtNo].GetWidth();
		initData.m_height = m_calcAvgRt[curRtNo].GetHeight();
		initData.m_colorBufferFormat[0] = m_calcAvgRt[curRtNo].GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSCalcLuminanceLogAvarage";
		initData.m_expandConstantBuffer = m_sampleUVOffsetArray;
		initData.m_expandConstantBufferSize = sizeof(m_sampleUVOffsetArray);
		initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
		m_calcAvgSprites[enCalcAvgLog].Init(initData);
	}

	// 平均をとるスプライトを初期化。
	curRtNo++;
	int calsAvgSpriteNo = enCalcAvg_Start;
	while (curRtNo < enCalcAvg_End) {
		SpriteInitData initData;
		initData.m_width = m_calcAvgRt[curRtNo].GetWidth();
		initData.m_height = m_calcAvgRt[curRtNo].GetHeight();
		initData.m_colorBufferFormat[0] = m_calcAvgRt[curRtNo].GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSCalcLuminanceAvarage";
		initData.m_expandConstantBuffer = m_sampleUVOffsetArray;
		initData.m_expandConstantBufferSize = sizeof(m_sampleUVOffsetArray);
		initData.m_textures[0] = &m_calcAvgRt[curRtNo - 1].GetRenderTargetTexture();
		m_calcAvgSprites[calsAvgSpriteNo].Init(initData);
		calsAvgSpriteNo++;
		curRtNo++;
	}
	// exp関数を用いて自然対数を底とする対数から平均輝度に復元するためのスプライトを初期化。
	{
		SpriteInitData initData;
		initData.m_width = m_calcAvgRt[curRtNo].GetWidth();
		initData.m_height = m_calcAvgRt[curRtNo].GetHeight();
		initData.m_colorBufferFormat[0] = m_calcAvgRt[curRtNo].GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSCalcLuminanceExpAvarage";
		initData.m_expandConstantBuffer = m_sampleUVOffsetArray;
		initData.m_expandConstantBufferSize = sizeof(m_sampleUVOffsetArray);
		initData.m_textures[0] = &m_calcAvgRt[curRtNo - 1].GetRenderTargetTexture();
		m_calcAvgSprites[curRtNo].Init(initData);
	}
	// 明暗順応
	{
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = m_calcAvgRt[enCalcAvgExp].GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSCalcAdaptedLuminance";
		initData.m_expandConstantBuffer = &m_tonemapParam;
		initData.m_expandConstantBufferSize = sizeof(m_tonemapParam);
		initData.m_textures[0] = &m_calcAvgRt[enCalcAvgExp].GetRenderTargetTexture();
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
void Tonemap::CalcLuminanceAvarage(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	for (int spriteNo = 0; spriteNo < enNumCalcAvgSprite; spriteNo++) {
		// レンダリングターゲットとして利用できるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_calcAvgRt[spriteNo]);
		// レンダリングターゲットを設定
		rc.SetRenderTargetAndViewport(m_calcAvgRt[spriteNo]);
		rc.ClearRenderTargetView(m_calcAvgRt[spriteNo]);
		GetSampleOffsets4x4(
			m_calcAvgSprites[spriteNo].GetTextureWidth(0),
			m_calcAvgSprites[spriteNo].GetTextureHeight(0),
			m_sampleUVOffsetArray
		);
		m_calcAvgSprites[spriteNo].Draw(rc);

		// レンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(m_calcAvgRt[spriteNo]);
	}
}
void Tonemap::Execute(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// シーンの輝度の平均を計算していく。
	CalcLuminanceAvarage( rc, mainRenderTarget);

	// 明暗順応。
	
	m_tonemapParam.currentAvgTexNo = m_currentAvgRt;

	rc.WaitUntilToPossibleSetRenderTarget(m_avgRt[m_currentAvgRt]);
	// レンダリングターゲットを設定
	rc.SetRenderTargetAndViewport(m_avgRt[m_currentAvgRt]);
	m_calcAdapteredLuminanceSprite.Draw(rc);


	// レンダリングターゲットへの書き込み終了待ち
	rc.WaitUntilFinishDrawingToRenderTarget(m_avgRt[m_currentAvgRt]);
	// 求めた平均輝度を使ってトーンマップを行う。
	// レンダリングターゲットを設定
	rc.SetRenderTargetAndViewport(m_finalRt);
	// 最終合成。
	m_finalSprite.Draw(rc);
	// レンダリングターゲットへの書き込み終了待ち
	rc.WaitUntilFinishDrawingToRenderTarget(m_finalRt);
	
	// 最終合成された絵をメインレンダリングターゲットにコピーする。
	rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
	// レンダリングターゲットを設定
	rc.SetRenderTargetAndViewport(mainRenderTarget);
	m_copyMainRtSprite.Draw(rc);
	// レンダリングターゲットへの書き込み終了待ち
	rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);


	m_currentAvgRt = 1 ^ m_currentAvgRt;
}