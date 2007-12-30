#ifndef __UTYPING_CONFIG
#define __UTYPING_CONFIG

enum{
CONFIG_NEVER,
CONFIG_QUERY,
CONFIG_ALWAYS,
};

#include <stdio.h>
#include <string.h>

class UTypingConfig{
public:
	UTypingConfig();
	void init();
	void read();
public:
	//bool f_useMultiThread;
	int loadSoundType;
	int volume;
	
	char defaultReplayFile[256];
	int overwriteReplay;
	
	bool f_rankingCheckDate;
	bool f_rankingCheckChallenge;
	
	bool f_failByGauge;
	
	bool f_fullScreen;
	bool f_showFPS;
	bool f_waitVSync;
	
	bool f_debugMode;
	bool f_debugBeat;	/* false 普通に debug / true BEATLINE を音符にする */
	int debugTime;	/* デバッグ開始位置（ミリ秒） */
	
	bool f_showProcessTime;
};

UTypingConfig g_config;

UTypingConfig::UTypingConfig(){
	init();
}

void UTypingConfig::init(){
	//f_useMultiThread = true;
	loadSoundType = DX_SOUNDDATATYPE_MEMPRESS;
	volume = -1;
	
	strcpy(defaultReplayFile, "default.rep");
	overwriteReplay = CONFIG_QUERY;
	
	f_rankingCheckDate = false;
	f_rankingCheckChallenge = true;
	
	f_failByGauge = false;
	
	f_fullScreen = false;
	f_showFPS = false;
	f_waitVSync = true;
	
	f_debugMode = false;
	f_debugBeat = false;
	debugTime = 0;
	
	f_showProcessTime = false;
}

void UTypingConfig::read(){
	FILE *fp;
	fp = fopen("UTyping_config.txt", "r");
	if(!fp){
		/* configがなければ、特に何もしないでよい。 */
		/* configがないことをエラーにするのは微妙すぎる。 */
		return;
	}
	char buf[256];
	while(fgetline(buf, fp)){
		char *ptr1 = strtok(buf, "=");
		if(ptr1 == NULL || ptr1[0] == '#'){	/* 何も書かれていないか'#'で始まるコメント行 */
			continue;
		}
		char *ptr2 = strtok(NULL, "");
		if(ptr2 == NULL){
			ptr2 = "";	/* ""を指させておく */
		}
		/*
		if(!strcmp(ptr1, "UseMultiThread")){
			if(!strcmp(ptr2, "true")){
				f_useMultiThread = true;
			}else if(!strcmp(ptr2, "false")){
				f_useMultiThread = false;
			}else{
				throw "[config] UseMultiThread は true または false で指定しなければならない。（デフォルト: true ）";
			}
		}else */
		if(!strcmp(ptr1, "LoadSoundType")){
			if(!strcmp(ptr2, "0")){	/* 少し読み込み時間が長い、再生負荷は軽い */
				loadSoundType = DX_SOUNDDATATYPE_MEMNOPRESS;	/* 読み込むときに圧縮を展開 */
			}else if(!strcmp(ptr2, "1")){	/* 程々の読み込み時間と再生負荷 */
				loadSoundType = DX_SOUNDDATATYPE_MEMPRESS;	/* 再生するときに圧縮を展開 */
			}else if(!strcmp(ptr2, "2")){	/* 読み込み時間は短い、再生負荷は重い */
				loadSoundType = DX_SOUNDDATATYPE_FILE;	/* メモリにロードしない */
			}else{
				throw "[config] LoadSoundType は 0, 1 または 2 で指定しなければならない。（デフォルト: 1 ）";
			}
		}else if(!strcmp(ptr1, "Volume")){
			if(!strcmp(ptr2, "default")){
				volume = -1;
			}else{
				int n;
				int tmp;
				tmp = sscanf(ptr2, "%d", &n);
				if(tmp < 1 || n < 0 || n > 255){
					throw "[config] Volume は default または 0 以上 255 以下 で指定しなければならない。（デフォルト: default ）";
				}
				volume = n;
			}
		}else if(!strcmp(ptr1, "DefaultReplayFile")){
			strcpy(defaultReplayFile, ptr2);
			/* 特に失敗はしない */
			//throw "";
		}else if(!strcmp(ptr1, "OverwriteReplay")){
			if(!strcmp(ptr2, "never")){
				overwriteReplay = CONFIG_NEVER;
			}else if(!strcmp(ptr2, "query")){
				overwriteReplay = CONFIG_QUERY;
			}else if(!strcmp(ptr2, "always")){
				overwriteReplay = CONFIG_ALWAYS;
			}else{
				throw "[config] OverwriteReplay は never, query または always で指定しなければならない。（デフォルト: query ）";
			}
		}else if(!strcmp(ptr1, "RankingCheckDate")){
			if(!strcmp(ptr2, "true")){
				f_rankingCheckDate = true;
			}else if(!strcmp(ptr2, "false")){
				f_rankingCheckDate = false;
			}else{
				throw "[config] RankingCheckDate は true または false で指定しなければならない。（デフォルト: false ）";
			}
		}else if(!strcmp(ptr1, "RankingCheckChallenge")){
			if(!strcmp(ptr2, "true")){
				f_rankingCheckChallenge = true;
			}else if(!strcmp(ptr2, "false")){
				f_rankingCheckChallenge = false;
			}else{
				throw "[config] RankingCheckChallenge は true または false で指定しなければならない。（デフォルト: true ）";
			}
		}else if(!strcmp(ptr1, "FailByGauge")){
			if(!strcmp(ptr2, "true")){
				f_failByGauge = true;
			}else if(!strcmp(ptr2, "false")){
				f_failByGauge = false;
			}else{
				throw "[config] FailByGauge は true または false で指定しなければならない。（デフォルト: false ）";
			}
		}else if(!strcmp(ptr1, "FullScreen")){
			if(!strcmp(ptr2, "true")){
				f_fullScreen = true;
			}else if(!strcmp(ptr2, "false")){
				f_fullScreen = false;
			}else{
				throw "[config] FullScreen は true または false で指定しなければならない。（デフォルト: false ）";
			}
		}else if(!strcmp(ptr1, "ShowFPS")){
			if(!strcmp(ptr2, "true")){
				f_showFPS = true;
			}else if(!strcmp(ptr2, "false")){
				f_showFPS = false;
			}else{
				throw "[config] ShowFPS は true または false で指定しなければならない。（デフォルト: false ）";
			}
		}else if(!strcmp(ptr1, "WaitVSync")){
			if(!strcmp(ptr2, "true")){
				f_waitVSync = true;
			}else if(!strcmp(ptr2, "false")){
				f_waitVSync = false;
			}else{
				throw "[config] WaitVSync は true または false で指定しなければならない。（デフォルト: true ）";
			}
		}else if(!strcmp(ptr1, "DebugMode")){
			if(!strcmp(ptr2, "true")){
				f_debugMode = true;
				f_debugBeat = false;
			}else if(!strcmp(ptr2, "beat")){
				f_debugMode = true;
				f_debugBeat = true;
			}else if(!strcmp(ptr2, "false")){
				f_debugMode = false;
			}else{
				throw "[config] DebugMode は true または beat または false で指定しなければならない。（デフォルト: false ）";
			}
		}else if(!strcmp(ptr1, "DebugTime")){
			int n;
			int tmp;
			tmp = sscanf(ptr2, "%d", &n);
			if(tmp < 1 || n < 0){
				throw "[config] DebugTime は 0 以上 で指定しなければならない。（デフォルト: 0 ）";
			}
			debugTime = n;
		}else if(!strcmp(ptr1, "ShowProcessTime")){
			if(!strcmp(ptr2, "true")){
				f_showProcessTime = true;
			}else if(!strcmp(ptr2, "false")){
				f_showProcessTime = false;
			}else{
				throw "[config] ShowProcessTime は true または false で指定しなければならない。（デフォルト: false ）";
			}
		}else{
			//throw "[config] 設定できる値は UseMultiThread, LoadSoundType, DebugMode である。";
			//throw "[config] 設定できる値は LoadSoundType, Volume, FullScreen, ShowFPS, WaitVSync, DebugMode である。";
			throw "[config] 設定できない項目を設定しようとした。";
		}
	}
	fclose(fp);
}

#endif
