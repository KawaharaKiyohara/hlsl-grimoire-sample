#pragma once
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
	/// 現在のシーンの平均輝度を計算する。
	/// </summary>
	void ExecuteCalcAvgInCurrentScene(RenderContext& rc);
	/// <summary>
	/// トーンマップで使用する平均輝度を計算する。
	/// </summary>
	/// <param name="rc"></param>
	void ExecuteCalcAvgInTonemap(RenderContext& rc);
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
	/// トーンマップで使用した平均輝度テクスチャをコピー。
	/// </summary>
	/// <param name="rc"></param>
	void ExecuteCopyLuminanceAvgInTonemap(RenderContext& rc);
	/// <summary>
	/// 各種レンダリングターゲットを初期化。
	/// </summary>
	void InitRenderTargets(RenderTarget& mainRenderTarget);
	/// <summary>
	/// 各種スプライトを初期化。
	/// </summary>
	/// <param name="mainRenderTarget"></param>
	void InitSprites(RenderTarget& mainRenderTarget);
private:
	static const int NUM_SAMPLES = 16;	// 平均輝度を計算する際にサンプリングするテクセルの数。

	struct STonemapParam {
		float deltaTime;
		float midddleGray;
		int currentAvgTexNo;
	};
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
	
	RenderTarget m_avgRt[2];						// 平均輝度が格納されるレンダリングターゲット。
	RenderTarget m_finalRt;							// 最終合成レンダリングターゲット。
	Sprite m_calcAvgSprites[enNumCalcAvgStep];		// 平均輝度計算用のスプライト。
	Sprite m_finalSprite;							// 最終合成用のスプライト。
	Sprite m_copyMainRtSprite;						// メインレンダリングターゲットに描画するためのスプライト。
	
	Vector4 m_sampleUVOffsetArray[NUM_SAMPLES];		// 16テクセルサンプリングする際のUVオフセットのテーブル。
	STonemapParam m_tonemapParam;
	RenderTarget m_calcAvgRt[enNumCalcAvgStep];			// 平均輝度計算用のレンダリングターゲット。
	// step-1 各種メンバ変数を追加する。
	RenderTarget m_luminanceAvgInTonemapLastFrameRt;	// 1フレーム前のトーンマップで使用した平均輝度を記憶するためのレンダリングターゲット。
	RenderTarget m_luminanceAvgInTonemapRt;				// 現在のフレームのトーンマップで使用する平均輝度を記憶するためのレンダリングターゲット。	
	Sprite m_luminanceAvgInTonemapSprite;				// トーンマップで使用する平均輝度計算用のスプライト。
	Sprite m_copyLuminanceAvgInTonemapSprite;			// トーンマップで使用した平均輝度コピー用のスプライト。
};

