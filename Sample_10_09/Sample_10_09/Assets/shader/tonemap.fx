/*!
 * @brief	�g�[���}�b�v�B
 */

struct VSInput{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};
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

float3 Rgb2Hsv(float3 rgb)
{
    float3 hsv;
    
    // RGB 2 HSV
    float fMax = max(rgb.r, max(rgb.g, rgb.b));
    float fMin = min(rgb.r, min(rgb.g, rgb.b));
    float delta = fMax - fMin;

	hsv.z = fMax; // v
	if (fMax != 0.0){
	    hsv.y = delta / fMax;//s
	}else{
	    hsv.y = 0.0;//s
    }
	
//	if (hsv.y == 0.0) {
//		hsv.x = NO_HUE; // h
//	} else {
      if ( rgb.r == fMax ){
          hsv.x =     (rgb.g - rgb.b) / delta;// h
      }else if (rgb.g == fMax){
          hsv.x = 2 + (rgb.b - rgb.r) / delta;// h
      }else{
          hsv.x = 4 + (rgb.r - rgb.g) / delta;// h
      }
      hsv.x /= 6.0;
      if (hsv.x < 0) hsv.x += 1.0;
//  }

    return hsv;
}
// RGB����HSV��V(�P�x)�����߂�
float Rgb2V( float3 rgb)
{
    return max(rgb.r, max(rgb.g, rgb.b));
}
float3 Hsv2Rgb(float3 hsv)
{
    float3 ret;
    // HSV 2 RGB
    if ( hsv.y == 0 ){ /* Grayscale */
        ret.r = ret.g = ret.b = hsv.z;// v
    } else {
        if (1.0 <= hsv.x) hsv.x -= 1.0;
        hsv.x *= 6.0;
        float i = floor (hsv.x);
        float f = hsv.x - i;
        float aa = hsv.z * (1 - hsv.y);
        float bb = hsv.z * (1 - (hsv.y * f));
        float cc = hsv.z * (1 - (hsv.y * (1 - f)));
        if( i < 1 ){
	        ret.r = hsv.z; ret.g = cc;    ret.b = aa;
        }else if( i < 2 ){
	    	ret.r = bb;    ret.g = hsv.z; ret.b = aa;
        }else if( i < 3 ){
    		ret.r = aa;    ret.g = hsv.z; ret.b = cc;
        }else if( i < 4 ){
    		ret.r = aa;    ret.g = bb;    ret.b = hsv.z;
        }else if( i < 5 ){
    		ret.r = cc;    ret.g = aa;    ret.b = hsv.z;
        }else{
    		ret.r = hsv.z; ret.g = aa;    ret.b = bb;
        }
    }
	return ret;
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
cbuffer cbCalcLuminanceLog : register(b0){
	float4 g_avSampleOffsets[MAX_SAMPLES];
};
/*!
 *@brief	���R�ΐ����Ƃ���P�x�̑ΐ����ς����߂�B
 */
float4 PSCalcLuminanceLogAvarage(PSInput In) : SV_Target0
{
	float3 vSample = 0.0f;
    float  fLogLumSum = 0.0f;
    // ����Ȃ��X�e�N�Z���Ȃ񂾂낤�B�����Ȋ����E�E�E�B
    for(int iSample = 0; iSample < 9; iSample++)
    {

        vSample = max( sceneTexture.Sample(Sampler, In.uv+g_avSampleOffsets[iSample].xy), 0.001f );
        float v = Rgb2V( vSample );
        // ���R�ΐ����Ƃ���P�x�̑ΐ������Z����B
        fLogLumSum += log(v);
    }
    // 9�ŏ��Z���ĕ��ς����߂�B
    fLogLumSum /= 9;

    return float4(fLogLumSum, fLogLumSum, fLogLumSum, 1.0f);
}
////////////////////////////////////////////////////////
// �P�x�̕��ς����߂�B
////////////////////////////////////////////////////////
/*!
 *@brief	���ϋP�x�v�Z�s�N�Z���V�F�[�_�[�B
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

/////////////////////////////////////////////////////////
// �w���֐����g�p���ĕ��ϋP�x�����߂�
/////////////////////////////////////////////////////////
/*!
 *@brief	���R�ΐ����Ƃ���ΐ�����A�P�x�ɕ�������B
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

Texture2D<float4> lumAvgTexture : register(t1);		        //���ϋP�x


float ACESFilm(float x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

/*!
 *@brief	���ϋP�x����g�[���}�b�v���s���s�N�Z���V�F�[�_�[�B
 */
float4 PSFinal( PSInput In) : SV_Target0
{
	float4 vSample = sceneTexture.Sample(Sampler, In.uv );
    float3 hsv = Rgb2Hsv(vSample.xyz);

	float fAvgLum = lumAvgTexture.Sample(Sampler, float2( 0.5f, 0.5f)).r;
    
    // �I���l���v�Z����B
    // ���ϋP�x��0.18�ɂ��邽�߂̃X�P�[���l�����߂�B
    float k = ( 0.18f / ( max(fAvgLum, 0.001f )));
    // �X�P�[�������̃s�N�Z���̋P�x�Ɋ|���Z����B
    hsv.z *= k;
    // ���̃s�N�Z���̋P�x���g�[���}�b�v����B
    // 
    // hsv.z = ACESFilm(hsv.z);
    // Reinhard�֐��B
    hsv.z = ( hsv.z / (hsv.z + 1.0f) ) * (1 + hsv.z / 4.0f);

    // HSV��RGB�ɕϊ����ďo��
    float4 color;
    color.xyz = Hsv2Rgb(hsv);
    color.w= 1.0f;
    
    // �K���}�␳
    color = pow( max( color, 0.0001f), 1.0f / 2.2f );
	return color;
}