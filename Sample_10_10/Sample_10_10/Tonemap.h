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
	/// <summary>
	/// ���ϋP�x���v�Z����B
	/// </summary>
	void ExecuteCalcAvg(RenderContext& rc);
	/// <summary>
	/// ���Ï���
	/// </summary>
	/// <param name="rc"></param>
	void ExecuteLuminanceAdapter(RenderContext& rc);
	/// <summary>
	/// ���ϋP�x���g���ăg�[���}�b�v�����s����B
	/// </summary>
	void ExecuteTonemap(RenderContext& rc);
	/// <summary>
	/// �g�[���}�b�v�������ʂ����C�������_�����O�^�[�Q�b�g�ɃR�s�[����B
	/// </summary>
	/// <param name="rc"></param>
	/// <param name="mainRenderTarget"></param>
	void ExecuteCopyResultToMainRenderTarget(RenderContext& rc, RenderTarget& mainRenderTarget);
	/// <summary>
	/// �e�탌���_�����O�^�[�Q�b�g���������B
	/// </summary>
	void InitRenderTargets(RenderTarget& mainRenderTarget);
	/// <summary>
	/// �e��X�v���C�g���������B
	/// </summary>
	/// <param name="mainRenderTarget"></param>
	void InitSprites(RenderTarget& mainRenderTarget);
private:
	static const int NUM_SAMPLES = 16;	// ���ϋP�x���v�Z����ۂɃT���v�����O����e�N�Z���̐��B
	struct STonemapParam {
		float deltaTime;
		float midddleGray;
		int currentAvgTexNo;
	};
	// ���ϋP�x���v�Z����Ƃ������X�e�b�v
	enum CalcAvgStep {
		enCalcAvgStep_0,	// ���ϋP�x���v�Z�B�V�[���̋P�x�����R�ΐ����Ƃ���ΐ��ɕϊ����Ă���ߖT16�e�N�Z���̕��ς��v�Z����B
		enCalcAvgStep_1,	// ���ϋP�x���v�Z�B16�e�N�Z���T���v�����O���ĕ��ς��v�Z����B
		enCalcAvgStep_2,	// ���ϋP�x���v�Z�B16�e�N�Z���T���v�����O���ĕ��ς��v�Z����
		enCalcAvgStep_3,	// ���ϋP�x���v�Z�B16�e�N�Z���T���v�����O���Ă̕��ς��v�Z����
		enCalcAvgStep_4,	// ���ϋP�x���v�Z�B16�e�N�Z���T���v�����O���Ă̕��ς��v�Z����
		enCalcAvgStep_5,	// ���ϋP�x���v�Z�B16�e�N�Z���T���v�����O���Ă���ΐ��̒l����P�x�ɕ�������B
		enNumCalcAvgStep,	// �����̃X�e�b�v���B
	};
	RenderTarget m_calcAvgRt[enNumCalcAvgStep];		// ���ϋP�x�v�Z�p�̃����_�����O�^�[�Q�b�g�B
	RenderTarget m_avgRt[2];						// ���ϋP�x���i�[����郌���_�����O�^�[�Q�b�g�B
	RenderTarget m_finalRt;							// �ŏI���������_�����O�^�[�Q�b�g�B
	int m_currentAvgRt;								// ���݂̃t���[���Ŏg�p���镽�ϒl���������܂�Ă��郌���_�����O�^�[�Q�b�g�B
	Sprite m_calcAvgSprites[enNumCalcAvgStep];		// ���ϋP�x�v�Z�p�̃X�v���C�g�B
	Sprite m_finalSprite;							// �ŏI�����p�̃X�v���C�g�B
	Sprite m_copyMainRtSprite;						// ���C�������_�����O�^�[�Q�b�g�ɕ`�悷�邽�߂̃X�v���C�g�B
	Sprite m_calcAdapteredLuminanceSprite;			// ���Ï������s�����߂̃X�v���C�g�B
	Vector4 m_sampleUVOffsetArray[NUM_SAMPLES];		// 16�e�N�Z���T���v�����O����ۂ�UV�I�t�Z�b�g�̃e�[�u���B
	STonemapParam m_tonemapParam;
};

