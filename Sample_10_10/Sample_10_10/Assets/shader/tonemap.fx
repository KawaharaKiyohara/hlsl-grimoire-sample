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



////////////////////////////////////////////////////////
// �P�x�̑ΐ����ς����߂�B
////////////////////////////////////////////////////////

static const float3 LUMINANCE_VECTOR  = float3(0.2125f, 0.7154f, 0.0721f);
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
        float3 color = max( sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[i].xy), 0.001f );
        float v = dot( LUMINANCE_VECTOR, color.xyz );
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

Texture2D<float4> lumAvgTexture : register(t0);		        //���ϋP�x
Texture2D<float4> lastLumAvgTextureArray[2] : register(t1);	//�P�t���[���O�̕��ϋP�x�̔z��

/*!
 *@brief	���Ï����̂��߂̕��ϋP�x�̓K��������s�N�Z���V�F�[�_�[�B
 */
float4 PSCalcAdaptedLuminance( PSInput In ) : SV_Target0
{
	float fAdaptedLum;
    
    if( currentAvgTexNo == 0){
        fAdaptedLum = lastLumAvgTextureArray[1].Sample(Sampler, float2( 0.5f, 0.5f));
    }else{
        fAdaptedLum = lastLumAvgTextureArray[0].Sample(Sampler, float2( 0.5f, 0.5f));
    } 
    float fCurrentLum = lumAvgTexture.Sample(Sampler, float2(0.5f, 0.5f));
    
    float fNewAdaptation = fAdaptedLum + (fCurrentLum - fAdaptedLum) * ( 1 - pow( 0.98f, 60 * deltaTime ) );
    return float4(fNewAdaptation, fNewAdaptation, fNewAdaptation, 1.0f);
}



/*!
 *@brief	���ϋP�x����g�[���}�b�v���s���s�N�Z���V�F�[�_�[�B
 */
float4 PSFinal( PSInput In) : SV_Target0
{
	// �V�[���̃J���[���T���v�����O����B
	float4 sceneColor = sceneTexture.Sample(Sampler, In.uv );

	float avgLum = 0.0f;
    if( currentAvgTexNo == 0){
        avgLum = lastLumAvgTextureArray[0].Sample(Sampler, float2( 0.5f, 0.5f)).r;
    }else{
        avgLum = lastLumAvgTextureArray[1].Sample(Sampler, float2( 0.5f, 0.5f)).r;
    }
    // �I���l���v�Z����B
    // ���ϋP�x��0.18�ɂ��邽�߂̃X�P�[���l�����߂�B
    float k = ( 0.18f / ( max(avgLum, 0.001f )));
    // �X�P�[�������̃s�N�Z���̋P�x�Ɋ|���Z����B
    sceneColor.xyz *= k;
    
    // Reinhard�֐��B�����reinhard�́A�P�x2.0�ȏ�͂����Ĕ���т���悤�ɂ��Ă���B
    float luminanceLimit = 2.0f;
    sceneColor.xyz = ( sceneColor.xyz / (sceneColor.xyz + 1.0f) ) * (1 + sceneColor.xyz / ( luminanceLimit * luminanceLimit ));

    // �K���}�␳
    sceneColor.xyz = pow( max( sceneColor.xyz, 0.0001f), 1.0f / 2.2f );
    
	return sceneColor;
}