#include "stdafx.h"
#include "CODBC.h"
/************************************************************************
/* HandleDiagnosticRecord : display error/warning information
/*
/* Parameters:
/* hHandle ODBC handle
/* hType Type of handle (SQL_HANDLE_STMT, SQL_HANDLE_ENV, SQL_HANDLE_DBC)
/* RetCode Return code of failing command
/* ���� �߻��� ��ó
/* - ������ ũ�⸦ ǥ���ϴ� �Ű������� �׻� ���� �־�� �Ѵ�.
/*   �� ��Ʈ���� ��� SQL_NTS�� ���� �־�� �Ѵ�. (���� �μ��� SQL_NTS�� �����Ͽ� ������ �����Ͱ� ��(Null)�� ������ ǥ���� �� �ִ�.)
/* - SQLCHAR�� SQLWCHAR�� �ٲٰ� ��� ��Ʈ���� ��XXX������ L��XXX���� �ٲپ�� �Ѵ�.
/* - �ѱ� �����޼��� ���� ��
/*   �� setlocale(LC_ALL, "korean");
/*   �� std::wcout.imbue(std::locale("korean"));
/************************************************************************/
void CODBC::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	// SQLGetDiagRec �Լ��� ���� �ٸ� SQL�Լ��� 'SQL_SUCCESS'�� �ƴ� �ٸ� ���� ��ȯ���� ��� �� ������ �� �� �ִ�.
	// SQLGetDiagRec �Լ� ���� ������ ���(��������Ʈ�� ��� �ְų� �ùٸ��� ���� �޸� ������ ���� ��)�� �ֱ� ������ ������ ��쿡�� ����ϵ��� �Ѵ�.
	SQLSMALLINT wszMessageSize = sizeof(wszMessage) / sizeof(WCHAR);
	
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, wszMessageSize, &length) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5))
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
	}
}

/*
  ODBC�� ����ϸ鼭 �ٷ�� �� �ڵ���
  ȯ�� �ڵ�(Environment Handle), ���� �ڵ�(Connection Handle), ��� �ڵ�(Statement Handle)�� ��ǥ���̴�.
  ȯ�� �ڵ��� ODBC ȯ�� ���� ���� ���õ� ������,
  ���� �ڵ��� DBMS ���� ������,
  ��� �ڵ��� ���� ���� ������ �����մϴ�.
  �� �Լ����� ȯ�� �ڵ�� ���� �ڵ鸸 �Ҵ��ϰ�,
  ��� �ڵ��� �ڿ� �������� �����ų� �غ��� �� �Ҵ��ؾ� �Ѵ�.
*/
void CODBC::AllocateHandles()
{
	// ȯ�� �ڵ鷯 �Ҵ�
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// ODBC ����̹� ���� ���
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
		/*
		  SQLSetEnvAttr�� ȯ�� ���� �����ϴ� �Լ���,
		  �ٸ� ���� ����� ������ ODBC ����̹� ������ �ݵ�� ������־�� �Ѵ�.
		  ODBC�� 2.x�� 3.x�� �����µ�, �� �� ���̿� �������� �ֱ� ����.
		  �ڼ��� ������ �Ʒ� ��ũ�� Ÿ�� Ȯ���� �� �ִ�.
		  https://docs.microsoft.com/en-us/sql/odbc/reference/develop-app/declaring-the-application-s-odbc-version
		*/

		// ���� �ڵ鷯 �Ҵ�
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
  DBMS�� �����ϴ� �Լ�.
  DSN�� Data Source Name�� ���ڷ�, �� ������ ��� ������ DBMS�� ��ǥ�ϴ� �̸����� ���� �ȴ�.
  User Name���� DBMS ������ ������ �� ����ϴ� ����� �̸��� �����ش�.
  Password���� DBMS ������ �����ϴ� ������� ��й�ȣ�� �����ش�.

  ������ ������ ODBC �޼ҵ� �ϳ��� ȣ���ϰ� ������.
  ������ ȯ�� ����(.ini)�� ������ �����ϰ� �ű⼭ ���� �о�´�.
*/
void CODBC::ConnectDataSource(SQLWCHAR* dsn, SQLWCHAR* UserName, SQLWCHAR* Password)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// ���� Ÿ�Ӿƿ� ���� (5��)
		SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

		// ������ ������ ����
		retcode = SQLConnect(hdbc, dsn, SQL_NTS, UserName, SQL_NTS, Password, SQL_NTS);
		// NTS: Null Terminated String (�� ���� ���ڿ�)
		// �ش� ���ڿ� �� null���� ���������� ���� �о�´�.

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			printf("DB Connect Success\n");
		else
			CODBC::HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
	}
	else
		CODBC::HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
}

/*
  �������� ���ڷ� �ް�, �غ� �������� �ٷ� �����Ű�� �Լ�.
  �������� �ٷ� �����Ű�� �Լ��� SQLExecDirect �Լ��� ȣ���ϱ� ���� ��� �ڵ��� �Ҵ��ϴ� ���� �� �� �ִ�.
*/
void CODBC::ExecuteStatementDirect(SQLWCHAR* sql)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// ��� �ڵ鷯 �Ҵ�
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

		// �������� ����
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
  ������ �غ��Ű�� �Լ�
  SQLExecDirect �Լ��� ȣ���� ���� �������� �غ��� ���� ���� ������ ���⼭�� �ٽ� ��� �ڵ��� �Ҵ����ش�.
*/
void CODBC::PrepareStatement(SQLWCHAR * sql)
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		// ��� �ڵ鷯 �Ҵ�
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	else
		CODBC::HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);

	// ������ �غ�
	retcode = SQLPrepare(hstmt, sql, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		printf("\nQuery Prepare Success\n");
	else
		CODBC::HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
}

/*
  �غ� ������ ��ģ �������� �����Ű�� �Լ�
  SQLExecuteDirect�� �ƴ� SQLExecute �Լ��� ȣ���� �������� �����Ų��.
*/
void CODBC::ExecuteStatement()
{
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		// �������� ����
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
  ������ �������� ����� �˻��ϴ� �Լ�
  SELECT�� ���� �޾ƿ��� ���ؼ� �� ���� ����� �ִ�.
  �� �÷�(�ʵ�)���� ������ ������ ���� ���ε��� ���� �޾ƿ��ų�,
  Ŀ���� ������ (���� ��(���ڵ�)�� �������) �޾ƿ��� ����� �ִ�.
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
			// �� �÷�(�ʵ�)���� ������ ������ ���� ���ε��� ���� �޾ƿ� �غ� �Ѵ�.
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
				float* floatRef = va_arg(arg_pointer, float*); // arg_pointer�� float ũ�⸸ŭ ���������� �̵�
				DataRefs.emplace_back(make_pair(floatRef, 'f'));
				DataLens.emplace_back();
				SQLBindCol(hstmt, i + 1, SQL_C_LONG, floatRef, sizeof(int), &DataLens.back());
				break;
			}
			case 'c': // arg type is chracter
			{
				char* charRef = va_arg(arg_pointer, char*); // arg_pointer�� char ũ�⸸ŭ ���������� �̵�
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
		va_end(arg_pointer);    // ���� ���� �����͸� NULL�� �ʱ�ȭ
		

		// �� ��(���ڵ�)�� ���� �޾ƿ´�.(Fetch)
		retcode = SQLFetch(hstmt);
		/*
		���� ��� ���� �о���� �Լ�
		�� Ŀ�� ��ũ���� �ƴ϶�� �� ��(���ڵ�)�� �о���� �ǰ�, �� �÷�(�ʵ�)�� ���� ���ε��ߴ� ������ ����ȴ�.
		�Լ��� ȣ���� ������ ���� ��(���ڵ�)�� �о����, ������ ��(���ڵ�)���� ���� ���¿��� �� �� �� ȣ���Ѵٸ� 'SQL_NO_DATA'�� ��ȯ�Ѵ�.
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
		��ɹ� ���� ������ �����ϴ� �Լ�
		��� ��� ���� �޾ƿ����Ƿ� �� �̻� ���ܵ� �ʿ䰡 ����. �����ϵ��� �Ѵ�.
		SQL_UNBIND�� SQLBindCol �Լ��� ���ε��ߴ� ����(����)�� ��� ����(releasing)�Ѵ�.
		*/
	}
	else
	{
		ProcessAble = false;
		CODBC::HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}

	return ProcessAble;
}

// �Ҵ��ߴ� �ڵ��� ��� �����ϴ� �Լ�
void CODBC::DisconnectDataSource()
{
	if (hstmt)
	{
		// SQLFreeStmt �Լ��� �ڵ��� �����ϴ� ���� �ƴϱ� ������ ���⼭ �������־�� �Ѵ�.
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = NULL;
	}

	// �� �Լ��� ������ �����ϴ� �Լ�. ���� �ڵ��� �����ϱ� ���� ȣ���ؼ� ������ ���� �����ϵ��� �Ѵ�.
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
