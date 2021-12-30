#pragma once
/// <summary>
/// �V���h�E�}�b�v�N���X
/// </summary>
class ShadowMap
{
public:
	/// <summary>
	/// ������
	/// </summary>
	void Init();
	/// <summary>
	/// �V���h�E�}�b�v�ɕ`��
	/// </summary>
	/// <param name="rc">�����_�����O�R���e�L�X�g</param>
	void Draw(RenderContext& rc);
	/// <summary>
	/// �V���h�E�L���X�^�[��o�^�B
	/// </summary>
	/// <param name="caster"></param>
	void RegisterShadowCaster(Model& caster)
	{
		m_shadowCasterArray.emplace_back(&caster);
	}
	/// <summary>
	/// �V���h�E�}�b�v���擾�B
	/// </summary>
	/// <returns></returns>
	Texture& GetShadowMap()
	{
		return m_shadowMap.GetRenderTargetTexture();
	}
	/// <summary>
	/// ���C�g�r���[�v���W�F�N�V�����s����擾
	/// </summary>
	/// <returns></returns>
	const Matrix& GetLVPMatrix()
	{
		return m_lightCamera.GetViewProjectionMatrix();
	}
private:
	
	Camera m_lightCamera;								// ���C�g�J����
	RenderTarget m_shadowMap;						// �V���h�E�}�b�v�B
	std::vector<Model*> m_shadowCasterArray;	// �V���h�E�L���X�^�[�̔z��B
};

