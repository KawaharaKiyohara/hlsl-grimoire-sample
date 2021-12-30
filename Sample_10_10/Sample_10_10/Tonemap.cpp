#include "stdafx.h"
#include "Tonemap.h"

namespace {
	/// <summary>
	/// �ߖT16�s�N�Z�����T���v�����O���邽�߂�UV�I�t�Z�b�g���v�Z����B
	/// </summary>
	/// <param name="dwWidth">���̃e�N�X�`���̕�</param>
	/// <param name="dwHeight">���̃e�N�X�`���̍���</param>
	/// <param name="avSampleOffsets">UV�I�t�Z�b�g�̋L����B</param>
	/// <returns></returns>
	void GetSampleOffsets4x4(DWORD dwWidth, DWORD dwHeight, Vector4 avSampleOffsets[])
	{
		// 1�e�N�Z���I�t�Z�b�g
		float tU = 1.0f / dwWidth;
		float tV = 1.0f / dwHeight;

		int index = 0;
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				avSampleOffsets[index].x = (x - 1.5f) * tU;
				avSampleOffsets[index].y = (y - 1.5f) * tV;

				index++;
			}
		}
	}
	/// <summary>
	/// �ߐ�9�s�N�Z�����T���v�����O���邽�߂�UV�I�t�Z�b�g���v�Z����B
	/// </summary>
	/// <param name="dwWidth"></param>
	/// <param name="dwHeight"></param>
	/// <param name="avSampleOffsets"></param>
	void GetSampleOffset3x3(DWORD dwWidth, DWORD dwHeight, Vector4 avSampleOffsets[])
	{
		// 1�e�N�Z���I�t�Z�b�g
		float tU = 1.0f / dwWidth;
		float tV = 1.0f / dwHeight;

		int index = 0;
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {
				avSampleOffsets[index].x = (x - 1.0f) * tU;
				avSampleOffsets[index].y = (y - 1.0f) * tV;

				index++;
			}
		}
	}
}


void Tonemap::InitRenderTargets(RenderTarget& mainRenderTarget)
{
	// ���ϒl���v�Z���鏈���̃����_�����O�^�[�Q�b�g�̃T�C�Y�B
	static const int calcAVGRtSize[enNumCalcAvgStep] = {
		1024,		// enCalcAvg_0
		256,		// enCalcAvg_1
		64,			// enCalcAvg_2
		16,			// enCalcAvg_3
		4,			// enCalcAvg_4
		1			// enCalcAvg_5
	};
	// ���ϋP�x�v�Z�p�̃����_�����O�^�[�Q�b�g���쐬�B
	for (int i = 0; i < enNumCalcAvgStep; i++) {
		m_calcAvgRt[i].Create(
			calcAVGRtSize[i],
			calcAVGRtSize[i],
			1,
			1,
			DXGI_FORMAT_R16_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
	}
	// �ŏI�I�Ƀg�[���}�b�v�Ŏg�p���镽�ϋP�x���������ރ����_�����O�^�[�Q�b�g���쐬
	for (auto& avgRt : m_avgRt) {
		avgRt.Create(
			1, 1, 1, 1, DXGI_FORMAT_R16_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
	}
	// �g�[���}�b�v�p�̃X�v���C�g���쐬
	m_finalRt.Create(
		mainRenderTarget.GetWidth(),
		mainRenderTarget.GetHeight(),
		1,
		1,
		mainRenderTarget.GetColorBufferFormat(),
		DXGI_FORMAT_UNKNOWN
	);
}

void Tonemap::InitSprites(RenderTarget& mainRenderTarget)
{
	for (int procStep = 0; procStep < enNumCalcAvgStep; procStep++)
	{
		SpriteInitData initData;
		initData.m_width = m_calcAvgRt[procStep].GetWidth();
		initData.m_height = m_calcAvgRt[procStep].GetHeight();
		initData.m_colorBufferFormat[0] = m_calcAvgRt[procStep].GetColorBufferFormat();
		initData.m_expandConstantBuffer = m_sampleUVOffsetArray;
		initData.m_expandConstantBufferSize = sizeof(m_sampleUVOffsetArray);
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";

		if (procStep == enCalcAvgStep_0) {
			// ���R�ΐ����Ƃ���ΐ��̕��ς��Ƃ�X�v���C�g��������
			initData.m_psEntryPoinFunc = "PSCalcLuminanceLogAvarage";
			initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
			m_calcAvgSprites[procStep].Init(initData);
		}
		else if (procStep == enCalcAvgStep_5) {
			// exp�֐���p���Ď��R�ΐ����Ƃ���ΐ����畽�ϋP�x�ɕ������邽�߂̃X�v���C�g���������B
			initData.m_psEntryPoinFunc = "PSCalcLuminanceExpAvarage";
			initData.m_textures[0] = &m_calcAvgRt[procStep - 1].GetRenderTargetTexture();
			m_calcAvgSprites[procStep].Init(initData);
		}
		else {
			// ���ς��Ƃ�X�v���C�g���������B
			initData.m_psEntryPoinFunc = "PSCalcLuminanceAvarage";
			initData.m_textures[0] = &m_calcAvgRt[procStep - 1].GetRenderTargetTexture();
			m_calcAvgSprites[procStep].Init(initData);
		}
	}
	// ���Ï���
	{
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = m_calcAvgRt[enCalcAvgStep_5].GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSCalcAdaptedLuminance";
		initData.m_expandConstantBuffer = &m_tonemapParam;
		initData.m_expandConstantBufferSize = sizeof(m_tonemapParam);
		initData.m_textures[0] = &m_calcAvgRt[enCalcAvgStep_5].GetRenderTargetTexture();
		initData.m_textures[1] = &m_avgRt[0].GetRenderTargetTexture();
		initData.m_textures[2] = &m_avgRt[1].GetRenderTargetTexture();

		m_calcAdapteredLuminanceSprite.Init(initData);
	}
	// ���ϋP�x���g���ăg�[���}�b�v���s�����߂̃X�v���C�g���������B
	{
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/tonemap.fx";
		initData.m_psEntryPoinFunc = "PSFinal";
		initData.m_expandConstantBuffer = &m_tonemapParam;
		initData.m_expandConstantBufferSize = sizeof(m_tonemapParam);
		initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
		initData.m_textures[1] = &m_avgRt[0].GetRenderTargetTexture();
		initData.m_textures[2] = &m_avgRt[1].GetRenderTargetTexture();

		m_finalSprite.Init(initData);
	}
	// �g�[���}�b�v���ꂽ�G�����C�������_�����O�^�[�Q�b�g�ɃR�s�[���邽�߂̃X�v���C�g���������B
	{
		SpriteInitData initData;
		initData.m_width = mainRenderTarget.GetWidth();
		initData.m_height = mainRenderTarget.GetHeight();
		initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();
		initData.m_fxFilePath = "Assets/shader/preset/sprite.fx";
		initData.m_textures[0] = &m_finalRt.GetRenderTargetTexture();
		m_copyMainRtSprite.Init(initData);
	}
}

void Tonemap::ExecuteCalcAvg(RenderContext& rc)
{
	for (int procStep = 0; procStep < enNumCalcAvgStep; procStep++) {
		// �����_�����O�^�[�Q�b�g�Ƃ��ė��p�ł���܂ő҂�
		rc.WaitUntilToPossibleSetRenderTarget(m_calcAvgRt[procStep]);
		// �����_�����O�^�[�Q�b�g��ݒ�
		rc.SetRenderTargetAndViewport(m_calcAvgRt[procStep]);
		rc.ClearRenderTargetView(m_calcAvgRt[procStep]);
		GetSampleOffsets4x4(
			m_calcAvgSprites[procStep].GetTextureWidth(0),
			m_calcAvgSprites[procStep].GetTextureHeight(0),
			m_sampleUVOffsetArray
		);
		m_calcAvgSprites[procStep].Draw(rc);

		// �����_�����O�^�[�Q�b�g�ւ̏������ݏI���҂�
		rc.WaitUntilFinishDrawingToRenderTarget(m_calcAvgRt[procStep]);
	}
}

void Tonemap::ExecuteTonemap(RenderContext& rc)
{
	// �����_�����O�^�[�Q�b�g��ݒ�
	rc.WaitUntilToPossibleSetRenderTarget(m_finalRt);
	rc.SetRenderTargetAndViewport(m_finalRt);
	// �ŏI�����B
	m_finalSprite.Draw(rc);
	// �����_�����O�^�[�Q�b�g�ւ̏������ݏI���҂�
	rc.WaitUntilFinishDrawingToRenderTarget(m_finalRt);
}

void Tonemap::ExecuteCopyResultToMainRenderTarget(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// �ŏI�������ꂽ�G�����C�������_�����O�^�[�Q�b�g�ɃR�s�[����B
	rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
	// �����_�����O�^�[�Q�b�g��ݒ�
	rc.SetRenderTargetAndViewport(mainRenderTarget);
	m_copyMainRtSprite.Draw(rc);
	// �����_�����O�^�[�Q�b�g�ւ̏������ݏI���҂�
	rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
}
void Tonemap::ExecuteLuminanceAdapter(RenderContext& rc)
{
	m_tonemapParam.currentAvgTexNo = m_currentAvgRt;

	rc.WaitUntilToPossibleSetRenderTarget(m_avgRt[m_currentAvgRt]);
	// �����_�����O�^�[�Q�b�g��ݒ�
	rc.SetRenderTargetAndViewport(m_avgRt[m_currentAvgRt]);
	m_calcAdapteredLuminanceSprite.Draw(rc);
	// �����_�����O�^�[�Q�b�g�ւ̏������ݏI���҂�
	rc.WaitUntilFinishDrawingToRenderTarget(m_avgRt[m_currentAvgRt]);
}

void Tonemap::Init(RenderTarget& mainRenderTarget)
{
	m_tonemapParam.midddleGray = 0.4f;
	m_tonemapParam.deltaTime = 1.0f / 60.0f;

	// 1. �e�탌���_�����O�^�[�Q�b�g������������B
	InitRenderTargets(mainRenderTarget);

	// 2. �e��X�v���C�g������������B
	InitSprites(mainRenderTarget);
}
void Tonemap::Execute(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// 1. �V�[���̕��ϋP�x�v�Z�B
	ExecuteCalcAvg( rc);

	// 2. ���Ï����B
	ExecuteLuminanceAdapter(rc);

	// 3. �V�[���̕��ϋP�x���g���ăg�[���}�b�v�B
	ExecuteTonemap(rc);
	
	// 4. �g�[���}�b�v�������ʂ̉摜�����C�������_�����O�^�[�Q�b�g�ɃR�s�[����B
	ExecuteCopyResultToMainRenderTarget(rc, mainRenderTarget);

	// 5. ���ϒl���v�Z���郌���_�����O�^�[�Q�b�g�̔ԍ������ւ���B
	m_currentAvgRt = 1 ^ m_currentAvgRt;
}