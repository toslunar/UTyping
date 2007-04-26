#include "DxLib.h"

#include <stdio.h>
#include <string.h>
#include <vector>

using namespace std;

#define INFTY 1000000000

#define SEC_EXCELLENT 0.02
#define SEC_GOOD 0.05
#define SEC_FAIR 0.1
#define SEC_POOR 0.5

#define SCORE_EXCELLENT 1500
#define SCORE_GOOD 1000
#define SCORE_FAIR 500
#define SCORE_POOR 0

#define SCORE_COMBO 10
#define SCORE_COMBO_MAX 1000

#define SCORE_TYPING 200
/* タイピングで1バイト確定させるごとに */

/* ============================================================ */

class CConvertData{
public:
	CConvertData(const char *buf1, const char *buf3);
	bool match_front(const char *str);
	bool match(const char *str);
public:
	/* まあ、もともとstructにするつもりだったぐらいだし */
	char m_str[5];	/* 日本語 → ローマ字, 全角 → 半角 の変換先は 4 文字以内 */
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
	char ch;	/* 変換された文字（あいう、。～abc123+-など）の1バイト */
	double timeJust,timeUntil;	/* ちょうどの時間、その文字以前だけを打つときに、最も遅い時間 */
	bool isBlockStart;	/* 1音節（タイミング判定をするかたまり）の最初 */
	bool isTyped;	/* すでに打たれたか */
	bool isScoringTarget;	/* 現在タイミング判定をする対象であるか */
};

/* ============================================================ */

class CTyping{
public:
	CTyping();
	~CTyping();
	// void loadDictionary(const char *fileName);
	/* ローマ字辞書は最初に読む */
	void load(const char *fileName);
	void keyboard(char ch);
	bool idle();
	void draw();
private:
	void setText(const char *str, int color);
	void setTime();
	double getTime();
	bool input(char *typeBuffer, int &typeBufferLen,
		vector<Lyrics>::iterator &lyricsPosition, double time);
	int scoreTyping(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd);
	int scoreAccuracy(double timeDiff);
private:
	CTrieNode m_trie;
	
	vector<Lyrics> m_lyrics;
	vector<Lyrics>::iterator m_lyricsPosition;
	char m_typeBuffer[5];
	int m_typeBufferLen;
	
	bool m_started;
	bool m_finished;
	
	int m_timeStart;
	
	int m_score;
	int m_combo;
	
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
	if(fgets(m_musicFileName, sizeof(m_musicFileName), fp) == NULL){	/* 再生する音楽ファイル名 */
		throw __LINE__;
	}
	int len = strlen(m_musicFileName);
	if(m_musicFileName[len - 1] == '\n'){
		m_musicFileName[len - 1] = '\0';
	}
	
	/* 歌詞を読み込む */
	m_lyrics.clear();
	
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
	
	char buf[256], bufLast[256];
	strcpy(bufLast,"");
	double time, timeLast;
	bool flag = true;
	while(flag){
		if(fgets(buf,sizeof(buf),fp) == NULL){
			flag = false;
			time = INFTY;
		}else{
			int n = sscanf(buf, "%lf%s", &time, buf);
			if(n <= 0){	/* いらない行 */
				continue;
			}
			if(n == 1){	/* 歌詞の切れ目 */
				strcpy(buf, " ");
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
			
			ly.isBlockStart = false;	/* 最初の文字のみを true にするため */
			ly.isScoringTarget = false;
		}
//printf("]\n");
		timeLast = time;
		strcpy(bufLast, buf);
	}
//printf("\n");
	/* 番兵として最後に' ',INFTYを追加しておく */
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
	
	// /* ' 'で区切った歌詞のNextとかも表示したいので、もうひとつ追加しておく */
	// m_lyrics.push_back(ly);
	
	/* 現在打っている歌詞の位置 */
	m_lyricsPosition = m_lyrics.begin();
	
	/* キー入力を初期化 */
	m_typeBufferLen = 0;
	
	/* 何かキーを押すまでは始まらない */
	m_started = false;
	/* 当然終わってもいない */
	m_finished = false;
	
	/* 点数初期化 */
	m_score = 0;
	m_combo = 0;
	
	strcpy(m_text, "");
}

void CTyping::keyboard(char ch){
	if(!m_started){
		m_started = true;
		PlayMusic(m_musicFileName, DX_PLAYTYPE_BACK);	/* 音楽を流し始める */
		setTime();	/* 始まった時刻を覚える */
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
	char tmpBuffer[5];
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
	if(!m_started){	/* 開始していなければ特にすることは無い */
		return true;
	}
	if(CheckMusic() <= 0){	/* 音楽の再生が終了している */
		m_finished = true;
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
				/* 実際に打たれている部分の長さ。これは負とかになりうる。 */
				char tmpBuf[5];
				strcpy(tmpBuf, (*i).m_str);
				char *tmpTypeBuffer = tmpBuf + (*i).m_len;	/* 未確定ローマ字になるべき部分 */
				int tmpLen = typeBufferLen - (*i).m_len;
				if(tmpLen <= 0){	/* 今回に必要ならキーを足して一度に打てる範囲はここまで */
				/* これは、{んば……,nb……}でnだけ打った状況なども含まれる。 */
//printf("[%f,%f -%f]\n",time,(*(tmpLyricsPosition-1)).timeJust,(*(tmpLyricsPosition-1)).timeUntil);
					if(time >= (*(tmpLyricsPosition-1)).timeUntil){
						/* その最後の文字が時間切れしていてはいけない */
						continue;
					}
				}
				if(input(tmpTypeBuffer, tmpLen, tmpLyricsPosition, time)){
				/* 再帰の結果打てることが分かったとき */
					if(typeBufferLen >= 1 && (*lyricsPosition).isScoringTarget){
					/* 新しい音節の打ち始め(=得点対象になっている) */
						m_score += scoreAccuracy(time - (*lyricsPosition).timeJust);
						/* ちょうどのタイミングとのずれから計算した得点を与える。 */
						(*lyricsPosition).isScoringTarget = false;	/* 2回以上得点を与えられることはない */
					}
					/* 「っ」の処理をあとでなんか考える！ */
					if(typeBufferLen >= 2 && lyricsPosition + 2 < tmpLyricsPosition &&
							(*(lyricsPosition + 2)).isScoringTarget){
					/* 2文字打って、2バイト先(日本語の1文字先)が今回一度に打てる範囲で、さらに採点対象である */
					/* つまり、「っか」における"kk"の時点で「か」が採点される。 */
						m_score += scoreAccuracy(time - (*(lyricsPosition + 2)).timeJust);
						/* ちょうどのタイミングとのずれから計算した得点を与える。 */
						(*(lyricsPosition + 2)).isScoringTarget = false;	/* 2回以上得点を与えられることはない */
					}
					if((*i).match(typeBuffer)){	/* 完全一致 */
						/* 変換された歌詞の分だけ得点を与える */
						m_score += scoreTyping(lyricsPosition, tmpLyricsPosition);
						
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

int CTyping::scoreTyping(vector<Lyrics>::iterator lyBegin, vector<Lyrics>::iterator lyEnd){
/* [lyBegin, lyEnd)を打ったときの得点 */
	return SCORE_TYPING * distance(lyBegin, lyEnd);
}

int CTyping::scoreAccuracy(double timeDiff){
	m_combo++;	/* とりあえずコンボ数を 1 増やす */
	int scoreCombo = SCORE_COMBO * m_combo;
	if(scoreCombo > SCORE_COMBO_MAX){
		scoreCombo = SCORE_COMBO;
	}
	if(timeDiff < 0.0){
		timeDiff = -timeDiff;
	}
	int score;
	char buf[256];
	char *strAccuracy;	/* 判定の文字（優良可不可） */
	int color;	/* 表示色 */
	if(timeDiff < SEC_EXCELLENT){
		score = SCORE_EXCELLENT + scoreCombo;
		strAccuracy = "優";
		color = GetColor(255, 255, 0);
	}else if(timeDiff < SEC_GOOD){
		score = SCORE_GOOD + scoreCombo;
		strAccuracy = "良";
		color = GetColor(0, 255, 0);
	}else if(timeDiff < SEC_FAIR){
		score = SCORE_FAIR + scoreCombo;
		strAccuracy = "可";
		color = GetColor(0, 128, 255);
	}else{
		score = SCORE_POOR;
		m_combo = 0;	/* コンボが途切れていた */
		strAccuracy = "不可";
		color = GetColor(128, 128, 128);
	}
	if(m_combo >= 10){	/* コンボが10を超えたらコンボ数を表示 */
		sprintf(buf, "%s %d", strAccuracy, m_combo);
	}else{
		sprintf(buf, "%s", strAccuracy);
	}
	setText(buf, color);
	return score;
}

void CTyping::draw(){
	double time;
	if(m_finished){	/* 終わっている */
		DrawFormatStringToHandle(10, 10, GetColor(255, 255, 255), m_fontHandleBig,
			"得点: %10d", m_score);
		return;
	}
	if(!m_started){
		SetFontSize(36);
		DrawString(50, 400, "なにかキーを押してスタート", GetColor(255, 255, 255));
		//return;
		time = 0.0;	/* 始まる前は0秒で止まっている */
	}else{
		time = (GetNowCount() - m_timeStart) / 1000.0;	/* 開始時刻からの経過秒を取得 */
	}
	DrawFormatStringToHandle(10, 10, GetColor(255, 255, 255), m_fontHandleNormal,
		"得点: %10d, %10d コンボ", m_score, m_combo);
	//printf("%7.3f ", time);
	
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
			DrawCircle(pos, 200, 30, GetColor(128, 128, 128), false);	/* 灰色の円を流す */
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
			DrawStringToHandle(100 - strWidth / 2, 250, buf,
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
		DrawCircle(pos, 200, 30, Color, TRUE);	/* 流れる円 */
		DrawCircle(pos, 200, 30, GetColor(0, 0, 0), FALSE);	/* 流れる円の輪郭 */
		int strWidth = GetDrawStringWidthToHandle(buf, len, m_fontHandleNormal);
		DrawStringToHandle(pos - strWidth / 2, 195, buf,
			GetColor(255, 255, 255), m_fontHandleNormal);	/* 流れる円に書かれる文字 */
	}
	DrawCircle(100, 200, 30, GetColor(255, 255, 255), FALSE);	/* 判定位置の円 */
	
	{	/* メッセージ表示 */
		double timeDiff = time - m_textTime;
		if(timeDiff <= 0.5){	/* メッセージ（優良可不可）が0.5秒間表示される */
			int strLen = strlen(m_text);
			//int strWidth = GetDrawStringWidthToHandle(m_text, strLen, m_fontHandleBig);
			int strX;
			//strX = 100 - strWidth / 2;
			strX = 100 - 30;	/* 円の左端 */
			int strY;	/* 表示されるY座標 */
			strY = 120;
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
	DrawStringToHandle(100 - strWidth / 2, 310, m_typeBuffer,
		GetColor(255, 255, 255), m_fontHandleBig);
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
		fp = fopen("list.txt", "r");
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
			return;
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
