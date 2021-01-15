// HIFIMATCHING.cpp: 구현 파일
//

#include "pch.h"
#include "HIFISOL.h"
#include "HIFIMATCHING.h"
#include "afxdialogex.h"


// HIFIMATCHING 대화 상자

IMPLEMENT_DYNAMIC(HIFIMATCHING, CDialog)

class CSorting
{
public:
	CSorting(CString str, DWORD dw);
	~CSorting() {};
	CString m_str;
	DWORD m_dw;
};

CSorting::CSorting(CString str, DWORD dw)
{
	m_str = str;
	m_dw = dw;
}

HIFIMATCHING::HIFIMATCHING(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MATCHING_DIALOG, pParent)
{
}

HIFIMATCHING::~HIFIMATCHING()
{
}

void HIFIMATCHING::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIST, l_CLIST);
	DDX_Control(pDX, IDC_WLIST, l_WLIST);
	DDX_Control(pDX, IDC_LMATCHING, l_MATCHING);
}

BEGIN_MESSAGE_MAP(HIFIMATCHING, CDialog)
	ON_BN_CLICKED(IDC_MATCHING, &HIFIMATCHING::OnBnClickedMatching)
	ON_LBN_SELCHANGE(IDC_WLIST, &HIFIMATCHING::OnLbnSelchangeWlist)
	ON_LBN_SELCHANGE(IDC_CLIST, &HIFIMATCHING::OnLbnSelchangeClist)
	ON_NOTIFY(NM_CLICK, IDC_WLIST, &HIFIMATCHING::OnNMClickWlist)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LMATCHING, &HIFIMATCHING::OnCustomdrawMatching)
END_MESSAGE_MAP()

// HIFIMATCHING 메시지 처리기

BOOL HIFIMATCHING::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.


	// ODBC 드라이버에 연결을 위한 기본 정보를 설정한다.
	SQLSetEnvAttr(NULL, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_DRIVER, SQL_IS_INTEGER);

	// ODBC 기술을 사용하기 위한 환경을 구성한다.
	// 구성된 환경 정보에 대한 핸들 값은 HF_Environment에 저장한다.
	if (SQL_ERROR != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &HF_Environment))
	{
		SQLSetEnvAttr(HF_Environment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
		SQLSetEnvAttr(HF_Environment, SQL_ATTR_CP_MATCH, (SQLPOINTER)SQL_CP_RELAXED_MATCH, SQL_IS_INTEGER);

		// ODBC 함수를 사용하기 위한 정보를 구성한다.
		// 이 정보에 대한 핸들 값은 HF_ODBC에 저장한다.
		if (SQL_ERROR != SQLAllocHandle(SQL_HANDLE_DBC, HF_Environment, &HF_ODBC))
		{
			RETCODE Ret = SQLConnect(HF_ODBC,
				// 접속할 DSN 설정
				(SQLWCHAR*)L"hifi_db", SQL_NTS,
				// 접속에 사용할 ID
				(SQLWCHAR*)L"root", SQL_NTS,
				// 접속에 사용할 PASSWORD
				(SQLWCHAR*)L"JKs900605!!", SQL_NTS);
			if (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO)
			{
				// ODBC를 사용하여 데이터베이스 서버에 성공적으로 접속한 경우
				// MessageBox(L"Success");
				m_connect_flag = 1;
				ReadWORKERData();
				ReadCOOPData();
				MatchingForm();
				GetCoopData();
				return TRUE;
			}

			else
			{
				// MessageBox(L"Fail");
				// 접속에 실패한 경우, 구성했던 메모리를 제거한다.
				if (HF_ODBC != SQL_NULL_HDBC)
				{
					SQLFreeHandle(SQL_HANDLE_DBC, HF_ODBC);
				}

				if (HF_ODBC != SQL_NULL_HENV)
				{
					SQLFreeHandle(SQL_HANDLE_ENV, HF_Environment);
				}
			}
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL HIFIMATCHING::DestroyWindow()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (m_connect_flag == 1)
	{
		if (HF_ODBC != SQL_NULL_HDBC)
		{
			SQLFreeHandle(SQL_HANDLE_DBC, HF_ODBC);
		}
		if (HF_Environment != SQL_NULL_HENV)
		{
			SQLFreeHandle(SQL_HANDLE_ENV, HF_Environment);
		}
	}

	return CDialog::DestroyWindow();
}

void HIFIMATCHING::OnBnClickedMatching()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (l_WLIST.GetSelectionMark() == -1)
	{
		MessageBox(_T("구직자가 선택되지 않았습니다."));
	}
	else
	{
		Matching();
	}
}

void HIFIMATCHING::OnLbnSelchangeWlist()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void HIFIMATCHING::OnLbnSelchangeClist()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void HIFIMATCHING::ReadWORKERData()
{
	// ComboBox에 저장된 항목들을 모두 제거한다.
	CRect rt;

	l_WLIST.GetWindowRect(&rt);
	l_WLIST.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	l_WLIST.InsertColumn(0, TEXT("순서"), LVCFMT_CENTER, rt.Width()*0.06f);
	l_WLIST.InsertColumn(1, TEXT("이름"), LVCFMT_CENTER, rt.Width()*0.09f);
	l_WLIST.InsertColumn(2, TEXT("근무형태"), LVCFMT_CENTER, rt.Width()*0.09f);
	l_WLIST.InsertColumn(3, TEXT("근무종류"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_WLIST.InsertColumn(4, TEXT("근무시간"), LVCFMT_CENTER, rt.Width()*0.09f);
	l_WLIST.InsertColumn(5, TEXT("상위직무"), LVCFMT_CENTER, rt.Width()*0.09f);
	l_WLIST.InsertColumn(6, TEXT("하위직무"), LVCFMT_CENTER, rt.Width()*0.09f);
	l_WLIST.InsertColumn(7, TEXT("시/도"), LVCFMT_CENTER, rt.Width()*0.09f);
	l_WLIST.InsertColumn(8, TEXT("시/군/구"), LVCFMT_CENTER, rt.Width()*0.09f);

	// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
	wchar_t query[256] = L"SELECT * FROM worker";
	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_WORKER];
	CString str;

	// 데이터를 저장할 배열을 초기화 한다.
	memset(worker_data, 0, sizeof(worker_data));

	HSTMT h_statement;
	RETCODE Ret;
	int num = 0, count = 0;

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(worker_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_WORKER, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &worker_data[0].index, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, worker_data[0].NAME, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 3, SQL_WCHAR, worker_data[0].TYPE, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 4, SQL_WCHAR, worker_data[0].STYLE, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 5, SQL_WCHAR, worker_data[0].TIME, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 6, SQL_WCHAR, worker_data[0].TOPJOB, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 7, SQL_WCHAR, worker_data[0].BOTJOB, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 8, SQL_WCHAR, worker_data[0].SID, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 9, SQL_WCHAR, worker_data[0].SIG, sizeof(wchar_t) * 30, NULL);

		// SQL 명령문을 실행한다.
		Ret = SQLExecDirect(h_statement, (SQLWCHAR*)query, SQL_NTS);

		// SQL 명령문의 실행 결과로 받은 데이터를 ComboBox에 추가한다.
		while (Ret = SQLFetchScroll(h_statement, SQL_FETCH_NEXT, 0) != SQL_NO_DATA)
		{
			// 데이터의 개수만큼 반복하면서 작업한다.
			for (unsigned int i = 0; i < record_num; i++)
			{
				// 가져온 데이터가 삭제된 정보가 아니라면 해당 속성으로
				// 합쳐서 문자열로 구성하고 ComboBox에 등록한다.
				if (record_state[i] != SQL_ROW_DELETED && record_state[i] != SQL_ROW_ERROR)
				{
					num = l_WLIST.GetItemCount();
					str.Format(_T("%d"), worker_data[i].index);
					l_WLIST.InsertItem(num, str);
					l_WLIST.SetItem(count, 0, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
					l_WLIST.SetItem(count, 1, LVIF_TEXT, worker_data[i].NAME, NULL, NULL, NULL, NULL);
					l_WLIST.SetItem(count, 2, LVIF_TEXT, worker_data[i].STYLE, NULL, NULL, NULL, NULL);
					l_WLIST.SetItem(count, 3, LVIF_TEXT, worker_data[i].TYPE, NULL, NULL, NULL, NULL);
					l_WLIST.SetItem(count, 4, LVIF_TEXT, worker_data[i].TIME, NULL, NULL, NULL, NULL);
					l_WLIST.SetItem(count, 5, LVIF_TEXT, worker_data[i].TOPJOB, NULL, NULL, NULL, NULL);
					l_WLIST.SetItem(count, 6, LVIF_TEXT, worker_data[i].BOTJOB, NULL, NULL, NULL, NULL);
					l_WLIST.SetItem(count, 7, LVIF_TEXT, worker_data[i].SID, NULL, NULL, NULL, NULL);
					l_WLIST.SetItem(count, 8, LVIF_TEXT, worker_data[i].SIG, NULL, NULL, NULL, NULL);
					count++;
				}
			} 
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}
}

void HIFIMATCHING::ReadCOOPData()
{
	// ComboBox에 저장된 항목들을 모두 제거한다.
	CRect rt;

	l_CLIST.GetWindowRect(&rt);
	l_CLIST.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	l_CLIST.InsertColumn(0, TEXT("순서"), LVCFMT_CENTER, rt.Width() * 0.06f);
	l_CLIST.InsertColumn(1, TEXT("이름"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(2, TEXT("근무형태"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(3, TEXT("근무종류"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(4, TEXT("근무시간"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(5, TEXT("상위직무"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(6, TEXT("하위직무"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(7, TEXT("시/도"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(8, TEXT("시/군/구"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(9, TEXT("읍/면/동"), LVCFMT_CENTER, rt.Width() * 0.09f);
	l_CLIST.InsertColumn(10, TEXT("채용인원"), LVCFMT_CENTER, rt.Width() * 0.09f);

	// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
	wchar_t query[256] = L"SELECT * FROM coop";
	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_WORKER];
	CString str;

	// 데이터를 저장할 배열을 초기화 한다.
	memset(worker_data, 0, sizeof(worker_data));

	HSTMT h_statement;
	RETCODE Ret;
	int num = 0, count = 0;

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(worker_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_WORKER, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &coop_data[0].index, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, coop_data[0].NAME, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 3, SQL_WCHAR, coop_data[0].TYPE, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 4, SQL_WCHAR, coop_data[0].STYLE, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 5, SQL_WCHAR, coop_data[0].TIME, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 6, SQL_WCHAR, coop_data[0].TOPJOB, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 7, SQL_WCHAR, coop_data[0].BOTJOB, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 8, SQL_WCHAR, coop_data[0].SID, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 9, SQL_WCHAR, coop_data[0].SIG, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 10, SQL_WCHAR, coop_data[0].EMD, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 11, SQL_INTEGER, &coop_data[0].recruit, sizeof(int) * 30, NULL);

		// SQL 명령문을 실행한다.
		Ret = SQLExecDirect(h_statement, (SQLWCHAR*)query, SQL_NTS);

		// SQL 명령문의 실행 결과로 받은 데이터를 ComboBox에 추가한다.
		while (Ret = SQLFetchScroll(h_statement, SQL_FETCH_NEXT, 0) != SQL_NO_DATA)
		{
			// 데이터의 개수만큼 반복하면서 작업한다.
			for (unsigned int i = 0; i < record_num; i++)
			{
				// 가져온 데이터가 삭제된 정보가 아니라면 해당 속성으로
				// 합쳐서 문자열로 구성하고 ComboBox에 등록한다.
				if (record_state[i] != SQL_ROW_DELETED && record_state[i] != SQL_ROW_ERROR)
				{
					num = l_WLIST.GetItemCount();
					str.Format(_T("%d"), coop_data[i].index);
					l_CLIST.InsertItem(num, str);
					l_CLIST.SetItem(count, 0, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 1, LVIF_TEXT, coop_data[i].NAME, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 2, LVIF_TEXT, coop_data[i].STYLE, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 3, LVIF_TEXT, coop_data[i].TYPE, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 4, LVIF_TEXT, coop_data[i].TIME, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 5, LVIF_TEXT, coop_data[i].TOPJOB, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 6, LVIF_TEXT, coop_data[i].BOTJOB, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 7, LVIF_TEXT, coop_data[i].SID, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 8, LVIF_TEXT, coop_data[i].SIG, NULL, NULL, NULL, NULL);
					l_CLIST.SetItem(count, 9, LVIF_TEXT, coop_data[i].EMD, NULL, NULL, NULL, NULL);
					str.Format(_T("%d"), coop_data[i].recruit);
					l_CLIST.SetItem(count, 10, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
					count++;
				}
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}
}

void HIFIMATCHING::MatchingForm()
{
	CRect rt;

	l_MATCHING.GetWindowRect(&rt);
	l_MATCHING.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	l_MATCHING.InsertColumn(0, TEXT("순서"), LVCFMT_CENTER, rt.Width() * 0.03f);
	l_MATCHING.InsertColumn(1, TEXT("매칭기업"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(2, TEXT("근무형태"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(3, TEXT("구인자상태"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(4, TEXT("근무시간"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(5, TEXT("구인자상태"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(6, TEXT("상위직무"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(7, TEXT("구인자상태"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(8, TEXT("하위직무"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(9, TEXT("구인자상태"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(10, TEXT("시/도"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(11, TEXT("구인자상태"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(12, TEXT("시/군/구"), LVCFMT_CENTER, rt.Width() * 0.07f);
	l_MATCHING.InsertColumn(13, TEXT("구인자상태"), LVCFMT_CENTER, rt.Width() * 0.05f);
	l_MATCHING.InsertColumn(14, TEXT("점수"), LVCFMT_CENTER, rt.Width() * 0.05f);
}

void HIFIMATCHING::Matching()
{
	float score = 0.0;
	CString str;
	int num;
	bool pass = true;
	l_MATCHING.DeleteAllItems();

	for (short i = 0; i < l_CLIST.GetItemCount(); i++)
	{
		pass = true;
		score = 0.0f;
		//-----------------------------------------------------------------------------------------------------------
		if (!wcscmp(sel_work.SID, recv_data[i].SID))
		{
			num = l_MATCHING.GetItemCount();
			score += 0.5f;
			str.Format(_T("%d"), i);
			l_MATCHING.InsertItem(num, str);
			l_MATCHING.SetItem(num, 0, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
			l_MATCHING.SetItem(num, 1, LVIF_TEXT, recv_data[i].NAME, NULL, NULL, NULL, NULL);
			l_MATCHING.SetItem(num, 10, LVIF_TEXT, TEXT("일치"), NULL, NULL, NULL, NULL);
			l_MATCHING.SetItem(num, 11, LVIF_TEXT, recv_data[i].SID, NULL, NULL, NULL, NULL);
		}

		else if (!wcscmp(recv_data[i].SID, _T("무관")))
		{
			num = l_MATCHING.GetItemCount();
			str.Format(_T("%d"), i);
			score += 0.4f;
			l_MATCHING.InsertItem(num, str);
			l_MATCHING.SetItem(num, 0, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
			l_MATCHING.SetItem(num, 1, LVIF_TEXT, recv_data[i].NAME, NULL, NULL, NULL, NULL);
			l_MATCHING.SetItem(num, 10, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
			l_MATCHING.SetItem(num, 11, LVIF_TEXT, recv_data[i].SID, NULL, NULL, NULL, NULL);
		}

		else
		{
			pass = false;
			if (!wcscmp(sel_work.SID, _T("무관")))
			{
				num = l_MATCHING.GetItemCount();
				str.Format(_T("%d"), i);
				score += 0.9f;
				l_MATCHING.InsertItem(num, str);
				l_MATCHING.SetItem(num, 0, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 1, LVIF_TEXT, recv_data[i].NAME, NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 10, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 11, LVIF_TEXT, recv_data[i].SID, NULL, NULL, NULL, NULL);
				pass = true;
			}
		}
		//-----------------------------------------------------------------------------------------------------------
		if (pass == true)
		{
			if (!wcscmp(sel_work.SIG, recv_data[i].SIG))
			{
				score += 0.5f;
				l_MATCHING.SetItem(num, 12, LVIF_TEXT, TEXT("일치"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 13, LVIF_TEXT, recv_data[i].SIG, NULL, NULL, NULL, NULL);
			}

			else
			{
				str.Format(_T("일치(%s)"), recv_data[i].SIG);
				l_MATCHING.SetItem(num, 12, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 13, LVIF_TEXT, recv_data[i].SIG, NULL, NULL, NULL, NULL);
				if (!wcscmp(recv_data[i].SIG, _T("NULL")))
				{
					score += 0.5f;
					l_MATCHING.SetItem(num, 12, LVIF_TEXT, TEXT("일치(무관)"), NULL, NULL, NULL, NULL);
					l_MATCHING.SetItem(num, 13, LVIF_TEXT, recv_data[i].SIG, NULL, NULL, NULL, NULL);
				}
			}
			//-----------------------------------------------------------------------------------------------------------
			if (!wcscmp(sel_work.STYLE, recv_data[i].STYLE))
			{
				score += 0.5f;
				l_MATCHING.SetItem(num, 2, LVIF_TEXT, TEXT("일치"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 3, LVIF_TEXT, recv_data[i].STYLE, NULL, NULL, NULL, NULL);
			}

			else if (!wcscmp(recv_data[i].STYLE, _T("무관")))
			{
				score += 0.4f;
				l_MATCHING.SetItem(num, 2, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 3, LVIF_TEXT, recv_data[i].STYLE, NULL, NULL, NULL, NULL);
			}

			else
			{
				l_MATCHING.SetItem(num, 2, LVIF_TEXT, TEXT("불일치"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 3, LVIF_TEXT, recv_data[i].STYLE, NULL, NULL, NULL, NULL);
				if (!wcscmp(sel_work.STYLE, _T("무관")))
				{
					score += 0.4f;
					l_MATCHING.SetItem(num, 2, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
					l_MATCHING.SetItem(num, 3, LVIF_TEXT, recv_data[i].STYLE, NULL, NULL, NULL, NULL);
				}
			}
			//-----------------------------------------------------------------------------------------------------------
			if (!wcscmp(sel_work.TIME, recv_data[i].TIME))
			{
				score += 0.5f;
				l_MATCHING.SetItem(num, 4, LVIF_TEXT, TEXT("일치"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 5, LVIF_TEXT, recv_data[i].TIME, NULL, NULL, NULL, NULL);
			}

			else if (!wcscmp(recv_data[i].TIME, _T("무관")))
			{
				score += 0.4f;
				l_MATCHING.SetItem(num, 4, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 5, LVIF_TEXT, recv_data[i].TIME, NULL, NULL, NULL, NULL);
			}

			else
			{
				l_MATCHING.SetItem(num, 4, LVIF_TEXT, TEXT("불일치"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 5, LVIF_TEXT, recv_data[i].TIME, NULL, NULL, NULL, NULL);
				if(!wcscmp(sel_work.TIME, _T("무관")))
				{
					score += 0.4f;
					l_MATCHING.SetItem(num, 4, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
					l_MATCHING.SetItem(num, 5, LVIF_TEXT, recv_data[i].TIME, NULL, NULL, NULL, NULL);
				}
			}
			//-----------------------------------------------------------------------------------------------------------
			if (!wcscmp(sel_work.TOPJOB, recv_data[i].TOPJOB))
			{
				score += 0.5f;
				l_MATCHING.SetItem(num, 6, LVIF_TEXT, TEXT("일치"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 7, LVIF_TEXT, recv_data[i].TOPJOB, NULL, NULL, NULL, NULL);
			}

			else if (!wcscmp(recv_data[i].TOPJOB, _T("무관")) || !wcscmp(sel_work.TOPJOB, _T("무관")))
			{
				score += 0.4f;
				l_MATCHING.SetItem(num, 6, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 7, LVIF_TEXT, recv_data[i].TOPJOB, NULL, NULL, NULL, NULL);
			}

			else
			{
				l_MATCHING.SetItem(num, 6, LVIF_TEXT, TEXT("불일치"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 7, LVIF_TEXT, recv_data[i].TOPJOB, NULL, NULL, NULL, NULL);
			}
			//-----------------------------------------------------------------------------------------------------------
			if (!wcscmp(sel_work.BOTJOB, recv_data[i].BOTJOB))
			{
				score += 0.5f;
				l_MATCHING.SetItem(num, 8, LVIF_TEXT, TEXT("일치"), NULL, NULL, NULL, NULL);
				l_MATCHING.SetItem(num, 9, LVIF_TEXT, recv_data[i].BOTJOB, NULL, NULL, NULL, NULL);
			}

			else if (wcscmp(sel_work.BOTJOB, recv_data[i].BOTJOB))
			{
				if (!wcscmp(recv_data[i].BOTJOB, _T("NULL")))
				{
					score += 0.5f;
					l_MATCHING.SetItem(num, 8, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
					l_MATCHING.SetItem(num, 9, LVIF_TEXT, recv_data[i].BOTJOB, NULL, NULL, NULL, NULL);
				}

				else if (!wcscmp(recv_data[i].TOPJOB, _T("무관")) || !wcscmp(sel_work.TOPJOB, _T("무관")))
				{
					score += 0.5f;
					l_MATCHING.SetItem(num, 8, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
					l_MATCHING.SetItem(num, 9, LVIF_TEXT, recv_data[i].BOTJOB, NULL, NULL, NULL, NULL);
				}

				else
				{
					l_MATCHING.SetItem(num, 8, LVIF_TEXT, TEXT("불일치"), NULL, NULL, NULL, NULL);
					l_MATCHING.SetItem(num, 9, LVIF_TEXT, recv_data[i].BOTJOB, NULL, NULL, NULL, NULL);
					if (!wcscmp(sel_work.TOPJOB, recv_data[i].TOPJOB))
					{
						str.Format(_T("일치(%s)"), recv_data[i].BOTJOB);
						l_MATCHING.SetItem(num, 8, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
						l_MATCHING.SetItem(num, 9, LVIF_TEXT, recv_data[i].BOTJOB, NULL, NULL, NULL, NULL);
					}
				}
			}

			else
			{
				score += 0.5f;
				l_MATCHING.SetItem(num, 12, LVIF_TEXT, TEXT("무관"), NULL, NULL, NULL, NULL);
			}
			str.Format(_T("%.2f"), score);
			l_MATCHING.SetItem(num, 14, LVIF_TEXT, str, NULL, NULL, NULL, NULL);
		}
	}
	SortData(14);
}

void HIFIMATCHING::OnNMClickWlist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int idx = pNMListView->iItem;

	// 선택된 아이템값의 아이템을 (0,1 ... n 번째 인덱스) 한개 가져온다.
	sel_work.index = _ttoi(l_WLIST.GetItemText(idx, 0));
	wcscpy_s(sel_work.NAME, l_WLIST.GetItemText(idx, 1));
	wcscpy_s(sel_work.STYLE, l_WLIST.GetItemText(idx, 2));
	wcscpy_s(sel_work.TYPE, l_WLIST.GetItemText(idx, 3));
	wcscpy_s(sel_work.TIME, l_WLIST.GetItemText(idx, 4));
	wcscpy_s(sel_work.TOPJOB, l_WLIST.GetItemText(idx, 5));
	wcscpy_s(sel_work.BOTJOB, l_WLIST.GetItemText(idx, 6));
	wcscpy_s(sel_work.SID, l_WLIST.GetItemText(idx, 7));
	wcscpy_s(sel_work.SIG, l_WLIST.GetItemText(idx, 8));
	//	MessageBox(sel_work.SID);
	*pResult = 0;
}

void HIFIMATCHING::GetCoopData()
{
	for (short i = 0; i <= l_CLIST.GetItemCount(); i++)
	{
			recv_data[i].index = _ttoi(l_CLIST.GetItemText(i, 0));
			wcscpy_s(recv_data[i].NAME, l_CLIST.GetItemText(i, 1));
			wcscpy_s(recv_data[i].STYLE, l_CLIST.GetItemText(i, 2));
			wcscpy_s(recv_data[i].TYPE, l_CLIST.GetItemText(i, 3));
			wcscpy_s(recv_data[i].TIME, l_CLIST.GetItemText(i, 4));
			wcscpy_s(recv_data[i].TOPJOB, l_CLIST.GetItemText(i, 5));
			wcscpy_s(recv_data[i].BOTJOB, l_CLIST.GetItemText(i, 6));
			wcscpy_s(recv_data[i].SID, l_CLIST.GetItemText(i, 7));
			wcscpy_s(recv_data[i].SIG, l_CLIST.GetItemText(i, 8));
			wcscpy_s(recv_data[i].EMD, l_CLIST.GetItemText(i, 9));
			recv_data[i].index = _ttoi(l_CLIST.GetItemText(i, 10));

	}
}

static int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSorting* pLc1 = (CSorting*)lParam1;
	CSorting* pLc2 = (CSorting*)lParam2;

	int nReturn = pLc2->m_str.Compare(pLc1->m_str);
	if (lParamSort)
	{
		return nReturn;
	}

	else
	{
		return ~nReturn;
	}
}

BOOL HIFIMATCHING::SortData(int nCol)
{
	CListCtrl* pLc = (CListCtrl*)GetDlgItem(IDC_LMATCHING);
	int items = pLc->GetItemCount();

	for (short i = 0; i < items; i++)
	{
		DWORD dw = pLc->GetItemData(i);
		CString txt = pLc->GetItemText(i, nCol);
		pLc->SetItemData(i, (DWORD)new CSorting(txt,items));
	}

	pLc->SortItems(Compare, TRUE);

	for (short i = 0; i < items; i++)                                                                                                                                                                                                                                                                                                                            
	{
		CSorting* pItem = (CSorting*)pLc->GetItemData(i);
		ASSERT(pItem);
		pLc->SetItemData(i, pItem->m_dw);
		delete pItem;
	}
	return TRUE;
}

void HIFIMATCHING::OnCustomdrawMatching(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW *pLVCD = (NMLVCUSTOMDRAW*)pNMHDR;
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
	int nRow = 0;
	int nCol = 0;
	CString strCol = _T("");

	switch (pLVCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;

	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		return;

	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		COLORREF crText, crBkgnd;
		strCol = l_MATCHING.GetItemText(pLVCD->nmcd.dwItemSpec, 2);

		int spare = (int)(pLVCD->iSubItem);

		if (3==spare || 5 == spare || 7 == spare || 9 == spare || 11 == spare || 13 == spare)
		{
			crText = RGB(255, 255, 255);
			crBkgnd = RGB(255, 0, 125);
		}

		else
		{
			crText = RGB(0, 0, 0);
			crBkgnd = RGB(255, 255, 255);
		}

		pLVCD->clrText = crText;
		pLVCD->clrTextBk = crBkgnd;

		*pResult = CDRF_DODEFAULT;
		return;
	}

}
