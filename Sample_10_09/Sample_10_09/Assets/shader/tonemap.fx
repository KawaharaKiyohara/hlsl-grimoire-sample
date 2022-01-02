/*!
 * @brief	�g�[���}�b�v�B
 */


// ���_�V�F�[�_�[�ւ̓��͍\���̑́B
struct VSInput{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};
// �s�N�Z���V�F�[�_�[�ւ̓��͍\���́B
struct PSInput{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP�s��
    float4 mulColor;    // ��Z�J���[
};

/*!
 * @brief	���_�V�F�[�_�[�B
 */
PSInput VSMain(VSInput In) 
{
	PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}


Texture2D<float4> sceneTexture : register(t0);	//�V�[���e�N�X�`���B
sampler Sampler : register(s0);

static const int    MAX_SAMPLES            = 16;    // Maximum texture grabs

/*!
 * @brief	�萔�o�b�t�@�B
 */
cbuffer cbCalcLuminanceAvg : register(b0){
	float4 g_avSampleOffsets[MAX_SAMPLES];
};


/*!
 *@brief ACES�g�[���}�b�p�[
 */
float ACESFilm(float x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

////////////////////////////////////////////////////////
// RGB->HSV, HSV->RGB�ւ̐F��ԕϊ��֘A�̊֐��W�B
////////////////////////////////////////////////////////

/*!
 * @brief RGB�n����HSV�n�ɕϊ�����B
 */
float3 Rgb2Hsv(float3 c)
{
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
/*!
 * @brief RGB�n����HSV��V(�P�x)�����߂�B
 */
float Rgb2V( float3 rgb)
{
    return max(rgb.r, max(rgb.g, rgb.b));
}
/*!
 * @brief HSV�n����RGB�n�ɕϊ�����B
 */
float3 Hsv2Rgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

////////////////////////////////////////////////////////
// ��������P�x�̕��ς����߂鏈���B
////////////////////////////////////////////////////////

/*!
 * @brief �P�x�̎��R�ΐ��̕��ς����߂�s�N�Z���V�F�[�_�[
 * @detail �l�C�s�A�����Ƃ���P�x�̎��R�ΐ����ς����߂܂��B</br>
 *         C++����enCalcAvgStep_0�̏����ɊY�����܂��B
 */
float4 PSCalcLuminanceLogAvarage(PSInput In) : SV_Target0
{
    // step-10 �P�x�̑ΐ����ς����߂�B

}
/*!
 * @brief 16�e�N�Z���̕��ϋP�x�v�Z�����߂�s�N�Z���V�F�[�_�[
 * @detail ��������e�N�Z���̋ߖT16�e�N�Z�����T���v�����O���āA</br>
 *         ���ϒl���o�͂��܂��B</br>
 *         C++����enCalcAvgStep_0 �` enCalcAvgStep_4�ɊY�����܂��B
 */
float4 PSCalcLuminanceAvarage(PSInput In) : SV_Target0
{
    // step-11 �P�x�̑ΐ����ς̕��ς����߂�B

}

/*!
 *@brief	�P�x�̎��R�ΐ�����P�x�ɕ�������s�N�Z���V�F�[�_�[�̃G���g���[�|�C���g
 *@detail   ���ϋP�x�v�Z�̍Ō�̏����ł��B�ߖT16�e�N�Z�����T���v�����O���āA</br>
 *          ���ϒl���v�Z�������ƂŁAexp()�֐��𗘗p���Ď��R�ΐ�����P�x�ɕ������܂��B</br>
 *          C++����enCalcAvgStep_5�ɊY�����܂��B
 */
float4 PSCalcLuminanceExpAvarage( PSInput In ) : SV_Target0
{
    // step-12 �ΐ����ς̌v�Z�ƕ����B

}

////////////////////////////////////////////////////////
// �P�x�̕��ς����߂鏈���I���B
////////////////////////////////////////////////////////

Texture2D<float4> lumAvgTexture : register(t1);		        //���ϋP�x���L������Ă���e�N�X�`���B

/*!
 *@brief	���ϋP�x����g�[���}�b�v���s���s�N�Z���V�F�[�_�[�B
 */
float4 PSFinal( PSInput In) : SV_Target0
{
    // step-13 �V�[���̃J���[����P�x���v�Z����B

    // step-14 ���ϋP�x���g���āA���̃s�N�Z���̋P�x���X�P�[���_�E������B

    // step-15 �P�x�̕ω�����`�������`�ɂ���B

    // step-16 RGB�ɖ߂��B
    
    // step-17 �K���}�␳�B
    
	return sceneColor;
}