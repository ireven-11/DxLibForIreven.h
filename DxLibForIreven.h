#pragma once
#include"DxLib.h"
#include<math.h>

/// <summary>
/// dxlib設定
/// </summary>
/// <param name="screenWidht">スクリーンの幅</param>
/// <param name="screenHeight">スクリーンの高さ</param>
void DxlibSetting(unsigned int screenWidht = 1920, unsigned int screenHeight = 1080)
{
    SetGraphMode(screenWidht, screenHeight, 32);//ウィンドウのサイズとカラーモードを決める
    ChangeWindowMode(TRUE);						//ウィンドウモードにする
    SetWindowStyleMode(7);						//最大化ボタンが存在するウインドウモードに変更

    if (DxLib_Init() == -1)return;

    // サイズ変更を可能にする
    SetWindowSizeChangeEnableFlag(TRUE, FALSE);

    // ウインドウサイズはゲーム画面と一致させる
    SetWindowSize(screenWidht, screenHeight);

    SetMainWindowText("gamename");              //ウィンドウ（白いところ）にゲーム名を書く
    SetDrawScreen(DX_SCREEN_BACK);		        //背景をセットする
}

/// <summary>
/// 背景透過動画再生関数
/// </summary>
/// <param name="movieHandle">動画ハンドル</param>
/// <param name="screenHandle">スクリーンハンドル※MakeScreen関数でハンドルを作ってその時に第三引数をTRUEにする必要がある</param>
/// <param name="movieBackColorType">動画背景色のタイプ(0:黒, 1:緑, 2:白を指定する)</param>
/// <param name="movieWidht">動画幅</param>
/// <param name="movieHeght">動画高さ</param>
/// <param name="isLoop">動画をループするか</param>
/// <param name="screenType">最終的に描画するとこのスクリーンハンドル</param>
void PlayTransparentMovie(int movieHandle, int screenHandle, unsigned short movieBackColorType = 0, int movieWidht = 1920, int movieHeight = 1080,
    VECTOR position = VGet(0.0f, 0.0f, 0.0f), bool isLoop = true, int screenType = DX_SCREEN_BACK)
{
    //スクリーンハンドルに動画を描画する
    SetDrawScreen(screenHandle);
    if (isLoop)//ループ再生するかどうか
    {
        PlayMovieToGraph(movieHandle, DX_PLAYTYPE_LOOP);
    }
    else
    {
        PlayMovieToGraph(movieHandle);
    }
    DrawExtendGraph(position.x, position.y, position.x + movieWidht, position.y + movieHeight, movieHandle, TRUE);

    //元のスクリーンハンドルに戻す
    SetDrawScreen(screenType);

    //スクリーンハンドルを画像として透過してから描画
    //背景の色によって透過する色を変える
    if (movieBackColorType == 0)
    {
        GraphFilter(screenHandle, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 128, TRUE, GetColor(0, 255, 0), 0);
    }
    else if (movieBackColorType == 1)
    {
        GraphFilter(screenHandle, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 128, TRUE, GetColor(0, 0, 0), 0);
    }
    else
    {
        GraphFilter(screenHandle, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 128, TRUE, GetColor(0, 255, 0), 0);
    }
    DrawExtendGraph(position.x, position.y, position.x + movieWidht, position.y + movieHeight, screenHandle, TRUE);
}

/// <summary>
/// 距離計算関数
/// </summary>
/// <param name="movieHandle">座標1</param>
/// <param name="screenHandle">座標2</param>
template<typename T>
T CalculateDistance(VECTOR position1, VECTOR position2 = VGet(0.0f, 0.0f, 0.0f))
{
    //座標１から座標２までの距離を計算して返す
    VECTOR tempVector   = VGet(position1.x, position1.y, position1.z);
    tempVector          = VSub(tempVector, position2);
    tempVector          = VGet(tempVector.x * tempVector.x, tempVector.y * tempVector.y, tempVector.z * tempVector.z);
    T value             = tempVector.x + tempVector.y + tempVector.z;
    return sqrt(value);
}

/// <summary>
/// 画像アニメーション描画
/// </summary>
/// <param name="position">描画座標</param>
/// <param name="graphHandle">画像ハンドル(分割読み込みしたもの)</param>
/// <param name="graphWidth">画像の幅(1分割分)</param>
/// <param name="graphHeight">画像の高さ(1分割分)</param>
/// <param name="tilSwitchTime">画像が切り替わるまでの時間</param>
/// <param name="finishGraphNumber">最後のアニメーション画像が何枚目かを表す数字</param>
/// <param name="startGraphNumber">最初のアニメーション画像が何枚目かを表す数字</param>
void DrawAnimationGraph(VECTOR position, int graphHandle[], int graphWidth, int graphHeight,
    unsigned int tilSwitchTime, unsigned int finishGraphNumber, unsigned int startGraphNumber = 0)
{
    //切り替えまでカウントを増やして待つ
    static int waitCount;
    static int animationCount = startGraphNumber;
    ++waitCount;

    //別種のアニメーションへの切り替えを感知してカウントを０に戻す
    static int checkChangeAnim = startGraphNumber;
    if (checkChangeAnim != startGraphNumber)
    {
        waitCount       = 0;
        animationCount  = startGraphNumber;
        checkChangeAnim = startGraphNumber;
    }

    if (waitCount >= tilSwitchTime)
    {
        //アニメーションカウントを増やす
        ++animationCount;
        waitCount = 0;

        //アニメーションが最後までいったら最初に戻す
        if (animationCount > finishGraphNumber)
        {
            animationCount = startGraphNumber;
        }
    }
    DrawExtendGraph(position.x, position.y, position.x + graphWidth, position.y + graphHeight, graphHandle[animationCount], TRUE);
}

/// <summary>
/// 文字点滅描画関数(フォント指定可能)
/// </summary>
/// <param name="position">描画座標</param>
/// <param name="text">描画したいテキスト</param>
/// <param name="color">文字の色</param>
/// <param name="fontHandle">フォントハンドル</param>
/// <param name="brinkSpeed">点滅スピード(デフォルトは2、0を入れると点滅しない)</param>
void DrawBrinkStringToHandle(VECTOR position, const char* text, int color, int fontHandle = 0, unsigned short brinkSpeed = 2)
{
    //点滅スピードが0だったら点滅しない
    if (brinkSpeed == 0)
    {
        DrawStringToHandle(position.x, position.y, text, color, fontHandle);
    }
    else
    {
        //点滅カウント
        static int brinkCount;
        brinkCount += 1 * brinkSpeed;
        if (brinkCount > 100)
        {
            brinkCount = 0;
        }

        //点滅カウントが一定値以下のときだけ描画する
        if (brinkCount > 100 / brinkSpeed)
        {
            DrawStringToHandle(position.x, position.y, text, color, fontHandle);
        }
    }
}

/// <summary>
/// 画像点滅描画関数
/// </summary>
/// <param name="position">描画座標</param>
/// <param name="widht">画像の幅</param>
/// <param name="height">画像の高さ</param>
/// <param name="graphHandle">画像ハンドル</param>
/// <param name="brinkSpeed">点滅スピード(デフォルトは2、0を入れると点滅しない)</param>
/// <param name="isTrans">画像透過を許すかどうか</param>
void DrawExtendBrinkGraph(VECTOR position, float widht, float height, int graphHandle,  unsigned short brinkSpeed = 2, bool isTrans = true)
{
    //点滅スピードが0だったら点滅しない
    if (brinkSpeed == 0)
    {
        DrawExtendGraph(position.x, position.y, position.x + widht, position.y + height, graphHandle, isTrans);
    }
    else
    {
        //点滅カウント
        static int brinkCount;
        brinkCount += 1 * brinkSpeed;
        if (brinkCount > 100)
        {
            brinkCount = 0;
        }

        //点滅カウントが一定値以下のときだけ描画する
        if (brinkCount > 100 / brinkSpeed)
        {
            DrawExtendGraph(position.x, position.y, position.x + widht, position.y + height, graphHandle, isTrans);
        }
    }
}