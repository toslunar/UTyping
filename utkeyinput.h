#ifndef __UTYPING_KEYINPUT
#define __UTYPING_KEYINPUT

#include "dxlib.h"

#define KEYBOARD_INPUT_BUFFER_SIZE 256

struct KeyboardInputData{
	char ch;
	LONGLONG timeCount;	/* キーが押された時間 */
};

class KeyboardInput{
public:
	KeyboardInput();
	~KeyboardInput();
	void clear();
	void push(KeyboardInputData &data);
	KeyboardInputData pop();
	bool empty();
private:
	void EnterCS();
	void LeaveCS();
private:
	KeyboardInputData m_queue[KEYBOARD_INPUT_BUFFER_SIZE];
	int m_queueBegin, m_queueEnd;
	CRITICAL_SECTION m_cs;
};

KeyboardInput g_keyboardInput;
/* グローバルで作っておく */

KeyboardInput::KeyboardInput(){
	InitializeCriticalSection(&m_cs);
	m_queueEnd = 0;
	clear();
}

KeyboardInput::~KeyboardInput(){
	DeleteCriticalSection(&m_cs);
}

void KeyboardInput::clear(){
	m_queueBegin = m_queueEnd;
}

void KeyboardInput::push(KeyboardInputData &data){
	EnterCS();
	if(m_queueEnd - m_queueBegin < KEYBOARD_INPUT_BUFFER_SIZE){
	/* バッファがいっぱいではない */
		m_queue[m_queueEnd % KEYBOARD_INPUT_BUFFER_SIZE] = data;
		m_queueEnd++;
	}
	LeaveCS();
}

KeyboardInputData KeyboardInput::pop(){
	EnterCS();
	KeyboardInputData data;
	data = m_queue[m_queueBegin % KEYBOARD_INPUT_BUFFER_SIZE];
	m_queueBegin++;
	LeaveCS();
	return data;
}

bool KeyboardInput::empty(){
	bool isEmpty;
	EnterCS();
	if(m_queueBegin == m_queueEnd){
		isEmpty = true;
	}else{
		isEmpty = false;
	}
	LeaveCS();
	return isEmpty;
}

void KeyboardInput::EnterCS(){
	EnterCriticalSection(&m_cs);
}

void KeyboardInput::LeaveCS(){
	LeaveCriticalSection(&m_cs);
}

/* ============================================================ */

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
			KeyboardInputData data;
			data.ch = (char)wParam;
			data.timeCount = GetNowHiPerformanceCount();
//printfDx("%c(%d), %f\n", data.ch, data.ch, (double)data.timeCount);
			g_keyboardInput.push(data);
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
					KeyboardInputData data;
					data.ch = *(CCode + 1);
					data.timeCount = GetNowHiPerformanceCount();
//printfDx("%c(%d), %f\n", data.ch, data.ch, (double)data.timeCount);
					g_keyboardInput.push(data);
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
	if(g_keyboardInput.empty()){	/* キューが空 */
		ch = 0;
	}else{	/* キューが空でないのでそれを取得 */
		KeyboardInputData data = g_keyboardInput.pop();
		ch = data.ch;
		//timeCount = data.timeCount;
	}
	return ch;
}

char GetKeyboardInput(LONGLONG &timeCount){
	char ch;
	if(g_keyboardInput.empty()){	/* キューが空 */
		ch = 0;
	}else{	/* キューが空でないのでそれを取得 */
		KeyboardInputData data = g_keyboardInput.pop();
		ch = data.ch;
		timeCount = data.timeCount;
	}
	return ch;
}

#endif
