#ifndef __UTYPING_SCORE
#define __UTYPING_SCORE

#include "utchallenge.h"
#include "ututil.h"

#define RANKING_FILE_VERSION 2

#define RANKING_LEN 20
/* ランキングに記録する順位が何位までか */

#define RANKING_DRAW_LEN 5
/* 何位ずつランキングを表示するか */
/* RANKING_LEN の約数だと表示に無駄がない */

#define H_RANKING1 60
/* 1位のみを描画するときの幅 */

#define H_RANKING 252
/* RANKING_LEN 位分を描画するときの幅 */

class CScore{
public:
	CScore();
	CScore(const char *n,int s,int sa,int st,int ce,int cg,int cf,int cp,int cx,
		int ca,int cm,const CChallenge &ch);
	void init();
	void read(FILE *fp, int version);
	void write(FILE *fp)const;
	
	void draw(int x, int y, int n, int fontHandle)const;
	
	bool nameCmp(CScore &score)const;
	bool dateCmp(CScore &score)const;
	bool challengeCmp(CScore &score)const;
	/*
	bool operator ==(CScore &score)const{
		return m_score == score.m_score;
	}
	bool operator !=(CScore &score)const{
		return m_score != score.m_score;
	}
	bool operator <(CScore &score)const{
		return m_score < score.m_score;
	}
	bool operator <=(CScore &score)const{
		return m_score <= score.m_score;
	}*/
	bool operator >(CScore &score)const{
		return m_score > score.m_score;
	}/*
	bool operator >=(CScore &score)const{
		return m_score >= score.m_score;
	}*/
private:
	char m_name[NAME_LEN + 1];
	int m_score;
	int m_scoreAccuracy, m_scoreTyping;
	int m_count[5];
		/* Excellent, Good, Fair, Poor, Pass */
	int m_countAll;
	int m_comboMax;
	CChallenge m_challenge;
	int m_date;
};

CScore::CScore(){
	init();
}

CScore::CScore(const char *n,int s,int sa,int st,int ce,int cg,int cf,int cp,int cx,
		int ca,int cm,const CChallenge &ch){
	strcpy(m_name, n);
	m_score = s;
	m_scoreAccuracy = sa;
	m_scoreTyping = st;
	m_count[0] = ce;
	m_count[1] = cg;
	m_count[2] = cf;
	m_count[3] = cp;
	m_count[4] = cx;
	m_countAll = ca;
	m_comboMax = cm;
	m_challenge = ch;
	m_date = getDateInt();
}

void CScore::init(){
	strcpy(m_name, "_");
	m_score = 0;
	m_scoreAccuracy = 0;
	m_scoreTyping = 0;
	for(int i=0; i<5; i++){
		m_count[i] = 0;
	}
	m_countAll = 0;
	m_comboMax = 0;
	m_challenge.invalidate();
	m_date = 0;
}

void CScore::read(FILE *fp, int version){
	init();
	if(version >= 1){
		readChars(m_name, NAME_LEN, fp);
	}else{
		readChars(m_name, 8, fp);
	}
	readInt(m_score, fp);
	readInt(m_scoreAccuracy, fp);
	readInt(m_scoreTyping, fp);
	for(int i=0; i<5; i++){
		readInt(m_count[i], fp);
	}
	readInt(m_countAll, fp);
	readInt(m_comboMax, fp);
	if(version >= 1){
		m_challenge.read(fp);
	}
	if(version >= 2){
		readInt(m_date, fp);
	}
/*
	fscanf(fp, "%s%d%d%d%d%d%d%d%d%d%d", m_name, &m_score, &m_scoreAccuracy, &m_scoreTyping,
		&m_countExcellent, &m_countGood, &m_countFair, &m_countPoor, &m_countPass,
		&m_countAll, &m_comboMax);
*/
}

void CScore::write(FILE *fp)const{
/*
	fprintf(fp, "%s %d %d %d %d %d %d %d %d %d %d\n", m_name, m_score, m_scoreAccuracy, m_scoreTyping,
		m_countExcellent, m_countGood, m_countFair, m_countPoor, m_countPass,
		m_countAll, m_comboMax);
*/
	writeChars(m_name, NAME_LEN, fp);
	writeInt(m_score, fp);
	writeInt(m_scoreAccuracy, fp);
	writeInt(m_scoreTyping, fp);
	for(int i=0; i<5; i++){
		writeInt(m_count[i], fp);
	}
	writeInt(m_countAll, fp);
	writeInt(m_comboMax, fp);
	m_challenge.write(fp);
	writeInt(m_date, fp);
}

void CScore::draw(int x, int y, int n, int fontHandle)const{
	char buf[256];
	int color;
	/*
	switch(n){
	case 1:
		color = GetColor(230, 180, 34);
		break;
	case 2:
		color = GetColor(175, 175, 176);
		break;
	case 3:
		color = GetColor(149, 65, 28);
		break;
	default:
		color = GetColor(128, 128, 128);
		break;
	}
	*/
	color = GetColor(255, 255, 255);
	{
		char buf2[64];
		getOrdinal(buf2, n);	/* 順位を1st,2nd,3rd,...で表示 */
		sprintf(buf, "%s: %-16s %8d 点( %7d + %7d ),", buf2,
			m_name, m_score, m_scoreAccuracy, m_scoreTyping);
		DrawStringToHandle(40 + x, y + 6, buf, color, fontHandle);
	}
	{
		getDateStrFromInt(buf, m_date);	/* 日付を表示 */
		int width = GetDrawStringWidthToHandle(buf, strlen(buf), fontHandle);
		DrawStringToHandle((W_WINDOW - 40) - width + x, y + 6, buf, color, fontHandle);
	}
	{
		char buf2[64];
		m_challenge.getStr(buf2);
		if(m_comboMax >= 0){
			sprintf(buf, "%s, 最大 %4d コンボ, (%4d/%4d/%4d/%4d/%4d)", buf2, m_comboMax,
				m_count[0], m_count[1], m_count[2], m_count[3], m_count[4]);
		}else{	/* m_comboMax == -1 でフルコンボ扱い */
			sprintf(buf, "%s,      フル コンボ, (%4d/%4d/%4d/%4d/%4d)", buf2,
				m_count[0], m_count[1], m_count[2], m_count[3], m_count[4]);
		}
		int width = GetDrawStringWidthToHandle(buf, strlen(buf), fontHandle);
		DrawStringToHandle((W_WINDOW - 40) - width + x, y + (48-6-16), buf, color, fontHandle);
	}
}

bool CScore::nameCmp(CScore &score)const{
	return strcmp(m_name, score.m_name) == 0;
}

bool CScore::dateCmp(CScore &score)const{
	return m_date == score.m_date;
}

bool CScore::challengeCmp(CScore &score)const{
	return m_challenge == score.m_challenge;
}


/* ============================================================ */

class CRanking{
public:
	CRanking();
	~CRanking();
	int update(const CScore &score, bool f_checkDate, bool f_checkChallenge);
		/* 日付が異なったら異なるとみなすか、チャレンジが異なったら異なるとみなすか */
	void open(const char *fileName);
	void close();
	void read();
	void write();
	
	void draw(int x, int y, int rankBegin, int rankLen, int fontHandle);
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
int CRanking::update(const CScore &score, bool f_checkDate, bool f_checkChallenge){
	int lastRank = RANKING_LEN - 1;	/* すでに入っている順位（のうち最後のもの） */
	/* 入っていない場合は、RANKING_LEN - 1が都合が良い */
	/* データをずらすときに、ランク外だったとしても、最後のランクだったとしても同じになるから。 */
	for(int i = RANKING_LEN - 1; i >= 0; i--){
		if(score.nameCmp(m_score[i])){
			if(f_checkDate && !score.dateCmp(m_score[i])){
				continue;
			}
			if(f_checkChallenge && !score.challengeCmp(m_score[i])){
				continue;
			}
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
	int version;
	bool flag = readInt(version, m_fp);
	if(!flag){	/* ファイルが空 */
		for(int i=0; i<RANKING_LEN; i++){
			m_score[i].init();
		}
		return;
	}
	if((version & 0xff) > ' '){	/* 最初のバージョン対策 */
		version = 0;
		rewind(m_fp);
	}
	if(version > RANKING_FILE_VERSION || version < 0){
		throw "新しいバージョンのランキングファイルです。プログラムを更新してください。";
	}
	for(int i=0; i<RANKING_LEN; i++){
		m_score[i].read(m_fp, version);
	}
	rewind(m_fp);
	return;
}

void CRanking::write(){
	rewind(m_fp);
	writeInt(RANKING_FILE_VERSION, m_fp);
	for(int i=0; i<RANKING_LEN; i++){
		m_score[i].write(m_fp);
	}
	rewind(m_fp);
}

/* rankBeginからrankLen位分表示 */
void CRanking::draw(int x, int y, int rankBegin, int rankLen, int fontHandle){
	for(int i = 0; i < rankLen; i++){
		int j = rankBegin + i;	/* 書く順位 */
		if(j >= RANKING_LEN){	/* 記録されているランキングの外 */
			break;
		}
		m_score[j].draw(x, y + 48 * i, j + 1, fontHandle);	/* 実際は、0番目は1位 etc. */
	}
}

#endif
