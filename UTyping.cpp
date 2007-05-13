#include "DxLib.h"

#include <stdio.h>
#include <string.h>

#include <vector>
#include <deque>
#include <bitset>

#include "utuid.h"

/*
#define chkI LONGLONG txxx,tyyy;
#define chkB txxx=GetNowHiPerformanceCount();
#define chkE tyyy=GetNowHiPerformanceCount(); {int t=(int)(tyyy-txxx); if(t>=10)printfDx((t>=16384)?"--------%5d--------\n":"%5d\n", t);}
*/

using namespace std;

#define COLOR_EXCELLENT GetColor(255, 255, 0)
#define COLOR_GOOD GetColor(0, 255, 0)
#define COLOR_FAIR GetColor(0, 128, 255)
#define COLOR_POOR GetColor(128, 128, 128)

#define SEC_EXCELLENT 0.02
#define SEC_GOOD 0.05
#define SEC_FAIR 0.10
#define SEC_POOR 0.20

#define SCORE_EXCELLENT 1500
#define SCORE_GOOD 1000
#define SCORE_FAIR 500
#define SCORE_POOR 0

#define SCORE_COMBO 10
#define SCORE_COMBO_MAX 1000

#define SCORE_TYPING 500
/* タイピングで1文字確定させるごとに（1バイト文字、2バイト文字を問わない） */

#define TYPE_BUFFER_LEN 8
/* タイピングに用いるバッファの長さ */
/* 名前の最大長にも影響する */

#define RANKING_LEN 20
/* ランキングに記録する順位が何位までか */

#define RANKING_DRAW_LEN 5
/* 何位ずつランキングを表示するか */
/* RANKING_LEN の約数だと表示に無駄がない */

/* ============================================================ */

/* 描画位置など */

#define CIRCLE_SPEED 250.0
/* 流れる円の標準の速さ（ドット/秒） */

#define Y_INFO 10
#define Y_INFO2 40

#define X_CIRCLE 100
#define Y_CIRCLE 180
#define R_CIRCLE 30

#define Y0_BAR (Y_CIRCLE - 60)
#define Y1_BAR (Y_CIRCLE + 50)
#define Y0_BEAT (Y_CIRCLE - 45)
#define Y1_BEAT (Y_CIRCLE + 40)

#define X_ACCURACY (X_CIRCLE - R_CIRCLE)
/* 円の左端 */
#define Y_ACCURACY 90

#define Y_LYRICS (Y_CIRCLE + R_CIRCLE + 30)

#define X_LYRICS_KANJI 100
/* これの左に"Next: ", 右に歌詞を書く */
#define Y_LYRICS_KANJI (Y_LYRICS + 35)
#define Y_LYRICS_KANJI_NEXT (Y_LYRICS + 70)

#define Y_LYRICS_BIG 350

#define Y_BUFFER 390

#define X_HIDDEN (X_CIRCLE + R_CIRCLE + 60)
/* Hiddenで見える左の限界 */
#define X_SUDDEN (X_HIDDEN + R_CIRCLE)
/* Suddenで見える右の限界 */
/* 両方かけたときに、ちょうど半円ぐらい見える */

#define SCALE_FUNCTION 60.0
/* 判定位置の円を原点とする座標と思うときの、長さ1に相当する画面の長さ */

/* ============================================================ */

#define W_WINDOW 640
#define H_WINDOW 480

/* ============================================================ */

#define INFTY 1000000000

enum{
PHASE_READY,	/* 開始前 */
PHASE_MAIN,	/* メイン */
PHASE_RESULT,	/* スコア表示 */
PHASE_FINISHED,	/* 名前入力後（ハイスコア処理完了） */
PHASE_EXIT,	/* 終わったので消してもらう */
};

enum{
BEATLINE_BAR,
BEATLINE_BEAT,
};

enum{
CHALLENGE_HIDDEN,	/* 隠れるの */
CHALLENGE_SUDDEN,	/* 突然現れるの */
CHALLENGE_STEALTH,	/* 見えないの */
CHALLENGE_LYRICS_STEALTH,	/* 歌詞が見えないの */
CHALLENGE_SIN,
CHALLENGE_COS,
CHALLENGE_TAN,

CHALLENGE_NUM
};

/* ============================================================ */

/* 雑多な関数（予定） */

bool readInt(int &n, FILE *fp);
void writeInt(int n, FILE *fp);
bool readChar8(char *str, FILE *fp);
void writeChar8(char *str, FILE *fp);

void getDirFromPath(char *directoryName, const char *fileName);

//char *fgetline(char buf[], FILE *fp);

void getOrdinal(char *buf, int n);
/* 序数を取得 */

void getDateStr(char *buf);
/* 日付と時刻をbufに格納 */

void outputError(char *str);
/* エラー出力 */

HANDLE newThread(LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter);
void deleteThread(HANDLE handle);

bool readInt(int &n, FILE *fp){
	int b[4];
	for(int i=0; i<4; i++){
		b[i] = getc(fp);
		if(b[i] == EOF){
			return false;
		}
	}
	n = b[0] | b[1]<<8 | b[2]<<16 | b[3]<<24;
	return true;
}

void writeInt(int n, FILE *fp){
	int b[4];
	for(int i=0; i<4; i++){
		b[i] = n >> (8*i);
	}
	for(int i=0; i<4; i++){
		putc(b[i], fp);
	}
	return;
}

bool readChar8(char *str, FILE *fp){
	int b[8];
	for(int i=0; i<8; i++){
		b[i] = getc(fp);
		if(b[i] == EOF){
			return false;
		}
	}
	for(int i=0; i<8; i++){
		str[i] = b[i];
	}
	str[8] = '\0';
	return true;
}

void writeChar8(char *str, FILE *fp){
	int b[8];
	for(int i=0; i<8; i++){
		b[i] = str[i];
	}
	for(int i=1; i<8; i++){
		if(b[i-1] == '\0'){
			b[i] = '\0';
		}
	}
	for(int i=0; i<8; i++){
		putc(b[i], fp);
	}
	return;
}

void getDirFromPath(char *directoryName, const char *fileName){
	strcpy(directoryName, fileName);
	for(int i = strlen(directoryName) - 1; i >= 0; i--){	/* 後ろからたどる */
		if(directoryName[i] == '/' || directoryName[i] == '\\'){	/* '/'や'\'が現れたら */
			directoryName[i + 1] = '\0';	/* そこまでなので、次を'\0'に */
			return;
		}
	}
	/* 最後まで現れなかったら */
	directoryName[0] = '\0';
	return;
}

#define fgetline(buf, fp) fgetline_1(buf, sizeof(buf), fp)
/* fpから1行読む。ただし、最後の'\n'は削除 */

char *fgetline_1(char *buf, size_t size, FILE *fp){
	if(fgets(buf, size, fp) == NULL){
		return NULL;
	}
	int len = strlen(buf) - 1;
	if(buf[len] == '\n'){
		buf[len] = '\0';
	}
	return buf; 
}

void getOrdinal(char *buf, int n){
	switch(n % 10){	/* 1の位で判定 */
	case 1:
		sprintf(buf, "%2dst", n);
		break;
	case 2:
		sprintf(buf, "%2dnd", n);
		break;
	case 3:
		sprintf(buf, "%2drd", n);
		break;
	default:
		sprintf(buf, "%2dth", n);
		break;
	}
	if((n / 10) % 10 == 1){	/* 下二桁11,12,13をthにする */
		sprintf(buf, "%2dth", n);
	}
}

void getDateStr(char *buf){
	DATEDATA date;
	GetDateTime(&date);
	sprintf(buf, "%d/%02d/%02d %02d:%02d:%02d", date.Year, date.Mon,
		date.Day, date.Hour, date.Min, date.Sec);
}

void outputError(char *str){
	char buf[256];
	getDateStr(buf);
	FILE *fp;
	fp = fopen("エラーログ.txt","a");
	fprintf(fp, "[%s] %s\n", buf, str);
	fclose(fp);
}

//#ifdef UTYPING_USE_MULTI_THREAD

/* DWORD WINAPI (*func)(LPVOID)を新しいスレッドとして作る */
HANDLE newThread(LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter){
	DWORD dwID;
	return CreateThread(NULL,0,lpStartAddress,lpParameter,0,&dwID);
}

void deleteThread(HANDLE handle){
	if(!TerminateThread(handle, 0)){
		throw __LINE__;
	}
	CloseHandle(handle);
}

//#endif

/* ============================================================ */

/* Config */

class UTypingConfig{
public:
	UTypingConfig();
	void init();
	void read();
public:
	bool f_useMultiThread;
	int loadSoundType;
	
	bool f_debugMode;
};

UTypingConfig g_config;

UTypingConfig::UTypingConfig(){
	init();
}

void UTypingConfig::init(){
	f_useMultiThread = true;
	loadSoundType = DX_SOUNDDATATYPE_MEMPRESS;
	f_debugMode = false;
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
		if(ptr1 == NULL){
			continue;
		}
		char *ptr2 = strtok(NULL, "");
		if(ptr2 == NULL){
			ptr2 = "";	/* ""を指させておく */
		}
		if(!strcmp(ptr1, "UseMultiThread")){
			if(!strcmp(ptr2, "true")){
				f_useMultiThread = true;
			}else if(!strcmp(ptr2, "false")){
				f_useMultiThread = false;
			}else{
				throw "[config] UseMultiThread は true または false で指定しなければならない。（デフォルト: true ）";
			}
		}else if(!strcmp(ptr1, "LoadSoundType")){
			if(!strcmp(ptr2, "0")){	/* 少し読み込み時間が長い、再生負荷は軽い */
				loadSoundType = DX_SOUNDDATATYPE_MEMNOPRESS;	/* 読み込むときに圧縮を展開 */
			}else if(!strcmp(ptr2, "1")){	/* 程々の読み込み時間と再生負荷 */
				loadSoundType = DX_SOUNDDATATYPE_MEMPRESS;	/* 再生するときに圧縮を展開 */
			}else if(!strcmp(ptr2, "2")){	/* 読み込み時間は短い、再生負荷は重い */
				loadSoundType = DX_SOUNDDATATYPE_FILE;	/* メモリにロードしない */
			}else{
				throw "[config] LoadSoundType は 0, 1 または 2 で指定しなければならない。（デフォルト: 1 ）";
			}
		}else if(!strcmp(ptr1, "DebugMode")){
			if(!strcmp(ptr2, "true")){
				f_debugMode = true;
			}else if(!strcmp(ptr2, "false")){
				f_debugMode = false;
			}else{
				throw "[config] DebugMode は true または false で指定しなければならない。（デフォルト: false ）";
			}
		}else{
			throw "[config] 設定できる値は UseMultiThread, LoadSoundType, DebugMode である。";
		}
	}
	fclose(fp);
}

/* ============================================================ */

/* UTypingUserIDを調べる */

bool checkUTypingUserID(){
	FILE *fp;
	fp = fopen("UTyping_ID.txt", "r");
	if(!fp){
		throw "UTyping_ID.txt が開けません。";
	}
	int ID[3];
	getUTypingUserID(ID);
	for(int i=0; i<3; i++){
		int tmp;
		fscanf(fp, "%X", &tmp);
		if(tmp != ID[i]){
			return false;
		}
	}
	fclose(fp);
	return true;
}

/* ============================================================ */

#define CCHECK_N_DATA (H_WINDOW/8)

class CCheck{
public:
	CCheck();
	void begin();
	void end();
	void draw(int t, int Color);	/* t マイクロ秒を長さ 1 で描く */
private:
	int m_counter;
	int m_data[CCHECK_N_DATA];
	LONGLONG m_timeCount;
};

CCheck::CCheck(){
	m_counter = 0;
	for(int i=0; i<CCHECK_N_DATA; i++){
		m_data[i] = 0;
	}
}

void CCheck::begin(){
	m_timeCount=GetNowHiPerformanceCount();
}

void CCheck::end(){
	m_data[m_counter] = GetNowHiPerformanceCount() - m_timeCount;
	m_counter++;
	if(m_counter == CCHECK_N_DATA){
		m_counter = 0;
	}
}

void CCheck::draw(int t, int Color){
	for(int i=0; i<CCHECK_N_DATA; i++){
		DrawBox(0, i * 8, (m_data[(i + m_counter) % CCHECK_N_DATA] / t),
			(i + 1) * 8, Color, TRUE);
	}
}

/* ============================================================ */

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
	ClearInputCharBuf();	/* 最初にバッファを消しておく */
	while(1){
		KeyboardInput ki;
		ki.ch = GetInputChar(TRUE);	/* キー入力（TRUE:一度読み込んだら消す） */
		//ki.ch = GetInputCharWait(TRUE);	/* キー入力（TRUE:一度読み込んだら消す） */
		if(ki.ch == 0){	/* 「なし」と帰ってきたら終わり */
			Sleep(0);
			//WaitKey();
			continue;
		}
		ki.timeCount = GetNowHiPerformanceCount();
		EnterCriticalSection(&g_csKeyboardInput);
		{
			g_deqKeyboardInput.push_back(ki);	/* キューに追加 */
		}
		LeaveCriticalSection(&g_csKeyboardInput);
	}
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

/* configを参照して分岐 */

char GetKeyboardInput(){
	if(g_config.f_useMultiThread){
		return MT_GetKeyboardInput();
	}else{
		return ST_GetKeyboardInput();
	}
}

char GetKeyboardInput(LONGLONG &timeCount){
	if(g_config.f_useMultiThread){
		return MT_GetKeyboardInput(timeCount);
	}else{
		return ST_GetKeyboardInput(timeCount);
	}
}


/* ============================================================ */

class CScore{
public:
	CScore();
	CScore(const char *n,int s,int sa,int st,int ce,int cg,int cf,int cp,int cx,int ca,int cm);
	void read(FILE *fp);
	void write(FILE *fp);
	
	void draw(int y, char *str, int fontHandle);
	
	bool nameCmp(CScore &score);
	
	bool operator ==(CScore &score){
		return m_score == score.m_score;
	}
	bool operator !=(CScore &score){
		return m_score != score.m_score;
	}
	bool operator <(CScore &score){
		return m_score < score.m_score;
	}
	bool operator <=(CScore &score){
		return m_score <= score.m_score;
	}
	bool operator >(CScore &score){
		return m_score > score.m_score;
	}
	bool operator >=(CScore &score){
		return m_score >= score.m_score;
	}
private:
	char m_name[TYPE_BUFFER_LEN + 1];
	int m_score;
	int m_scoreAccuracy, m_scoreTyping;
	int m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass, m_countAll;
	int m_comboMax;
};

CScore::CScore(){
	strcpy(m_name, "_");
	m_score = 0;
	m_scoreAccuracy = 0;
	m_scoreTyping = 0;
	m_countExcellent = 0;
	m_countGood = 0;
	m_countFair = 0;
	m_countPoor = 0;
	m_countPass = 0;
	m_countAll = 0;
	m_comboMax = 0;
}

CScore::CScore(const char *n,int s,int sa,int st,int ce,int cg,int cf,int cp,int cx,int ca,int cm){
	strcpy(m_name, n);
	m_score = s;
	m_scoreAccuracy = sa;
	m_scoreTyping = st;
	m_countExcellent = ce;
	m_countGood = cg;
	m_countFair = cf;
	m_countPoor = cp;
	m_countPass = cx;
	m_countAll = ca;
	m_comboMax = cm;
}

void CScore::read(FILE *fp){
	strcpy(m_name, "_");
	m_score = 0;
	m_scoreAccuracy = 0;
	m_scoreTyping = 0;
	m_countExcellent = 0;
	m_countGood = 0;
	m_countFair = 0;
	m_countPoor = 0;
	m_countPass = 0;
	m_countAll = 0;
	m_comboMax = 0;
	readChar8(m_name, fp);
	readInt(m_score, fp);
	readInt(m_scoreAccuracy, fp);
	readInt(m_scoreTyping, fp);
	readInt(m_countExcellent, fp);
	readInt(m_countGood, fp);
	readInt(m_countFair, fp);
	readInt(m_countPoor, fp);
	readInt(m_countPass, fp);
	readInt(m_countAll, fp);
	readInt(m_comboMax, fp);
/*
	fscanf(fp, "%s%d%d%d%d%d%d%d%d%d%d", m_name, &m_score, &m_scoreAccuracy, &m_scoreTyping,
		&m_countExcellent, &m_countGood, &m_countFair, &m_countPoor, &m_countPass,
		&m_countAll, &m_comboMax);
*/
}

void CScore::write(FILE *fp){
/*
	fprintf(fp, "%s %d %d %d %d %d %d %d %d %d %d\n", m_name, m_score, m_scoreAccuracy, m_scoreTyping,
		m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass,
		m_countAll, m_comboMax);
*/
	writeChar8(m_name, fp);
	writeInt(m_score, fp);
	writeInt(m_scoreAccuracy, fp);
	writeInt(m_scoreTyping, fp);
	writeInt(m_countExcellent, fp);
	writeInt(m_countGood, fp);
	writeInt(m_countFair, fp);
	writeInt(m_countPoor, fp);
	writeInt(m_countPass, fp);
	writeInt(m_countAll, fp);
	writeInt(m_comboMax, fp);
}

void CScore::draw(int y, char *str, int fontHandle){
	char buf[256];
	int width;
	sprintf(buf, "%s: %-8s %10d 点(%10d +%10d ), ", str,
		m_name, m_score, m_scoreAccuracy, m_scoreTyping);
	DrawStringToHandle(40, y + 6, buf, GetColor(255, 255, 255), fontHandle);
	if(m_comboMax >= 0){
		sprintf(buf, "最大 %4d コンボ, (%4d/%4d/%4d/%4d/%4d)", m_comboMax,
			m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass);
	}else{	/* m_comboMax == -1 でフルコンボ扱い */
		sprintf(buf, "フルコンボ, (%4d/%4d/%4d/%4d/%4d)",
			m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass);
	}
	width = GetDrawStringWidthToHandle(buf, strlen(buf), fontHandle);
	DrawStringToHandle((W_WINDOW - 40) - width, y + 26, buf, GetColor(255, 255, 255), fontHandle);
}

bool CScore::nameCmp(CScore &score){
	return strcmp(m_name, score.m_name) == 0;
}


/* ============================================================ */

class CRanking{
public:
	CRanking();
	~CRanking();
	int update(CScore &score);
	void open(const char *fileName);
	void close();
	void read();
	void write();
	
	void draw(int y, int rankBegin, int rankLen, int fontHandle);
private:
	FILE *m_fp;
	CScore m_score[RANKING_LEN];
};

CRanking::CRanking(){
	m_fp = NULL;	/* まだファイルを開いていない */
}

CRanking::~CRanking(){
	close();
}

/* ランクインなら順位(0～RANKING_LEN-1)を返す。そうでなければ -1 を返す */
int CRanking::update(CScore &score){
	int lastRank = RANKING_LEN - 1;	/* すでに入っている順位、入っていない場合は、RANKING_LEN - 1が都合が良い */
	/* データをずらすときに、ランク外だったとしても、最後のランクだったとしても同じになるから。 */
	for(int i = 0; i < RANKING_LEN; i++){
		if(score.nameCmp(m_score[i])){
			lastRank = i;
			break;
		}
	}
	for(int i = 0; i < RANKING_LEN; i++){
		if(score > m_score[i]){	/* i位のスコアより真に大きいので、i位にランクイン */
			if(i > lastRank){	/* すでにある自分のスコアより悪い（「同じ」ときもこの判定になる） */
				return -1;
			}
			
			/* i位以降、前の自分のデータ以前のデータを後ろに1つずつずらす */
			for(int j = lastRank; j > i; j--){
				m_score[j] = m_score[j-1];
			}
			m_score[i] = score;
			return i;
		}
	}
	return -1;
}

void CRanking::open(const char *fileName){
	if(m_fp != NULL){
		close();
	}
	/* バイナリで開くので"b"が入る */
	m_fp = fopen(fileName, "r+b");	/* あればそれを開いて、 */
	if(m_fp == NULL){
		m_fp = fopen(fileName, "w+b");	/* なければ作成 */
		if(m_fp == NULL){
			throw __LINE__;
		}
	}
}

void CRanking::close(){
	if(m_fp != NULL){
		fclose(m_fp);
		m_fp = NULL;
	}
}

void CRanking::read(){
	rewind(m_fp);
	for(int i=0; i<RANKING_LEN; i++){
		m_score[i].read(m_fp);
	}
	rewind(m_fp);
	return;
}

void CRanking::write(){
	rewind(m_fp);
	for(int i=0; i<RANKING_LEN; i++){
		m_score[i].write(m_fp);
	}
	rewind(m_fp);
}

/* rankBeginからrankLen位分表示 */
void CRanking::draw(int y, int rankBegin, int rankLen, int fontHandle){
	char buf[256];
	for(int i = 0; i < rankLen; i++){
		int j = rankBegin + i;	/* 書く順位 */
		if(j >= RANKING_LEN){	/* 記録されているランキングの外 */
			break;
		}
		getOrdinal(buf, j + 1);	/* 実際は、0番目は1位 etc. */
		m_score[j].draw(y + 48 * i, buf, fontHandle);
	}
}

/* ============================================================ */

#define CEFFECT1_SEC_CLEAR 0.6
#define CEFFECT1_FONT_SIZE 40

class CEffect1{
public:
	CEffect1();
	~CEffect1();
	void clear();
	void insert(int x, int y, char *str, int color, double time);
	void insert(int x, int y, char ch, int color, double time);
	void draw(double time);
private:
	int m_fontHandle;
	struct Data{
		int x, y;
		char str[20];
		int color;
		double time;
		int strWidth;
	};
	deque<Data> m_deq;
};

CEffect1::CEffect1(){
	m_fontHandle = CreateFontToHandle("ＭＳ 明朝", CEFFECT1_FONT_SIZE, 2);
}

CEffect1::~CEffect1(){
	DeleteFontToHandle(m_fontHandle);
}

void CEffect1::clear(){
	m_deq.clear();
}

void CEffect1::insert(int x, int y, char *str, int color, double time){
	CEffect1::Data data;
	data.x = x;
	data.y = y;
	strcpy(data.str, str);
	data.color = color;
	data.time = time;
	data.strWidth = GetDrawStringWidthToHandle(data.str, strlen(data.str), m_fontHandle);
	m_deq.push_back(data);
}

void CEffect1::insert(int x, int y, char ch, int color, double time){
	/* char1文字の場合を文字列の場合に帰着 */
	char buf[2];
	buf[0] = ch;
	buf[1] = '\0';
	insert(x, y, buf, color, time);
}

void CEffect1::draw(double time){
	while(!m_deq.empty() &&
			time - m_deq.front().time >= CEFFECT1_SEC_CLEAR){
		m_deq.pop_front();
	}
	for(deque<CEffect1::Data>::iterator itr = m_deq.begin(); itr != m_deq.end(); itr++){
		double timeDiff = time - (*itr).time;
		double ExRate;	/* 文字の拡大率 */
		{
			double tmp = 1.0 - (timeDiff / CEFFECT1_SEC_CLEAR);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 * (tmp * tmp * tmp));
			/* 時間がたつにつれ透明に */
		}
		{	/* 時間が経つにつれ拡大 */
			double tmp = timeDiff / CEFFECT1_SEC_CLEAR;
			ExRate = 1.0 + 4.0 * tmp * tmp;
		}
		DrawExtendStringToHandle(
			(*itr).x - ((*itr).strWidth * ExRate / 2), (*itr).y - (CEFFECT1_FONT_SIZE * ExRate / 2),
			ExRate, ExRate, (*itr).str, (*itr).color, m_fontHandle);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);	/* ブレンドしないように戻す */
}

/* ============================================================ */

class CConvertData{
public:
	CConvertData(const char *buf1, const char *buf3);
	bool match_front(const char *str);
	bool match(const char *str);
public:
	/* まあ、もともとstructにするつもりだったぐらいだし */
	char m_str[9];	/* 日本語 → ローマ字, 全角 → 半角 の変換先は 4 文字以内だから、5以上にしておけばよい */
	int m_len;
	/* str には条件に使われる文字列が入っているが、そのうち確定される長さ */
	/* 例: 「ん」に対して {"nb", 1}, 「っっ」に対して {"lltu", 4} など */
};

CConvertData::CConvertData(const char *buf1, const char *buf3){
	strcpy(m_str, buf1);
	m_len = strlen(buf1) - strlen(buf3);
	/* ここでは、データを信用して、buf3がbuf1の後方と一致することを仮定する。 */
}

bool CConvertData::match_front(const char *str){	/* データの前方とstrが一致するか */
	for(int i=0; str[i]!='\0'; i++){
		if(str[i] != m_str[i]){
			return false;
		}
	}
	return true;
}

bool CConvertData::match(const char *str){
	return (strcmp(m_str, str) == 0);
}

/* ============================================================ */

class CTrieNode{
public:
	CTrieNode();
	~CTrieNode();
	void insert(const char *str, const CConvertData &data);	/* strでたどった先にdataを入れる */
	CTrieNode *find(const char *str);
private:
	CTrieNode *m_next[256];
public:
	/* もうprivateにするの面倒 */
	vector<CConvertData> m_data;
};

CTrieNode::CTrieNode(){
	for(int i=0; i<256; i++){
		m_next[i] = NULL;
	}
}

CTrieNode::~CTrieNode(){
	for(int i=0; i<256; i++){
		if(m_next[i] != NULL){
			delete m_next[i];
		}
	}
}

void CTrieNode::insert(const char *str, const CConvertData &data){
	if(*str == '\0'){	/* たどり終わったのでデータを入れる */
		m_data.push_back(data);
		return;
	}
	int i = *str & 0xff;	/* 0(1)～255 にする */
	if(m_next[i] == NULL){	/* たどれないので新しく作る */
		m_next[i] = new CTrieNode();
	}
	m_next[i]->insert(str+1, data);	/* 次をたどる */
}

CTrieNode *CTrieNode::find(const char *str){
	if(*str == '\0'){	/* ここでOK */
		return this;
	}
	int i = *str & 0xff;	/* 0(1)～255 にする */
	if(m_next[i] == NULL){	/* ない */
		return NULL;
	}
	return m_next[i]->find(str+1);	/* 次をたどる */
}

/* ============================================================ */

struct Lyrics{
public:
	bool isJapanese1(); 
public:
	char ch;	/* 変換された文字（あいう、。～abc123+-など）の1バイト */
	double timeJust,timeUntil;	/* ちょうどの時間、その文字以前だけを打つときに、最も遅い時間 */
	bool isBlockStart;	/* 1音節（タイミング判定をするかたまり）の最初 */
	bool isTyped;	/* すでに打たれたか */
	bool isScoringTarget;	/* 現在タイミング判定をする対象であるか */
};

bool Lyrics::isJapanese1(){
	if(this->ch & 0x80){
		return true;
	}else{
		return false;
	}
}

/* ============================================================ */

struct LyricsKanji{	/* 表示される歌詞 */
	char str[256];	/* 歌詞 */
	double timeBegin;	/* 表示が始まる時間 */
	double timeEnd;	/* 表示が終わる時間 */
};

/* ============================================================ */

struct BeatLine{
	int type;
	double time;
};

/* ============================================================ */

class CChallenge{
public:
	CChallenge();
	
	bool isEasy();
	
	void reset();
	void set(int pos);
	void reset(int pos);
	void flip(int pos);
	bool test(int pos);
	
	void speedUp();
	void speedDown();
	double speed();	/* 流れる速さの標準との比を返す */
	
	void keyUp();
	void keyDown();
	int key();
	double frequencyRate();	/* 周波数が何倍されるか */
private:
	bitset<CHALLENGE_NUM> m_flags;
	int m_speed;	/* 円が流れる速さの標準との比の10倍(intでもつため) */
	int m_key;
};

CChallenge::CChallenge(){
	reset();
}

bool CChallenge::isEasy(){
	if(m_key < 0){	/* 再生速度を遅くした */
		return true;
	}
	return false;
}

void CChallenge::reset(){
	m_flags.reset();
	m_speed = 10;
	m_key = 0;
}

void CChallenge::set(int pos){
	m_flags.set(pos);
}

void CChallenge::reset(int pos){
	m_flags.reset(pos);
}

void CChallenge::flip(int pos){
	m_flags.flip(pos);
}

bool CChallenge::test(int pos){
	return m_flags.test(pos);
}

void CChallenge::speedUp(){
	if(m_speed < 20){
		m_speed++;
	}else if(m_speed < 30){
		m_speed += 2;
	}else if(m_speed < 40){
		m_speed += 5;
	}
}

void CChallenge::speedDown(){
	if(m_speed > 30){
		m_speed -= 5;
	}else if(m_speed > 20){
		m_speed -= 2;
	}else if(m_speed > 5){
		m_speed--;
	}
}

double CChallenge::speed(){
	return m_speed / 10.0;
}

void CChallenge::keyUp(){
	if(m_key < 12){
		m_key++;
	}
}

void CChallenge::keyDown(){
	if(m_key > -12){
		m_key--;
	}
}

int CChallenge::key(){
	return m_key;
}

double CChallenge::frequencyRate(){
	return pow(2.0, m_key / 12.0);
}

/* ============================================================ */

class CTyping{
public:
	CTyping();
	~CTyping();
	// void loadDictionary(const char *fileName);
	/* ローマ字辞書はコンストラクタで読む */
	void load(const char *fumenFileName, const char *rankingFileName);
	void unload();
	void setChallenge(CChallenge &challenge);
	void keyboard(char ch, LONGLONG timeCount);
	bool idle();
private:
	void loadRanking(const char *fileName);
	void unloadRanking();
	
	void setText(const char *str, int color);
	
	void setTime();
	void synchronizeTime(int soundHandle, double frequencyRate);
	double getTime();
	double getTime(LONGLONG timeCount);
	
	void phase(int phaseNum);
	
	bool input(char *typeBuffer, int &typeBufferLen, vector<Lyrics>::iterator &lyricsPosition,
		double time, bool isCheckOnly);
	bool input_1(char *typeBuffer, int &typeBufferLen, vector<Lyrics>::iterator &lyricsPosition,
		double time, bool isCheckOnly, bool isSaiki);	/* inputの実際に計算する部分 */
	
	void scoreTyping(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd);
	void scoreAccuracy(double time, vector<Lyrics>::iterator lyricsPosition);
	
	int getDrawPosX(double timeDiff);
	int getDrawPosY(int x);
public:
	void draw();
	void drawResult();
private:
	CTrieNode m_trie;
	
	vector<Lyrics> m_lyrics;
	vector<Lyrics>::iterator m_lyricsPosition;
	
	vector<Lyrics>::iterator m_lyricsPositionEnd;
	/* 現在打っている歌詞がどこまでか、つまり、[m_lyricsPosition, m_lyricsPositionEnd) */
	
	vector<Lyrics>::iterator m_lyricsDrawLeft;
	vector<Lyrics>::iterator m_lyricsDrawRight;
	/* drawで、直前に描画された範囲 */
	
	double m_timeLength;	/* 譜面の（時間の）長さ */
	
	vector<LyricsKanji> m_lyricsKanji;
	vector<LyricsKanji>::iterator m_lyricsKanjiPosition;
	
	vector<BeatLine> m_beatLine;
	vector<BeatLine>::iterator m_beatLineDrawLeft;
	vector<BeatLine>::iterator m_beatLineDrawRight;
	
	char m_typeBuffer[TYPE_BUFFER_LEN + 1];
	int m_typeBufferLen;
	
	int m_phase;
	
	LONGLONG m_timeBegin;
	
	int m_score;
	int m_scoreTyping;	/* タイピングによる得点 */
	int m_scoreAccuracy;	/* 精度による得点 */
	int m_combo;
	
	int m_comboMax;
	
	int m_countExcellent;
	int m_countGood;
	int m_countFair;
	int m_countPoor;
	int m_countPass;	/* スコア表示時に使う */
	int m_countAll;
	
	CRanking m_ranking;	/* ハイスコアのデータ */
	int m_rank;	/* ハイスコアの中での順位、ランクインしなければ -1 */
	
	CEffect1 m_effect1;	/* キー入力エフェクト */
	
	char m_text[256];
	int m_textColor;
	double m_textTime;
	
	CChallenge m_challenge;
	
	char m_musicFileName[256];
	
	int m_idleCounter;	/* idleが何回目の呼び出しかを知るためのカウンタ */
	
	int m_fontHandleNormal;
	int m_fontHandleBig;
	
	int m_soundHandleMusic;
};

CTyping::CTyping(){
//#ifndef DEBUG_MODE
	if(!g_config.f_debugMode){	/* 通常のとき */
		FILE *fp;
		fp = fopen("convert.dat","r");
		if(fp == NULL){
			throw "convert.dat が開けません。";
		}
		char buf[256], buf1[256], buf2[256], buf3[256];
		while(fgets(buf, sizeof(buf), fp)!=NULL){
			int n=sscanf(buf, "%s%s%s", buf1, buf2, buf3);
			/* ローマ字、日本語、残るローマ字 */
			if(n<2){
				throw "convert.dat の形式が不正です。";
			}
			int len;
			if(n == 2){
				strcpy(buf3, "");
			}
			m_trie.insert(buf2, CConvertData(buf1, buf3));
			/* 日本語をキーにローマ字のデータを入れる */
		}
		fclose(fp);
	}else{	/* デバッグ時 */
//#else
		char buf1[256], buf2[256], buf3[256];
		strcpy(buf2, "+");
		strcpy(buf3, "");
		for(char ch = ' ' + 1; ch < 127; ch++){
			buf1[0] = ch;
			buf1[1] = '\0';
			m_trie.insert(buf2, CConvertData(buf1, buf3));
		}
	}
//#endif
	
	/* チャレンジを初期化（Defaultは何も変なことしない） */
	/* チャレンジは曲ロードのとき初期化しないので */
	m_challenge.reset();
	
	m_idleCounter = 0;
	
	m_fontHandleNormal = CreateFontToHandle(NULL, 16, 3);
	m_fontHandleBig = CreateFontToHandle("ＭＳ 明朝", 36, 2);
	
	m_soundHandleMusic = -1;	/* 何も読み込んだりしてはいない */
}

CTyping::~CTyping(){
	unload();
	DeleteFontToHandle(m_fontHandleNormal);	/* フォントを削除 */
	DeleteFontToHandle(m_fontHandleBig);
}

void CTyping::load(const char *fumenFileName, const char *rankingFileName){
	char directoryName[256];
	getDirFromPath(directoryName, fumenFileName);
	FILE *fp;
	fp = fopen(fumenFileName,"r");
	if(fp == NULL){
		throw "譜面ファイルが開けません。";
	}
	
	/* 読み込む前に全消去 */
	m_lyrics.clear();
	m_lyricsKanji.clear();
	m_beatLine.clear();
	strcpy(m_musicFileName, "");
	
	/* 歌詞のかたまりの数（=m_count○○の和の最大値）を数えるために0に */
	m_countAll = 0;
	
	double frequencyRate = m_challenge.frequencyRate();
	/* KeyShiftによる周波数の倍率 */
	
	/* 読み込み開始 */
	
	char tmpBuf[256];
	char buf[256], bufLast[256];
	strcpy(bufLast,"");
	double time, timeLast;
	bool flag = true;
	while(flag){
		if(fgetline(tmpBuf, fp) == NULL){
			m_timeLength = timeLast;	/* 譜面終了時刻を記録 */
			flag = false;
			time = INFTY;
		}else{
			int n;
			switch(tmpBuf[0]){
			case '+':	/* 打つ歌詞 */
				n = sscanf(tmpBuf + 1, "%lf%s", &time, buf);
				if(n < 2){
					throw "譜面が不正です。(+)";
				}
				time /= frequencyRate;
//#ifdef DEBUG_MODE
				if(g_config.f_debugMode){
					strcpy(buf, "+");
				}
//#endif
				break;
			case '*':	/* 表示する歌詞 */
				{
					char *ptr = strtok(tmpBuf + 1, " \t\n");	/* スペースで区切られるまでの部分を取り出す */
					if(ptr == NULL){
						throw "譜面が不正です。(*)";
					}
					n = sscanf(ptr, "%lf", &time);	/* 時刻を読む */
					if(n < 1){
						throw "譜面が不正です。(*)";
					}
					time /= frequencyRate;
					ptr = strtok(NULL, "");	/* 残りの部分を取り出す */
					LyricsKanji lk;
					strcpy(lk.str, ptr);
					lk.timeBegin = time;
					lk.timeEnd = INFTY;	/* 終わりが設定されなければ、表示され続ける */
					m_lyricsKanji.push_back(lk);
				}
				continue;
			case '=':	/* 小節線 */
			case '-':	/* 拍線 */
				{
					n = sscanf(tmpBuf + 1, "%lf", &time);	/* 時刻を読む */
					if(n < 1){
						throw "譜面が不正です。(=,-)";
					}
					time /= frequencyRate;
					BeatLine bl;
					if(tmpBuf[0] == '='){
						bl.type = BEATLINE_BAR;
					}else{	/* == '-' */
						bl.type = BEATLINE_BEAT;
					}
					bl.time = time;
					m_beatLine.push_back(bl);
				}
				continue;
			case '/':	/* 区切り */
				n = sscanf(tmpBuf + 1, "%lf", &time);
				if(n < 1){
					throw "譜面が不正です。(/)";
				}
				time /= frequencyRate;
				strcpy(buf, " ");
				
				if(!m_lyricsKanji.empty()){	/* 表示する歌詞が存在している */
					LyricsKanji &lk = *(m_lyricsKanji.end() - 1);	/* そのなかで最後のものを操作 */
					if(lk.timeEnd == INFTY){	/* 終了時刻が設定されていなければ、現在に設定 */
						lk.timeEnd = time;
					}
				}
				break;
			case '@':
				sprintf(m_musicFileName, "%s%s", directoryName, tmpBuf + 1);	/* 再生する音楽ファイル名を設定 */
				continue;
			default:
				continue;
			}
		}
		Lyrics ly;
		ly.isBlockStart = true;
		ly.isScoringTarget = true;
		for(char *ptr=bufLast; *ptr!='\0'; ptr++){
			ly.ch = *ptr;
			ly.timeJust = timeLast;
			ly.timeUntil = time;	/* 次の歌詞のtimeJustまで */
			if(ly.ch == ' '){
				ly.timeUntil = -INFTY;
				ly.isTyped = true;	/* 区切れの文字はすでに打ったという扱い */
				ly.isScoringTarget = false;	/* 区切りを「打って得点を得る」ことはできない */
			}else{
				ly.isTyped = false;
			}
			m_lyrics.push_back(ly);
			
			if(ly.isScoringTarget){	/* 判定のある歌詞の数を数える */
				m_countAll++;
			}
			
			ly.isBlockStart = false;	/* 最初の文字のみを true にするため */
			ly.isScoringTarget = false;
		}
		timeLast = time;
		strcpy(bufLast, buf);
	}
	fclose(fp);
	/* 打つ歌詞の番兵として最後に' ',INFTYを追加しておく */
	{
		Lyrics ly;
		ly.ch = ' ';
		ly.timeJust = INFTY;
		ly.timeUntil = INFTY;
		ly.isBlockStart = true;
		ly.isTyped = true;
		ly.isScoringTarget = false;
		m_lyrics.push_back(ly);
		
		/* ' 'なら1つ進むことへの対策として、'\n'を追加しておく */
		ly.ch = '\n';
		m_lyrics.push_back(ly);
	}
	
	/* 表示する歌詞の番兵として、"",INFTYを追加 */
	{
		LyricsKanji lk;
		strcpy(lk.str, "");
		lk.timeBegin = INFTY;
		lk.timeEnd = INFTY;
		m_lyricsKanji.push_back(lk);
	}
	
	/* 同じく番兵 */
	{
		BeatLine bl;
		bl.time = INFTY;
		m_beatLine.push_back(bl);
	}
	
	/* 現在打っている歌詞の位置 */
	m_lyricsPosition = m_lyrics.begin();
	m_lyricsKanjiPosition = m_lyricsKanji.begin();
	
	m_lyricsPositionEnd = m_lyricsPosition;	/* 最初は範囲なしということにしておこう */
	
	m_lyricsDrawLeft = m_lyrics.begin();
	m_lyricsDrawRight = m_lyrics.begin();
	/* 最初のdrawで設定されるから何を入れてもよいのだが、 */
	/* 右にしか進まないので最初にしておかなければならない */
	
	/* 理由は同上 */
	m_beatLineDrawLeft = m_beatLine.begin();
	m_beatLineDrawRight = m_beatLine.begin();
	
	/* キー入力を初期化 */
	m_typeBufferLen = 0;
	
	/* 何かキーを押すまでは始まらないので、まだはじまっていない */
	phase(PHASE_READY);
	
	/* 点数初期化 */
	m_score = 0;
	m_scoreTyping = 0;
	m_scoreAccuracy = 0;
	m_combo = 0;
	
	m_comboMax = 0;
	
	/* 優良可不可判定カウント初期化 */
	m_countExcellent = 0;
	m_countGood = 0;
	m_countFair = 0;
	m_countPoor = 0;
	
	strcpy(m_text, "");
	
	m_effect1.clear();	/* キー入力エフェクトを初期化 */
	
	loadRanking(rankingFileName);
	/* ランキング読み込み */
	
	m_soundHandleMusic = -1;
}

void CTyping::unload(){
	if(m_soundHandleMusic != -1){
		DeleteSoundMem(m_soundHandleMusic);
		m_soundHandleMusic = -1;
	}
	unloadRanking();
}

void CTyping::setChallenge(CChallenge &challenge){
	m_challenge = challenge;
}

void CTyping::keyboard(char ch, LONGLONG timeCount){
	if(m_phase == PHASE_READY){	/* 開始前なら */
		if(m_soundHandleMusic != -1){	/* 音楽がすでに読み込まれた */
			phase(PHASE_MAIN);
			PlaySoundMem(m_soundHandleMusic, DX_PLAYTYPE_BACK);	/* 音楽を流し始める */
			setTime();	/* 始まった時刻を覚える */
		}
		return;
	}
	if(m_phase == PHASE_RESULT){	/* スコア表示中なら */
		/* 名前記入のためにバッファを読み書き */
		if(ch < CTRL_CODE_CMP){	/* 文字コードでないとき */
			switch(ch){
			case CTRL_CODE_CR:	/* 改行なら確定 */
				if(!g_config.f_debugMode){
					if(m_typeBufferLen > 0){	/* 名前が入力されているか確認 */
						phase(PHASE_FINISHED);
					}
				}else{	/* デバッグモード時 */
					phase(PHASE_FINISHED);
				}
				break;
			case CTRL_CODE_BS:	/* BackSpaceや左キーやDeleteなら */
			case CTRL_CODE_LEFT:
			case CTRL_CODE_DEL:
				if(m_typeBufferLen > 0){
					m_typeBufferLen--;	/* 一文字削除 */
				}
				break;
			}
			return;
		}
		if(ch == ' '){	/* スペースはアンダーバーに変換（ランキングファイルの仕様のため） */
			ch = '_';
		}
		if(m_typeBufferLen < TYPE_BUFFER_LEN){
			m_typeBuffer[m_typeBufferLen++] = ch;
		}
		return;
	}
	if(m_phase == PHASE_FINISHED){	/* ランキング表示中なら */
		if(ch == CTRL_CODE_CR){	/* Enterで終了 */
			phase(PHASE_EXIT);
		}
		return;
	}
	
	if(m_phase != PHASE_MAIN){	/* ここから先はゲームメインのときの処理 */
		return;
	}
	if(ch == '\0' || ch == ' '){	/* いろいろ困りそうだし、打つ意味がないから */
		return;
	}
	double time = getTime(timeCount);	/* キーボードを押した時の時間を取得 */
	
	if(time < (*m_lyricsPosition).timeJust - SEC_POOR){
		/* poorより前なので打てない */
		return;
	}
	
	m_typeBuffer[m_typeBufferLen++] = ch;
	
	if(input(m_typeBuffer, m_typeBufferLen, m_lyricsPosition, time, false)){
	/* その入力が現在の位置で入った */
		/* キー入力エフェクト */
		m_effect1.insert(X_CIRCLE, Y_CIRCLE, ch, GetColor(170, 170, 255), time);
		
		while((*m_lyricsPosition).ch == ' '){	/* 歌詞の切れ目を指しているなら */
			m_lyricsPosition++;	/* 進める */
		}
		return;
	}
	m_typeBufferLen--;
	char tmpBuffer[TYPE_BUFFER_LEN + 1];
	int tmpLen;
	vector<Lyrics>::iterator tmpLyricsPosition;
	tmpBuffer[0] = ch;
	tmpLen = 1;
	tmpLyricsPosition = m_lyricsPosition;
	
	int comboBackUp = m_combo;	/* m_comboを保存しておく */
	m_combo = 0;
	while(1){
		tmpLyricsPosition++;	/* 必ず1つ以上飛ばす必要があるので最初に++ */
		if((*tmpLyricsPosition).isBlockStart){
			if(time >= (*tmpLyricsPosition).timeUntil){
				/* 飛ばして、かつ、時間切れは打てない */
				/* ここではじかなくても、inputではじけるが、処理の重さが気になりそう */
				continue;
			}
			if(time < (*tmpLyricsPosition).timeJust - SEC_POOR){
				/* poor範囲に入っているところより外に探索が出た */
				break;
			} 
			if(input(tmpBuffer, tmpLen, tmpLyricsPosition, time, false)){
			/* 新しい位置でその入力が入った */
				/* キー入力エフェクト */
				m_effect1.insert(X_CIRCLE, Y_CIRCLE, ch, GetColor(170, 170, 255), time);
				
				/* 新しいデータを書き込む */
				strcpy(m_typeBuffer, tmpBuffer);
				m_typeBufferLen = tmpLen;	/* 1とは限らないはずなので */
				m_lyricsPosition = tmpLyricsPosition;
				while((*m_lyricsPosition).ch == ' '){	/* 歌詞の切れ目を指しているなら */
					m_lyricsPosition++;	/* 進める */
				}
				return;	/* m_comboは元に戻さない */
			}
		}
	}
	m_combo = comboBackUp;	/* m_comboを元に戻す */
	return;
}

bool CTyping::idle(){	/* 問題なければ true を返す */
	if(m_phase == PHASE_EXIT){	/* 終了することになっている */
		return false;
	}
	if(m_phase == PHASE_READY){	/* 開始していなければ特にすることは無い */
		if(m_soundHandleMusic == -1){	/* まだ音楽を読み込んでなかったら読み込み */
			SetCreateSoundDataType(g_config.loadSoundType);	/* configをみて読み込み方を決定 */
			m_soundHandleMusic = LoadSoundMem(m_musicFileName);
			
			if(m_challenge.key() != 0){	/* キーシフトが行われている */
				int oldFrequency = GetFrequencySoundMem(m_soundHandleMusic);	/* 元の再生周波数を読み込み */
				int newFrequency = (int)(oldFrequency *m_challenge.frequencyRate());
					/* 変更する倍率をかけて新しい周波数を出す */
				SetFrequencySoundMem(newFrequency, m_soundHandleMusic);	/* 変更 */
			}
		}
		return true;
	}
	if(m_phase != PHASE_MAIN){	/* ここから先はゲームメインのときの処理 */
		return true;
	}
	
//static CCheck check;
	if(m_idleCounter++ % 20 == 0){	/* 20回に1回実行 */
//check.begin();
		synchronizeTime(m_soundHandleMusic, m_challenge.frequencyRate());
//check.end();
	}
//check.draw(1);
	
	double time = getTime();
	bool flag = false;	/* m_lyricsPositionEndが変化したか、 */
	/* つまり、打てなくなったことの再チェックが必要か */
	
	if(m_lyricsPositionEnd < m_lyricsPosition){
		m_lyricsPositionEnd = m_lyricsPosition;
		flag = true;
	}
	
	/* まだ、ちょうどの位置に達していない最初の歌詞まで進む */
	while(time >= (*m_lyricsPositionEnd).timeJust){
		m_lyricsPositionEnd++;
		flag = true;
	}
	
	if(flag){	/* 毎回チェックすると重いかもしれないので、必要なときに限定する */
		/* 「何かのキーを押せば現在の位置で入力可能」ではない */
		if(!input(m_typeBuffer, m_typeBufferLen, m_lyricsPosition, time, true)){
			/* 入力が切れたものは飛ばす（m_lyricsPositionが進んで、コンボが切れる） */
			if(time >= (*m_lyricsPosition).timeUntil){
				/* わざわざ進むかどうかチェックしているのは、最後の'\n'にいるときを除くため */
				do{
				m_lyricsPosition++;
				}while(time >= (*m_lyricsPosition).timeUntil);
				/* 本当にm_lyricsPositionが進んだら、ここに来る */
				m_combo = 0;
				m_typeBufferLen = 0;
				/* 入力中のはもう使われることはない（から、ここで消してしまってよい） */
			}
		}
	}
	
	if(time >= m_timeLength){	/* 最後の譜面が通過 */
		phase(PHASE_RESULT);
		return true;
	}
	
	return true;
}

void CTyping::loadRanking(const char *fileName){
	m_ranking.open(fileName);
	m_ranking.read();
}

void CTyping::unloadRanking(){
	m_ranking.close();
}

void CTyping::setText(const char *str, int color){	/* 表示文字列を設定 */
	m_textTime = getTime();
	strcpy(m_text, str);
	m_textColor = color;
}

void CTyping::setTime(){	/* 開始時刻を設定 */
	m_timeBegin = GetNowHiPerformanceCount();
}

/* 音楽のスタートと開始時刻が等しくなるように、開始時刻を調整 */
void CTyping::synchronizeTime(int soundHandle, double frequencyRate){
	if(soundHandle == -1 || CheckSoundMem(soundHandle) != 1){
	/* 音楽が読み込まれていないか、再生されていないかのとき */
		return;
	}
	
	int musicTime = (int)(GetSoundCurrentTime(soundHandle) * 1000 / frequencyRate);
	/* 【 GetSoundCurrentTime を使っているので注意が必要かもしれない】 */
	int realTime = (int)(GetNowHiPerformanceCount() - m_timeBegin);
	int diff = realTime - musicTime;
	/* ここに、diffが大きすぎるときに変化を抑えるとか入れるといいかもしれない */
	/* diffが小さいときに無視するといいかもしれない */
	m_timeBegin += diff;
}

double CTyping::getTime(){	/* 開始時刻からの経過秒を取得 */
	return (GetNowHiPerformanceCount() - m_timeBegin) / 1000000.0;
}

double CTyping::getTime(LONGLONG timeCount){	/* timeCountの開始時刻からの経過秒を取得 */
	return (int)(timeCount - m_timeBegin) / 1000000.0;
}

void CTyping::phase(int phaseNum){
	if(phaseNum == PHASE_RESULT){	/* 終了して、スコア表示に */
		setTime();	/* 表示開始されてからの時間を利用するためにセット */
		m_typeBufferLen = 0;	/* 名前入力に使うのでバッファをクリア */
		
		/* 通過した個数を数えておく */
		m_countPass = m_countAll - m_countExcellent - m_countGood - m_countFair - m_countPoor;
		
		/* コンボ数が判定位置数で終了した（最後の文字もちゃんと打ち切った） */
		if(m_combo == m_countAll){
			m_comboMax = -1;	/* 最大コンボ数を「フルコンボ」としておく */
		}
	}
	if(phaseNum == PHASE_FINISHED){
//#ifndef DEBUG_MODE
		if(!g_config.f_debugMode){
			CScore score(m_typeBuffer, m_score, m_scoreAccuracy, m_scoreTyping,
				m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass,
				m_countAll, m_comboMax);
			if(m_challenge.isEasy()){	/* 簡単になるオプションを使用した場合、ランキングにのせない */
				m_rank = -1;
			}else{
				m_rank = m_ranking.update(score);
			}
			if(m_rank >= 0){
				m_ranking.write();
			}else{
				m_phase = PHASE_EXIT;	/* ランクインしなければ終了 */
				return;
			}
		}else{
//#else
			m_phase = PHASE_EXIT;
			return;
		}
//#endif
	}
	m_phase = phaseNum;
}

bool CTyping::input(char *typeBuffer, int &typeBufferLen, vector<Lyrics>::iterator &lyricsPosition,
		double time, bool isCheckOnly){
	typeBuffer[typeBufferLen] = '\0';
	/* typeBufferLenを「実際に打った長さ」として用いるため */
	
	if(isCheckOnly){
		int tmpTypeBufferLen = typeBufferLen + 1;	/* typeBufferLenは更新しない問題なし */
		/* 実際に打った長さを1つ多くすることによって、任意の1文字さらに打ったとして時間制限を判定する */
		return input_1(typeBuffer, tmpTypeBufferLen, lyricsPosition, time, true, false);
		/* 最後のfalseは再帰でないことを示す。 */
	}else{
		return input_1(typeBuffer, typeBufferLen, lyricsPosition, time, false, false);
	}
}

bool CTyping::input_1(char *typeBuffer, int &typeBufferLen, vector<Lyrics>::iterator &lyricsPosition,
		double time, bool isCheckOnly, bool isSaiki){
/* isSaikiは外からの呼び出しではfalse,内からの呼び出しではtrue */
	if(isSaiki && strlen(typeBuffer) == 0){
	/* 未確定ローマ字がないのに再帰した */
		return true;
	}
	vector<Lyrics>::iterator tmpLyricsPosition = lyricsPosition;
	CTrieNode *trie = &m_trie;
	while(1){
		char buf[2];
		buf[0] = (*tmpLyricsPosition).ch;
		tmpLyricsPosition++;
		buf[1] = '\0';
		trie = trie->find(buf);	/* 日本語1バイト進む */
		if(trie == NULL){	/* trieをたどれない、つまり、まとめて打てる範囲を超えた。 */
			return false;
		}
		for(vector<CConvertData>::iterator i = trie->m_data.begin();
				i != trie->m_data.end(); i++){	/* ローマ字候補を探索 */
			if((*i).match_front(typeBuffer)){
				int tmpLen = typeBufferLen - (*i).m_len;
				/* 今回変換される部分の次の部分からに対して実際に打たれている部分の長さ。 */
				/* これは負とかになりうる。 */
				if(tmpLen <= 0){	/* 今回に必要ならキーを足して一度に打てる範囲はここまで */
				/* これは、{んば……,nb……}でnだけ打った状況なども含まれる。 */
					if(time >= (*(tmpLyricsPosition-1)).timeUntil){
						/* その最後の文字が時間切れしていてはいけない */
						continue;
					}
				}
				char tmpTypeBuffer[TYPE_BUFFER_LEN + 1];
				strcpy(tmpTypeBuffer, (*i).m_str + (*i).m_len);	/* 未確定ローマ字になる予定の部分 */
				if(input_1(tmpTypeBuffer, tmpLen, tmpLyricsPosition, time, isCheckOnly, true)){
				/* 再帰の結果打てることが分かったとき */
					if(isCheckOnly){	/* 調べるためのときは、何もしない。 */
						return true;
					}
					/* 調べるためではなく、実際に打っているとき */
					if(typeBufferLen >= 1){
					/* 新しい音節の打ち始め */
						scoreAccuracy(time, lyricsPosition);
						/* ちょうどのタイミングとのずれから計算した得点を与える。 */
					}
					
					/* 「っ」の処理、全ての3バイト以上への変換に効果があるので注意 */
					if(typeBufferLen >= 2 && lyricsPosition + 2 < tmpLyricsPosition){
					/* 2文字打って、2バイト先(日本語の1文字先)が今回一度に打てる範囲で、さらに採点対象である */
					/* つまり、「っか」における"kk"の時点で「か」が採点される。 */
						scoreAccuracy(time, lyricsPosition + 2);
						/* ちょうどのタイミングとのずれから計算した得点を与える。 */
					}
					if((*i).match(typeBuffer)){	/* 完全一致 */
						/* 変換された歌詞の分だけ得点を与える */
						if(!isSaiki){
						/* 再帰の2段目以降でもタイピング点を計算すると2重になるので、直接の呼び出しのみ処理 */
							scoreTyping(lyricsPosition, tmpLyricsPosition);
						}
						
						for(vector<Lyrics>::iterator i = lyricsPosition; i != tmpLyricsPosition; i++){
							(*i).isTyped = true;	/* 打った歌詞の範囲を記録 */
						}
						lyricsPosition = tmpLyricsPosition;	/* 新しい歌詞の位置に */
						strcpy(typeBuffer, tmpTypeBuffer);
						typeBufferLen = tmpLen;
					}
					return true;
				}else{
					/* 再帰の結果だめだった */
					continue;
				}
			}
		}
	}
}

void CTyping::scoreTyping(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd){
/* [lyBegin, lyEnd)を打ったときの得点 */
	bool isJapanese2 = false;
	for(vector<Lyrics>::iterator i = lyBegin; i != lyEnd; i++){
		if(isJapanese2){
			isJapanese2 = false;
		}else{
			m_scoreTyping += SCORE_TYPING;
			if((*i).isJapanese1()){	/* 日本語の1バイト目なら */
				isJapanese2 = true;
			}
		}
	}
	m_score = m_scoreTyping + m_scoreAccuracy;
}

void CTyping::scoreAccuracy(double time, vector<Lyrics>::iterator lyricsPosition){
	if(!(*lyricsPosition).isScoringTarget){
	/* タイミングの採点対象ではない、または、もう採点された */
		return;
	}
	double timeDiff = time - (*lyricsPosition).timeJust;
//#ifdef DEBUG_MODE
	double tmp = timeDiff;
//#endif
	int scoreCombo = SCORE_COMBO * m_combo;	/* コンボ数を増やす前にコンボ数ボーナスを計算 */
	if(scoreCombo > SCORE_COMBO_MAX){
		scoreCombo = SCORE_COMBO_MAX;
	}
	
	m_combo++;	/* とりあえずコンボ数を 1 増やす */
	if(timeDiff < 0.0){
		timeDiff = -timeDiff;
	}
	int score;
	char buf[256];
	char *strAccuracy;	/* 判定の文字（優良可不可） */
	int color;	/* 表示色 */
	if(timeDiff < SEC_EXCELLENT){
		score = SCORE_EXCELLENT + scoreCombo;
		m_countExcellent++;
		strAccuracy = "優";
		color = COLOR_EXCELLENT;
	}else if(timeDiff < SEC_GOOD){
		score = SCORE_GOOD + scoreCombo;
		m_countGood++;
		strAccuracy = "良";
		color = COLOR_GOOD;
	}else if(timeDiff < SEC_FAIR){
		score = SCORE_FAIR + scoreCombo;
		m_countFair++;
		strAccuracy = "可";
		color = COLOR_FAIR;
	}else{
		score = SCORE_POOR;
		m_countPoor++;
		m_combo = 0;	/* コンボが途切れていた */
		strAccuracy = "不可";
		color = COLOR_POOR;
	}
	if(m_combo > m_comboMax){	/* コンボ数の最大値を更新 */
		m_comboMax = m_combo;
	}
//#ifndef DEBUG_MODE
	if(!g_config.f_debugMode){
		if(m_combo >= 10){	/* コンボが10を超えたらコンボ数を表示 */
			sprintf(buf, "%s %d", strAccuracy, m_combo);
		}else{
			sprintf(buf, "%s", strAccuracy);
		}
	}else{	/* デバッグ時 */
//#else
		sprintf(buf, "%+lf", tmp);	/* 時間のずれを表示 */
		if(-SEC_FAIR < tmp && tmp < 0.0){
			//color = GetColor((int)255 * (-tmp/SEC_FAIR), (int)255 * (1 - (-tmp/SEC_FAIR)), 0);
			color = GetColor(255, 0, 0);
		}else if(0.0 <= tmp && tmp < SEC_FAIR){
			//color = GetColor(0, (int)255 * (1 - (tmp/SEC_FAIR)), (int)255 * (tmp/SEC_FAIR));
			color = GetColor(0, 0, 255);
		}
	}
//#endif
	setText(buf, color);
	
	m_scoreAccuracy += score;	/* 得点加算 */
	m_score = m_scoreTyping + m_scoreAccuracy;
	(*lyricsPosition).isScoringTarget = false;	/* 2回以上得点を与えられることはない */
}

/* timeDiff: ちょうどの位置の何秒後か */
int CTyping::getDrawPosX(double timeDiff){
	/* CIRCLE_SPEED * m_challenge.speed() が速さ、（ただし、左向き） */
	return X_CIRCLE + (int)(-timeDiff * (CIRCLE_SPEED * m_challenge.speed()));
}

int CTyping::getDrawPosY(int x){
	x -= X_CIRCLE;	/* 画面の0を座標の0とあわせる */
	double y = 0;
	if(m_challenge.test(CHALLENGE_SIN)){
		y += sin(x / SCALE_FUNCTION) * SCALE_FUNCTION;
	}
	if(m_challenge.test(CHALLENGE_COS)){
		y += cos(x / SCALE_FUNCTION) * SCALE_FUNCTION;
	}
	if(m_challenge.test(CHALLENGE_TAN)){
		y += tan(x / SCALE_FUNCTION) * SCALE_FUNCTION;
	}
	return (int)-y;	/* スクリーン座標は上下が逆*/
}

void CTyping::draw(){
	if(m_phase >= PHASE_RESULT){	/* スコア表示以降 */
		drawResult();
		return;
	}
	double time;
	if(m_phase == PHASE_READY){
		if(m_soundHandleMusic == -1){
			DrawStringToHandle(50, 70, "Now loading...", GetColor(255, 255, 255), m_fontHandleBig);
		}else{
			DrawStringToHandle(50, 70, "Press any key to start.", GetColor(255, 255, 255), m_fontHandleBig);
		}
		time = 0.0;	/* 始まる前は0秒で止まっている */
	}else{
		time = getTime();	/* 開始時刻からの経過秒を取得 */
	}
	//*
	DrawFormatStringToHandle(10, Y_INFO, GetColor(255, 255, 255), m_fontHandleNormal,
		"%10d 点", m_score);
	DrawFormatStringToHandle(10, Y_INFO2, GetColor(255, 255, 255), m_fontHandleNormal,
		"%10d コンボ", m_combo);
	/*/
	DrawFormatStringToHandle(10, Y_INFO, GetColor(255, 255, 255), m_fontHandleNormal,
		"得点: %10d, %10d コンボ", m_score, m_combo);
	DrawFormatStringToHandle(10, Y_INFO2, GetColor(255, 255, 255), m_fontHandleNormal,
		"( %d + %d ), ( %d / %d / %d / %d), %d",
		m_scoreAccuracy, m_scoreTyping, m_countExcellent, m_countGood, m_countFair, m_countPoor, m_comboMax);
	//*/
#if 0
	{	/* 現在の歌詞の大きなかたまりを表示 */
		vector<Lyrics>::iterator i = m_lyricsPosition;
		while((*i).ch != ' '){	/* ブロックの最初でなければ戻る */
			i--;
		}
		string str;
		i++;
		if((*i).ch != '\n'){
			do{
				str += (*i).ch;
				i++;
			}while((*i).ch != ' ');	/* 次のブロックが始まるまで */
			DrawStringToHandle(10, 60, str.c_str(),
				GetColor(255, 255, 255), m_fontHandleNormal);
		}
	}
#endif
	
	/* 小節線、拍線を表示 */
	while(1){
		double timeDiff = time - (*m_beatLineDrawLeft).time;
		int posX = getDrawPosX(timeDiff);
		if(posX >= 0){
			/* 画面から出ていない */
			break;
		}
		/* すでに画面から出てしまったので、描画対象から削除 */
		m_beatLineDrawLeft++;
	}
	while(1){
		double timeDiff = time - (*m_beatLineDrawRight).time;
		int posX = getDrawPosX(timeDiff);
		if(posX >= W_WINDOW){
			/* 画面に入ってきていない */
			break;
		}
		/* 新しく画面に入ってきたので、描画対象に追加 */
		m_beatLineDrawRight++;
	}
	for(vector<BeatLine>::iterator i = m_beatLineDrawLeft; i != m_beatLineDrawRight; i++){
		double timeDiff = time - (*i).time;	/* 通り過ぎる位置になってから何秒後か */
		int posX = getDrawPosX(timeDiff);
		int posY = getDrawPosY(posX);
		if((*i).type == BEATLINE_BAR){
			DrawLine(posX, Y0_BAR  + posY, posX, Y1_BAR  + posY, GetColor(255, 255, 255));
		}else{	/* == BEATLINE_BEAT */
			DrawLine(posX, Y0_BEAT + posY, posX, Y1_BEAT + posY, GetColor(128, 128, 128));
		}
	}
	
	if(!m_challenge.test(CHALLENGE_STEALTH)){	/* Stealthなら表示しない */
		/* 円と、円の下の歌詞を表示 */
		
		int xMin = 0, xMax = W_WINDOW;	/* （Hidden, Sudden のための）描く左端と右端 */
		if(m_challenge.test(CHALLENGE_HIDDEN)){
			xMin = X_HIDDEN;
		}
		if(m_challenge.test(CHALLENGE_SUDDEN)){
			xMax = X_SUDDEN;
		}
		SetDrawArea(xMin, 0, xMax, H_WINDOW);	/* 描画範囲を制限 */
		
		for(;; m_lyricsDrawLeft++){
			if(!(*m_lyricsDrawLeft).isBlockStart){	/* 音節の最初でないなら進めてよい */
				continue;
			}
			double timeDiff = time - (*m_lyricsDrawLeft).timeJust;
			int posX = getDrawPosX(timeDiff);
			if(posX >= -(R_CIRCLE + 50)){	/* 円をはみ出る歌詞を考慮して広めに取る */
				/* 画面から出ていない */
				break;
			}
			/* すでに画面から出てしまったので、描画対象から削除 */
		}
		
		for(;; m_lyricsDrawRight++){
			if(!(*m_lyricsDrawRight).isBlockStart){	/* 音節の最初でないなら進めてよい */
				continue;
			}
			double timeDiff = time - (*m_lyricsDrawRight).timeJust;
			int posX = getDrawPosX(timeDiff);
			if(posX >= W_WINDOW + (R_CIRCLE + 50)){	/* 円をはみ出る歌詞を考慮して広めに取る */
				/* 画面に入ってきていない */
				break;
			}
			/* 新しく画面に入ってきたので、描画対象に追加 */
		}
		
		for(vector<Lyrics>::iterator i = m_lyricsDrawRight; i != m_lyricsDrawLeft;){
			i--;	/* [Left, Right)なので、ここでデクリメント */
			if(!(*i).isBlockStart){	/* 音節の最初でないなら無視 */
				continue;
			}
			double timeDiff = time - (*i).timeJust;	/* ちょうど打つ位置になってから何秒後か */
			int posX = getDrawPosX(timeDiff);
			int posY = getDrawPosY(posX);
			
			if((*i).ch == ' '){	/* 歌詞の切れ目 */
				if(timeDiff < 0.0){	/* 切れ目は判定位置に来るより前のときだけ表示 */
					DrawCircle(posX, Y_CIRCLE + posY, R_CIRCLE, GetColor(128, 128, 128), false);
					/* 灰色の円を流す */
				}
				continue;
			}
			
			char buf[256];
			int len = 0;
			{
				vector<Lyrics>::iterator j = i;
				do{	/* ブロック内の未タイプの文字をbufに格納 */
					if(!(*j).isTyped){
						buf[len++] = (*j).ch;
					}
					j++;
				}while(!(*j).isBlockStart);
			}
			buf[len] = '\0';
			if(len == 0){	/* すべてタイプされていた */
				continue;
			}
			
			int Color;
			if((*i).isScoringTarget){	/* まだタイミング点をもらってない */
				Color = GetColor(255, 0, 0);
			}else{
				Color = GetColor(0, 0, 255);
			}
			DrawCircle(posX, Y_CIRCLE + posY, R_CIRCLE - 1, Color, TRUE);	/* 流れる円 */
			DrawCircle(posX, Y_CIRCLE + posY, R_CIRCLE, GetColor(0, 0, 0), FALSE);	/* 流れる円の輪郭 */
			
			/* 円の下の歌詞 */
			if(!m_challenge.test(CHALLENGE_LYRICS_STEALTH)){	/* LyricsStealthなら表示しない */
				int strWidth = GetDrawStringWidthToHandle(buf, len, m_fontHandleNormal);
				DrawStringToHandle(posX - strWidth / 2, Y_LYRICS + posY, buf,
					GetColor(255, 255, 255), m_fontHandleNormal);	/* 流れる円に書かれる文字 */
			}
		}
		SetDrawArea(0, 0, W_WINDOW, H_WINDOW);	/* 描画範囲を元に戻す */
	}
	
	DrawCircle(X_CIRCLE, Y_CIRCLE, R_CIRCLE, GetColor(255, 255, 255), FALSE);	/* 判定位置の円 */
	
	{	/* メッセージ表示 */
		double timeDiff = time - m_textTime;
		if(timeDiff <= 0.5){	/* メッセージ（優良可不可）が0.5秒間表示される */
			//int strLen = strlen(m_text);
			//int strWidth = GetDrawStringWidthToHandle(m_text, strLen, m_fontHandleBig);
			int strX;
			//strX = 100 - strWidth / 2;
			strX = X_ACCURACY;
			int strY;	/* 表示されるY座標 */
			strY = Y_ACCURACY;
			if(timeDiff < 0.05){	/* 0.05秒後には定位置 */
				strY += ((0.05 - timeDiff) / 0.05) * 10;	/* 10だけ下から出てくる */
			}
			DrawStringToHandle(strX, strY, m_text,
				m_textColor, m_fontHandleBig);
		}
	}
	
	if(!m_challenge.test(CHALLENGE_LYRICS_STEALTH)){	/* LyricsStealth だと表示されない */
		/* 表示する歌詞を出力 */
		while(time >= (*m_lyricsKanjiPosition).timeEnd){	/* 表示終了しているなら次に進む */
			m_lyricsKanjiPosition++;
		}
		static int strWidth = -1;
		static char *strNext = "Next: ";
		if(strWidth < 0){
			strWidth = GetDrawStringWidthToHandle(strNext, strlen(strNext), m_fontHandleNormal);
		}
		DrawStringToHandle(X_LYRICS_KANJI - strWidth, Y_LYRICS_KANJI_NEXT, strNext,
			GetColor(255, 255, 255), m_fontHandleNormal);
		if(time >= (*m_lyricsKanjiPosition).timeBegin){	/* 表示開始しているなら */
			DrawStringToHandle(X_LYRICS_KANJI, Y_LYRICS_KANJI,
				(*m_lyricsKanjiPosition).str,
				GetColor(255, 255, 255), m_fontHandleNormal);	/* 出力 */
			DrawStringToHandle(X_LYRICS_KANJI, Y_LYRICS_KANJI_NEXT,
				(*(m_lyricsKanjiPosition + 1)).str,
				GetColor(255, 255, 255), m_fontHandleNormal);	/* Nextを出力 */
		}else{
			DrawStringToHandle(X_LYRICS_KANJI, Y_LYRICS_KANJI_NEXT,
				(*m_lyricsKanjiPosition).str,
				GetColor(255, 255, 255), m_fontHandleNormal);	/* Nextを出力 */
		}
	}
	
	if(!m_challenge.test(CHALLENGE_LYRICS_STEALTH)){	/* LyricsStealth だと表示されない */
		/* いま対象になっているローマ字歌詞(m_lyricsPositionからm_lyricsPositionEndまで)を表示 */
		/* Hidden や Stealth のとき、ヒントになるのを避けるため、 */
		/* 何かを打つまでは例外的に表示されない */
		if(!( (m_challenge.test(CHALLENGE_HIDDEN) || m_challenge.test(CHALLENGE_STEALTH))
				&& m_typeBufferLen == 0 )){
			char buf[256];
			int len = 0;
			for(vector<Lyrics>::iterator i = m_lyricsPosition; i != m_lyricsPositionEnd; i++){
				if((*i).ch != ' '){
					buf[len++] = (*i).ch;
				}
			}
			buf[len] = '\0';
			//int strWidth = GetDrawStringWidthToHandle(buf, len, m_fontHandleBig);
			DrawStringToHandle(X_CIRCLE - R_CIRCLE, Y_LYRICS_BIG, buf,
				GetColor(255, 255, 255), m_fontHandleBig);
		}
	}
	
	/* タイプした文字を表示 */
	{
		m_typeBuffer[m_typeBufferLen] = '\0';
		//int strLen = strlen(m_typeBuffer);
		//int strWidth = GetDrawStringWidthToHandle(m_typeBuffer, strLen, m_fontHandleBig);
		DrawStringToHandle(X_CIRCLE - R_CIRCLE, Y_BUFFER, m_typeBuffer,
			GetColor(255, 255, 255), m_fontHandleBig);
	}
	
	m_effect1.draw(time);	/* キー入力エフェクト */
}

void CTyping::drawResult(){
	double time = getTime();
	DrawFormatStringToHandle(30, 10, GetColor(255, 255, 255), m_fontHandleBig,
		"判定 :");
	if(time >= 0.5){
		DrawFormatStringToHandle(320, 10, COLOR_EXCELLENT, m_fontHandleNormal,
			"　優 : %5d / %d", m_countExcellent, m_countAll);
	}
	if(time >= 0.7){
		DrawFormatStringToHandle(320, 35, COLOR_GOOD, m_fontHandleNormal,
			"　良 : %5d / %d", m_countGood, m_countAll);
	}
	if(time >= 0.9){
		DrawFormatStringToHandle(320, 60, COLOR_FAIR, m_fontHandleNormal,
			"　可 : %5d / %d", m_countFair, m_countAll);
	}
	if(time >= 1.1){
		DrawFormatStringToHandle(320, 85, COLOR_POOR, m_fontHandleNormal,
			"不可 : %5d / %d", m_countPoor, m_countAll);
	}
	if(time >= 1.3){
		DrawFormatStringToHandle(320, 110, GetColor(255, 255, 255), m_fontHandleNormal,
			"通過 : %5d / %d", m_countPass, m_countAll);
	}
	if(time >= 2.3){
		if(m_comboMax >= 0){
			DrawFormatStringToHandle(30, 150, GetColor(255, 255, 255), m_fontHandleBig,
				"最大 %d コンボ", m_comboMax);
		}else{
			DrawFormatStringToHandle(30, 150, GetColor(255, 255, 0), m_fontHandleBig,
				"フルコンボ");
		}
	}
	if(time >= 3.3){
		DrawFormatStringToHandle(30, 210, GetColor(255, 255, 255), m_fontHandleBig,
			"得点 :");
	}
	if(time >= 3.5){
		DrawFormatStringToHandle(320, 210, GetColor(255, 255, 255), m_fontHandleNormal,
			"質 : %10d 点", m_scoreAccuracy);
	}
	if(time >= 3.7){
		DrawFormatStringToHandle(320, 235, GetColor(255, 255, 255), m_fontHandleNormal,
			"量 : %10d 点", m_scoreTyping);
	}
	if(time >= 4.2){
		DrawFormatStringToHandle(30, 275, GetColor(255, 255, 255), m_fontHandleBig,
			"総得点 : %10d 点",m_score);
	}
	if(!g_config.f_debugMode){	/* 通常時 */
		if(time >= 4.4){
			DrawFormatStringToHandle(30, 375, GetColor(255, 255, 255), m_fontHandleNormal,
				"名前を入力してください :");
		}
		m_typeBuffer[m_typeBufferLen] = '\0';
		DrawStringToHandle(60, 400, m_typeBuffer, GetColor(255, 255, 255), m_fontHandleBig);
		/* 入力された文字列を表示 */
	}
	if(m_phase == PHASE_FINISHED){
		if(m_rank >= 0){
			int strWidth;
			strWidth = GetDrawFormatStringWidthToHandle( m_fontHandleBig,
				"%d 位にランクインしました", m_rank + 1);
			int boxWidth = strWidth + 100;
			if(boxWidth > W_WINDOW){
				boxWidth = W_WINDOW;
			}
			DrawBox(320 - boxWidth / 2, 200, 320 + boxWidth/2, 280, GetColor(255, 255, 255), TRUE);
			DrawFormatStringToHandle(320 - strWidth / 2, 240 - 18,
				GetColor(0, 0, 0), m_fontHandleBig,
				"%d 位にランクインしました", m_rank + 1);
		}
	}
	return;
}

/* ============================================================ */

void main3(CTyping &typing){
//CCheck check;
//CCheck check2;
//CCheck check3;
	/* 描いて(draw)、キーボード処理(keyboard)、時間切れ処理など雑用(idle) */
	/* ・時間切れ処理はキーボード処理を終わらせた後にやらなければならない */
	/* ・最初の idle で音楽を読み込む（ある程度重い）　　ので、この順は固定 */
//check2.begin();
//check3.begin();
	ClearDrawScreen();
	while(1){
		if(ProcessMessage() == -1){
			return;
		}
//check.begin();
		typing.draw();
//check.end();
//check3.end();
		ScreenFlip();
//check2.end();
//check2.begin();
		ClearDrawScreen();
//check2.draw(16666/100, GetColor(32, 32, 64));
//check3.draw(16666/100, GetColor(64, 64, 32));
//check.draw(16666/100, GetColor(64, 32, 32));
//check3.begin();
		while(1){
			LONGLONG timeCount;
			char ch = GetKeyboardInput(timeCount);
			if(ch == 0){	/* バッファが空になればキー入力処理終了 */
				break;
			}
			if(ch == CTRL_CODE_ESC){	/* Escでゲームを中断 */
				goto L1;
			}
			//if(ch != 0 && ch >= CTRL_CODE_CMP){
			typing.keyboard(ch, timeCount);
		}
		if(!typing.idle()){
			break;
		}
	}
L1:
	return;
}

/* ============================================================ */

class CMusicInfo{
public:
	CMusicInfo();
	CMusicInfo(const CMusicInfo &info);
	~CMusicInfo();
	
	void load(CTyping &typing);
	
	void readRanking();
	
	void draw(int y, int color);
	void drawRanking(int y, int rankBegin, int rankLen);
	
	//void renewFont();
private:
	void createFont();
	void deleteFont();
public:
	char m_title[256];
	char m_artist[256];
	char m_fumenAuthor[256];
	char m_fumenFileName[256];
	char m_rankingFileName[256];
	
	int m_num;
	int m_numAll;
private:
	CRanking m_ranking;
	static int m_count;	/* 作られているCMusicInfoの個数 */
	static int m_fontHandleNormal;
	static int m_fontHandleTitle;
};

int CMusicInfo::m_count = 0;
int CMusicInfo::m_fontHandleNormal;
int CMusicInfo::m_fontHandleTitle;

CMusicInfo::CMusicInfo(){
//printfDx("%d++ ", m_count);
	if(m_count++ == 0){
		createFont();
	}
}

CMusicInfo::CMusicInfo(const CMusicInfo &info){
	*this = info;
//printfDx("%d++ ", m_count);
	if(m_count++ == 0){
		createFont();
	}
}

CMusicInfo::~CMusicInfo(){
	if(--m_count == 0){
		deleteFont();
	}
//printfDx("--%d ", m_count);
}

void CMusicInfo::load(CTyping &typing){
	typing.load(m_fumenFileName, m_rankingFileName);
}

/* ランキングを読み込む */
void CMusicInfo::readRanking(){
	m_ranking.open(m_rankingFileName);
	m_ranking.read();
	m_ranking.close();
}

void CMusicInfo::draw(int y, int color){	/* 曲情報をyから高さ60で描く */
	/* タイトル */
	DrawStringToHandle(30, y + (30 - 30/2), m_title, color, m_fontHandleTitle);
	
	int width;
	/* アーティスト */
	width = GetDrawFormatStringWidthToHandle(m_fontHandleNormal, "%s", m_artist);
	DrawFormatStringToHandle((W_WINDOW - 100) - width, y + (60-44), color, m_fontHandleNormal,
		"%s", m_artist);
	
	/* 譜面作者 */
	width = GetDrawFormatStringWidthToHandle(m_fontHandleNormal, "(譜面作成　%s)", m_fumenAuthor);
	DrawFormatStringToHandle((W_WINDOW - 100) - width, y + (60-22), color, m_fontHandleNormal,
		"(譜面作成　%s)", m_fumenAuthor);
	
	/* 通し番号 */
	width = GetDrawFormatStringWidthToHandle(m_fontHandleNormal, "%d/%d", m_num, m_numAll);
	DrawFormatStringToHandle((W_WINDOW - 30) - width, y + (60-44), color, m_fontHandleNormal,
		"%d/%d", m_num, m_numAll);
}

void CMusicInfo::drawRanking(int y, int rankBegin, int rankLen){
	m_ranking.draw(y, rankBegin, rankLen, m_fontHandleNormal);
}
/*
void CMusicInfo::renewFont(){
	deleteFont();
	createFont();
}
*/
void CMusicInfo::createFont(){
	m_fontHandleNormal = CreateFontToHandle(NULL, 16, 3);
	m_fontHandleTitle = CreateFontToHandle("ＭＳ 明朝", 30, 3, DX_FONTTYPE_ANTIALIASING);
}

void CMusicInfo::deleteFont(){	/* フォントを削除 */
	DeleteFontToHandle(m_fontHandleNormal);
	DeleteFontToHandle(m_fontHandleTitle);
}

/* ============================================================ */

vector<CMusicInfo> g_infoArray;

vector<CMusicInfo>::iterator prevInfo(vector<CMusicInfo>::iterator itr){
	if(itr == g_infoArray.begin()){
		itr = g_infoArray.end();
	}
	itr--;
	return itr;
}

vector<CMusicInfo>::iterator nextInfo(vector<CMusicInfo>::iterator itr){
	itr++;
	if(itr == g_infoArray.end()){
		itr = g_infoArray.begin();
	}
	return itr;
}

/* ============================================================ */

/* メイン画面（曲選択画面）を描く */
void drawMain(vector<CMusicInfo>::iterator infoArrayItr, int rankingPos,
		CChallenge &challenge, int fontHandleDefault){
	if(rankingPos >= 0){
		DrawBox(10, 10, W_WINDOW - 10, 350, GetColor(32, 32, 64), TRUE);
		
		/* 上下に水平線を書いておく */
		DrawLine(10, 10, W_WINDOW - 10, 10, GetColor(128, 128, 128));
		DrawLine(10, 350, W_WINDOW - 10, 350, GetColor(128, 128, 128));
		
		/* タイトルなど表示 */
		(*infoArrayItr).draw(20, GetColor(255, 255, 255));
		
		/* 水平線で区切る */
		DrawLine(40, 90, W_WINDOW - 40, 90, GetColor(64, 64, 64));
		
		/* ランキング表示（rankingPosからRANKING_DRAW_LEN個） */
		(*infoArrayItr).drawRanking(90 + 10, rankingPos, RANKING_DRAW_LEN);
	}else{
		DrawBox(10, 120, W_WINDOW - 10, 240, GetColor(32, 32, 64), TRUE);
		/* タイトルなど表示 */
		(*prevInfo(prevInfo(infoArrayItr))).draw(0, GetColor(85, 85, 85));
		          (*prevInfo(infoArrayItr)).draw(60, GetColor(128, 128, 128));
		                    (*infoArrayItr).draw(120, GetColor(255, 255, 255));
		          (*nextInfo(infoArrayItr)).draw(240, GetColor(128, 128, 128));
		(*nextInfo(nextInfo(infoArrayItr))).draw(300, GetColor(85, 85, 85));
		
		/* 水平線で区切る */
		DrawLine(10,  60, W_WINDOW - 10,  60, GetColor(85, 85, 85));
		DrawLine(10, 120, W_WINDOW - 10, 120, GetColor(128, 128, 128));
		DrawLine(40, 180, W_WINDOW - 40, 180, GetColor(64, 64, 64));
		DrawLine(10, 240, W_WINDOW - 10, 240, GetColor(128, 128, 128));
		DrawLine(10, 300, W_WINDOW - 10, 300, GetColor(85, 85, 85));
		
		/* ランキング表示（1位のみ） */
		(*infoArrayItr).drawRanking(180 + (60-48)/2, 0, 1);
	}
	DrawBox(0, 360, W_WINDOW, H_WINDOW - 25, GetColor(32, 32, 32), TRUE);
	DrawLine(0, 360, W_WINDOW, 360, GetColor(170, 170, 170));
	DrawLine(0, H_WINDOW - 25, W_WINDOW, H_WINDOW - 25, GetColor(170, 170, 170));
	
	DrawStringToHandle(10, 370, "↑/↓: 曲選択, ←/→: ランキング表示,   Tab: Window ←→ FullScreen,",
		GetColor(255, 255, 255), fontHandleDefault);
	
	{
		char *str = "Enter: 曲決定, R: リスト再読, Esc: 終了";
		int width = GetDrawStringWidthToHandle(str, strlen(str), fontHandleDefault);
		DrawStringToHandle(W_WINDOW - 10 - width, 390, str,
			GetColor(255, 255, 255), fontHandleDefault);	/* 右から10のところに描画 */
	}
	
	DrawStringToHandle(10, H_WINDOW - 65, "H: Hidden, S: Sudden, C: CircleStealth, L: LyricsStealth,",
		GetColor(255, 255, 255), fontHandleDefault);
	DrawStringToHandle(10, H_WINDOW - 45, "</>: Speed Down/Up, +/-: KeyShift",
		GetColor(255, 255, 255), fontHandleDefault);
	{
		char *str = "D: Reset to Default";
		int width = GetDrawStringWidthToHandle(str, strlen(str), fontHandleDefault);
		DrawStringToHandle(W_WINDOW - 10 - width, H_WINDOW - 65, str,
			GetColor(255, 255, 255), fontHandleDefault);	/* 右から10のところに描画 */
	}
	{
		char *str = "Q: sin, W: cos, E: tan,";
		int width = GetDrawStringWidthToHandle(str, strlen(str), fontHandleDefault);
		DrawStringToHandle(W_WINDOW - 10 - width, H_WINDOW - 45, str,
			GetColor(255, 255, 255), fontHandleDefault);	/* 右から10のところに描画 */
	}
	int color;
	if(challenge.test(CHALLENGE_STEALTH)){
		DrawStringToHandle(10, H_WINDOW - 20, "[ CircleStealth ]", GetColor(255, 128, 0), fontHandleDefault);
	}else{
		if(challenge.test(CHALLENGE_HIDDEN)){
			color = GetColor(255, 255, 0);
		}else{
			color = GetColor(64, 64, 64);
		}
		DrawStringToHandle(10, H_WINDOW - 20, "[ Hidden ]", color, fontHandleDefault);
		
		if(challenge.test(CHALLENGE_SUDDEN)){
			color = GetColor(255, 255, 0);
		}else{
			color = GetColor(64, 64, 64);
		}
		DrawStringToHandle(100, H_WINDOW - 20, "[ Sudden ]", color, fontHandleDefault);
	}
	if(challenge.test(CHALLENGE_LYRICS_STEALTH)){
		color = GetColor(255, 255, 0);
	}else{
		color = GetColor(64, 64, 64);
	}
	DrawStringToHandle(190, H_WINDOW - 20, "[ LStealth ]", color, fontHandleDefault);
	
	{
		char buf[256];
		sprintf(buf, "[ Speed x%3.1f ]", challenge.speed());
		if(challenge.speed() >= 2.0){
			color = GetColor(255, 128, 0);
		}else if(challenge.speed() > 1.0){
			color = GetColor(255, 255, 0);
		}else if(challenge.speed() < 1.0){
			color = GetColor(128, 255, 0);
		}else{
			color = GetColor(64, 64, 64);
		}
		DrawStringToHandle(300, H_WINDOW - 20, buf, color, fontHandleDefault);
	}
	
	{
		char buf[256];
		if(challenge.key() >= 6){
			sprintf(buf, "[ Key +%2d ]", challenge.key());
			color = GetColor(255, 128, 0);
		}else if(challenge.key() > 0){
			sprintf(buf, "[ Key +%2d ]", challenge.key());
			color = GetColor(255, 255, 0);
		}else if(challenge.key() < 0){
			sprintf(buf, "[ Key -%2d ]", -challenge.key());
			color = GetColor(128, 255, 0);
		}else{
			sprintf(buf, "[ Key %3d]", challenge.key());
			color = GetColor(64, 64, 64);
		}
		DrawStringToHandle(420, H_WINDOW - 20, buf, color, fontHandleDefault);
	}
	
	if(challenge.test(CHALLENGE_SIN)){
		color = GetColor(255, 255, 0);
	}else{
		color = GetColor(64, 64, 64);
	}
	DrawStringToHandle(540, H_WINDOW - 20, "[s]", color, fontHandleDefault);
	
	if(challenge.test(CHALLENGE_COS)){
		color = GetColor(255, 255, 0);
	}else{
		color = GetColor(64, 64, 64);
	}
	DrawStringToHandle(570, H_WINDOW - 20, "[c]", color, fontHandleDefault);
	
	if(challenge.test(CHALLENGE_TAN)){
		color = GetColor(255, 255, 0);
	}else{
		color = GetColor(64, 64, 64);
	}
	DrawStringToHandle(600, H_WINDOW - 20, "[t]", color, fontHandleDefault);
	
}

void editChallenge(CChallenge &challenge, char ch){
	switch(ch){
	case 'h':
	case 'H':
		challenge.flip(CHALLENGE_HIDDEN);
		if(challenge.test(CHALLENGE_HIDDEN)){
			challenge.reset(CHALLENGE_STEALTH);
		}
		break;
	case 's':
	case 'S':
		challenge.flip(CHALLENGE_SUDDEN);
		if(challenge.test(CHALLENGE_SUDDEN)){
			challenge.reset(CHALLENGE_STEALTH);
		}
		break;
	case 'c':
	case 'C':
		challenge.flip(CHALLENGE_STEALTH);
		if(challenge.test(CHALLENGE_STEALTH)){
			challenge.reset(CHALLENGE_HIDDEN);
			challenge.reset(CHALLENGE_SUDDEN);
		}
		break;
	case 'l':
	case 'L':
		challenge.flip(CHALLENGE_LYRICS_STEALTH);
		break;
	case '>':
		challenge.speedUp();
		break;
	case '<':
		challenge.speedDown();
		break;
	case '+':
		challenge.keyUp();
		break;
	case '-':
		challenge.keyDown();
		break;
	case 'q':
	case 'Q':
		challenge.flip(CHALLENGE_SIN);
		break;
	case 'w':
	case 'W':
		challenge.flip(CHALLENGE_COS);
		break;
	case 'e':
	case 'E':
		challenge.flip(CHALLENGE_TAN);
		break;
	case 'd':
	case 'D':
		challenge.reset();	/* 全チャレンジをなくす */
		break;
	default:
		break;
	}
}

bool main2(bool &isWindowMode){	/* falseを返せば、終了、trueを返せば、isWindowModeでウィンドウを必要なら変更して再読み込み */
	int fontHandleDefault = CreateFontToHandle(NULL, 16, 3);
	
	bool retValue;	/* 画面モードを変えて続ける場合true */
	
	CChallenge challenge;
	
	vector<CMusicInfo>::iterator infoArrayItr = g_infoArray.begin();
	int rankingPos = -RANKING_DRAW_LEN;
	/* 負なら、1位のみ表示。-RANKING_DRAW_LENにしておくと、RANKING_DRAW_LEN位ずつ表示するときと都合が良い。 */
	
	CTyping typing;
	
	while(1){
		if(ProcessMessage() == -1){
			retValue = false;
			break;
		}
		
		ClearDrawScreen();
		drawMain(infoArrayItr, rankingPos, challenge, fontHandleDefault);
		ScreenFlip();
		
		char ch = GetKeyboardInput();
		switch(ch){
		case CTRL_CODE_ESC:	/* 終了 */
			retValue = false;
			goto L1;
		case 'r':	/* 再読み込み */
		case 'R':
			retValue = true;
			goto L1;
		case CTRL_CODE_TAB:	/* Window ←→ FullScreen */
			isWindowMode = !isWindowMode;	/* Windowかどうかを逆にする */
			retValue = true;
			goto L1;
		case CTRL_CODE_CR:	/* ゲーム開始 */
			typing.setChallenge(challenge);	/* ロード前にチャレンジを設定 */
			(*infoArrayItr).load(typing);	/* ゲーム情報「に」譜面・ハイスコアのファイルをロード */
			/* ↑これをtyping.nanka(infoArrayItr)みたいにしたいんだが、いろいろ面倒 */
			main3(typing);
			typing.unload();	/* 終了処理 */
			
			rankingPos = -RANKING_DRAW_LEN;
			/* 戻ってきたときに、詳細ランキングから抜ける */
			/* （なんとなく、1位にランクインしたのにほかのところが表示されてたら悲しい） */
			
			(*infoArrayItr).readRanking();	/* ランキングは更新されているかもしれない */
			break;
		case CTRL_CODE_UP:	/* 1つ戻る。最初から1つ戻ると最後に。 */
			infoArrayItr = prevInfo(infoArrayItr);
			rankingPos = -RANKING_DRAW_LEN;
			break;
		case CTRL_CODE_DOWN:	/* 1つ進む。1つ進んで最後を超えると最初に。 */
			infoArrayItr = nextInfo(infoArrayItr);
			rankingPos = -RANKING_DRAW_LEN;
			break;
		case CTRL_CODE_LEFT:	/* 上位を表示 */
			if(rankingPos < 0){
				break;
			}
			rankingPos -= RANKING_DRAW_LEN;
			break;
		case CTRL_CODE_RIGHT:	/* 下位を表示 */
			if(rankingPos + RANKING_DRAW_LEN >= RANKING_LEN){
				break;
			}
			rankingPos += RANKING_DRAW_LEN;
			break;
		default:
			editChallenge(challenge, ch);
			break;
		}
	}
	
L1:
	DeleteFontToHandle(fontHandleDefault);
	return retValue;
}

/* flag はリストの読み込みが完了したかどうか */
void drawTitle(int fontHandleTitle, int fontHandleInfo, char *strInfo){
	//drawBox(0, 0, W_WINDOW, H_WINDOW, Color(32, 32, 32);
	char *strTitle = "UTyping";
	{
		int strWidth = GetDrawStringWidthToHandle(strTitle, strlen(strTitle), fontHandleTitle);
		DrawStringToHandle((W_WINDOW - strWidth) / 2, H_WINDOW / 3 - 24, strTitle,
			GetColor(255, 255, 255), fontHandleTitle, GetColor(170, 170, 170));
	}
	{
		int strWidth = GetDrawStringWidthToHandle(strInfo, strlen(strInfo), fontHandleInfo);
		DrawStringToHandle((W_WINDOW - strWidth) / 2, (H_WINDOW) * 2 / 3 - 12, strInfo,
			GetColor(255, 255, 255), fontHandleInfo);
	}
}

void main1(){
	bool isWindowMode = true;
	while(1){
		{
			SetDrawScreen(DX_SCREEN_BACK);
			/* ダブルバッファを用いる */
			
			int fontHandleTitle = CreateFontToHandle("ＭＳ 明朝", 48, 3, DX_FONTTYPE_ANTIALIASING_EDGE);
			int fontHandleInfo = CreateFontToHandle("ＭＳ 明朝", 24, 3, DX_FONTTYPE_ANTIALIASING);
			ClearDrawScreen();
			drawTitle(fontHandleTitle, fontHandleInfo, "Now loading...");
			ScreenFlip();
			
			bool isCorrectID = checkUTypingUserID();
			
			int count = 0;
			
			g_infoArray.clear();
			FILE *fp;
			fp = fopen("UTyping_list.txt", "r");
			if(fp == NULL){
				throw "UTyping_list.txt が開けません。";
			}
			char buf[256];
			while(fgetline(buf, fp) != NULL){
				FILE *fpInfo;
				CMusicInfo info;
				fpInfo = fopen(buf, "r");	/* 情報ファイルを開く */
				if(fpInfo == NULL){	/* 開けないファイルは無視 */
					continue;
				}
				char dirName[256];	/* ディレクトリは情報ファイルを基準とする */
				getDirFromPath(dirName, buf);
				char buf1[256], buf2[256], *chk;
				fgetline(info.m_title, fpInfo);	/* 1行目にタイトル */
				fgetline(info.m_artist, fpInfo);	/* 2行目に原作者 */
				fgetline(info.m_fumenAuthor, fpInfo);	/* 3行目に譜面作者 */
				fgetline(buf1, fpInfo);
				chk = fgetline(buf2, fpInfo);
				fclose(fpInfo);
				if(chk == NULL){	/* 全行読み込めなかった */
					continue;
				}
				sprintf(info.m_fumenFileName, "%s%s", dirName, buf1);	/* 4行目に譜面ファイル名 */
				sprintf(info.m_rankingFileName, "%s%s", dirName, buf2);	/* 5行目にハイスコアファイル名 */
				count++;
				info.m_num = count;	/* 通し番号をつける */
				g_infoArray.push_back(info);
			}
			fclose(fp);
			if(g_infoArray.empty()){
				"曲が一曲も存在しません。";
			}
			
			for(vector<CMusicInfo>::iterator i = g_infoArray.begin(); i != g_infoArray.end(); i++){
				(*i).readRanking();		/* ランキングを読み込み */
				(*i).m_numAll = count;
			}
			
			while(1){
				if(ProcessMessage() == -1){
					return;
				}
				SetDrawScreen(DX_SCREEN_BACK);
				ClearDrawScreen();
				drawTitle(fontHandleTitle, fontHandleInfo, isCorrectID ? "Press Enter key." : "Wrong UTyping user ID...");
				ScreenFlip();
				char ch = GetKeyboardInput();
				if(ch == 0){
					continue;
				}
				if(!isCorrectID){	/* IDが不正な場合は、どのキーを押しても終了。 */
					return;
				}
				if(ch == CTRL_CODE_ESC){	/* Escを押すと終了 */
					return;
				}
				if(ch == CTRL_CODE_TAB){	/* Tabを押すと、Window ←→ FullScreen */
					isWindowMode = !isWindowMode;	/* isWindowModeを変更して再起動 */
					break;
				}
				if(ch == CTRL_CODE_CR){	/* 改行が押されたら曲選択画面へ */
					if(!main2(isWindowMode)){	/* falseが戻った場合（終了するとき） */
						return;
					}
					break;	/* trueが戻った場合（再起動するとき） */
				}
			}
			DeleteFontToHandle(fontHandleTitle);
			DeleteFontToHandle(fontHandleInfo);
			
		}
		ChangeWindowMode(isWindowMode);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow){
	try{
//SetGraphMode(W_WINDOW, H_WINDOW, 32);
		ChangeWindowMode(TRUE);
		g_config.read();
		if(!g_config.f_debugMode){
			SetMainWindowText("UTyping (c)2007 tos");
		}else{	/* デバッグモードであることを上に出す */
			SetMainWindowText("UTyping (c)2007 tos -- (Debug Mode)");
		}

		if(DxLib_Init() == -1){
			return -1;
		}
		if(g_config.f_useMultiThread){
			InitializeCriticalSection(&g_csKeyboardInput);
			HANDLE hThreadKI;
			hThreadKI = newThread(KeyboardInputThread, NULL);
			main1();
			deleteThread(hThreadKI);
			DeleteCriticalSection(&g_csKeyboardInput);
		}else{
			main1();
		}
	}catch(int n){
		char buf[256];
		sprintf(buf, "%d 行目でエラーが発生しました。", n);
		outputError(buf);
	}catch(char *str){
		outputError(str);
	}
	DxLib_End();
	return 0;
}
