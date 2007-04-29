#include "DxLib.h"

#include <stdio.h>
#include <string.h>
#include <vector>

using namespace std;

#define INFTY 1000000000

enum{
PHASE_NOT_STARTED,	/* 開始前 */
PHASE_STARTED,	/* メイン */
PHASE_RESULT,	/* スコア表示 */
PHASE_FINISHED,	/* 名前入力後（ハイスコア処理完了） */
PHASE_EXIT,	/* 終わったので消してもらう */
};

#define TYPE_BUFFER_LEN 8

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

/* ============================================================ */

class CScore{
public:
	CScore();
	CScore(const char *n,int s,int sa,int st,int ce,int cg,int cf,int cp,int cx,int ca,int cm);
	void read(FILE *fp);
	void write(FILE *fp);
	bool operator ==(CScore score){
		return m_score == score.m_score;
	}
	bool operator !=(CScore score){
		return m_score != score.m_score;
	}
	bool operator <(CScore score){
		return m_score < score.m_score;
	}
	bool operator <=(CScore score){
		return m_score <= score.m_score;
	}
	bool operator >(CScore score){
		return m_score > score.m_score;
	}
	bool operator >=(CScore score){
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
	fscanf(fp, "%s%d%d%d%d%d%d%d%d%d%d", m_name, &m_score, &m_scoreAccuracy, &m_scoreTyping,
		&m_countExcellent, &m_countGood, &m_countFair, &m_countPoor, &m_countPass,
		&m_countAll, &m_comboMax);
}

void CScore::write(FILE *fp){
	fprintf(fp, "%s %d %d %d %d %d %d %d %d %d %d\n", m_name, m_score, m_scoreAccuracy, m_scoreTyping,
		m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass,
		m_countAll, m_comboMax);
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
private:
	FILE *m_fp;
	CScore m_score[5];
};

CRanking::CRanking(){
	m_fp = NULL;	/* まだファイルを開いていない */
}

CRanking::~CRanking(){
	close();
}

/* ランクインなら順位(0-4)を返す。そうでなければ -1 を返す */
int CRanking::update(CScore &score){
	for(int i = 0; i < 5; i++){
		if(score > m_score[i]){	/* i位より大きい */
			for(int j = 5 - 1; j > i; j--){	/* i位以降のデータを後ろに1つずつずらす */
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
	m_fp = fopen(fileName, "r+");	/* あればそれを開いて、 */
	if(m_fp == NULL){
		m_fp = fopen(fileName, "w+");	/* なければ作成 */
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
	for(int i=0; i<5; i++){
		m_score[i].read(m_fp);
	}
	rewind(m_fp);
	return;
}

void CRanking::write(){
	rewind(m_fp);
	for(int i=0; i<5; i++){
		m_score[i].write(m_fp);
	}
	rewind(m_fp);
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
//printf("%s %d\n",m_str,m_len);
}

bool CConvertData::match_front(const char *str){
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
	//void chk();
private:
	CTrieNode *m_next[256];
public:
	/* もうprivateにするの面倒 */
	vector<CConvertData> m_data;
};
/*
void CTrieNode::chk(){
	for(vector<CConvertData>::iterator i=m_data.begin(); i != m_data.end(); i++){
		printf("%s(%d) ", (*i).m_str, (*i).m_len);
	}
	printf("|\n");
}
*/
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
//printf("[%s %s %d]",str,data.m_str,data.m_len);
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
	string str;	/* 歌詞 */
	double timeBegin;	/* 表示が始まる時間 */
	double timeEnd;	/* 表示が終わる時間 */
};

/* ============================================================ */

class CTyping{
public:
	CTyping();
	~CTyping();
	// void loadDictionary(const char *fileName);
	/* ローマ字辞書はコンストラクタで読む */
	void load(const char *fileName);
	void loadRanking(const char *fileName);
	void keyboard(char ch);
	bool idle();
private:
	void setText(const char *str, int color);
	void setTime();
	double getTime();
	void phase(int phaseNum);
	bool input(char *typeBuffer, int &typeBufferLen,
		vector<Lyrics>::iterator &lyricsPosition, double time);
	void scoreTyping(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd);
	void scoreAccuracy(double time, vector<Lyrics>::iterator lyricsPosition);
public:
	void draw();
	void drawResult();
private:
	CTrieNode m_trie;
	
	vector<Lyrics> m_lyrics;
	vector<Lyrics>::iterator m_lyricsPosition;
	char m_typeBuffer[TYPE_BUFFER_LEN + 1];
	int m_typeBufferLen;
	
	vector<LyricsKanji> m_lyricsKanji;
	vector<LyricsKanji>::iterator m_lyricsKanjiPosition;
	
	int m_phase;
	
	int m_timeStart;
	
	int m_score;
	int m_scoreTyping;	/* タイピングによる得点 */
	int m_scoreAccuracy;	/* 精度による得点 */
	int m_combo;
	
	int m_comboMax;
	
	int m_countExcellent;
	int m_countGood;
	int m_countFair;
	int m_countPoor;
	int m_countPass;
	int m_countAll;
	
	CRanking m_ranking;	/* ハイスコアのデータ */
	int m_rank;	/* ハイスコアの中での順位、ランクインしなければ -1 */
	
	char m_text[256];
	int m_textColor;
	double m_textTime;
	
	char m_musicFileName[256];
	
	int m_fontHandleNormal;
	int m_fontHandleBig;
};

CTyping::CTyping(){
	FILE *fp;
	fp = fopen("convert.dat","r");
	if(fp == NULL){
		throw __LINE__;
	}
	char buf[256], buf1[256], buf2[256], buf3[256];
	while(fgets(buf, sizeof(buf), fp)!=NULL){
		int n=sscanf(buf, "%s%s%s", buf1, buf2, buf3);
		/* ローマ字、日本語、残るローマ字 */
		if(n<2){
			throw __LINE__;
		}
		int len;
		if(n == 2){
			strcpy(buf3, "");
		}
		m_trie.insert(buf2, CConvertData(buf1, buf3));
		/* 日本語をキーにローマ字のデータを入れる */
	}
	fclose(fp);
	
	m_fontHandleNormal = CreateFontToHandle(NULL, 16, 3);
	m_fontHandleBig = CreateFontToHandle("ＭＳ 明朝", 36, 2);
	
	//ClearInputCharBuf();	/* キー入力を消す */
	
	//timeBeginPeriod(1);	/* timeGetTimeの精度を上げる */
}

CTyping::~CTyping(){
	DeleteFontToHandle(m_fontHandleNormal);	/* フォントを削除 */
	DeleteFontToHandle(m_fontHandleBig);
	
	if(CheckMusic()){	/* 音楽が流れてたら止める */
		StopMusic();
	}
	//timeEndPeriod(1);	/* timeGetTimeの精度を元に戻す */
}

void CTyping::load(const char *fileName){
	FILE *fp;
	fp = fopen(fileName,"r");
	if(fp == NULL){
		throw __LINE__;
	}
	
	/* 読み込む前に全消去 */
	m_lyrics.clear();
	m_lyricsKanji.clear();
	strcpy(m_musicFileName, "");
	
	/* 歌詞のかたまりの数（=m_count○○の和の最大値）を数えるために0に */
	m_countAll = 0;
	
	/* 読み込み開始 */

#if 0
	/* 番兵として最初に' ',0を追加しておく */
	{
		Lyrics ly;
		ly.ch = ' ';
		ly.timeJust = 0;
		ly.timeUntil = 0;
		ly.isBlockStart = true;
		ly.isTyped = true;
		ly.isScoringTarget = false;
		m_lyrics.push_back(ly);
	}
#endif
	
	char tmpBuf[256];
	char buf[256], bufLast[256];
	strcpy(bufLast,"");
	double time, timeLast;
	bool flag = true;
	while(flag){
		if(fgets(tmpBuf,sizeof(tmpBuf),fp) == NULL){
			flag = false;
			time = INFTY;
		}else{
			/* 改行コードは消す */
			int len = strlen(tmpBuf);
			if(tmpBuf[len - 1] == '\n'){
				tmpBuf[len - 1] = '\0';
			}
			int n;
			switch(tmpBuf[0]){
			case '+':	/* 打つ歌詞 */
				n = sscanf(tmpBuf + 1, "%lf%s", &time, buf);
				if(n < 2){
					throw __LINE__;
				}
				break;
			case '*':	/* 表示する歌詞 */
				{
					char *ptr = strtok(tmpBuf + 1, " \t\n");	/* スペースで区切られるまでの部分を取り出す */
					if(ptr == NULL){
						throw __LINE__;
					}
					n = sscanf(ptr, "%lf", &time);	/* 時刻を読む */
					if(n < 1){
						throw __LINE__;
					}
					ptr = strtok(NULL, "");	/* 残りの部分を取り出す */
					LyricsKanji lk;
					lk.str = string(ptr);
					lk.timeBegin = time;
					lk.timeEnd = INFTY;	/* 終わりが設定されなければ、表示され続ける */
					m_lyricsKanji.push_back(lk);
				}
				continue;
			case '/':	/* 区切り */
				n = sscanf(tmpBuf + 1, "%lf", &time);
				if(n < 1){
					throw __LINE__;
				}
				strcpy(buf, " ");
				
				if(!m_lyricsKanji.empty()){	/* 表示する歌詞が存在している */
					LyricsKanji &lk = *(m_lyricsKanji.end() - 1);	/* そのなかで最後のものを操作 */
					if(lk.timeEnd == INFTY){	/* 終了時刻が設定されていなければ、現在に設定 */
						lk.timeEnd = time;
					}
				}
				break;
			case '@':
				strcpy(m_musicFileName, tmpBuf + 1);	/* 再生する音楽ファイル名を設定 */
				continue;
			default:
				continue;
			}
//printf("%s(%3.1f)",buf,time);
		}
		Lyrics ly;
		ly.isBlockStart = true;
		ly.isScoringTarget = true;
		for(char *ptr=bufLast; *ptr!='\0'; ptr++){
			ly.ch = *ptr;
			ly.timeJust = timeLast;
			ly.timeUntil = time;	/* 次の歌詞のtimeJustまで */
//printf("%c ,%lf,%lf\n",ly.ch,ly.timeJust,ly.timeUntil);
			if(ly.ch == ' '){
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
//printf("]\n");
		timeLast = time;
		strcpy(bufLast, buf);
	}
//printf("\n");
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
		lk.str = "";
		lk.timeBegin = INFTY;
		lk.timeEnd = INFTY;
		m_lyricsKanji.push_back(lk);
	}
	
	/* 現在打っている歌詞の位置 */
	m_lyricsPosition = m_lyrics.begin();
	m_lyricsKanjiPosition = m_lyricsKanji.begin();
	
	/* キー入力を初期化 */
	m_typeBufferLen = 0;
	
	/* 何かキーを押すまでは始まらないので、まだはじまっていない */
	phase(PHASE_NOT_STARTED);
	
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
}

void CTyping::loadRanking(const char *fileName){
	m_ranking.open(fileName);
	m_ranking.read();
}

void CTyping::keyboard(char ch){
	if(m_phase == PHASE_NOT_STARTED){	/* 開始前なら */
		phase(PHASE_STARTED);
		PlayMusic(m_musicFileName, DX_PLAYTYPE_BACK);	/* 音楽を流し始める */
		setTime();	/* 始まった時刻を覚える */
		return;
	}
	if(m_phase == PHASE_RESULT){	/* スコア表示中なら */
		/* 名前記入のためにバッファを読み書き */
		if(ch < CTRL_CODE_CMP){	/* 文字コードでないとき */
			switch(ch){
			case CTRL_CODE_CR:	/* 改行なら確定 */
				if(m_typeBufferLen > 0){	/* 名前が入力されているか確認 */
					phase(PHASE_FINISHED);
				}
				break;
			case CTRL_CODE_BS:	/* BackSpaceや左キーやDeleteなら */
			case CTRL_CODE_LEFT:
			case CTRL_CODE_DEL:
				if(m_typeBufferLen > 0){
					m_typeBuffer[--m_typeBufferLen] = '\0';	/* 一文字削除 */
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
			m_typeBuffer[m_typeBufferLen] = '\0';
		}
		return;
	}
	if(m_phase == PHASE_FINISHED){	/* ランキング表示中なら */
		if(ch == CTRL_CODE_CR){	/* Enterで終了 */
			phase(PHASE_EXIT);
		}
		return;
	}
	
	if(m_phase != PHASE_STARTED){	/* ここから先はゲームメインのときの処理 */
		return;
	}
	if(ch == '\0' || ch == ' '){	/* いろいろ困りそうだし、打つ意味がないから */
		return;
	}
	double time = getTime();
	
	if(time < (*m_lyricsPosition).timeJust - SEC_POOR){
		/* poorより前なので打てない */
//printf("!\n!\n");
		return;
	}
	
	m_typeBuffer[m_typeBufferLen++] = ch;
	m_typeBuffer[m_typeBufferLen] = '\0';
	
	if(input(m_typeBuffer, m_typeBufferLen, m_lyricsPosition, time)){
	/* その入力が現在の位置で入った */
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
	tmpBuffer[1] = '\0';
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
			if(input(tmpBuffer, tmpLen, tmpLyricsPosition, time)){
			/* 新しい位置でその入力が入った */
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
	if(m_phase == PHASE_NOT_STARTED){	/* 開始していなければ特にすることは無い */
		return true;
	}
	if(m_phase != PHASE_STARTED){	/* ここから先はゲームメインのときの処理 */
		return true;
	}
	
	if(CheckMusic() <= 0){	/* 音楽の再生が終了している */
		phase(PHASE_RESULT);
		return true;
	}
	double time = getTime();
	for(vector<Lyrics>::iterator i = m_lyricsPosition; ; i++){
		if((*i).ch == ' '){	/* 打ち遅れている歌詞をたどったら、大きな区切れまで来てしまった */
			m_combo = 0;	/* コンボは途切れる */
			m_typeBufferLen = 0;
			m_lyricsPosition = i;	/* 入力位置をそこまで移動してしまう */
			break;
		}
		if((*i).timeUntil >= time){
			break;
		}
	}
	return true;
}

void CTyping::setText(const char *str, int color){	/* 表示文字列を設定 */
	m_textTime = getTime();
	strcpy(m_text, str);
	m_textColor = color;
}

void CTyping::setTime(){	/* 開始時刻を設定 */
	m_timeStart = GetNowCount();
}

double CTyping::getTime(){	/* 開始時刻からの経過秒を取得 */
	return (GetNowCount() - m_timeStart) / 1000.0;
}

void CTyping::phase(int phaseNum){	/* 終了して、スコア表示に */
	if(phaseNum == PHASE_RESULT){
		setTime();	/* 表示開始されてからの時間を利用するためにセット */
		m_typeBuffer[0]= '\0';
		m_typeBufferLen = 0;	/* 名前入力に使うのでバッファをクリア */
		
		/* 通過した個数を数えておく */
		m_countPass = m_countAll - m_countExcellent - m_countGood - m_countFair - m_countPoor;
	}
	if(phaseNum == PHASE_FINISHED){
		CScore score(m_typeBuffer, m_score, m_scoreAccuracy, m_scoreTyping,
			m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass,
			m_countAll, m_comboMax);
		m_rank = m_ranking.update(score);
		if(m_rank >= 0){
			m_ranking.write();
		}
	}
	m_phase = phaseNum;
}

bool CTyping::input(char *typeBuffer, int &typeBufferLen,
		vector<Lyrics>::iterator &lyricsPosition, double time){
	if(strlen(typeBuffer) == 0){
	/* 未確定ローマ字がないのに再帰した */
		return true;
	}
	vector<Lyrics>::iterator tmpLyricsPosition = lyricsPosition;
	CTrieNode *trie = &m_trie;
	while(1){
		char buf[2];
		buf[0] = (*tmpLyricsPosition).ch;
		tmpLyricsPosition++;
//printf("%c",buf[0]);
		buf[1] = '\0';
//if(trie!=NULL)//debug
		trie = trie->find(buf);	/* 日本語1バイト進む */
		if(trie == NULL){
//printf("//");
			return false;
		}
//trie->chk();
		for(vector<CConvertData>::iterator i = trie->m_data.begin();
				i != trie->m_data.end(); i++){	/* ローマ字候補を探索 */
//printf("[%s]",(*i).m_str);
			if((*i).match_front(typeBuffer)){
				int tmpLen = typeBufferLen - (*i).m_len;
				/* 今回変換される部分の次の部分からに対して実際に打たれている部分の長さ。 */
				/* これは負とかになりうる。 */
				if(tmpLen <= 0){	/* 今回に必要ならキーを足して一度に打てる範囲はここまで */
				/* これは、{んば……,nb……}でnだけ打った状況なども含まれる。 */
//printf("[%f,%f -%f]\n",time,(*(tmpLyricsPosition-1)).timeJust,(*(tmpLyricsPosition-1)).timeUntil);
					if(time >= (*(tmpLyricsPosition-1)).timeUntil){
						/* その最後の文字が時間切れしていてはいけない */
						continue;
					}
				}
				char tmpTypeBuffer[TYPE_BUFFER_LEN + 1];
				strcpy(tmpTypeBuffer, (*i).m_str + (*i).m_len);	/* 未確定ローマ字になる予定の部分 */
				if(input(tmpTypeBuffer, tmpLen, tmpLyricsPosition, time)){
				/* 再帰の結果打てることが分かったとき */
					if(typeBufferLen >= 1 && (*lyricsPosition).isScoringTarget){
					/* 新しい音節の打ち始め(=得点対象になっている) */
						scoreAccuracy(time, lyricsPosition);
						/* ちょうどのタイミングとのずれから計算した得点を与える。 */
					}
					
					/* 「っ」の処理、全ての3バイト以上への変換に効果があるので注意 */
					if(typeBufferLen >= 2 && lyricsPosition + 2 < tmpLyricsPosition &&
							(*(lyricsPosition + 2)).isScoringTarget){
					/* 2文字打って、2バイト先(日本語の1文字先)が今回一度に打てる範囲で、さらに採点対象である */
					/* つまり、「っか」における"kk"の時点で「か」が採点される。 */
						scoreAccuracy(time, lyricsPosition + 2);
						/* ちょうどのタイミングとのずれから計算した得点を与える。 */
					}
					if((*i).match(typeBuffer)){	/* 完全一致 */
						/* 変換された歌詞の分だけ得点を与える */
						scoreTyping(lyricsPosition, tmpLyricsPosition);
						
						for(vector<Lyrics>::iterator i = lyricsPosition; i != tmpLyricsPosition; i++){
							(*i).isTyped = true;	/* 打った歌詞の範囲を記録 */
						}
						lyricsPosition = tmpLyricsPosition;	/* 新しい歌詞の位置に */
						strcpy(typeBuffer, tmpTypeBuffer);
						typeBufferLen = tmpLen;
					}
//printf("!!\n");
					return true;
				}else{
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
		if(!isJapanese2){
			m_scoreTyping += SCORE_TYPING;
		}
		isJapanese2 = false;
		if((*i).isJapanese1()){	/* 日本語の1バイト目なら */
			isJapanese2 = true;
		}
	}
	m_score = m_scoreTyping + m_scoreAccuracy;
}

void CTyping::scoreAccuracy(double time, vector<Lyrics>::iterator lyricsPosition){
	double timeDiff = time - (*lyricsPosition).timeJust;
	int scoreCombo = SCORE_COMBO * m_combo;	/* コンボ数を増やす前にコンボ数ボーナスを計算 */
	if(scoreCombo > SCORE_COMBO_MAX){
		scoreCombo = SCORE_COMBO;
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
	if(m_combo >= 10){	/* コンボが10を超えたらコンボ数を表示 */
		sprintf(buf, "%s %d", strAccuracy, m_combo);
	}else{
		sprintf(buf, "%s", strAccuracy);
	}
	setText(buf, color);
	
	m_scoreAccuracy += score;	/* 得点加算 */
	m_score = m_scoreTyping + m_scoreAccuracy;
	(*lyricsPosition).isScoringTarget = false;	/* 2回以上得点を与えられることはない */
}

#define Y_INFO 10
#define Y_INFO2 40
#define Y_LYRICS_KANJI 90
#define Y_LYRICS_KANJI_NEXT 130
#define Y_ACCURACY 200
#define Y_CIRCLE 290
#define R_CIRCLE 30
#define Y_LYRICS (Y_CIRCLE - 5)
#define Y_LYRICS_BIG 340
#define Y_BUFFER 390

void CTyping::draw(){
	if(m_phase >= PHASE_RESULT){	/* スコア表示以降 */
		drawResult();
		return;
	}
	double time;
	if(m_phase == PHASE_NOT_STARTED){
		SetFontSize(36);
		DrawString(50, 400, "なにかキーを押してスタート", GetColor(255, 255, 255));
		//return;
		time = 0.0;	/* 始まる前は0秒で止まっている */
	}else{
		time = getTime();	/* 開始時刻からの経過秒を取得 */
	}
	DrawFormatStringToHandle(10, Y_INFO, GetColor(255, 255, 255), m_fontHandleNormal,
		"%10d 点", m_score);
	DrawFormatStringToHandle(10, Y_INFO2, GetColor(255, 255, 255), m_fontHandleNormal,
		"%10d コンボ", m_combo);
	/*
	DrawFormatStringToHandle(10, Y_INFO, GetColor(255, 255, 255), m_fontHandleNormal,
		"得点: %10d, %10d コンボ", m_score, m_combo);
	DrawFormatStringToHandle(10, Y_INFO2, GetColor(255, 255, 255), m_fontHandleNormal,
		"( %d + %d ), ( %d / %d / %d / %d), %d",
		m_scoreAccuracy, m_scoreTyping, m_countExcellent, m_countGood, m_countFair, m_countPoor, m_comboMax);
	*/
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
	
	/* 表示する歌詞を出力 */
	while(time >= (*m_lyricsKanjiPosition).timeEnd){	/* 表示終了しているなら次に進む */
		m_lyricsKanjiPosition++;
	}
	if(time >= (*m_lyricsKanjiPosition).timeBegin){	/* 表示開始しているなら */
		DrawFormatStringToHandle(10, Y_LYRICS_KANJI,
			GetColor(255, 255, 255), m_fontHandleNormal,
			"%s", (*m_lyricsKanjiPosition).str.c_str());	/* 出力 */
		DrawFormatStringToHandle(10, Y_LYRICS_KANJI_NEXT,
			GetColor(255, 255, 255), m_fontHandleNormal,
			"Next: %s", (*(m_lyricsKanjiPosition + 1)).str.c_str());	/* Nextを出力 */
	}else{
		DrawFormatStringToHandle(10, Y_LYRICS_KANJI_NEXT,
			GetColor(255, 255, 255), m_fontHandleNormal,
			"Next: %s", (*m_lyricsKanjiPosition).str.c_str());	/* Nextを出力 */
	}
	
	for(vector<Lyrics>::iterator i = m_lyrics.end() - 1; i != m_lyrics.begin() - 1; i--){
		if(!(*i).isBlockStart){	/* 音節の最初でないなら無視 */
			continue;
		}
		if((*i).ch == '\n'){	/* 最後 */
			continue;
		}
		bool isCurrent = false;	/* 今打っているものかどうか */
		{
			vector<Lyrics>::iterator j = i;
			do{	/* ブロック内の未タイプの文字をbufに格納 */
				if(j == m_lyricsPosition){	/* 現在打っている場所を含むなら */
					isCurrent = true;
				}
				j++;
			}while(!(*j).isBlockStart);
		}
		
		double timeDiff = time - (*i).timeJust;	/* ちょうど打つ位置になってから何秒後か */
		int pos = (int)(100 - timeDiff * 250.0);
		if((!isCurrent) && (pos < -32 || pos > 640+32)){	/* 画面外 */
			continue;
		}
		/* isCurrentなら進ませるのは、現在打っている文字を表示するため */
		
		if((*i).ch == ' '){	/* 歌詞の切れ目 */
			if(timeDiff < 0.0){	/* 切れ目は判定位置に来るより前のときだけ表示 */
				DrawCircle(pos, Y_CIRCLE, R_CIRCLE, GetColor(128, 128, 128), false);	/* 灰色の円を流す */
			}
		}
		char buf[256];
		int len = 0;
		{
		vector<Lyrics>::iterator j = i;
		do{	/* ブロック内の未タイプの文字をbufに格納 */
			if(j == m_lyricsPosition){	/* 現在打っている場所を含むなら */
				isCurrent = true;
			}
			if(!(*j).isTyped){
				buf[len++] = (*j).ch;
			}
			j++;
		}while(!(*j).isBlockStart);
		}
		buf[len] = '\0';
		if(isCurrent){
			int strWidth = GetDrawStringWidthToHandle(buf, len, m_fontHandleBig);
			DrawStringToHandle(100 - strWidth / 2, Y_LYRICS_BIG, buf,
				GetColor(255, 255, 255), m_fontHandleBig);	/* 打っている文字は下にも出す */
		}
		if(len == 0){	/* すべてタイプされていた */
			continue;
		}
		int Color;
		if((*i).isScoringTarget){	/* まだタイミング点をもらってない */
			Color = GetColor(255, 0, 0);
		}else{
			Color = GetColor(0, 0, 255);
		}
		DrawCircle(pos, Y_CIRCLE, R_CIRCLE - 1, Color, TRUE);	/* 流れる円 */
		DrawCircle(pos, Y_CIRCLE, R_CIRCLE, GetColor(0, 0, 0), FALSE);	/* 流れる円の輪郭 */
		int strWidth = GetDrawStringWidthToHandle(buf, len, m_fontHandleNormal);
		DrawStringToHandle(pos - strWidth / 2, Y_LYRICS, buf,
			GetColor(255, 255, 255), m_fontHandleNormal);	/* 流れる円に書かれる文字 */
	}
	DrawCircle(100, Y_CIRCLE, R_CIRCLE, GetColor(255, 255, 255), FALSE);	/* 判定位置の円 */
	
	{	/* メッセージ表示 */
		double timeDiff = time - m_textTime;
		if(timeDiff <= 0.5){	/* メッセージ（優良可不可）が0.5秒間表示される */
			//int strLen = strlen(m_text);
			//int strWidth = GetDrawStringWidthToHandle(m_text, strLen, m_fontHandleBig);
			int strX;
			//strX = 100 - strWidth / 2;
			strX = 100 - R_CIRCLE;	/* 円の左端 */
			int strY;	/* 表示されるY座標 */
			strY = Y_ACCURACY;
			if(timeDiff < 0.05){	/* 0.05秒後には定位置 */
				strY += ((0.05 - timeDiff) / 0.05) * 10;	/* 10だけ下から出てくる */
			}
			DrawStringToHandle(strX, strY, m_text,
				m_textColor, m_fontHandleBig);
		}
	}
	
	/* タイプした文字を表示 */
	m_typeBuffer[m_typeBufferLen] = '\0';
	int strLen = strlen(m_typeBuffer);
	int strWidth = GetDrawStringWidthToHandle(m_typeBuffer, strLen, m_fontHandleBig);
	DrawStringToHandle(100 - strWidth / 2, Y_BUFFER, m_typeBuffer,
		GetColor(255, 255, 255), m_fontHandleBig);
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
		DrawFormatStringToHandle(30, 150, GetColor(255, 255, 255), m_fontHandleBig,
			"最大 %d コンボ", m_comboMax);
	}
	if(time >= 3.3){
		DrawFormatStringToHandle(30, 210, GetColor(255, 255, 255), m_fontHandleBig,
			"得点 :");
	}
	if(time >= 3.8){
		DrawFormatStringToHandle(320, 210, GetColor(255, 255, 255), m_fontHandleNormal,
			"質 : %10d 点", m_scoreAccuracy);
	}
	if(time >= 4.0){
		DrawFormatStringToHandle(320, 235, GetColor(255, 255, 255), m_fontHandleNormal,
			"量 : %10d 点", m_scoreTyping);
	}
	if(time >= 4.5){
		DrawFormatStringToHandle(30, 275, GetColor(255, 255, 255), m_fontHandleBig,
			"総得点 : %10d 点",m_score);
	}
	if(time >= 5.0){
		DrawFormatStringToHandle(30, 375, GetColor(255, 255, 255), m_fontHandleNormal,
			"名前を入力してください :");
	}
	DrawFormatStringToHandle(60, 400, GetColor(255, 255, 255), m_fontHandleBig,
		"%s", m_typeBuffer);
	if(m_phase == PHASE_FINISHED){
		if(m_rank >= 0){
			int strWidth;
			strWidth = GetDrawFormatStringWidthToHandle( m_fontHandleBig,
				"%d 位にランクインしました", m_rank + 1);
			int boxWidth = strWidth + 100;
			if(boxWidth > 640){
				boxWidth = 640;
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

void main2(CTyping &typing){
	while(1){
		if(ProcessMessage() == -1){
			return;
		}
		if(!typing.idle()){
			break;
		}
		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();
		typing.draw();
		ScreenFlip();
		char ch = GetInputChar(TRUE);
		if(ch == CTRL_CODE_ESC){	/* Escで中断 */
			return;
		}
		//if(ch != 0 && ch >= CTRL_CODE_CMP){
		if(ch != 0){	/* バッファに何もなければ関数を呼ばない */
			typing.keyboard(ch);
		}
	}
}


void main1(){
	vector<string> fileNameArray;
	{
		FILE *fp;
		fp = fopen("UTyping_list.txt", "r");
		if(fp == NULL){
			throw __LINE__;
		}
		char buf[256];
		while(fscanf(fp, "%s", buf) >= 1){
			fileNameArray.push_back(string(buf));
		}
		fclose(fp);
	}
	vector<string>::iterator fileNameArrayItr = fileNameArray.begin();
	
	while(1){
		if(ProcessMessage() == -1){
			return;
		}
		char ch = GetInputChar(TRUE);
		switch(ch){
		case CTRL_CODE_ESC:	/* 終了 */
			goto L1;
		case CTRL_CODE_LEFT:	/* 1つ戻る。最初から1つ戻ると最後に。 */
			if(fileNameArrayItr == fileNameArray.begin()){
				fileNameArrayItr = fileNameArray.end();
			}
			fileNameArrayItr--;
			break;
		case CTRL_CODE_RIGHT:	/* 1つ進む。1つ進んで最後を超えると最初に。 */
			fileNameArrayItr++;
			if(fileNameArrayItr == fileNameArray.end()){
				fileNameArrayItr = fileNameArray.begin();
			}
			break;
		case CTRL_CODE_CR:	/* ゲーム開始 */
			{
				CTyping typing;
				typing.load((*fileNameArrayItr).c_str());	/* ファイルを読み込む */
				typing.loadRanking(((*fileNameArrayItr) + "-rank.dat").c_str());
				main2(typing);
			}
			break;
		default:
			break;
		}
		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();
		
		/* 現在選択されている譜面ファイル名を書く */
		SetFontSize(24);

		DrawString(50, 200, (*fileNameArrayItr).c_str(), GetColor(255, 255, 255));
		//(*fileNameArrayItr).c_str()
		
		ScreenFlip();
	}
L1:
	return;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow){
	ChangeWindowMode(TRUE);
	SetMainWindowText("UTyping");
	if(DxLib_Init() == -1){
		return -1;
	}
	try{
		main1();
	}catch(int n){
		FILE *fp;
		fp = fopen("エラーログ.txt","a");
		fprintf(fp, "%d 行目でエラーが発生しました。\n", n);
		fclose(fp);
	}
	DxLib_End();
	return 0;
}
