﻿#pragma once

#include "ShadowMap.h"

const int NUM_DIRECTIONAL_LIGHT = 4; // ディレクションライトの数

/// <summary>
/// ディレクションライト
/// </summary>
struct DirectionalLight
{
    Vector3 direction;  // ライトの方向
    float pad0;         // パディング
    Vector4 color;      // ライトのカラー
};

/// <summary>
/// ライト構造体
/// </summary>
struct Light
{
    DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT]; // ディレクションライト
    Vector3 eyePos;         // カメラの位置
    float specPow;          // スペキュラの絞り
    Vector3 ambinetLight;   // 環境光
};

struct SModelExCB {
    Light light;            // ライト。
    float pad;
    Matrix lvpMatrix;   // ライトビュープロジェクション行列。
};

const int NUM_WEIGHTS = 8;

/// <summary>
/// ブラー用のパラメータ
/// </summary>
struct SBlurParam
{
    float weights[NUM_WEIGHTS];
};

// 関数宣言
void InitLight(Light& light);
void InitBackgourndModel(Model& model, Model& shadowCasterModel, Light& light, ShadowMap& shadowMap, SModelExCB& modelExCb);
void InitSkyModel(Model& skyModel);
void InitCopyMainRenderTargetToFrameBufferSprite(Sprite& copySprite, RenderTarget& mainRenderTarget);
void DrawSceneToMainRenderTarget(Model& bgModel, Model& skyModel, RenderTarget& mainRenderTarget, RenderContext& renderContext);
void MoveCamera();