// SmartPOS.h : Declaration of the CSmartPOS

#include "resource.h"       // main symbols

//#include "SmartPOSDemoCOM.h"
//#include "_ISmartPOSEvents_CP.h"
#include "Runtime/AccessManager.h"
#include "Runtime/UIControlImpl.h"
#include "Runtime/CryptoControlImpl.h"
#include "Runtime/SCRControlImpl.h"
#include "Runtime/SCRControlServer.h"
#include "Utility/utility.h"
#include "Runtime/ApplSelControlInterface.h"
#include "Runtime/POSControl.h"
#include "EMV_Library/Prompter.h"
#include "Runtime/clcVSDC_constants.h"
#include "EMV_Library/emv_constants.h"
#include "Utility/common_functions.h"
#include "Runtime/SCRInterface.h"
#include "Runtime/POSControlVSDCImpl.h"
#include "Runtime/ApplSelControlImpl.h"
#include "Runtime/stdafx.h"

#define ERR_COMPONENT_METHOD_IS_BUSY 0x88880001
#define ERR_THREAD_ALREADY_EXIST     0x88880002
#define ERR_TERMINAL_IS_NOT_CONNECTED 0x88880003

#define TERMINAL_WINDOW_NAME "SmartPOS Terminal Demo"

// CSmartPOS

/*
class CSmartPOS : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSmartPOS, &CLSID_SmartPOS>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CSmartPOS>,
	public CProxy_ISmartPOSEvents<CSmartPOS>, 
	public IDispatchImpl<ISmartPOS, &IID_ISmartPOS, &LIBID_SmartPOSDemoCOMLib, //wMajor = 
			1, //wMinor =
			 0>
{
public:
	CSmartPOS()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SMARTPOS)


BEGIN_COM_MAP(CSmartPOS)
	COM_INTERFACE_ENTRY(ISmartPOS)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CSmartPOS)
	CONNECTION_POINT_ENTRY(__uuidof(_ISmartPOSEvents))
END_CONNECTION_POINT_MAP()
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		bstrPaymentAmount = "0.00";
		bstrCashbackAmount = "0.00";
		bstrTransactionType = "Goods and Services (Purchase)";
		bstrCurrencyCode = "US Dollar";
		bstrErrSource = "";
		bstrErrDescription = "";
		POSControl = 0;
		return S_OK;
	}
	
	void FinalRelease() 
	{
		disconnectFromTerminal();
	}

public:

private:
	_bstr_t bstrPaymentAmount;
	_bstr_t bstrTransactionType;
	_bstr_t bstrCurrencyCode;
	_bstr_t bstrErrSource;
	_bstr_t bstrErrDescription;
	_bstr_t bstrCashbackAmount;

	//SCRControlInterface SCR;
	POSControlInterface *POSControl;

	LPOLESTR lpErrSource;
	LPOLESTR lpErrDescription;

public:

	STDMETHOD(get_PaymentAmount)(BSTR* pVal);
	STDMETHOD(put_PaymentAmount)(BSTR newVal);
	STDMETHOD(get_TransactionType)(BSTR* pVal);
	STDMETHOD(put_TransactionType)(BSTR newVal);
	STDMETHOD(get_CurrencyCode)(BSTR* pVal);
	STDMETHOD(put_CurrencyCode)(BSTR newVal);
	STDMETHOD(connect2Terminal)(LONG* lpResult);
	STDMETHOD(disconnectFromTerminal)(void);
	STDMETHOD(runTerminalAppl)(LONG* TransactionResult, 
							   LONG* lpResult);
	STDMETHOD(resetTransaction)(LONG* lpReturn);
	STDMETHOD(get_ErrSource)(BSTR* pVal);
	STDMETHOD(get_ErrDescription)(BSTR* pVal);
	STDMETHOD(getTransactionData)(LONG Tag, BSTR* Value, 
								  BSTR* Format, SHORT TransactionOnly,
								  LONG* lpReturn);

private:
	// Sets the source and description of error object
	void setErrorDescription(LPOLESTR source, LPOLESTR Description);
	int initErrDescription(const char *source, const char *description);

	int OpenServices();
	void CloseServices();
	int OpenServiceCntrl (ServiceControl *sc);
	char* createString(const char *str1, 
						const char *str2 = 0, 
						const char *str3 = 0,
						const char *str4 = 0);
	void ShutDownExitThread();
	
	int InitializeUI(UIControlInterface &UI);
	int ConfirmSelection(UIControlInterface &UI, int *btn);
	int execTransaction(long *TransResult);
	void releaseErrDesrip();
	void releasePOS (POSControlInterface **ppPOSControl);

public:
	static HANDLE evntExitThreadCreated;
	static HANDLE hMutex;
	static HANDLE hWaitEvent;
	static HANDLE hEvntCardIsPresent;
	static HANDLE hEvntTransactionInProgress;
	static HANDLE hEvntInsertEnabled;
	static HANDLE hEvntRemoveEnabled;
	static int InsertCounter;

	static ApplSelControlInterface Selector;
	static SCRControlInterface SCR;
	
	STDMETHOD(get_CashbackAmount)(BSTR* pVal);
	STDMETHOD(put_CashbackAmount)(BSTR newVal);
	STDMETHOD(checkCA_PKI_Integrity)(LONG* keysVerified, LONG* VerificationResult);
};

OBJECT_ENTRY_AUTO(__uuidof(SmartPOS), CSmartPOS)
*/
