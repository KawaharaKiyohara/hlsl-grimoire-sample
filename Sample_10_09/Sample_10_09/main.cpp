#include "stdafx.h"
#include "system/system.h"
#include "sub.h"
#include "bloom.h"
#include "ShadowMap.h"

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
    // ルートシグネチャの初期化
    RootSignature rs;
    InitRootSignature(rs);

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
    
    // 背景モデルを初期化。
    Model bgModel, bgShadowCaster;
    InitBackgourndModel(bgModel, bgShadowCaster, light, shadowMap);
    // 空モデルを初期化。
    Model skyModel;
    InitSkyModel(skyModel);

    // ブルームを初期化。
    Bloom bloom;
    bloom.Init(mainRenderTarget);

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

        //////////////////////////////////////
        // ここから絵を描くコードを記述する
        //////////////////////////////////////
        
        // シャドウマップ作成。
        shadowMap.Draw(renderContext);

        // メインれだリングターゲットにシーンを描画
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

        // ここからポストエフェクト
        // 
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

