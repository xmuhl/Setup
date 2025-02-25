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

	// 查找资源
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	// 加载资源
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	// 锁定内存中的指定资源
	LPVOID lpVoid = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	// 解除内存中的指定资源
	::GlobalUnlock(hNew);

	// 从指定内存创建流对象
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		// 加载图片
		pImage->Load(pStream);
		m_bIsPicLoaded = TRUE;
		GlobalFree(hNew);
	}

	// 释放资源
	::FreeResource(hImgData);

	return true;
}

bool CPictureCtrl::LoadImageFromResource(/*IN CImage* pImage,*/ IN UINT nResID, IN LPCTSTR lpTyp)
{
	SetLastError(ERROR_SUCCESS);
	if (m_bIsPicLoaded)
		FreeData();

	// 查找资源
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	// 加载资源
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	// 锁定内存中的指定资源
	LPVOID lpVoid = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	// 解除内存中的指定资源
	::GlobalUnlock(hNew);

	// 从指定内存创建流对象
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		// 加载图片
		m_image.Load(pStream);
		m_bIsPicLoaded = TRUE;
		GlobalFree(hNew);
	}

	// 释放资源
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

	//判断是BMP还是其他格式
	if (lpTyp == _T(""))
	{
		//BMP格式
		m_image.LoadFromResource(hModule, resourceID);//加载提示图片
	}
	else
	{
		//其它格式
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

		//使用GDI画图
		if (m_image.GetBPP() == 32) //确认该图像包含Alpha通道
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

		if (m_image.GetBPP() == 32) //确认该图像包含Alpha通道
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
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	HCURSOR hCur = LoadCursor(NULL, IDC_HAND);
	::SetCursor(hCur);
	CStatic::OnMouseMove(nFlags, point);
}