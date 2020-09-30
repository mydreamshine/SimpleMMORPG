#pragma once
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")
#include <sqlext.h>

// ODBC를 이용하기 위해 정의한 클래스.
class CODBC {
	SQLHENV   henv;
	SQLHDBC   hdbc;
	SQLHSTMT  hstmt = 0;
	SQLRETURN retcode;

	SQLLEN STR_MAX = NAME_LEN;

	//////////////////// 에러 메세지를 위한 변수 ////////////////////
	SQLSMALLINT length, iRec = 0;
	SQLINTEGER  iError;
	SQLWCHAR wszState[SQL_SQLSTATE_SIZE + 1], wszMessage[1000];
	/////////////////////////////////////////////////////////////////

public:

	// ODBC 처리결과를 출력.
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

	// ODBC 핸들 변수를 초기화.
	void AllocateHandles();

	// 연결할 DBMS에 접속.
	void ConnectDataSource(SQLWCHAR* dsn, SQLWCHAR* UserName, SQLWCHAR* Password);

	// 쿼리문을 인자로 받아 쿼리문 준비 과정없이 실행.
	void ExecuteStatementDirect(SQLWCHAR* sql);

	// 쿼리문을 실행시키기 위해 따로 준비 과정을 거치는 함수.
	// 준비 과정을 거치면 한 번 실행시킨 쿼리문을 다시 실행시킬 수 있기 때문에 여러 번 사용할 경우라면 준비 과정을 거치는 것이 좋다.
	void PrepareStatement(SQLWCHAR* sql);

	// 실행 준비된 쿼리문을 실행시키는 함수.
	void ExecuteStatement();

	// 쿼리문의 결과를 읽는 함수. (return 행(레코드)의 개수)
	bool RetrieveResult(char* dataTypes, ...);

	// 할당했던 핸들을 모두 해제하는 함수. 
	void DisconnectDataSource();

	void SetBufferLength(SQLLEN size) { STR_MAX = size; }

	// void SetBindColData(char* dataType, ...);
	// void GetColData(char* dataType, ...);

};