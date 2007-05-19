#ifndef KEYINPUT_H
#define KEYINPUT_H

#include "dxlib.h"
#include <deque>
using namespace std;

struct KeyboardInput{
	char ch;
	LONGLONG timeCount;	/* キーが押された時間 */
};

deque<KeyboardInput> g_deqKeyboardInput;
//CRITICAL_SECTION g_csKeyboardInput;

long FAR PASCAL KeyboardInputProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	static char CtrlCode[][ 2 ] =
	{
		VK_DELETE	,	CTRL_CODE_DEL	,	// ＤＥＬキー

		VK_LEFT		,	CTRL_CODE_LEFT	,	// ←キー
		VK_RIGHT	,	CTRL_CODE_RIGHT	,	// →キー
		VK_UP		,	CTRL_CODE_UP	,	// ↑キー
		VK_DOWN		,	CTRL_CODE_DOWN	,	// ↓キー
		0			,	0
	} ;

	switch(message){
	// 文字コードメッセージ
	case WM_CHAR :
		{
			// 文字をバッファにコピー
			//ST_StokInputChar( ( char )wParam ) ;
			KeyboardInput ki;
			ki.ch = (char)wParam;
			ki.timeCount = GetNowHiPerformanceCount();
//printfDx("%c(%d), %f\n", ki.ch, ki.ch, (double)ki.timeCount);
			g_deqKeyboardInput.push_back(ki);
		}
		break ;
	case WM_KEYDOWN:
		{
			int VKey = ( int )wParam ;
			char *CCode = ( char * )CtrlCode ;

			// コントロール文字コードに対応するキーが
			// 押されていたらバッファに格納する
			while( *CCode )
			{
				if( *CCode == VKey )
				{
					// バッファに文字コードを代入
					//ST_StokInputChar( *( CCode + 1 ) ) ;
					KeyboardInput ki;
					ki.ch = *(CCode + 1);
					ki.timeCount = GetNowHiPerformanceCount();
//printfDx("%c(%d), %f\n", ki.ch, ki.ch, (double)ki.timeCount);
					g_deqKeyboardInput.push_back(ki);
					break ;
				}
				CCode += 2 ;
			}
		}
		break ;
	}
	return 0;
}

/* キーを押した時刻を必要としない（返さない）バージョン */
char GetKeyboardInput(){
	char ch;
	if(g_deqKeyboardInput.empty()){	/* キューが空 */
		ch = 0;
	}else{	/* キューが空でないのでそれを取得 */
		KeyboardInput &ki = g_deqKeyboardInput.front();
		ch = ki.ch;
		//timeCount = ki.timeCount;
		g_deqKeyboardInput.pop_front();	/* 読んだら削除 */
	}
	return ch;
}

char GetKeyboardInput(LONGLONG &timeCount){
	char ch;
	if(g_deqKeyboardInput.empty()){	/* キューが空 */
		ch = 0;
	}else{	/* キューが空でないのでそれを取得 */
		KeyboardInput &ki = g_deqKeyboardInput.front();
		ch = ki.ch;
		timeCount = ki.timeCount;
		g_deqKeyboardInput.pop_front();	/* 読んだら削除 */
	}
	return ch;
}

#endif