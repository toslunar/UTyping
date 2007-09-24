#ifndef __UTYPING_CHALLENGE
#define __UTYPING_CHALLENGE

#include "ututil.h"

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

class CChallenge{
public:
	CChallenge();
	
	void invalidate();
	
	bool isEasy();
	
	void reset();
	void set(int pos);
	void reset(int pos);
	void flip(int pos);
	bool test(int pos)const;
	
	void speedUp();
	void speedDown();
	double speed()const;	/* 流れる速さの標準との比を返す */
	
	void keyUp();
	void keyDown();
	int key()const;
	double frequencyRate()const;	/* 周波数が何倍されるか */
	
	void getStr(char *buf)const;

	void read(FILE *fp);
	void write(FILE *fp)const;
	
	bool operator ==(CChallenge &challenge)const{
		return m_isValid && challenge.m_isValid &&
			(m_flags == challenge.m_flags && m_key == challenge.m_key);
		/* どちらかも有効で、speed以外の項目が等しいとき */
	}
private:
	bool m_isValid;
	bitset<CHALLENGE_NUM> m_flags;
	int m_speed;	/* 円が流れる速さの標準との比の10倍(intでもつため) */
	int m_key;
};

CChallenge::CChallenge(){
	reset();
}

void CChallenge::invalidate(){
	m_isValid = false;
}

bool CChallenge::isEasy(){
	if(m_key < 0){	/* 再生速度を遅くした */
		return true;
	}
	return false;
}

void CChallenge::reset(){
	m_isValid = true;
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

bool CChallenge::test(int pos)const{
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

double CChallenge::speed()const{
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

int CChallenge::key()const{
	return m_key;
}

double CChallenge::frequencyRate()const{
	return pow(2.0, m_key / 12.0);
}

void CChallenge::getStr(char *buf)const{
	if(!m_isValid){
		strcpy(buf, "----");
		return;
	}
	strcpy(buf, "");
	{
		char tmp[64];
		sprintf(tmp, "x%.1f", speed());
		strcat(buf, tmp);
	}
	if(test(CHALLENGE_SUDDEN)){
		strcat(buf, "S");
	}
	if(test(CHALLENGE_HIDDEN)){
		strcat(buf, "H");
	}
	if(test(CHALLENGE_STEALTH)){
		strcat(buf, "C");
	}
	if(test(CHALLENGE_LYRICS_STEALTH)){
		strcat(buf, "L");
	}
	if(m_key != 0){
		char tmp[64];
		sprintf(tmp, "%+d", m_key);
		strcat(buf, tmp);
	}
	if(test(CHALLENGE_SIN)){
		strcat(buf, "s");
	}
	if(test(CHALLENGE_COS)){
		strcat(buf, "c");
	}
	if(test(CHALLENGE_TAN)){
		strcat(buf, "t");
	}
}

void CChallenge::read(FILE *fp){
	reset();
	for(int i=0; i<CHALLENGE_NUM; i++){
		if(getc(fp)){
			set(i);
		}
	}
	for(int i=CHALLENGE_NUM; i<16; i++){
		getc(fp);
	}
	readInt(m_speed, fp);
	readInt(m_key, fp);
	if(m_speed == 0){	/* 無効なときはすべて0なので、これを見ればOK */
		m_isValid = false;
	}else{
		m_isValid = true;
	}
}

void CChallenge::write(FILE *fp)const{
	if(m_isValid){
		for(int i=0; i<CHALLENGE_NUM; i++){
			putc(test(i)?1:0, fp);
		}
		for(int i=CHALLENGE_NUM; i<16; i++){
			putc(0, fp);
		}
		writeInt(m_speed, fp);
		writeInt(m_key, fp);
	}else{
		for(int i=0; i<24; i++){
			putc(0, fp);
		}
	}
}

#endif
