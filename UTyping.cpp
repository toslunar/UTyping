#include "DxLib.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <deque>
#include <bitset>
#include <list>

using namespace std;

/* ============================================================ */

/* 雑多な関数 */
#include "ututil.h"

/* ============================================================ */

#include "utuid.h"

/* ============================================================ */

#include "utchallenge.h"

/* ============================================================ */

#define COLOR_EXCELLENT GetColor(255, 255, 0)
#define COLOR_GOOD GetColor(0, 255, 0)
#define COLOR_FAIR GetColor(0, 128, 255)
#define COLOR_POOR GetColor(128, 128, 128)

#define COLOR_EXCELLENT2 GetColor(255, 255, 128)
#define COLOR_GOOD2 GetColor(128, 255, 128)
#define COLOR_FAIR2 GetColor(128, 192, 255)
#define COLOR_POOR2 GetColor(192, 192, 192)

#define COLOR_TIME GetColor(64, 192, 255)
#define COLOR_TIME2 GetColor(192, 192, 192)

#define COLOR_RED_ZONE GetColor(255, 32, 0)
#define COLOR_YELLOW_ZONE GetColor(224, 255, 0)
#define COLOR_BLUE_ZONE GetColor(0, 64, 255)
#define COLOR_CLEAR_ZONE GetColor(255, 255, 255)

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

#define GAUGE_COUNT 100
#define GAUGE_RED_ZONE 15
#define GAUGE_YELLOW_ZONE 45
#define GAUGE_CLEAR_ZONE 75
/*
failed:0
red   :(0,RED]
yellow:(RED,YELLOW]
blue  :(YELLOW,CLEAR]
clear :(CLEAR,COUNT]
*/

#define NAME_LEN 16
#define TYPE_BUFFER_LEN NAME_LEN
/* タイピングに用いるバッファの長さ */
/* 名前の最大長に等しい、変換のために最低4文字は必要 */

#define H_COMMENT 240
/* 【コメントの幅（高さ）はRANKING_LEN 位分を描画するときと同じ】だった */

#define H_MUSIC_INFO_SUB (6+H_COMMENT+4+H_PLAY_DATA+6)

#define MUSIC_INFO_HEIGHT 60
/* 曲情報の幅（高さ） */

/* ============================================================ */

/* 描画位置など */

#define CIRCLE_SPEED 250.0
/* 流れる円の標準の速さ（ドット/秒） */

#define X_SCORE (W_WINDOW - 25)
#define Y_SCORE 70

#define X_STAT_GAUGE (W_WINDOW - 10)
#define W_STAT_GAUGE 400
#define Y_STAT_GAUGE 10
#define H_STAT_GAUGE 40

#define X_GAUGE 90
//#define W_GAUGE 500
#define W_GAUGE_SEGMENT 5
#define W_GAUGE (W_GAUGE_SEGMENT * GAUGE_COUNT)
#define Y_GAUGE 455
#define H_GAUGE 10
#define W_GAUGE_PADDING 4
#define H_GAUGE_PADDING 2

#define X_TIME X_GAUGE
#define W_TIME W_GAUGE
#define Y_TIME 445
#define H_TIME 4

#define X_INFO 160
#define Y_INFO 10
//#define Y_INFO2 40

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

#define X_LYRICS_BIG (X_CIRCLE - R_CIRCLE)
#define Y_LYRICS_BIG 350

#define X_BUFFER (X_CIRCLE - R_CIRCLE)
#define Y_BUFFER 390

#define X_HIDDEN (X_CIRCLE + R_CIRCLE + 60)
/* Hiddenで見える左の限界 */
#define X_SUDDEN (X_HIDDEN + R_CIRCLE)
/* Suddenで見える右の限界 */
/* 両方かけたときに、ちょうど半円ぐらい見える */

#define SCALE_FUNCTION 60.0
/* 判定位置の円を原点とする座標と思うときの、長さ1に相当する画面の長さ */

#define SEC_FADEOUT 1.2
/* 終わってからスコア表示画面に移行する前にフェードアウトする時間 */

/* ============================================================ */

#define W_WINDOW 640
#define H_WINDOW 480

/* ============================================================ */

#define INFTY 1000000000

enum{
ID_EXCELLENT,
ID_GOOD,
ID_FAIR,
ID_POOR,
ID_PASS,
};

enum{
ID_FAILED_ZONE,
ID_RED_ZONE,
ID_YELLOW_ZONE,
ID_BLUE_ZONE,
ID_CLEAR_ZONE,
};

enum{
PHASE_READY,	/* 開始前 */
PHASE_MAIN,	/* メイン */
PHASE_FADEOUT,	/* MAIN終了後フェードアウト */
PHASE_RESULT,	/* スコア表示 */
PHASE_FINISHED,	/* 名前入力後（ハイスコア処理完了） */
PHASE_EXIT,	/* 終わったので消してもらう */
};

enum{
BEATLINE_BAR,
BEATLINE_BEAT,
};

/* ============================================================ */
/* グローバル変数とか */

int g_fontHandleDebug;

/* ============================================================ */

double myGetNowHiPerformanceCount(){
	static LARGE_INTEGER Frequency = {0, 0};
	static LARGE_INTEGER PerformanceCount0;	/* 最初に呼び出した時の値を保存 */
	LARGE_INTEGER PerformanceCount;
	if(Frequency.QuadPart == 0){
		if(!QueryPerformanceFrequency(&Frequency)){
			throw "高分解能パフォーマンスカウンタが取得できません。";
		}
		QueryPerformanceCounter(&PerformanceCount0);
	}
	QueryPerformanceCounter(&PerformanceCount);
	return (double)(PerformanceCount.QuadPart - PerformanceCount0.QuadPart) 
		/ Frequency.QuadPart;
}

/* ============================================================ */

#include "utcheck.h"

/* ============================================================ */

#include "utconfig.h"
/* Config */

/* ============================================================ */

static bool g_isUsingDxLib = false;

int myDxLib_Init(){
	int ret;
	if(!g_isUsingDxLib){
		ret = DxLib_Init();
		g_isUsingDxLib = true;
	}else{
		ret = 0;
	}
	
	/* ダブルバッファを用いる */
	SetDrawScreen(DX_SCREEN_BACK);
	ClearDrawScreen();
	
	g_check.begin(GetColor(16, 16, 16));
	return ret;
}


int myDxLib_End(){
	int ret;
	if(g_isUsingDxLib){
		ret = DxLib_End();
		g_isUsingDxLib = false;
	}else{
		ret = 0;
	}
	
	g_check.end();
	return ret;
}

int myScreenFlip(){
	static int count = 0;
	static double fps = -1.0;
	static double timeCountBegin;
	if(g_config.f_showFPS){
		if(count == 0){
			timeCountBegin = myGetNowHiPerformanceCount();
		}else if(count == 30){
			double timeCountEnd = myGetNowHiPerformanceCount();
			fps = 30.0 / (timeCountEnd - timeCountBegin);
			/* x[us] のとき 30/(x/1000000) = 30000000/x */
			/* てか、x[s]のとき30/x */
			count = 0;
			timeCountBegin = timeCountEnd;
		}
		count++;
		if(fps >= 0.0){
			char buf[256];
			sprintf(buf, "[%4.1ffps]", fps);
			int width = GetDrawStringWidthToHandle(buf, strlen(buf), g_fontHandleDebug);
			DrawStringToHandle(W_WINDOW - width, 0, buf, GetColor(255, 255, 255), g_fontHandleDebug);
			/* 右上にfps表示 */
		}
	}
	if(g_config.f_debugMode){	/* デバッグモードなら左上に書いておく */
		DrawStringToHandle(0, 0, "Debug Mode", GetColor(255, 0, 0), g_fontHandleDebug);
	}
	int ret0, ret1;
	g_check.rap(GetColor(16, 16, 16));
	ret0 = ScreenFlip();
	g_check.end();
	g_check.begin(GetColor(32, 32, 32));
	ret1 = ClearDrawScreen();
	g_check.rap(GetColor(16, 16, 16));
	if(g_config.f_showProcessTime){
		g_check.draw();
	}
	return (ret0 == -1 || ret1 == -1) ? -1 : 0;
}

/* ============================================================ */

/* UTypingUserIDを調べる */

bool checkUTypingUserID(){
	FILE *fp;
	fp = fopen("UTyping_ID.txt", "r");
	if(!fp){
		throw "UTyping_ID.txt が開けません。";
	}
	int ID[4];
	getUTypingUserID(ID);
	for(int i=0; i<4; i++){
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

#include "utkeyinput.h"
/*
char GetKeyboardInput(){
	return GetInputChar(TRUE);
}

char GetKeyboardInput(double &timeCount){
	return GetInputChar(TRUE);
}
//*/

/* configを参照して分岐 */
/*
char GetKeyboardInput(){
	if(g_config.f_useMultiThread){
		return MT_GetKeyboardInput();
	}else{
		return ST_GetKeyboardInput();
	}
}

char GetKeyboardInput(double &timeCount){
	if(g_config.f_useMultiThread){
		return MT_GetKeyboardInput(timeCount);
	}else{
		return ST_GetKeyboardInput(timeCount);
	}
}
*/

/* ============================================================ */

#include "utscore.h"
/* score,ranking */

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
	m_fontHandle = CreateFontToHandle("ＭＳ 明朝", CEFFECT1_FONT_SIZE, 2, DX_FONTTYPE_ANTIALIASING);
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

/* 後ろに文字が1文字ずつ追加され、前が消えていく文字列を対象にしたEffect */
/* 一応、文字列が長さを増やさず変化する可能性にも対応 */

#define CEFFECTSTR_SEC 0.3
/* 削除後消えるまでの時間 */

#define CEFFECTSTR_SEC2 0.08
/* 打ったエフェクトがかかる時間 */

#define CEFFECTSTR_H 10
/* 打ち切った後に落ちる距離 */

//#define CEFFECTSTR_V 15.0
/* 1秒で左向き速度が15.0文字分 */
/* これはinitのposVで設定できるようになった */

#define CEFFECTSTR_EX 2.4
/* エフェクトの文字の大きさの最初 */

class CEffectStr{
public:
	void init(int fontHandle, bool flagTypeEffect, double posV, double alphaCleared);
		/* 使用するフォントと、タイピングしたようなエフェクトをかけるかを最初に設定 */
		/* 表示であいた左を詰める速度（文字/秒）、削除直後の不透明度も設定 */
	void push(char ch, double time);	/* 末尾に一文字のデータを追加 */
	void push(char *str, double time);	/* 末尾に数文字のデータを追加 */
	void update(const char *str, int len, double time);
		/* 現在のデータを設定。主に前方が消えたことを教える */
		/* ここで、データの後方とstrが一致しなかったら直す */
	void clear();
		/* 現在のデータを単純に（消去エフェクトなしで）消す。既存のエフェクトは残す。 */
	
	void draw(int X, int Y, double time);
		/* 現在の文字列を参照しつつ表示 */
private:
	void drawChar(double X, double Y, const char *buf, double t, double alpha) const;
		/* 2バイト文字の関係上文字列で渡す */
	void updatePos(double time);
private:
	int m_fontHandle;
	bool m_flagTypeEffect;
	int m_fontWidth;
	
	double m_posV;
	double m_alphaCleared;
	
	struct Data{
		char ch;
		double time;	/* 追加された時刻 */
		double timeCleared;	/* 削除された時刻 */
		double x;	/* 削除済みの文字ついて意味を持ち、表示する位置 */
		//bool isCleared;
	};
	deque<Data> m_deq;	/* 現在のデータの文字たち */
	deque<Data> m_deqCleared;	/* 削除された文字たち */
	
	double m_pos;	/* 表示される位置がどれだけ左にずれるか */
	double m_lastTime;	/* 最後に更新した時間 */
};

void CEffectStr::init(int fontHandle, bool flagTypeEffect, double posV, double alphaCleared){
	m_fontHandle = fontHandle;
	{
		char str[2] = " ";
		m_fontWidth = GetDrawStringWidthToHandle(str, 1, m_fontHandle);
	}
	
	m_flagTypeEffect = flagTypeEffect;
	m_posV = posV * m_fontWidth;
	m_alphaCleared = alphaCleared;
	
	m_deq.clear();
	m_deqCleared.clear();
	
	m_pos = 0.0;
	//m_lastTime = 0.0;
}

void CEffectStr::push(char ch, double time){
	Data data;
	data.ch = ch;
#if 0
	if(m_deq.empty() || m_deq.back().isCleared){	/* 存在しているデータがない */
		data.x = 0.0;
	}else{	/* 最後のデータの後ろ */
		data.x = m_deq.back().x + m_fontWidth;	// + 5.0;
	}
#endif
	data.time = time;
	//data.isCleared = false;
	m_deq.push_back(data);
}

void CEffectStr::push(char *str, double time){
	while(*str){
		push(*str, time);
		str++;
	}
}


void CEffectStr::update(const char *str, int len, double time){
	updatePos(time);
	
	int diff = m_deq.size() - len;
		/* データのどこまでが消えたか */
	if(diff < 0){
		throw __LINE__;
	}
	/* 消えたデータを移動 */
	for(int i=0; i<diff; i++){
		m_deq[i].timeCleared = time;
		m_deq[i].x = m_pos;
		m_pos += m_fontWidth;
		m_deqCleared.push_back(m_deq[i]);
//printfDx("%10c, %10d, %10f, %10f\n", m_deq[i].ch, (int)m_deq[i].x, m_deq[i].time, m_deq[i].timeCleared);
	}
	for(int i=0; i<diff; i++){
		m_deq.pop_front();
	}
	
	if(len == 0){	/* データが空になったら、左端を0にする */
		m_pos = 0.0;
	}
	
	for(int i=0; i<len; i++){
		m_deq[i].ch = str[i];	/* 念のため新しいデータで上書き */
	}
	
}

void CEffectStr::clear(){
	m_deq.clear();
}

void CEffectStr::draw(int X, int Y, double time){
	updatePos(time);
	
	while(!m_deqCleared.empty() &&
			time - m_deqCleared.front().timeCleared >= CEFFECTSTR_SEC){	/* 消えてから十分長い時間がたった */
		m_deqCleared.pop_front();
	}
	
	char buf[3];
	for(deque<Data>::iterator itr = m_deqCleared.begin(); itr != m_deqCleared.end(); itr++){
		double x = itr->x;
		buf[0] = itr->ch;
		if(isJapanese1st(itr->ch)){
			itr++;
			if(itr == m_deqCleared.end()){
				break;
			}
			buf[1] = itr->ch;
			buf[2] = '\0';
		}else{
			buf[1] = '\0';
		}
		double xTime = (time - itr->timeCleared)/CEFFECTSTR_SEC;
		double t = 1.0 - xTime;
		if(t > 1.0) t = 1.0;
		t *= t*t;
		drawChar(X + x, Y + CEFFECTSTR_H * xTime * xTime,
			buf, time - itr->time, m_alphaCleared * t);
	}
	
	double pos = m_pos;
	for(deque<Data>::iterator itr = m_deq.begin(); itr != m_deq.end(); itr++){
		double x = pos;
		buf[0] = itr->ch;
		if(isJapanese1st(itr->ch)){
			itr++;
			if(itr == m_deq.end()){
				break;
			}
			buf[1] = itr->ch;
			buf[2] = '\0';
			pos += 2*m_fontWidth;
		}else{
			buf[1] = '\0';
			pos += m_fontWidth;
		}
		drawChar(X + x, Y, buf, time - itr->time, 1.0);
	}
}

void CEffectStr::drawChar(double X, double Y, const char *buf, double t, double alpha) const{
	if(m_flagTypeEffect && t < CEFFECTSTR_SEC2){
		t = 1.0 - t/CEFFECTSTR_SEC2;
		if(t > 1.0) t = 1.0;
		double ExRate = 1.0 + (CEFFECTSTR_EX - 1.0) * t * t;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 64 * alpha / ExRate);
		DrawExtendStringToHandle(
			X - m_fontWidth * (ExRate - 1.0) / 2.0 , Y - m_fontWidth * (ExRate - 1.0),
			ExRate, ExRate, buf, GetColor(255, 255, 255), m_fontHandle);
		/* フォントの縦:横=1:2という仮定を使っている */
		/* さらに、全角文字が来ないことが必要だが、タイピングではローマ字しか入らないのでOK */
	}
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 * alpha);
	DrawStringToHandle(X, Y, buf, GetColor(255, 255, 255), m_fontHandle);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
/*
if(alpha!=1.0){
DrawStringToHandle(X, Y, buf, GetColor(255, 0, 0), m_fontHandle);
}
*/
}

void CEffectStr::updatePos(double time){
	if(m_pos == 0.0){
		m_lastTime = time;
		return;
	}
	double timeDiff = time - m_lastTime;
	if(timeDiff < 0.0){
		return;
	}
	m_lastTime = time;
	m_pos -= timeDiff * m_posV;
	if(m_pos < 0.0){
		m_pos = 0.0;
	}
}

/* ============================================================ */

/* 良可判定のゲージ */
class CStatGauge{
public:
	void init();
	void draw();
	void inc(int ID_accuracy);
private:
	void drawBar(double val, int y, int h, int color) const;
	void drawLight(double Light, int y, int h, int color) const;
private:
	int m_val[4];	/* ゲージの現在描かれる長さ */
	double m_drawX[4];
	double m_drawV[4];	/* ゲージの現在描かれる長さの動く速度 */
	double m_rate;	/* ↑の長さ1に相当する画面上の長さ */
	double m_light[4];	/* ゲージのそれぞれを照らす光の強さ */
	
	int m_color[4][2];
};

void CStatGauge::init(){
	for(int i=0; i<4; i++){	/* 最初の値 */
		m_val[i] = 0;
		m_drawX[i] = 0.0;
		m_drawV[i] = 0.0;
		m_light[i] = 0.0;
	}
	m_rate = 20.0;	/* 最初の、長さ1あたりの表示する長さ */
	
	m_color[ID_EXCELLENT][0] = COLOR_EXCELLENT;
	m_color[ID_EXCELLENT][1] = COLOR_EXCELLENT2;
	m_color[ID_GOOD][0] = COLOR_GOOD;
	m_color[ID_GOOD][1] = COLOR_GOOD2;
	m_color[ID_FAIR][0] = COLOR_FAIR;
	m_color[ID_FAIR][1] = COLOR_FAIR2;
	m_color[ID_POOR][0] = COLOR_POOR;
	m_color[ID_POOR][1] = COLOR_POOR2;
}

void CStatGauge::draw(){	/* ゲージを表示 */
	int y = Y_STAT_GAUGE;
	int h = H_STAT_GAUGE / 4;
	
	for(int i=0; i<4; i++){	/* ゲージの指すべき値に近づける */
		//m_statGauge[i] += (x[i] - m_statGauge[i]) * 0.2;
		m_drawV[i] += 0.12 * (m_val[i] - m_drawX[i]);
		m_drawX[i] += m_drawV[i];
		/* 摩擦 */
		m_drawV[i] *= 0.7;
	}
		
	{
		double rate = 20.0;
		while(1){
			bool flag = true;
			for(int i=0; i<4; i++){
				if(m_val[i] * rate > W_STAT_GAUGE){
					flag = false;
				}
			}
			if(flag){
				break;
			}
			/* 超えるたびに0.75倍ずつしていき、超えなくなるまでやる */
			rate *= 0.75;
		}
		m_rate += (rate - m_rate) * 0.2;
	}
	
	
	{
		double x[4];	/* m_drawX[]から縮小処理してx[]に入れる */
		for(int i=0; i<4; i++){
			x[i] = m_drawX[i] * m_rate;
		}
		/* もともと濃い灰色で描画しないのは tan などへの配慮 */
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);	/* 不透明さ1/2 */
		DrawBox(X_STAT_GAUGE - W_STAT_GAUGE, y, X_STAT_GAUGE, y + 4 * h, GetColor(64, 64, 64), TRUE);
		/* それぞれの長さを表示 */
		for(int i=0; i<4; i++){
			drawBar(x[i], y+i*h, h, m_color[i][0]);
		}
	}
	
	/* 最近変更されたものを照らす */
	for(int i=0; i<4; i++){
		drawLight(m_light[i], y+i*h, h, m_color[i][1]);
		/*
		double Light = m_light[i];
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(192 * Light));
		Light *= 4.0;
		if(Light > 1.0){
			Light = 1.0;
		}
		int dh = (int)(h * 0.5 * (1.0 - Light)*(1.0 - Light));
		DrawBox(X_STAT_GAUGE - W_STAT_GAUGE, y+i*h + dh, X_STAT_GAUGE, y+(i+1)*h - dh, m_color[i][1], TRUE);
		*/
		m_light[i] *= 0.85;
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void CStatGauge::inc(int ID_accuracy){
	m_val[ID_accuracy]++;
	m_light[ID_accuracy] = 1.0;
}

void CStatGauge::drawBar(double val, int y, int h, int color) const{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 224);	/* 不透明さ7/8 */
	//DrawBox((int)(X_STAT_GAUGE - val + 0.5), y, X_STAT_GAUGE, y+h, color, TRUE);
	int i_val = floor(val);
	double f_val = val - i_val;
	DrawBox(X_STAT_GAUGE - i_val, y, X_STAT_GAUGE, y+h, color, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(224 * f_val));	/* 不透明さ(7/8)*小数部分 */
	DrawBox(X_STAT_GAUGE - i_val - 1, y, X_STAT_GAUGE - i_val, y+h, color, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void CStatGauge::drawLight(double Light, int y, int h, int color) const{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(192 * Light));
	Light *= 4.0;
	if(Light > 1.0){
		Light = 1.0;
	}
	int dh = (int)(h * 0.5 * (1.0 - Light)*(1.0 - Light));
	DrawBox(X_STAT_GAUGE - W_STAT_GAUGE, y + dh, X_STAT_GAUGE, y + h - dh, color, TRUE);
}

/* ============================================================ */

struct ConvertData{
public:
	ConvertData(const char *buf1, const char *buf3);
	bool match_front(const char *str);
	bool match(const char *str);
	bool get_unprocessed(char *buf);
public:
	char m_str[8];	/* 日本語 → ローマ字, 全角 → 半角 の変換先は 4 文字以内だから、5以上にしておけばよい */
	int m_len;
	/* str には条件に使われる文字列が入っているが、そのうち確定される長さ */
	/* 例: 「ん」に対して {"nb", 1}, 「っっ」に対して {"lltu", 4} など */
};

ConvertData::ConvertData(const char *buf1, const char *buf3){
	strcpy(m_str, buf1);
	m_len = strlen(buf1) - strlen(buf3);
	/* ここでは、データを信用して、buf3がbuf1の後方と一致することを仮定する。 */
}

bool ConvertData::match_front(const char *str){	/* データの前方とstrが一致するか */
	for(int i=0; str[i]!='\0'; i++){
		if(str[i] != m_str[i]){
			return false;
		}
	}
	return true;
}

bool ConvertData::match(const char *str){
	return (strcmp(m_str, str) == 0);
}

bool ConvertData::get_unprocessed(char *buf){
	/* 未確定ローマ字部分を取得 */
	strcpy(buf, m_str + m_len);
	return strlen(buf) != 0;
}

/* ============================================================ */

class CTrieNode{
public:
	CTrieNode();
	~CTrieNode();
	void insert(const char *str, const ConvertData &data);	/* strでたどった先にdataを入れる */
	CTrieNode *find(const char *str);
	CTrieNode *find(char ch);
private:
	CTrieNode *m_next[256];
public:
	/* もうprivateにするの面倒 */
	vector<ConvertData> m_data;
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

void CTrieNode::insert(const char *str, const ConvertData &data){
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

CTrieNode *CTrieNode::find(char ch){
	char buf[2];
	buf[0] = ch;
	buf[1] = '\0';
	return find(buf);
}

/* ============================================================ */

struct LyricsBlock{
	int lyricsNum;	/* Lyricsの配列の何番目からか */
	int lyricsLen;	/* 一かたまりの長さ（Byte） */
	double timeJust,timeUntil;	/* ちょうどの時間、（ここまでのみを）打つことができる最も遅い時間 */
	bool isValid;	/* 有効ならtrue、そうでない（スコア対象ではない管理用のいろいろ）ならfalse */
	int nTyped;	/* 何バイト打たれたか */
	bool isScoringTarget;	/* 現在タイミング判定をする対象であるか */
	int scoringCount;	/* ここより前に（これ自身を含まない）スコア対象のものがあったか */
	double scoringTime;
		/* タイミング判定をされた時刻、!isScoringTargetのとき以外は意味はない */
	//int r, g, b;	/* 打ち始めのエフェクトの色（scoringTimeが有効の時、有効） */
	double clearedTime;	/* Blockが打ち切られた時刻、負ならまだ */
		/* 区切りについては、区切りが意味を持たなくなった時刻 */
	double x, y, vx, vy, vz;
		/* （clearedTimeが非負の時）打ち切られた時の跳ねかた（向きはランダム）を保持 */
public:
	int lyricsPos();
};

int LyricsBlock::lyricsPos(){	/* 打ってない最初のもののLyrics配列での添え字を返す */
	return lyricsNum + nTyped;
}

struct Lyrics{
	char ch;	/* 変換された文字（あいう、。～abc123+-など）の1バイト */
	int blockNum;	/* LyricsBlockの配列の何番目に含まれるか */
	int blockPos;	/* そのなかで何バイト目か */
};

/* ============================================================ */

struct LyricsKanji{	/* 表示される歌詞 */
public:
	char str[256];	/* 歌詞 */
	double timeBegin;	/* 表示が始まる時間 */
	double timeEnd;	/* 表示が終わる時間 */
public:
	void draw(int x, int y, int fontHandle) const;
private:
	int getDrawColor(bool flag) const;
};

void LyricsKanji::draw(int x, int y, int fontHandle) const{
	char buf[256], *ptr1;
	const char *ptr2;
	bool flag = true;	/* 普通に書く(true)か、灰色で書く(false)か */
	ptr1 = buf;
	ptr2 = this->str;
	while(*ptr2 != '\0'){
		if(*ptr2 == '\\'){
			ptr2++;
			switch(*ptr2){
			case '\0':	/* *ptr2を0のままにしてwhileからこのまま抜ける */
				break;
			case '|':	/* ここで、色を変更 */
				{
					*ptr1 = '\0';
					int len = strlen(buf);
					int width = GetDrawStringWidthToHandle(buf, len, fontHandle);
					DrawStringToHandle(x, y, buf,
						getDrawColor(flag), fontHandle);
					x += width;
					ptr2++;
					ptr1 = buf;
					flag = !flag;
				}
				break;
			default:
				*ptr1++ = *ptr2++;
				break;
			}
		}else if(isJapanese1st(*ptr2)){
			*ptr1++ = *ptr2++;
			if(*ptr2 == '\0'){
				break;
			}
			*ptr1++ = *ptr2++;
		}else{
			*ptr1++ = *ptr2++;
		}
	}
	*ptr1 = '\0';
	DrawStringToHandle(x, y, buf,
		getDrawColor(flag), fontHandle);
	return;
}

int LyricsKanji::getDrawColor(bool flag) const{
	if(flag)return GetColor(255, 255, 255);
	else return GetColor(64, 64, 85);
}

/* ============================================================ */

struct BeatLine{
	int type;
	double time;
};

/* ============================================================ */

struct ReplayData{
	char ch;
	double time;
};

/* ============================================================ */

class CTyping{
public:
	CTyping();
	~CTyping();
private:
	void loadDictionary(const char *fileName);
	
	void addLyrics(const char *buf, double time);
	void addNullLyrics(double time, bool isLast);
public:
	void load(const char *fumenFileName, const char *rankingFileName);
	void unload();
	void setName(const char *name);
	void setChallenge(CChallenge &challenge);
private:
	void keyboard(char ch, double timeCount);
	void keyboard_1(char ch, double time);
	void keyboardEffect(char ch, double time);
	void updateLyricsBuffer(double time);
	void clearLyricsBuffer(vector<LyricsBlock>::iterator lyricsPos);
	
	bool idle(double timeCount);
private:
	void loadRanking(const char *fileName);
	void unloadRanking();
	
	void setText(const char *str, int color);
	
	void setTime();
	void synchronizeTime(int soundHandle, double frequencyRate);
	double getTime() const;
	double getTime(double timeCount) const;
	//void skipTime(double sec);
	
	void phase(int phaseNum);
	
	/* 入力処理。typeBufferがすでにある時に、chが入るかどうか、入ったら得点計算およびtypeBufferを更新 */
	/* ch = '\0'とすることにより、入る文字が存在するかどうかをテストできる */
	bool input(char ch, char *typeBuffer, int &typeBufferLen,
		vector<LyricsBlock>::iterator &lyricsPosition, bool f_successive, double time);
	bool input_1(char *typeBuffer, int &typeBufferLen, vector<Lyrics>::iterator &lyricsPos,
		bool f_successive, double time, bool isCheckOnly, bool isSaiki);	/* inputの実際に計算する部分 */
	
	void setTyped(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd);
	
	void scoreTyping(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd);
	void scoreAccuracy(vector<Lyrics>::iterator lyricsPos, bool f_successive, double time);
	void scoreAccuracySub(vector<LyricsBlock>::iterator lyricsPosition, int ID_accuracy);
	
	void initGauge();
	int getGaugeZone();
	void updateGauge();
	int getDrawGaugeColor(int pos, int cnt);
	double getDrawGaugeAlpha(int pos, int cnt);
	void drawGauge(bool isResult);

	void drawElapsedTime(double time);
	
	void drawTypeBuffer(double time);
	
	int getBlockStr(vector<LyricsBlock>::iterator itr, char *buf, bool f_getUntyped) const;
	int getLyricsBuffer(char *buf) const;
	
	int getDrawPosX(double timeDiff) const;
	int getDrawPosY(int x) const;
	
	void draw();
	void drawResult();
	
	void drawFlagAbort();
public:
	void mainLoop();
	void replayLoop();
private:
	bool saveReplay(const char *fileName, bool &f_confirm);
	bool loadReplay(const char *fileName);
public:
	bool saveReplayLoop();
	bool loadReplayLoop();
private:
	CTrieNode m_trie;
	
	vector<Lyrics> m_lyrics;
	
	vector<LyricsBlock> m_lyricsBlock;
	vector<LyricsBlock>::iterator m_lyricsPosition;
	
	vector<LyricsBlock>::iterator m_lyricsPositionEnd;
	/* 現在打っている歌詞がどこまでか、つまり、[m_lyricsPosition, m_lyricsPositionEnd) */
	/* この範囲の文字列が流れてくるところと別のところに表示される */
	CEffectStr m_lyricsBufferEffect;
	
	vector<LyricsBlock>::iterator m_lyricsBlockDrawLeft;
	vector<LyricsBlock>::iterator m_lyricsBlockDrawRight;
	/* drawで、直前に描画された範囲 */
	
	vector<LyricsKanji> m_lyricsKanji;
	vector<LyricsKanji>::iterator m_lyricsKanjiPosition;
	
	vector<BeatLine> m_beatLine;
	vector<BeatLine>::iterator m_beatLineDrawLeft;
	vector<BeatLine>::iterator m_beatLineDrawRight;
	
	char m_typeBuffer[TYPE_BUFFER_LEN + 1];
	int m_typeBufferLen;
	CEffectStr m_typeBufferEffect;
	//deque<double> m_typeBufferTime;
	
	int m_phase;
	
	double m_timeBegin;
	
	char m_name[NAME_LEN + 1];	/* プレーヤーの名前（設定されていたら） */
	
	bool m_flagLockName;
		/* 名前が既に設定されているときは、Escが一度押されるまで入力から保護する */
	
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
	
	int m_countClear;	/* 打ち切った個数 */
	
	CRanking m_ranking;	/* ハイスコアのデータ */
	int m_rank;	/* ハイスコアの中での順位、ランクインしなければ -1 */
	
	bool m_isReplay;
	vector<ReplayData> m_replayData;
	
	int m_scoreDraw;	/* 表示するスコア（の1/10）、本当のスコアと比べ遅延がある */
	
	CEffect1 m_effect1;	/* キー入力エフェクト */
	
	int m_gauge;	/* いわゆる魂ゲージみたいなもの */
		/* 太鼓の達人に影響を受けたわけじゃないんだからねっ */
	int m_gaugeMax;	/* 0以上これ以下の間で内部的には計算する */
	int m_gaugeLastCount;	/* 前回の判定数を保存 */
	int m_gaugeNewCount;	/* ↓のため新規判定数を保存 */
	int m_gaugeLastLost;	/* 前回の打ち逃し数を保存 */
	
	CStatGauge m_statGauge;
	
	char m_text[256];	/* 判定（およびコンボ数）を表示 */
	int m_textColor;
	double m_textTime;
	
	double m_timeFinished;	/* ゲームが終了し、フェードアウトが始まった（曲が始まってからの）時刻を保存 */
	double m_timeLength;	/* 譜面の最後、進行率表示に使用 */
	
	CChallenge m_challenge;
	
	char m_musicFileName[256];
	
	int m_idleCounter;	/* idleが何回目の呼び出しかを知るためのカウンタ */
	
	int m_fontHandleNormal;
	int m_fontHandleBig;
	
	int m_soundHandleMusic;
	
	bool m_flagFailed;	/* ゲージが切れて強制終了 */
	
	bool m_flagAbort;	/* Escで中断されかけたときにtrueになる */
};

CTyping::CTyping(){
	/* ローマ字辞書はコンストラクタで読む */
	if(!g_config.f_debugMode){	/* 通常のとき */
		loadDictionary("convert.dat");
	}
	strcpy(m_name, "");
	
	/* チャレンジを初期化（Defaultは何も変なことしない） */
	/* チャレンジは曲ロードのとき初期化しないので */
	m_challenge.reset();
	
	m_idleCounter = 0;
	
	m_fontHandleNormal = CreateFontToHandle(NULL, 16, 2);
	m_fontHandleBig = CreateFontToHandle("ＭＳ 明朝", 36, 2, DX_FONTTYPE_ANTIALIASING);
	
	m_soundHandleMusic = -1;	/* 何も読み込んだりしてはいない */
}

CTyping::~CTyping(){
	unload();
	DeleteFontToHandle(m_fontHandleNormal);	/* フォントを削除 */
	DeleteFontToHandle(m_fontHandleBig);
}

void CTyping::loadDictionary(const char *fileName){
	FILE *fp;
	fp = fopen(fileName,"r");
	if(fp == NULL){
		throw "ローマ字辞書が開けません。";
	}
	char buf[256], buf1[256], buf2[256], buf3[256];
	while(fgetline(buf, fp)!=NULL){
		int n=sscanf(buf, "%s%s%s", buf1, buf2, buf3);
		/* ローマ字、日本語、残るローマ字 */
		if(n<2){
			throw "ローマ字辞書の形式が不正です。";
		}
		int len;
		if(n == 2){
			strcpy(buf3, "");
		}
		m_trie.insert(buf2, ConvertData(buf1, buf3));
		/* 日本語をキーにローマ字のデータを入れる */
	}
	fclose(fp);
}

void CTyping::addLyrics(const char *buf, double time){
	if(!m_lyricsBlock.empty()){	/* 歌詞が存在している */
		LyricsBlock &lb = *(m_lyricsBlock.end() - 1);	/* そのなかで最後のものを操作 */
		if(lb.timeUntil == INFTY){	/* 終了時刻が設定されていなければ、現在に設定 */
			lb.timeUntil = time;
		}
	}
	
	LyricsBlock lb;
	lb.lyricsNum = m_lyrics.size();
	lb.lyricsLen = strlen(buf);
//printfDx("%d ", lb.lyricsLen);
	lb.timeJust = time;
	lb.timeUntil = INFTY;
	lb.isValid = true;
	lb.nTyped = 0;
	lb.isScoringTarget = true;
	lb.scoringCount = m_countAll;
	lb.clearedTime = -INFTY;
	Lyrics ly;
	ly.blockNum = m_lyricsBlock.size();
	for(int cnt=0; buf[cnt]!='\0'; cnt++){
		ly.ch = buf[cnt];
		ly.blockPos = cnt;
		m_lyrics.push_back(ly);
	}
	m_lyricsBlock.push_back(lb);
	
	m_countAll++;	/* 判定のある歌詞の数を数える */
}

void CTyping::addNullLyrics(double time, bool isLast){
	if(!m_lyricsBlock.empty()){	/* 歌詞が存在している */
		LyricsBlock &lb = *(m_lyricsBlock.end() - 1);	/* そのなかで最後のものを操作 */
		if(lb.timeUntil == INFTY){	/* 終了時刻が設定されていなければ、現在に設定 */
			lb.timeUntil = time;
		}
	}
	
	LyricsBlock lb;
	lb.lyricsNum = m_lyrics.size();
	lb.lyricsLen = 0;	/* 「" "があるから1」などをわざわざしなくてもよい */
		/* むしろ、文字列取得の時に0でないとゴミが入って困る */
	lb.isValid = isLast;	/* 最後にisValid = trueの番兵を置く。これは無害。 */
	lb.nTyped = 0;
	lb.timeJust = time;
	lb.timeUntil = time;
	lb.isScoringTarget = false;
	lb.scoringCount = m_countAll;
	lb.clearedTime = -INFTY;
	Lyrics ly;
	ly.blockNum = m_lyricsBlock.size();
	ly.ch = ' ';	/* 打つことができない' 'を番兵にする */
	m_lyricsBlock.push_back(lb);
	m_lyrics.push_back(ly);
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
	m_lyricsBlock.clear();
	
	m_lyricsKanji.clear();
	m_beatLine.clear();
	strcpy(m_musicFileName, "");
	
	/* 歌詞のかたまりの数（=m_count○○の和の最大値）を数えるために0に */
	/* また、LyricsBlockのscoringCountを設定していくため */
	m_countAll = 0;
	
	double frequencyRate = m_challenge.frequencyRate();
	/* KeyShiftによる周波数の倍率 */
	
	m_timeLength = -1;

	/* 読み込み開始 */
	
	char tmpBuf[256];
	char buf[256];
	double time;
	while(fgetline(tmpBuf, fp) != NULL){
		switch(tmpBuf[0]){
		case '+':	/* 打つ歌詞 */
		{
			if(g_config.f_debugMode && g_config.f_debugBeat){
				/* 打つ歌詞はすべて無視 */
				continue;
			}
			int n = sscanf(tmpBuf + 1, "%lf%s", &time, buf);
			if(n < 2){
				throw "[譜面] 書式が不正です。(+)";
			}
			time /= frequencyRate;
			addLyrics(buf, time);
		}
			break;
		case '*':	/* 表示する歌詞 */
		{
			char *ptr = strtok(tmpBuf + 1, " \t\n");	/* スペースで区切られるまでの部分を取り出す */
			if(ptr == NULL){
				throw "[譜面] 書式が不正です。(*)";
			}
			int n = sscanf(ptr, "%lf", &time);	/* 時刻を読む */
			if(n < 1){
				throw "[譜面] 書式が不正です。(*)";
			}
			time /= frequencyRate;
			ptr = strtok(NULL, "");	/* 残りの部分を取り出す */
			LyricsKanji lk;
			strcpy(lk.str, ptr);
			lk.timeBegin = time;
			lk.timeEnd = INFTY;	/* 終わりが設定されなければ、表示され続ける */
			m_lyricsKanji.push_back(lk);
		}
			break;
		case '=':	/* 小節線 */
		case '-':	/* 拍線 */
		{
			int n = sscanf(tmpBuf + 1, "%lf", &time);	/* 時刻を読む */
			if(n < 1){
				throw "[譜面] 書式が不正です。(=,-)";
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
			if(g_config.f_debugMode && g_config.f_debugBeat){
				/* 拍子線を"+"を打つ歌詞とする */
				addLyrics("+", time);
			}
		}
			break;
		case '/':	/* 区切り */
		{
			int n = sscanf(tmpBuf + 1, "%lf", &time);
			if(n < 1){
				throw "[譜面] 書式が不正です。(/)";
			}
			m_timeLength = time;	/* 毎回書き込んで、最後の区切りの位置を取得 */
			time /= frequencyRate;
			if(!m_lyricsKanji.empty()){	/* 表示する歌詞が存在している */
				LyricsKanji &lk = *(m_lyricsKanji.end() - 1);	/* そのなかで最後のものを操作 */
				if(lk.timeEnd == INFTY){	/* 終了時刻が設定されていなければ、現在に設定 */
					lk.timeEnd = time;
				}
			}
			if(g_config.f_debugMode && g_config.f_debugBeat){
				/* 打つ歌詞はすべて無視なので、区切りも無視 */
				continue;
			}
			
			addNullLyrics(time, false);
		}
			break;
		case '@':
			sprintf(m_musicFileName, "%s%s", directoryName, tmpBuf + 1);	/* 再生する音楽ファイル名を設定 */
			continue;
		default:
			continue;
		}
	}
	fclose(fp);

	if(m_countAll == 0){
		throw "[譜面] 打つ歌詞がありません。";
	}
	
	/* 打つ歌詞の番兵として最後に' ',INFTYを追加しておく */
	addNullLyrics(INFTY, false);
	/* !isValidなら1つ進むことへの対策として、isValidなのを追加しておく */
	addNullLyrics(INFTY, true);
	
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
	m_lyricsPosition = m_lyricsBlock.begin();
	while(!(*m_lyricsPosition).isValid){
		m_lyricsPosition++;
	}
	
	m_lyricsKanjiPosition = m_lyricsKanji.begin();
	
	m_lyricsPositionEnd = m_lyricsPosition;	/* 最初は範囲なしということにしておこう */
	
	/* タイプするべき歌詞へかけるタイピング効果なしエフェクトを初期化 */
	/* typeBufferと比べて小さめのエフェクト */
	m_lyricsBufferEffect.init(m_fontHandleBig, false, 50.0, 0.15);
	
	m_lyricsBlockDrawLeft = m_lyricsBlock.begin();
	m_lyricsBlockDrawRight = m_lyricsBlockDrawLeft;
	/* 最初のdrawで設定されるから何を入れてもよいのだが、 */
	/* 右にしか進まないので最初にしておかなければならない */
	
	/* 理由は同上 */
	m_beatLineDrawLeft = m_beatLine.begin();
	m_beatLineDrawRight = m_beatLine.begin();
	
	/* キー入力を初期化 */
	m_typeBufferLen = 0;
	
	/* タイプする位置へかけるタイピング効果ありエフェクトを初期化 */
	/* lyricsBufferと比べて大きめのエフェクト */
	m_typeBufferEffect.init(m_fontHandleBig, true, 15.0, 0.3);
	//m_typeBufferTime.clear();
	
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
	
	/* 打ち切りカウント初期化 */
	m_countClear = 0;
	
	strcpy(m_text, "");
	
	m_scoreDraw = 0;
	
	m_effect1.clear();	/* キー入力エフェクトを初期化 */
	
	initGauge();	/* ゲージ初期化 */
	
	m_statGauge.init();	/* 統計ゲージ初期化 */
	
	loadRanking(rankingFileName);
	/* ランキング読み込み */
	
	m_soundHandleMusic = -1;
	
	m_flagFailed = false;
	
	m_flagAbort = false;
}

void CTyping::unload(){
	if(m_soundHandleMusic != -1){
		DeleteSoundMem(m_soundHandleMusic);
		m_soundHandleMusic = -1;
	}
	unloadRanking();
}

void CTyping::setName(const char *name){
	strcpy(m_name, name);
}

void CTyping::setChallenge(CChallenge &challenge){
	m_challenge = challenge;
}

void CTyping::keyboard(char ch, double timeCount){
	if(m_phase < PHASE_RESULT && ch == CTRL_CODE_ESC){
	/* 結果画面より後ではEscによる終了をさせない */
		if(m_flagAbort){	/* これが2回目 */
			phase(PHASE_EXIT);
		}else{
			m_flagAbort = true;
		}
		return;
	}
	m_flagAbort = false;	/* 他のキーが押されたら中断を取り消す */
	
	if(m_phase == PHASE_READY){	/* 開始前なら */
		if(m_soundHandleMusic != -1){	/* 音楽がすでに読み込まれた */
			phase(PHASE_MAIN);
			
			if(g_config.f_debugMode && g_config.debugTime > 0){	/* 途中から始めるデバッグモードのとき */
				SetSoundCurrentTime(g_config.debugTime, m_soundHandleMusic);
				/* 【 SetSoundCurrentTime を使っているので注意が必要かもしれない】 */
				PlaySoundMem(m_soundHandleMusic, DX_PLAYTYPE_BACK, FALSE);	/* 音楽を流し始める */
			}else{
				/* 通常時 */
				PlaySoundMem(m_soundHandleMusic, DX_PLAYTYPE_BACK);	/* 音楽を流し始める */
			}
			
			setTime();	/* 始まった時刻を覚える */
			
			synchronizeTime(m_soundHandleMusic, m_challenge.frequencyRate());
			/* 一応タイマー調節。どちらかといえば、再生開始位置を変更したときに有用。 */
		}
		return;
	}
	if(m_phase == PHASE_RESULT){	/* スコア表示中なら */
		/* 名前記入のためにバッファを読み書き */
		double time = getTime(timeCount);	/* キーボードを押した時までの時間を取得 */
		
		if(ch == ' '){	/* スペースはアンダーバーに変換（スペースのみの違いを作らないため） */
			ch = '_';
		}
		int ret;
		if(m_flagLockName){
			char buf[1];
			int len = 0;
			ret = editBuffer(ch, buf, len);	/* ダミーを操作 */
			/* EnterやEscが押されたかどうかのみが重要 */
		}else{
			ret = editBuffer(ch, m_typeBuffer, m_typeBufferLen);
		}
		switch(ret){
		case EDIT_BUFFER_OK:
			if(time < 4.0){	/* 結果がすべて出るまで確定させない */
				//skipTime(1.0);	/* 待つのが嫌な人のために…… */
				return;
			}
			if(m_challenge.isEasy() || m_isReplay || g_config.f_debugMode){
				/* 簡単になるオプションを使用した場合、ランキングにのせない */
				/* リプレイ時やデバッグ時は記録しない */
				phase(PHASE_FINISHED);
			}else{	/* リプレイ時・デバッグモード時 */
				if(m_typeBufferLen > 0){	/* 名前が入力されているか確認 */
					phase(PHASE_FINISHED);
				}
			}
			break;
		case EDIT_BUFFER_CANCEL:
			if(m_flagLockName){
				m_flagLockName = false;	/* 名前の書き換え防止をなくす */
			}else{
				/* 名前入力を全消去 */
				strcpy(m_typeBuffer, "");
				m_typeBufferLen = 0;
			}
			break;
		}
		return;
	}
	if(m_phase == PHASE_FINISHED){	/* ランキング（何位にランクインしたか）表示中なら */
		if(ch == CTRL_CODE_CR){	/* Enterで終了 */
			phase(PHASE_EXIT);
		}
		return;
	}
	
	if(m_phase == PHASE_MAIN){	/* ゲームメインのときの処理 */
		double time = getTime(timeCount);	/* キーボードを押した時までの時間を取得 */
		keyboard_1(ch, time);
	}
}

void CTyping::keyboard_1(char ch, double time){
	/* 任意のものを打ったことにできる'\0'は入力に来ないはずだが、来ると非常に困る */
#if 0
	if(ch == '\0' || ch == ' '){	/* ' 'はどうせ入らないのだが、悪影響を与える可能性が高いので */
#endif
	if(ch < CTRL_CODE_CMP || ch == ' '){	/* ' 'を入れるようなことはしない */
	/* 文字じゃない場合は無視してよい */
		return;
	}
	
//printf("%c %f\n", ch, time);
#if 0
	/* input内でチェックすることになった */
	if(time < (*m_lyricsPosition).timeJust - SEC_POOR){
		/* poorより前なので打てない */
		return;
	}
#endif
	
	if(input(ch, m_typeBuffer, m_typeBufferLen, m_lyricsPosition, true, time)){
//printf("ok\n");
	/* その入力が現在の位置で入った */
		if(!m_isReplay){	/* リプレイに記録 */
			ReplayData rd;
			rd.ch = ch;
			rd.time = time;
			m_replayData.push_back(rd);
		}
		
		while(!(*m_lyricsPosition).isValid){	/* 歌詞の切れ目を指しているなら */
			m_lyricsPosition->clearedTime = time;
			m_lyricsPosition++;	/* 進める */
		}
		
		/* キー入力エフェクト */
		keyboardEffect(ch, time);
		
		updateLyricsBuffer(time);
		
		updateGauge();
		return;
	}
	
	char tmpBuffer[TYPE_BUFFER_LEN + 1];
	int tmpLen;
	vector<LyricsBlock>::iterator tmpLyricsPosition;
	tmpLen = 0;
	tmpLyricsPosition = m_lyricsPosition;
	
	while(1){
//printf("loop\n");
		tmpLyricsPosition++;	/* 必ず1つ以上飛ばす必要があるので最初に++ */
		if(time < (*tmpLyricsPosition).timeJust - SEC_POOR){
			/* poor範囲に入っているところより外に探索が出た */
			break;
		}
		if(!(*tmpLyricsPosition).isScoringTarget){
			/* すでにタイミングが採点されたところや、区切り */
			continue;
		}
		vector<LyricsBlock>::iterator lyricsPos = tmpLyricsPosition;
			/* inputでtmpLyricsPositionが変更されうるので控えておく。 */
			/* 【アルゴリズム変えたい】 */
		
		if(input(ch, tmpBuffer, tmpLen, tmpLyricsPosition, false, time)){
//printf("ok\n");
		/* 新しい位置でその入力が入った */
			if(!m_isReplay){	/* リプレイに記録 */
				ReplayData rd;
				rd.ch = ch;
				rd.time = time;
				m_replayData.push_back(rd);
			}
			
			/* 新しいデータを書き込む */
			memcpy(m_typeBuffer, tmpBuffer, tmpLen);
			m_typeBufferLen = tmpLen;
			m_lyricsPosition = tmpLyricsPosition;
			while(!(*m_lyricsPosition).isValid){	/* 歌詞の切れ目を指しているなら */
				m_lyricsPosition->clearedTime = time;
				m_lyricsPosition++;	/* 進める */
			}
			
			/* キー入力エフェクト */
			keyboardEffect(ch, time);
			
			clearLyricsBuffer(lyricsPos);	/* 古い内容を一度消去してから */
			updateLyricsBuffer(time);	/* 新しく設定 */
			
			updateGauge();
			return;
		}
	}
//printf("ng\n");
	return;
}

void CTyping::keyboardEffect(char ch, double time){
	m_effect1.insert(X_CIRCLE, Y_CIRCLE, ch, GetColor(170, 170, 255), time);
	//m_typeBufferTime.push_back(time);
	m_typeBufferEffect.push(ch, time);
	m_typeBufferEffect.update(m_typeBuffer, m_typeBufferLen, time);
	return;
}

void CTyping::updateLyricsBuffer(double time){
	/* m_lyricsPositionEndの更新とそれに伴うエフェクトの情報の追加 */
	{
		vector<LyricsBlock>::iterator lastPos = m_lyricsPositionEnd;
		
		if(m_lyricsPositionEnd < m_lyricsPosition){
			m_lyricsPositionEnd = m_lyricsPosition;
		}
		
		if(m_lyricsPositionEnd->timeJust != INFTY){
#if 0
			while(m_lyricsPositionEnd->timeJust != INFTY &&
						/* 端を超えないようにチェック */
					( time >= m_lyricsPositionEnd->timeJust
						/* まだ、ちょうどの位置に達していない最初の歌詞まで進む */
					|| (m_lyricsPositionEnd->isValid && !m_lyricsPositionEnd->isScoringTarget) )){
						/* 採点済み、つまり、打ちかけのものを含むように範囲を増やす */
				m_lyricsPositionEnd++;
			}
#endif
			if(!(m_challenge.test(CHALLENGE_HIDDEN) || m_challenge.test(CHALLENGE_STEALTH))){
			/* Hidden, Stealthでは、時間経過により打つべきになったことを教えない */
				while(time >= m_lyricsPositionEnd->timeJust){
					/* まだ、ちょうどの位置に達していない最初の歌詞まで進む */
					m_lyricsPositionEnd++;
				}
			}
			
			while(m_lyricsPositionEnd->isValid && !m_lyricsPositionEnd->isScoringTarget){
				/* 採点済み、つまり、打ちかけのものを含むように範囲を増やす */
				m_lyricsPositionEnd++;
			}
		}
		
		while(lastPos != m_lyricsPositionEnd){
			char buf[256];
			getBlockStr(lastPos, buf, false);
			if(lastPos->timeJust < time){
				m_lyricsBufferEffect.push(buf, lastPos->timeJust);
			}else{
				m_lyricsBufferEffect.push(buf, time);
			}
			lastPos++;
		}
	}
	
	{	/* どの部分が実際に残っているかを教える */
		char buf[256];
		int len;
		len = getLyricsBuffer(buf);
		
		m_lyricsBufferEffect.update(buf, len, time);
	}
}

/* どこまでの内容が消えたかを伝える */
void CTyping::clearLyricsBuffer(vector<LyricsBlock>::iterator lyricsPos){
	/* LyricsBufferの内容を消去。これによって残像を発生させない。 */
	/* もともとあった残像は残す。 */
	m_lyricsPositionEnd = lyricsPos;
	m_lyricsBufferEffect.clear();
}

/* ------------------------------------------------------------ */

bool CTyping::idle(double timeCount){	/* 続けるなら（終了するとき以外） true を返す */
	if(m_phase == PHASE_EXIT){	/* 終了することになっている */
		return false;
	}
	if(m_phase == PHASE_READY){	/* 開始していなければ特にすることは無い */
		if(m_soundHandleMusic == -1){	/* まだ音楽を読み込んでなかったら読み込み */
			SetCreateSoundDataType(g_config.loadSoundType);	/* configをみて読み込み方を決定 */
			m_soundHandleMusic = LoadSoundMem(m_musicFileName);
			if(m_soundHandleMusic == -1){
				throw "[譜面] 曲ファイルが再生できません。";
			}
			if(g_config.volume >= 0){
				/* 音量が指定されていれば、音量設定 */
				ChangeVolumeSoundMem(g_config.volume, m_soundHandleMusic);
			}
			
			if(m_challenge.key() != 0){	/* キーシフトが行われている */
				int oldFrequency = GetFrequencySoundMem(m_soundHandleMusic);	/* 元の再生周波数を読み込み */
				int newFrequency = (int)(oldFrequency *m_challenge.frequencyRate());
					/* 変更する倍率をかけて新しい周波数を出す */
				SetFrequencySoundMem(newFrequency, m_soundHandleMusic);	/* 変更 */
			}
		}
		return true;
	}
	if(m_phase == PHASE_FADEOUT){
		double time = getTime(timeCount);
		if(time >= m_timeFinished + SEC_FADEOUT){	/* 最後の音符が通過した後フェードアウトが終了した */
			phase(PHASE_RESULT);	/* 結果画面にうつる */
			if(m_phase == PHASE_EXIT){	/* （結果画面がなしで）終了することになった */
				return false;
			}
			return true;
		}
	}
	
	if(m_phase != PHASE_MAIN){	/* ここから先はゲームメインのときの処理 */
		return true;
	}
	
	if(m_flagFailed){	/* ゲージ切れで強制終了 */
		phase(PHASE_FADEOUT);
		return true;
	}
	
//printf("idle[");
	double time = getTime(timeCount);
	
#if 0
	if((*m_lyricsPosition).timeJust == INFTY){
		/* 最後のisValid = trueなのに番兵なやつのところにいる */
		phase(PHASE_FADEOUT);
		return true;
	}
	
	if(flag){	/* 毎回チェックすると重いかもしれないので、必要なときに限定する */
		/* 「何かのキーを押せば現在の位置で入力可能」ではない */
		if(!input('\0', m_typeBuffer, m_typeBufferLen, m_lyricsPosition, true, time)){
			/* chを'\0'として、打てる入力が存在するかチェック */
			/* f_successiveがtrueなのに特に意味はない */
			/* 入力が切れたものは飛ばす（m_lyricsPositionが進んで、コンボが切れる） */
			while(time >= (*m_lyricsPosition).timeUntil){
				m_lyricsPosition++;
				
				/* 本当にm_lyricsPositionが進んだら、ここに来る */
				m_combo = 0;
				m_typeBufferLen = 0;
				/* 入力中のはもう使われることはない（から、ここで消してしまってよい） */
			}
			
			updateGauge();
		}
	}
#endif
#if 0
	if(!input('\0', m_typeBuffer, m_typeBufferLen, m_lyricsPosition, true, time)){
		/* chを'\0'として、打てる入力が存在するかチェック */
		/* f_successiveがtrueなのに特に意味はない */
		/* 入力が切れたものは飛ばす（m_lyricsPositionが進んで、コンボが切れる） */
//printfDx(".");
		m_combo = 0;
		m_typeBufferLen = 0;
		/* 入力中のはもう使われることはない（から、ここで消してしまってよい） */
		do{
		/* 【別にwhileでも同じだが、譜面のミスで「絶対打てない文字」があった場合飛ばせる】 */
//printfDx("+\n");
			m_lyricsPosition++;
		}while(time >= (*m_lyricsPosition).timeUntil);
		updateGauge();
	}
#endif
	while(1){
		if(m_lyricsPosition->timeJust == INFTY){
			/* 最後のisValid = trueなのに番兵なやつのところにいる */
			//m_lyricsPositionEnd = m_lyricsPosition;
			phase(PHASE_FADEOUT);
			return true;
		}
		/* chを'\0'として、打てる入力が存在するかチェック */
		/* f_successiveがtrueなのに特に意味はない */
		if(input('\0', m_typeBuffer, m_typeBufferLen, m_lyricsPosition, true, time)){
			/* 入力可能ならループを抜ける */
			break;
		}
		/* 入力が切れたものは飛ばす（m_lyricsPositionが進んで、コンボが切れる） */
		m_combo = 0;
		m_typeBufferLen = 0;
		/* 入力中のはもう使われることはない（から、ここで消してしまってよい） */
		m_lyricsPosition++;
		while(!(*m_lyricsPosition).isValid){
			m_lyricsPosition++;
		}
		clearLyricsBuffer(m_lyricsPosition);
		
		updateGauge();
	}
	
	// lyricsPositionEndの更新はupdateLyricsBufferに移動。そもそもdrawでやるからここでやらない。
	
//printf("]\n");
	if(m_idleCounter++ % 20 == 0){	/* 20回に1回実行 */
		/* 時刻を音楽と同期 */
		synchronizeTime(m_soundHandleMusic, m_challenge.frequencyRate());
	}
	
	return true;
}

/* ------------------------------------------------------------ */

void CTyping::loadRanking(const char *fileName){
	m_ranking.open(fileName);
	m_ranking.read();
}

void CTyping::unloadRanking(){
	m_ranking.close();
}

/* ------------------------------------------------------------ */

void CTyping::setText(const char *str, int color){	/* 表示文字列を設定 */
	m_textTime = getTime();
	strcpy(m_text, str);
	m_textColor = color;
}

/* ------------------------------------------------------------ */

void CTyping::setTime(){	/* 開始時刻を設定 */
	m_timeBegin = myGetNowHiPerformanceCount();
}

/* 音楽のスタートと開始時刻が等しくなるように、開始時刻を調整 */
void CTyping::synchronizeTime(int soundHandle, double frequencyRate){
	if(soundHandle == -1 || CheckSoundMem(soundHandle) != 1){
	/* 音楽が読み込まれていないか、再生されていないかのとき */
		return;
	}
	
	double musicTime;
	musicTime = (GetSoundCurrentTime(soundHandle) / 1000.0) / frequencyRate;
	/* 【 GetSoundCurrentTime を使っているので注意が必要かもしれない】 */
	/* 曲の周波数を変えてる時は、実際に流れた時間をとるため、周波数の比だけ割る */
	double realTime = myGetNowHiPerformanceCount() - m_timeBegin;
	double diff = realTime - musicTime;
//printfDx("%.4f %.4f\n", musicTime, realTime);
	
	/* わずかなずれ（GetSoundCurrentTimeでとれないような）は無視 */
	if(diff >= 0.0005){
		diff -= 0.0005;
	}else if(diff <= -0.0005){
		diff += 0.0005;
	}else{
		return;
	}
	
	m_timeBegin += diff;
}

double CTyping::getTime() const{	/* 開始時刻からの経過秒を取得 */
	return myGetNowHiPerformanceCount() - m_timeBegin;
}

double CTyping::getTime(double timeCount) const{	/* timeCountの開始時刻からの経過秒を取得 */
	return timeCount - m_timeBegin;
}

#if 0
void CTyping::skipTime(double sec){	/* 経過時間をsecだけ増やす */
	m_timeBegin -= sec;	/* 開始時刻が早かったことにすれば解決 */
}
#endif

/* ------------------------------------------------------------ */

void CTyping::phase(int phaseNum){
	if(m_phase == PHASE_MAIN){	/* プレイ時間を加算 */
		if(!(m_isReplay || g_config.f_debugMode)){	/* 通常時のみ */
		/* m_challenge.isEasy()でも数える */
			double playTime = getTime();
			m_ranking.addPlayTime(playTime);
		}
	}
	
	if(phaseNum == PHASE_FADEOUT){	/* 終わったのでフェードアウトする */
		m_timeFinished = getTime();	/* 終わった時刻を保存 */
	}else if(phaseNum == PHASE_RESULT){	/* 終了して、スコア表示に */
		if(m_flagFailed){
			phase(PHASE_EXIT);	/* 強制終了時は、スコア表示なし */
			return;
		}
		setTime();	/* 表示開始されてからの時間を利用するためにセット */
		
		strcpy(m_typeBuffer, m_name);	/* バッファに名前を入れる。 */
		m_typeBufferLen = strlen(m_name);
		if(m_typeBufferLen > 0){	/* 名前が既に入力されているなら、保護する */
			m_flagLockName = true;
		}else{
			m_flagLockName = false;
		}
		
		m_flagAbort = false;	/* Escによる中断を取り消す */
		
		/* 通過した個数を数えておく */
		m_countPass = m_countAll - m_countExcellent - m_countGood - m_countFair - m_countPoor;
		
		/* コンボ数が判定位置数で終了した（最後の文字もちゃんと打ち切った） */
		if(m_combo == m_countAll){
			m_comboMax = -1;	/* 最大コンボ数を「フルコンボ」としておく */
		}
	}else if(phaseNum == PHASE_FINISHED){
		if(m_challenge.isEasy() || m_isReplay || g_config.f_debugMode){
			/* 簡単になるオプションを使用した場合、ランキングにのせない */
			/* リプレイ時やデバッグ時は記録しない */
			phase(PHASE_EXIT);
			return;
		}else{
			CScore score(m_typeBuffer, m_score, m_scoreAccuracy, m_scoreTyping,
				m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass,
				m_countAll, m_comboMax, m_challenge);
			int achievement = score.getLevel();
			if(achievement == SCORE_NO_DATA){	/* ゲージに依存する達成度 */
				switch(getGaugeZone()){
				case ID_FAILED_ZONE:
					achievement = SCORE_FAILED; break;
				case ID_RED_ZONE:
					achievement = SCORE_RED_ZONE; break;
				case ID_YELLOW_ZONE:
					achievement = SCORE_YELLOW_ZONE; break;
				case ID_BLUE_ZONE:
					achievement = SCORE_BLUE_ZONE; break;
				case ID_CLEAR_ZONE:
					achievement = SCORE_CLEAR; break;
				}
			}

			m_rank = m_ranking.update(score, achievement, 
					g_config.f_rankingCheckDate, g_config.f_rankingCheckChallenge);
			m_ranking.write();
			if(m_rank < 0){
				phase(PHASE_EXIT);	/* ランクインしなければ終了 */
				return;
			}
		}
	}
	m_phase = phaseNum;
}

/* ------------------------------------------------------------ */

bool CTyping::input(char ch, char *typeBuffer, int &typeBufferLen,
		vector<LyricsBlock>::iterator &lyricsPosition, bool f_successive, double time){
	if(typeBufferLen == 0){
		/* 新たに打ち始めるとき */
		if(time >= lyricsPosition->timeUntil){
			/* 打ち始める位置が時間切れなら打てない */
			return false;
		}
		if(ch != '\0'){
			/* チェック時は、今より遅く打つ可能性を考慮する */
			if(lyricsPosition->isScoringTarget && time < lyricsPosition->timeJust - SEC_POOR){
				/* 打ち始める位置が未採点で、それがちょうどよりSEC_POOR以上前のとき */
				/* 　（⇔不可領域の外で採点されうる時）、 */
				return false;
			}
		}
	}
	
	vector<Lyrics>::iterator lyricsPos = &m_lyrics[lyricsPosition->lyricsPos()];
	
	typeBuffer[typeBufferLen++] = ch;	/* 打った文字を追加 */
	/* CheckOnly（打てる入力が存在するか調べる）のときは、\0を追加する。 */
	/* 　そのとき、実際に打った長さを1つ多くすることによって、*/
	/* 　任意の1文字さらに打ったとして時間制限を判定できる */
	
	typeBuffer[typeBufferLen] = '\0';
	/* typeBufferLenを「実際に打った長さ」として用いるため */
	/* これを書いておかないとバッファオーバーランが起きる */
	
	bool ret;
	
	if(g_config.f_debugMode){
		ret = (time < lyricsPosition->timeUntil);
		/* 【この関数の前の方で弾かれなかったので実はretは常にtrueのような】 */
		if(ch == '\0' || !ret){	/* チェックのときや入らないときは元に戻して返す */
			typeBufferLen--;
		}else{
			/* 何を打っても、それで1つすべて打てたことにする */
			typeBufferLen = 0;
			
			scoreAccuracy(lyricsPos, f_successive, time);
			
			lyricsPosition++;
			vector<Lyrics>::iterator tmpLyricsPos = &m_lyrics[lyricsPosition->lyricsNum];
			scoreTyping(lyricsPos, tmpLyricsPos);
			setTyped(lyricsPos, tmpLyricsPos);
		}
	}else{
		ret = input_1(typeBuffer, typeBufferLen, lyricsPos, f_successive, time, (ch=='\0'), false);
			/* 最後から2番目はチェックのみか */
			/* 最後のfalseは再帰でないことを示す。 */
		if(ch != '\0' && ret){	/* 実際に打ち、それが入った場合 */
			lyricsPosition = &m_lyricsBlock[lyricsPos->blockNum];
				/* 新しい歌詞の位置に */
			/* typeBufferが更新されてるから、戻さない */
		}else{
			typeBufferLen--;	/* typeBufferLenを元に戻す */
		}
	}
	
	return ret;
#if 0
	if(ch == '\0'){
		ret = input_1(typeBuffer, typeBufferLen, lyricsPos, f_successive, time, true, false);
		/* 最後から2番目のtrueはCheckOnlyであることを示す */
		/* 最後のfalseは再帰でないことを示す。 */
		typeBufferLen--;	/* typeBufferLenを元に戻す */
	}else{
		ret = input_1(typeBuffer, typeBufferLen, lyricsPos, f_successive, time, false, false);
		/* 最後から2番目のfalseは実際に打ったことを示す */
		/* 最後のfalseは再帰でないことを示す。 */
		if(ret){
			lyricsPosition = &m_lyricsBlock[lyricsPos->blockNum];
				/* 新しい歌詞の位置に */
			/* typeBufferが更新されてるから、戻さない */
		}else{
			typeBufferLen--;
		}
	}
	return ret;
#endif
}

bool CTyping::input_1(char *typeBuffer, int &typeBufferLen, vector<Lyrics>::iterator &lyricsPos,
		bool f_successive, double time, bool isCheckOnly, bool isSaiki){
/* isSaikiは外からの呼び出しではfalse,内からの呼び出しではtrue */
#if 0
	if(isSaiki && strlen(typeBuffer) == 0){
	/* 未確定ローマ字がないのに再帰した */
		return true;
	}
#endif
	vector<Lyrics>::iterator tmpLyricsPos = lyricsPos;
	CTrieNode *trie = &m_trie;
	while(1){
		trie = trie->find(tmpLyricsPos->ch);	/* 日本語1バイト進む */
		tmpLyricsPos++;
		if(trie == NULL){	/* trieをたどれない、つまり、まとめて打てる範囲を超えた。 */
			return false;
		}
		for(vector<ConvertData>::iterator convData = trie->m_data.begin();
				convData != trie->m_data.end(); convData++){	/* ローマ字候補を探索 */
			if(convData->match_front(typeBuffer)){
				int tmpLen = typeBufferLen - convData->m_len;
				/* 今回変換される部分の次の部分からに対して実際に打たれている部分の長さ。 */
				/* これは負とかになりうる。 */
				if(tmpLen <= 0){	/* 今回に必要ならキーを足して一度に打てる範囲はここまで */
				/* これは、{んば……,nb……}でnだけ打った状況なども含まれる。 */
					if(time >= m_lyricsBlock[(*(tmpLyricsPos-1)).blockNum].timeUntil){
						/* その最後の文字が時間切れしていてはいけない */
						continue;
					}
				}
				char tmpTypeBuffer[TYPE_BUFFER_LEN + 1];
				bool flag;
				flag = convData->get_unprocessed(tmpTypeBuffer);
					/* 未確定ローマ字になる予定の部分を取得 */
				//strcpy(tmpTypeBuffer, (*convData).m_str + (*convData).m_len);
				if((!flag)
					|| input_1(tmpTypeBuffer, tmpLen, tmpLyricsPos, f_successive, time, isCheckOnly, true)){
				/* 未確定になるローマ字がないときや、 */
				/* 存在しても、再帰の結果それが打てることが分かったとき */
					if(isCheckOnly){	/* 調べるためのときは、何もしない。 */
						return true;
					}
					/* 調べるためではなく、実際に打っているとき */
					if(typeBufferLen >= 1){
					/* 新しい音節の打ち始め */
						scoreAccuracy(lyricsPos, f_successive, time);
						/* ちょうどのタイミングとのずれから計算した得点を与える。 */
					}
					
					/* 「っ」の処理、全ての3バイト以上への変換に効果があるので注意 */
					/* 【 てか、これは、convert.datを拡張し、ここで例外処理しなくても済むようにしたい 】 */
					if(typeBufferLen >= 2 && tmpLyricsPos - lyricsPos > 3){
					/* 2文字打って、2バイト先(日本語の1文字先)が今回一度に打てる範囲で、さらに採点対象である */
					/* つまり、「っか」における"kk"の時点で「か」が採点される。 */
						scoreAccuracy(lyricsPos + 2, f_successive, time);
						/* ちょうどのタイミングとのずれから計算した得点を与える。 */
					}
					if(convData->match(typeBuffer)){	/* 完全一致 */
						/* 変換された歌詞の分だけ得点を与える */
						if(!isSaiki){
						/* 再帰の2段目以降でもタイピング点を計算すると2重になるので、直接の呼び出しのみ処理 */
						/* 例：{んｔ,nt} */
							scoreTyping(lyricsPos, tmpLyricsPos);
							setTyped(lyricsPos, tmpLyricsPos);
						}
						lyricsPos = tmpLyricsPos;	/* どこまで行ったかを戻す */
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

/* ------------------------------------------------------------ */

void CTyping::setTyped(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd){
	vector<LyricsBlock>::iterator lyBlockBegin = &m_lyricsBlock[lyBegin->blockNum];
	vector<LyricsBlock>::iterator lyBlockEnd = &m_lyricsBlock[lyEnd->blockNum];
	/* 打ち始めから打ち終わりより前のBlockはすべて打たれる */
	for(vector<LyricsBlock>::iterator itr = lyBlockBegin; itr != lyBlockEnd; itr++){
		itr->nTyped = itr->lyricsLen;
		m_countClear++;
	}
	/* 最後のBlockは、打ち終わりの歌詞が相当する位置まで */
	lyBlockEnd->nTyped = lyEnd->blockPos;
	if(lyBlockEnd->nTyped == lyBlockEnd->lyricsLen){
		m_countClear++;
	}
}

/* ------------------------------------------------------------ */

void CTyping::scoreTyping(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd){
/* [lyBegin, lyEnd)を打ったときの得点 */
	bool isJapanese2 = false;
	for(vector<Lyrics>::iterator i = lyBegin; i != lyEnd; i++){
		if(isJapanese2){
			isJapanese2 = false;
		}else{
			m_scoreTyping += SCORE_TYPING;
			if(isJapanese1st((*i).ch)){	/* 日本語の1バイト目なら */
				isJapanese2 = true;
			}
		}
	}
	m_score = m_scoreTyping + m_scoreAccuracy;
}

void CTyping::scoreAccuracy(vector<Lyrics>::iterator lyricsPos, bool f_successive, double time){
	vector<LyricsBlock>::iterator lyricsPosition;
	lyricsPosition = &m_lyricsBlock[(*lyricsPos).blockNum];
	if(!(*lyricsPosition).isScoringTarget){
	/* タイミングの採点対象ではない、または、もう採点された */
		return;
	}
	
	if(!f_successive){
		m_combo = 0;
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
		
		scoreAccuracySub(lyricsPosition, ID_EXCELLENT);
		/*
		(*lyricsPosition).r = 192;
		(*lyricsPosition).g = 192;
		(*lyricsPosition).b = 64;
		*/
	}else if(timeDiff < SEC_GOOD){
		score = SCORE_GOOD + scoreCombo;
		m_countGood++;
		strAccuracy = "良";
		color = COLOR_GOOD;
		
		scoreAccuracySub(lyricsPosition, ID_GOOD);
		/*
		(*lyricsPosition).r = 170;
		(*lyricsPosition).g = 170;
		(*lyricsPosition).b = 85;
		*/
	}else if(timeDiff < SEC_FAIR){
		score = SCORE_FAIR + scoreCombo;
		m_countFair++;
		strAccuracy = "可";
		color = COLOR_FAIR;
		
		scoreAccuracySub(lyricsPosition, ID_FAIR);
		/*
		(*lyricsPosition).r = 128;
		(*lyricsPosition).g = 128;
		(*lyricsPosition).b = 128;
		*/
	}else{
		score = SCORE_POOR;
		m_countPoor++;
		m_combo = 0;	/* コンボが途切れていた */
		strAccuracy = "不可";
		color = COLOR_POOR;
		
		scoreAccuracySub(lyricsPosition, ID_POOR);
		/*
		(*lyricsPosition).r = 85;
		(*lyricsPosition).g = 85;
		(*lyricsPosition).b = 85;
		*/
	}
	if(m_combo > m_comboMax){	/* コンボ数の最大値を更新 */
		m_comboMax = m_combo;
	}
	if(!g_config.f_debugMode){
		if(m_combo >= 10){	/* コンボが10を超えたらコンボ数を表示 */
			sprintf(buf, "%s %d", strAccuracy, m_combo);
		}else{
			sprintf(buf, "%s", strAccuracy);
		}
	}else{	/* デバッグ時 */
		sprintf(buf, "%+lf", tmp);	/* 時間のずれを表示 */
		if(-SEC_FAIR < tmp && tmp < 0.0){
			//color = GetColor((int)255 * (-tmp/SEC_FAIR), (int)255 * (1 - (-tmp/SEC_FAIR)), 0);
			color = GetColor(255, 0, 0);
		}else if(0.0 <= tmp && tmp < SEC_FAIR){
			//color = GetColor(0, (int)255 * (1 - (tmp/SEC_FAIR)), (int)255 * (tmp/SEC_FAIR));
			color = GetColor(0, 0, 255);
		}
	}
	setText(buf, color);
	
	m_scoreAccuracy += score;	/* 得点加算 */
	m_score = m_scoreTyping + m_scoreAccuracy;
	(*lyricsPosition).isScoringTarget = false;	/* 2回以上得点を与えられることはない */
	(*lyricsPosition).scoringTime = time;	/* 得点がつけられた時刻を記録 */
}

/* lyricsPositionはそのうち使うかもしれない…… */
void CTyping::scoreAccuracySub(vector<LyricsBlock>::iterator lyricsPosition, int ID_accuracy){
	if(ID_accuracy <= ID_POOR){
		m_statGauge.inc(ID_accuracy);
	}
	
	int ID_gaugeZone = getGaugeZone();
	switch(ID_accuracy){
	case ID_EXCELLENT:
		switch(ID_gaugeZone){
		case ID_RED_ZONE:
			m_gauge += 200; break;
		case ID_YELLOW_ZONE:
			m_gauge += 200; break;
		case ID_BLUE_ZONE:
			m_gauge += 150; break;
		case ID_CLEAR_ZONE:
			m_gauge += 100; break;
		}
		break;
	case ID_GOOD:
		switch(ID_gaugeZone){
		case ID_RED_ZONE:
			m_gauge += 100; break;
		case ID_YELLOW_ZONE:
			m_gauge += 100; break;
		case ID_BLUE_ZONE:
			m_gauge += 75; break;
		case ID_CLEAR_ZONE:
			m_gauge += 50; break;
		}
		break;
	case ID_FAIR:
		switch(ID_gaugeZone){
		case ID_RED_ZONE:
			m_gauge += 0; break;
		//case ID_YELLOW_ZONE:
		//case ID_BLUE_ZONE:
		//case ID_CLEAR_ZONE:
		//	m_gauge += 0; break;
		}
		break;
	case ID_POOR:
		switch(ID_gaugeZone){
		case ID_RED_ZONE:
			m_gauge -= 50; break;
		case ID_YELLOW_ZONE:
			m_gauge -= 100; break;
		case ID_BLUE_ZONE:
			m_gauge -= 150; break;
		case ID_CLEAR_ZONE:
			m_gauge -= 150 + m_countAll; break;
		}
		break;
	}
	m_gaugeNewCount++;

	if(m_gauge > m_gaugeMax)m_gauge = m_gaugeMax;
	if(m_gauge <= 0){
		m_gauge = 0;
		if(g_config.f_failByGauge){
			m_flagFailed = true;
		}
	}
}

/* ------------------------------------------------------------ */

/*
		red	yellow	blue		clear
Excellent	+200	+200	+150		+100
Good		+100	+100	+ 75		+ 50
Fair		+  0	+  0	+  0		+  0
Poor		- 50	-100	-150		-150-n
Passed		-100	-150	-200		-200-n
非打ち切り	- 50	-100	-100-n/2	-100-n

25nからスタート
値の範囲は 0 - 100n
表示は0-100、切り上げ
*/

void CTyping::initGauge(){
	m_gauge = m_countAll * 25;
	m_gaugeMax = m_countAll * GAUGE_COUNT;
	m_gaugeLastCount = 0;
	m_gaugeNewCount = 0;
	m_gaugeLastLost = 0;
}

int CTyping::getGaugeZone(){
	if(m_gauge == 0){
		return ID_FAILED_ZONE;
	}else if(m_gauge <= m_countAll * GAUGE_RED_ZONE){
		return ID_RED_ZONE;
	}else if(m_gauge <= m_countAll * GAUGE_YELLOW_ZONE){
		return ID_YELLOW_ZONE;
	}else if(m_gauge <= m_countAll * GAUGE_CLEAR_ZONE){
		return ID_BLUE_ZONE;
	}else{
		return ID_CLEAR_ZONE;
	}
}

/* 【 この関数は得点を得るときと、m_lyricsPositionが変わったときごとに呼ばれる必要がある 】 */
/* というのは、0%以下や100%以上になって修正されるとき */
void CTyping::updateGauge(){
	{
		int count = m_lyricsPosition->scoringCount;	/* 流れてきた個数 */
		
		if(count > m_gaugeLastCount){
			/* count が進んだら打ち逃しと通過を判定 */
			{	/* 非打ち切りを減点 */
				int lost = count - m_countClear;	/* 打ちきれなかった個数 */
				int t = lost - m_gaugeLastLost;	/* 今回処理する分 */
				m_gaugeLastLost = lost;
				while(t-- > 0){
					switch(getGaugeZone()){
					case ID_RED_ZONE:
						m_gauge -= 50; break;
					case ID_YELLOW_ZONE:
						m_gauge -= 100; break;
					case ID_BLUE_ZONE:
						m_gauge -= 100 + (m_countAll/2); break;
					case ID_CLEAR_ZONE:
						m_gauge -= 100 + m_countAll; break;
					}

				}
				/* 打ちきれないと減点（以下のPassの減点と重複する） */
			}
			
			{	/* 通過を減点 */
				int t = count - m_gaugeLastCount;
				m_gaugeLastCount = count;
				
				t -= m_gaugeNewCount;
				if(t < 0){	/* countが進む前に打てることがあるが、これは後回しする */
					m_gaugeNewCount = -t;
				}else{
					while(t-- > 0){
						switch(getGaugeZone()){
						case ID_RED_ZONE:
							m_gauge -= 100; break;
						case ID_YELLOW_ZONE:
							m_gauge -= 150; break;
						case ID_BLUE_ZONE:
							m_gauge -= 200; break;
						case ID_CLEAR_ZONE:
							m_gauge -= 200 + m_countAll; break;
						}
					}
					m_gaugeNewCount = 0;
				}
			}
		}
	}
	
	if(m_gauge <= 0){
		m_gauge = 0;
		if(g_config.f_failByGauge){
			m_flagFailed = true;
		}
	}
}


int CTyping::getDrawGaugeColor(int pos, int cnt){
	if(pos < GAUGE_RED_ZONE){
		return GetColor(255, 32, 0);
	}else if(pos < GAUGE_YELLOW_ZONE){
		return GetColor(224, 255, 0);
	}else if(pos < GAUGE_CLEAR_ZONE){
		return GetColor(0, 64, 255);
	}else{
		return GetColor(255, 255, 255);
	}
}
/*
int CTyping::getDrawGaugeColor(int pos, int cnt){
	if(pos < 12){
		if(pos < cnt) return GetColor(255, 32, 0);
		else return GetColor(64, 8, 0);
	}else if(pos < 36){
		if(pos < cnt) return GetColor(224, 255, 0);
		else return GetColor(56, 64, 0);
	}else if(pos < 72){
		if(pos < cnt) return GetColor(0, 64, 255);
		else return GetColor(0, 16, 64);
	}else{
		if(pos < cnt) return GetColor(255, 255, 255);
		else return GetColor(64, 64, 64);
	}
}

int CTyping::getDrawGaugeColor(int pos, int cnt){
	cnt--;
	if(pos < 12){
		if(pos == cnt) return GetColor(255, 32, 0);
		else if(pos < cnt) return GetColor(192, 24, 0);
		else return GetColor(64, 8, 0);
	}else if(pos < 36){
		if(pos == cnt) return GetColor(224, 255, 0);
		else if(pos < cnt) return GetColor(168, 192, 0);
		else return GetColor(56, 64, 0);
	}else if(pos < 72){
		if(pos == cnt) return GetColor(0, 64, 255);
		else if(pos < cnt) return GetColor(0, 48, 192);
		else return GetColor(0, 16, 64);
	}else{
		if(pos == cnt) return GetColor(255, 255, 255);
		else if(pos < cnt) return GetColor(192, 192, 192);
		else return GetColor(64, 64, 64);
	}
}
*/
double CTyping::getDrawGaugeAlpha(int pos, int cnt){
	if(pos >= cnt) return 0.25;
	else return 1.0;
#if 0
	cnt--;
	cnt-=pos;	/* 点灯している右端から何番目か（0から） */
	if(cnt > 5) return 0.6;
	double tmp = 0.4;
	while(cnt--){
		tmp *= 0.7;
	}
	return 0.6+tmp;
#endif
}

void CTyping::drawGauge(bool isResult){
	//if(m_flagFailed){	/* 【ゲージ0で死んだ後に回復したりすることがないようにする】 */
		//m_gauge = 0;
	//}
	
	int y0, y1;
	
	if(isResult){
		y0 = 355;
	}else{
		y0 = Y_GAUGE;
	}
	y1 = y0 + H_GAUGE;
	
	//int cnt = (GAUGE_COUNT * m_gauge + m_gaugeMax-1)/ m_gaugeMax;
		/* m_gauge / m_gaugeMax を 0 - GAUGE_COUNTにする（切り上げ） */
		/* GAUGE_COUNT * 100(==GAUGE_COUNT) * m_countAll は、音符の数が10万個以下ならintに入る　*/
	int cnt = (m_gauge + m_countAll - 1)/ m_countAll;
	
	/* もともと濃い灰色で描画しないのは tan などへの配慮 */
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);	/* 不透明さ1/2 */
	DrawBox(X_GAUGE - W_GAUGE_PADDING, y0 - H_GAUGE_PADDING,
		X_GAUGE + W_GAUGE + W_GAUGE_PADDING, y1 + H_GAUGE_PADDING,
		GetColor(16, 16, 16), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	
	for(int i=0; i<GAUGE_COUNT; i++){
		int x0 = X_GAUGE + W_GAUGE_SEGMENT * i;
		int x1 = x0 + W_GAUGE_SEGMENT;
		int color = getDrawGaugeColor(i, cnt);
		double alpha = getDrawGaugeAlpha(i, cnt);
		if(i<cnt){
			/* あふれる光を表現 */

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(48*alpha));	/* 不透明さ3/16倍 */
			/*
			if(i==cnt-1){
				DrawBox(x0-2, y0, x1+2, y1, 
					color, TRUE);
			}
			*/
			DrawBox(x0 +1, y0 -1, x1 -1, y1 +1, 
				color, TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255*alpha));
		DrawBox(x0 +1, y0, x1 -1, y1, 
			color, TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void CTyping::drawElapsedTime(double time){
	time /= m_timeLength;	/* 割合に直す */
	if(time > 1.0) time = 1.0;	/* 超えていたら打ち切る */
	int pos = (int)(W_TIME*time);
	DrawBox(X_TIME, Y_TIME, X_TIME+pos, Y_TIME+H_TIME,
		COLOR_TIME, TRUE);
	DrawBox(X_TIME+pos -3, Y_TIME, X_TIME+pos+ 3, Y_TIME+H_TIME,
		COLOR_TIME2, TRUE);
}

/* ------------------------------------------------------------ */

void CTyping::drawTypeBuffer(double time){
/*
	m_typeBuffer[m_typeBufferLen] = '\0';
	DrawStringToHandle(X_BUFFER, Y_BUFFER, m_typeBuffer,
		GetColor(255, 255, 255), m_fontHandleBig);
*/
	m_typeBufferEffect.update(m_typeBuffer, m_typeBufferLen, time);	/* 念のため【本来は不要】 */
	m_typeBufferEffect.draw(X_BUFFER, Y_BUFFER, time);
#if 0
	int x = 0;
	char str[2];
	str[1] = '\0';
	
	/* いろいろ面倒なので、等幅フォントを信用することにする */
	str[0] = ' ';
	int w = GetDrawStringWidthToHandle(str, 1, m_fontHandleBig);
	
	{
		int s = m_typeBufferTime.size();
		while(s > m_typeBufferLen){
			s--;
			m_typeBufferTime.pop_front();
		}
	}
	for(int i=0; i<m_typeBufferLen; i++){
		/* i文字目を描画 */
		str[0] = m_typeBuffer[i];
		
		int y = 0;
		double timeDiff = time - m_typeBufferTime[i];
		if(timeDiff < 0.03){	/* 0.03秒後には定位置 */
			//y -= ((0.03 - timeDiff) / 0.03) * 3;	/* 3だけ上から出てくる */
			x += ((0.03 - timeDiff) / 0.03) * 5;	/* 5だけ右から出てくる */
		}
		DrawStringToHandle(X_BUFFER + x, Y_BUFFER + y, str, GetColor(255, 255, 255), m_fontHandleBig);
		
		/* 次の文字の位置へ */
		x += w;
		//x += GetDrawStringWidthToHandle(str, 1, m_fontHandleBig);
	}
#endif
}

/* ------------------------------------------------------------ */

/* true:未タイプのものを取得, false:すべて取得 */
int CTyping::getBlockStr(vector<LyricsBlock>::iterator itr, char *buf, bool f_getUntyped) const{
	int len = 0;
	for(int j = (f_getUntyped ? itr->nTyped : 0);
			j<(*itr).lyricsLen; j++){	/* ブロック内の未タイプの文字をbufに格納 */
		buf[len++] = m_lyrics[(*itr).lyricsNum + j].ch;
	}
	buf[len] = '\0';
	return len;
}

int CTyping::getLyricsBuffer(char *buf) const{
	int len = 0;
	for(vector<LyricsBlock>::iterator i = m_lyricsPosition; i != m_lyricsPositionEnd; i++){
		len += getBlockStr(i, &buf[len], true);
	}
	buf[len] = '\0';
	return len;
}

/* ------------------------------------------------------------ */

/* timeDiff: ちょうどの位置の何秒後か */
int CTyping::getDrawPosX(double timeDiff) const{
	/* CIRCLE_SPEED * m_challenge.speed() が速さ、（ただし、左向き） */
	return X_CIRCLE + (int)(-timeDiff * (CIRCLE_SPEED * m_challenge.speed()));
}

int CTyping::getDrawPosY(int x) const{
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

/* ------------------------------------------------------------ */

void CTyping::draw(){
	if(m_phase >= PHASE_RESULT){	/* スコア表示以降 */
		drawResult();
		//drawFlagAbort();
		return;
	}
	double time;
	if(m_phase == PHASE_READY){
		if(m_soundHandleMusic == -1){
			DrawStringToHandle(50, 370, "Now loading...", GetColor(255, 255, 255), m_fontHandleBig);
		}else{
			DrawStringToHandle(50, 370, "Press any key to start.", GetColor(255, 255, 255), m_fontHandleBig);
		}
		time = 0.0;	/* 始まる前は0秒で止まっている */
		if(g_config.f_debugMode && g_config.debugTime > 0){	/* 途中から始めるデバッグモードのとき */
			time = g_config.debugTime / 1000.0;
		}
	}else{
		time = getTime();	/* 開始時刻からの経過秒を取得 */
	}
	
	/*{
		char buf[256];
		sprintf(buf, "%4d コンボ", m_combo);
		int width = GetDrawStringWidthToHandle(buf, strlen(buf), m_fontHandleNormal);
		DrawStringToHandle(X_INFO - width, Y_INFO2, buf,
			GetColor(255, 255, 255), m_fontHandleNormal);
	}*/
	/*
	DrawFormatStringToHandle(10, Y_INFO, GetColor(255, 255, 255), m_fontHandleNormal,
		"%10d 点", 10*m_scoreDraw);
	DrawFormatStringToHandle(10, Y_INFO2, GetColor(255, 255, 255), m_fontHandleNormal,
		"%10d コンボ", m_combo);
	*/
	
	/* 小節線、拍線を表示 */
	while((*m_beatLineDrawLeft).time != INFTY){
		double timeDiff = time - (*m_beatLineDrawLeft).time;
		int posX = getDrawPosX(timeDiff);
		if(posX >= 0){
			/* 画面から出ていない */
			break;
		}
		/* すでに画面から出てしまったので、描画対象から削除 */
		m_beatLineDrawLeft++;
	}
	while((*m_beatLineDrawRight).time != INFTY){
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
		
		while((*m_lyricsBlockDrawLeft).timeJust != INFTY){
			double timeDiff = time - (*m_lyricsBlockDrawLeft).timeJust;
			int posX = getDrawPosX(timeDiff);
			if(posX >= -(R_CIRCLE + 300)){
				/* 円をはみ出る歌詞や、打ち切って跳ねることを考慮して広めに取る */
				/* 画面から出ていない */
				break;
			}
			/* すでに画面から出てしまったので、描画対象から削除 */
			m_lyricsBlockDrawLeft++;
		}
		
		while((*m_lyricsBlockDrawRight).timeJust != INFTY){
			double timeDiff = time - (*m_lyricsBlockDrawRight).timeJust;
			int posX = getDrawPosX(timeDiff);
			if(posX >= W_WINDOW + R_CIRCLE){
				/* 画面に入ってきていない */
				break;
			}
			/* 新しく画面に入ってきたので、描画対象に追加 */
			m_lyricsBlockDrawRight++;
		}
		
		for(vector<LyricsBlock>::iterator i = m_lyricsBlockDrawRight; i != m_lyricsBlockDrawLeft;){
			i--;	/* [Left, Right)なので、ここでデクリメント */
			double timeDiff = time - (*i).timeJust;	/* ちょうど打つ位置になってから何秒後か */
			int posX = getDrawPosX(timeDiff);
			int posY = getDrawPosY(posX);
			
			if(!(*i).isValid){	/* 歌詞の切れ目 */
				if(timeDiff < 0.0){	/* 切れ目は判定位置に来るより前のときだけ表示 */
					double alpha = 1.0;
					if((*i).clearedTime >= 0.0){	/* 切れ目が消えているとき */
						double t = ((*i).timeJust - time) / ((*i).timeJust - (*i).clearedTime);
						alpha = t*t*t;
					}else if(i < m_lyricsPosition){	/* 切れ目が消されずに次に進んだ場合 */
					/* つまり、打ち切らずに、新しい部分を早めに打ち始めた場合（に限るかな？） */
						continue;	/* 表示しない */
					}
					/* 灰色の円を流す */
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, 51 * alpha);	/* alpha * 1/5の不透明度で描画 */
					DrawCircle(posX, Y_CIRCLE + posY, R_CIRCLE - 1, GetColor(85, 85, 85), TRUE);
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 * alpha);
					DrawCircle(posX, Y_CIRCLE + posY, R_CIRCLE, GetColor(85, 85, 85), FALSE);
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
				}
				continue;
			}
			
			char buf[256];
			int len = getBlockStr(i, buf, true);	/* ブロック内の未タイプの文字をbufに格納 */
			if(len == 0){	/* すべてタイプされていた */
				if((*i).clearedTime < 0){
					/* 時刻を保存 */
					(*i).clearedTime = time;
					/* 位置を保存 */
					(*i).x = posX;
					(*i).y = posY;
					double arg = (GetRand(32) - 16) / 20.0;
					/* -0.8以上0.8以下0.05刻みでランダム */
					double arg2 = GetRand(354) / 113.0;
					/* 0～πぐらいまでのランダム（355/113≒π） */
					(*i).vx = 100.0 * sin(arg) * sin(arg2);
					(*i).vy = 100.0 * cos(arg);
					(*i).vz = 0.7 * sin(arg) * cos(arg2);
				}
				// continue;
			}
			
			int Color;
			if((*i).isScoringTarget){	/* まだタイミング点をもらってない */
				Color = GetColor(255, 0, 0);
			}else{
				double sTime = (time - (*i).scoringTime) / 0.20;
				if(sTime < 1.0){
					if(sTime < 0.0){
						sTime = 0.0;	/* 時刻が調整されうるので念のため */
					}
					
					Color = GetColor((int)(255*0.4*(1.0+sTime)*(1.0-sTime)),
						(int)(255*0.4*(1.0-sTime)*(1.0-sTime)),
						(int)(255*0.4*(1.0+1.5*sTime)));
					
				}else{
					Color = GetColor(0, 0, 255);
				}
			}
			{
				int rCircle = R_CIRCLE;
				if((*i).clearedTime >= 0.0){
					double fTime = (time - (*i).clearedTime)/0.25;
					if(fTime >= 1.0){	/* 打ち切ってから十分時間がたった */
						continue;
					}
					if(fTime < 0.0){
						fTime = 0.0;	/* 時刻が調整されうるので念のため */
					}
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 * 0.3 * (1.0 - fTime*fTime));
					/* 最初からある程度半透明、さらに徐々に半透明に */
					posX = (int)((*i).vx * fTime) + (*i).x;
					posY = (int)(((125.0 * fTime) - (*i).vy) * fTime) + (*i).y;
					/* 初速度(vx,vy)、加速度下に2*125.0で位置計算 */
					rCircle = R_CIRCLE / (1.0 + (*i).vz * fTime);
				}
				DrawCircle(posX, Y_CIRCLE + posY, rCircle - 1, Color, TRUE);	/* 流れる円 */
				DrawCircle(posX, Y_CIRCLE + posY, rCircle, GetColor(0, 0, 0), FALSE);	/* 流れる円の輪郭 */
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
			
			/* 円の下の歌詞 */
			if(!m_challenge.test(CHALLENGE_LYRICS_STEALTH)){	/* LyricsStealthなら表示しない */
				int strWidth = GetDrawStringWidthToHandle(buf, len, m_fontHandleNormal);
				int posXLeft = posX - strWidth / 2;	/* 中心を円の中心に */
				if(posXLeft < posX - R_CIRCLE){	/* 円の左端より出てるなら */
					posXLeft = posX - R_CIRCLE;	/* 円の左端にあわせる */
				}
				DrawStringToHandle(posXLeft, Y_LYRICS + posY, buf,
					GetColor(255, 255, 255), m_fontHandleNormal);	/* 流れる円に書かれる文字 */
			}
		}
		SetDrawArea(0, 0, W_WINDOW, H_WINDOW);	/* 描画範囲を元に戻す */
	}
	
	DrawCircle(X_CIRCLE, Y_CIRCLE, R_CIRCLE, GetColor(255, 255, 255), FALSE);	/* 判定位置の円 */
	
	{	/* メッセージ表示 */
		double timeDiff = time - m_textTime;
		if(timeDiff <= (0.5+0.1)){	/* メッセージ（優良可不可）が0.5秒間表示される */
			/* 0.1秒残像が残る */
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
			if(timeDiff > 0.5){
				double t = (timeDiff - 0.5) / 0.1;
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * (1.0 - t)));
			}
			DrawStringToHandle(strX, strY, m_text,
				m_textColor, m_fontHandleBig);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
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
			(*m_lyricsKanjiPosition).draw(
				X_LYRICS_KANJI, Y_LYRICS_KANJI, m_fontHandleNormal);	/* 出力 */
			(*(m_lyricsKanjiPosition + 1)).draw(
				X_LYRICS_KANJI, Y_LYRICS_KANJI_NEXT, m_fontHandleNormal);	/* Nextを出力 */
		}else{
			(*m_lyricsKanjiPosition).draw(
				X_LYRICS_KANJI, Y_LYRICS_KANJI_NEXT, m_fontHandleNormal);	/* Nextを出力 */
		}
	}
	
	if(!m_challenge.test(CHALLENGE_LYRICS_STEALTH)){	/* LyricsStealth だと表示されない */
		updateLyricsBuffer(time);
		
		/* いま対象になっているローマ字歌詞(m_lyricsPositionからm_lyricsPositionEndまで)を表示 */
		/* Hidden や Stealth のとき、ヒントになるのを避けるため、 */
		/* 何かを打つまでは例外的に表示されない */
		/* 【updateLyricsBufferでPositionEndを賢く管理することになった】 */
#if 0
		if(!( (m_challenge.test(CHALLENGE_HIDDEN) || m_challenge.test(CHALLENGE_STEALTH))
				&& m_typeBufferLen == 0 )){
#endif
			m_lyricsBufferEffect.draw(X_LYRICS_BIG, Y_LYRICS_BIG, time);
			/*
			char buf[256];
			getLyricsBuffer(buf);
			DrawStringToHandle(X_LYRICS_BIG, Y_LYRICS_BIG, buf,
				GetColor(255, 255, 255), m_fontHandleBig);
			//*/
#if 0
		}
#endif
	}
	
	/* タイプした文字を表示 */
	drawTypeBuffer(time);
	
	drawGauge(false);

	drawElapsedTime(time);
	
	m_statGauge.draw();
	
	m_scoreDraw = m_score/10 - (int)(0.7 * (m_score/10 - m_scoreDraw));
	{
		char buf[256];
		sprintf(buf, "%8d", 10*m_scoreDraw);	// "%8d 点"
		int width = GetDrawStringWidthToHandle(buf, strlen(buf), m_fontHandleBig);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 64);	/* 1/4の不透明さで黒で描く */
		DrawBox(X_SCORE - width - 4, Y_SCORE - 4, X_SCORE + 4, Y_SCORE + 36 + 4,
			GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawStringToHandle(X_SCORE - width, Y_SCORE, buf,
			GetColor(255, 255, 255), m_fontHandleBig);
	}
	
	{
		char buf[256];
		sprintf(buf, "最大 %4d コンボ", m_comboMax);
		int width = GetDrawStringWidthToHandle(buf, strlen(buf), m_fontHandleNormal);
		DrawStringToHandle(X_INFO - width, Y_INFO, buf,
			GetColor(255, 255, 255), m_fontHandleNormal);
	}
	
	m_effect1.draw(time);	/* キー入力エフェクト */
	
	if(m_phase == PHASE_FADEOUT){	/* フェードアウト */
		double xTime = (time - m_timeFinished) / SEC_FADEOUT;
		if(xTime >= 1.0){
			xTime = 1.0;
		}
		if(m_flagFailed){
			/* 上下からおろす */
			double t = xTime * 1.5;	/* こっちは1.5倍の速さで完了させる */
			t *= 1.5;
			if(t >= 1.0){
				t = 1.0;
			}
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 224);
			double tmp = t * (1 + 3.0 * t * (t - 1.0));
			int h = H_WINDOW * 0.5 * tmp;
			DrawBox(0, 0, W_WINDOW, h, GetColor(64, 64, 64), TRUE);
			DrawBox(0, H_WINDOW - h, W_WINDOW, H_WINDOW, GetColor(64, 64, 64), TRUE);
		}
		
		{
			double tmp = ((-2.0*xTime)+3.0)*xTime*xTime;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 * tmp);
			DrawBox(0, 0, W_WINDOW, H_WINDOW, GetColor(0, 0, 0), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	
	drawFlagAbort();
}

void CTyping::drawResult(){
	double time = getTime();
	DrawFormatStringToHandle(30, 10, GetColor(255, 255, 255), m_fontHandleBig,
		"判定 :");
	if(time >= 0.6){
		DrawFormatStringToHandle(320, 15, COLOR_EXCELLENT, m_fontHandleNormal,
			"　優 : %4d / %d", m_countExcellent, m_countAll);
	}
	if(time >= 0.7){
		DrawFormatStringToHandle(320, 40, COLOR_GOOD, m_fontHandleNormal,
			"　良 : %4d / %d", m_countGood, m_countAll);
	}
	if(time >= 0.8){
		DrawFormatStringToHandle(320, 65, COLOR_FAIR, m_fontHandleNormal,
			"　可 : %4d / %d", m_countFair, m_countAll);
	}
	if(time >= 0.9){
		DrawFormatStringToHandle(320, 90, COLOR_POOR, m_fontHandleNormal,
			"不可 : %4d / %d", m_countPoor, m_countAll);
	}
	if(time >= 1.0){
		DrawFormatStringToHandle(320, 115, GetColor(255, 255, 255), m_fontHandleNormal,
			"通過 : %4d / %d", m_countPass, m_countAll);
	}
	if(time >= 1.8){
		if(m_comboMax >= 0){
			DrawFormatStringToHandle(30, 160, GetColor(255, 255, 255), m_fontHandleBig,
				"最大 %d コンボ", m_comboMax);
		}else{
			DrawFormatStringToHandle(30, 160, GetColor(255, 255, 0), m_fontHandleBig,
				"フルコンボ");
		}
	}
	if(time >= 2.6){
		DrawFormatStringToHandle(30, 225, GetColor(255, 255, 255), m_fontHandleBig,
			"得点 :");
	}
	if(time >= 2.9){
		DrawFormatStringToHandle(320, 225, GetColor(255, 255, 255), m_fontHandleNormal,
			"質 : %7d 点", m_scoreAccuracy);
	}
	if(time >= 3.2){
		DrawFormatStringToHandle(320, 250, GetColor(255, 255, 255), m_fontHandleNormal,
			"量 : %7d 点", m_scoreTyping);
	}
	if(time >= 4.0){
		DrawFormatStringToHandle(30, 295, GetColor(255, 255, 255), m_fontHandleBig,
			"総得点 : %8d 点", m_score);
		
		drawGauge(true);
	}
	if(!(m_challenge.isEasy() || m_isReplay || g_config.f_debugMode)){	/* 通常時のみ */
		/* 簡単になるオプションを使用した場合やリプレイ時やデバッグ時　でなければ、 */
		/* ランキングにいれるため、名前を入力。 */
		
		DrawStringToHandle(60, 410, m_typeBuffer, GetColor(255, 255, 255), m_fontHandleBig);
		/* 入力された名前を表示 */
		if(time >= 4.0){
			/* 名前が設定されてない場合、一定時間が経ったら入力開始 */
			if(m_phase < PHASE_FINISHED){	/* 名前が確定したのちは表示しない */
				DrawFormatStringToHandle(30, 395, GetColor(255, 255, 255), m_fontHandleNormal,
					"名前を入力してください :");
				if(m_flagLockName){
					DrawFormatStringToHandle(280, 395, GetColor(64, 64, 85), m_fontHandleNormal,
						"（ Esc: 名前変更 ）");
				}
			}
		}
	}
	if(m_phase == PHASE_FINISHED){
		if(m_rank >= 0){
			int strWidth;
			strWidth = GetDrawFormatStringWidthToHandle( m_fontHandleBig,
				"%d 位にランクインしました", m_rank + 1);
			int boxWidth = strWidth + 80;
			if(boxWidth > W_WINDOW){
				boxWidth = W_WINDOW;
			}
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);	/* （1/3の）半透明で描画 */
			DrawBox(320 - boxWidth / 2, 200, 320 + boxWidth/2, 280, GetColor(170, 170, 170), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 192);	/* （1/4の）半透明で描画 */
			DrawFormatStringToHandle(320 - strWidth / 2, 240 - 18,
				GetColor(0, 0, 0), m_fontHandleBig,
				"%d 位にランクインしました", m_rank + 1);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);	/* ブレンドしないように戻す */
		}
	}
	return;
}

void CTyping::drawFlagAbort(){
	if(m_flagAbort){
		const char *str = "もう一度Escを押すと中断します";
		int strWidth;
		strWidth = GetDrawStringWidthToHandle(str, strlen(str), m_fontHandleBig);
		int boxWidth = strWidth + 80;
		if(boxWidth > W_WINDOW){
			boxWidth = W_WINDOW;
		}
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);	/* （1/3の）半透明で描画 */
		DrawBox(320 - boxWidth / 2, 200, 320 + boxWidth/2, 280, GetColor(170, 170, 170), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 192);	/* （1/4の）半透明で描画 */
		DrawStringToHandle(320 - strWidth / 2, 240 - 18,
			str, GetColor(0, 0, 0), m_fontHandleBig);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);	/* ブレンドしないように戻す */
	}
}

/* ------------------------------------------------------------ */

void CTyping::mainLoop(){
	m_isReplay = false;
	m_replayData.clear();
	
	//ClearDrawScreen();
	while(1){
g_check.rap(GetColor(16, 0, 0));
		draw();
		myScreenFlip();
		double timeCountTmp = myGetNowHiPerformanceCount();	/* 時刻を保存しておく */
		
g_check.rap(GetColor(16, 16, 0));
	//ProcessStreamSoundMemAll();
		//ClearDrawScreen();
		if(ProcessMessage() == -1){
			break;
		}
g_check.rap(GetColor(0, 0, 16));
		while(1){
			double timeCount;
			char ch = GetKeyboardInput(timeCount);
			if(ch == 0){	/* バッファが空になればキー入力処理終了 */
				break;
			}
			//if(ch == CTRL_CODE_ESC){	/* Escでゲームを中断 */
			//	goto L1;
			//}
			keyboard(ch, timeCount);
		}
		
g_check.rap(GetColor(0, 16, 0));
		if(!idle(timeCountTmp)){
			break;
		}
	}
//L1:
	return;
}

void CTyping::replayLoop(){
	m_isReplay = true;
	
	vector<ReplayData>::iterator itrReplay = m_replayData.begin();
	
	//ClearDrawScreen();
	while(1){
g_check.rap(GetColor(16, 0, 0));
		draw();
		myScreenFlip();
		
		double timeCountTmp = myGetNowHiPerformanceCount();	/* 時刻を保存しておく */
		
g_check.rap(GetColor(16, 16, 0));
		if(ProcessMessage() == -1){
			break;
		}
		//ClearDrawScreen();
		
g_check.rap(GetColor(0, 0, 16));
		while(1){
			double timeCount;
			char ch = GetKeyboardInput(timeCount);
			if(ch == 0){	/* バッファが空になればキー入力処理終了 */
				break;
			}
			if(ch == CTRL_CODE_ESC){	/* Escでゲームを中断 */
				goto L1;
			}
			/* リプレイの時は、ゲームメイン時、キー入力をEsc以外無視 */
			if(m_phase != PHASE_MAIN){
				keyboard(ch, timeCount);
			}
		}
		
		if(m_phase == PHASE_MAIN){
			double time = getTime(timeCountTmp);
			while(itrReplay != m_replayData.end() && time > (*itrReplay).time){
				keyboard_1((*itrReplay).ch, (*itrReplay).time);
				itrReplay++;
			}
		}
		
g_check.rap(GetColor(0, 16, 0));
		if(!idle(timeCountTmp)){
			break;
		}
	}
L1:
	return;
}

/* ------------------------------------------------------------ */

bool CTyping::saveReplay(const char *fileName, bool &f_confirm){
/* f_confirm = false: 上書き確認がまだ, true: 上書き確認了承済み */
	FILE *fp;
	if(g_config.overwriteReplay != CONFIG_ALWAYS && !f_confirm){
		/* 常に上書きや、上書き確認→確認済みの場合はこの操作は不要 */
		/* ファイルが存在するかチェックする */
		fp = fopen(fileName, "rb");
		if(fp != NULL){	/* 存在した */
			fclose(fp);
			if(g_config.overwriteReplay == CONFIG_QUERY){	/* 確認する設定なら */
				/* 確認することにして */
				f_confirm = true;
			}
			/* どちらにせよ（確認するとしてもひとまず）失敗 */
			return false;
		}
	}
	f_confirm = false;	/* 戻しておく */
	fp = fopen(fileName, "wb");
	if(fp == NULL){
		return false;
	}
	for(vector<ReplayData>::iterator itr = m_replayData.begin(); itr != m_replayData.end(); itr++){
		fwrite(&(*itr).ch, sizeof(char), 1, fp);
		fwrite(&(*itr).time, sizeof(double), 1, fp);
	}
	fclose(fp);
	return true;
}

bool CTyping::loadReplay(const char *fileName){
	FILE *fp;
	fp = fopen(fileName, "rb");
	if(fp == NULL){
		return false;
	}
	m_replayData.clear();	/* 読み込む前に現在入っているのを消去 */
	while(1){
		ReplayData rd;
		if(fread(&rd.ch, sizeof(char), 1, fp) < 1){
			break;
		}
		if(fread(&rd.time, sizeof(double), 1, fp) < 1){
			break;
		}
		m_replayData.push_back(rd);
	}
	fclose(fp);
	return true;
}

/* ------------------------------------------------------------ */

bool CTyping::saveReplayLoop(){
	char message[256];
	strcpy(message, "");
	bool isSaved = false;
	
	bool f_confirm = false;	/* 現在上書き確認を聞いているところであるか */
	
	char buf[256];
	strcpy(buf, "");
	int len = 0;
	while(1){
		if(ProcessMessage() == -1){
			return false;
		}
		//ClearDrawScreen();
		
		DrawStringToHandle(10, 10, "save replay as", GetColor(255, 255, 255), m_fontHandleBig);
		if(len != 0){
			DrawStringToHandle(10, 50, buf, GetColor(255, 255, 255), m_fontHandleBig);
		}else{
			int color = GetColor(128, 128, 128);
			if(isSaved){	/* セーブ後なら同じ扱い */
				color = GetColor(255, 255, 255);
			}
			DrawStringToHandle(10, 50, g_config.defaultReplayFile, color, m_fontHandleBig);
		}
		DrawStringToHandle(10, 90, message, GetColor(255, 255, 255), m_fontHandleBig);
		
		myScreenFlip();
		while(1){
			char ch = GetKeyboardInput();
			if(ch == 0){	/* キー入力処理終了 */
				break;
			}
			if(isSaved){	/* セーブ後、何か押したら戻る */
				return true;
			}
			strcpy(message, "");	/* 何か打ったらメッセージは消去 */
			if(f_confirm){
				switch(ch){
				case CTRL_CODE_ESC:	/* そもそも中断 */
					return false;
				case 'y':
				case 'Y':
					/* yesならf_confirm=trueの元でセーブ */
					isSaved = saveReplay((len != 0) ? buf : g_config.defaultReplayFile, f_confirm);
					if(isSaved){
						strcpy(message, "saved.");
					}else{
						strcpy(message, "error.");
					}
					break;
				case 'n':
				case 'N':
					/* noならf_confirmを取り消し、メッセージもなしに戻す */
					f_confirm = false;
					strcpy(message, "");
					break;
				}
			}else{
				switch(editBuffer(ch, buf, len)){	/* バッファを操作 */
				case EDIT_BUFFER_OK:
					isSaved = saveReplay((len != 0) ? buf : g_config.defaultReplayFile, f_confirm);
					if(isSaved){
						strcpy(message, "saved.");
					}else if(f_confirm){
						strcpy(message, "already exists. overwrite? [y/n]");
					}else{
						strcpy(message, "error.");
					}
					break;
				case EDIT_BUFFER_CANCEL:
					return false;
				}
			}
		}
	}
}

bool CTyping::loadReplayLoop(){
	char message[256];
	strcpy(message, "");
	
	bool isLoaded = false;
	
	char buf[256];
	strcpy(buf, "");
	int len = 0;
	while(1){
		if(ProcessMessage() == -1){
			return false;
		}
		//ClearDrawScreen();
		
		DrawStringToHandle(10, 10, "find replay file", GetColor(255, 255, 255), m_fontHandleBig);
		if(len != 0){
			DrawStringToHandle(10, 50, buf, GetColor(255, 255, 255), m_fontHandleBig);
		}else{
			int color = GetColor(128, 128, 128);
			if(isLoaded){	/* セーブ後なら同じ扱い */
				color = GetColor(255, 255, 255);
			}
			DrawStringToHandle(10, 50, g_config.defaultReplayFile, color, m_fontHandleBig);
		}
		DrawStringToHandle(10, 90, message, GetColor(255, 255, 255), m_fontHandleBig);
		
		myScreenFlip();
		while(1){
			char ch = GetKeyboardInput();
			if(ch == 0){	/* キー入力処理終了 */
				break;
			}
			if(isLoaded){	/* ロード後、何か押したら戻る */
				return true;
			}
			strcpy(message, "");	/* 何か打ったらメッセージは消去 */
			
			switch(editBuffer(ch, buf, len)){	/* バッファを操作 */
			case EDIT_BUFFER_OK:
				isLoaded = loadReplay((len != 0) ? buf : g_config.defaultReplayFile);
				if(isLoaded){
					strcpy(message, "loaded.");
				}else{
					strcpy(message, "error.");
				}
				break;
			case EDIT_BUFFER_CANCEL:
				return false;
			}
		}
	}
}

/* ============================================================ */

#define COMMENT_N_LINES 12

struct MusicInfo{
	char m_title[256];
	char m_artist[256];
	char m_fumenAuthor[256];
	int m_level;
	char m_fumenFileName[256];
	char m_rankingFileName[256];
	char m_comment[COMMENT_N_LINES][256];
	
	int m_num;
	int m_numAll;
public:
	MusicInfo();
	MusicInfo(const MusicInfo &info);
	~MusicInfo();
	
	void load(CTyping &typing);
	
	void readRanking();
	
	void draw(int y, int brightness);
	void drawComment(int x, int y);
	void drawPlayData(int x, int y);
	void drawRanking(int x, int y, int rankBegin, int rankLen);
	
	bool titleCmp(const char *buf);
	//void renewFont();
private:
	void createFont();
	void deleteFont();
private:
	CRanking m_ranking;
	static int m_count;	/* 作られているMusicInfoの個数 */
	static int m_fontHandleNormal;
	static int m_fontHandleTitle;
	static int m_fontHandleRanking;
	static int m_fontHandleAchievement;
};

int MusicInfo::m_count = 0;
int MusicInfo::m_fontHandleNormal;
int MusicInfo::m_fontHandleTitle;
int MusicInfo::m_fontHandleRanking;
int MusicInfo::m_fontHandleAchievement;

MusicInfo::MusicInfo(){
//printfDx("%d++ ", m_count);
	if(m_count++ == 0){
		createFont();
	}
}

MusicInfo::MusicInfo(const MusicInfo &info){
	*this = info;
//printfDx("%d++ ", m_count);
	if(m_count++ == 0){
		createFont();
	}
}

MusicInfo::~MusicInfo(){
	if(--m_count == 0){
		deleteFont();
	}
//printfDx("--%d ", m_count);
}

void MusicInfo::load(CTyping &typing){
	typing.load(m_fumenFileName, m_rankingFileName);
}

/* ランキングを読み込む */
void MusicInfo::readRanking(){
	m_ranking.open(m_rankingFileName);
	m_ranking.read();
	m_ranking.close();
}

#define X_TITLE 40
#define Y_TITLE (30 - 30/2)
#define X_NUM (W_WINDOW - 25)
#define Y_NUM (60-44)
#define Y_ARTIST Y_NUM
#define X_LEVEL (W_WINDOW - 270)
#define Y_LEVEL (60-22)
#define X_F_AUTHOR (W_WINDOW - 30)
#define Y_F_AUTHOR Y_LEVEL

#define X_ACHIEVEMENT (X_TITLE - 25)
#define Y_ACHIEVEMENT (30 - 24/2)

void MusicInfo::draw(int y, int brightness){	/* 曲情報をyから高さ60で描く */
	int color = GetColor(brightness, brightness, brightness);
	/* タイトルの長さを測っておく */
	int widthTitle = GetDrawStringWidthToHandle(m_title, strlen(m_title), m_fontHandleTitle);
	
	/* 通し番号 */
	int widthNum = GetDrawFormatStringWidthToHandle(m_fontHandleNormal, "%d/%d", m_num, m_numAll);
	DrawFormatStringToHandle(X_NUM - widthNum, y + Y_NUM, color, m_fontHandleNormal,
		"%d/%d", m_num, m_numAll);
	
	int width;
	
	/* アーティスト */
	{
		width = GetDrawFormatStringWidthToHandle(m_fontHandleNormal, "%s", m_artist);
		int x;
		x = (W_WINDOW - 150) - width * 2 / 3;
		/* 前から2/3の位置をW_WINDOW - 150にあわせる */
		
		/* 左右の長さを取る */
		int widthL = x - (X_TITLE + widthTitle);
		int widthR = (X_NUM - widthNum) - (x + width);
		
		if(widthL <= 20 || widthR <=20){	/* タイトルや通し番号に近い場合は、 */
			x += (widthR - widthL) / 4;
			/* ↑で "/ 2" とするとタイトルの端と通し番号から近距離に */
			//x = (xLeft + xRight - width) / 2;	/* タイトルの端と通し番号から近距離に */
		}
		
		/* 左右の長さを取る（もう一回） */
		widthL = x - (X_TITLE + widthTitle);
		widthR = (X_NUM - widthNum) - (x + width);
		
		if(widthL < 0 || widthL + widthR <= 0){
		/* タイトルには被らないようにする */
		/* そもそも範囲に収まらないとき */
			x = X_TITLE + widthTitle;	/* タイトルの直後 */
		}else if(widthR < 0){
		/* タイトルには余裕があるが、通し番号に被るとき */
			x += widthR;	/* 通し番号の直前 */
		}
		DrawFormatStringToHandle(x, y + Y_ARTIST, color, m_fontHandleNormal,
			"%s", m_artist);
	}
	
	/* 譜面作者 */
	{
		int tmp = brightness * 2 / 3;
		int tmpColor = GetColor(tmp, tmp, tmp);
		width = GetDrawFormatStringWidthToHandle(m_fontHandleNormal, "(譜面作成　%s)", m_fumenAuthor);
		DrawFormatStringToHandle(X_F_AUTHOR - width, y + Y_F_AUTHOR, tmpColor, m_fontHandleNormal,
			"(譜面作成　%s)", m_fumenAuthor);
	}
	
	/* 難易度 */
	for(int i=0; i<5; i++){
		if(m_level <= i){
			int tmp = brightness / 2;
			int tmpColor = GetColor(tmp, tmp, tmp);
			DrawStringToHandle(X_LEVEL + 16 * i, y + Y_LEVEL, "☆",	/* ★ */
				tmpColor, m_fontHandleNormal);
		}else{
			DrawStringToHandle(X_LEVEL + 16 * i, y + Y_LEVEL, "★",
				GetColor(brightness, brightness, 0), m_fontHandleNormal);
		}
	}

	/* 達成度 */
	{
		int achievement = m_ranking.getScoreLevel();
		switch(achievement){
		case SCORE_NO_DATA:
			break;
		case SCORE_FAILED:
			DrawStringToHandle(X_ACHIEVEMENT, y + Y_ACHIEVEMENT, "●",
				GetColor(brightness/3, brightness/3, brightness/3), m_fontHandleAchievement,
				GetColor(brightness/4, brightness/4, brightness/4));
			break;
		case SCORE_RED_ZONE:
			DrawStringToHandle(X_ACHIEVEMENT, y + Y_ACHIEVEMENT, "●",
				GetColor(brightness*2/3, brightness/12, 0), m_fontHandleAchievement,
				GetColor(brightness/2, brightness/16, 0));
			break;
		case SCORE_YELLOW_ZONE:
			DrawStringToHandle(X_ACHIEVEMENT, y + Y_ACHIEVEMENT, "●",
				GetColor(brightness*7/12, brightness*2/3, 0), m_fontHandleAchievement,
				GetColor(brightness*7/16, brightness/2, 0));
			break;
		case SCORE_BLUE_ZONE:
			DrawStringToHandle(X_ACHIEVEMENT, y + Y_ACHIEVEMENT, "●",
				GetColor(0, brightness/6, brightness*2/3), m_fontHandleAchievement,
				GetColor(0, brightness/8, brightness/2));
			break;
		case SCORE_CLEAR:
			DrawStringToHandle(X_ACHIEVEMENT, y + Y_ACHIEVEMENT, "◆",
				GetColor(brightness*2/3, brightness*2/3, brightness*2/3), m_fontHandleAchievement,
				GetColor(brightness/2, brightness/2, brightness/2));
			break;
		case SCORE_FULL_COMBO:
			DrawStringToHandle(X_ACHIEVEMENT, y + Y_ACHIEVEMENT, "★",
				GetColor(brightness/4, brightness*5/8, brightness), m_fontHandleAchievement,
				GetColor(0, brightness/2, brightness));
			break;
		case SCORE_FULL_GOOD:
			DrawStringToHandle(X_ACHIEVEMENT, y + Y_ACHIEVEMENT, "★",
				GetColor(brightness/4, brightness, brightness/4), m_fontHandleAchievement,
				GetColor(0, brightness, 0));
			break;
		case SCORE_PERFECT:
			DrawStringToHandle(X_ACHIEVEMENT, y + Y_ACHIEVEMENT, "★",
				GetColor(brightness, brightness, brightness/4), m_fontHandleAchievement,
				GetColor(brightness, brightness, 0));
			break;
		}
	}
	
	/* タイトル（最後に書く） */
	DrawStringToHandle(X_TITLE, y + Y_TITLE, m_title, color, m_fontHandleTitle);
}

void MusicInfo::drawComment(int x, int y){
	for(int i = 0; i < COMMENT_N_LINES; i++){
		DrawStringToHandle(30+x, y+2 +20* i, m_comment[i], GetColor(255, 255, 255), m_fontHandleNormal);
	}
}

void MusicInfo::drawPlayData(int x, int y){
	m_ranking.drawPlayData(x, y, m_fontHandleNormal);
}


void MusicInfo::drawRanking(int x, int y, int rankBegin, int rankLen){
	m_ranking.draw(x, y, rankBegin, rankLen, m_fontHandleRanking);
}
/*
void MusicInfo::renewFont(){
	deleteFont();
	createFont();
}
*/

bool MusicInfo::titleCmp(const char *buf){
/* 【 関数名頭悪い。要修正 】 */
/* タイトルの一部と一致するかを返す */
	return isSubstr(buf, m_title);
}

void MusicInfo::createFont(){
	m_fontHandleNormal = CreateFontToHandle("ＭＳ ゴシック", 16, 3, DX_FONTTYPE_ANTIALIASING);
	m_fontHandleTitle = CreateFontToHandle("ＭＳ 明朝", 30, 3, DX_FONTTYPE_ANTIALIASING);
	m_fontHandleRanking = CreateFontToHandle(NULL, 16, 2);
	m_fontHandleAchievement = CreateFontToHandle("ＭＳ ゴシック", 24, 2, DX_FONTTYPE_ANTIALIASING_EDGE);
}

void MusicInfo::deleteFont(){	/* フォントを削除 */
	DeleteFontToHandle(m_fontHandleNormal);
	DeleteFontToHandle(m_fontHandleTitle);
	DeleteFontToHandle(m_fontHandleRanking);
	DeleteFontToHandle(m_fontHandleAchievement);
}

/* ============================================================ */

vector<MusicInfo> g_infoArray;

vector<MusicInfo>::iterator prevInfo(vector<MusicInfo>::iterator itr){
/* 1つ戻る。最初から1つ戻ると最後に。 */
	if(itr == g_infoArray.begin()){
		itr = g_infoArray.end();
	}
	itr--;
	return itr;
}

vector<MusicInfo>::iterator nextInfo(vector<MusicInfo>::iterator itr){
/* 1つ進む。1つ進んで最後を超えると最初に。 */
	itr++;
	if(itr == g_infoArray.end()){
		itr = g_infoArray.begin();
	}
	return itr;
}

vector<MusicInfo>::iterator randomInfo(){
	return &g_infoArray[GetRand(g_infoArray.size() - 1)];
}

/* ============================================================ */

struct DrawMainInfo{
public:
	vector<MusicInfo>::iterator musicInfoItr;	/* MusicInfoの配列のどこを指しているか */
	int rankingPos;
	int rankingFlag;	/* true:詳細ランキング、false:1位のみまたは、コメント表示 */
	
	int drawPosX;
	double drawPosY;
	int *addHeight;
private:
	int _addHeight[21];
	int m_addHeightWait;	/* 0から0以外になる時に待つ。それを数える */
	
	char m_findStr[256];	/* 検索に用いる文字列。再検索用 */
public:
	DrawMainInfo();
	void resetPos();
	
	void step();
	
	void next();
	void prev();
	
	void left();
	void right();
	
	void find(const char *buf);
	void findNext();
	void randomSelect();
};

DrawMainInfo::DrawMainInfo(){
	musicInfoItr = g_infoArray.begin();
	
	addHeight = _addHeight + 10;	/* 【これコピーした時にまずい予感】 */
	strcpy(m_findStr, "");
	
	resetPos();
}

void DrawMainInfo::resetPos(){
	rankingPos = 0;
	rankingFlag = false;
	drawPosX = 0;
	drawPosY = 0.0;
	for(int i=-10; i<=10; i++){
		addHeight[i] = 0;
	}
	m_addHeightWait = 25;
}

void DrawMainInfo::step(){
	for(int i=-10; i<=10; i++){
		int h = 0;	/* 本来の幅 */
		if(i==0){
			if(rankingFlag){
				h = H_RANKING;
			}else if(rankingPos == 0){
				h = H_RANKING1;
			}else{
				h = H_MUSIC_INFO_SUB;
			}
			if(addHeight[0] == 0 && m_addHeightWait > 0){
			/* 幅0から開こうとしていて、それに必要な時間経過していない */
				m_addHeightWait--;
				continue;
			}
		}
		addHeight[i] = h + (int)((addHeight[i] - h) * 0.80);
	}
	{
		drawPosX *= 0.70;
	}
	{
		int sign = drawPosY==0 ? 0 : drawPosY>0 ? 1 : -1;
		double tmp = -0.05 * drawPosY;	/* drawPosYを補正する量 */
		tmp -= sign * 1.0;
		tmp -= sign * 0.0010 * drawPosY * drawPosY;
		drawPosY += tmp;
		if(drawPosY * tmp > 0.0){	/* 0を通り過ぎた */
			drawPosY = 0.0;
		}
	}
}

void DrawMainInfo::next(){
	musicInfoItr = nextInfo(musicInfoItr);	/* 次の曲 */
	
	drawPosY += MUSIC_INFO_HEIGHT + (addHeight[0] + addHeight[1]) / 2.0;
	for(int i=-10; i<10; i++){
		addHeight[i] = addHeight[i+1];
	}
	addHeight[10] = 0;
	
	m_addHeightWait = 25;
	
	rankingPos = 0;
	rankingFlag = false;
}

void DrawMainInfo::prev(){
	musicInfoItr = prevInfo(musicInfoItr);	/* 前の曲 */
	
	drawPosY -= MUSIC_INFO_HEIGHT + (addHeight[0] + addHeight[-1]) / 2.0;
	for(int i=10; i>-10; i--){
		addHeight[i] = addHeight[i-1];
	}
	addHeight[-10] = 0;
	
	m_addHeightWait = 25;
	
	rankingPos = 0;
	rankingFlag = false;
}

void DrawMainInfo::left(){
	if(rankingPos < 0){	/* コメント表示の前へは行けない */
		return;
	}
	
	m_addHeightWait /= 2;	/* 詳細ランキングとかを見たい時には時間を減らしてあげる */
	
	if(rankingPos == 0 && rankingFlag){	/* 1位のみ←1～RANKING_DRAW_LEN位のとき */
		rankingFlag = false;
	}else{	/* その他の時 */
		rankingPos -= RANKING_DRAW_LEN;
		drawPosX -= W_WINDOW;
	}
}

void DrawMainInfo::right(){
	if(rankingPos + RANKING_DRAW_LEN >= RANKING_LEN){
		return;
	}
	
	m_addHeightWait /= 2;	/* 詳細ランキングとかを見たい時には時間を減らしてあげる */
	
	if(rankingPos == 0 && !rankingFlag){	/* 1位のみ→1～RANKING_DRAW_LEN位のとき */
		rankingFlag = true;
	}else{	/* その他の時 */
		rankingPos += RANKING_DRAW_LEN;
		drawPosX += W_WINDOW;
	}
}

void DrawMainInfo::find(const char *buf){
	strcpy(m_findStr, buf);
	findNext();
}

void DrawMainInfo::findNext(){
	if(strlen(m_findStr) > 0){	/* 何か入力されていれば検索 */
		vector<MusicInfo>::iterator tmpItr = musicInfoItr;
		do{
			musicInfoItr = nextInfo(musicInfoItr);
			if(musicInfoItr->titleCmp(m_findStr)){
				resetPos();
				/* 検索したものは最初から中心に表示 */
				break;
			}
		}while(musicInfoItr != tmpItr);	/* 元の場所に戻ってくるまで */
	}
}

void DrawMainInfo::randomSelect(){
	musicInfoItr = randomInfo();
	resetPos();
}

/* ============================================================ */

/* rankingPos位からのランキング（負の時コメント）を(x,y)から描画。高さはhに制限 */
void drawMainRanking(vector<MusicInfo>::iterator itr, int rankingPos,
		int x, int y, int h){
	int yMin = y, yMax = y+h;
	if(yMin < 0){
		yMin = 0;
	}
	if(yMax > 360){
		yMax = 360;
	}
	SetDrawArea(10, yMin, W_WINDOW-10, yMax);
	if(rankingPos >= 0){
		/* ランキング表示（rankingPosからRANKING_DRAW_LEN個） */
		/* 1位しか表示しない時も隠れるので問題なし */
		itr->drawRanking(x, y + 6, rankingPos, RANKING_DRAW_LEN);	/* 6==(60-48)/2 */
	}else{
		/* コメント表示 */
		itr->drawComment(x, y + 6);
		itr->drawPlayData(x, y + (6+H_COMMENT+4));
	}
}

/* メイン画面（曲選択画面）を描く */
void drawMain(DrawMainInfo &dInfo, CChallenge &challenge,
		const char *name, int inputHandle, int fontHandleDefault){
	//if(dInfo.rankingPos == -RANKING_DRAW_LEN){
	int dy = 180 + (MUSIC_INFO_HEIGHT - dInfo.addHeight[0])/2
		+ (int)floor(dInfo.drawPosY + 0.5);
	SetDrawArea(10, 0, W_WINDOW-10, 360);	/* 描画範囲を制限 */
	{
		DrawBox(10, dy - MUSIC_INFO_HEIGHT, W_WINDOW - 10, dy + dInfo.addHeight[0], GetColor(16, 16, 32), TRUE);
		dInfo.musicInfoItr->draw(dy - MUSIC_INFO_HEIGHT, 255);	/* タイトルなど表示 */
		DrawLine(40, dy, W_WINDOW - 40, dy, GetColor(64, 64, 64));
		
		drawMainRanking(dInfo.musicInfoItr, dInfo.rankingPos, dInfo.drawPosX, dy, dInfo.addHeight[0]);
		if(dInfo.drawPosX < 0){	/* 右のを描く */
			drawMainRanking(dInfo.musicInfoItr, dInfo.rankingPos + RANKING_DRAW_LEN,
				dInfo.drawPosX + W_WINDOW, dy, dInfo.addHeight[0]);
		}else if(dInfo.drawPosX > 0){	/* 左のを描く */
			drawMainRanking(dInfo.musicInfoItr, dInfo.rankingPos - RANKING_DRAW_LEN,
				dInfo.drawPosX - W_WINDOW, dy, dInfo.addHeight[0]);
		}
		/* 選んだものが見えていなくて、2つ左や右が見えている場合とかもあるが、わざわざ描かない */
	}
	SetDrawArea(10, 0, W_WINDOW-10, 360);	/* 描画範囲を制限 */
	/* タイトルなど表示、水平線で区切る */
	{	/* 前の曲 */
		vector<MusicInfo>::iterator itr = dInfo.musicInfoItr;
		int y = dy - MUSIC_INFO_HEIGHT;
		int cnt = 0;
		while(y >= 0){	/* 枠内の間 */
			DrawLine(10, y, W_WINDOW - 10, y,
				GetColor(255/(cnt+2), 255/(cnt+2), 255/(cnt+2)));
			itr = prevInfo(itr);
			cnt++;
			y -= MUSIC_INFO_HEIGHT + dInfo.addHeight[-cnt];
			(*itr).draw(y, 255/(cnt+1));
		}
	}
	{	/* 後の曲 */
		vector<MusicInfo>::iterator itr = dInfo.musicInfoItr;
		int y = dy + dInfo.addHeight[0];
		int cnt = 0;
		while(y <360){	/* 枠内の間 */
			DrawLine(10, y, W_WINDOW - 10, y,
				GetColor(255/(cnt+2), 255/(cnt+2), 255/(cnt+2)));
			itr = nextInfo(itr);
			cnt++;
			(*itr).draw(y, 255/(cnt+1));
			y += MUSIC_INFO_HEIGHT + dInfo.addHeight[cnt];
		}
	}
	SetDrawArea(0, 0, W_WINDOW, H_WINDOW);	/* 描画範囲を元に戻す */
	DrawBox(0, 360, W_WINDOW, H_WINDOW - 25, GetColor(24, 24, 24), TRUE);
	DrawLine(0, 360, W_WINDOW, 360, GetColor(170, 170, 170));
	
	if(inputHandle == -1){	/* 検索時以外 */
		DrawStringToHandle(10, 370, "↑/↓: 曲選択,   F: 検索, R: ランダム,   F5: 再読込,",
			GetColor(255, 255, 255), fontHandleDefault);
		
		DrawStringToHandle(10, 390, "←/→: 情報/ランキング表示, Tab: 全画面切替,",
			GetColor(255, 255, 255), fontHandleDefault);
		
		{
			char *str = "Enter: 決定";
			int width = GetDrawStringWidthToHandle(str, strlen(str), fontHandleDefault);
			DrawStringToHandle(W_WINDOW - 10 - width, 370, str,
				GetColor(255, 255, 255), fontHandleDefault);	/* 右から10のところに描画 */
		}
		{
			char *str = "!/?: Replay Save/Load";
			int width = GetDrawStringWidthToHandle(str, strlen(str), fontHandleDefault);
			DrawStringToHandle(W_WINDOW - 10 - width, 390, str,
				GetColor(255, 255, 255), fontHandleDefault);	/* 右から10のところに描画 */
		}
		
		DrawStringToHandle(10, H_WINDOW - 65, "[H]idden, [S]udden, [C]ircleStealth, [L]yricsStealth,",
			GetColor(255, 255, 255), fontHandleDefault);
		DrawStringToHandle(10, H_WINDOW - 45, "</>: Speed Down/Up, +/-: KeyShift,   F6/F7/F8: sin/cos/tan,",
			GetColor(255, 255, 255), fontHandleDefault);
		{
			char *str = "Reset to [D]efault";
			int width = GetDrawStringWidthToHandle(str, strlen(str), fontHandleDefault);
			DrawStringToHandle(W_WINDOW - 10 - width, H_WINDOW - 65, str,
				GetColor(255, 255, 255), fontHandleDefault);	/* 右から10のところに描画 */
		}
		{
			char *str = "Esc: 終了";
			int width = GetDrawStringWidthToHandle(str, strlen(str), fontHandleDefault);
			DrawStringToHandle(W_WINDOW - 10 - width, H_WINDOW - 45, str,
				GetColor(255, 255, 255), fontHandleDefault);	/* 右から10のところに描画 */
		}
	}else{
		//SetDrawArea(0, 360, W_WINDOW, H_WINDOW - 25);
		DrawStringToHandle(10, 370, "検索曲名:",
			GetColor(255, 255, 255), fontHandleDefault);
		DrawStringToHandle(10, H_WINDOW - 45, "Enter: 検索, （検索後に）F3: 次を検索",
			GetColor(255, 255, 255), fontHandleDefault);
		DrawKeyInputModeString(W_WINDOW, H_WINDOW - 45);	//(W_WINDOW, H_WINDOW - 25);
		DrawKeyInputString(10, 390, inputHandle);	/* 検索文字列を表示 */
		//SetDrawArea(0, 0, W_WINDOW, H_WINDOW);	/* 描画範囲を元に戻す */
	}
	
	{	/* 名前を表示 */
		char buf[256];
		int color;
		if(strlen(name) > 0){
			strcpy(buf, name);
			color = GetColor(255, 255, 255);
		}else{
			strcpy(buf, "(guest)");
			color = GetColor(128, 128, 128);
		}
		int width = GetDrawStringWidthToHandle(buf, strlen(buf), fontHandleDefault);
		DrawBox(W_WINDOW - 20 - width, H_WINDOW - 25, W_WINDOW, H_WINDOW, GetColor(16, 16, 32), TRUE);
		DrawStringToHandle(W_WINDOW - 10 - width, H_WINDOW - 20, buf, color, fontHandleDefault);
	}
	
	DrawLine(0, H_WINDOW - 25, W_WINDOW, H_WINDOW - 25, GetColor(170, 170, 170));
	
	if(challenge.test(CHALLENGE_STEALTH)){
		DrawStringToHandle(130, H_WINDOW - 20, "[ CStealth ]", GetColor(255, 32, 0), fontHandleDefault);
	}else{
		if(challenge.test(CHALLENGE_HIDDEN)){
			if(challenge.test(CHALLENGE_SUDDEN)){
				DrawStringToHandle(130, H_WINDOW - 20, "[ Hid.Sud. ]", GetColor(255, 128, 0), fontHandleDefault);
			}else{
				DrawStringToHandle(130, H_WINDOW - 20, "[  Hidden  ]", GetColor(255, 255, 0), fontHandleDefault);
			}
		}else{
			if(challenge.test(CHALLENGE_SUDDEN)){
				DrawStringToHandle(130, H_WINDOW - 20, "[  Sudden  ]", GetColor(255, 255, 0), fontHandleDefault);
			}else{
				DrawStringToHandle(130, H_WINDOW - 20, "[ Hid.Sud. ]", GetColor(64, 64, 64), fontHandleDefault);
			}
		}
	}
	{
		int color;
		if(challenge.test(CHALLENGE_LYRICS_STEALTH)){
			color = GetColor(255, 128, 0);
		}else{
			color = GetColor(64, 64, 64);
		}
		DrawStringToHandle(230, H_WINDOW - 20, "[ LStealth ]", color, fontHandleDefault);
	}
	{
		char buf[256];
		int color;
		sprintf(buf, "[ Speed x%3.1f ]", challenge.speed());
		if(challenge.speed() > 3.0){
			color = GetColor(255, 32, 0);
		}else if(challenge.speed() > 2.0){
			color = GetColor(255, 128, 0);
		}else if(challenge.speed() > 1.0){
			color = GetColor(255, 255, 0);
		}else if(challenge.speed() < 1.0){
			color = GetColor(128, 128, 255);
		}else{
			color = GetColor(64, 64, 64);
		}
		DrawStringToHandle(10, H_WINDOW - 20, buf, color, fontHandleDefault);
	}
	
	{
		char buf[256];
		int color;
		if(challenge.key() > 8){
			sprintf(buf, "[ Key +%2d ]", challenge.key());
			color = GetColor(255, 32, 0);
		}else if(challenge.key() > 4){
			sprintf(buf, "[ Key +%2d ]", challenge.key());
			color = GetColor(255, 128, 0);
		}else if(challenge.key() > 0){
			sprintf(buf, "[ Key +%2d ]", challenge.key());
			color = GetColor(255, 255, 0);
		}else if(challenge.key() < 0){
			sprintf(buf, "[ Key -%2d ]", -challenge.key());
			color = GetColor(128, 255, 0);
		}else{
			sprintf(buf, "[ Key %3d ]", challenge.key());
			color = GetColor(64, 64, 64);
		}
		DrawStringToHandle(330, H_WINDOW - 20, buf, color, fontHandleDefault);
	}
	
	if(challenge.test(CHALLENGE_SIN)){
		if(challenge.test(CHALLENGE_COS)){
			if(challenge.test(CHALLENGE_TAN)){
				DrawStringToHandle(430, H_WINDOW - 20, "[sct]", GetColor(255, 128, 0), fontHandleDefault);
			}else{
				DrawStringToHandle(430, H_WINDOW - 20, "[sc-]", GetColor(255, 128, 0), fontHandleDefault);
			}
		}else{
			if(challenge.test(CHALLENGE_TAN)){
				DrawStringToHandle(430, H_WINDOW - 20, "[s-t]", GetColor(255, 128, 0), fontHandleDefault);
			}else{
				DrawStringToHandle(430, H_WINDOW - 20, "[sin]", GetColor(255, 255, 0), fontHandleDefault);
			}
		}
	}else{
		if(challenge.test(CHALLENGE_COS)){
			if(challenge.test(CHALLENGE_TAN)){
				DrawStringToHandle(430, H_WINDOW - 20, "[-ct]", GetColor(255, 128, 0), fontHandleDefault);
			}else{
				DrawStringToHandle(430, H_WINDOW - 20, "[cos]", GetColor(255, 255, 0), fontHandleDefault);
			}
		}else{
			if(challenge.test(CHALLENGE_TAN)){
				DrawStringToHandle(430, H_WINDOW - 20, "[tan]", GetColor(255, 255, 0), fontHandleDefault);
			}else{
				DrawStringToHandle(430, H_WINDOW - 20, "[sct]", GetColor(64, 64, 64), fontHandleDefault);
			}
		}
	}
	/*
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
	*/
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
	case CTRL_CODE_F6:
		challenge.flip(CHALLENGE_SIN);
		break;
	case CTRL_CODE_F7:
		challenge.flip(CHALLENGE_COS);
		break;
	case CTRL_CODE_F8:
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

bool main2(bool &isWindowMode, const char *name){
	/* falseを返せば、終了、trueを返せば、isWindowModeでウィンドウを必要なら変更して再読み込み */
	int fontHandleDefault = CreateFontToHandle(NULL, 16, 3);
	
	bool retValue;	/* 画面モードを変えて続ける場合true */
	
	DrawMainInfo dInfo;
	
	CChallenge challenge;
	
	int inputHandle = -1;	/* 検索用入力のキー入力ハンドル */
	SetKeyInputStringFont(fontHandleDefault);
	SetKeyInputStringColor(
		GetColor( 255 , 255 , 255 ) ,
		GetColor( 255 , 255 , 255 ) ,
		GetColor( 255 , 255 ,   0 ) ,
		GetColor(   0 , 255 , 255 ) ,
		GetColor( 255 , 255 , 255 ) ,
		GetColor( 255 ,  0 ,  0 ) ,
		GetColor( 255 , 255 ,  20 )
	);
	
	CTyping typing;
	
	typing.setName(name);
	
	while(1){
		if(ProcessMessage() == -1){
			retValue = false;
			break;
		}
		
		//ClearDrawScreen();
		drawMain(dInfo, challenge, name, inputHandle, fontHandleDefault);
		dInfo.step();
		myScreenFlip();
		
		if(inputHandle == -1){	/* 検索文字列を入力中ではない */
			char ch = GetKeyboardInput();
			switch(ch){
			case CTRL_CODE_ESC:	/* 終了 */
				retValue = false;
				goto L1;
			case CTRL_CODE_F5:	/* 再読み込み */
				retValue = true;
				goto L1;
			case CTRL_CODE_TAB:	/* Window ←→ FullScreen */
				isWindowMode = !isWindowMode;	/* Windowかどうかを逆にする */
				retValue = true;
				goto L1;
			case CTRL_CODE_CR:	/* ゲーム開始 */
				typing.setChallenge(challenge);	/* ロード前にチャレンジを設定 */
				(*dInfo.musicInfoItr).load(typing);	/* ゲーム情報「に」譜面・ハイスコアのファイルをロード */
				/* ↑これをtyping.nanka(musicInfoItr)みたいにしたいんだが、いろいろ面倒 */
				typing.mainLoop();	/* 描画とかをCTypingに任せる */
				typing.unload();	/* 終了処理 */
				
				dInfo.resetPos();
				/* 戻ってきたときに、詳細ランキングから抜ける */
				/* （なんとなく、1位にランクインしたのにほかのところが表示されてたら悲しい） */
				
				(*dInfo.musicInfoItr).readRanking();	/* ランキングは更新されているかもしれない */
				break;
			/* 最後のゲームのを再生 */
			/*
			case '@':
				(*musicInfoItr).load(typing);
				typing.replayLoop();
				typing.unload();
				break;
			*/
			case '!':	/* リプレイを保存*/
				typing.saveReplayLoop();
				break;
			case '?':	/* 保存されたリプレイを再生 */
				if(!typing.loadReplayLoop()){
					break;	/* リプレイの読み込みに失敗した場合は戻る */
				}
				typing.setChallenge(challenge);	/* ロード前にチャレンジを設定 */
				(*dInfo.musicInfoItr).load(typing);
				typing.replayLoop();
				typing.unload();
				break;
			case 'r':	/* ランダム選択 */
			case 'R':
				dInfo.randomSelect();
				break;
			case 'f':	/* 検索 */
			case 'F':
				/* キャンセルあり、半角または数字に限定はしない */
				inputHandle = MakeKeyInput(64, TRUE, FALSE, FALSE);
				if(inputHandle == -1){
					throw __LINE__;
				}
				SetActiveKeyInput(inputHandle);
				break;
			case CTRL_CODE_F3:	/* 次を検索 */
				dInfo.findNext();
				break;
			case CTRL_CODE_UP:	/* 前の曲へ */
				dInfo.prev();
				break;
			case CTRL_CODE_DOWN:	/* 後の曲へ */
				dInfo.next();
				break;
			case CTRL_CODE_LEFT:	/* 上位を表示 */
				dInfo.left();
				break;
			case CTRL_CODE_RIGHT:	/* 下位を表示 */
				dInfo.right();
				break;
			default:
				editChallenge(challenge, ch);
				break;
			}
			
		}else{	/* 検索文字列を入力中 */
			int tmp = CheckKeyInput(inputHandle);
			switch(tmp){
			case 0:
				break;
			case 1:	/* 入力終了 */
				char buf[256];
				GetKeyInputString(buf, inputHandle);
				dInfo.find(buf);
				/* ここで、 break; しない */
			case 2:
				/* 上の場合に加え、キャンセルされた場合 */
				DeleteKeyInput(inputHandle);
				inputHandle = -1;
				ClearKeyboardInput();	/* キー入力（自分で処理してる方）を消しておく */
				break;
			default:	/* 何かエラー（-1のとき） */
				throw __LINE__;
			}
		}
	}
	
L1:
	DeleteFontToHandle(fontHandleDefault);
	return retValue;
}

/* flag はリストの読み込みが完了したかどうか */
void drawTitle(int fontHandleTitle, int fontHandleCopyright, int fontHandleInfo, const char *strInfo){
	//drawBox(0, 0, W_WINDOW, H_WINDOW, Color(32, 32, 32);
	{
		const char *strTitle = "UTyping";
		int strWidth = GetDrawStringWidthToHandle(strTitle, strlen(strTitle), fontHandleTitle);
		DrawStringToHandle((W_WINDOW - strWidth) / 2, H_WINDOW / 3 - 24, strTitle,
			GetColor(255, 255, 255), fontHandleTitle, GetColor(170, 170, 170));
	}
	{
		const char *strCopyright = "(c)2007-2008 tos";
		int strWidth = GetDrawStringWidthToHandle(strCopyright, strlen(strCopyright), fontHandleCopyright);
		DrawStringToHandle(W_WINDOW - 10 - strWidth, H_WINDOW - 10 - 12, strCopyright,
			GetColor(255, 255, 255), fontHandleCopyright);
	}
	{
		int strWidth = GetDrawStringWidthToHandle(strInfo, strlen(strInfo), fontHandleInfo);
		DrawStringToHandle((W_WINDOW - strWidth) / 2, (H_WINDOW) * 2 / 3 - 12, strInfo,
			GetColor(255, 255, 255), fontHandleInfo);
	}
	/*
	DrawStringToHandle(10, 10, "内部データ構造を大幅に書き換えました。",
		GetColor(255, 255, 0), fontHandleInfo);
	DrawStringToHandle(10, 40, "挙動がおかしくなった点があれば教えてください。",
		GetColor(255, 255, 0), fontHandleInfo);
	*/
}

int readList(){
	int count = 0;
	g_infoArray.clear();	/* 読み込む前に消去 */
	FILE *fpList;
	fpList = fopen("UTyping_list.txt", "r");	/* リストを開く */
	if(fpList == NULL){
		throw "UTyping_list.txt が開けません。";
	}
	char buf[256];
	while(fgetline(buf, fpList) != NULL){
		FILE *fpInfo;
		MusicInfo info;
		fpInfo = fopen(buf, "r");	/* 情報ファイルを開く */
		if(fpInfo == NULL){	/* 開けないファイルは無視 */
			continue;
		}
		char dirName[256];	/* ディレクトリは情報ファイルを基準とする */
		getDirFromPath(dirName, buf);
		char buf0[256], buf1[256], buf2[256], *chk;
		fgetline(info.m_title, fpInfo);	/* 1行目にタイトル */
		fgetline(info.m_artist, fpInfo);	/* 2行目に原作者 */
		fgetline(info.m_fumenAuthor, fpInfo);	/* 3行目に譜面作者 */
		fgetline(buf0, fpInfo);
		fgetline(buf1, fpInfo);
		chk = fgetline(buf2, fpInfo);
		if(chk == NULL){	/* 全行読み込めなかった */
			continue;
		}
		for(int i = 0; i < COMMENT_N_LINES; i++){	/* 残りの行はコメント */
			chk = fgetline(info.m_comment[i], fpInfo);
			if(chk == NULL){
				strcpy(info.m_comment[i],"");
			}
		}
		fclose(fpInfo);	/* 情報ファイルを閉じる */
		int n = sscanf(buf0, "%d", &info.m_level);	/* 4行目に難易度 */
		if(n < 1){
			continue;
		}
		sprintf(info.m_fumenFileName, "%s%s", dirName, buf1);	/* 5行目に譜面ファイル名 */
		sprintf(info.m_rankingFileName, "%s%s", dirName, buf2);	/* 6行目にハイスコアファイル名 */
		count++;
		info.m_num = count;	/* 通し番号をつける */
		g_infoArray.push_back(info);
	}
	fclose(fpList);	/* リストを閉じる */
	
	for(vector<MusicInfo>::iterator i = g_infoArray.begin(); i != g_infoArray.end(); i++){
		(*i).readRanking();		/* ランキングを読み込み */
		(*i).m_numAll = count;
	}
	
	return count;
}

void main1(bool &isWindowMode){
	char name[NAME_LEN + 1];
	int nameLen;
	name[0] = '\0';
	nameLen = 0;
	while(1){
		g_config.read();	/* configを読み込む */
		
		myDxLib_Init();
		
		/* VSyncを待つかを設定 */
		if(SetWaitVSyncFlag(g_config.f_waitVSync) == -1){
			throw "WaitVSyncの設定に失敗しました。";
		}
		
		g_fontHandleDebug = CreateFontToHandle(NULL, 10, 3, DX_FONTTYPE_EDGE);
		
		{
			int fontHandleTitle = CreateFontToHandle("ＭＳ 明朝", 48, 3, DX_FONTTYPE_ANTIALIASING_EDGE);
			int fontHandleNormal = CreateFontToHandle("ＭＳ ゴシック", 16, 2, DX_FONTTYPE_ANTIALIASING);
			int fontHandleInfo = CreateFontToHandle("ＭＳ 明朝", 24, 3, DX_FONTTYPE_ANTIALIASING);
			int fontHandleName = CreateFontToHandle("ＭＳ 明朝", 36, 2, DX_FONTTYPE_ANTIALIASING);
			//ClearDrawScreen();
			drawTitle(fontHandleTitle, fontHandleNormal, fontHandleInfo, "Now loading...");
			myScreenFlip();
			
			bool isCorrectID = checkUTypingUserID();
			
			int count = readList();
			
			if(count == 0){
				throw "曲が一曲も存在しません。";
			}
			
			while(1){
				if(ProcessMessage() == -1){
					return;
				}
				//SetDrawScreen(DX_SCREEN_BACK);
				//ClearDrawScreen();
				drawTitle(fontHandleTitle, fontHandleNormal, fontHandleInfo,
					isCorrectID ? "Press Enter key." : "Wrong UTyping user ID...");
				if(isCorrectID){
					DrawFormatStringToHandle(30, 375, GetColor(255, 255, 255), fontHandleNormal,
						"名前を入力してください :");
					if(nameLen > 0){
						DrawStringToHandle(60, 400, name, GetColor(255, 255, 255), fontHandleName);
					}else{
						DrawStringToHandle(60, 400, "（未設定）", GetColor(128, 128, 128), fontHandleName);
					}
				}
				myScreenFlip();
				char ch = GetKeyboardInput();
				if(ch == 0){
					continue;
				}
				if(!isCorrectID){	/* IDが不正な場合は、どのキーを押しても終了。 */
					return;
				}
				if(ch == CTRL_CODE_TAB){	/* Tabを押すと、Window ←→ FullScreen */
					isWindowMode = !isWindowMode;	/* isWindowModeを変更して再起動 */
					break;
				}
				int ret = editBuffer(ch, name, nameLen);
				if(ret == EDIT_BUFFER_OK){	/* 改行が押されたら曲選択画面へ */
					if(!main2(isWindowMode, name)){	/* falseが戻った場合（終了するとき） */
						return;
					}
					break;	/* trueが戻った場合（再起動するとき） */
					
				}else if(ret == EDIT_BUFFER_CANCEL){	/* 入力がキャンセルされた */
					if(nameLen > 0){	/* 入力があった場合は入力を消す */
						name[0] = '\0';
						nameLen = 0;
					}else{	/* そうでない場合は終了 */
						return;
					}
				}
			}
			
			g_infoArray.clear();
			
			DeleteFontToHandle(fontHandleTitle);
			DeleteFontToHandle(fontHandleInfo);
			DeleteFontToHandle(fontHandleName);
			DeleteFontToHandle(fontHandleNormal);
		}
		DeleteFontToHandle(g_fontHandleDebug);
		if(ChangeWindowMode(isWindowMode) != DX_CHANGESCREEN_OK){
			/* WindowModeの切り替えに失敗した */
			throw "Window ←→ FullScreen の切り替えに失敗しました。";
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow){
	
	try{
		g_config.read();	/* configを読み込む */
		bool isWindowMode = !g_config.f_fullScreen;
		ChangeWindowMode(isWindowMode);
		
		SetMainWindowText("UTyping");
		
		SetHookWinProc(KeyboardInputProc);
		
		main1(isWindowMode);
	}catch(int n){
		char buf[256];
		sprintf(buf, "%d 行目でエラーが発生しました。", n);
		outputError(buf);
	}catch(char *str){
		outputError(str);
	}
	myDxLib_End();
	return 0;
}

#if 0
int main(){
	return WinMain(0,0,0,0);
}
#endif
