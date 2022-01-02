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

//�g�[���}�b�v�̋��ʒ萔�o�b�t�@�B
cbuffer cbTonemapCommon : register(b1){
	float deltaTime;
	float middleGray;
    int currentAvgTexNo;
}

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
// �P�x�̑ΐ����ς����߂�B
////////////////////////////////////////////////////////

Texture2D<float4> sceneTexture : register(t0);	//�V�[���e�N�X�`���B
sampler Sampler : register(s0);

static const int    MAX_SAMPLES            = 16;    // Maximum texture grabs

/*!
 * @brief	�萔�o�b�t�@�B
 */
cbuffer cbCalcLuminanceAvg : register(b0){
	float4 g_avSampleOffsets[MAX_SAMPLES];
};

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
    float3 LUMINANCE_VECTOR  = float3(0.2125f, 0.7154f, 0.0721f);   // �P�x���o�p�̃x�N�g���B
    float  fLogLumSum = 0.0f;                                       // �P�x�̎��R�ΐ��̍��v���L������ϐ��B
    
    // 9�e�N�Z���T���v�����O����B
    for(int i = 0; i < 9; i++)
    {
        // �V�[���̃J���[���T���v�����O�B
        float3 color = max( sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[i].xy), 0.001f );
        // RGB����P�x�����߂�B
        float v = Rgb2V(color);
        // �l�C�s�A�����Ƃ���P�x�̎��R�ΐ������Z����B
        fLogLumSum += log(v);
    }
    // 9�ŏ��Z���ĕ��ς����߂�B
    fLogLumSum /= 9;

    return float4(fLogLumSum, fLogLumSum, fLogLumSum, 1.0f);
}
/*!
 * @brief 16�e�N�Z���̕��ϋP�x�v�Z�����߂�s�N�Z���V�F�[�_�[
 * @detail ��������e�N�Z���̋ߖT16�e�N�Z�����T���v�����O���āA</br>
 *         ���ϒl���o�͂��܂��B</br>
 *         C++����enCalcAvgStep_0 �` enCalcAvgStep_4�ɊY�����܂��B
 */
float4 PSCalcLuminanceAvarage(PSInput In) : SV_Target0
{
	float fResampleSum = 0.0f; 
    
    // 16�e�N�Z���T���v�����O���ĕ��ς����߂�B
    for(int iSample = 0; iSample < 16; iSample++)
    {
        fResampleSum += sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[iSample].xy);
    }
    
    fResampleSum /= 16;

    return float4(fResampleSum, fResampleSum, fResampleSum, 1.0f);
}

/*!
 *@brief	�P�x�̎��R�ΐ�����P�x�ɕ�������s�N�Z���V�F�[�_�[�̃G���g���[�|�C���g
 *@detail   ���ϋP�x�v�Z�̍Ō�̏����ł��B�ߖT16�e�N�Z�����T���v�����O���āA</br>
 *          ���ϒl���v�Z�������ƂŁAexp()�֐��𗘗p���Ď��R�ΐ�����P�x�ɕ������܂��B</br>
 *          C++����enCalcAvgStep_5�ɊY�����܂��B
 */
float4 PSCalcLuminanceExpAvarage( PSInput In ) : SV_Target0
{
	float fResampleSum = 0.0f;
    
    for(int iSample = 0; iSample < 16; iSample++)
    {
        fResampleSum += sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[iSample]);
    }
    
    // exp()�𗘗p���ĕ�������B
    fResampleSum = exp(fResampleSum/16);
    
    return float4(fResampleSum, fResampleSum, fResampleSum, 1.0f);
}

////////////////////////////////////////////////////////
// �P�x�̕��ς����߂鏈���I���B
////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// �������疾�Ï���
/////////////////////////////////////////////////////////

// step-7 �����t���[���̕��ϋP�x�v�Z�Ŏg�p����e�N�X�`���ɃA�N�Z�X��ϐ���`�B


/*!
 *@brief	�����t���[���ɂ킽���Ă̕��ϋP�x���v�Z����s�N�Z���V�F�[�_�[�B
 */
float4 PSCalcLuminanceInTonemap( PSInput In ) : SV_Target0
{
    // step-8 �����t���[���ɂ킽���Ă̕��ϋP�x���v�Z����B
    
}


Texture2D<float4> lumAvgInTonemapTexture : register(t1);    // �g�[���}�b�v�Ŏg�p���镽�ϋP�x�B

/*!
 *@brief	���ϋP�x����g�[���}�b�v���s���s�N�Z���V�F�[�_�[�B
 */
float4 PSFinal( PSInput In) : SV_Target0
{
	// �V�[���̃J���[���T���v�����O����B
	float4 sceneColor = sceneTexture.Sample(Sampler, In.uv );
    // �V�[���̃J���[��RGB�n����HSV�n�ɕϊ�����B
    float3 hsv = Rgb2Hsv(sceneColor);

	float avgLum = lumAvgInTonemapTexture.Sample(Sampler, float2( 0.5f, 0.5f)).r;
    
    // ���ϋP�x��middleGray�̒l�ɂ��邽�߂̃X�P�[���l�����߂�B
    float k = ( middleGray / ( max(avgLum, 0.001f )));
    // �X�P�[���l���g���āA�P�x���X�P�[���_�E���B
    hsv.z *= k;
    
    // ACES�g�[���}�b�p�[���g���ċP�x�̕ω������`�ɂ���B
    hsv.z = ACESFilm(hsv.z);

    // HSV�n����RGB�n�ɖ߂��B
    sceneColor.xyz = Hsv2Rgb(hsv);

    // �K���}�␳
    sceneColor.xyz = pow( max( sceneColor.xyz, 0.0001f), 1.0f / 2.2f );
    
	return sceneColor;
}