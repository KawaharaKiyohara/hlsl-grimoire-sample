#pragma once
/// <summary>
/// トーンマップクラス
/// </summary>
class Tonemap
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Init(RenderTarget& mainRenderTarget);
	/// <summary>
	/// トーンマップを実行。
	/// </summary>
	void Execute(RenderContext& rc, RenderTarget& mainRenderTarget);
private:
	/// <summary>
	/// 平均輝度を計算する。
	/// </summary>
	void ExecuteCalcAvg(RenderContext& rc);
	/// <summary>
	/// 平均輝度を使ってトーンマップを実行する。
	/// </summary>
	void ExecuteTonemap(RenderContext& rc);
	/// <summary>
	/// トーンマップした結果をメインレンダリングターゲットにコピーする。
	/// </summary>
	/// <param name="rc"></param>
	/// <param name="mainRenderTarget"></param>
	void ExecuteCopyResultToMainRenderTarget(RenderContext& rc, RenderTarget& mainRenderTarget);
	/// <summary>
	/// 各種レンダリングターゲットを初期化。
	/// </summary>
	void InitRenderTargets( RenderTarget& mainRenderTarget );
	/// <summary>
	/// 各種スプライトを初期化。
	/// </summary>
	/// <param name="mainRenderTarget"></param>
	void InitSprites(RenderTarget& mainRenderTarget);
private:
	static const int NUM_SAMPLES = 16;	// 平均輝度を計算する際にサンプリングするテクセルの数。
	// 平均輝度を計算するとき処理ステップ
	enum CalcAvgStep {
		enCalcAvgStep_0,	// 平均輝度を計算。シーンの輝度を自然対数を底とする対数に変換してから近傍16テクセルの平均を計算する。
		enCalcAvgStep_1,	// 平均輝度を計算。16テクセルサンプリングして平均を計算する。
		enCalcAvgStep_2,	// 平均輝度を計算。16テクセルサンプリングして平均を計算する
		enCalcAvgStep_3,	// 平均輝度を計算。16テクセルサンプリングしての平均を計算する
		enCalcAvgStep_4,	// 平均輝度を計算。16テクセルサンプリングしての平均を計算する
		enCalcAvgStep_5,	// 平均輝度を計算。16テクセルサンプリングしてから対数の値から輝度に復元する。
		enNumCalcAvgStep,	// 処理のステップ数。
	};
	// step-1 Tonemapクラスに各種メンバ変数を追加する。
	RenderTarget m_calcAvgRt[enNumCalcAvgStep];		// 平均輝度計算用のレンダリングターゲット。
	RenderTarget m_tonemapRt;						// トーンマップ用のレンダリングターゲット。
	Sprite m_calcAvgSprites[enNumCalcAvgStep];		// 平均輝度計算用のスプライト。
	Sprite m_tonemapSprite;							// トーンマップ用のスプライト。
	Sprite m_copyMainRtSprite;						// メインレンダリングターゲットに描画するためのスプライト。
	Vector4 m_sampleUVOffsetArray[NUM_SAMPLES];		// 16テクセルサンプリングする際のUVオフセットのテーブル。
};

