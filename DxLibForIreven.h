#pragma once
#include"DxLib.h"
#include<math.h>
#include<cassert>
#include <windows.h>
#include<vector>

#define winmainSetting	_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int	//winmainの引数
#define gameRoopSetting ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0	//ゲームループの引数(ゲームループの最初にClearDrawScreen()を最後にScreenFlip()を呼ぶ必要がある)

/// <summary>
/// dxlib初期化
/// </summary>
/// <param name="screenWidht">スクリーン幅</param>
/// <param name="screenHeight">スクリーン高さ</param>
inline void InitDxLib(unsigned int screenWidht = 1920, unsigned int screenHeight = 1080)
{
    ChangeWindowMode(FALSE);					//ウィンドウモードにする
    SetWindowStyleMode(7);						//最大化ボタンが存在するウインドウモードに変更

    if (DxLib_Init() == -1)return;

    // サイズ変更を可能にする
    SetWindowSizeChangeEnableFlag(TRUE, FALSE);

    //pc自体の画面解像度をdxlibのスクリーンサイズに合うように変更
    DEVMODE pcScreenInfo;	//DEVMODEはpcのスクリーン情報を格納する構造体（windowsAPI）
    pcScreenInfo.dmPelsWidth = screenWIDTH;
    pcScreenInfo.dmPelsHeight = screenHEIGHT;
    ChangeDisplaySettings(&pcScreenInfo, CDS_FULLSCREEN);	//第二引数を「CDS_FULLSCREEN」にするとアプリ終了時に元の解像度に戻る

    //ウィンドウのサイズとカラーモードを決める
    SetGraphMode(screenWIDTH, screenHEIGHT, 32);

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
inline bool PlayTransparentMovie(int movieHandle, int screenHandle, unsigned char movieBackColorType = 0, bool isLoop = true, int movieWidht = 1920, int movieHeight = 1080,
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
    DrawExtendGraphF(moviePosition.x, moviePosition.y, moviePosition.x + movieWidht, moviePosition.y + movieHeight, movieHandle, TRUE);

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
    DrawExtendGraphF(moviePosition.x, moviePosition.y, moviePosition.x + movieWidht, moviePosition.y + movieHeight, screenHandle, TRUE);

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
inline T CalculateDistance(VECTOR position1, VECTOR position2 = VGet(0.0f, 0.0f, 0.0f))
{
    //2点間の距離を計算する
    VECTOR tempVector = VGet(position1.x, position1.y, position1.z);
    tempVector = VSub(tempVector, position2);
    tempVector = VGet(tempVector.x * tempVector.x, tempVector.y * tempVector.y, tempVector.z * tempVector.z);
    T value = tempVector.x + tempVector.y + tempVector.z;
    return static_cast<T>(sqrt(value));
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
inline void DrawAnimationGraph(VECTOR position, int graphHandle[], int graphWidth, int graphHeight,
    unsigned int tilSwitchTime, unsigned int finishGraphNumber, unsigned int startGraphNumber = 0)
{
    //待ちカウントとアニメーションカウントを定義
    static unsigned int waitCount;
    static unsigned int animationCount = startGraphNumber;

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
    DrawExtendGraphF(position.x, position.y, position.x + graphWidth, position.y + graphHeight, graphHandle[animationCount], TRUE);
}

/// <summary>
/// 文字点滅描画関数(フォント指定可能)
/// </summary>
/// <param name="position">描画座標</param>
/// <param name="text">描画したいテキスト</param>
/// <param name="color">文字の色</param>
/// <param name="fontHandle">フォントハンドル</param>
/// <param name="brinkSpeed">点滅スピード(デフォルトは2、0を入れると点滅しない)</param>
inline void DrawBrinkStringToHandle(VECTOR position, const char* text, int color, int fontHandle = 0, unsigned short brinkSpeed = 2)
{
    //点滅スピードが0だったら点滅しない
    if (brinkSpeed == 0)
    {
        DrawStringToHandle(static_cast<int>(position.x), static_cast<int>(position.y), text, color, fontHandle);
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
            DrawStringToHandle(static_cast<int>(position.x), static_cast<int>(position.y), text, color, fontHandle);
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
inline void DrawExtendBrinkGraph(VECTOR position, float widht, float height, int graphHandle, unsigned short brinkSpeed = 2, bool isTrans = true)
{
    //点滅スピードが0だったら点滅しない
    if (brinkSpeed == 0)
    {
        DrawExtendGraphF(position.x, position.y, position.x + widht, position.y + height, graphHandle, isTrans);
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
            DrawExtendGraphF(position.x, position.y, position.x + widht, position.y + height, graphHandle, isTrans);
        }
    }
}

/// <summary>
/// ベクトルの値を行列にセット
/// </summary>
/// <param name="matrix">値をセットする行列</param>
/// <param name="vector">セットしたいベクトル</param>
inline void SetMatrixFromVector(MATRIX& matrix, const VECTOR& vector)
{
    matrix.m[3][0] = vector.x;
    matrix.m[3][1] = vector.y;
    matrix.m[3][2] = vector.z;
}

/// <summary>
/// コントローラー(DirectInput)の傾けてる角度を得る(返り値は0～3.14または0～-3.14)
/// </summary>
/// <param name="controllerNumber">どのコントローラから値を得るかの数値</param>
/// <param name="deadZone">デッドゾーンの数値(0.0～1.0)</param>
/// <returns></returns>
inline double GetDInputStickTiltAngle(const int controllerNumber = DX_INPUT_PAD1, const float deadZone = 0.2f)
{
    //コントローラーのデッドゾーンを設定
    SetJoypadDeadZone(controllerNumber, deadZone);

    //移動方向を決定
    DINPUT_JOYSTATE input;
    //コントローラの入力を検知
    GetJoypadDirectInputState(controllerNumber, &input);

    //スティックの倒れてる数値から角度を求める
    double tiltAngle_ = atan2(static_cast<double>(input.Y), static_cast<double>(input.X));

    return tiltAngle_;
}

/// <summary>
/// モデルボーン回転行列取得関数
/// </summary>
/// <param name="MHandle">モデルハンドル</param>
/// <param name="FrameIndex">回転させたいボーン</param>
/// <param name="Xaxis">x軸の回転させる値</param>
/// <param name="Yaxis">y軸の回転させる値</param>
/// <param name="Zaxis">z軸の回転させる値</param>
inline MATRIX MV1GetFrameRotateMatrix(int MHandle, int FrameIndex, double Xaxis, double Yaxis, double Zaxis)
{
    auto frame_matrix = MV1GetFrameLocalMatrix(MHandle, FrameIndex);
    auto result_matrix = MGetIdent();
    const auto frame_pos = MGetTranslateElem(frame_matrix);

    CreateRotationZXYMatrix(&result_matrix, static_cast<float>(Xaxis), static_cast<float>(Yaxis), static_cast<float>(Zaxis));
    SetMatrixFromVector(result_matrix, frame_pos);

    return result_matrix;
}

/// <summary>
/// 2D用の外積を求める
/// </summary>
/// <param name="a">外積の計算に使う座標1</param>
/// <param name="b">外積の計算に使う座標2</param>
inline float Cross2D(VECTOR a, VECTOR b)
{
    return a.x * b.z - a.z * b.x;
}

/// <summary>
/// 点が三角形ABCの内部にあるか（辺上も含む）
/// </summary>
/// <param name="A">三角形の座標１</param>
/// <param name="B">三角形の座標２</param>
/// <param name="C">三角形の座標３</param>
/// <param name="P">判定したい座標</param>
inline bool HitTriangleAndPixel(VECTOR A, VECTOR B, VECTOR C, VECTOR P)
{
    VECTOR AB = { B.x - A.x, 0, B.z - A.z };
    VECTOR BC = { C.x - B.x, 0, C.z - B.z };
    VECTOR CA = { A.x - C.x, 0, A.z - C.z };

    VECTOR AP = { P.x - A.x, 0, P.z - A.z };
    VECTOR BP = { P.x - B.x, 0, P.z - B.z };
    VECTOR CP = { P.x - C.x, 0, P.z - C.z };

    float c1 = Cross2D(AB, AP);
    float c2 = Cross2D(BC, BP);
    float c3 = Cross2D(CA, CP);

    // すべて同じ符号なら内部（0は辺上）
    if ((c1 >= 0 && c2 >= 0 && c3 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0)) {
        return true;
    }

    return false;
}

/// <summary>
/// 円が三角形ABCの内部にあるか（辺上も含む）
/// </summary>
/// <param name="trianglePos1">三角形の座標１</param>
/// <param name="trianglePos2">三角形の座標２</param>
/// <param name="trianglePos3">三角形の座標３</param>
/// <param name="circlePos">判定したい円の座標</param>
/// <param name="radius">判定したい円の半径</param>
inline bool CheckHitCircleWithTriangle(VECTOR trianglePos1, VECTOR trianglePos2, VECTOR trianglePos3, VECTOR circlePos, float radius)
{
    VECTOR edge;				//線分
    VECTOR edgeStartToCirclePos;//線分の始点から円の座標まで伸びるベクトル
    VECTOR edgeEndToCirclePos;	//線分の終点から円の座標まで伸びるベクトル
    VECTOR crossProduct;		//外積
    float shortestDistance;		//線分との最短距離
    for (auto i = 0; i < 3; i++)
    {
        switch (i)
        {
        case 0:
            edge = VSub(trianglePos2, trianglePos1);
            edgeStartToCirclePos = VSub(circlePos, trianglePos1);
            edgeEndToCirclePos = VSub(circlePos, trianglePos2);
            crossProduct = VCross(edge, edgeStartToCirclePos);
            shortestDistance = CalculateDistance<float>(crossProduct) / CalculateDistance<float>(edge);
            break;
        case 1:
            edge = VSub(trianglePos3, trianglePos2);
            edgeStartToCirclePos = VSub(circlePos, trianglePos2);
            edgeEndToCirclePos = VSub(circlePos, trianglePos3);
            crossProduct = VCross(edge, edgeStartToCirclePos);
            shortestDistance = CalculateDistance<float>(crossProduct) / CalculateDistance<float>(edge);
            break;
        case 2:
            edge = VSub(trianglePos1, trianglePos3);
            edgeStartToCirclePos = VSub(circlePos, trianglePos3);
            edgeEndToCirclePos = VSub(circlePos, trianglePos1);
            crossProduct = VCross(edge, edgeStartToCirclePos);
            shortestDistance = CalculateDistance<float>(crossProduct) / CalculateDistance<float>(edge);
            break;
        }

        //最短距離が半径より小さかったら当たってる可能性がある
        if (shortestDistance <= radius)
        {
            //線分の外に円の中心があるときは角度が両方鈍角だったら当たってる
            if (VDot(edge, edgeStartToCirclePos) * VDot(edge, edgeEndToCirclePos) <= 0)
            {
                return true;
            }

            //半径が円へのベクトルの距離より大きかったら当たってる
            if (radius > CalculateDistance<float>(edgeStartToCirclePos) || radius > CalculateDistance<float>(edgeEndToCirclePos))
            {
                return true;
            }
        }

        //三角形の中に円の中心点があるか
        if (HitTriangleAndPixel(trianglePos1, trianglePos2, trianglePos3, circlePos))
        {
            return true;
        }
    }

    return false;
}

/// <summary>
/// 3d上に六角形を平面描画する関数
/// </summary>
/// <param name="standardPosition">六角形の基準となる座標</param>
/// <param name="sideX">六角形のｘの長さ</param>
/// <param name="sideZ">六角形のｚの長さ</param>
/// <param name="sideZ2">六角形の何でいえばいいんだろ、こう、上と下の三角形の部分あるじゃん。そこまでのｚの長さっていえばいいのか？</param>
/// <param name="color"></param>
/// <param name="fill"></param>
inline void DrawHexagon3D(VECTOR standardPosition, float sideX, float sideZ, float sideZ2, int color, bool fill)
{
    const float width_adjust = 2.0f;
    const float height_adjust = 1.5f;
    DrawTriangle3D(VGet(standardPosition.x - sideX / width_adjust, 0.0f, standardPosition.z + sideZ2 / height_adjust),
        VGet(standardPosition.x + sideX / width_adjust, 0.0f, standardPosition.z + sideZ2 / height_adjust),
        VGet(standardPosition.x, 0.0f, standardPosition.z + sideZ / height_adjust), color, fill);
    DrawTriangle3D(VGet(standardPosition.x - sideX / width_adjust, 0.0f, standardPosition.z + sideZ2 / height_adjust),
        VGet(standardPosition.x + sideX / width_adjust, 0.0f, standardPosition.z + sideZ2 / height_adjust),
        VGet(standardPosition.x + sideX / width_adjust, 0.0f, standardPosition.z - sideZ2 / height_adjust), color, fill);
    DrawTriangle3D(VGet(standardPosition.x - sideX / width_adjust, 0.0f, standardPosition.z + sideZ2 / height_adjust),
        VGet(standardPosition.x - sideX / width_adjust, 0.0f, standardPosition.z - sideZ2 / height_adjust),
        VGet(standardPosition.x + sideX / width_adjust, 0.0f, standardPosition.z - sideZ2 / height_adjust), color, fill);
    DrawTriangle3D(VGet(standardPosition.x - sideX / width_adjust, 0.0f, standardPosition.z - sideZ2 / height_adjust),
        VGet(standardPosition.x + sideX / width_adjust, 0.0f, standardPosition.z - sideZ2 / height_adjust),
        VGet(standardPosition.x, 0.0f, standardPosition.z - sideZ / height_adjust), color, fill);
}

/// <summary>
/// ベクトルをY軸回転する
/// </summary>
/// <param name="vector">回転させるベクトル</param>
/// <param name="rotationAngle">回転角度</param>
/// <returns></returns>
inline VECTOR RotationVectorY(VECTOR vector, double rotationAngle)
{
    //角度法だとエラーが出るようにする
    assert((-DX_PI_F < rotationAngle && rotationAngle < DX_PI_F) && "回転角度が弧度法（ラジアン）になってないです！これだとベクトルがよくわからんぐらい回転します！");

    //回転
    MATRIX rotationMatrix = MGetRotY(static_cast<float>(rotationAngle));
    vector = VTransform(vector, rotationMatrix);
    return vector;
}

/// <summary>
/// 座標を上下移動させる
/// </summary>
/// <param name="positionY">移動させるy座標</param>
/// <param name="amplitude">上下移動の振れ幅</param>
/// <param name="period">上下移動の間隔</param>
/// <returns></returns>
inline float UpDownPositionY(float positionY, const float amplitude = 1.0f, const float period = 1.0f)
{
    //上下移動失敗
    if (period == 0.0f) return -1.0f;

    //角度法で処理する
    static	float rad = 0.0f;
    const	float add_rad = 10.0f;
    rad += add_rad;
    if (rad > 360.0f)
    {
        rad = 0.0f;
    }

    //sinfで移動
    positionY += sinf(rad / period) * amplitude;
    return positionY;
}

/// <summary>
/// セミオート画像分割読み込み関数
/// </summary>
/// <param name="graphPath">画像パス</param>
/// <param name="divNumberX">横分割数</param>
/// <param name="divNumberY">縦分割数</param>
inline std::vector<int> semiAutoLoadDivGraph(const char* graphPath, int divNumberX, int divNumberY)
{
    //画像ハンドル
    std::vector<int> graphHandle;

    int divGraphAllNumber = divNumberX * divNumberY;//画像分割総数
    int xSize, ySize;	//画像サイズ用変数

    //画像のサイズを取得
    int tempGraph = LoadGraph(graphPath);
    GetGraphSize(tempGraph, &xSize, &ySize);
    DeleteGraph(tempGraph);

    //要素数をセット
    graphHandle.resize(divGraphAllNumber);

    //画像を分割読み込み
    LoadDivGraph(graphPath, divGraphAllNumber, divNumberX, divNumberY, xSize / divNumberX, ySize / divNumberY, graphHandle.data());

    return graphHandle;
}