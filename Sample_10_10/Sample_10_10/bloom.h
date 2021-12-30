#pragma once
/// <summary>
/// �u���[��
/// </summary>
class Bloom
{
public:
	/// <summary>
	/// ������
	/// </summary>
	/// <param name="mainRenderTarget">���C�������_�����O�^�[�Q�b�g</param>
	void Init(RenderTarget& mainRenderTarget);
	
	/// <summary>
	/// �`��
	/// </summary>
	/// <param name="rc">�����_�����O�R���e�L�X�g</param>
	void Draw(RenderContext& rc, RenderTarget& mainRenderTarget);
private:
	RenderTarget m_luminanceRenderTarget;	// �P�x���o�p�̃����_�����O�^�[�Q�b�g�B
	Sprite m_luminanceSprite;				// �P�x���o�p�̃X�v���C�g�B
	GaussianBlur m_blur[4];					// �P�x�e�N�X�`�����ڂ��������B
	Sprite m_finalSprite;					// �ŏI�����p�̃X�v���C�g


};

