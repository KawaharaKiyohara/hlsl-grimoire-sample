#pragma once
class Tonemap
{
public:
	/// <summary>
	/// ������
	/// </summary>
	void Init(RenderTarget& mainRenderTarget);
	/// <summary>
	/// �g�[���}�b�v�����s�B
	/// </summary>
	void Execute(RenderContext& rc, RenderTarget& mainRenderTarget);
private:
	static const int NUM_SAMPLES = 16;	// ���ϋP�x���v�Z����ۂɃT���v�����O����e�N�Z���̐��B
	enum CalcAvgSprite {
		enCalcAvgLog,					// �ΐ����ς����߂�B
		enCalcAvg_Start,
		enCalcAvg_0 = enCalcAvg_Start,	// ���ϋP�x���v�Z�B
		enCalcAvg_1,					// ���ϋP�x���v�Z�B
		enCalcAvg_2,					// ���ϋP�x���v�Z�B	
		enCalcAvg_3,					// ���ϋP�x���v�Z����B
		enCalcAvg_End,
		enCalcAvgExp = enCalcAvg_End,	// exp()��p���čŏI���ς����߂�B
		enNumCalcAvgSprite
	};
	RenderTarget m_calcAvgRt[enNumCalcAvgSprite];	// ���ϋP�x�v�Z�p�̃����_�����O�^�[�Q�b�g�B
	RenderTarget m_avgRt;							// ���ϋP�x���i�[����郌���_�����O�^�[�Q�b�g�B
	RenderTarget m_finalRt;							// �ŏI���������_�����O�^�[�Q�b�g�B
	Sprite m_calcAvgSprites[enNumCalcAvgSprite];	// ���ϋP�x�v�Z�p�̃X�v���C�g�B
	Sprite m_finalSprite;							// �ŏI�����p�̃X�v���C�g�B
	Sprite m_copyMainRtSprite;						// ���C�������_�����O�^�[�Q�b�g�ɕ`�悷�邽�߂̃X�v���C�g�B
	Vector4 m_sampleUVOffsetArray[NUM_SAMPLES];		// 16�e�N�Z���T���v�����O����ۂ�UV�I�t�Z�b�g�̃e�[�u���B
};

