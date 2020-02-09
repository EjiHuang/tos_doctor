#pragma once

#include <Windows.h>
#include <comutil.h>

namespace SafeArrayHelper 
{
	template <class T, VARTYPE v>
	void CreateSafeArrayEx
	(
		T* lpT,
		ULONG ulSize,
		PVOID pvExtraInfo,
		SAFEARRAY*& pSafeArrayReceiver
	)
	{
		HRESULT hrRetTemp = S_OK;
		SAFEARRAYBOUND rgsabound[1];
		ULONG ulIndex = 0;
		long lRet = 0;

		// Initialise receiver.
		pSafeArrayReceiver = NULL;

		if (lpT)
		{
			rgsabound[0].lLbound = 0;
			rgsabound[0].cElements = ulSize;

			pSafeArrayReceiver = (SAFEARRAY*)SafeArrayCreateEx
			(
				(VARTYPE)v,
				(unsigned int)1,
				(SAFEARRAYBOUND*)rgsabound,
				(PVOID)pvExtraInfo
			);
		}

		if (pSafeArrayReceiver == NULL)
		{
			// If not able to create SafeArray,
			// exit immediately.
			return;
		}

		for (ulIndex = 0; ulIndex < ulSize; ulIndex++)
		{
			long lIndexVector[1];

			lIndexVector[0] = ulIndex;

			SafeArrayPutElement
			(
				(SAFEARRAY*)pSafeArrayReceiver,
				(long*)lIndexVector,
				(void*)(&(lpT[ulIndex]))
			);
		}

		return;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="lpszTypeLibraryPath"></param>
	/// <param name="refguid"></param>
	/// <param name="ppIRecordInfoReceiver"></param>
	/// <returns></returns>
	HRESULT GetIRecordType
	(
		LPCTSTR lpszTypeLibraryPath,
		REFGUID refguid,
		IRecordInfo** ppIRecordInfoReceiver
	)
	{
		_bstr_t	bstTypeLibraryPath = lpszTypeLibraryPath;
		ITypeLib* pTypeLib = NULL;
		ITypeInfo* pTypeInfo = NULL;
		HRESULT hrRet = S_OK;

		*ppIRecordInfoReceiver = NULL;  // Initialize receiver.
		hrRet = LoadTypeLib((const OLECHAR FAR*)bstTypeLibraryPath, &pTypeLib);

		if (SUCCEEDED(hrRet))
		{
			if (pTypeLib)
			{
				hrRet = pTypeLib->GetTypeInfoOfGuid(refguid, &pTypeInfo);
				pTypeLib->Release();
				pTypeLib = NULL;
			}

			if (pTypeInfo)
			{
				hrRet = GetRecordInfoFromTypeInfo(pTypeInfo, ppIRecordInfoReceiver);
				pTypeInfo->Release();
				pTypeInfo = NULL;
			}
		}

		return hrRet;
	}
}