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
	/// シーンの平均輝度を計算する。
	/// </summary>
	/// <param name="rc">レンダリングコンテキスト</param>
	/// <param name="mainRenderTarget">メインレンダリングターゲット。</param>
	void CalcLuminanceAvarage(RenderContext& rc, RenderTarget& mainRenderTarget);
private:
	static const int NUM_SAMPLES = 16;	// 平均輝度を計算する際にサンプリングするテクセルの数。
	struct STonemapParam {
		float deltaTime;
		float midddleGray;
		int currentAvgTexNo;
	};
	enum CalcAvgSprite {
		enCalcAvgLog,					// 対数平均を求める。
		enCalcAvg_Start,
		enCalcAvg_0 = enCalcAvg_Start,	// 平均輝度を計算。
		enCalcAvg_1,					// 平均輝度を計算。
		enCalcAvg_2,					// 平均輝度を計算。	
		enCalcAvg_3,					// 平均輝度を計算する。
		enCalcAvg_End,
		enCalcAvgExp = enCalcAvg_End,	// exp()を用いて最終平均を求める。
		enNumCalcAvgSprite
	};
	RenderTarget m_calcAvgRt[enNumCalcAvgSprite];	// 平均輝度計算用のレンダリングターゲット。
	RenderTarget m_avgRt[2];						// 平均輝度が格納されるレンダリングターゲット。
	RenderTarget m_finalRt;							// 最終合成レンダリングターゲット。
	int m_currentAvgRt;								// 現在のフレームで使用する平均値が書き込まれているレンダリングターゲット。
	Sprite m_calcAvgSprites[enNumCalcAvgSprite];	// 平均輝度計算用のスプライト。
	Sprite m_finalSprite;							// 最終合成用のスプライト。
	Sprite m_copyMainRtSprite;						// メインレンダリングターゲットに描画するためのスプライト。
	Sprite m_calcAdapteredLuminanceSprite;			// 明暗順応を行うためのスプライト。
	Vector4 m_sampleUVOffsetArray[NUM_SAMPLES];		// 16テクセルサンプリングする際のUVオフセットのテーブル。
	STonemapParam m_tonemapParam;
};

