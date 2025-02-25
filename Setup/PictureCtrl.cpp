///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////
// PictureCtrl.cpp
//
// Author: Tobias Eiseler
//
// E-Mail: tobias.eiseler@sisternicky.com
//
// Function: A MFC Picture Control to display
//           an image on a Dialog, etc.
///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PictureCtrl.h"
//#include <GdiPlus.h>

//using namespace Gdiplus;
//
//#pragma comment(lib, "GdiPlus.lib")
#include "resource.h"
//Macro to release COM Components

#ifdef SAFE_RELEASE
#undef SAFE_RELEASE
#endif
#define SAFE_RELEASE(x) do{\
							if((x) != NULL)\
							{\
								while((x)->Release() != 0);\
								(x) = NULL;\
							}\
						}while(0)

CPictureCtrl::CPictureCtrl(void)
	:CStatic()
	, m_bIsPicLoaded(FALSE)

{
}

CPictureCtrl::~CPictureCtrl(void)
{
	//Tidy up
	FreeData();
}

bool CPictureCtrl::LoadImageFromResource(IN CImage* pImage, IN UINT nResID, IN LPCTSTR lpTyp)
{
	SetLastError(ERROR_SUCCESS);

	if (m_bIsPicLoaded)
		FreeData();

	// ������Դ
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	// ������Դ
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	// �����ڴ��е�ָ����Դ
	LPVOID lpVoid = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	// ����ڴ��е�ָ����Դ
	::GlobalUnlock(hNew);

	// ��ָ���ڴ洴��������
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		// ����ͼƬ
		pImage->Load(pStream);
		m_bIsPicLoaded = TRUE;
		GlobalFree(hNew);
	}

	// �ͷ���Դ
	::FreeResource(hImgData);

	return true;
}

bool CPictureCtrl::LoadImageFromResource(/*IN CImage* pImage,*/ IN UINT nResID, IN LPCTSTR lpTyp)
{
	SetLastError(ERROR_SUCCESS);
	if (m_bIsPicLoaded)
		FreeData();

	// ������Դ
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	// ������Դ
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	// �����ڴ��е�ָ����Դ
	LPVOID lpVoid = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	// ����ڴ��е�ָ����Դ
	::GlobalUnlock(hNew);

	// ��ָ���ڴ洴��������
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		// ����ͼƬ
		m_image.Load(pStream);
		m_bIsPicLoaded = TRUE;
		GlobalFree(hNew);
	}

	// �ͷ���Դ
	::FreeResource(hImgData);

	return true;
}
BOOL CPictureCtrl::Load(CString& szFilePath)
{
	//Set success error state
	SetLastError(ERROR_SUCCESS);
	if (m_bIsPicLoaded)
		FreeData();
	//Open the specified file

	m_image.Load(szFilePath);

	m_bIsPicLoaded = TRUE;

	Invalidate();
	RedrawWindow();

	return TRUE;
}

BOOL CPictureCtrl::Load(HMODULE hModule, UINT resourceID, CString lpTyp /*= _T("")*/)
{
	if (m_bIsPicLoaded)
		FreeData();

	//�ж���BMP����������ʽ
	if (lpTyp == _T(""))
	{
		//BMP��ʽ
		m_image.LoadFromResource(hModule, resourceID);//������ʾͼƬ
	}
	else
	{
		//������ʽ
		//LoadImageFromResource(/*&m_image, */resourceID, lpTyp);
		LoadImageFromResource(&m_image, resourceID, lpTyp);
	}

	//Mark as loaded

	m_bIsPicLoaded = TRUE;

	Invalidate();
	RedrawWindow();

	return TRUE;
}

void CPictureCtrl::FreeData()
{
	if (m_image)
	{
		HBITMAP hbitmap = m_image.Detach();
		DeleteObject(hbitmap);
		m_image.Destroy();
	}

	m_bIsPicLoaded = FALSE;
}

void CPictureCtrl::PreSubclassWindow()
{
	CStatic::PreSubclassWindow();
	ModifyStyle(0, SS_OWNERDRAW);
}

void CPictureCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	//Check if pic data is loaded
	if (m_bIsPicLoaded)
	{
		//Get control measures
		/*RECT rc;
		this->GetClientRect(&rc);

		m_image.Draw(lpDrawItemStruct->hDC, rc);*/

		//ʹ��GDI��ͼ
		if (m_image.GetBPP() == 32) //ȷ�ϸ�ͼ�����Alphaͨ��
		{
			int i; int j;
			for (i = 0; i < m_image.GetWidth(); i++)
			{
				for (j = 0; j < m_image.GetHeight(); j++)
				{
					byte* pByte = (byte*)m_image.GetPixelAddress(i, j);
					pByte[0] = pByte[0] * pByte[3] / 255;
					pByte[1] = pByte[1] * pByte[3] / 255;
					pByte[2] = pByte[2] * pByte[3] / 255;
				}
			}
		}

		RECT rc;
		this->GetClientRect(&rc);
		m_image.Draw(lpDrawItemStruct->hDC, rc);
	}
}

BOOL CPictureCtrl::OnEraseBkgnd(CDC* pDC)
{
	if (m_bIsPicLoaded)
	{
		//Get control measures
		/*RECT rc;
		this->GetClientRect(&rc);
		m_image.Draw(pDC->m_hDC, rc);*/

		if (m_image.GetBPP() == 32) //ȷ�ϸ�ͼ�����Alphaͨ��
		{
			int i; int j;
			for (i = 0; i < m_image.GetWidth(); i++)
			{
				for (j = 0; j < m_image.GetHeight(); j++)
				{
					byte* pByte = (byte*)m_image.GetPixelAddress(i, j);
					pByte[0] = pByte[0] * pByte[3] / 255;
					pByte[1] = pByte[1] * pByte[3] / 255;
					pByte[2] = pByte[2] * pByte[3] / 255;
				}
			}
		}

		RECT rc;
		this->GetClientRect(&rc);
		m_image.Draw(pDC->m_hDC, rc);

		return TRUE;
	}
	else
	{
		return CStatic::OnEraseBkgnd(pDC);
	}
}

BEGIN_MESSAGE_MAP(CPictureCtrl, CStatic)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CPictureCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	HCURSOR hCur = LoadCursor(NULL, IDC_HAND);
	::SetCursor(hCur);
	CStatic::OnMouseMove(nFlags, point);
}