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
	/// ���݂̃V�[���̕��ϋP�x���v�Z����B
	/// </summary>
	void ExecuteCalcAvgInCurrentScene(RenderContext& rc);
	/// <summary>
	/// �g�[���}�b�v�Ŏg�p���镽�ϋP�x���v�Z����B
	/// </summary>
	/// <param name="rc"></param>
	void ExecuteCalcAvgInTonemap(RenderContext& rc);
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
	/// �g�[���}�b�v�Ŏg�p�������ϋP�x�e�N�X�`�����R�s�[�B
	/// </summary>
	/// <param name="rc"></param>
	void ExecuteCopyLuminanceAvgInTonemap(RenderContext& rc);
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
	
	RenderTarget m_avgRt[2];						// ���ϋP�x���i�[����郌���_�����O�^�[�Q�b�g�B
	RenderTarget m_finalRt;							// �ŏI���������_�����O�^�[�Q�b�g�B
	Sprite m_calcAvgSprites[enNumCalcAvgStep];		// ���ϋP�x�v�Z�p�̃X�v���C�g�B
	Sprite m_finalSprite;							// �ŏI�����p�̃X�v���C�g�B
	Sprite m_copyMainRtSprite;						// ���C�������_�����O�^�[�Q�b�g�ɕ`�悷�邽�߂̃X�v���C�g�B
	
	Vector4 m_sampleUVOffsetArray[NUM_SAMPLES];		// 16�e�N�Z���T���v�����O����ۂ�UV�I�t�Z�b�g�̃e�[�u���B
	STonemapParam m_tonemapParam;
	RenderTarget m_calcAvgRt[enNumCalcAvgStep];			// ���ϋP�x�v�Z�p�̃����_�����O�^�[�Q�b�g�B
	// step-1 �e�탁���o�ϐ���ǉ�����B
	RenderTarget m_luminanceAvgInTonemapLastFrameRt;	// 1�t���[���O�̃g�[���}�b�v�Ŏg�p�������ϋP�x���L�����邽�߂̃����_�����O�^�[�Q�b�g�B
	RenderTarget m_luminanceAvgInTonemapRt;				// ���݂̃t���[���̃g�[���}�b�v�Ŏg�p���镽�ϋP�x���L�����邽�߂̃����_�����O�^�[�Q�b�g�B	
	Sprite m_luminanceAvgInTonemapSprite;				// �g�[���}�b�v�Ŏg�p���镽�ϋP�x�v�Z�p�̃X�v���C�g�B
	Sprite m_copyLuminanceAvgInTonemapSprite;			// �g�[���}�b�v�Ŏg�p�������ϋP�x�R�s�[�p�̃X�v���C�g�B
};

