#pragma once
#include"DxLib.h"
#include<math.h>

/// <summary>
/// dxlib�ݒ�
/// </summary>
/// <param name="screenWidht">�X�N���[���̕�</param>
/// <param name="screenHeight">�X�N���[���̍���</param>
void DxlibSetting(unsigned int screenWidht = 1920, unsigned int screenHeight = 1080)
{
    SetGraphMode(screenWidht, screenHeight, 32);//�E�B���h�E�̃T�C�Y�ƃJ���[���[�h�����߂�
    ChangeWindowMode(TRUE);						//�E�B���h�E���[�h�ɂ���
    SetWindowStyleMode(7);						//�ő剻�{�^�������݂���E�C���h�E���[�h�ɕύX

    if (DxLib_Init() == -1)return;

    // �T�C�Y�ύX���\�ɂ���
    SetWindowSizeChangeEnableFlag(TRUE, FALSE);

    // �E�C���h�E�T�C�Y�̓Q�[����ʂƈ�v������
    SetWindowSize(screenWidht, screenHeight);

    SetMainWindowText("gamename");              //�E�B���h�E�i�����Ƃ���j�ɃQ�[����������
    SetDrawScreen(DX_SCREEN_BACK);		        //�w�i���Z�b�g����
}

/// <summary>
/// �w�i���ߓ���Đ��֐�
/// </summary>
/// <param name="movieHandle">����n���h��</param>
/// <param name="screenHandle">�X�N���[���n���h����MakeScreen�֐��Ńn���h��������Ă��̎��ɑ�O������TRUE�ɂ���K�v������</param>
/// <param name="movieBackColorType">����w�i�F�̃^�C�v(0:��, 1:��, 2:�����w�肷��)</param>
/// <param name="movieWidht">���敝</param>
/// <param name="movieHeght">���捂��</param>
/// <param name="isLoop">��������[�v���邩</param>
/// <param name="screenType">�ŏI�I�ɕ`�悷��Ƃ��̃X�N���[���n���h��</param>
void PlayTransparentMovie(int movieHandle, int screenHandle, unsigned short movieBackColorType = 0, int movieWidht = 1920, int movieHeight = 1080,
    VECTOR position = VGet(0.0f, 0.0f, 0.0f), bool isLoop = true, int screenType = DX_SCREEN_BACK)
{
    //�X�N���[���n���h���ɓ����`�悷��
    SetDrawScreen(screenHandle);
    if (isLoop)//���[�v�Đ����邩�ǂ���
    {
        PlayMovieToGraph(movieHandle, DX_PLAYTYPE_LOOP);
    }
    else
    {
        PlayMovieToGraph(movieHandle);
    }
    DrawExtendGraph(position.x, position.y, position.x + movieWidht, position.y + movieHeight, movieHandle, TRUE);

    //���̃X�N���[���n���h���ɖ߂�
    SetDrawScreen(screenType);

    //�X�N���[���n���h�����摜�Ƃ��ē��߂��Ă���`��
    //�w�i�̐F�ɂ���ē��߂���F��ς���
    if (movieBackColorType == 0)//黒
{
    GraphFilter(screenHandle, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 10, TRUE, GetColor(0, 255, 0), 0);
}
else if (movieBackColorType == 1)//緑
{
    GraphFilter(screenHandle, DX_GRAPH_FILTER_REPLACEMENT, 0, 255, 0, 255, 0, 0, 0, 0);
}
else//白
{
    GraphFilter(screenHandle, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_GREATER, 245, TRUE, GetColor(0, 255, 0), 0);
}
    DrawExtendGraph(position.x, position.y, position.x + movieWidht, position.y + movieHeight, screenHandle, TRUE);
}

/// <summary>
/// �����v�Z�֐�
/// </summary>
/// <param name="movieHandle">���W1</param>
/// <param name="screenHandle">���W2</param>
template<typename T>
T CalculateDistance(VECTOR position1, VECTOR position2 = VGet(0.0f, 0.0f, 0.0f))
{
    //���W�P������W�Q�܂ł̋������v�Z���ĕԂ�
    VECTOR tempVector   = VGet(position1.x, position1.y, position1.z);
    tempVector          = VSub(tempVector, position2);
    tempVector          = VGet(tempVector.x * tempVector.x, tempVector.y * tempVector.y, tempVector.z * tempVector.z);
    T value             = tempVector.x + tempVector.y + tempVector.z;
    return sqrt(value);
}

/// <summary>
/// �摜�A�j���[�V�����`��
/// </summary>
/// <param name="position">�`����W</param>
/// <param name="graphHandle">�摜�n���h��(�����ǂݍ��݂�������)</param>
/// <param name="graphWidth">�摜�̕�(1������)</param>
/// <param name="graphHeight">�摜�̍���(1������)</param>
/// <param name="tilSwitchTime">�摜���؂�ւ��܂ł̎���</param>
/// <param name="finishGraphNumber">�Ō�̃A�j���[�V�����摜�������ڂ���\������</param>
/// <param name="startGraphNumber">�ŏ��̃A�j���[�V�����摜�������ڂ���\������</param>
void DrawAnimationGraph(VECTOR position, int graphHandle[], int graphWidth, int graphHeight,
    unsigned int tilSwitchTime, unsigned int finishGraphNumber, unsigned int startGraphNumber = 0)
{
    //�؂�ւ��܂ŃJ�E���g�𑝂₵�đ҂�
    static int waitCount;
    static int animationCount = startGraphNumber;
    ++waitCount;

    //�ʎ�̃A�j���[�V�����ւ̐؂�ւ������m���ăJ�E���g���O�ɖ߂�
    static int checkChangeAnim = startGraphNumber;
    if (checkChangeAnim != startGraphNumber)
    {
        waitCount       = 0;
        animationCount  = startGraphNumber;
        checkChangeAnim = startGraphNumber;
    }

    if (waitCount >= tilSwitchTime)
    {
        //�A�j���[�V�����J�E���g�𑝂₷
        ++animationCount;
        waitCount = 0;

        //�A�j���[�V�������Ō�܂ł�������ŏ��ɖ߂�
        if (animationCount > finishGraphNumber)
        {
            animationCount = startGraphNumber;
        }
    }
    DrawExtendGraph(position.x, position.y, position.x + graphWidth, position.y + graphHeight, graphHandle[animationCount], TRUE);
}

/// <summary>
/// �����_�ŕ`��֐�(�t�H���g�w��\)
/// </summary>
/// <param name="position">�`����W</param>
/// <param name="text">�`�悵�����e�L�X�g</param>
/// <param name="color">�����̐F</param>
/// <param name="fontHandle">�t�H���g�n���h��</param>
/// <param name="brinkSpeed">�_�ŃX�s�[�h(�f�t�H���g��2�A0������Ɠ_�ł��Ȃ�)</param>
void DrawBrinkStringToHandle(VECTOR position, const char* text, int color, int fontHandle = 0, unsigned short brinkSpeed = 2)
{
    //�_�ŃX�s�[�h��0��������_�ł��Ȃ�
    if (brinkSpeed == 0)
    {
        DrawStringToHandle(position.x, position.y, text, color, fontHandle);
    }
    else
    {
        //�_�ŃJ�E���g
        static int brinkCount;
        brinkCount += 1 * brinkSpeed;
        if (brinkCount > 100)
        {
            brinkCount = 0;
        }

        //�_�ŃJ�E���g�����l�ȉ��̂Ƃ������`�悷��
        if (brinkCount > 100 / brinkSpeed)
        {
            DrawStringToHandle(position.x, position.y, text, color, fontHandle);
        }
    }
}

/// <summary>
/// �摜�_�ŕ`��֐�
/// </summary>
/// <param name="position">�`����W</param>
/// <param name="widht">�摜�̕�</param>
/// <param name="height">�摜�̍���</param>
/// <param name="graphHandle">�摜�n���h��</param>
/// <param name="brinkSpeed">�_�ŃX�s�[�h(�f�t�H���g��2�A0������Ɠ_�ł��Ȃ�)</param>
/// <param name="isTrans">�摜���߂��������ǂ���</param>
void DrawExtendBrinkGraph(VECTOR position, float widht, float height, int graphHandle,  unsigned short brinkSpeed = 2, bool isTrans = true)
{
    //�_�ŃX�s�[�h��0��������_�ł��Ȃ�
    if (brinkSpeed == 0)
    {
        DrawExtendGraph(position.x, position.y, position.x + widht, position.y + height, graphHandle, isTrans);
    }
    else
    {
        //�_�ŃJ�E���g
        static int brinkCount;
        brinkCount += 1 * brinkSpeed;
        if (brinkCount > 100)
        {
            brinkCount = 0;
        }

        //�_�ŃJ�E���g�����l�ȉ��̂Ƃ������`�悷��
        if (brinkCount > 100 / brinkSpeed)
        {
            DrawExtendGraph(position.x, position.y, position.x + widht, position.y + height, graphHandle, isTrans);
        }
    }
}