#pragma once

#include <odbcinst.h>
#include <sqlext.h>

#define MAX_WORKER	300
#define MAX_COOP	300

// HIFIMATCHING 대화 상자

class HIFIMATCHING : public CDialog
{
	DECLARE_DYNAMIC(HIFIMATCHING)

public:
	HIFIMATCHING(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~HIFIMATCHING();

private:
	SQLHANDLE HF_Environment;		// ODBC 기술을 사용하기 위한 환경 정보
	SQLHDBC HF_ODBC;				// ODBC 함수를 사용하기 위한 정보
	char m_connect_flag = 0;		// 서버와 연결 여부를 저장 할 변수 (1: 연결)

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MATCHING_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	// 마지막으로 호출된 사람의 인덱스
	int pIndex = 1;

	struct WORKER
	{
		// 노동자 인덱스
		int index;
		// 노동자 이름
		wchar_t NAME[30];
		// 노동자 근무 타입
		wchar_t TYPE[30];
		// 노동자 근무 유형
		wchar_t STYLE[30];
		// 노동자 근무 시간 (일근 일 때만)
		wchar_t TIME[30];
		// 노동자 상위 직무 선택 항목
		wchar_t TOPJOB[30];
		// 노동자 하위 직무 선택 항목
		wchar_t BOTJOB[30];
		// 노동자 시, 도 선택 항목
		wchar_t SID[30];
		// 노동자 시, 군, 구 선택 항목
		wchar_t SIG[30];
	};

	struct COOP
	{
		// 기업 인덱스
		int index;
		// 기업 이름
		wchar_t NAME[30];
		// 기업 근무 타입
		wchar_t TYPE[30];
		// 기업 근무 유형
		wchar_t STYLE[30];
		// 기업 근무 시간 (일근 일 때만)
		wchar_t TIME[30];
		// 기업 상위 직무 선택 항목
		wchar_t TOPJOB[30];
		// 기업 하위 직무 선택 항목
		wchar_t BOTJOB[30];
		// 기업 시, 도 선택 항목
		wchar_t SID[30];
		// 기업 시, 군, 구 선택 항목
		wchar_t SIG[30];
		// 기업 읍, 면, 동 입력 항목
		wchar_t EMD[30];
		// 구인 기업 채용 인원 항목
		wchar_t recruit;
	};

	// 공용 구조체
	WORKER		worker_data[MAX_WORKER];
	WORKER		sel_work;
	COOP		coop_data[MAX_COOP];
	COOP		recv_data[MAX_COOP];
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	afx_msg void OnBnClickedMatching();
	afx_msg void OnLbnSelchangeWlist();
	afx_msg void OnLbnSelchangeClist();
	void ReadWORKERData();
	void ReadCOOPData();
	void MatchingForm();
	void Matching();
	CListCtrl l_WLIST;
	CListCtrl l_CLIST;
	CListCtrl l_MATCHING;
	afx_msg void OnNMClickWlist(NMHDR* pNMHDR, LRESULT* pResult);
	void GetCoopData();
	BOOL SortData(int nCol);
	afx_msg void OnCustomdrawMatching(NMHDR* pNMHDR, LRESULT* pResult);
};