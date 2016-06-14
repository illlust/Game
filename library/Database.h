#pragma once

//#import "C:\Program Files (x86)\Common Files\System\ado\msado28.tlb"
//#import msado28.tlb
#import	"C:\Program Files (x86)\Common Files\System\ado\msado28.tlb" \
	rename("EOF", "ADOCGEOF") \
	rename_namespace("ADOCG")
//using namespace ADOCG;

#import "C:\Program Files (x86)\Common Files\System\ado\msado15.dll" \
	rename("EOF", "ADOEOF") \
	rename("BOF", "ADOBOF") \
	rename_namespace("ADO")
using namespace ADO;

#import "C:\Program Files (x86)\Common Files\System\ado\msjro.dll" \
	rename_namespace("JRO")
using namespace JRO;

#import "C:\Program Files (x86)\Common Files\System\ado\msadox.dll" \
	rename("LockTypeEnum", "XLockTypeEnum") \
	rename("DataTypeEnum", "XDataTypeEnum") \
	rename("FieldAttributeEnum", "XFieldAttributeEnum") \
	rename("EditModeEnum", "XEditModeEnum") \
	rename("RecordStatusEnum", "XRecordStatusEnum") \
	rename("ParameterDirectionEnum", "XParameterDirectionEnum") \
	rename_namespace("ADOX")
using namespace ADOX;

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define REPORT_COM_ERROR(x) ReportError(x);
#else
#define REPORT_COM_ERROR(x) __noop(x); x;
#endif

//////////////////////////////////////////////////////////////////////////

#include <string>
#include <memory>

class Variant
{
public:
	Variant(FieldPtr field = NULL);
	Variant(const Variant& rhs);
	virtual ~Variant();

	operator void*();

	void GetValue(int* value);
	void GetValue(float* value);
	void GetValue(double* value);
	void GetValue(std::string* value);
	void GetValue(bool* value);
	void GetValue(tm* value);

	template<class T>
	operator T()
	{
		T value;
		GetValue(&value);
		return value;
	}

	Variant& operator =(const Variant& rhs);

	template<typename T>
	Variant& operator =(T value)
	{
		_field->Value = value;
		return *this;
	}

	Variant& operator =(int value);
	Variant& operator =(std::string value);

	bool IsNull();

private:
	FieldPtr _field;
};

//////////////////////////////////////////////////////////////////////////

class Records
{
public:
	template<class T>
	void GetFieldValue(std::string index, T* value)
	{
		if (IsNull())
			return;

		try
		{
			Variant variant = _rs->Fields->GetItem((_variant_t)index.c_str());
			variant.GetValue(value);
		}
		catch (_com_error&)
		{
		}
	}

	Records(_RecordsetPtr rs = NULL);
	Records(const Records& rhs);
	virtual ~Records();

	Records& operator =(const Records& rhs);
	operator void*(); // _RecordsetPtr 이 NULL 이거나 BOF, EOF 일 때 포인터처럼 널 체크를 가능하도록 한다
	Records& operator ++();
	Records& operator --();
	const Records operator ++(int);
	const Records operator --(int);
	Variant operator [](std::string index); // 테이블 필드명을 인덱스로 값을 얻을 수 있다
	Variant operator [](int index);
	//	_Recordset* operator->();

public:
	long GetFieldType(int index);
	std::string GetTableName(int index);
	std::string GetFieldDescription(int index, _CatalogPtr catalog = NULL);
	std::string GetFieldName(int index);
	int GetFieldCount();
	int GetRecordCount();
	void MoveFirst();
	void MoveLast();
	void MoveNext();
	void MovePrev();
	void Move(int offset);
	void MoveAbsolutePosition(int position);
	int GetAbsolutePosition();
	bool IsNull();
	Variant GetFieldValue(std::string index);
	Variant GetFieldValue(int index);
	std::string GetMergeFieldDescription(std::string header, std::string footer, _CatalogPtr catalog = NULL);
	std::string GetMergeFieldName(std::string header, std::string footer);
	std::string GetMergeFieldValue(std::string header, std::string footer, bool quote = true);
	std::string GetMergeFieldPair(std::string separator, std::string footer, std::string table = "");
	Records Clone(); // 레코드셋의 새로운 복사본을 생성한다
	bool Find(std::string filter);

protected:
	_RecordsetPtr _rs;
};

//////////////////////////////////////////////////////////////////////////

class Database
{
public:
	_ConnectionPtr _con;
	_CommandPtr _cmd;
	//_RecordsetPtr _rs;

public:
	Database();
	virtual ~Database();
	void Open(std::string id, std::string password, std::string database, std::string ip = "127.0.0.1", int port = 1433);
	void Close();
	Records Select(const char* format, ...);
	int Insert(const char* format, ...);
	int Update(const char* format, ...);
	int Delete(const char* format, ...);

public:
	Records Query(const char* sql, int cursor = adOpenKeyset, int lock = adLockOptimistic);
	int Execute(const char* sql);
};
