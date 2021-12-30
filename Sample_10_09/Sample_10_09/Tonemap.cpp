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
}

void Tonemap::Init(RenderTarget& mainRenderTarget)
{
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
	m_avgRt.Create(
		1,1,1,1, DXGI_FORMAT_R16_FLOAT,
		DXGI_FORMAT_UNKNOWN
	);
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
	// 平均輝度を使ってトーンマップを行うためのスプライトを初期化。
	{
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSFinal";
		initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
		initData.m_textures[1] = &m_calcAvgRt[enCalcAvgExp].GetRenderTargetTexture();
		
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

void Tonemap::Execute(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// シーンの輝度の平均を計算していく。
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

}