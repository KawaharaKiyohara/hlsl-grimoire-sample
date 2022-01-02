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
	
	// step-3 トーンマップを行うためのレンダリングターゲットを作成。
	
}
void Tonemap::InitSprites(RenderTarget& mainRenderTarget)
{
	// step-4 平均輝度計算のためのスプライトを初期化。
	
	{
		// step-5 トーンマップを行うためのスプライトを初期化。
	
	}
	{
		// step-6 トーンマップされた絵をメインレンダリングターゲットにコピーするためのスプライトを初期化。
	
	}
}

void Tonemap::ExecuteCalcAvg(RenderContext& rc)
{
	// step-7 シーンの輝度の平均を計算する処理を実行。
	
}
void Tonemap::ExecuteTonemap(RenderContext& rc)
{
	// step-8 トーンマップを実行。
	
}
void Tonemap::ExecuteCopyResultToMainRenderTarget(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// step-9 トーンマップされた絵をメインレンダリングターゲットにコピー。

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