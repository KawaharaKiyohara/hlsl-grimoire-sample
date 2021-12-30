#include "stdafx.h"
#include "sub.h"

void InitMainDepthRenderTarget(RenderTarget& mainRenderTarget, RenderTarget& depthRenderTarget)
{
    //メインレンダリングターゲットと深度レンダリングターゲットを作成。
   //シーンのカラーを描きこむメインレンダリングターゲットを作成。
    mainRenderTarget.Create(
        1280,
        720,
        1,
        1,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_D32_FLOAT
    );
    //シーンのカメラ空間でのZ値を書きこむレンダリングターゲットを作成。
    float clearColor[4] = { 10000.0f, 10000.0f, 10000.0f, 1.0f };
    depthRenderTarget.Create(
        1280,
        720,
        1,
        1,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN,
        clearColor
    );
}

/// <summary>
/// 背景モデルを初期化。
/// </summary>
void InitBackgourndModel(Model& model, Model& shadowCasterModel, Light& light, ShadowMap& shadowMap, SModelExCB& modelExCb)
{
    {
        // モデルの初期化情報を設定する
        ModelInitData initData;
        // tkmファイルを指定する
        initData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";
        // シェーダーファイルを指定する
        initData.m_fxFilePath = "Assets/shader/preset/model.fx";
        // ユーザー拡張の定数バッファーに送るデータを指定する
        initData.m_expandConstantBuffer = &modelExCb;
        // ユーザー拡張の定数バッファーに送るデータのサイズを指定する
        initData.m_expandConstantBufferSize = sizeof(modelExCb);
        // レンダリングするカラーバッファーのフォーマットを指定する
        initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        // 拡張シェーダーリソースとして、シャドウマップを渡す。
        initData.m_expandShaderResoruceView[0] = &shadowMap.GetShadowMap();

        // 設定した初期化情報をもとにモデルを初期化する
        model.Init(initData);
    }
    {
        // 続いてシャドウキャスター
        ModelInitData initData;
        initData.m_tkmFilePath = "Assets/modelData/bg/bg.tkm";
        // シェーダーファイルを指定する
        initData.m_fxFilePath = "Assets/shader/preset/shadowCaster.fx";
        // レンダリングするカラーバッファーのフォーマットを指定する
        initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
        // 設定した初期化情報をもとにモデルを初期化する
        shadowCasterModel.Init(initData);

        shadowMap.RegisterShadowCaster(shadowCasterModel);
    }
}
/// <summary>
/// 空モデルを初期化
/// </summary>
/// <param name=""></param>
void InitSkyModel(Model& skyModel)
{
    ModelInitData initData;
    initData.m_tkmFilePath = "Assets/modelData/sky.tkm";
    initData.m_fxFilePath = "Assets/shader/preset/sky.fx";
    initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    skyModel.Init(initData);
    skyModel.UpdateWorldMatrix(g_vec3Zero, g_quatIdentity, { 500.0f, 500.0f, 500.0f });

}
/// <summary>
/// ライトを初期化
/// </summary>
/// <param name="light"></param>
void InitLight(Light& light)
{
    // 光を強めに設定する
    light.directionalLight[0].color.x = 50.8f;
    light.directionalLight[0].color.y = 50.8f;
    light.directionalLight[0].color.z = 50.8f;

    light.directionalLight[0].direction.x = -1.0f;
    light.directionalLight[0].direction.y = -1.0f;
    light.directionalLight[0].direction.z = -1.0f;
    light.directionalLight[0].direction.Normalize();

    light.ambinetLight.x = 0.5f;
    light.ambinetLight.y = 0.5f;
    light.ambinetLight.z = 0.5f;
    light.eyePos = g_camera3D->GetPosition();
}
/// <summary>
/// メインレンダリングターゲットの内容をフレームバッファにコピーするためのスプライトを初期化。
/// </summary>
/// <param name="copySprite"></param>
void InitCopyMainRenderTargetToFrameBufferSprite(Sprite& copySprite, RenderTarget& mainRenderTarget)
{
    SpriteInitData spriteInitData;
    // テクスチャはmainRenderTargetのカラーバッファー
    spriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
    spriteInitData.m_width = 1280;
    spriteInitData.m_height = 720;
    // モノクロ用のシェーダーを指定する
    spriteInitData.m_fxFilePath = "Assets/shader/preset/sprite.fx";
    // 初期化オブジェクトを使って、スプライトを初期化する
    copySprite.Init(spriteInitData);
}
void DrawSceneToMainRenderTarget(Model& bgModel, Model& skyModel, RenderTarget& mainRenderTarget, RenderContext& renderContext)
{
    // レンダリングターゲットとして利用できるまで待つ
    renderContext.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
    // メインレンダリングターゲットをカレントレンダリングターゲットとして設定。
    renderContext.SetRenderTargetAndViewport(mainRenderTarget);
    // レンダリングターゲットをクリア
    renderContext.ClearRenderTargetView(mainRenderTarget);

    // メインレンダリングターゲットに背景を描画する
    bgModel.Draw(renderContext);
    // 空を描画。
    skyModel.Draw(renderContext);

    // レンダリングターゲットへの書き込み終了待ち
    renderContext.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
}
void MoveCamera()
{
    g_camera3D->MoveForward(g_pad[0]->GetLStickYF() * 3.0f);
    g_camera3D->RotateY(g_pad[0]->GetLStickXF() * 0.03f);
}