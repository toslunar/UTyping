
//#ifdef UTYPING_USE_MULTI_THREAD
/* マルチスレッド用 */

struct KeyboardInput{
	char ch;
	LONGLONG timeCount;	/* キーが押された時間 */
};

deque<KeyboardInput> g_deqKeyboardInput;
CRITICAL_SECTION g_csKeyboardInput;

/* キーを押した時刻を必要としない（返さない）バージョン */
char MT_GetKeyboardInput(){
	char ch;
	EnterCriticalSection(&g_csKeyboardInput);
	{
		if(g_deqKeyboardInput.empty()){	/* キューが空 */
			ch = 0;
		}else{	/* キューが空でないのでそれを取得 */
			KeyboardInput &ki = g_deqKeyboardInput.front();
			ch = ki.ch;
			//timeCount = ki.timeCount;
			g_deqKeyboardInput.pop_front();	/* 読んだら削除 */
		}
	}
	LeaveCriticalSection(&g_csKeyboardInput);
	return ch;
}

/* キーを押した時刻を返すバージョン */
char MT_GetKeyboardInput(LONGLONG &timeCount){
	char ch;
	EnterCriticalSection(&g_csKeyboardInput);
	{
		if(g_deqKeyboardInput.empty()){	/* キューが空 */
			ch = 0;
		}else{	/* キューが空でないのでそれを取得 */
			KeyboardInput &ki = g_deqKeyboardInput.front();
			ch = ki.ch;
			timeCount = ki.timeCount;
			g_deqKeyboardInput.pop_front();	/* 読んだら削除 */
		}
	}
	LeaveCriticalSection(&g_csKeyboardInput);
	return ch;
}

DWORD WINAPI KeyboardInputThread(LPVOID lpParameter){
#if 0
	char keyState[256];	/* キーの状態 */
	memset(keyState, 0, 256);	/* 最初は全部押されていないとみなす */
#endif
	LONGLONG lastCount = GetNowHiPerformanceCount();
	
	ClearInputCharBuf();	/* 最初にバッファを消しておく */
	while(1){
		LONGLONG count = GetNowHiPerformanceCount();
		if(count - lastCount < 1000){	/* 1ミリ秒以上たっていないなら何もしない */
			Sleep(0);
			continue;
		}
		lastCount = count;
		
		KeyboardInput ki;
#if 0
		while(1){
			char buf[256];
			if(GetHitKeyStateAll(buf) == -1){	/* エラーが起きたら終了 */
				goto L1;
			}
			if(memcmp(keyState, buf, 256) != 0){	/* キーの押下状態が前回と異なる */
				ki.timeCount = GetNowHiPerformanceCount();	/* 時刻保存 */
				memcpy(keyState, buf, 256);
				break;
			}
			Sleep(0);
		}
#endif
//ProcessStreamSoundMemAll();
//g_check.begin();
		ki.timeCount = GetNowHiPerformanceCount();
		/* ProcessMessageの前に時刻を保存し、ProcessMessageが遅くても大丈夫にする。 */
		if(ProcessMessage() == -1){
			break;
		}
//g_check.end(100);
		while(1){
			ki.ch = GetInputChar(TRUE);	/* キー入力（TRUE:一度読み込んだら消す） */
			if(ki.ch == 0){	/* 「なし」と帰ってきたら終わり */
				break;
			}
			EnterCriticalSection(&g_csKeyboardInput);
			{
				g_deqKeyboardInput.push_back(ki);	/* キューに追加 */
			}
			LeaveCriticalSection(&g_csKeyboardInput);
		}
	}
L1:
	return 0;
}

//#else
/* シングルスレッド用 */

char ST_GetKeyboardInput(){
	return GetInputChar(TRUE);
}

char ST_GetKeyboardInput(LONGLONG &timeCount){
	timeCount = GetNowHiPerformanceCount();
	return GetInputChar(TRUE);
}

//#endif
