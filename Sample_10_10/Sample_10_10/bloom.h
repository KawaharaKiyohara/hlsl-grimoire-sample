#pragma once
/// <summary>
/// ブルーム
/// </summary>
class Bloom
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="mainRenderTarget">メインレンダリングターゲット</param>
	void Init(RenderTarget& mainRenderTarget);
	
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="rc">レンダリングコンテキスト</param>
	void Draw(RenderContext& rc, RenderTarget& mainRenderTarget);
private:
	RenderTarget m_luminanceRenderTarget;	// 輝度抽出用のレンダリングターゲット。
	Sprite m_luminanceSprite;				// 輝度抽出用のスプライト。
	GaussianBlur m_blur[4];					// 輝度テクスチャをぼかす処理。
	Sprite m_finalSprite;					// 最終合成用のスプライト


};

