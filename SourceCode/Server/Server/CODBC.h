#pragma once
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")
#include <sqlext.h>

// ODBC�� �̿��ϱ� ���� ������ Ŭ����.
class CODBC {
	SQLHENV   henv;
	SQLHDBC   hdbc;
	SQLHSTMT  hstmt = 0;
	SQLRETURN retcode;

	SQLLEN STR_MAX = NAME_LEN;

	//////////////////// ���� �޼����� ���� ���� ////////////////////
	SQLSMALLINT length, iRec = 0;
	SQLINTEGER  iError;
	SQLWCHAR wszState[SQL_SQLSTATE_SIZE + 1], wszMessage[1000];
	/////////////////////////////////////////////////////////////////

public:

	// ODBC ó������� ���.
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

	// ODBC �ڵ� ������ �ʱ�ȭ.
	void AllocateHandles();

	// ������ DBMS�� ����.
	void ConnectDataSource(SQLWCHAR* dsn, SQLWCHAR* UserName, SQLWCHAR* Password);

	// �������� ���ڷ� �޾� ������ �غ� �������� ����.
	void ExecuteStatementDirect(SQLWCHAR* sql);

	// �������� �����Ű�� ���� ���� �غ� ������ ��ġ�� �Լ�.
	// �غ� ������ ��ġ�� �� �� �����Ų �������� �ٽ� �����ų �� �ֱ� ������ ���� �� ����� ����� �غ� ������ ��ġ�� ���� ����.
	void PrepareStatement(SQLWCHAR* sql);

	// ���� �غ�� �������� �����Ű�� �Լ�.
	void ExecuteStatement();

	// �������� ����� �д� �Լ�. (return ��(���ڵ�)�� ����)
	bool RetrieveResult(char* dataTypes, ...);

	// �Ҵ��ߴ� �ڵ��� ��� �����ϴ� �Լ�. 
	void DisconnectDataSource();

	void SetBufferLength(SQLLEN size) { STR_MAX = size; }

	// void SetBindColData(char* dataType, ...);
	// void GetColData(char* dataType, ...);

};