// HIFICOOP.cpp: 구현 파일
//

#include "pch.h"
#include "HIFISOL.h"
#include "HIFICOOP.h"
#include "afxdialogex.h"
#include <odbcinst.h>
#include <sqlext.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "odbc32.lib")

// HIFICOOP 대화 상자

IMPLEMENT_DYNAMIC(HIFICOOP, CDialog)

HIFICOOP::HIFICOOP(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_COOP_DIALOG, pParent)
{

}

HIFICOOP::~HIFICOOP()
{
}

void HIFICOOP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CNAME, e_NAME);
	DDX_Control(pDX, IDC_CINDEX, e_INDEX);
	DDX_Control(pDX, IDC_CTYPE, c_CTYPE);
	DDX_Control(pDX, IDC_CTIME, c_CTIME);
	DDX_Control(pDX, IDC_CSTYLE, c_CSTYLE);
	DDX_Control(pDX, IDC_CTOPJOB, c_TOPJOB);
	DDX_Control(pDX, IDC_CBOTJOB, c_BOTJOB);
	DDX_Control(pDX, IDC_CSID, c_SID);
	DDX_Control(pDX, IDC_CSIG, c_SIG);
	DDX_Control(pDX, IDC_COOP, l_COOP);
	DDX_Control(pDX, IDC_RECRUIT, e_RECRUIT);
	DDX_Control(pDX, IDC_CEMD, e_EMD);
}

BOOL HIFICOOP::OnInitDialog()
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
				ReadCOOPData();
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

BEGIN_MESSAGE_MAP(HIFICOOP, CDialog)
	ON_BN_CLICKED(IDC_CDELETE, &HIFICOOP::OnBnClickedCdelete)
	ON_BN_CLICKED(IDC_CREFRESH, &HIFICOOP::OnBnClickedCrefresh)
	ON_BN_CLICKED(IDC_CSAVE, &HIFICOOP::OnBnClickedCsave)
	ON_CBN_SELCHANGE(IDC_CSID, &HIFICOOP::OnCbnSelchangeCsid)
	ON_CBN_SELCHANGE(IDC_CTOPJOB, &HIFICOOP::OnCbnSelchangeCtopjob)
	ON_CBN_SELCHANGE(IDC_CTYPE, &HIFICOOP::OnCbnSelchangeCtype)
	ON_LBN_SELCHANGE(IDC_COOP, &HIFICOOP::OnLbnSelchangeCoop)
END_MESSAGE_MAP()


// HIFICOOP 메시지 처리기

void HIFICOOP::ReadSIDData()
{
	// ComboBox에 저장된 항목들을 모두 제거한다.
	c_SID.ResetContent();

	// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
	wchar_t query[256] = L"SELECT * FROM sid";
	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_SID];
	CString str;

	// 데이터를 저장할 배열을 초기화 한다.
	memset(sid_data, 0, sizeof(sid_data));

	HSTMT h_statement;
	RETCODE Ret;

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(sid_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_SID, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &sid_data[0].v_SID, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, sid_data[0].s_SID, sizeof(wchar_t) * 30, NULL);

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
					str.Format(_T("%d, %s"), sid_data[i].v_SID, sid_data[i].s_SID);
					c_SID.AddString(sid_data[i].s_SID);
				}
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}
	c_SID.SelectString(0, _T("무관"));
}

void HIFICOOP::ReadSIGData(CString get_SID)
{
	// ComboBox에 저장된 항목들을 모두 제거한다.
	c_SIG.ResetContent();
	wchar_t query[256] = L"\0";

	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_SIG];
	// 읽어온 데이터를 저장할 변수
	CString str;

	// 데이터를 저장할 배열을 초기화 한다.
	memset(sig_data, 0, sizeof(sig_data));

	HSTMT h_statement;
	RETCODE Ret;

	c_SIG.AddString(_T("무관"));
	c_SIG.SelectString(0, _T("무관"));

	// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
	if (!get_SID.Compare(_T("서울특별시")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 11000 AND serial < 12000");
	}

	else if (!get_SID.Compare(_T("부산광역시")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 26000 AND serial < 27000");
	}

	else if (!get_SID.Compare(_T("대구광역시")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 27000 AND serial < 28000");
	}

	else if (!get_SID.Compare(_T("인천광역시")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 28000 AND serial < 29000");
	}

	else if (!get_SID.Compare(_T("광주광역시")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 29000 AND serial < 30000");
	}

	else if (!get_SID.Compare(_T("대전광역시")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 30000 AND serial < 31000");
	}

	else if (!get_SID.Compare(_T("울산광역시")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 31000 AND serial < 32000");
	}

	else if (!get_SID.Compare(_T("세종특별자치시")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 36000 AND serial < 37000");
	}

	else if (!get_SID.Compare(_T("경기도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 41000 AND serial < 42000");
	}

	else if (!get_SID.Compare(_T("강원도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 42000 AND serial < 43000");
	}

	else if (!get_SID.Compare(_T("충청북도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 43000 AND serial < 44000");
	}

	else if (!get_SID.Compare(_T("충청남도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 44000 AND serial < 45000");
	}

	else if (!get_SID.Compare(_T("전라북도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 45000 AND serial < 46000");
	}

	else if (!get_SID.Compare(_T("전라남도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 46000 AND serial < 47000");
	}

	else if (!get_SID.Compare(_T("경상북도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 47000 AND serial < 48000");
	}

	else if (!get_SID.Compare(_T("경상남도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 48000 AND serial < 49000");
	}

	else if (!get_SID.Compare(_T("제주특별자치도")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig WHERE serial > 50000 AND serial < 51000");
	}

	else
	{
		wcscpy_s(query, 256, L"SELECT * FROM sig");
		goto END;
	}

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(sig_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_SID, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &sig_data[0].v_SIG, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, sig_data[0].s_SIG, sizeof(wchar_t) * 30, NULL);

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
					c_SIG.AddString(sig_data[i].s_SIG); // 디버깅
				}

			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}

	if (FALSE)
	{
	END:
		c_SIG.SelectString(0, _T("무관"));
	}
}

void HIFICOOP::ReadTOPJOBData()
{
	// ComboBox에 저장된 항목들을 모두 제거한다.
	c_TOPJOB.ResetContent();

	// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
	wchar_t query[256] = L"SELECT * FROM topjob";
	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_TOPJOB];
	CString str;

	// 데이터를 저장할 배열을 초기화 한다.
	memset(tjob_data, 0, sizeof(tjob_data));

	HSTMT h_statement;
	RETCODE Ret;

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(tjob_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_TOPJOB, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &tjob_data[0].v_TOPJOB, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, tjob_data[0].s_TOPJOB, sizeof(wchar_t) * 30, NULL);

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
					c_TOPJOB.AddString(tjob_data[i].s_TOPJOB);
				}
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}
	c_TOPJOB.SelectString(0, _T("무관"));
}

void HIFICOOP::ReadBOTJOBData(CString get_TOPJOB)
{
	// ComboBox에 저장된 항목들을 모두 제거한다.
	c_BOTJOB.ResetContent();
	wchar_t query[256] = L"\0";

	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_BOTJOB];
	// 읽어온 데이터를 저장할 변수
	CString str;

	// 데이터를 저장할 배열을 초기화 한다.
	memset(bjob_data, 0, sizeof(bjob_data));

	HSTMT h_statement;
	RETCODE Ret;

	// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
	if (!get_TOPJOB.Compare(_T("경영사무")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM botjob WHERE serial > 100000 AND serial < 200000");
	}

	else if (!get_TOPJOB.Compare(_T("영업")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM botjob WHERE serial > 200000 AND serial < 300000");
	}

	else if (!get_TOPJOB.Compare(_T("서비스")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM botjob WHERE serial > 300000 AND serial < 400000");
	}

	else if (!get_TOPJOB.Compare(_T("생산, 제조")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM botjob WHERE serial > 400000 AND serial < 500000");
	}

	else if (!get_TOPJOB.Compare(_T("건설")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM botjob WHERE serial > 500000 AND serial < 600000");
	}

	else if (!get_TOPJOB.Compare(_T("운송")))
	{
		wcscpy_s(query, 256, L"SELECT * FROM botjob WHERE serial > 600000 AND serial < 700000");
	}

	else
	{
		wcscpy_s(query, 256, L"SELECT * FROM botjob");
	}

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(bjob_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_BOTJOB, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &bjob_data[0].v_BOTJOB, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, bjob_data[0].s_BOTJOB, sizeof(wchar_t) * 30, NULL);

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
					c_BOTJOB.AddString(bjob_data[i].s_BOTJOB); // 디버깅
				}

			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}

	c_BOTJOB.SelectString(0, _T("기타"));
}

void HIFICOOP::ReadWTYPEData()
{
	wchar_t query[256];
	/* int SearchType = 0;
	// ComboBox에 저장된 항목들을 모두 제거한다.
	if (!get_WTYPE.Compare(_T("일근")))
	{
		c_CSTYLE.ResetContent();
		c_CSTYLE.AddString(_T("선택해주세요"));
		c_CSTYLE.SelectString(0, _T("선택해주세요"));
		c_CTIME.AddString(_T("선택해주세요"));
		c_CTIME.SelectString(0, _T("선택해주세요"));
		wcscpy_s(query, 256, L"SELECT * FROM wtype WHERE serial > 2000 AND serial < 3000");
		c_CTIME.EnableWindow(true);
		SearchType = 0;
	}

	else if (!get_WTYPE.Compare(_T("교대근무")))
	{
		c_CSTYLE.ResetContent();
		c_CTIME.ResetContent();
		c_CSTYLE.AddString(_T("선택해주세요"));
		c_CSTYLE.SelectString(0, _T("선택해주세요"));
		wcscpy_s(query, 256, L"SELECT * FROM wtype WHERE serial > 30 AND serial < 2000");
		c_CTIME.EnableWindow(false);
		SearchType = 1;
	}

	else
	{
		c_CTYPE.ResetContent();
		c_CSTYLE.ResetContent();
		c_CTIME.ResetContent();
		c_CTYPE.AddString(_T("선택해주세요"));
		c_CTYPE.SelectString(0, _T("선택해주세요"));
		// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
		wcscpy_s(query, 256, L"SELECT * FROM wtype WHERE serial < 30");
		c_CTIME.EnableWindow(false);
		SearchType = 2;
	} */
	
	c_CTYPE.ResetContent();
	c_CSTYLE.ResetContent();
	c_CTIME.ResetContent();
	wcscpy_s(query, 256, L"SELECT * FROM wtype");

	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_CTYPE];
	CString str;

	// 데이터를 저장할 배열을 초기화 한다.
	memset(wtype_data, 0, sizeof(wtype_data));

	HSTMT h_statement;
	RETCODE Ret;

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(wtype_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_CTYPE, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &wtype_data[0].v_CTYPE, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, wtype_data[0].s_CTYPE, sizeof(wchar_t) * 30, NULL);

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
					/*if (SearchType == 0)
					{
						if (wtype_data[i].v_CTYPE < 2100)
						{
							c_CSTYLE.AddString(wtype_data[i].s_CTYPE); // 디버깅
						}

						else
						{
							c_CTIME.AddString(wtype_data[i].s_CTYPE); // 디버깅
						}
					}

					else if (SearchType == 1)
					{
						c_CSTYLE.AddString(wtype_data[i].s_CTYPE); // 디버깅
					}

					else
					{
						c_CTYPE.AddString(wtype_data[i].s_CTYPE); // 디버깅
					}
					*/
					if (wtype_data[i].v_CTYPE < 2100 && wtype_data[i].v_CTYPE > 2000)
					{
						c_CSTYLE.AddString(wtype_data[i].s_CTYPE); // 디버깅
					}

					else if (wtype_data[i].v_CTYPE < 2000 && wtype_data[i].v_CTYPE > 1000)
					{
						c_CTYPE.AddString(wtype_data[i].s_CTYPE); // 디버깅
					}

					else
					{
						if (wtype_data[i].v_CTYPE > 2100)
						{
							c_CTIME.AddString(wtype_data[i].s_CTYPE); // 디버깅
						}
					}
				}
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}
	c_CTYPE.SelectString(0, _T("무관"));
	c_CSTYLE.SelectString(0, _T("무관"));
	c_CTIME.SelectString(0, _T("무관"));
}

void HIFICOOP::ReadCOOPData()
{
	// ListBox에 저장된 항목들을 모두 제거한다.
	l_COOP.ResetContent();

	// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
	wchar_t query[256] = L"SELECT number, name FROM coop ORDER BY number";
	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_COOP];
	CString str;

	// 데이터를 저장할 배열을 초기화 한다.
	memset(coop_data, 0, sizeof(coop_data));

	HSTMT h_statement;
	RETCODE Ret;

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(coop_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_COOP, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &coop_data[0].index, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, coop_data[0].name, sizeof(wchar_t) * 30, NULL);

		// SQL 명령문을 실행한다.
		Ret = SQLExecDirect(h_statement, (SQLWCHAR*)query, SQL_NTS);

		// SQL 명령문의 실행 결과로 받은 데이터를 ListBox에 추가한다.
		while (Ret = SQLFetchScroll(h_statement, SQL_FETCH_NEXT, 0) != SQL_NO_DATA)
		{
			// 데이터의 개수만큼 반복하면서 작업한다.
			for (unsigned int i = 0; i < record_num; i++)
			{
				// 가져온 데이터가 삭제된 정보가 아니라면 해당 속성으로
				// 합쳐서 문자열로 구성하고 ComboBox에 등록한다.
				if (record_state[i] != SQL_ROW_DELETED && record_state[i] != SQL_ROW_ERROR)
				{
					l_COOP.InsertString(-1, coop_data[i].name);
				}
				pIndex = coop_data[i].index + 1;
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}
}

void HIFICOOP::OnBnClickedCdelete()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
// List Box에 선택 항목의 위치를 얻는다.
	int index = l_COOP.GetCurSel();

	// 선택된 항목이 없으면 종료한다.
	if (index == LB_ERR)
	{
		return;
	}

	CString str;
	// 선택된 항목의 문자열을 복사한다.
	l_COOP.GetText(index, str);

	CString query;
	// str 값(선택된 이름)을 사용하여 SQL 명령문을 구성한다.
	query.Format(L"DELETE FROM coop WHERE name = \'%s\'", str);

	SQLHSTMT h_statement;
	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetEnvAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);

		// SQL 명령문을 실행한다.
		RETCODE Ret = SQLExecDirect(h_statement, (SQLWCHAR*)(const wchar_t*)query, SQL_NTS);

		SQLExecDirect(h_statement, (SQLWCHAR*)(const wchar_t*)query, SQL_NTS);

		// 성공적으로 완료되었는지 체크한다.
		if (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO)
		{
		}

		// 명령 수행이 완료되었다는 것을 설정한다.
		SQLEndTran(SQL_HANDLE_ENV, HF_Environment, SQL_COMMIT);

		// Querya 문을 위해 할당한 메모리를 해체한다.
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}

	// WORKER 리스트를 갱신한다.
	SORTINGDATA();
	ReadCOOPData();
}

void HIFICOOP::OnBnClickedCrefresh()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	wchar_t buf[20];

	c_SID.ResetContent();
	c_SIG.ResetContent();
	c_TOPJOB.ResetContent();
	c_BOTJOB.ResetContent();
	ReadSIDData();
	ReadTOPJOBData();
	ReadWTYPEData();

	_itow_s(pIndex, buf, 10);
	e_NAME.SetWindowText(_T("\0"));
	e_INDEX.SetWindowText(buf);
	e_RECRUIT.SetWindowText(_T("\0"));
	e_EMD.SetWindowText(_T("\0"));
}

void HIFICOOP::OnBnClickedCsave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	pIndex++;

	// 노동자의 정보를 저장 할 객체들
	CString name, type, style, time, topjob, botjob, sid, sig, emd, str;

	// Edit 컨트롤에서 정보를 복사한다.
	int index = GetDlgItemInt(IDC_CINDEX);
	GetDlgItemText(IDC_CNAME, name);
	GetDlgItemText(IDC_CTYPE, type);
	GetDlgItemText(IDC_CSTYLE, style);
	GetDlgItemText(IDC_CTIME, time);
	if (!time.Compare(_T("\0")))
	{
		time = _T("NULL");
	}
	GetDlgItemText(IDC_CTOPJOB, topjob);
	GetDlgItemText(IDC_CBOTJOB, botjob);
	if (!botjob.Compare(_T("\0")))
	{
		botjob = _T("NULL");
	}
	GetDlgItemText(IDC_CSID, sid);
	GetDlgItemText(IDC_CSIG, sig);
	if (!sig.Compare(_T("\0")))
	{
		sig = _T("NULL");
	}
	GetDlgItemText(IDC_CEMD, emd);
	if (!emd.Compare(_T("\0")))
	{
		emd = _T("NULL");
	}
	int incruit = GetDlgItemInt(IDC_RECRUIT);

	str.Format(L"INSERT INTO coop VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d)",
		index, name, type, style, time, topjob, botjob, sid, sig, emd, incruit);

	SQLHSTMT h_statement;

	// Query 문을 실행 할 때 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);

		// SQL 명령문을 실행한다.
		RETCODE Ret = SQLExecDirect(h_statement, (SQLWCHAR*)(const wchar_t*)str, SQL_NTS);

		// 성공적으로 완료되었는지 체크한다.
		if (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO)
		{
		}
		// 명령 수행이 완료되었다는 것을 설정한다.
		SQLEndTran(SQL_HANDLE_ENV, HF_Environment, SQL_COMMIT);
		// Query 문을 위해 할당한 메모리를 해체한다.
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}

	OnBnClickedCrefresh();
	ReadCOOPData();
}

void HIFICOOP::OnCbnSelchangeCsid()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	c_SIG.ResetContent();
	e_EMD.SetWindowText(_T("\0"));
	short SID_Selected = c_SID.GetCurSel();
	short SID_Length = c_SID.GetLBTextLen(SID_Selected);

	CString get_SID;

	c_SID.GetLBText(SID_Selected, get_SID.GetBuffer(SID_Length));
	get_SID.ReleaseBuffer();

	if (get_SID.Compare(_T("무관")))
	{
		ReadSIGData(get_SID);
	}
}

void HIFICOOP::OnCbnSelchangeCtopjob()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	c_BOTJOB.ResetContent();
	short TOPJOB_Selected = c_TOPJOB.GetCurSel();
	short TOPJOB_Length = c_TOPJOB.GetLBTextLen(TOPJOB_Selected);

	CString get_TOPJOB;

	c_TOPJOB.GetLBText(TOPJOB_Selected, get_TOPJOB.GetBuffer(TOPJOB_Length));
	get_TOPJOB.ReleaseBuffer();
	if (get_TOPJOB.Compare(_T("무관")))
	{
		ReadBOTJOBData(get_TOPJOB);
	}
}

void HIFICOOP::OnCbnSelchangeCtype()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	/* short CTYPE_Selected = c_CTYPE.GetCurSel();
	short CTYPE_Length = c_CTYPE.GetLBTextLen(CTYPE_Selected);

	CString get_CTYPE;

	c_CTYPE.GetLBText(CTYPE_Selected, get_CTYPE.GetBuffer(CTYPE_Length));
	get_CTYPE.ReleaseBuffer();

	ReadWTYPEData(); */
}

void HIFICOOP::OnLbnSelchangeCoop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	c_CTYPE.ResetContent();
	c_CSTYLE.ResetContent();
	c_CTIME.ResetContent();
	c_TOPJOB.ResetContent();
	c_BOTJOB.ResetContent();
	c_SID.ResetContent();
	c_SIG.ResetContent();

	short WLIST_Selected = l_COOP.GetCurSel() + 1;

	// sid 테이블에 저장된 데이터를 모두 읽을 SQL 명령문
	CString query;
	query.Format(L"SELECT * FROM coop WHERE number = %d", WLIST_Selected);
	// 읽어온 데이터의 상태를 기록할 변수
	unsigned short record_state[MAX_COOP];
	CString str;
	wchar_t buf[30];

	// 데이터를 저장할 배열을 초기화 한다.
	memset(ecoop_data, 0, sizeof(ecoop_data));

	HSTMT h_statement;
	RETCODE Ret;

	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// 읽은 데이터의 개수를 저장할 변수
		unsigned long record_num = 0;
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);
		// 가져온 데이터를 저장할 메모리의 크기를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(ecoop_data), 0);
		// 데이터를 가져올 때 동시성에 대한 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, SQL_IS_UINTEGER);
		// 검색된 데이터의 위치를 기억하는 방식을 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER);
		// 데이터를 가져오는 최대 단위를 설정한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_NUMBER, (SQLPOINTER)MAX_COOP, SQL_IS_UINTEGER);
		// 읽은 데이터의 상태를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROW_STATUS_PTR, record_state, 0);
		// 읽은 데이터의 개수를 저장할 변수의 주소를 전달한다.
		SQLSetStmtAttr(h_statement, SQL_ATTR_ROWS_FETCHED_PTR, &record_num, 0);

		// 테이블에서 가져온 데이터를 속성별로 raw_data 변수에 저장하기 위해서
		// 속성별로 저장할 메모리 위치를 설정한다.
		SQLBindCol(h_statement, 1, SQL_INTEGER, &ecoop_data[0].index, sizeof(int), NULL);
		SQLBindCol(h_statement, 2, SQL_WCHAR, ecoop_data[0].CNAME, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 3, SQL_WCHAR, ecoop_data[0].CTYPE, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 4, SQL_WCHAR, ecoop_data[0].CSTYLE, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 5, SQL_WCHAR, ecoop_data[0].CTIME, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 6, SQL_WCHAR, ecoop_data[0].CTOPJOB, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 7, SQL_WCHAR, ecoop_data[0].CBOTJOB, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 8, SQL_WCHAR, ecoop_data[0].CSID, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 9, SQL_WCHAR, ecoop_data[0].CSIG, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 10, SQL_WCHAR, ecoop_data[0].CEMD, sizeof(wchar_t) * 30, NULL);
		SQLBindCol(h_statement, 11, SQL_INTEGER, &ecoop_data[0].recruit, sizeof(int) * 30, NULL);


		// SQL 명령문을 실행한다.
		Ret = SQLExecDirect(h_statement, (SQLWCHAR*)(const wchar_t*)query, SQL_NTS);

		// SQL 명령문의 실행 결과로 받은 데이터를 ListBox에 추가한다.
		while (Ret = SQLFetchScroll(h_statement, SQL_FETCH_NEXT, 0) != SQL_NO_DATA)
		{
			// 데이터의 개수만큼 반복하면서 작업한다.
			for (unsigned int i = 0; i < record_num; i++)
			{
				// 가져온 데이터가 삭제된 정보가 아니라면 해당 속성으로 ComboBox에 등록한다.
				_itow_s(ecoop_data[i].index, buf, 10);
				e_INDEX.SetWindowText(buf);
				e_NAME.SetWindowText(ecoop_data[i].CNAME);
				c_CTYPE.AddString(ecoop_data[i].CTYPE);
				c_CTYPE.SelectString(0, ecoop_data[i].CTYPE);
				c_CSTYLE.AddString(ecoop_data[i].CSTYLE);
				c_CSTYLE.SelectString(0, ecoop_data[i].CSTYLE);
				c_CTIME.EnableWindow(true);
				c_CTIME.AddString(ecoop_data[i].CTIME);
				c_CTIME.SelectString(0, ecoop_data[i].CTIME);
				c_TOPJOB.AddString(ecoop_data[i].CTOPJOB);
				c_TOPJOB.SelectString(0, ecoop_data[i].CTOPJOB);
				c_BOTJOB.AddString(ecoop_data[i].CBOTJOB);
				c_BOTJOB.SelectString(0, ecoop_data[i].CBOTJOB);
				c_SID.AddString(ecoop_data[i].CSID);
				c_SID.SelectString(0, ecoop_data[i].CSID);
				c_SIG.AddString(ecoop_data[i].CSIG);
				c_SIG.SelectString(0, ecoop_data[i].CSIG);
				e_EMD.SetWindowText(ecoop_data[i].CEMD);
				_itow_s(ecoop_data[i].recruit, buf, 10);
				e_RECRUIT.SetWindowText(buf);
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}
}

BOOL HIFICOOP::DestroyWindow()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	// 서버에 연결되어 있는 경우에만 제거한다. 서버에 연결을 실패했다면
	// 실패한 시점에서 아래의 정보가 정리되었기 때문이다.
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

void HIFICOOP::SORTINGDATA()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// List Box에 선택 항목의 위치를 얻는다.
	int index = l_COOP.GetCurSel();

	// 선택된 항목이 없으면 종료한다.
	if (index == LB_ERR)
	{
		return;
	}

	CString str;
	// 선택된 항목의 문자열을 복사한다.
	l_COOP.GetText(index, str);

	CString query;
	// str 값(선택된 이름)을 사용하여 SQL 명령문을 구성한다.
	query.Format(L"UPDATE coop SET number = number - 1 WHERE number > %d", index);

	SQLHSTMT h_statement;
	// Query 문을 위한 메모리를 할당한다.
	if (SQL_SUCCESS == SQLAllocHandle(SQL_HANDLE_STMT, HF_ODBC, &h_statement))
	{
		// Query 문을 실행할 때 타임 아웃을 설정한다.
		SQLSetEnvAttr(h_statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)15, SQL_IS_UINTEGER);

		// SQL 명령문을 실행한다.
		RETCODE Ret = SQLExecDirect(h_statement, (SQLWCHAR*)(const wchar_t*)query, SQL_NTS);

		// 성공적으로 완료되었는지 체크한다.
		if (Ret == SQL_SUCCESS || Ret == SQL_SUCCESS_WITH_INFO)
		{
			MessageBox(L"삭제되었습니다.");
		}

		// 명령 수행이 완료되었다는 것을 설정한다.
		SQLEndTran(SQL_HANDLE_ENV, HF_Environment, SQL_COMMIT);

		// Querya 문을 위해 할당한 메모리를 해체한다.
		SQLFreeHandle(SQL_HANDLE_STMT, h_statement);
	}
}