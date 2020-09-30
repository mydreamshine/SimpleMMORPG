#include "stdafx.h"
#include "CODBC.h"
/************************************************************************
/* HandleDiagnosticRecord : display error/warning information
/*
/* Parameters:
/* hHandle ODBC handle
/* hType Type of handle (SQL_HANDLE_STMT, SQL_HANDLE_ENV, SQL_HANDLE_DBC)
/* RetCode Return code of failing command
/* 오류 발생시 대처
/* - 변수의 크기를 표시하는 매개변수는 항상 맞춰 주어야 한다.
/*   ※ 스트링의 경우 SQL_NTS를 적어 주어야 한다. (길이 인수를 SQL_NTS로 설정하여 연관된 데이터가 널(Null)로 끝남을 표시할 수 있다.)
/* - SQLCHAR를 SQLWCHAR로 바꾸고 모든 스트링을 “XXX”에서 L”XXX”로 바꾸어야 한다.
/* - 한글 오류메세지 보는 법
/*   ※ setlocale(LC_ALL, "korean");
/*   ※ std::wcout.imbue(std::locale("korean"));
/************************************************************************/
void CODBC::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	// SQLGetDiagRec 함수를 통해 다른 SQL함수가 'SQL_SUCCESS'가 아닌 다른 값을 반환했을 경우 그 원인을 알 수 있다.
	// SQLGetDiagRec 함수 또한 실패할 경우(에러리스트가 비어 있거나 올바르지 못한 메모리 엑세스 참조 등)가 있기 때문에 성공할 경우에만 출력하도록 한다.
	SQLSMALLINT wszMessageSize = sizeof(wszMessage) / sizeof(WCHAR);
	
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, wszMessageSize, &length) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5))
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
	}
}

/*
  ODBC를 사용하면서 다루게 될 핸들은
  환경 핸들(Environment Handle), 연결 핸들(Connection Handle), 명령 핸들(Statement Handle)이 대표적이다.
  환경 핸들은 ODBC 환경 설정 값과 관련된 정보를,
  연결 핸들은 DBMS 연결 정보를,
  명령 핸들은 쿼리 관련 정보를 저장합니다.
  이 함수에선 환경 핸들과 연결 핸들만 할당하고,
  명령 핸들은 뒤에 쿼리문을 던지거나 준비할 때 할당해야 한다.
*/
void CODBC::AllocateHandles()
{
	// 환경 핸들러 할당
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// ODBC 드라이버 버전 명시
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
		/*
		  SQLSetEnvAttr는 환경 값을 설정하는 함수로,
		  다른 값은 상관이 없지만 ODBC 드라이버 버전은 반드시 명시해주어야 한다.
		  ODBC는 2.x와 3.x로 나뉘는데, 이 둘 사이에 차이점이 있기 때문.
		  자세한 내용은 아래 링크를 타면 확인할 수 있다.
		  https://docs.microsoft.com/en-us/sql/odbc/reference/develop-app/declaring-the-application-s-odbc-version
		*/

		// 연결 핸들러 할당
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
			
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				printf("ODBC Environment Allocate Success\n");
			else
				CODBC::HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
		}
		else
			CODBC::HandleDiagnosticRecord(henv, SQL_HANDLE_ENV, retcode);
	}
	else
		CODBC::HandleDiagnosticRecord(henv, SQL_HANDLE_ENV, retcode);
}

/*
  DBMS에 연결하는 함수.
  DSN은 Data Source Name의 약자로, 이 예제의 경우 연결할 DBMS를 대표하는 이름으로 보면 된다.
  User Name에는 DBMS 서버에 접속할 때 사용하는 사용자 이름을 적어준다.
  Password에는 DBMS 서버에 접속하는 사용자의 비밀번호를 적어준다.

  간단한 예제라 ODBC 메소드 하나만 호출하고 끝난다.
  보통은 환경 파일(.ini)에 정보를 저장하고 거기서 값을 읽어온다.
*/
void CODBC::ConnectDataSource(SQLWCHAR* dsn, SQLWCHAR* UserName, SQLWCHAR* Password)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// 연결 타임아웃 설정 (5초)
		SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

		// 데이터 원본에 연결
		retcode = SQLConnect(hdbc, dsn, SQL_NTS, UserName, SQL_NTS, Password, SQL_NTS);
		// NTS: Null Terminated String (널 종단 문자열)
		// 해당 문자열 중 null값을 만날때까지 값을 읽어온다.

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			printf("DB Connect Success\n");
		else
			CODBC::HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
	}
	else
		CODBC::HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
}

/*
  쿼리문을 인자로 받고, 준비 과정없이 바로 실행시키는 함수.
  쿼리문을 바로 실행시키는 함수인 SQLExecDirect 함수를 호출하기 전에 명령 핸들을 할당하는 것을 볼 수 있다.
*/
void CODBC::ExecuteStatementDirect(SQLWCHAR* sql)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// 명령 핸들러 할당
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

		// 쿼리문을 실행
		retcode = SQLExecDirect(hstmt, sql, SQL_NTS);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			printf("Query Execute Success\n");
		else
			CODBC::HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}
	else
		CODBC::HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
}

/*
  쿼리를 준비시키는 함수
  SQLExecDirect 함수를 호출한 다음 쿼리문을 준비할 일은 없기 때문에 여기서도 다시 명령 핸들을 할당해준다.
*/
void CODBC::PrepareStatement(SQLWCHAR * sql)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		// 명령 핸들러 할당
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	else
		CODBC::HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);

	// 쿼리문 준비
	retcode = SQLPrepare(hstmt, sql, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		printf("\nQuery Prepare Success\n");
	else
		CODBC::HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
}

/*
  준비 과정을 거친 쿼리문을 실행시키는 함수
  SQLExecuteDirect가 아닌 SQLExecute 함수를 호출해 쿼리문을 실행시킨다.
*/
void CODBC::ExecuteStatement()
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		// 쿼리문을 실행
		retcode = SQLExecute(hstmt);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			printf("Query Execute Success\n");
		else
			CODBC::HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}
	else
		CODBC::HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
}

/*
  실행한 쿼리문의 결과를 검색하는 함수
  SELECT한 값을 받아오기 위해선 두 가지 방법이 있다.
  각 컬럼(필드)별로 변수를 선언한 다음 바인딩해 값을 받아오거나,
  커서를 선언해 (여러 행(레코드)을 블록으로) 받아오는 방식이 있다.
*/
bool CODBC::RetrieveResult(char* dataTypes, ...)
{
	bool ProcessAble = true;
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		va_list arg_pointer;
		int i = 0;
		va_start(arg_pointer, dataTypes);

		list<SQLLEN> DataLens;
		list<pair<void*, char>> DataRefs;

		while (dataTypes[i] != '\0')
		{
			// 각 컬럼(필드)별로 변수를 선언한 다음 바인딩해 값을 받아올 준비를 한다.
			switch (dataTypes[i])
			{
			case 'i': // arg type is integer
			{
				int* intRef = va_arg(arg_pointer, int*);
				DataRefs.emplace_back(make_pair(intRef, 'i'));
				DataLens.emplace_back();
				SQLBindCol(hstmt, i + 1, SQL_C_LONG, intRef, sizeof(int), &DataLens.back());
				break;
			}
			case 'f': // arg type is float
			{
				float* floatRef = va_arg(arg_pointer, float*); // arg_pointer를 float 크기만큼 순방향으로 이동
				DataRefs.emplace_back(make_pair(floatRef, 'f'));
				DataLens.emplace_back();
				SQLBindCol(hstmt, i + 1, SQL_C_LONG, floatRef, sizeof(int), &DataLens.back());
				break;
			}
			case 'c': // arg type is chracter
			{
				char* charRef = va_arg(arg_pointer, char*); // arg_pointer를 char 크기만큼 순방향으로 이동
				DataRefs.emplace_back(make_pair(charRef, 'c'));
				DataLens.emplace_back();
				SQLBindCol(hstmt, i + 1, SQL_C_CHAR, charRef, sizeof(char), &DataLens.back());
				break;
			}
			case 's': // arg type is string
			{
				char *strRef = va_arg(arg_pointer, char*);
				DataRefs.emplace_back(make_pair(strRef, 's'));
				DataLens.emplace_back();
				SQLBindCol(hstmt, i + 1, SQL_C_CHAR, strRef, STR_MAX, &DataLens.back());
				break;
			}
			default:
				printf("Unknown data type!\n");
				break;
			}
			i++;
		}
		va_end(arg_pointer);    // 가변 인자 포인터를 NULL로 초기화
		

		// 한 행(레코드)씩 값을 받아온다.(Fetch)
		retcode = SQLFetch(hstmt);
		/*
		쿼리 결과 값을 읽어오는 함수
		한 커서 스크롤이 아니라면 한 줄(레코드)씩 읽어오게 되고, 각 컬럼(필드)의 값은 바인딩했던 변수에 저장된다.
		함수를 호출할 때마다 다음 줄(레코드)을 읽어오며, 마지막 줄(레코드)까지 읽은 상태에서 한 번 더 호출한다면 'SQL_NO_DATA'를 반환한다.
		*/

		if (retcode == SQL_ERROR || retcode == SQL_NO_DATA)
		{
			CODBC::HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
			ProcessAble = false;
		}
		/*if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			for (auto dataRef : DataRefs)
			{
				switch (dataRef.second)
				{
				case 'i':
					wprintf(L"%d  ", *(reinterpret_cast<int*>(dataRef.first)));
					break;
				case 'f':
					wprintf(L"%f  ", *(reinterpret_cast<float*>(dataRef.first)));
					break;
				case 'c':
					wprintf(L"%c  ", *(reinterpret_cast<char*>(dataRef.first)));
					break;
				case 's':
					wprintf(L"%S  ", reinterpret_cast<char*>(dataRef.first));
					break;
				}
			}
		}
		wprintf(L"\n");*/

		SQLFreeStmt(hstmt, SQL_UNBIND);
		/*
		명령문 관련 정보를 해제하는 함수
		모든 결과 값을 받아왔으므로 더 이상 남겨둘 필요가 없다. 해제하도록 한다.
		SQL_UNBIND는 SQLBindCol 함수로 바인딩했던 버퍼(변수)를 모두 해제(releasing)한다.
		*/
	}
	else
	{
		ProcessAble = false;
		CODBC::HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}

	return ProcessAble;
}

// 할당했던 핸들을 모두 해제하는 함수
void CODBC::DisconnectDataSource()
{
	if (hstmt)
	{
		// SQLFreeStmt 함수는 핸들을 해제하는 것이 아니기 때문에 여기서 해제해주어야 한다.
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = NULL;
	}

	// 이 함수는 연결을 해제하는 함수. 연결 핸들을 해제하기 전에 호출해서 연결을 먼저 해제하도록 한다.
	SQLDisconnect(hdbc);

	if (hdbc) {
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		hdbc = NULL;
	}

	if (henv) {
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		henv = NULL;
	}
}
