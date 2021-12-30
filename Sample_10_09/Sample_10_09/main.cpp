#include "stdafx.h"
#include "system/system.h"
#include "sub.h"
#include "bloom.h"
#include "ShadowMap.h"
#include "Tonemap.h"


///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // ゲームの初期化
    InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));

    //////////////////////////////////////
    // ここから初期化を行うコードを記述する
    //////////////////////////////////////
    g_camera3D->SetFar(50000.0f);
    g_camera3D->Update();

    // メインレンダリングターゲットを作成。
    RenderTarget mainRenderTarget;
    mainRenderTarget.Create(
        1280,
        720,
        1,
        1,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_D32_FLOAT
    );

    // シャドウマップを初期化。
    ShadowMap shadowMap;
    shadowMap.Init();

    // ライトを初期化。
    Light light;
    InitLight(light);
    
    // 通常描画の背景モデルと、シャドウマップ描画用の背景モデル(シャドウキャスターを初期化。
    SModelExCB modelExCb;
    Model bgModel, bgShadowCaster;
    InitBackgourndModel(bgModel, bgShadowCaster, light, shadowMap, modelExCb);

    // 空モデルを初期化。
    Model skyModel;
    InitSkyModel(skyModel);

    // ブルームを初期化。
    Bloom bloom;
    bloom.Init(mainRenderTarget);

    // トーンマップを初期化。
    Tonemap tonemap;
    tonemap.Init(mainRenderTarget);
    // メインレンダリングターゲットの内容をフレームバッファにコピーするためのスプライトを初期化
    Sprite copyToFrameBufferSprite;
    InitCopyMainRenderTargetToFrameBufferSprite(copyToFrameBufferSprite, mainRenderTarget);

     
    //////////////////////////////////////
    // 初期化を行うコードを書くのはここまで！！！
    //////////////////////////////////////
    auto& renderContext = g_graphicsEngine->GetRenderContext();

    // ここからゲームループ
    while (DispatchWindowMessage())
    {
        // 1フレームの開始
        g_engine->BeginFrame();
        
        // カメラを動かす。
        MoveCamera();
        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////
        
        // モデルの拡張定数バッファの内容を更新。
        modelExCb.light = light;
        modelExCb.lvpMatrix = shadowMap.GetLVPMatrix();
        // シャドウマップ作成。
        shadowMap.Draw(renderContext);

        // メインレンダリンターゲットにシーンを描画
        DrawSceneToMainRenderTarget(bgModel, skyModel, mainRenderTarget, renderContext);

        // ここからポストエフェクト
        
        tonemap.Execute(renderContext, mainRenderTarget);

        // ブルームを実行        
        bloom.Draw(renderContext, mainRenderTarget);

        // メインレンダリングターゲットの絵をフレームバッファーにコピー
        renderContext.SetRenderTarget(
            g_graphicsEngine->GetCurrentFrameBuffuerRTV(),
            g_graphicsEngine->GetCurrentFrameBuffuerDSV()
        );
        copyToFrameBufferSprite.Draw(renderContext);

        //////////////////////////////////////
        // 絵を描くコードを書くのはここまで！！！
        //////////////////////////////////////

        // 1フレーム終了
        g_engine->EndFrame();
    }
    return 0;
}

