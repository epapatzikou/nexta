#ifndef RecordsetExt_H
#define RecordsetExt_H

#define MAX_FIELD 200

#include <afxdb.h>          // MFC database support
#include <afxdao.h>

class CRecordsetExt : public CDaoRecordset
{
public:
   CString GetCString(CString ColumnName);

   long GetLong(CString ColumnName,bool &bExist,bool bRequired);
   double GetDouble(CString ColumnName,bool &bExist,bool bRequired);
   float GetFloat(CString ColumnName);
   // Use with Column Numbers


   int m_nFields;


public:
   CRecordsetExt(CDaoDatabase* pDatabase = NULL);
   DECLARE_DYNAMIC(CRecordsetExt)

      // Field/Param Data
      //{{AFX_FIELD(CRecordsetExt, CRecordset)
      //}}AFX_FIELD

      // Overrides
      // ClassWizard generated virtual function overrides
      //{{AFX_VIRTUAL(CRecordsetExt)
      public:
      virtual CString GetDefaultDBName();		// Default database name
   virtual CString GetDefaultSQL();			// Default SQL for Recordset
   virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support
							//}}AFX_VIRTUAL

};

#endif
