#pragma once
/// <summary>
/// シャドウマップクラス
/// </summary>
class ShadowMap
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Init();
	/// <summary>
	/// シャドウマップに描画
	/// </summary>
	/// <param name="rc">レンダリングコンテキスト</param>
	void Draw(RenderContext& rc);
	/// <summary>
	/// シャドウキャスターを登録。
	/// </summary>
	/// <param name="caster"></param>
	void RegisterShadowCaster(Model& caster)
	{
		m_shadowCasterArray.emplace_back(&caster);
	}
	/// <summary>
	/// シャドウマップを取得。
	/// </summary>
	/// <returns></returns>
	Texture& GetShadowMap()
	{
		return m_shadowMap.GetRenderTargetTexture();
	}
private:
	Camera m_lightCamera;						// ライトカメラ
	RenderTarget m_shadowMap;					// シャドウマップ。
	std::vector<Model*> m_shadowCasterArray;	// シャドウキャスターの配列。
};

