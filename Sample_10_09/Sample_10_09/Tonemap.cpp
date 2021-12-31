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
	// step-2 平均輝度計算用のレンダリングターゲットを作成。
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
	// step-3 トーンマップを行うためのレンダリングターゲットを作成。
	// 最終合成用のスプライトを作成
	m_tonemapRt.Create(
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
	// step-4 平均輝度計算のためのスプライトを初期化。
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
	{
		// step-5 トーンマップを行うためのスプライトを初期化。
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSFinal";
		initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
		initData.m_textures[1] = &m_calcAvgRt[enCalcAvgStep_5].GetRenderTargetTexture();

		m_tonemapSprite.Init(initData);
	}
	{
		// step-6 トーンマップされた絵をメインレンダリングターゲットにコピーするためのスプライトを初期化。
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/preset/sprite.fx";
		initData.m_textures[0] = &m_tonemapRt.GetRenderTargetTexture();
		m_copyMainRtSprite.Init(initData);
	}
}

void Tonemap::ExecuteCalcAvg(RenderContext& rc)
{
	// step-7 シーンの輝度の平均を計算する処理を実行。
	for (int procStep = 0; procStep < enNumCalcAvgStep; procStep++) {
		// レンダリングターゲットとして利用できるまで待つ
		rc.WaitUntilToPossibleSetRenderTarget(m_calcAvgRt[procStep]);
		// レンダリングターゲットを設定
		rc.SetRenderTargetAndViewport(m_calcAvgRt[procStep]);
		rc.ClearRenderTargetView(m_calcAvgRt[procStep]);
		if (procStep == enCalcAvgStep_0) {
			// 対数平均は9x9テクセルをサンプリングするので、UVオフセットの計算の仕方を変更する。
			GetSampleOffset3x3(
				m_calcAvgSprites[procStep].GetTextureWidth(0),
				m_calcAvgSprites[procStep].GetTextureHeight(0),
				m_sampleUVOffsetArray
			);
		}
		else {
			// それ以外は4x4テクセルをサンプリングする。
			GetSampleOffsets4x4(
				m_calcAvgSprites[procStep].GetTextureWidth(0),
				m_calcAvgSprites[procStep].GetTextureHeight(0),
				m_sampleUVOffsetArray
			);
		}
		m_calcAvgSprites[procStep].Draw(rc);

		// レンダリングターゲットへの書き込み終了待ち
		rc.WaitUntilFinishDrawingToRenderTarget(m_calcAvgRt[procStep]);
	}
}
void Tonemap::ExecuteTonemap(RenderContext& rc)
{
	// step-8 トーンマップを実行。
	// レンダリングターゲットを設定
	rc.WaitUntilToPossibleSetRenderTarget(m_tonemapRt);
	rc.SetRenderTargetAndViewport(m_tonemapRt);
	// トーンマップを実行
	m_tonemapSprite.Draw(rc);
	// レンダリングターゲットへの書き込み終了待ち
	rc.WaitUntilFinishDrawingToRenderTarget(m_tonemapRt);
}
void Tonemap::ExecuteCopyResultToMainRenderTarget(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// step-9 トーンマップされた絵をメインレンダリングターゲットにコピー。
	// 最終合成された絵をメインレンダリングターゲットにコピーする。
	rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
	// レンダリングターゲットを設定
	rc.SetRenderTargetAndViewport(mainRenderTarget);
	m_copyMainRtSprite.Draw(rc);
	// レンダリングターゲットへの書き込み終了待ち
	rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
}
void Tonemap::Init(RenderTarget& mainRenderTarget)
{
	// 1. 各種レンダリングターゲットを初期化。
	InitRenderTargets(mainRenderTarget);

	// 2. 各種スプライトを初期化。
	InitSprites(mainRenderTarget);
}
void Tonemap::Execute(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// 1. シーンの平均輝度計算。
	ExecuteCalcAvg(rc);
	
	// 2. シーンの平均輝度を使ってトーンマップ。
	ExecuteTonemap(rc);

	// 3. トーンマップした結果の画像をメインレンダリングターゲットにコピーする。
	ExecuteCopyResultToMainRenderTarget(rc, mainRenderTarget);
	
}