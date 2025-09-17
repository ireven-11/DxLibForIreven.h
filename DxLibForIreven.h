#pragma once
#include"DxLib.h"
#include<math.h>

/// <summary>
/// dxlib設定
/// </summary>
/// <param name="screenWidht">スクリーン幅</param>
/// <param name="screenHeight">スクリーン高さ</param>
void DxlibSetting(unsigned int screenWidht = 1920, unsigned int screenHeight = 1080)
{
    SetGraphMode(screenWidht, screenHeight, 32);//ウィンドウのサイズとカラーモードを決める
    ChangeWindowMode(TRUE);						//ウィンドウモードにする
    SetWindowStyleMode(7);						//最大化ボタンが存在するウインドウモードに変更

    //dxlib初期化
    if (DxLib_Init() == -1)return;

    //サイズ変更を可能にする
    SetWindowSizeChangeEnableFlag(TRUE, FALSE);

    //ウインドウサイズはゲーム画面と一致させる
    SetWindowSize(screenWidht, screenHeight);

    SetMainWindowText("gamename");              //ウィンドウ（白いところ）にゲーム名を書く
    SetDrawScreen(DX_SCREEN_BACK);		        //背景をセットする
}

/// <summary>
/// 背景透過動画再生関数
/// </summary>
/// <param name="movieHandle">動画ハンドル</param>
/// <param name="screenHandle">スクリーンハンドル※MakeScreen関数でハンドルを作ってその時に第三引数をTRUEにする必要がある</param>
/// <param name="movieBackColorType">動画背景色のタイプ(0:黒, 1:白を指定する)</param>
/// <param name="isLoop">動画をループするか</param>
/// <param name="movieWidht">動画幅</param>
/// <param name="movieHeght">動画高さ</param>
/// <param name="moviePosition">動画座標</param>
/// <param name="originScreenHandle">元々のスクリーンハンドル</param>
bool PlayTransparentMovie(int movieHandle, int screenHandle, unsigned char movieBackColorType = 0, bool isLoop = true, int movieWidht = 1920, int movieHeight = 1080,
    VECTOR moviePosition = VGet(0.0f, 0.0f, 0.0f), int originScreenHandle = DX_SCREEN_BACK)
{
    //スクリーンハンドルに動画を描画する
    SetDrawScreen(screenHandle);

    //ループ再生するかどうか
    if (isLoop)
    {
        PlayMovieToGraph(movieHandle, DX_PLAYTYPE_LOOP);
    }
    else
    {
        PlayMovieToGraph(movieHandle);
    }
    DrawExtendGraph(moviePosition.x, moviePosition.y, moviePosition.x + movieWidht, moviePosition.y + movieHeight, movieHandle, TRUE);

    //元のスクリーンハンドルに戻す
    SetDrawScreen(originScreenHandle);

    //スクリーンハンドルを画像として透過してから描画
    //背景の色によって透過する色を変える
    if (movieBackColorType == 0)//黒
    {
        GraphFilter(screenHandle, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 10, TRUE, GetColor(0, 255, 0), 0);
    }
    else//白
    {
        GraphFilter(screenHandle, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_GREATER, 245, TRUE, GetColor(0, 255, 0), 0);
    }
    DrawExtendGraph(moviePosition.x, moviePosition.y, moviePosition.x + movieWidht, moviePosition.y + movieHeight, screenHandle, TRUE);

    //動画が再生してるかを返す
    if (GetMovieStateToGraph(movieHandle))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/// <summary>
/// 距離計算関数
/// </summary>
/// <param name="movieHandle">座標1</param>
/// <param name="screenHandle">座標2</param>
template<typename T>
T CalculateDistance(VECTOR position1, VECTOR position2 = VGet(0.0f, 0.0f, 0.0f))
{
    //2点間の距離を計算する
    VECTOR tempVector = VGet(position1.x, position1.y, position1.z);
    tempVector = VSub(tempVector, position2);
    tempVector = VGet(tempVector.x * tempVector.x, tempVector.y * tempVector.y, tempVector.z * tempVector.z);
    T value = tempVector.x + tempVector.y + tempVector.z;
    return sqrt(value);
}

/// <summary>
/// 画像アニメーション描画関数
/// </summary>
/// <param name="position">座標</param>
/// <param name="graphHandle">画像ハンドル</param>
/// <param name="graphWidth">画像幅</param>
/// <param name="graphHeight">画像高さ</param>
/// <param name="tilSwitchTime">アニメーションが切り替わるまでの時間</param>
/// <param name="finishGraphNumber">最後の画像がアニメーションの何番目かを表す数値</param>
/// <param name="startGraphNumber">最初の画像がアニメーションの何番目かを表す数値</param>
void DrawAnimationGraph(VECTOR position, int graphHandle[], int graphWidth, int graphHeight,
    unsigned int tilSwitchTime, unsigned int finishGraphNumber, unsigned int startGraphNumber = 0)
{
    //待ちカウントとアニメーションカウントを定義
    static int waitCount;
    static int animationCount = startGraphNumber;

    //待ちカウントを進める
    ++waitCount;

    //アニメーションする画像が切り替わった時にアニメーションを最初からにする
    static int checkChangeAnim = startGraphNumber;
    if (checkChangeAnim != startGraphNumber)
    {
        waitCount = 0;
        animationCount = startGraphNumber;
        checkChangeAnim = startGraphNumber;
    }

    if (waitCount >= tilSwitchTime)
    {
        //アニメーションが切り替わるまでの時間を超えたらアニメーションを進める
        ++animationCount;
        waitCount = 0;

        //アニメーションが最後まで行ったら最初に戻す
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
/// 画像点滅描画関数(拡大縮小可能)
/// </summary>
/// <param name="position">描画座標</param>
/// <param name="widht">画像幅</param>
/// <param name="height">画像高さ</param>
/// <param name="graphHandle">画像反動</param>
/// <param name="brinkSpeed">点滅スピード(デフォルトは2、0を入れると点滅しない)</param>
/// <param name="isTrans">画像の透過処理を許すか</param>
void DrawExtendBrinkGraph(VECTOR position, float widht, float height, int graphHandle, unsigned short brinkSpeed = 2, bool isTrans = true)
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