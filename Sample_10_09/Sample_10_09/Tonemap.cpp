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
	// step-2 ���ϋP�x�v�Z�p�̃����_�����O�^�[�Q�b�g���쐬�B
	
	// step-3 �g�[���}�b�v���s�����߂̃����_�����O�^�[�Q�b�g���쐬�B
	
}
void Tonemap::InitSprites(RenderTarget& mainRenderTarget)
{
	// step-4 ���ϋP�x�v�Z�̂��߂̃X�v���C�g���������B
	
	{
		// step-5 �g�[���}�b�v���s�����߂̃X�v���C�g���������B
	
	}
	{
		// step-6 �g�[���}�b�v���ꂽ�G�����C�������_�����O�^�[�Q�b�g�ɃR�s�[���邽�߂̃X�v���C�g���������B
	
	}
}

void Tonemap::ExecuteCalcAvg(RenderContext& rc)
{
	// step-7 �V�[���̋P�x�̕��ς��v�Z���鏈�������s�B
	
}
void Tonemap::ExecuteTonemap(RenderContext& rc)
{
	// step-8 �g�[���}�b�v�����s�B
	
}
void Tonemap::ExecuteCopyResultToMainRenderTarget(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// step-9 �g�[���}�b�v���ꂽ�G�����C�������_�����O�^�[�Q�b�g�ɃR�s�[�B

}
void Tonemap::Init(RenderTarget& mainRenderTarget)
{
	// 1. �e�탌���_�����O�^�[�Q�b�g���������B
	InitRenderTargets(mainRenderTarget);

	// 2. �e��X�v���C�g���������B
	InitSprites(mainRenderTarget);
}
void Tonemap::Execute(RenderContext& rc, RenderTarget& mainRenderTarget)
{
	// 1. �V�[���̕��ϋP�x�v�Z�B
	ExecuteCalcAvg(rc);
	
	// 2. �V�[���̕��ϋP�x���g���ăg�[���}�b�v�B
	ExecuteTonemap(rc);

	// 3. �g�[���}�b�v�������ʂ̉摜�����C�������_�����O�^�[�Q�b�g�ɃR�s�[����B
	ExecuteCopyResultToMainRenderTarget(rc, mainRenderTarget);
	
}