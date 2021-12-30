#include "stdafx.h"
#include "ShadowMap.h"

void ShadowMap::Init()
{
	// シャドウマップを作成。
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_shadowMap.Create(
		4096,
		4096,
		1,
		1,
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_D32_FLOAT,
		clearColor
	);

	// ライトカメラを初期化。
	Vector3 lightTarget = { 0.0f, 0.0f, 0.0f };
	m_lightCamera.SetTarget(lightTarget);
	Vector3 dirLight = { 1.0f, 1.0f, 1.0f };
	dirLight.Normalize();
	Vector3 lightPos = lightTarget + dirLight * 5000.0f;
	m_lightCamera.SetPosition(lightPos);
	m_lightCamera.SetUp(g_vec3AxisX);
	m_lightCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
	m_lightCamera.SetWidth(5000.0f);
	m_lightCamera.SetHeight(5000.0f);
	m_lightCamera.SetFar(10000.0f);
	m_lightCamera.Update();
}

void ShadowMap::Draw(RenderContext& rc)
{
	rc.WaitUntilToPossibleSetRenderTarget(m_shadowMap);
	rc.SetRenderTargetAndViewport(m_shadowMap);
	rc.ClearRenderTargetView(m_shadowMap);

	for (Model* caster : m_shadowCasterArray) {
		caster->Draw(rc, m_lightCamera);
	}

	rc.WaitUntilFinishDrawingToRenderTarget(m_shadowMap);
}