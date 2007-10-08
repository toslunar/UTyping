#ifndef __UTYPING_KEYINPUT
#define __UTYPING_KEYINPUT

#include "DxLib.h"

/* DxLib.hで定義してないキーコード */
/* それとかぶる08,09,0d,10,1c,1d,1e,1fや20以降は避ける */
#define CTRL_CODE_F1 (0x01)	// F1
#define CTRL_CODE_F2 (0x02)	// F2
#define CTRL_CODE_F3 (0x03)	// F3
#define CTRL_CODE_F4 (0x04)	// F4
#define CTRL_CODE_F5 (0x05)	// F5
#define CTRL_CODE_F6 (0x06)	// F6
#define CTRL_CODE_F7 (0x07)	// F7
#define CTRL_CODE_F8 (0x0a)	// F8
#define CTRL_CODE_F9 (0x0b)	// F9
#define CTRL_CODE_F10 (0x0c)	// F10
#define CTRL_CODE_F11 (0x0e)	// F11
#define CTRL_CODE_F12 (0x0f)	// F12


#define KEYBOARD_INPUT_BUFFER_SIZE 256

struct KeyboardInputData{
	char ch;
	double timeCount;	/* キーが押された時間 */
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
	EnterCS();
	m_queueBegin = m_queueEnd;
	LeaveCS();
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
		VK_F1		,	CTRL_CODE_F1	,	// F1キー
		VK_F2		,	CTRL_CODE_F2	,	// F2キー
		VK_F3		,	CTRL_CODE_F3	,	// F3キー
		VK_F4		,	CTRL_CODE_F4	,	// F4キー
		VK_F5		,	CTRL_CODE_F5	,	// F5キー
		VK_F6		,	CTRL_CODE_F6	,	// F6キー
		VK_F7		,	CTRL_CODE_F7	,	// F7キー
		VK_F8		,	CTRL_CODE_F8	,	// F8キー
		VK_F9		,	CTRL_CODE_F9	,	// F9キー
		VK_F10		,	CTRL_CODE_F10	,	// F10キー
		VK_F11		,	CTRL_CODE_F11	,	// F11キー
		VK_F12		,	CTRL_CODE_F12	,	// F12キー
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
			data.timeCount = myGetNowHiPerformanceCount();
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
					data.timeCount = myGetNowHiPerformanceCount();
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

char GetKeyboardInput(double &timeCount){
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

int ClearKeyboardInput(){
	g_keyboardInput.clear();
	return 0;
}

#endif
