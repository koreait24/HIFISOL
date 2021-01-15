#pragma once

#include <odbcinst.h>
#include <sqlext.h>

#define MAX_SID 20
#define MAX_SIG 253
#define MAX_TOPJOB 6
#define MAX_BOTJOB 33
#define MAX_CTYPE 13
#define MAX_COOP 300


// HIFICOOP 대화 상자

class HIFICOOP : public CDialog
{
	DECLARE_DYNAMIC(HIFICOOP)

public:
	HIFICOOP(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~HIFICOOP();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COOP_DIALOG };
#endif

private:
	SQLHANDLE HF_Environment;		// ODBC 기술을 사용하기 위한 환경 정보
	SQLHDBC HF_ODBC;				// ODBC 함수를 사용하기 위한 정보
	char m_connect_flag = 0;		// 서버와 연결 여부를 저장 할 변수 (1: 연결)

public:
	// 마지막으로 호출된 사람의 인덱스
	int pIndex = 1;

	struct SID
	{
		// 시, 도 값 일련번호 Value 집합
		int v_SID;
		// 시, 도 값 이름 Value 집합
		wchar_t s_SID[30];
	};

	struct SIG
	{
		// 시, 군 값 일련번호 Value 집합
		int v_SIG;
		// 시, 군 값 이름 Value 집합
		wchar_t s_SIG[30];
	};

	struct TOPJOB
	{
		// 상위 직종 분류 값 일련번호 Value 집합
		int v_TOPJOB;
		// 상위 직종 분류 값 이름 Value 집합
		wchar_t s_TOPJOB[30];
	};

	struct BOTJOB
	{
		// 하위 직종 분류 값 일련번호 Value 집합
		int v_BOTJOB;
		// 하위 직종 분류 값 이름 Value 집합
		wchar_t s_BOTJOB[30];
	};

	struct WTYPE
	{
		// 구직 타입 분류 값 일련번호 Value 집합
		int v_CTYPE;
		// 구직 타입 분류 값 이름 Value 집합
		wchar_t s_CTYPE[30];
	};

	struct COOP
	{
		// 노동자 인덱스
		int index;
		// 노동자 이름
		wchar_t name[30];
	};

	struct EXTRACOOP
	{
		// 구인 기업 인덱스
		int index;
		// 구인 기업 이름
		wchar_t CNAME[30];
		// 구인 기업 근무 타입
		wchar_t CTYPE[30];
		// 구인 기업 근무 유형
		wchar_t CSTYLE[30];
		// 구인 기업 근무 시간 (일근 일 때만)
		wchar_t CTIME[30];
		// 구인 기업 상위 직무 선택 항목
		wchar_t CTOPJOB[30];
		// 구인 기업 하위 직무 선택 항목
		wchar_t CBOTJOB[30];
		// 구인 기업 시, 도 선택 항목
		wchar_t CSID[30];
		// 구인 기업 시, 군, 구 선택 항목
		wchar_t CSIG[30];
		// 구인 기업 읍, 면, 동 선택 항목
		wchar_t CEMD[30];
		// 구인 기업 채용 인원 항목
		wchar_t recruit;
	};

	// 공용 구조체
	SID sid_data[MAX_SID];
	SIG sig_data[MAX_SIG];
	TOPJOB tjob_data[MAX_TOPJOB];
	BOTJOB bjob_data[MAX_BOTJOB];
	WTYPE wtype_data[MAX_CTYPE];
	COOP coop_data[MAX_COOP];
	EXTRACOOP ecoop_data[MAX_COOP];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:

	CEdit e_NAME;
	CEdit e_INDEX;
	CComboBox c_CTYPE;
	CComboBox c_CTIME;
	CComboBox c_CSTYLE;
	CComboBox c_TOPJOB;
	CComboBox c_BOTJOB;
	CComboBox c_SID;
	CComboBox c_SIG;
	CListBox l_COOP;
	afx_msg
	void ReadCOOPData();
	void ReadSIDData();
	void ReadSIGData(CString get_SID);
	void ReadTOPJOBData();
	void ReadBOTJOBData(CString get_TOPJOB);
	void ReadWTYPEData();
	void OnBnClickedCdelete();
	afx_msg void OnBnClickedCrefresh();
	afx_msg void OnBnClickedCsave();
	afx_msg void OnCbnSelchangeCsid();
	afx_msg void OnCbnSelchangeCtopjob();
	afx_msg void OnCbnSelchangeCtype();
	afx_msg void OnLbnSelchangeCoop();
	virtual BOOL DestroyWindow();
	void SORTINGDATA();
	virtual BOOL OnInitDialog();
	CEdit e_RECRUIT;
	CEdit e_EMD;
};
