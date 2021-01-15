#pragma once
#include <odbcinst.h>
#include <sqlext.h>

#define MAX_SID 20
#define MAX_SIG 253
#define MAX_TOPJOB 6
#define MAX_BOTJOB 33
#define MAX_WTYPE 13
#define MAX_WORKER 300


// HIFIWORKER 대화 상자

class HIFIWORKER : public CDialog
{
	DECLARE_DYNAMIC(HIFIWORKER)

public:
	HIFIWORKER(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~HIFIWORKER();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WORKER_DIALOG };
#endif

private:
	SQLHANDLE HF_Environment;		// ODBC 기술을 사용하기 위한 환경 정보
	SQLHDBC HF_ODBC;				// ODBC 함수를 사용하기 위한 정보
	char m_connect_flag = 0;		// 서버와 연결 여부를 저장 할 변수 (1: 연결)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
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
		int v_WTYPE;
		// 구직 타입 분류 값 이름 Value 집합
		wchar_t s_WTYPE[30];
	};

	struct WORKER
	{
		// 노동자 인덱스
		int index;
		// 노동자 이름
		wchar_t name[30];
	};

	struct EXTRAWORKER
	{
		// 노동자 인덱스
		int index;
		// 노동자 이름
		wchar_t WNAME[30];
		// 노동자 근무 타입
		wchar_t WTYPE[30];
		// 노동자 근무 유형
		wchar_t WSTYLE[30];
		// 노동자 근무 시간 (일근 일 때만)
		wchar_t WTIME[30];
		// 노동자 상위 직무 선택 항목
		wchar_t WTOPJOB[30];
		// 노동자 하위 직무 선택 항목
		wchar_t WBOTJOB[30];
		// 노동자 시, 도 선택 항목
		wchar_t WSID[30];
		// 노동자 시, 군, 구 선택 항목
		wchar_t WSIG[30];
	};

	// 공용 구조체
	SID sid_data[MAX_SID];
	SIG sig_data[MAX_SIG];
	TOPJOB tjob_data[MAX_TOPJOB];
	BOTJOB bjob_data[MAX_BOTJOB];
	WTYPE wtype_data[MAX_WTYPE];
	WORKER worker_data[MAX_WORKER];
	EXTRAWORKER eworker_data[MAX_WORKER];

	void ReadSIDData();
	void ReadSIGData(CString get_SID);
	void ReadTOPJOBData();
	void ReadBOTJOBData(CString get_TOPJOB);
	void ReadWTYPEData();
	void ReadWORKERData();
	// 시, 도 정보를 표시하는 콤보박스
	CComboBox c_SID;
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRefresh();
	CComboBox c_SIG;
	afx_msg void OnCbnSelchangeSid();
	CComboBox c_TOPJOB;
	CComboBox c_BOTJOB;
	afx_msg void OnCbnSelchangeTopjob();
	CComboBox c_WTYPE;
	CComboBox c_WSTYLE;
	CComboBox c_WTIME;
	afx_msg void OnCbnSelchangeWtype();
	CEdit e_INDEX;
	afx_msg void OnBnClickedSave();
	CEdit e_NAME;
	CListBox l_WORKER;
	afx_msg void OnLbnSelchangeWorker();
	afx_msg void OnBnClickedDelete();
	void SORTINGDATA();
};