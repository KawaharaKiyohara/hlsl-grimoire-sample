#include "stdafx.h"
#include "bloom.h"

void Bloom::Init(RenderTarget& mainRenderTarget)
{
    // 輝度抽出用のレンダリングターゲットを作成。
    m_luminanceRenderTarget.Create(
        1280,   // 解像度はメインレンダリングターゲットと同じ
        720,    // 解像度はメインレンダリングターゲットと同じ
        1,
        1,
        // 【注目】カラーバッファーのフォーマットを32bit浮動小数点にしている
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_D32_FLOAT
    );
    // 輝度抽出用のスプライトを初期化
    // 初期化情報を作成する。
    SpriteInitData luminanceSpriteInitData;
    // 輝度抽出用のシェーダーのファイルパスを指定する
    luminanceSpriteInitData.m_fxFilePath = "Assets/shader/preset/bloom.fx";
    // 頂点シェーダーのエントリーポイントを指定する
    luminanceSpriteInitData.m_vsEntryPointFunc = "VSMain";
    // ピクセルシェーダーのエントリーポイントを指定する
    luminanceSpriteInitData.m_psEntryPoinFunc = "PSSamplingLuminance";
    // スプライトの幅と高さはluminnceRenderTargetと同じ
    luminanceSpriteInitData.m_width = 1280;
    luminanceSpriteInitData.m_height = 720;
    // テクスチャはメインレンダリングターゲットのカラーバッファー
    luminanceSpriteInitData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
    // 描き込むレンダリングターゲットのフォーマットを指定する
    luminanceSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    // 作成した初期化情報をもとにスプライトを初期化する
    m_luminanceSprite.Init(luminanceSpriteInitData);

    // ブラー処理を初期化する
    // gaussianBlur[0]は輝度テクスチャにガウシアンブラーをかける
    m_blur[0].Init(&m_luminanceRenderTarget.GetRenderTargetTexture());
    // gaussianBlur[1]はgaussianBlur[0]のテクスチャにガウシアンブラーをかける
    m_blur[1].Init(&m_blur[0].GetBokeTexture());
    // gaussianBlur[2]はgaussianBlur[1]のテクスチャにガウシアンブラーをかける
    m_blur[2].Init(&m_blur[1].GetBokeTexture());
    // gaussianBlur[3]はgaussianBlur[2]のテクスチャにガウシアンブラーをかける
    m_blur[3].Init(&m_blur[2].GetBokeTexture());

    // ボケ画像を合成して書き込むためのスプライトを初期化
    // 初期化情報を設定する。
    SpriteInitData finalSpriteInitData;
    // ボケテクスチャを4枚指定する
    finalSpriteInitData.m_textures[0] = &m_blur[0].GetBokeTexture();
    finalSpriteInitData.m_textures[1] = &m_blur[1].GetBokeTexture();
    finalSpriteInitData.m_textures[2] = &m_blur[2].GetBokeTexture();
    finalSpriteInitData.m_textures[3] = &m_blur[3].GetBokeTexture();
    // 解像度はmainRenderTargetの幅と高さ
    finalSpriteInitData.m_width = 1280;
    finalSpriteInitData.m_height = 720;
    // ぼかした画像を、通常の2Dとしてメインレンダリングターゲットに描画するので、
    // 2D用のシェーダーを使用する
    finalSpriteInitData.m_fxFilePath = "Assets/shader/preset/bloom.fx";
    finalSpriteInitData.m_psEntryPoinFunc = "PSBloomFinal";

    // ただし、加算合成で描画するので、アルファブレンディングモードを加算にする
    finalSpriteInitData.m_alphaBlendMode = AlphaBlendMode_Add;
    // カラーバッファーのフォーマットは例によって、32ビット浮動小数点バッファー
    finalSpriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    // 初期化情報を元に加算合成用のスプライトを初期化する
    m_finalSprite.Init(finalSpriteInitData);
}

void Bloom::Draw(RenderContext& rc, RenderTarget& mainRenderTarget)
{
    // 輝度抽出
        // 輝度抽出用のレンダリングターゲットに変更
    rc.WaitUntilToPossibleSetRenderTarget(m_luminanceRenderTarget);
    // レンダリングターゲットを設定
    rc.SetRenderTargetAndViewport(m_luminanceRenderTarget);
    // レンダリングターゲットをクリア
    rc.ClearRenderTargetView(m_luminanceRenderTarget);
    // 輝度抽出を行う
    m_luminanceSprite.Draw(rc);
    // レンダリングターゲットへの書き込み終了待ち
    rc.WaitUntilFinishDrawingToRenderTarget(m_luminanceRenderTarget);

    // step-3 ガウシアンブラーを4回実行する
    m_blur[0].ExecuteOnGPU(rc, 10);
    m_blur[1].ExecuteOnGPU(rc, 10);
    m_blur[2].ExecuteOnGPU(rc, 10);
    m_blur[3].ExecuteOnGPU(rc, 10);

    // step-4 ボケ画像を合成してメインレンダリングターゲットに加算合成
    // レンダリングターゲットとして利用できるまで待つ
    rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
    // レンダリングターゲットを設定
    rc.SetRenderTargetAndViewport(mainRenderTarget);
    // 最終合成
    m_finalSprite.Draw(rc);
    // レンダリングターゲットへの書き込み終了待ち
    rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
}
