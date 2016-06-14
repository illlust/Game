#include <cstdio>
#include <iostream>
#include <atlconv.h>
#include "Database.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////

void ReportError(_com_error& e)
{
	_bstr_t source = e.Source();
	_bstr_t description = e.Description();

	std::cerr << hex << e.Error() << dec << endl;
	std::cerr << (const char*)source << endl;
	std::cerr << (const char*)description << endl;

	OutputDebugStringA((const char*)source);
	OutputDebugStringA((const char*)description);
}

//////////////////////////////////////////////////////////////////////////

Variant::Variant(FieldPtr field) : _field(field)
{
}

Variant::Variant(const Variant& rhs) : _field(rhs._field)
{
}

Variant::~Variant()
{
}

Variant::operator void*()
{
	if (IsNull())
		return NULL;

	return this;
}

void Variant::GetValue(int* value)
{
	if (IsNull())
		return;

	VARIANT variant;
	VariantInit(&variant);
	VARIANT source = _field->Value;

	HRESULT hr = VariantChangeType(&variant, &source, 0, VT_INT);
	if (FAILED(hr))
		return;

	*value = variant.intVal;
	VariantClear(&variant);
}

void Variant::GetValue(float* value)
{
	if (IsNull())
		return;

	VARIANT variant;
	VariantInit(&variant);
	VARIANT source = _field->Value;

	HRESULT hr = VariantChangeType(&variant, &source, 0, VT_R4);
	if (FAILED(hr))
		return;

	*value = variant.fltVal;
	VariantClear(&variant);
}

void Variant::GetValue(double* value)
{
	if (IsNull())
		return;

	VARIANT variant;
	VariantInit(&variant);
	VARIANT source = _field->Value;

	HRESULT hr = VariantChangeType(&variant, &source, 0, VT_R8);
	if (FAILED(hr))
		return;

	*value = variant.dblVal;
	VariantClear(&variant);
}

void Variant::GetValue(bool* value)
{
	if (IsNull())
		return;

	VARIANT variant;
	VariantInit(&variant);
	VARIANT source = _field->Value;

	HRESULT hr = VariantChangeType(&variant, &source, 0, VT_BOOL);
	if (FAILED(hr))
		return;

	*value = (variant.boolVal == VARIANT_TRUE);
	VariantClear(&variant);
}

void Variant::GetValue(string* value)
{
	if (IsNull())
		return;

	VARIANT variant;
	VariantInit(&variant);
	VARIANT source = _field->Value;

	HRESULT hr = VariantChangeType(&variant, &source, 0, VT_BSTR);
	if (FAILED(hr))
		return;

	USES_CONVERSION;
	*value = OLE2A(variant.bstrVal);
	VariantClear(&variant);
}

void Variant::GetValue(tm* value)
{
	if (IsNull())
		return;

	VARIANT variant;
	VariantInit(&variant);
	VARIANT source = _field->Value;

	HRESULT hr = VariantChangeType(&variant, &source, 0, VT_DATE);
	if (FAILED(hr))
		return;

	SYSTEMTIME time;
	VariantTimeToSystemTime(variant.date, &time);
	value->tm_year = time.wYear - 1900;
	value->tm_mon = time.wMonth - 1;
	value->tm_wday = time.wDayOfWeek;
	value->tm_mday = time.wDay;
	value->tm_hour = time.wHour;
	value->tm_min = time.wMinute;
	value->tm_sec = time.wSecond;
	VariantClear(&variant);
}

bool Variant::IsNull()
{
	if (!_field)
		return true;
	if (_field->Value.vt == VT_NULL)
		return true;
	if (_field->Value.vt == VT_EMPTY)
		return true;

	return false;
}

Variant& Variant::operator =(const Variant& rhs)
{
	if (this != &rhs)
	{
		_field = rhs._field;
	}
	return *this;
}

Variant& Variant::operator =(int value)
{
	_field->Value = (long)value;
	return *this;
}

Variant& Variant::operator =(string value)
{
	_field->Value = value.c_str();
	return *this;
}

//////////////////////////////////////////////////////////////////////////

bool IsStringType(long type)
{
	switch (type)
	{
	case ADO::adBSTR:
	case ADO::adChar:
	case ADO::adVarChar:
	case ADO::adLongVarChar:
	case ADO::adWChar:
	case ADO::adVarWChar:
	case ADO::adLongVarWChar:
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////

Records::Records(_RecordsetPtr rs) : _rs(rs)
{
}

Records::Records(const Records& rhs) : _rs(rhs._rs)
{
}

Records::~Records()
{
}

Records& Records::operator =(const Records& rhs)
{
	if (this != &rhs)
	{
		// _RecordsetPtr 자체가 레퍼런스 카운트를 관리하므로 별도로 해줄 것이 아무 것도 없다
		_rs = rhs._rs;
	}

	return *this;
}

Records& Records::operator --()
{
	MovePrev();
	return *this;
}

Records& Records::operator ++()
{
	MoveNext();
	return *this;
}

const Records Records::operator --(int)
{
	const Records set = *this;
	--(*this);
	return set;
}

const Records Records::operator ++(int)
{
	const Records set = *this;
	++(*this);
	return set;
}

Records::operator void*()
{
	if (IsNull())
		return NULL;

	return this;
}

Variant Records::operator [](string index)
{
	return GetFieldValue(index);
}

Variant Records::operator [](int index)
{
	return GetFieldValue(index);
}

// _Recordset* Records::operator->()
// {
// 	return _rs.GetInterfacePtr();
// }

Variant Records::GetFieldValue(string index)
{
	if (IsNull())
		return NULL;

	try
	{
		return _rs->Fields->GetItem((_variant_t)index.c_str());
	}
	catch (_com_error& e)
	{
		REPORT_COM_ERROR(e);
	}

	return NULL;
}

Variant Records::GetFieldValue(int index)
{
	if (IsNull())
		return NULL;

	try
	{
		return _rs->Fields->GetItem((long)index);
	}
	catch (_com_error& e)
	{
		REPORT_COM_ERROR(e);
	}

	return NULL;
}

string Records::GetFieldName(int index)
{
//	ASSERT(index < GetFieldCount());
	return (const char*)(_bstr_t)_rs->Fields->GetItem((_variant_t)(long)index)->Name;
}

string Records::GetTableName(int index)
{
//	ASSERT(index < GetFieldCount());
	//	return (LPCTSTR)(_bstr_t)_rs->Fields->GetItem((_variant_t)(long)index)->Properties->GetItem("BaseTableName")->Value;

	_variant_t value = _rs->Fields->GetItem((_variant_t)(long)index)->Properties->GetItem("BaseTableName")->Value;
	if (value.vt == VT_NULL || value.vt == VT_EMPTY)
		return "";

	USES_CONVERSION;
	return OLE2A(value.bstrVal);
}

string Records::GetFieldDescription(int index, _CatalogPtr catalog)
{
//	ASSERT(index < GetFieldCount());

	_bstr_t tableName = GetTableName(index).c_str();
	_bstr_t fieldName = GetFieldName(index).c_str();

	if (fieldName.length() == 0)
		return "";

	if (tableName.length() == 0)
		return (const char*)fieldName;

	if (!catalog)
	{
		catalog.CreateInstance(__uuidof(Catalog));
	}

	catalog->PutActiveConnection(_rs->GetActiveConnection());

	try
	{
		_bstr_t description = catalog->GetTables()->GetItem(tableName)->Columns->GetItem(fieldName)->Properties->GetItem("Description")->Value;
		return description.length() == 0 ? (const char*)fieldName : (const char*)description;
	}
	catch (_com_error& e)
	{
		REPORT_COM_ERROR(e);
	}

	return "";
}

int Records::GetFieldCount()
{
	if (IsNull())
		return 0;

	return _rs->Fields->Count;
}

int Records::GetRecordCount()
{
	if (!_rs)
		return 0;

	if (_rs->State == adStateClosed)
		return 0;

	return _rs->RecordCount;
}

void Records::MoveFirst()
{
	if (!_rs)
		return;

	if (_rs->State == adStateClosed)
		return;

	_rs->MoveFirst();
}

void Records::MoveLast()
{
	if (!_rs)
		return;

	if (_rs->State == adStateClosed)
		return;

	_rs->MoveLast();
}

void Records::MoveNext()
{
	if (IsNull())
		return;

	_rs->MoveNext();
}

void Records::MovePrev()
{
	if (IsNull())
		return;

	_rs->MovePrevious();
}

void Records::Move(int offset)
{
	if (IsNull())
		return;

	_rs->Move(offset);
}

void Records::MoveAbsolutePosition(int position)
{
	if (!_rs)
		return;

	if (_rs->State == adStateClosed)
		return;

	MoveFirst();
	Move(position);
}

int Records::GetAbsolutePosition()
{
	if (IsNull())
		return -1;

	return _rs->GetAbsolutePosition();
}

bool Records::IsNull()
{
	if (!_rs)
		return true;

	if (_rs->State == adStateClosed)
		return true;

	if (_rs->ADOBOF || _rs->ADOEOF)
		return true;

	return false;
}

string Records::GetMergeFieldDescription(string header, string footer, _CatalogPtr catalog)
{
	if (IsNull())
		return "";

	string merge;

	for (int i = 0; i < GetFieldCount(); i++)
	{
		merge += header;
		merge += GetFieldDescription(i, catalog);
		merge += footer;
	}

	return merge;
}

string Records::GetMergeFieldName(string header, string footer)
{
	if (IsNull())
		return "";

	string merge;

	for (int i = 0; i < GetFieldCount(); i++)
	{
		merge += header;
		merge += GetFieldName(i);
		merge += footer;
	}

	return merge;
}

long Records::GetFieldType(int index)
{
	if (IsNull())
		return 0;

	try
	{
		return _rs->Fields->GetItem((long)index)->Type;
	}
	catch (_com_error& e)
	{
		REPORT_COM_ERROR(e);
	}

	return 0;
}

string Records::GetMergeFieldValue(string header, string footer, bool)
{
	if (IsNull())
		return "";

	string merge;

	for (int i = 0; i < GetFieldCount(); i++)
	{
		merge += header;

		// 문자열 타입이면 필드값 앞뒤를 ' 따옴표로 감싸줘서 SQL 쿼리를 만들 수 있도록 해준다
		long type = GetFieldType(i);
		string quote = IsStringType(type) ? "'" : "";		
		merge += quote;
		Variant v = GetFieldValue(i);
		string s = v;
		merge += s;
		merge += quote;

		merge += footer;
	}

	return merge;
}

string Records::GetMergeFieldPair(string separator, string footer, string table)
{
	if (IsNull())
		return "";

	string merge;

	for (int i = 0; i < GetFieldCount(); i++)
	{
		// 조인된 테이블일 경우 원하는 테이블의 값들만 수집 가능하도록 테이블명을 매개 변수로 받는다
		if (table == "" || GetTableName(i) == table) // 전체 테이블 || 선택된 테이블명
		{
			merge += GetFieldName(i);
			merge += separator;

			long type = GetFieldType(i);
			string quote = IsStringType(type) ? "'" : "";
			merge += quote;
			Variant v = GetFieldValue(i);
			string s = v;
			merge += s;
			merge += quote;

			merge += footer;
		}
	}

	return merge;
}

Records Records::Clone()
{
	if (IsNull())
		return NULL;

	_RecordsetPtr rs;
	rs.CreateInstance(__uuidof(Recordset));

	rs->CursorLocation = _rs->CursorLocation;

	_CommandPtr cmd = _rs->ActiveCommand;
	rs->Open(cmd->CommandText, _rs->GetActiveConnection(), _rs->CursorType, _rs->LockType, adCmdText);

	rs->Move(_rs->GetAbsolutePosition() - 1);

	return rs;
}

bool Records::Find(string filter)
{
	if (IsNull())
		return false;

	MoveFirst();

	HRESULT hr = _rs->Find((_bstr_t)filter.c_str(), 0, adSearchForward);
	return SUCCEEDED(hr);
}

//////////////////////////////////////////////////////////////////////////

Database::Database() : _con(NULL), _cmd(NULL)
{
	CoInitialize(NULL);
}

Database::~Database()
{
	CoUninitialize();
}

void Database::Open(string id, string password, string database, string ip, int port)
{
	try
	{
		if (!_con)
		{
			_con.CreateInstance(__uuidof(Connection));

			char connection[1024];
			sprintf_s(connection, sizeof(connection), "Provider='sqloledb';User ID=%s;Password=%s;Data Source=%s;"
				"Initial Catalog=%s;Network Address=%s,%d;Network Library=dbmssocn",
				id.c_str(), password.c_str(), ip.c_str(), database.c_str(), ip.c_str(), port);
			_con->Open(connection, "", "", adConnectUnspecified);

			_cmd.CreateInstance(__uuidof(Command));
			_cmd->ActiveConnection = _con;
			_cmd->CommandType = adCmdText;
		}
	}
	catch (_com_error& e)
	{
		REPORT_COM_ERROR(e);
	}
}

void Database::Close()
{
	if (_con)
	{
		if (_con->State == adStateOpen)
		{
			_con->Close();
		}

		_con.Release();
		_con = NULL;

		_cmd.Release();
		_cmd = NULL;
	}
}

Records Database::Select(const char* format, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf_s(buf, sizeof(buf), format, args);
	va_end(args);
	return Query(buf);
}

int Database::Insert(const char* format, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf_s(buf, sizeof(buf), format, args);
	va_end(args);
	return Execute(buf);
}

int Database::Update(const char* format, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf_s(buf, sizeof(buf), format, args);
	va_end(args);
	return Execute(buf);
}

int Database::Delete(const char* format, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf_s(buf, sizeof(buf), format, args);
	va_end(args);
	return Execute(buf);
}

Records Database::Query(const char* sql, int cursor, int lock)
{
	_RecordsetPtr rs;
	rs.CreateInstance(__uuidof(Recordset));

	try
	{
		rs->CursorLocation = adUseClient;
		rs->Open(sql, _con.GetInterfacePtr(), (CursorTypeEnum)cursor, (LockTypeEnum)lock, adCmdText);

		if (rs->State == adStateClosed)
			return NULL;
		if (rs->RecordCount == 0)
			return NULL;
	}
	catch (_com_error& e)
	{
		REPORT_COM_ERROR(e);
	}

	return rs;
}

int Database::Execute(const char* sql)
{
	_variant_t affected;

	try
	{
		_bstr_t cmd = sql;
		_cmd->CommandText = cmd;
		_cmd->Execute(&affected, NULL, adCmdText);
	}
	catch (_com_error& e)
	{
		REPORT_COM_ERROR(e);
		return -1;
	}

	return affected.lVal;
}
