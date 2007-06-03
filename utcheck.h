#ifndef __UTYPING_CHECK
#define __UTYPING_CHECK

#include "DxLib.h"
#include <vector>
using namespace std;

#define CCHECK_N_DATA (480/8)

#define CCHECK_WIDTH 250
#define CCHECK_SCALE ((1.0/60)/CCHECK_WIDTH)

class CCheck{
public:
	CCheck();
	void begin(int color);
	void rap(int color);
	void end();
	void draw();
private:
	void drawBar(int i, int y0, int y1);
private:
	struct CheckData{
		double time;
		int color;
	};
	vector<CheckData> m_data[CCHECK_N_DATA];
	int m_counter;
	int m_color;	/* ëOÇÃåƒÇ—èoÇµÇ≈ê›íËÇµÇΩêF */
	LARGE_INTEGER m_begin;
	LARGE_INTEGER m_freq;
};

CCheck g_check;	/* debug */

CCheck::CCheck(){
	m_counter = 0;
	for(int i=0; i<CCHECK_N_DATA; i++){
		m_data[i].clear();
	}
	QueryPerformanceFrequency(&m_freq);
}

void CCheck::begin(int color){
	QueryPerformanceCounter(&m_begin);
	m_data[m_counter].clear();
	m_color = color;
}

void CCheck::rap(int color){
	LARGE_INTEGER m_end;
	QueryPerformanceCounter(&m_end);
	CheckData cd;
	cd.time = (double)(m_end.QuadPart - m_begin.QuadPart) / m_freq.QuadPart;
	cd.color = m_color;
	m_data[m_counter].push_back(cd);
	m_color = color;
}

void CCheck::end(){
	rap(0);
	m_counter++;
	if(m_counter == CCHECK_N_DATA){
		m_counter = 0;
	}
}

void CCheck::draw(){
	for(int i=0; i<CCHECK_N_DATA; i++){
		//drawBar((i + m_counter) % CCHECK_N_DATA, i * 8, (i + 1) * 8);
		drawBar(i, i * 8, (i + 1) * 8);
	}
	DrawLine(CCHECK_WIDTH, 0, CCHECK_WIDTH, 480, GetColor(32, 32, 32));
	DrawLine(CCHECK_WIDTH * 2, 0, CCHECK_WIDTH * 2, 480, GetColor(32, 32, 32));
}

void CCheck::drawBar(int i, int y0, int y1){
	int last = 0;
	int len = m_data[i].size();
	for(int j=0; j<len; j++){
		int tmp = (int)(m_data[i][j].time / CCHECK_SCALE);
		DrawBox(last, y0, tmp, y1, m_data[i][j].color, TRUE);
		last = tmp;
	}
}

#endif
