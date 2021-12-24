#include "stdafx.h"
#include "bloom.h"

void Bloom::Init(RenderTarget& mainRenderTarget)
{
    // �P�x���o�p�̃����_�����O�^�[�Q�b�g���쐬�B
    m_luminanceRenderTarget.Create(
        1280,   // �𑜓x�̓��C�������_�����O�^�[�Q�b�g�Ɠ���
        720,    // �𑜓x�̓��C�������_�����O�^�[�Q�b�g�Ɠ���
        1,
        1,
        // �y���ځz�J���[�o�b�t�@�[�̃t�H�[�}�b�g��32bit���������_�ɂ��Ă���
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_D32_FLOAT
    );
    // �P�x���o�p�̃X�v���C�g��������
    // �����������쐬����B
    SpriteInitData luminanceSpriteInitData;
    // �P�x���o�p�̃V�F�[�_�[�̃t�@�C���p�X���w�肷��
    luminanceSpriteInitData.m_fxFilePath = "Assets/shader/preset/bloom.fx";
    // ���_�V�F�[�_�[�̃G���g���[�|�C���g���w�肷��
    luminanceSpriteInitData.m_vsEntryPointFunc = "VSMain";
    // �s�N�Z���V�F�[�_�[�̃G���g���[�|�C���g���w�肷��
    luminanceSpriteInitData.m_psEntryPoinFunc = "PSSamplingLuminance";
    // �X�v���C�g�̕��ƍ�����luminnceRenderTarget�Ɠ���
    luminanceSpriteInitData.m_width = 1280;
    luminanceSpriteInitData.m_height = 720;
    // �e�N�X�`���̓��C�������_�����O�^�[�Q�b�g�̃J���[�o�b�t�@�[
    luminanceSpriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
    // �`�����ރ����_�����O�^�[�Q�b�g�̃t�H�[�}�b�g���w�肷��
    luminanceSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    // �쐬�����������������ƂɃX�v���C�g������������
    m_luminanceSprite.Init(luminanceSpriteInitData);

    // �u���[����������������
    // gaussianBlur[0]�͋P�x�e�N�X�`���ɃK�E�V�A���u���[��������
    m_blur[0].Init(&m_luminanceRenderTarget.GetRenderTargetTexture());
    // gaussianBlur[1]��gaussianBlur[0]�̃e�N�X�`���ɃK�E�V�A���u���[��������
    m_blur[1].Init(&m_blur[0].GetBokeTexture());
    // gaussianBlur[2]��gaussianBlur[1]�̃e�N�X�`���ɃK�E�V�A���u���[��������
    m_blur[2].Init(&m_blur[1].GetBokeTexture());
    // gaussianBlur[3]��gaussianBlur[2]�̃e�N�X�`���ɃK�E�V�A���u���[��������
    m_blur[3].Init(&m_blur[2].GetBokeTexture());

    // �{�P�摜���������ď������ނ��߂̃X�v���C�g��������
    // ����������ݒ肷��B
    SpriteInitData finalSpriteInitData;
    // �{�P�e�N�X�`����4���w�肷��
    finalSpriteInitData.m_textures[0] = &m_blur[0].GetBokeTexture();
    finalSpriteInitData.m_textures[1] = &m_blur[1].GetBokeTexture();
    finalSpriteInitData.m_textures[2] = &m_blur[2].GetBokeTexture();
    finalSpriteInitData.m_textures[3] = &m_blur[3].GetBokeTexture();
    // �𑜓x��mainRenderTarget�̕��ƍ���
    finalSpriteInitData.m_width = 1280;
    finalSpriteInitData.m_height = 720;
    // �ڂ������摜���A�ʏ��2D�Ƃ��ă��C�������_�����O�^�[�Q�b�g�ɕ`�悷��̂ŁA
    // 2D�p�̃V�F�[�_�[���g�p����
    finalSpriteInitData.m_fxFilePath = "Assets/shader/preset/bloom.fx";
    finalSpriteInitData.m_psEntryPoinFunc = "PSBloomFinal";

    // �������A���Z�����ŕ`�悷��̂ŁA�A���t�@�u�����f�B���O���[�h�����Z�ɂ���
    finalSpriteInitData.m_alphaBlendMode = AlphaBlendMode_Add;
    // �J���[�o�b�t�@�[�̃t�H�[�}�b�g�͗�ɂ���āA32�r�b�g���������_�o�b�t�@�[
    finalSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    // �������������ɉ��Z�����p�̃X�v���C�g������������
    m_finalSprite.Init(finalSpriteInitData);
}

void Bloom::Draw(RenderContext& rc, RenderTarget& mainRenderTarget)
{
    // �P�x���o
        // �P�x���o�p�̃����_�����O�^�[�Q�b�g�ɕύX
    rc.WaitUntilToPossibleSetRenderTarget(m_luminanceRenderTarget);
    // �����_�����O�^�[�Q�b�g��ݒ�
    rc.SetRenderTargetAndViewport(m_luminanceRenderTarget);
    // �����_�����O�^�[�Q�b�g���N���A
    rc.ClearRenderTargetView(m_luminanceRenderTarget);
    // �P�x���o���s��
    m_luminanceSprite.Draw(rc);
    // �����_�����O�^�[�Q�b�g�ւ̏������ݏI���҂�
    rc.WaitUntilFinishDrawingToRenderTarget(m_luminanceRenderTarget);

    // step-3 �K�E�V�A���u���[��4����s����
    m_blur[0].ExecuteOnGPU(rc, 10);
    m_blur[1].ExecuteOnGPU(rc, 10);
    m_blur[2].ExecuteOnGPU(rc, 10);
    m_blur[3].ExecuteOnGPU(rc, 10);

    // step-4 �{�P�摜���������ă��C�������_�����O�^�[�Q�b�g�ɉ��Z����
    // �����_�����O�^�[�Q�b�g�Ƃ��ė��p�ł���܂ő҂�
    rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
    // �����_�����O�^�[�Q�b�g��ݒ�
    rc.SetRenderTargetAndViewport(mainRenderTarget);
    // �ŏI����
    m_finalSprite.Draw(rc);
    // �����_�����O�^�[�Q�b�g�ւ̏������ݏI���҂�
    rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
}
