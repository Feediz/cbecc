/**********************************************************************
 *  Copyright (c) 2012-2017, California Energy Commission
 *  Copyright (c) 2012-2017, Wrightsoft Corporation
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  - Neither the name of the California Energy Commission nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *  DISCLAIMER: THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL CALIFORNIA ENERGY COMMISSION,
 *  WRIGHTSOFT CORPORATION, ITRON, INC. OR ANY OTHER AUTHOR OR COPYRIGHT HOLDER OF
 *  THIS SOFTWARE (COLLECTIVELY, THE "AUTHORS") BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 *  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  EACH LICENSEE AND SUBLICENSEE OF THE
 *  SOFTWARE AGREES NOT TO ASSERT ANY CLAIM AGAINST ANY OF THE AUTHORS RELATING TO
 *  THIS SOFTWARE, WHETHER DUE TO PERFORMANCE ISSUES, TITLE OR INFRINGEMENT ISSUES,
 *  STRICT LIABILITY OR OTHERWISE.
 **********************************************************************/

#if !defined(AFX_BEMProcDLGCREATE_H__82C122E1_90B9_11D1_9002_0080C732DDC2__INCLUDED_)
#define AFX_BEMProcDLGCREATE_H__82C122E1_90B9_11D1_9002_0080C732DDC2__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BEMProcDlgCreate.h : header file
//

#include "screens.h"
#include "BEMProcUI_Globals.h"
#include "BPUICtl_butn.h"
#include "BPUICtl_edit.h"
#include "BPUICtl_cmbb.h"

/////////////////////////////////////////////////////////////////////////////
// CBEMProcDlgCreate dialog

class CBEMProcDlgCreate : public CDialog
{
// Construction
public:
	CBEMProcDlgCreate( int i1Class, long lObjTypeDBID=0, long lObjTypeValue=0, CWnd* pParent = NULL, int iDlgMode=0 );

// Dialog Data
	//{{AFX_DATA(CBEMProcDlgCreate)
	enum { IDD = IDD_SACBLANKDLG };

   BOOL m_bCreateControls;

   int m_i1Class;
   CString m_sLongClassName;

   long m_lObjTypeDBID;
   long m_lObjTypeValue;

   CEditCtl*     m_pName;
   CComboBoxCtl* m_pParent;
   CComboBoxCtl* m_pCopy;

   CButtonCtl* m_pOK;
   CButtonCtl* m_pCancel;

   int  m_iDialogMode;  // enum SACWizDlgMode - SAC 6/12/00
   BEMObject*  m_pNewObj;  // SAC 6/12/00

		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBEMProcDlgCreate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBEMProcDlgCreate)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
   afx_msg void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BEMProcDLGCREATE_H__82C122E1_90B9_11D1_9002_0080C732DDC2__INCLUDED_)
