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

#include "stdafx.h"
#include "BEMCmpMgr.h"
#include "BEMCM_I.h"
#include "..\BEMProc\BEMProc.h"
#include "..\BEMProc\BEMClass.h"
#include "exec-stream.h"
#include "CSERunMgr.h"
#include "memLkRpt.h"
#ifdef OSWRAPPER
#include "OpenStudioInterface.h"
#endif

using namespace std;
#include <string>
#include <vector>
#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf

const char* pszRunAbbrev_u  = "u";
const char* pszRunAbbrev_pp  = "pp";  
const char* pszRunAbbrev_ppN = "pp-N";
const char* pszRunAbbrev_ppE = "pp-E";
const char* pszRunAbbrev_ppS = "pp-S";
const char* pszRunAbbrev_ppW = "pp-W";
const char* pszRunAbbrev_smf  = "smf";  
const char* pszRunAbbrev_p  = "p";
const char* pszRunAbbrev_pN = "p-N";
const char* pszRunAbbrev_pE = "p-E";
const char* pszRunAbbrev_pS = "p-S";
const char* pszRunAbbrev_pW = "p-W";
const char* pszRunAbbrev_s    = "s";    
const char* pszRunAbbrev_pfx  = "pfx";  
const char* pszRunAbbrev_pfxN = "pfx-N";
const char* pszRunAbbrev_pfxE = "pfx-E";
const char* pszRunAbbrev_pfxS = "pfx-S";
const char* pszRunAbbrev_pfxW = "pfx-W";
const char* pszRunAbbrev_dr   = "dr";   


int StringInArray( QVector<QString>& saStrs, QString& sStr )
{	int iSize = (int) saStrs.size();
	for (int i=0; i<iSize; i++)
	{	if (saStrs[i].compare( sStr, Qt::CaseInsensitive ) == 0)
			return i;
	}
	return -1;
}

CSERun::CSERun() :
	m_lRunNumber( 0),
	m_bLastRun( FALSE),
	m_bIsStdDesign( FALSE),
	m_iExitCode( 0),
	m_pES( NULL)
{
	m_sTDVFName.clear();		// SAC 4/16/17
}

CSERun::~CSERun()
{
	delete m_pES;
}

CSERunMgr::CSERunMgr(
	QString sCSEexe,
	QString sCSEWthr,
	QString sModelPathOnly,
	QString sModelFileOnlyNoExt,
	QString sProcessPath,
	bool bFullComplianceAnalysis,
	bool bInitHourlyResults,
	long lAllOrientations,
	long lAnalysisType,
	long lStdDesignBaseID,
	long lDesignRatingRunID,
	bool bVerbose,
	bool bStoreBEMProcDetails,
	bool bPerformSimulations,
	bool bBypassCSE,
	bool bSilent,
	void* pCompRuleDebugInfo,
	const char* pszUIVersionString,
	int iSimReportOpt,
	int iSimErrorOpt,
	long lStdMixedFuelRunReqd,
	long lPrelimPropRunReqd,
	long lPropFlexRunReqd,
	int iNumRuns,
	const char* pszCodeYear2Digit,
	std::vector<long>* plaRIBDIClsObjIndices) :

	m_sCSEexe( sCSEexe),
	m_sCSEWthr( sCSEWthr),
	m_sModelPathOnly( sModelPathOnly),
	m_sModelFileOnlyNoExt( sModelFileOnlyNoExt),
	m_sProcessPath( sProcessPath),
	m_bFullComplianceAnalysis( bFullComplianceAnalysis),
	m_bInitHourlyResults( bInitHourlyResults),
	m_lAllOrientations( lAllOrientations),
	m_lAnalysisType( lAnalysisType),
	m_lStdDesignBaseID( lStdDesignBaseID),
	m_lDesignRatingRunID( lDesignRatingRunID),
	m_bVerbose( bVerbose),
	m_bStoreBEMProcDetails( bStoreBEMProcDetails),
	m_bPerformSimulations( bPerformSimulations),
	m_bBypassCSE( bBypassCSE),
	m_bSilent( bSilent),
	m_pCompRuleDebugInfo( pCompRuleDebugInfo),
	m_pszUIVersionString( pszUIVersionString),
	m_iError( 0),
	m_iSimReportOpt(iSimReportOpt),
	m_iSimErrorOpt(iSimErrorOpt),
	m_lStdMixedFuelRunReqd(lStdMixedFuelRunReqd),
	m_lPrelimPropRunReqd(lPrelimPropRunReqd),
	m_lPropFlexRunReqd(lPropFlexRunReqd),
	m_iNumOpenGLErrors(0),
	m_iNumProgressRuns(-1),
	m_sCodeYear2Digit(pszCodeYear2Digit)
{
	m_plaRIBDIClsObjIndices = plaRIBDIClsObjIndices;
	if (iNumRuns > 0)
		m_iNumRuns = iNumRuns;
	else
	{	m_iNumRuns = (bFullComplianceAnalysis ? (lAllOrientations > 0 ? 5 : 2) : 1);
		if (m_lPropFlexRunReqd > 0)		// SAC 8/3/17
			m_iNumRuns++;
		if (m_lPrelimPropRunReqd > 0)		// SAC 12/29/17
			m_iNumRuns += (lAllOrientations > 0 ? 4 : 1);
		if (lAnalysisType > 0 /*bFullComplianceAnalysis*/ && m_lDesignRatingRunID > 0)		// SAC 3/27/15
			m_iNumRuns++;
		if (m_lStdMixedFuelRunReqd > 0)		// SAC 4/5/17
			m_iNumRuns++;  //+= (lAllOrientations > 0 ? 4 : 1);
	}
}		// CSERunMgr::CSERunMgr

CSERunMgr::~CSERunMgr()
{	DeleteRuns();
}

void CSERunMgr::DeleteRuns()
{	CSERun* pCSERun = NULL;
	while (!m_vCSERun.empty())
	{	pCSERun = m_vCSERun.back();
		m_vCSERun.pop_back();
		delete pCSERun;
	}
}

static int ExecuteNow( CSERunMgr* pRunMgr, QString sEXEFN, QString sEXEParams )
{
			int iExitCode = -99;
			bool bRunOK = true;
			exec_stream_t* pES=NULL;
			try
			{	
				pES = new exec_stream_t( sEXEFN.toLocal8Bit().constData(), sEXEParams.toLocal8Bit().constData(), CREATE_NO_WINDOW );
				//cseRun.SetExecStream( pES);
			}
			catch(exec_stream_t::error_t &e)
			{	std::string sLogMsg=e.what();
				BEMPX_WriteLogFile( sLogMsg.c_str(), NULL, FALSE, TRUE, FALSE );
				bRunOK = false;
			}
			if (pES && bRunOK)
			{
				bool bEXEDone=false, bFirstException=true;
				while (!bEXEDone)
				{	try
					{	if (pES->running())
						{	pRunMgr->ProcessRunOutput( pES, 0/*iRun*/, bFirstException);
							Sleep(100);
						}
						else
						{	bEXEDone = true;
							bool bFirstExceptionX=true;
							while( pRunMgr->ProcessRunOutput( pES, 0/*iRun*/, bFirstExceptionX));
							pES->close();
							iExitCode = pES->exit_code();
						}
					}
		//			{	if (pES->running())
		//				{	//ProcessRunOutput( pES, iRun, bFirstException);
		//					Sleep(100);
		//				}
		//				else
		//				{	bEXEDone = true;
		//					//while( ProcessRunOutput( pES, iRun, bFirstExceptionX));
		//					pES->close();
		//					iExitCode = pES->exit_code();
		//				}
		//			}
					catch(exec_stream_t::error_t &e2)
					{	if (bFirstException)
						{	std::string sLogMsg=e2.what();
							BEMPX_WriteLogFile( sLogMsg.c_str(), NULL, FALSE, TRUE, FALSE );
							bFirstException = false;
						}
				}	}
			}
			if (pES)
				delete pES;
					//      QString sMsg = QString( "BTPreRun CSE exit code: %1" ).arg( QString::number(iExitCode) );
					//      BEMMessageBox( sMsg );

	return iExitCode;
}


int CSERunMgr::SetupRun(
	int iRunIdx, int iRunType, QString& sErrorMsg, bool bAllowReportIncludeFile /*=true*/,		// SAC 4/29/15 - add argument to DISABLE report include files
	const char* pszRunAbbrev /*=NULL*/ )
{
	int iRetVal = 0;
	CSERun* pCSERun = new CSERun;
	m_vCSERun.push_back( pCSERun);
	QString sMsg, sLogMsg;
	BOOL bLastRun = (iRunIdx == (m_iNumRuns-1));
	BOOL bIsStdDesign = (iRunType == CRM_StdDesign);	// SAC 3/27/15 - was:  (iRunIdx > 0 && bLastRun);  // SAC 7/3/13 - consolidated some logic to identify 
	pCSERun->SetRunType( iRunType);							// SAC 3/27/15
	pCSERun->SetLastRun( bLastRun);
	pCSERun->SetIsStdDesign( bIsStdDesign);
	if (m_bStoreBEMProcDetails)	// SAC 1/20/13 - added export of additional "detail" file to help isolate unnecessary object creation issues
	{	QString sDbgFileName;
		//sDbgFileName = QString( "%1%2 - run %3.ibd-b4Evals" ).arg( m_sProcessPath, m_sModelFileOnlyNoExt, QString::number(iRunIdx+1) );
		sDbgFileName = QString( "%1%2 - %3.ibd-b4Evals" ).arg( m_sProcessPath, m_sModelFileOnlyNoExt, pszRunAbbrev );
		BEMPX_WriteProjectFile( sDbgFileName.toLocal8Bit().constData(), BEMFM_DETAIL /*FALSE*/ );
	}

	long lRunNumber = (iRunType == CRM_User ? 0 : iRunIdx+1 );		// SAC 4/21/15 - mods to set RunNumber in source code - necessary since we are now starting EACH run's BEMComp database w/ the Proposed Model's
	BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:RunNumber" ), BEMP_Int, (void*) &lRunNumber );
	if (pszRunAbbrev && strlen( pszRunAbbrev ) > 0)
		BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:RunAbbrev" ), BEMP_Str, (void*) pszRunAbbrev );

	QString sOrientLtr, sOrientName;
	if (iRunType < CRM_StdDesign /*!bIsStdDesign*/ && iRunType != CRM_StdMixedFuel && m_lAnalysisType > 0)
	{	if (m_lPrelimPropRunReqd && iRunType >= CRM_Prop)
			iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalPrelimRunError, "ApplyPrelimRunResults", m_bVerbose, m_pCompRuleDebugInfo );
		if (iRetVal == 0)
			iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalPropCompError, "ProposedCompliance", m_bVerbose, m_pCompRuleDebugInfo );
	}
	else if (iRunType >= CRM_PropFlex && iRunType <= CRM_WPropFlex)
		iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalSetupPFlxError, "SetupRun_ProposedFlexibility", m_bVerbose, m_pCompRuleDebugInfo );	// SAC 8/3/17
	else if (iRunType >= CRM_StdDesign || iRunType == CRM_StdMixedFuel)	// SAC 3/27/15 - was:  bIsStdDesign)
	{	// SAC 3/27/15 - SET 
		if (iRunType == CRM_DesignRating)
      	BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:StdDesignBase" ), BEMP_Int, (void*) &m_lDesignRatingRunID );
		// may at some point need to specifically set  StandardsVersion  if/when multiple standard vintages is supported by a single ruleset

		if (iRunType == CRM_StdMixedFuel && iRunIdx == 0)	// SAC 1/17/18 - eval 'ProposedCompliance' rules prior to BudgetConversion for StdMixedFuel run if no PrelimProp run being simulated
		{	iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalPropCompError, "ProposedCompliance", m_bVerbose, m_pCompRuleDebugInfo );
			if (iRetVal == 0)
				iRetVal = LocalEvaluateRuleset(	sErrorMsg, BEMAnal_CECRes_EvalPostPropError, "PostProposedInput", m_bVerbose, m_pCompRuleDebugInfo );		// setup for Proposed run
		}

		if (iRetVal == 0)
			iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalStdConvError, "BudgetConversion", m_bVerbose, m_pCompRuleDebugInfo );

		if (iRetVal == 0 && iRunType == CRM_StdMixedFuel)	//  SAC 1/16/18 - setup for StdMixedFuel run
			iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalSetupSMFError, "SetupRun_StandardMixedFuel", m_bVerbose, m_pCompRuleDebugInfo );	// SAC 4/5/17
	}
	if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
		iRetVal = BEMAnal_CECRes_RuleProcAbort;

//	long lRunNumber;
	QString sRunID, sRunIDProcFile, sRunAbbrev;
//	BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:RunNumber"     ),  lRunNumber     );
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunID"         ),  sRunID         );
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunIDProcFile" ),  sRunIDProcFile );
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunAbbrev"     ),  sRunAbbrev     );

	pCSERun->SetRunNumber( lRunNumber);
	pCSERun->SetRunID( sRunID);
	pCSERun->SetRunIDProcFile( sRunIDProcFile);
	pCSERun->SetRunAbbrev( sRunAbbrev);

	if (iRetVal == 0 && ( (iRunType >= CRM_NOrientPreProp && iRunType <= CRM_WOrientPreProp) ||
								 (iRunType >= CRM_NOrientProp    && iRunType <= CRM_WOrientProp   ) ||
								 (iRunType >= CRM_NPropFlex      && iRunType <= CRM_WPropFlex     ) ))		// SAC 3/27/15 - was:  m_bFullComplianceAnalysis && m_lAllOrientations > 0 && !bLastRun)
	{  // Set Orientation for this particular run
		BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:OrientAbbrev" ),  sOrientLtr  );
		BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:OrientName"   ),  sOrientName );
	}

	QString sMissing;
	if (sRunID.isEmpty())
		sMissing += "RunID, ";
	if (sRunAbbrev.isEmpty())
		sMissing += "RunAbbrev ";
	if (iRetVal == 0 && !sMissing.isEmpty())
	{	sMissing = sMissing.left( sMissing.length()-2 );
		sErrorMsg = "ERROR:  The following Proj properties not set:" + sMissing;
		iRetVal = BEMAnal_CECRes_GetReqdDataError;
	}
	assert( iRunIdx == (lRunNumber-1) );

	if (iRetVal == 0 && (m_sProcessPath.length() + m_sModelFileOnlyNoExt.length() + sRunIDProcFile.length() + 12) > _MAX_PATH)
	{	sErrorMsg = QString( "ERROR:  CSE processing path too long:  %1%2%3.*" ).arg( m_sProcessPath, m_sModelFileOnlyNoExt, sRunIDProcFile );
		iRetVal = BEMAnal_CECRes_ProcPathTooLong;
	}

	if (iRetVal == 0 && iRunIdx == 0 && m_bInitHourlyResults)  // only initialize hourly results for first simulation??
		BEMPX_InitializeHourlyResults();

	QString sProjFileAlone = m_sModelFileOnlyNoExt + sRunIDProcFile;
	if (iRetVal == 0)
	{	// try evaluating prop & postprop rulelists EVERY time
		iRetVal = LocalEvaluateRuleset(		sErrorMsg, BEMAnal_CECRes_EvalPropInp3Error, "ProposedInput", m_bVerbose, m_pCompRuleDebugInfo );		// generic project defaulting
		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
			iRetVal = BEMAnal_CECRes_RuleProcAbort;

		// ??? perform simulation check on BUDGET model as well ???
		if (iRetVal == 0) // && iRunIdx == 0)
			iRetVal = LocalEvaluateRuleset(	sErrorMsg, BEMAnal_CECRes_EvalSimChkError, "ProposedModelSimulationCheck", m_bVerbose, m_pCompRuleDebugInfo );		// check user input model for simulation-related errors
		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
			iRetVal = BEMAnal_CECRes_RuleProcAbort;

		if (iRetVal == 0)
			iRetVal = LocalEvaluateRuleset(	sErrorMsg, BEMAnal_CECRes_EvalPostPropError, "PostProposedInput", m_bVerbose, m_pCompRuleDebugInfo );		// setup for Proposed run
		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
			iRetVal = BEMAnal_CECRes_RuleProcAbort;

		if (iRetVal == 0)
			iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalSimPrepError, "CSE_SimulationPrep", m_bVerbose, m_pCompRuleDebugInfo );
		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
			iRetVal = BEMAnal_CECRes_RuleProcAbort;

		BEMPX_RefreshLogFile();	// SAC 5/19/14
		
		if (iRetVal == 0 && iRunIdx == 0)  // Store various software & ruleset versions prior to first run - SAC 12/19/12
		{	// SAC 4/24/15 - now done PRIOR to CSE run setup
			//if (m_pszUIVersionString && strlen( m_pszUIVersionString ) > 0)
			//	BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:SoftwareVersion" ), BEMP_Str, (void*) m_pszUIVersionString );
			//else
			//	BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:SoftwareVersion" ), m_iError );

			QString sVerTemp, sVerStr;
			BEMPX_GetRulesetID( sVerTemp, sVerStr );
			if (!sVerStr.isEmpty())
				BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:RulesetVersion" ), BEMP_QStr, (void*) &sVerStr );
			else
				BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:RulesetVersion" ), m_iError );

			QString sCSEVersion = GetVersionInfo();
			if (!sCSEVersion.isEmpty())
				BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:PrimSimEngingVer" ), BEMP_QStr, (void*) &sCSEVersion );
			else
				BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:PrimSimEngingVer" ), m_iError );

			// SAC 1/8/13 - DHW engine version stored to BEMBase DURING DHW simulation (immediately following the ...Init() call)
			BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:SecSimEngingVer" ), m_iError );
		}

		// Purge user-defined non-parent/child components which are not referenced  - SAC 1/20/13 - added to prevent simulation of objects that are not referenced in the building model
		// Purge performed HERE, before objects are cross-referenced (Cons <-> cseCONS & Mat <-> cseMATERIAL), w/ those cross-references preventing any useful purging
		BEMPX_PurgeUnreferencedComponents();

		QString sOutFiles[CSERun::OutFileCOUNT];
		if (iRetVal == 0)
		{	sOutFiles[CSERun::OutFileCSV] = m_sProcessPath + sProjFileAlone + ".csv";
			sOutFiles[CSERun::OutFileREP] = m_sProcessPath + sProjFileAlone + ".rep";
			sOutFiles[CSERun::OutFileERR] = m_sProcessPath + sProjFileAlone + ".err";
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileCSV], CSERun::OutFileCSV);
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileREP], CSERun::OutFileREP);
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileERR], CSERun::OutFileERR);

			static const char* pszOutFileDescs[] = { "CSV output", "REP output", "Error output" };
			int i=0;
			for (; (i<CSERun::OutFileCOUNT && iRetVal == 0); i++)
			{	sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
							"application before an updated file can be written.\n\nSelect 'Retry' to proceed "
								 "(once the file is closed), or \n'Abort' to abort the analysis." ).arg( pszOutFileDescs[i], sOutFiles[i] );
				if (!OKToWriteOrDeleteFile( sOutFiles[i].toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( pszOutFileDescs[i], sOutFiles[i] );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( pszOutFileDescs[i], sOutFiles[i] );
					iRetVal = BEMAnal_CECRes_SimOutWriteError;
				}
				// code to DELETE existing results files prior to simulation
				else if (FileExists( sOutFiles[i].toLocal8Bit().constData() ))
					DeleteFile( sOutFiles[i].toLocal8Bit().constData() );
			}
		}
		
		QString sRptIncFile, sProcRptIncFile, sZoneIncFile, sProcZoneIncFile;		QVector<QString> saZoneIncFiles, saProcZoneIncFiles, saZoneNameIncFiles;
		if (iRetVal == 0)
		{	// Write Report Include file statement
			long lProjReportIncludeFileDBID = BEMPX_GetDatabaseID( "Proj:ReportIncludeFile" );
			if (lProjReportIncludeFileDBID > 0)
				BEMPX_GetString( lProjReportIncludeFileDBID, sRptIncFile );
			if (bAllowReportIncludeFile && !sRptIncFile.isEmpty())
			{	QString sOrigRptIncFile = sRptIncFile;
				BOOL bOrigRptIncFileLastSlashIdx = sRptIncFile.lastIndexOf('\\');
				if (sRptIncFile.indexOf(':') < 0 && sRptIncFile.indexOf('\\') != 0)
				{	// report file does not include a FULL path, so expected to be a path relative to the project file - so prepend project path...
					sRptIncFile = m_sModelPathOnly + sRptIncFile;
					if (!FileExists( sRptIncFile.toLocal8Bit().constData() ))
					{	// if rpt incl file not found in model path, it might have been located elsewhere and already copied into processing directory - SAC 10/27/17
						QString sChkRptIncFile = m_sProcessPath + sOrigRptIncFile;
						if (FileExists( sChkRptIncFile.toLocal8Bit().constData() ))
							sRptIncFile = sChkRptIncFile;
				}	}
				if (!FileExists( sRptIncFile.toLocal8Bit().constData() ))
				{	sLogMsg = QString( "CSE report include file not found '%1'" ).arg( sRptIncFile );
					BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
					//WriteToLogFile( WM_LOGUPDATED, sRptFileErr );
					BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
					sRptIncFile.clear();
				}
				else
				{	QString sRptIncFileNoPath = sRptIncFile.right( sRptIncFile.length() - sRptIncFile.lastIndexOf('\\') - 1 );
					if (bOrigRptIncFileLastSlashIdx >= 0)
						// need to CHANGE report include file setting to reference only the filename (no path info)
						BEMPX_SetBEMData( lProjReportIncludeFileDBID, BEMP_QStr, (void*) &sRptIncFileNoPath );
					sProcRptIncFile = m_sProcessPath + sRptIncFileNoPath;
				}
				if (!sRptIncFile.isEmpty() && sRptIncFile.compare( sProcRptIncFile, Qt::CaseInsensitive ) == 0)
				{	// if we have a valid report include file but both the source and destination path\filenames are identical, then empty them, as no copying is required
					sRptIncFile.clear();
					sProcRptIncFile.clear();
				}
			}
			else if (!bAllowReportIncludeFile && !sRptIncFile.isEmpty())
			{	// SAC 9/4/17 - force re-default of Proj:ReportIncludeFile if it is specified but this run should not have it
				BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
				sRptIncFile.clear();
			}

			// SAC 7/2/15 - added logic here to check for and address usage of Zone:TstatIncludeFile
			long lUseTstatIncFile, lDBID_Zone_UseTstatIncFile = BEMPX_GetDatabaseID( "Zone:UseTstatIncFile" ), lDBID_Zone_TstatIncludeFile = BEMPX_GetDatabaseID( "Zone:TstatIncludeFile" );
			int iNumZones = (lDBID_Zone_UseTstatIncFile < 1 ? 0 : BEMPX_GetNumObjects( BEMPX_GetClassID( lDBID_Zone_UseTstatIncFile ) ));
			for (int iZnIdx=0; (iRetVal == 0 && iZnIdx < iNumZones); iZnIdx++)
			{	if (BEMPX_GetInteger( lDBID_Zone_UseTstatIncFile,  lUseTstatIncFile,    0, -1, iZnIdx ) && lUseTstatIncFile > 0 &&
					 BEMPX_GetString(  lDBID_Zone_TstatIncludeFile, sZoneIncFile, FALSE, 0, -1, iZnIdx ) && !sZoneIncFile.isEmpty() && StringInArray( saZoneIncFiles, sZoneIncFile ) < 0)
				{	//BOOL bOrigZoneIncFileLastSlashIdx = sZoneIncFile.lastIndexOf('\\');
					if (sZoneIncFile.indexOf(':') < 0 && sZoneIncFile.indexOf('\\') != 0)
						// include file does not include a FULL path, so expected to be a path relative to the project file - so prepend project path...
						sZoneIncFile = m_sModelPathOnly + sZoneIncFile;
					if (!FileExists( sZoneIncFile.toLocal8Bit().constData() ))
					{	if (!BEMPX_GetString( BEMPX_GetDatabaseID( "Zone:Name" ), sProcZoneIncFile, FALSE, 0, -1, iZnIdx ) || sProcZoneIncFile.isEmpty())
					 		sProcZoneIncFile = "<unknown>";
						sLogMsg = QString( "ERROR:  CSE zone '%1' include file not found '%2'" ).arg( sProcZoneIncFile, sZoneIncFile );
						BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
						iRetVal = BEMAnal_CECRes_MissingZnCSEIncFile;
						//BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
						sZoneIncFile.clear();
					}
					else
					{	QString sZoneIncFileNoPath = sZoneIncFile.right( sZoneIncFile.length() - sZoneIncFile.lastIndexOf('\\') - 1 );
					//	if (bOrigZoneIncFileLastSlashIdx >= 0)
					//		// need to CHANGE report include file setting to reference only the filename (no path info)
					//		BEMPX_SetBEMData( lProjReportIncludeFileDBID, BEMP_QStr, (void*) &sZoneIncFileNoPath );
						sProcZoneIncFile = m_sProcessPath + sZoneIncFileNoPath;
					}
					if (!sZoneIncFile.isEmpty() && sZoneIncFile.compare( sProcZoneIncFile, Qt::CaseInsensitive ) == 0)
					{	// if we have a valid report include file but both the source and destination path\filenames are identical, then empty them, as no copying is required
						sZoneIncFile.clear();
						sProcZoneIncFile.clear();
					}
					if (!sZoneIncFile.isEmpty() && !sProcZoneIncFile.isEmpty())
					{	saZoneIncFiles.push_back( sZoneIncFile );
						saProcZoneIncFiles.push_back( sProcZoneIncFile );
					 	if (BEMPX_GetString( BEMPX_GetDatabaseID( "Zone:Name" ), sProcZoneIncFile, FALSE, 0, -1, iZnIdx ) && !sProcZoneIncFile.isEmpty())
					 		saZoneNameIncFiles.push_back( sProcZoneIncFile );
					 	else
					 		saZoneNameIncFiles.push_back( "<unknown>" );
				}	}
			}
		}

		if (iRetVal == 0)
		{	// Copy weather & report include files into processing directory
			// ??? do this for each & every run ???
			QString sDestWthr;	int i;
			sDestWthr     = m_sProcessPath + m_sCSEWthr.right( m_sCSEWthr.length() - m_sCSEWthr.lastIndexOf('\\') - 1 ); // "CTZ12S13.CSW";
			const char* pszFileDescs[] = {	"CSE weather", 							"CSE report include" 												};
			QString* psaCopySrc[ ] = { 		&m_sCSEWthr,  								(sRptIncFile.isEmpty() ? NULL : &sRptIncFile), 		NULL	};
			QString* psaCopyDest[] = {			&sDestWthr, 								&sProcRptIncFile														};
			int      iaCopyError[] = {			BEMAnal_CECRes_SimWthrWriteError, 	BEMAnal_CECRes_CSEIncFIleWriteError								};
			for (i=0; (psaCopySrc[i] != NULL && iRetVal == 0); i++)
			{	sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
									"application before an updated file can be written.\n\nSelect 'Retry' to update the file "
									"(once the file is closed), or \n'Abort' to abort the analysis." ).arg( pszFileDescs[i], *psaCopyDest[i] );
				if (!OKToWriteOrDeleteFile( psaCopyDest[i]->toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( pszFileDescs[i], *psaCopyDest[i] );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( pszFileDescs[i], *psaCopyDest[i] );
					iRetVal = iaCopyError[i];
				}
				else if (!CopyFile( psaCopySrc[i]->toLocal8Bit().constData(), psaCopyDest[i]->toLocal8Bit().constData(), FALSE ))
				{	sErrorMsg = QString( "ERROR:  Unable to copy %1 file:  '%2'  to:  '%3'" ).arg( pszFileDescs[i], *psaCopySrc[i], *psaCopyDest[i] );
					iRetVal = iaCopyError[i]+1;
				}
			}
		// similar loop as above but for Zone include files
			for (i=0; (i < saZoneIncFiles.size() && iRetVal == 0); i++)
			{	sMsg = QString( "The zone '%1' CSE include file '%2' is opened in another application.  This file must be closed in that "
									"application before an updated file can be written.\n\nSelect 'Retry' to update the file "
									"(once the file is closed), or \n'Abort' to abort the analysis." ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
				if (!OKToWriteOrDeleteFile( saProcZoneIncFiles[i].toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite zone '%1' CSE include file:  %2" ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite zone '%1' CSE include file:  %2" ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
					iRetVal = BEMAnal_CECRes_CSEIncFIleWriteError;
				}
				else if (!CopyFile( saZoneIncFiles[i].toLocal8Bit().constData(), saProcZoneIncFiles[i].toLocal8Bit().constData(), FALSE ))
				{	sErrorMsg = QString( "ERROR:  Unable to copy %1 file:  '%2'  to:  '%3'" ).arg( "CSE include", saZoneIncFiles[i], saProcZoneIncFiles[i] );
					iRetVal = BEMAnal_CECRes_CSEIncFileCopyError;
				}
			}
		}

		// SAC 3/18/17 - storage of TDV CSV file to be made available to the CSE input file
		pCSERun->SetTDVFName( "" );		// SAC 4/16/17
		QString sTDVFile;
		if (iRetVal == 0)
		{	long lCSE_WriteTDV;
			if (BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:CSE_WriteTDV" ), lCSE_WriteTDV ) && lCSE_WriteTDV > 0)
			{	sTDVFile = sProjFileAlone + "-tdv.csv";
				QString sFullTDVFile = m_sProcessPath + sTDVFile;
				sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
				             "application before an updated file can be written.\n\nSelect 'Retry' to update the file "
								 "(once the file is closed), or \n'Abort' to abort the %3." ).arg( "TDV", sFullTDVFile, "compliance report generation" );
				if (!OKToWriteOrDeleteFile( sFullTDVFile.toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file (required for CSE simulation):  %2" ).arg( "TDV", sFullTDVFile );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file (required for CSE simulation):  %2" ).arg( "TDV", sFullTDVFile );
					iRetVal = BEMAnal_CECRes_TDVFileWriteError;	//  Error writing CSV file w/ TDV data (required for CSE simulation)
				}
				else
				{	double daTDVElec[8760], daTDVFuel[8760], daTDVSecElec[8760], daTDVSecFuel[8760];		long lCZ=0, lGas=0;
					bool bHaveSecTDVElec=false, bHaveSecTDVFuel=false;
					if (	!BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:ClimateZone" ), lCZ  ) || lCZ  < 1 || lCZ > 16 ||
							!BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:GasType"     ), lGas ) || lGas < 1 || lGas > 2 ||
							BEMPX_GetTableColumn( &daTDVElec[0], 8760, "TDVTable", ((lCZ-1) * 3) + 1       , NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) != 0 ||
							BEMPX_GetTableColumn( &daTDVFuel[0], 8760, "TDVTable", ((lCZ-1) * 3) + 1 + lGas, NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) != 0)
					{	assert( false );
						sErrorMsg = QString( "ERROR:  Unable to retrieve TDV data for CZ %1, Gas Type %2 (required for CSE simulation)" ).arg( QString::number(lCZ), QString::number(lGas) );
						iRetVal = BEMAnal_CECRes_TDVFileWriteError;	//  Error writing CSV file w/ TDV data (required for CSE simulation)
					}
					else
					{	QDateTime locTime = QDateTime::currentDateTime();
						QString timeStamp = locTime.toString("ddd dd-MMM-yy  hh:mm:ss ap");   // "Wed 14-Dec-16  12:30:29 pm"

						// SAC 9/23/17 - added code to summ GHG TDV adders to TDV data stored to 
						QString qsTDVSecTblElec, qsTDVSecTblFuel;
						bHaveSecTDVElec = ( BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:TDVSecTbl_Elec" ), qsTDVSecTblElec ) && 
												  qsTDVSecTblElec.length() > 0 && qsTDVSecTblElec.compare( "none", Qt::CaseInsensitive ) && 
												  BEMPX_GetTableColumn( &daTDVSecElec[0], 8760, qsTDVSecTblElec.toLocal8Bit().constData(), lCZ, NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) == 0 );
						if (lGas == 1)
							BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:TDVSecTbl_NGas" ), qsTDVSecTblFuel );
						else
							BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:TDVSecTbl_Othr" ), qsTDVSecTblFuel );
						bHaveSecTDVFuel = ( qsTDVSecTblFuel.length() > 0 && qsTDVSecTblFuel.compare( "none", Qt::CaseInsensitive ) && 
												  BEMPX_GetTableColumn( &daTDVSecFuel[0], 8760, qsTDVSecTblFuel.toLocal8Bit().constData(), lCZ, NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) == 0 );
		      		QString qsVer, qsGas;
						if (!BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:CompMgrVersion" ), qsVer ) || qsVer.length() < 1)
							qsVer = "(unknown version)";
						switch (lGas)
						{	case  1 :	qsGas = "NatGas";   break;
							case  2 :	qsGas = "Propane";  break;
							default :	qsGas = "(unknown)";  break;
						}

						FILE *fp_CSV;
						int iErrorCode;
						try
						{	iErrorCode = fopen_s( &fp_CSV, sFullTDVFile.toLocal8Bit().constData(), "wb" );
							if (iErrorCode != 0 || fp_CSV == NULL)
							{	assert( false );
								sErrorMsg = QString( "ERROR:  Unable to write TDV data file (required for CSE simulation):  %1" ).arg( sFullTDVFile );
								iRetVal = BEMAnal_CECRes_TDVFileWriteError;	//  Error writing CSV file w/ TDV data (required for CSE simulation)
							}
							else
							{	fprintf( fp_CSV, "\"TDV Data (TDV/Btu)\",\"001\"\n" );
								fprintf( fp_CSV, "\"%s\"\n", timeStamp.toLocal8Bit().constData() );
								fprintf( fp_CSV, "\"%s, CZ%ld, Fuel %s\",\"Hour\"\n", qsVer.toLocal8Bit().constData(), lCZ, qsGas.toLocal8Bit().constData() );
								fprintf( fp_CSV, "\"tdvElec\",\"tdvFuel\"\n" );
								for (int hr=0; hr<8760; hr++)
								{	if (bHaveSecTDVElec)
										daTDVElec[hr] += (daTDVSecElec[hr] / 3.413);
									if (bHaveSecTDVFuel)
										daTDVFuel[hr] += (daTDVSecFuel[hr] / 100);
									fprintf( fp_CSV, "%g,%g\n", daTDVElec[hr], daTDVFuel[hr] );
								}
								fflush( fp_CSV );
								fclose( fp_CSV );
							}
						}
						catch( ... )
						{	assert( false );
							sErrorMsg = QString( "ERROR:  Exception thrown writing TDV data file (required for CSE simulation):  %1" ).arg( sFullTDVFile );
							iRetVal = BEMAnal_CECRes_TDVFileWriteError;	//  Error writing CSV file w/ TDV data (required for CSE simulation)
					// how to handle error writing to TDV CSV file ??
						}
				}	}
				if (iRetVal == 0)
					pCSERun->SetTDVFName( sTDVFile );		// SAC 4/16/17
		}	}

		if (iRetVal == 0)
			iRetVal = SetupRunFinish( iRunIdx, sErrorMsg );
	}
	return iRetVal;
}		// CSERunMgr::SetupRun


int CSERunMgr::SetupRun_Simple(
	int iRunIdx, int iRunType, QString& sErrorMsg, bool bAllowReportIncludeFile /*=true*/,
	const char* pszRunAbbrev /*=NULL*/, const char* pszExtraCSECmdLineArgs /*=NULL*/,
	const char* pszAppendToCSEFile /*=NULL*/, int iModelType /*=0*/ )
{
	int iRetVal = 0;
	CSERun* pCSERun = new CSERun;
	m_vCSERun.push_back( pCSERun);
	QString sMsg, sLogMsg;
	BOOL bLastRun = (iRunIdx == (m_iNumRuns-1));
	BOOL bIsStdDesign = (iRunType == CRM_StdDesign);	// SAC 3/27/15 - was:  (iRunIdx > 0 && bLastRun);  // SAC 7/3/13 - consolidated some logic to identify 
	pCSERun->SetRunType( iRunType);							// SAC 3/27/15
	pCSERun->SetLastRun( bLastRun);
	pCSERun->SetIsStdDesign( bIsStdDesign);
	QString sModelFileOnlyNoExt = m_sModelFileOnlyNoExt;	// SAC 1/2/19 - added simple way to alter CSE input/output names
	if (pszAppendToCSEFile && strlen(pszAppendToCSEFile) > 0)
		sModelFileOnlyNoExt += pszAppendToCSEFile;
	if (m_bStoreBEMProcDetails)	// SAC 1/20/13 - added export of additional "detail" file to help isolate unnecessary object creation issues
	{	QString sDbgFileName;
		//sDbgFileName = QString( "%1%2 - run %3.ibd-b4Evals" ).arg( m_sProcessPath, m_sModelFileOnlyNoExt, QString::number(iRunIdx+1) );
		//sDbgFileName = QString( "%1%2 - %3.ibd-b4Evals" ).arg( m_sProcessPath, m_sModelFileOnlyNoExt, pszRunAbbrev );
		sDbgFileName = QString( "%1%2.ibd-b4Evals" ).arg( m_sProcessPath, sModelFileOnlyNoExt );
		BEMPX_WriteProjectFile( sDbgFileName.toLocal8Bit().constData(), BEMFM_DETAIL /*FALSE*/ );
	}

	long lRunNumber = (iRunType == CRM_User ? 0 : iRunIdx+1 );		// SAC 4/21/15 - mods to set RunNumber in source code - necessary since we are now starting EACH run's BEMComp database w/ the Proposed Model's
//	BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:RunNumber" ), BEMP_Int, (void*) &lRunNumber );
//	if (pszRunAbbrev && strlen( pszRunAbbrev ) > 0)
//		BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:RunAbbrev" ), BEMP_Str, (void*) pszRunAbbrev );

	if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
		iRetVal = BEMAnal_CECRes_RuleProcAbort;

	QString sRunID, sRunIDProcFile, sRunAbbrev;
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunID"         ),  sRunID         );
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunIDProcFile" ),  sRunIDProcFile );
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunAbbrev"     ),  sRunAbbrev     );

	pCSERun->SetRunNumber( lRunNumber);
	pCSERun->SetRunID( sRunID);
	pCSERun->SetRunIDProcFile( sRunIDProcFile);
	pCSERun->SetRunAbbrev( sRunAbbrev);

//	QString sMissing;
//	if (sRunID.isEmpty())
//		sMissing += "RunID, ";
//	if (sRunAbbrev.isEmpty())
//		sMissing += "RunAbbrev ";
//	if (iRetVal == 0 && !sMissing.isEmpty())
//	{	sMissing = sMissing.left( sMissing.length()-2 );
//		sErrorMsg = "ERROR:  The following Proj properties not set:" + sMissing;
//		iRetVal = BEMAnal_CECRes_GetReqdDataError;
//	}
//	assert( iRunIdx == (lRunNumber-1) );

//	if (iRetVal == 0 && (m_sProcessPath.length() + m_sModelFileOnlyNoExt.length() + sRunIDProcFile.length() + 12) > _MAX_PATH)
//	{	sErrorMsg = QString( "ERROR:  CSE processing path too long:  %1%2%3.*" ).arg( m_sProcessPath, m_sModelFileOnlyNoExt, sRunIDProcFile );
	if (iRetVal == 0 && (m_sProcessPath.length() + sModelFileOnlyNoExt.length() + 12) > _MAX_PATH)		// SAC 12/18/18 - sRunIDProcFile assumed to already be appended...
	{	sErrorMsg = QString( "ERROR:  CSE processing path too long:  %1%2.*" ).arg( m_sProcessPath, sModelFileOnlyNoExt );
		iRetVal = BEMAnal_CECRes_ProcPathTooLong;
	}

	if (iRetVal == 0 && iRunIdx == 0 && m_bInitHourlyResults)  // only initialize hourly results for first simulation??
		BEMPX_InitializeHourlyResults();

//	QString sProjFileAlone = m_sModelFileOnlyNoExt + sRunIDProcFile;
	QString sProjFileAlone = sModelFileOnlyNoExt;		// SAC 12/18/18 - sRunIDProcFile assumed to already be appended...
	if (iRetVal == 0)
	{
//		// try evaluating prop & postprop rulelists EVERY time
//		iRetVal = LocalEvaluateRuleset(		sErrorMsg, BEMAnal_CECRes_EvalPropInp3Error, "ProposedInput", m_bVerbose, m_pCompRuleDebugInfo );		// generic project defaulting
//		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
//			iRetVal = BEMAnal_CECRes_RuleProcAbort;
//
//		// ??? perform simulation check on BUDGET model as well ???
//		if (iRetVal == 0) // && iRunIdx == 0)
//			iRetVal = LocalEvaluateRuleset(	sErrorMsg, BEMAnal_CECRes_EvalSimChkError, "ProposedModelSimulationCheck", m_bVerbose, m_pCompRuleDebugInfo );		// check user input model for simulation-related errors
//		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
//			iRetVal = BEMAnal_CECRes_RuleProcAbort;
//
//		if (iRetVal == 0)
//			iRetVal = LocalEvaluateRuleset(	sErrorMsg, BEMAnal_CECRes_EvalPostPropError, "PostProposedInput", m_bVerbose, m_pCompRuleDebugInfo );		// setup for Proposed run
//		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
//			iRetVal = BEMAnal_CECRes_RuleProcAbort;
//
//		if (iRetVal == 0)
//			iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalSimPrepError, "CSE_SimulationPrep", m_bVerbose, m_pCompRuleDebugInfo );
//		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
//			iRetVal = BEMAnal_CECRes_RuleProcAbort;
//
//		BEMPX_RefreshLogFile();	// SAC 5/19/14
		
//		if (iRetVal == 0 && iRunIdx == 0)  // Store various software & ruleset versions prior to first run - SAC 12/19/12
//		{	QString sVerTemp, sVerStr;
//			BEMPX_GetRulesetID( sVerTemp, sVerStr );
//			if (!sVerStr.isEmpty())
//				BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:RulesetVersion" ), BEMP_QStr, (void*) &sVerStr );
//			else
//				BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:RulesetVersion" ), m_iError );
//
//			QString sCSEVersion = GetVersionInfo();
//			if (!sCSEVersion.isEmpty())
//				BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:PrimSimEngingVer" ), BEMP_QStr, (void*) &sCSEVersion );
//			else
//				BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:PrimSimEngingVer" ), m_iError );
//
//			// SAC 1/8/13 - DHW engine version stored to BEMBase DURING DHW simulation (immediately following the ...Init() call)
//			BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:SecSimEngingVer" ), m_iError );
//		}

//		// Purge user-defined non-parent/child components which are not referenced  - SAC 1/20/13 - added to prevent simulation of objects that are not referenced in the building model
//		// Purge performed HERE, before objects are cross-referenced (Cons <-> cseCONS & Mat <-> cseMATERIAL), w/ those cross-references preventing any useful purging
//		BEMPX_PurgeUnreferencedComponents();

		QString sOutFiles[CSERun::OutFileCOUNT];
		if (iRetVal == 0)
		{	sOutFiles[CSERun::OutFileCSV] = m_sProcessPath + sProjFileAlone + ".csv";
			sOutFiles[CSERun::OutFileREP] = m_sProcessPath + sProjFileAlone + ".rep";
			sOutFiles[CSERun::OutFileERR] = m_sProcessPath + sProjFileAlone + ".err";
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileCSV], CSERun::OutFileCSV);
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileREP], CSERun::OutFileREP);
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileERR], CSERun::OutFileERR);

			static const char* pszOutFileDescs[] = { "CSV output", "REP output", "Error output" };
			int i=0;
			for (; (i<CSERun::OutFileCOUNT && iRetVal == 0); i++)
			{	sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
							"application before an updated file can be written.\n\nSelect 'Retry' to proceed "
								 "(once the file is closed), or \n'Abort' to abort the analysis." ).arg( pszOutFileDescs[i], sOutFiles[i] );
				if (!OKToWriteOrDeleteFile( sOutFiles[i].toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( pszOutFileDescs[i], sOutFiles[i] );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( pszOutFileDescs[i], sOutFiles[i] );
					iRetVal = BEMAnal_CECRes_SimOutWriteError;
				}
				// code to DELETE existing results files prior to simulation
				else if (FileExists( sOutFiles[i].toLocal8Bit().constData() ))
					DeleteFile( sOutFiles[i].toLocal8Bit().constData() );
			}
	}	}

	if (iRetVal == 0 && iModelType == 0 /*T24R*/)	
	{	QString sRptIncFile, sProcRptIncFile, sZoneIncFile, sProcZoneIncFile;		QVector<QString> saZoneIncFiles, saProcZoneIncFiles, saZoneNameIncFiles;
		if (iRetVal == 0)
		{	// Write Report Include file statement
			long lProjReportIncludeFileDBID = BEMPX_GetDatabaseID( "Proj:ReportIncludeFile" );
			if (lProjReportIncludeFileDBID > 0)
				BEMPX_GetString( lProjReportIncludeFileDBID, sRptIncFile );
			if (bAllowReportIncludeFile && !sRptIncFile.isEmpty())
			{	QString sOrigRptIncFile = sRptIncFile;
				BOOL bOrigRptIncFileLastSlashIdx = sRptIncFile.lastIndexOf('\\');
				if (sRptIncFile.indexOf(':') < 0 && sRptIncFile.indexOf('\\') != 0)
				{	// report file does not include a FULL path, so expected to be a path relative to the project file - so prepend project path...
					sRptIncFile = m_sModelPathOnly + sRptIncFile;
					if (!FileExists( sRptIncFile.toLocal8Bit().constData() ))
					{	// if rpt incl file not found in model path, it might have been located elsewhere and already copied into processing directory - SAC 10/27/17
						QString sChkRptIncFile = m_sProcessPath + sOrigRptIncFile;
						if (FileExists( sChkRptIncFile.toLocal8Bit().constData() ))
							sRptIncFile = sChkRptIncFile;
				}	}
				if (!FileExists( sRptIncFile.toLocal8Bit().constData() ))
				{	sLogMsg = QString( "CSE report include file not found '%1'" ).arg( sRptIncFile );
					BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
					//WriteToLogFile( WM_LOGUPDATED, sRptFileErr );
					BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
					sRptIncFile.clear();
				}
				else
				{	QString sRptIncFileNoPath = sRptIncFile.right( sRptIncFile.length() - sRptIncFile.lastIndexOf('\\') - 1 );
					if (bOrigRptIncFileLastSlashIdx >= 0)
						// need to CHANGE report include file setting to reference only the filename (no path info)
						BEMPX_SetBEMData( lProjReportIncludeFileDBID, BEMP_QStr, (void*) &sRptIncFileNoPath );
					sProcRptIncFile = m_sProcessPath + sRptIncFileNoPath;
				}
				if (!sRptIncFile.isEmpty() && sRptIncFile.compare( sProcRptIncFile, Qt::CaseInsensitive ) == 0)
				{	// if we have a valid report include file but both the source and destination path\filenames are identical, then empty them, as no copying is required
					sRptIncFile.clear();
					sProcRptIncFile.clear();
				}
			}
			else if (!bAllowReportIncludeFile && !sRptIncFile.isEmpty())
			{	// SAC 9/4/17 - force re-default of Proj:ReportIncludeFile if it is specified but this run should not have it
				BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
				sRptIncFile.clear();
			}

			// SAC 7/2/15 - added logic here to check for and address usage of Zone:TstatIncludeFile
			long lUseTstatIncFile, lDBID_Zone_UseTstatIncFile = BEMPX_GetDatabaseID( "Zone:UseTstatIncFile" ), lDBID_Zone_TstatIncludeFile = BEMPX_GetDatabaseID( "Zone:TstatIncludeFile" );
			int iNumZones = (lDBID_Zone_UseTstatIncFile < 1 ? 0 : BEMPX_GetNumObjects( BEMPX_GetClassID( lDBID_Zone_UseTstatIncFile ) ));
			for (int iZnIdx=0; (iRetVal == 0 && iZnIdx < iNumZones); iZnIdx++)
			{	if (BEMPX_GetInteger( lDBID_Zone_UseTstatIncFile,  lUseTstatIncFile,    0, -1, iZnIdx ) && lUseTstatIncFile > 0 &&
					 BEMPX_GetString(  lDBID_Zone_TstatIncludeFile, sZoneIncFile, FALSE, 0, -1, iZnIdx ) && !sZoneIncFile.isEmpty() && StringInArray( saZoneIncFiles, sZoneIncFile ) < 0)
				{	//BOOL bOrigZoneIncFileLastSlashIdx = sZoneIncFile.lastIndexOf('\\');
					if (sZoneIncFile.indexOf(':') < 0 && sZoneIncFile.indexOf('\\') != 0)
						// include file does not include a FULL path, so expected to be a path relative to the project file - so prepend project path...
						sZoneIncFile = m_sModelPathOnly + sZoneIncFile;
					if (!FileExists( sZoneIncFile.toLocal8Bit().constData() ))
					{	if (!BEMPX_GetString( BEMPX_GetDatabaseID( "Zone:Name" ), sProcZoneIncFile, FALSE, 0, -1, iZnIdx ) || sProcZoneIncFile.isEmpty())
					 		sProcZoneIncFile = "<unknown>";
						sLogMsg = QString( "ERROR:  CSE zone '%1' include file not found '%2'" ).arg( sProcZoneIncFile, sZoneIncFile );
						BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
						iRetVal = BEMAnal_CECRes_MissingZnCSEIncFile;
						//BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
						sZoneIncFile.clear();
					}
					else
					{	QString sZoneIncFileNoPath = sZoneIncFile.right( sZoneIncFile.length() - sZoneIncFile.lastIndexOf('\\') - 1 );
					//	if (bOrigZoneIncFileLastSlashIdx >= 0)
					//		// need to CHANGE report include file setting to reference only the filename (no path info)
					//		BEMPX_SetBEMData( lProjReportIncludeFileDBID, BEMP_QStr, (void*) &sZoneIncFileNoPath );
						sProcZoneIncFile = m_sProcessPath + sZoneIncFileNoPath;
					}
					if (!sZoneIncFile.isEmpty() && sZoneIncFile.compare( sProcZoneIncFile, Qt::CaseInsensitive ) == 0)
					{	// if we have a valid report include file but both the source and destination path\filenames are identical, then empty them, as no copying is required
						sZoneIncFile.clear();
						sProcZoneIncFile.clear();
					}
					if (!sZoneIncFile.isEmpty() && !sProcZoneIncFile.isEmpty())
					{	saZoneIncFiles.push_back( sZoneIncFile );
						saProcZoneIncFiles.push_back( sProcZoneIncFile );
					 	if (BEMPX_GetString( BEMPX_GetDatabaseID( "Zone:Name" ), sProcZoneIncFile, FALSE, 0, -1, iZnIdx ) && !sProcZoneIncFile.isEmpty())
					 		saZoneNameIncFiles.push_back( sProcZoneIncFile );
					 	else
					 		saZoneNameIncFiles.push_back( "<unknown>" );
				}	}
			}
		}

		if (iRetVal == 0)
		{	// Copy weather & report include files into processing directory
			// ??? do this for each & every run ???
			QString sDestWthr;	int i;
			sDestWthr     = m_sProcessPath + m_sCSEWthr.right( m_sCSEWthr.length() - m_sCSEWthr.lastIndexOf('\\') - 1 ); // "CTZ12S13.CSW";
			const char* pszFileDescs[] = {	"CSE weather", 							"CSE report include" 												};
			QString* psaCopySrc[ ] = { 		&m_sCSEWthr,  								(sRptIncFile.isEmpty() ? NULL : &sRptIncFile), 		NULL	};
			QString* psaCopyDest[] = {			&sDestWthr, 								&sProcRptIncFile														};
			int      iaCopyError[] = {			BEMAnal_CECRes_SimWthrWriteError, 	BEMAnal_CECRes_CSEIncFIleWriteError								};
			for (i=0; (psaCopySrc[i] != NULL && iRetVal == 0); i++)
			{	sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
									"application before an updated file can be written.\n\nSelect 'Retry' to update the file "
									"(once the file is closed), or \n'Abort' to abort the analysis." ).arg( pszFileDescs[i], *psaCopyDest[i] );
				if (!OKToWriteOrDeleteFile( psaCopyDest[i]->toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( pszFileDescs[i], *psaCopyDest[i] );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( pszFileDescs[i], *psaCopyDest[i] );
					iRetVal = iaCopyError[i];
				}
				else if (!CopyFile( psaCopySrc[i]->toLocal8Bit().constData(), psaCopyDest[i]->toLocal8Bit().constData(), FALSE ))
				{	sErrorMsg = QString( "ERROR:  Unable to copy %1 file:  '%2'  to:  '%3'" ).arg( pszFileDescs[i], *psaCopySrc[i], *psaCopyDest[i] );
					iRetVal = iaCopyError[i]+1;
				}
			}
		// similar loop as above but for Zone include files
			for (i=0; (i < saZoneIncFiles.size() && iRetVal == 0); i++)
			{	sMsg = QString( "The zone '%1' CSE include file '%2' is opened in another application.  This file must be closed in that "
									"application before an updated file can be written.\n\nSelect 'Retry' to update the file "
									"(once the file is closed), or \n'Abort' to abort the analysis." ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
				if (!OKToWriteOrDeleteFile( saProcZoneIncFiles[i].toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite zone '%1' CSE include file:  %2" ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite zone '%1' CSE include file:  %2" ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
					iRetVal = BEMAnal_CECRes_CSEIncFIleWriteError;
				}
				else if (!CopyFile( saZoneIncFiles[i].toLocal8Bit().constData(), saProcZoneIncFiles[i].toLocal8Bit().constData(), FALSE ))
				{	sErrorMsg = QString( "ERROR:  Unable to copy %1 file:  '%2'  to:  '%3'" ).arg( "CSE include", saZoneIncFiles[i], saProcZoneIncFiles[i] );
					iRetVal = BEMAnal_CECRes_CSEIncFileCopyError;
				}
			}
	}	}

	if (iRetVal == 0)	
	{	// SAC 3/18/17 - storage of TDV CSV file to be made available to the CSE input file
		pCSERun->SetTDVFName( "" );		// SAC 4/16/17

		if (iRetVal == 0)
//			iRetVal = SetupRunFinish( iRunIdx, sErrorMsg );
// SAC 12/18/18 - replaced call to SetupRunFinish() w/ the minimal body needed in this _Simple case (below)
		{
		BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "cseTOP:tdvfName" ), m_iError );
		QString sProjCSEFile;
		if (iRetVal == 0)
		{	sProjCSEFile = m_sProcessPath + sProjFileAlone + ".cse";
					SetCurrentDirectory( m_sProcessPath.toLocal8Bit().constData() );
				//	if (sCSEFileCopy && strlen( sCSEFileCopy ) > 0)
				//	{	QString sCSECopyFileName = sLpCSEFile.left( sLpCSEFile.length()-4 );
				//		sCSECopyFileName += sCSEFileCopy;		sCSECopyFileName += ".cse";
				//		CopyFile( sLpCSEFile.toLocal8Bit().constData(), sCSECopyFileName.toLocal8Bit().constData(), FALSE );
				//	}
				//	if (m_bStoreBEMProcDetails)
				//	{	QString sDbgFileName = sLpCSEFile.left( sLpCSEFile.length()-3 );
				//		sDbgFileName += "ibd-Detail";
				//		BEMPX_WriteProjectFile( sDbgFileName.toLocal8Bit().constData(), BEMFM_DETAIL /*FALSE*/ );
				//	}
				//BEMPX_RefreshLogFile();	// SAC 5/19/14
			//}	// end of iFLp loop
		}
		if (iRetVal == 0 && m_bPerformSimulations && !m_bBypassCSE)
		{
			// remove trailing '.cse' file extension from path/file passed into CSE
			QString sProjCSEFileForArg = sProjCSEFile;
			if (!sProjCSEFileForArg.right(4).compare(".CSE", Qt::CaseInsensitive))
				sProjCSEFileForArg = sProjCSEFileForArg.left( sProjCSEFileForArg.length()-4 );

			// CSE SIMULATION
			QString sParams;
			//	sParams = QString( "-b \"%s.cse\"" ).arg( sProjFileAlone );		// SAC 8/20/12 - added '-b' (batch) option to prevent user prompt in the event of sim error
			if (pszExtraCSECmdLineArgs && strlen(pszExtraCSECmdLineArgs) > 0)
				sParams = QString( "-b \"%1\"%2" ).arg( sProjCSEFileForArg, pszExtraCSECmdLineArgs );
			else
				sParams = QString( "-b \"%1\"" ).arg( sProjCSEFileForArg );
	#ifdef _DEBUG
			sLogMsg = QString( "   CSE exePath:  %1" ).arg( m_sCSEexe );
			BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
			sLogMsg = QString( "   CSE cmdLine:  %1" ).arg( sParams );
			BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
	#endif
			pCSERun->SetArgs( sParams);
		}
		}	// end of excerpt from SetupRunFinish()
	}

	if (iRetVal != 0 && m_bVerbose)
	{	sLogMsg = QString( "   CSERunMgr::SetupRun_Simple() error - returning %1" ).arg( QString::number(iRetVal) );
		BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
	}	

	return iRetVal;
}		// CSERunMgr::SetupRun


//-----------------------------------------------------------------------------
vector< string> split_string( const string& line, const string delim)
{
	vector< string> toks;
	auto prev_pos = line.begin();
	auto next_pos = search(prev_pos, line.end(),
		delim.begin(), delim.end());
	while (next_pos != line.end())
	{
		toks.emplace_back(prev_pos, next_pos);
		prev_pos = next_pos + delim.size();
		next_pos = search(prev_pos, line.end(),
			delim.begin(), delim.end());
	}

	if (prev_pos != line.end())
		toks.emplace_back(prev_pos, line.end());
	return toks;
}		// split_string
//-----------------------------------------------------------------------------
//  ModelType:		0  T24-Res
//						1  T24-Com
// SAC 1/1/19 - version of sizing that involves 3 parallel sizing runs w/ fit to parabola to calculate optimum size
int CSE_PerformHPWHSizing_3Run( QString sCSEexe, QString sCSEWthr, QString sModelPathOnly, QString sModelFileOnlyNoExt, QString sProcessPath,
											std::vector<double>& daRunMults, int iSysIdx, QString sStdHPWHSzTDVTbl, long lStdHPWHSzTDVCol, bool bVerbose, int iModelType )
// returns 0 iff success
//         1 fail (CSE error, ...)
{
	bool bStoreBEMDetails = bVerbose;
	bool bSilent = false;
	CSERunMgr cseRunMgr( sCSEexe, sCSEWthr, sModelPathOnly, sModelFileOnlyNoExt, sProcessPath, false /*bFullComplianceAnalysis*/,
						false /*bInitHourlyResults*/, 0 /*lAllOrientations*/, 0 /*lAnalysisType*/, 2019 /*iRulesetCodeYear(unused?)*/, 0 /*lDesignRatingRunID*/,
						bVerbose, bStoreBEMDetails, true /*bPerformSimulations*/, false /*bBypassCSE*/, bSilent, NULL /*pCompRuleDebugInfo*/, NULL /*pszUIVersionString*/,
						0 /*iSimReportDetailsOption*/, 0 /*iSimErrorDetailsOption*/	);
	int iNumRuns = (int) daRunMults.size();
	int iMultDecPrec=5, iCSESimRetVal=0, iR=0;
	QString sErrMsg, sLogMsg;
	for (iR=0; (iCSESimRetVal == 0 && sErrMsg.isEmpty() && iR < iNumRuns); iR++)
	{
		QString sHPWHSizeCmdLineArg = QString( " -DHPWHSIZE=%1" ).arg( QString::number(daRunMults[iR], 'f', iMultDecPrec) );
		QString sNum = QString::number(iR+1);
		iCSESimRetVal = cseRunMgr.SetupRun_Simple( iR /*iRunIdx*/, CRM_User /*iRunType*/, sErrMsg, true /*bAllowReportIncludeFile*/,
													"HPWHSz" /*pszRunAbbrev*/, sHPWHSizeCmdLineArg.toLocal8Bit().constData(), sNum.toLocal8Bit().constData(), iModelType );
			//BEMMessageBox( QString("hsz_Run1():  cseRunMgr.SetupRun_Simple() returned %1 (0=>OK) for file:  %2").arg(QString::number(iCSESimRetVal), hsz_sModelFileOnlyNoExt) );		// debugging
	}

	if (iCSESimRetVal == 0)
	{	bool bSaveFreezeProg = sbFreezeProgress;
		sbFreezeProgress = true;	// SAC 5/31/16 - prevent progress reporting during (very quick) CSE DHW simulations
		cseRunMgr.DoRuns();
		sbFreezeProgress = bSaveFreezeProg;
	}

	double dHrlyRes[3][8760], dAnnUse[3] = {0.0,0.0,0.0};
	double dDbgRawRes[3] = {0.0,0.0,0.0};		// SAC 1/29/19
	for (iR=0; (iCSESimRetVal == 0 && sErrMsg.isEmpty() && iR < iNumRuns); iR++)
	{	const CSERun& cseRun = cseRunMgr.GetRun(iR/*iRunIdx*/);
		const QString& sRunID = cseRun.GetRunID();
		const QString& sRunAbbrev = cseRun.GetRunAbbrev();
		//long lRunNumber = 1;  //(lAnalysisType < 1 ? 1 : cseRun.GetRunNumber());
		int iCSERetVal = cseRun.GetExitCode();
		if (bVerbose)  // SAC 1/31/13
		{	sLogMsg.sprintf( "      %s simulation returned %d", "CSE"/*qsCSEName.toLocal8Bit().constData()*/, iCSERetVal );
			BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
		}
		BEMPX_RefreshLogFile();	// SAC 5/19/14

		if (iCSERetVal != 0)
		{	sErrMsg.sprintf( "ERROR:  %s simulation returned %d", "CSE"/*qsCSEName.toLocal8Bit().constData()*/, iCSERetVal );
			iCSESimRetVal = BEMAnal_CECRes_CSESimError;
			BEMPX_WriteLogFile( sErrMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
		}

		if (iCSESimRetVal == 0)
		{	assert( iR < 3 );	 // only set up for 3 runs of results (for now)
			if (iR >= 3)
				iCSESimRetVal = 1;
		}

		if (iCSESimRetVal == 0)
		{	// read CSE results (fixed column - to avoid dealing w/ BEMProc stored results...)
			filebuf fb;		int i;
			QString sResFile = QString( "%1%2%3.csv" ).arg( sProcessPath, sModelFileOnlyNoExt, QString::number(iR+1) );
			if (!fb.open(sResFile.toLocal8Bit().constData(), std::ios::in))
				iCSESimRetVal = 1;
			else
			{	string line;
				std::istream is(&fb);
				for (i=0;i<4;i++)
					getline(is, line);
				const int colTot = 5;
				for (i=0;i<8760;i++)
				{	getline(is, line);
					vector< string> toks = split_string(line, ",");
					dHrlyRes[iR][i] = stod(toks[colTot]);
					dDbgRawRes[iR] += dHrlyRes[iR][i];
				}
				fb.close();
		}	}

		if (iCSESimRetVal == 0)
		{	if (iModelType == 1)		// convert kBtu CSE results into kWh, as those are the units the Com TDV table are in
			{	for (int i=0;i<8760;i++)
					dHrlyRes[iR][i] /= 3.412;
			}

			// apply hourly TDV multipliers to DHW results (or simply sum hourlys into single use result)
			if (!sStdHPWHSzTDVTbl.isEmpty() && lStdHPWHSzTDVCol > 0)
				dAnnUse[iR] = BEMPX_ApplyHourlyMultipliersFromTable( dHrlyRes[iR], sStdHPWHSzTDVTbl.toLocal8Bit().constData(),	lStdHPWHSzTDVCol, bVerbose );
			else
			{	for (int i=0;i<8760;i++)
					dAnnUse[iR] += dHrlyRes[iR][i];
			}

			int iSetRetVal = BEMPX_SetBEMData( BEMPX_GetDatabaseID( "cseDHWSYS:StdHPWHSzRunUse[1]" )+iR, BEMP_Flt, (void*) &dAnnUse[iR], BEMO_User, iSysIdx-1 );
			if (iSetRetVal < 0)
			{	iCSESimRetVal = 1;
				sErrMsg = QString( "Error storing HPWH sizing run use for cseDHWSYS %1:  Setting of StdHPWHSzRunUse[%2] (%3) returned %4." ).arg(
														QString::number(iSysIdx), QString::number(iR+1), QString::number(dAnnUse[iR]), QString::number( iSetRetVal ) );
		}	}

	//	if (bVerbose)
		if (1)  // temporarily always log each sizing run
		{	if (iCSESimRetVal != 0)
				sLogMsg = QString( "   HPWH Sizing system %1, run %2 using multiplier %3, failed (returned %4)" ).arg(
														QString::number(iSysIdx), QString::number(iR+1), QString::number(daRunMults[iR], 'f', iMultDecPrec), QString::number(iCSESimRetVal) );
			else if (!sErrMsg.isEmpty())
				sLogMsg = QString( "   HPWH Sizing system %1, run %2 using multiplier %3,  %4" ).arg(
														QString::number(iSysIdx), QString::number(iR+1), QString::number(daRunMults[iR], 'f', iMultDecPrec), sErrMsg );
			else
				sLogMsg = QString( "   HPWH Sizing system %1, run %2 using multiplier %3 resulted in %4 kTDV   (%5 kWh)" ).arg(
														QString::number(iSysIdx), QString::number(iR+1), QString::number(daRunMults[iR], 'f', iMultDecPrec), QString::number(dAnnUse[iR], 'f', 0), QString::number(dDbgRawRes[iR]/3.412, 'f', 0) );
			BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
		}
	}

	if (bVerbose)
	{	// output to Project Log file
	}

	int ret=0;
	if (iCSESimRetVal != 0)
		ret = iCSESimRetVal;
	else if (!sErrMsg.isEmpty())
		ret = 1;

	return ret;
}	// CSE_PerformHPWHSizing_3Run
//-----------------------------------------------------------------------------
//  ModelType:		0  T24-Res
//						1  T24-Com
bool CSERunMgr::T24Res_HPWHSizing( QString sProjFileAlone, QString sRunID,
								QString& sErrorMsg, int iModelType )	// SAC 12/13/18 (HPWHSIZE)
{
	QString qsMainDWHSysClass = (iModelType == 1 ? "ResDHWSys" : "DHWSys");
	std::vector<int> iaCSEDHWSystemsSizedIdx, iaDHWSystemsSizedIdx;
	int iCID_DHWSys       = BEMPX_GetDBComponentID( qsMainDWHSysClass.toLocal8Bit().constData() );
	int iCID_CSEDHWSys    = BEMPX_GetDBComponentID( "cseDHWSYS" );
	int iCID_CSEDHWHeater = BEMPX_GetDBComponentID( "cseDHWHEATER" );
	long lDBID_SzRunMults = BEMPX_GetDatabaseID( (iModelType == 1 ? "StdHPWHSizingRunMults[1]" : "StdHPWHSzRunMlts[1]"), iCID_DHWSys );
	int iSV, iErr, iNumCSEDHWSystems = BEMPX_GetNumObjects( iCID_CSEDHWSys );
	bool bFirstDHWSysBeingSized = true;
	for (int iSIdx=0; (sErrorMsg.isEmpty() && iSIdx < iNumCSEDHWSystems); iSIdx++)
	{	BEMObject* pCentralSys = BEMPX_GetObjectPtr( BEMPX_GetDatabaseID( "cseDHWSYS:wsCentralDHWSYS" ), iSV, iErr, iSIdx );
		BEMObject* pSrcDHWSys  = BEMPX_GetObjectPtr( BEMPX_GetDatabaseID( "cseDHWSYS:SourceDHWSys"    ), iSV, iErr, iSIdx );
		int iDHWSysObjIdx = (pSrcDHWSys && pSrcDHWSys->getClass()) ? BEMPX_GetObjectIndex( pSrcDHWSys->getClass(), pSrcDHWSys ) : -1;
		long lHPWHSizingReqd=0;
		if (pCentralSys == NULL && iDHWSysObjIdx >= 0 && 
			 BEMPX_GetInteger( BEMPX_GetDatabaseID( "HPWHSizingReqd", iCID_DHWSys ), lHPWHSizingReqd, 0, -1, iDHWSysObjIdx ) && lHPWHSizingReqd > 0)
		{	// process each system that isn't a slave and that has the flag set HPWHSizingReqd
			iaCSEDHWSystemsSizedIdx.push_back( iSIdx );
			iaDHWSystemsSizedIdx.push_back( iDHWSysObjIdx );
			BEMObject* pCSEDHWSysObj = BEMPX_GetObjectByClass( iCID_CSEDHWSys, iErr, iSIdx );								assert( pCSEDHWSysObj );
			std::vector<int> iaCSEDHWHtrIdx;
			QString sWHHeatSrc;		bool bHaveFuelHtr=false;		BEM_ObjType bemObjTypeUser = BEMO_User;
			int iHtr, iNumCSEDHWHtrs = (int) BEMPX_GetNumChildren( iCID_CSEDHWSys, iSIdx, BEMO_User /*eParObjType*/, iCID_CSEDHWHeater );
			for (iHtr=1; (sErrorMsg.isEmpty() && iHtr <= iNumCSEDHWHtrs); iHtr++)
			{	int iCSEDHWHtrIdx = BEMPX_GetChildObjectIndex( iCID_CSEDHWSys, iCID_CSEDHWHeater, iErr, bemObjTypeUser, iHtr, iSIdx );
				if (iCSEDHWHtrIdx >= 0)
				{	BEMPX_GetString( BEMPX_GetDatabaseID( "cseDHWHEATER:whHeatSrc" ), sWHHeatSrc, TRUE, 0, -1, iCSEDHWHtrIdx );
					if (!sWHHeatSrc.compare("ASHPX"))
					{	// this is a heater child of the system being sized and is of type ASHPX, so set it up for sizing run
						BEMObject* pCSEDHWHtrObj = BEMPX_GetObjectByClass( iCID_CSEDHWHeater, iErr, iCSEDHWHtrIdx );			assert( pCSEDHWHtrObj );
		// SAC 1/8/19 - remove check for cseDHWHEATER:whXBUEndUse undefined as the latest rule mods have this set for EVERY cseDHWHEATER (per BW request)
		//				int iWHXBUEndUse_Status = BEMPX_GetDataStatus( BEMPX_GetDatabaseID( "cseDHWHEATER:whXBUEndUse" ), iCSEDHWHtrIdx );
		//				assert( iWHXBUEndUse_Status < 1 );
		//				if (iWHXBUEndUse_Status < 1)
		//				{
							if (!CMX_EvaluateRuleset( "CSE_DHWHeater_SetupHPWHSizingRun", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
														NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo,
														NULL /*QStringList* psaWarningMsgs*/, iCID_CSEDHWHeater, iCSEDHWHtrIdx, 0 /*iEvalOnlyObjType*/ ))
								sErrorMsg = QString( "Error evaluating 'CSE_DHWHeater_SetupHPWHSizingRun' rulelist on %1 run during HPWH sizing for DHWSys '%2' / cseDHWSYS '%3', heater '%4'" ).arg(
																			sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName(), pCSEDHWHtrObj->getName() );
							iaCSEDHWHtrIdx.push_back( iCSEDHWHtrIdx );
		//				}
		//				else
		//				{		assert( false );  // should never get here - indicates this cseDHWHEATER is already setup for sizing runs...
		//				}
					}
					else if (!sWHHeatSrc.compare("Fuel"))
					{	bHaveFuelHtr = true;
					}
			}	}
			if (sErrorMsg.isEmpty() && iaCSEDHWHtrIdx.size() < 1)
				sErrorMsg = QString( "Error performing %1 run HPWH sizing for DHWSys '%2' / cseDHWSYS '%3':  no ASHPX heaters identified." ).arg( sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName() );
			else if (sErrorMsg.isEmpty() && iaCSEDHWHtrIdx.size() > 0)
			{	// evaluate rules to setup cseDHWSYS for HPWHSIZE run(s)
				if (!CMX_EvaluateRuleset( "CSE_DHWSystem_SetupHPWHSizingRun", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
											NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo,
											NULL /*QStringList* psaWarningMsgs*/, iCID_CSEDHWSys, iSIdx, 0 /*iEvalOnlyObjType*/ ))
					sErrorMsg = QString( "Error evaluating 'CSE_DHWSystem_SetupHPWHSizingRun' rulelist on %1 run during HPWH sizing for DHWSys '%2' / cseDHWSYS '%3'" ).arg(
																sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName() );
			}
			if (sErrorMsg.isEmpty() && iaCSEDHWHtrIdx.size() > 0 && bFirstDHWSysBeingSized)
			{	// call 1-time (project level) rulelist to setup Export, REport and perhaps other objects used in HPWHSIZE runs
				bFirstDHWSysBeingSized = false;
				if (!CMX_EvaluateRuleset( "CSE_Project_SetupHPWHSizingRun", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
													NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo ))
					sErrorMsg = QString( "Error evaluating 'CSE_Project_SetupHPWHSizingRun' rulelist on %1 run during HPWH sizing process" ).arg( sRunID );
			}

		// store flags for each object to be written to HPWH Sizing CSE input
			std::vector<long> laClsObjIndicesToWrite;		// each index:  (ClassID * BEMF_ClassIDMult) + (0 for all objects, else 1-based object index)
			laClsObjIndicesToWrite.push_back( BEMF_ClassIDMult );  // Proj object
			laClsObjIndicesToWrite.push_back( (BEMPX_GetDBComponentID( "cseTOP" ) * BEMF_ClassIDMult) );  // cseTOP object
			int iCID_CSEMeter = BEMPX_GetDBComponentID( "cseMETER" );
			BEMObject* pObj = BEMPX_GetObjectByName( iCID_CSEMeter, iErr, "MtrElec" );		assert(pObj);
			if (pObj)
				laClsObjIndicesToWrite.push_back( (iCID_CSEMeter * BEMF_ClassIDMult) + BEMPX_GetObjectIndex( pObj->getClass(), pObj )+1 );			// Elec Meter
			if (bHaveFuelHtr)
			{	pObj = BEMPX_GetObjectByName( iCID_CSEMeter, iErr, "MtrNatGas" );		assert(pObj);
				if (pObj)
					laClsObjIndicesToWrite.push_back( (iCID_CSEMeter * BEMF_ClassIDMult) + BEMPX_GetObjectIndex( pObj->getClass(), pObj )+1 );		// NatGas Meter
			}
			laClsObjIndicesToWrite.push_back( (iCID_CSEDHWSys * BEMF_ClassIDMult) + iSIdx+1 );			// Main cseDHWSYS being sized
			for (int iS2Idx=0; (sErrorMsg.isEmpty() && iS2Idx < iNumCSEDHWSystems); iS2Idx++)
			{	BEMObject* pCentralSys2 = BEMPX_GetObjectPtr( BEMPX_GetDatabaseID( "cseDHWSYS:wsCentralDHWSYS" ), iSV, iErr, iS2Idx );
				if (pCentralSys2 && pCentralSys2 == pCSEDHWSysObj)
				{	assert( iSIdx != iS2Idx );
					laClsObjIndicesToWrite.push_back( (iCID_CSEDHWSys * BEMF_ClassIDMult) + iS2Idx+1 );		// Slave to Main cseDHWSYS being sized
			}	}
			// output HPWHSIZE-related EXPORT & REPORT objects
			int iCID_CSEExport = BEMPX_GetDBComponentID( "cseEXPORT" );
			int iCID_CSEReport = BEMPX_GetDBComponentID( "cseREPORT" );
			pObj = BEMPX_GetObjectByName( iCID_CSEExport, iErr, "HPWH Sizing Export" );		assert(pObj);  	// cseEXPORT object
			if (pObj)
				laClsObjIndicesToWrite.push_back( (iCID_CSEExport * BEMF_ClassIDMult) + BEMPX_GetObjectIndex( pObj->getClass(), pObj )+1 );
			pObj = BEMPX_GetObjectByName( iCID_CSEReport, iErr, "HPWH Sizing Report" );		assert(pObj);  	// cseREPORT object
			if (pObj)
				laClsObjIndicesToWrite.push_back( (iCID_CSEReport * BEMF_ClassIDMult) + BEMPX_GetObjectIndex( pObj->getClass(), pObj )+1 );

		//	double dHPWHSizingFrac=0.005, dHPWHSizingTol=0.001, dStdHPWHSizingMult=1.0;		- SAC 1/24/19 - removed due to unreferenced
		//	if (sErrorMsg.isEmpty())
		//	{	if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "Proj:CSE_HPWHSizingFrac" ), dHPWHSizingFrac ) ||
		//			 !BEMPX_GetFloat( BEMPX_GetDatabaseID( "Proj:CSE_HPWHSizingTol"  ), dHPWHSizingTol  ) ||
		//			 !BEMPX_GetFloat( BEMPX_GetDatabaseID( "Proj:StdHPWHSizingMult"  ), dStdHPWHSizingMult ))
		//		sErrorMsg = QString( "Error performing %1 run HPWH sizing for DHWSys '%2' / cseDHWSYS '%3':  Unable to retrieve CSE_HPWHSizingFrac, CSE_HPWHSizingTol or StdHPWHSizingMult." ).arg( sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName() );
		//	}

			long lStdHPWHSzNumRuns=0;
			BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:StdHPWHSzNumRuns" ), lStdHPWHSzNumRuns );				assert( lStdHPWHSzNumRuns > 0 );		// SAC 1/2/19
			QString sStdHPWHSzTDVTbl;		long lStdHPWHSzTDVCol;
			BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:StdHPWHSzTDVTbl" ), sStdHPWHSzTDVTbl );
			BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:StdHPWHSzTDVCol" ), lStdHPWHSzTDVCol );

			long lStdHPWHSizingFormula=0;
			BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:StdHPWHSizingForm" ), lStdHPWHSizingFormula );		assert( lStdHPWHSizingFormula >= 0 && lStdHPWHSizingFormula <= 1 );

// SAC 1/1/19 - removed code to store ElecRes version of std design HPWH sizing run (HPWHSIZE)
//			// depending on lStdHPWHSizingFormula, write separate ElecRes CSE input for FIRST sizing run, to serve as basis of sizing mechanism - SAC 12/29/18 (HPWHSIZE)
//			QString sERSzCSEFileOnlyNoExt;
//			if (sErrorMsg.isEmpty() && lStdHPWHSizingFormula == 1)
//			{	for (iHtr=0; (sErrorMsg.isEmpty() && iHtr < (int) iaCSEDHWHtrIdx.size()); iHtr++)
//				{	if (!CMX_EvaluateRuleset( "cseDHWHEATER_InstallSizingElecRes", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
//														NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo,
//														NULL /*QStringList* psaWarningMsgs*/, iCID_CSEDHWHeater, iaCSEDHWHtrIdx[iHtr], 0 /*iEvalOnlyObjType*/ ))
//					{	BEMObject* pCSEDHWHtrObj = BEMPX_GetObjectByClass( iCID_CSEDHWHeater, iErr, iaCSEDHWHtrIdx[iHtr] );			assert( pCSEDHWHtrObj );
//						sErrorMsg = QString( "Error evaluating 'cseDHWHEATER_InstallSizingElecRes' rulelist on %1 run during HPWH sizing for DHWSys '%2' / cseDHWSYS '%3', heater '%4'" ).arg(
//														sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName(), pCSEDHWHtrObj->getName() );
//				}	}
//				if (sErrorMsg.isEmpty())
//				{	sERSzCSEFileOnlyNoExt = QString( "%1-dhwszer%2" ).arg( sProjFileAlone, QString::number(iSIdx+1) );
//					QString sSzCSEFile = QString( "%1%2.cse" ).arg( m_sProcessPath, sERSzCSEFileOnlyNoExt );
//					// Write CSE input file  (and store BEM details file)
//					QString sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
//									 "application before an updated file can be written.\n\nSelect 'Retry' to update the file "
//									 "(once the file is closed), or \n'Abort' to abort the analysis." ).arg( "CSE input", sSzCSEFile );
//					if (!OKToWriteOrDeleteFile( sSzCSEFile.toLocal8Bit().constData(), sMsg, m_bSilent ))
//					{	if (m_bSilent)
//							sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( "ElecRes sizing CSE input", sSzCSEFile );
//						else
//							sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( "ElecRes sizing CSE input", sSzCSEFile );
//					}
//					else if (!BEMPX_WriteProjectFile( sSzCSEFile.toLocal8Bit().constData(), BEMFM_INPUT /*TRUE*/, FALSE /*bUseLogFileName*/,
//												FALSE /*bWriteAllProperties*/, FALSE /*bSupressAllMessageBoxes*/, BEMFT_CSE /*iFileType*/, false /*bAppend*/,
//												NULL /*ModelName*/, true /*WriteTerminator*/, -1 /*BEMProcIdx*/, -1 /*ModDate*/, false /*OnlyValidInputs*/,
//												true /*AllowCreateDateReset*/, 1 /*PropertyCommentOption*/, &laClsObjIndicesToWrite ))
//						sErrorMsg = QString( "ERROR:  Unable to write %1 file:  %2" ).arg( "ElecRes sizing CSE input", sSzCSEFile );
//					else
//					{	for (iHtr=0; (sErrorMsg.isEmpty() && iHtr < (int) iaCSEDHWHtrIdx.size()); iHtr++)
//						{	if (!CMX_EvaluateRuleset( "cseDHWHEATER_RestoreStdElecDHWHeater", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
//																NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo,
//																NULL /*QStringList* psaWarningMsgs*/, iCID_CSEDHWHeater, iaCSEDHWHtrIdx[iHtr], 0 /*iEvalOnlyObjType*/ ))
//							{	BEMObject* pCSEDHWHtrObj = BEMPX_GetObjectByClass( iCID_CSEDHWHeater, iErr, iaCSEDHWHtrIdx[iHtr] );			assert( pCSEDHWHtrObj );
//								sErrorMsg = QString( "Error evaluating 'cseDHWHEATER_RestoreStdElecDHWHeater' rulelist on %1 run during HPWH sizing for DHWSys '%2' / cseDHWSYS '%3', heater '%4'" ).arg(
//																sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName(), pCSEDHWHtrObj->getName() );
//					}	}	}
//			}	}

			QString sSzCSEFileOnlyNoExt = QString( "%1-dhwsz%2-1" ).arg( sProjFileAlone, QString::number(iSIdx+1) );
			if (sErrorMsg.isEmpty())
			{	QString sSzCSEFile = QString( "%1%2.cse" ).arg( m_sProcessPath, sSzCSEFileOnlyNoExt );
				// Write CSE input file  (and store BEM details file)
				QString sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
								 "application before an updated file can be written.\n\nSelect 'Retry' to update the file "
								 "(once the file is closed), or \n'Abort' to abort the analysis." ).arg( "CSE input", sSzCSEFile );
				if (!OKToWriteOrDeleteFile( sSzCSEFile.toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( "HPWH sizing CSE input", sSzCSEFile );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( "HPWH sizing CSE input", sSzCSEFile );
				}
				else if (!BEMPX_WriteProjectFile( sSzCSEFile.toLocal8Bit().constData(), BEMFM_INPUT /*TRUE*/, FALSE /*bUseLogFileName*/,
											FALSE /*bWriteAllProperties*/, FALSE /*bSupressAllMessageBoxes*/, BEMFT_CSE /*iFileType*/, false /*bAppend*/,
											NULL /*ModelName*/, true /*WriteTerminator*/, -1 /*BEMProcIdx*/, -1 /*ModDate*/, false /*OnlyValidInputs*/,
											true /*AllowCreateDateReset*/, 1 /*PropertyCommentOption*/, &laClsObjIndicesToWrite ))
					sErrorMsg = QString( "ERROR:  Unable to write %1 file:  %2" ).arg( "HPWH sizing CSE input", sSzCSEFile );
				else
				{	// HPWH Sizing CSE input file written - so now process it
//					double dWHMult = 1.5;

//QString sDbg = QString( "   CSE_PerformHPWHSizing_Iterate( '%1', " ).arg( m_sCSEexe );			BEMPX_WriteLogFile( sDbg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
//sDbg = QString(         "                          '%1', " ).arg( sSzCSEFile );		BEMPX_WriteLogFile( sDbg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
//sDbg = QString( "                          %1, %2, ... )" ).arg( QString::number(dHPWHSizingFrac), QString::number(dHPWHSizingTol) );		BEMPX_WriteLogFile( sDbg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );

									boost::posix_time::ptime	tmHPWHSizingStartTime = boost::posix_time::microsec_clock::local_time();

// previous version
//					double dWHMult=0.0;
//					double dSizeMultGuess;
//			 		BEMPX_GetFloat( BEMPX_GetDatabaseID( "DHWSys:HPWHSizeMultGuess" ), dSizeMultGuess, 0.0, -1, iDHWSysObjIdx );		// SAC 12/21/18 - HPWHSIZE
//					int iHPWHSzRetVal = CSE_PerformHPWHSizing_Iterate( m_sCSEexe /*"CSE"*/, m_sCSEWthr, m_sModelPathOnly, sSzCSEFileOnlyNoExt, sERSzCSEFileOnlyNoExt, m_sProcessPath,
//																			dHPWHSizingFrac /*fXBUTarg*/, dHPWHSizingTol /*tol*/, dSizeMultGuess, lStdHPWHSizingFormula, dWHMult, iSIdx+1, m_bVerbose );		// sErrorMsg
//
//									double dTimeForHPWHSizing = DeltaTime( tmHPWHSizingStartTime );
//									QString sLogMsg = QString( "HPWH sizing for DHWSys '%1' / cseDHWSYS '%2' %3, multiplier %4, processing time %5" ).arg(
//																	pSrcDHWSys->getName(), pCSEDHWSysObj->getName(), (iHPWHSzRetVal==0 ? "successful" : "failed"), QString::number(dWHMult, 'f', 4), QString::number(dTimeForHPWHSizing, 'f', 2) );
//									BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
//					}
//					if (sErrorMsg.isEmpty())
//					{	if (dWHMult <= 0.0)
//							sErrorMsg = QString( "Error performing %1 run HPWH sizing for DHWSys '%2' / cseDHWSYS '%3':  Multiplier = %4." ).arg( sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName(), QString::number( dWHMult ) );
//						else
//						{	dWHMult *= dStdHPWHSizingMult;	// SAC 12/26/18 - apply std design HPWH sizing multiplier
//							int iSetRetVal = BEMPX_SetBEMData( BEMPX_GetDatabaseID( "cseDHWSYS:CentralWHMult" ), BEMP_Flt, (void*) &dWHMult, BEMO_User, iSIdx );
//							if (iSetRetVal < 0)
//								sErrorMsg = QString( "Error performing %1 run HPWH sizing for DHWSys '%2' / cseDHWSYS '%3':  Setting of CentralWHMult (%4) returned %5." ).arg(
//																		sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName(), QString::number( dWHMult ), QString::number( iSetRetVal ) );

					double dRunMult;
					std::vector<double> daRunMults;
					QString sModelFileOnlyNoExt = sSzCSEFileOnlyNoExt.left( sSzCSEFileOnlyNoExt.length()-1 );
					QString sSzRun1CSEPathFile = QString( "%1%21.cse" ).arg( m_sProcessPath, sSzCSEFileOnlyNoExt );
					for (int iM=0; iM < lStdHPWHSzNumRuns; iM++)
					{	BEMPX_GetFloat( lDBID_SzRunMults+iM, dRunMult, 0.0, -1, iDHWSysObjIdx );		assert( dRunMult > 0 );
						if (dRunMult > 0)
						{	daRunMults.push_back( dRunMult );
							if (iM>0)
							{	// COPY first CSE input file to subsequent (unique) run filenames
								QString sSzCSEFile = QString( "%1%2%3.cse" ).arg( m_sProcessPath, sModelFileOnlyNoExt, QString::number(iM+1) );
								// Write CSE input file  (and store BEM details file)
								QString sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
												 "application before an updated file can be written.\n\nSelect 'Retry' to update the file "
												 "(once the file is closed), or \n'Abort' to abort the analysis." ).arg( "CSE input", sSzCSEFile );
								if (!OKToWriteOrDeleteFile( sSzCSEFile.toLocal8Bit().constData(), sMsg, m_bSilent ))
								{	if (m_bSilent)
										sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( "HPWH sizing CSE input", sSzCSEFile );
									else
										sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( "HPWH sizing CSE input", sSzCSEFile );
								}
								else if (!CopyFile( sSzRun1CSEPathFile.toLocal8Bit().constData(), sSzCSEFile.toLocal8Bit().constData(), FALSE ))
									sErrorMsg = QString( "Error:  Unable to copy run %1 cseDHWSYS '%2' HPWH sizing run CSE file:  '%3'  to:  '%4'" ).arg(
																	sRunID, pCSEDHWSysObj->getName(), sSzRun1CSEPathFile, sSzCSEFile );
								//else
								//	saModelFileOnlyNoExt.push_back( sSzCSEFile );
						}	}
						else
						{
// POST ERROR
					}	}
					int iHPWHSzRetVal = -1;
					if (sErrorMsg.isEmpty())
					{	iHPWHSzRetVal = CSE_PerformHPWHSizing_3Run( m_sCSEexe /*"CSE"*/, m_sCSEWthr, m_sModelPathOnly, sModelFileOnlyNoExt, m_sProcessPath,
																					daRunMults, iSIdx+1, sStdHPWHSzTDVTbl, lStdHPWHSzTDVCol, m_bVerbose, iModelType );		// sErrorMsg   // SAC 1/1/19 - version of sizing that involves 3 parallel sizing runs w/ fit to parabola to calculate optimum size
					}
					if (sErrorMsg.isEmpty() && iHPWHSzRetVal==0)
					{	// evaluat RULES to calculate final heater multiplier based on sizing run results already posted to the cseDHWSYS
						if (!CMX_EvaluateRuleset( "CSE_DHWSystem_CalculateHPWHMultiplier", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
															NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo,
															NULL /*QStringList* psaWarningMsgs*/, iCID_CSEDHWSys, iSIdx, 0 /*iEvalOnlyObjType*/ ))
							sErrorMsg = QString( "Error evaluating 'CSE_DHWSystem_CalculateHPWHMultiplier' rulelist on %1 run during HPWH sizing for DHWSys '%2' / cseDHWSYS '%3'" ).arg(
														sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName() );
						else
						{	double dWHMult=0.0;
					 		BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseDHWSYS:CentralWHMult" ), dWHMult, 0.0, -1, iSIdx );

									double dTimeForHPWHSizing = DeltaTime( tmHPWHSizingStartTime );
									QString sLogMsg = QString( "HPWH sizing for DHWSys '%1' / cseDHWSYS '%2' %3, multiplier %4, processing time %5" ).arg(
																	pSrcDHWSys->getName(), pCSEDHWSysObj->getName(), (iHPWHSzRetVal==0 ? "successful" : "failed"), QString::number(dWHMult, 'f', 4), QString::number(dTimeForHPWHSizing, 'f', 2) );
									BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );

							// roll back project data mods setup specifically for the system & heaters of the current system being sized
							if (!CMX_EvaluateRuleset( "CSE_DHWSystem_RestoreFollowingHPWHSizingRun", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
														NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo,
														NULL /*QStringList* psaWarningMsgs*/, iCID_CSEDHWSys, iSIdx, 0 /*iEvalOnlyObjType*/ ))
								sErrorMsg = QString( "Error evaluating 'CSE_DHWSystem_RestoreFollowingHPWHSizingRun' rulelist on %1 run during HPWH sizing for DHWSys '%2' / cseDHWSYS '%3'" ).arg(
															sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName() );
							for (iHtr=0; (sErrorMsg.isEmpty() && iHtr < (int) iaCSEDHWHtrIdx.size()); iHtr++)
							{	if (!CMX_EvaluateRuleset( "CSE_DHWHeater_RestoreFollowingHPWHSizingRun", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
													NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo,
													NULL /*QStringList* psaWarningMsgs*/, iCID_CSEDHWHeater, iaCSEDHWHtrIdx[iHtr], 0 /*iEvalOnlyObjType*/ ))
								{	BEMObject* pCSEDHWHtrObj = BEMPX_GetObjectByClass( iCID_CSEDHWHeater, iErr, iaCSEDHWHtrIdx[iHtr] );			assert( pCSEDHWHtrObj );
									sErrorMsg = QString( "Error evaluating 'CSE_DHWHeater_RestoreFollowingHPWHSizingRun' rulelist on %1 run during HPWH sizing for DHWSys '%2' / cseDHWSYS '%3', heater '%4'" ).arg(
																	sRunID, pSrcDHWSys->getName(), pCSEDHWSysObj->getName(), pCSEDHWHtrObj->getName() );
							}	}
					}	}
				}
			}
		}
		if (!sErrorMsg.isEmpty())		// write details file if HPWHSIZE run error encountered
		{	QString sDbgFileName = QString( "%1%2-dhwsz%3-Error.ibd-Detail" ).arg( m_sProcessPath, sProjFileAlone, QString::number(iSIdx+1) );
			BEMPX_WriteProjectFile( sDbgFileName.toLocal8Bit().constData(), BEMFM_DETAIL /*FALSE*/ );
		}
	}	// end of loop over each cseDHWSYS

	if (sErrorMsg.isEmpty() && !bFirstDHWSysBeingSized)		// roll back project data mods setup specifically for HPWHSIZE runs
	{	if (!CMX_EvaluateRuleset( "CSE_Project_RestoreHPWHSizingRun", m_bVerbose /*bReportToLog*/, FALSE /*bTagDataAsUserDefined*/, m_bVerbose,
											NULL /*plNumRuleEvals*/, NULL /*pdNumSeconds*/, NULL /*PLogMsgCallbackFunc pLogMsgCallbackFunc*/, m_pCompRuleDebugInfo ))
			sErrorMsg = QString( "Error evaluating 'CSE_Project_RestoreHPWHSizingRun' rulelist on %1 run during HPWH sizing process" ).arg( sRunID );
	}

	return (sErrorMsg.isEmpty());
}


int CSERunMgr::SetupRunFinish(
	int iRunIdx, QString& sErrorMsg, const char* sCSEFileCopy /*=NULL*/ )
{
	int iRetVal = 0;
	QString sMsg, sLogMsg;

	CSERun* pCSERun = (iRunIdx < (int) m_vCSERun.size() ? m_vCSERun[iRunIdx] : NULL);
	if (pCSERun == NULL)
	{	assert( false );
		return BEMAnal_CECRes_SimInputWriteError;
	}

//	long lRunNumber;
	QString sRunID, sRunIDProcFile, sRunAbbrev;
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunID"         ),  sRunID         );
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunIDProcFile" ),  sRunIDProcFile );
	BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:RunAbbrev"     ),  sRunAbbrev     );

	QString sProjFileAlone = m_sModelFileOnlyNoExt + sRunIDProcFile;

		QString sTDVFName = pCSERun->GetTDVFName();		// SAC 4/16/17
		if (sTDVFName.length() > 0)
			BEMPX_SetBEMData( BEMPX_GetDatabaseID( "cseTOP:tdvfName" ), BEMP_QStr, (void*) &sTDVFName );
		else
			BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "cseTOP:tdvfName" ), m_iError );

// SAC 12/14/16 - code to confirm need for CSE Battery PRE-RUN
		QString sProjCSEBattFile, sCSEBattCtrlCSVFile;
		BEMObject* pBattCtrlImpFileObj = NULL;
		if (iRetVal == 0)
		{	long lSecondBattSimReqd;
			if (BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:SecondBattSimReqd" ), lSecondBattSimReqd ) && lSecondBattSimReqd > 0)
			{	// modify name of CSV file referenced by Proj:BattCtrlImportFile
				if (BEMPX_GetObject( BEMPX_GetDatabaseID( "Proj:BattCtrlImportFile" ), pBattCtrlImpFileObj ) && pBattCtrlImpFileObj && pBattCtrlImpFileObj->getClass())
				{	int iBCIFObjIdx = BEMPX_GetObjectIndex( pBattCtrlImpFileObj->getClass(), pBattCtrlImpFileObj );		assert( iBCIFObjIdx >= 0 );
					sCSEBattCtrlCSVFile = sProjFileAlone + "-BTCtrl.csv";
					if (iBCIFObjIdx < 0)
					{	assert( false );	// Proj:BattCtrlImportFile object not found 
					}
					else if (BEMPX_SetBEMData( BEMPX_GetDatabaseID( "cseIMPORTFILE:imFileName" ), BEMP_QStr, (void*) &sCSEBattCtrlCSVFile, BEMO_User, iBCIFObjIdx ) < 0)
					{	assert( false );	// error setting Proj:BattCtrlImportFile:imFileName string
					}
					else
						sProjCSEBattFile = m_sProcessPath + sProjFileAlone + "-BTPreRun.cse";
			}	}
		}

		double btMaxCap, btMaxChgPwr, btMaxDschgPwr, btChgEff, btDschgEff;	// SAC 1/23/17 - moved up from below to ensure BATTERY inputs retrieved from BEMBase BEFORE battery is blasted for pre-run CSE writing
		if (!sProjCSEBattFile.isEmpty())
		{	// then need to feed BTPreRun results & TDV data into calc_bt_control.exe to produce ...BTCtrl.csv battery control CSV which the second/final simulation will read
			if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btMaxCap"      ), btMaxCap ))
				btMaxCap      = 16;	// CSE default
			if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btMaxChgPwr"   ), btMaxChgPwr ))
				btMaxChgPwr   = 4;	// CSE default
			if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btMaxDschgPwr" ), btMaxDschgPwr ))
				btMaxDschgPwr = 4;	// CSE default
			if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btChgEff"      ), btChgEff ))
				btChgEff      = 0.975;	// CSE default
			if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btDschgEff"    ), btDschgEff ))
				btDschgEff    = 0.975;	// CSE default
		}

		QString sProjCSEFile;
		if (iRetVal == 0)
		{	sProjCSEFile = m_sProcessPath + sProjFileAlone + ".cse";
			int iLastCSEIdx = (sProjCSEBattFile.isEmpty() ? 0 : 1);	// SAC 12/15/16
			for (int iFLp=0; (iRetVal == 0 && iFLp <= iLastCSEIdx); iFLp++)
			{
				if (iFLp==1)	// SAC 12/15/16
				{	// Have already written the FINAL CSE input (including Battery) - now we are writing the Battery PRE-RUN CSE input, so delete the battery stuff before continuing
					if (pBattCtrlImpFileObj)
						BEMPX_DeleteObject( pBattCtrlImpFileObj );
         		int iBattObjErr;
            	BEMObject* pBattObj = BEMPX_GetObjectByClass( BEMPX_GetDBComponentID( "cseBATTERY" ), iBattObjErr, 0 );
            	if (pBattObj)
						BEMPX_DeleteObject( pBattObj );

				// SAC 9/4/17 - toggle OFF ReportIncludeFile for BTPreRun CSE run - in this case defaulting to '1' (prevent in BTPreRun)
					long lProjReportIncludeFileDBID = BEMPX_GetDatabaseID( "Proj:ReportIncludeFile" ), lReportInclPropOnly = 1;
					QString sChkRptIncFile;
					if (lProjReportIncludeFileDBID > 0 && BEMPX_GetString( lProjReportIncludeFileDBID, sChkRptIncFile ) && !sChkRptIncFile.isEmpty() &&
						 !BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:ReportInclPropOnly" ), lReportInclPropOnly ))
						lReportInclPropOnly = 1;
					if (!sChkRptIncFile.isEmpty() && lReportInclPropOnly > 0)
						BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
				}

				// SAC 12/12/18 - code to initiate HPWH Sizing run(s) when called for (HPWHSIZE)
				long lCSE_HPWHSizingReqd=0, lHPWHSizInProc=0;
				long lDBID_HPWHSizInProc = BEMPX_GetDatabaseID( "Proj:CSE_HPWHSizInProc" );	// SAC 12/18/18
				if (lDBID_HPWHSizInProc > 0 && (!BEMPX_GetInteger( lDBID_HPWHSizInProc, lHPWHSizInProc ) || lHPWHSizInProc < 1) &&
					 BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:CSE_HPWHSizingReqd" ), lCSE_HPWHSizingReqd ) && lCSE_HPWHSizingReqd > 0)
				{	// set flag indicating that we are performing HPWHSizing (otherwise we get stuck in an infinite loop during setup of CSE run initiated by T24Res_HPWHSizing()
					lHPWHSizInProc = 1;
					BEMPX_SetBEMData( lDBID_HPWHSizInProc, BEMP_Int, (void*) &lHPWHSizInProc );
				// PERFORM HPWH Sizing
					if (!T24Res_HPWHSizing( sProjFileAlone, sRunID, sErrorMsg ))
						iRetVal = BEMAnal_CECRes_HPWHSizingError;
					// re-default (blast) flag indicating that we are performing HPWHSizing
					BEMPX_DefaultProperty( lDBID_HPWHSizInProc, m_iError );
				}

				QString sLpCSEFile = (iFLp==0 ? sProjCSEFile : sProjCSEBattFile);
				// Write CSE input file  (and store BEM details file)
				sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
							 "application before an updated file can be written.\n\nSelect 'Retry' to update the file "
								 "(once the file is closed), or \n'Abort' to abort the analysis." ).arg( "CSE input", sLpCSEFile );
				if (!OKToWriteOrDeleteFile( sLpCSEFile.toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( "CSE input", sLpCSEFile );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( "CSE input", sLpCSEFile );
					iRetVal = BEMAnal_CECRes_SimInputOpenError;
				}
				else if (!BEMPX_WriteProjectFile( sLpCSEFile.toLocal8Bit().constData(), BEMFM_INPUT /*TRUE*/, FALSE /*bUseLogFileName*/,
											FALSE /*bWriteAllProperties*/, FALSE /*bSupressAllMessageBoxes*/, BEMFT_CSE /*iFileType*/, false /*bAppend*/,
											NULL /*ModelName*/, true /*WriteTerminator*/, -1 /*BEMProcIdx*/, -1 /*ModDate*/, false /*OnlyValidInputs*/,
											true /*AllowCreateDateReset*/, 1 /*PropertyCommentOption*/ ))			// SAC 12/5/16 - added to enable files to include comments: 0-none / 1-units & long name / 
				{	sErrorMsg = QString( "ERROR:  Unable to write %1 file:  %2" ).arg( "CSE input", sLpCSEFile );
					iRetVal = BEMAnal_CECRes_SimInputWriteError;
				}
				else
				{	SetCurrentDirectory( m_sProcessPath.toLocal8Bit().constData() );
					if (sCSEFileCopy && strlen( sCSEFileCopy ) > 0)
					{	QString sCSECopyFileName = sLpCSEFile.left( sLpCSEFile.length()-4 );
						sCSECopyFileName += sCSEFileCopy;		sCSECopyFileName += ".cse";
						CopyFile( sLpCSEFile.toLocal8Bit().constData(), sCSECopyFileName.toLocal8Bit().constData(), FALSE );
					}
					if (m_bStoreBEMProcDetails)
					{	QString sDbgFileName = sLpCSEFile.left( sLpCSEFile.length()-3 );
						sDbgFileName += "ibd-Detail";
						BEMPX_WriteProjectFile( sDbgFileName.toLocal8Bit().constData(), BEMFM_DETAIL /*FALSE*/ );
						if (!m_sCodeYear2Digit.isEmpty())
						{	sDbgFileName = QString( "%1ribd%2i" ).arg( sLpCSEFile.left( sLpCSEFile.length()-3 ) ).arg( m_sCodeYear2Digit );	// SAC 5/17/19 - added export of 'input' version of each analysis model
			      		BEMPX_WriteProjectFile( sDbgFileName.toLocal8Bit().constData(), BEMFM_INPUT, false /*bUseLogFileName*/, false /*bWriteAllProperties*/,
            								FALSE /*bSupressAllMessageBoxes*/, 0 /*iFileType*/, false /*bAppend*/, NULL /*pszModelName*/,
            								true /*bWriteTerminator*/, -1 /*iBEMProcIdx*/, -1 /*lModDate*/, false /*bOnlyValidInputs*/,
												true /*bAllowCreateDateReset*/, 0 /*iPropertyCommentOption*/, m_plaRIBDIClsObjIndices, false /*bReportInvalidEnums*/ );	// SAC 5/20/19
					}	}
				}
				BEMPX_RefreshLogFile();	// SAC 5/19/14
			}	// end of iFLp loop
		}

	// Perform TDV export and Battery simulation PRE-RUN
		if (iRetVal == 0 && m_bPerformSimulations && !m_bBypassCSE && !sProjCSEBattFile.isEmpty())	// SAC 12/15/16
		{	bool bBattCtrlSetup = false;
#ifdef CM_QTGUI
				if (sqt_win && sqt_progress)
					sqt_win->repaint();
#endif

		// First need to simulate the BTPreRun CSE input...
			QString sCSEParams, sProjCSEFileForArg = sProjCSEBattFile;
			if (!sProjCSEFileForArg.right(4).compare(".CSE", Qt::CaseInsensitive))
				sProjCSEFileForArg = sProjCSEFileForArg.left( sProjCSEFileForArg.length()-4 );
			sCSEParams = QString( "-b \"%1\"" ).arg( sProjCSEFileForArg );
			int iCSEExitCode = ExecuteNow( this, m_sCSEexe, sCSEParams );
			if (iCSEExitCode != 0)
			{	assert( false );
// how to handle errant BTPreRun simulation ??
			}
			else
			{			
		// then need to write CSV of selected TDV series
				double daTDVData[8760], daTDVSecData[8760];		long lClimateZone=0;		bool bHaveSecTDV=false;
				if (	!BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:ClimateZone" ), lClimateZone ) ||
						BEMPX_GetTableColumn( &daTDVData[0], 8760, "TDVTable", ((lClimateZone-1) * 3) + 1, NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) != 0)
				{	assert( false );
// how to handle error retrieving climate zone or TDV data ??
				}
				else
				{	// SAC 9/23/17 - added code to summ GHG TDV adders to TDV data stored to 
					QString qsTDVSecTblElec;
					bHaveSecTDV = ( BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:TDVSecTbl_Elec" ), qsTDVSecTblElec ) && 
										 qsTDVSecTblElec.length() > 0 && qsTDVSecTblElec.compare( "none", Qt::CaseInsensitive ) && 
										 BEMPX_GetTableColumn( &daTDVSecData[0], 8760, qsTDVSecTblElec.toLocal8Bit().constData(), lClimateZone, NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) == 0 );
					bool bTDVDataOK = true;
					QString sTDVCSVFile = m_sProcessPath + sProjFileAlone + "-tdvelec.csv";  // switched TDV filename here to "tdvelec" to not conflict w/ new 2-col CSV above - SAC 3/18/17
					FILE *fp_CSV;
					int iErrorCode;
					try
					{	iErrorCode = fopen_s( &fp_CSV, sTDVCSVFile.toLocal8Bit().constData(), "wb" );
						if (iErrorCode != 0 || fp_CSV == NULL)
						{	assert( false );
							bTDVDataOK = false;
// how to handle error TDV CSV file ??
						}
						else
						{	fprintf( fp_CSV, "\"TDV Data\",001\n" );
							fprintf( fp_CSV, "\"Wed 14-Dec-16   9:39:00 am\",\n" );
							fprintf( fp_CSV, "\"TDV [TDV/Btu]\",\"Hour\"\n" );		// SAC 3/10/17 - fixed incorrect units label, was: TDV [kBtu/kWh]
							fprintf( fp_CSV, "\"tdv\"\n" );
							if (bHaveSecTDV)	// SAC 9/23/17
							{	for (int hr=0; hr<8760; hr++)
									fprintf( fp_CSV, "%g\n", (daTDVData[hr] + (daTDVSecData[hr]/3.413)) );
							}
							else
							{	for (int hr=0; hr<8760; hr++)
									fprintf( fp_CSV, "%g\n", daTDVData[hr] );
							}
							fflush( fp_CSV );
							fclose( fp_CSV );
						}
					}
					catch( ... )
					{	assert( false );
						bTDVDataOK = false;
// how to handle error writing to TDV CSV file ??
					}

					if (bTDVDataOK)
					{
		// then need to feed BTPreRun results & TDV data into calc_bt_control.exe to produce ...BTCtrl.csv battery control CSV which the second/final simulation will read
						// SAC 1/23/17 - now done above, since at this point cseBATTERY obejct has been deleted from BEMBase (to write the BTPreRun CSE input)
						//double btMaxCap, btMaxChgPwr, btMaxDschgPwr, btChgEff, btDschgEff;
						//if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btMaxCap"      ), btMaxCap ))
						//	btMaxCap      = 16;	// CSE default
						//if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btMaxChgPwr"   ), btMaxChgPwr ))
						//	btMaxChgPwr   = 4;	// CSE default
						//if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btMaxDschgPwr" ), btMaxDschgPwr ))
						//	btMaxDschgPwr = 4;	// CSE default
						//if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btChgEff"      ), btChgEff ))
						//	btChgEff      = 0.975;	// CSE default
						//if (!BEMPX_GetFloat( BEMPX_GetDatabaseID( "cseBATTERY:btDschgEff"    ), btDschgEff ))
						//	btDschgEff    = 0.975;	// CSE default

						QString sCBCexe = m_sCSEexe.left( std::max( m_sCSEexe.lastIndexOf('/'), m_sCSEexe.lastIndexOf('\\') )+1 );
						sCBCexe += "calc_bt_control.exe";
						QString sCBCParams = QString( "\"%1.csv\" \"%2\" \"%3\" %4 %5 %6 %7 %8" ).arg( sProjCSEBattFile.left( sProjCSEBattFile.length()-4 ), sCSEBattCtrlCSVFile, sTDVCSVFile,
														QString::number( btMaxCap ), QString::number( btMaxChgPwr ), QString::number( btMaxDschgPwr ), QString::number( btChgEff ), QString::number( btDschgEff ) );
											//  dist\calc_bt_control.exe test\1STORY.CSV test\BT_CONTROL.CSV test\tdv.csv 16.320 4.08 4.08 0.9747 0.9747
											//  Arguments:
											//  (1) path to input meter CSV file
											//  (2) path to output control CSV file
											//  (3) path to input TDV CSV file
											//  (4) Battery maximum usable capacity (kWh)
											//  (5) Battery maximum charging power (kW)
											//  (6) Battery maximum discharging power (kW)
											//  (7) Battery charge efficiency (0 < eff <= 1)
											//  (8) Battery discharge efficiency (0 < eff <= 1)
							// if verbose logging only ??
									QString sCBCLogMsg = QString( "   Processing BTPreRun results (via calc_bt_control):  %1" ).arg( sCBCParams );
									BEMPX_WriteLogFile( sCBCLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
//	QString sDbgMsg = QString( "about to run calc_bt_control:\n%1" ).arg( sCBCParams );
//	BEMMessageBox( sDbgMsg );

						int iCBCExitCode = ExecuteNow( this, sCBCexe, sCBCParams );
						if (iCBCExitCode != 0)
						{	assert( false );
// how to handle error running calc_bt_control ??
						}
						else
							bBattCtrlSetup = true;
//	sDbgMsg = QString( "calc_bt_control exit code: %1" ).arg( QString::number(iCBCExitCode) );
//	BEMMessageBox( sDbgMsg );
					}
				}
			}

			if (!bBattCtrlSetup)
				iRetVal = BEMAnal_CECRes_CSEBattCtrlSetupErr;
		}

		if (iRetVal == 0 && m_bPerformSimulations && !m_bBypassCSE)
		{
			// remove trailing '.cse' file extension from path/file passed into CSE
			QString sProjCSEFileForArg = sProjCSEFile;
			if (!sProjCSEFileForArg.right(4).compare(".CSE", Qt::CaseInsensitive))
				sProjCSEFileForArg = sProjCSEFileForArg.left( sProjCSEFileForArg.length()-4 );

			// CSE SIMULATION
			QString sParams;
			//	sParams = QString( "-b \"%s.cse\"" ).arg( sProjFileAlone );		// SAC 8/20/12 - added '-b' (batch) option to prevent user prompt in the event of sim error
			sParams = QString( "-b \"%1\"" ).arg( sProjCSEFileForArg );
	#ifdef _DEBUG
			sLogMsg = QString( "   CSE exePath:  %1" ).arg( m_sCSEexe );
			BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
			sLogMsg = QString( "   CSE cmdLine:  %1" ).arg( sParams );
			BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
	#endif
			pCSERun->SetArgs( sParams);
		}

	return iRetVal;
}		// CSERunMgr::SetupRunFinish

int CSERunMgr::SetupRun_NonRes(int iRunIdx, int iRunType, QString& sErrorMsg, bool bAllowReportIncludeFile /*=true*/,		// SAC 5/24/16
											const char* pszRunID /*=NULL*/, const char* pszRunAbbrev /*=NULL*/, QString* psCSEVer /*=NULL*/, int iBEMProcIdx /*=-1*/,
											bool bRemovePVBatt /*=false*/ )
{
	int iRetVal = 0;
#ifdef OSWRAPPER
	int iPrevBEMProcIdx = -1;	// SAC 7/23/18
	if (iBEMProcIdx >= 0)
	{	iPrevBEMProcIdx = BEMPX_GetActiveModel();
		BEMPX_SetActiveModel( iBEMProcIdx );
	}
	CSERun* pCSERun = new CSERun;
	m_vCSERun.push_back( pCSERun);
	QString sMsg, sLogMsg;
	m_iNumRuns = 1;		// SAC 5/24/16 - not running parallel DHW simulations for non-res (for now)
	BOOL bLastRun = (iRunIdx == (m_iNumRuns-1));						assert( bLastRun );	// remove if/when we do parallel runs in -Com...
	BOOL bIsStdDesign = (iRunType == CRM_StdDesign);
	pCSERun->SetRunType( iRunType);
	pCSERun->SetLastRun( bLastRun);
	pCSERun->SetIsStdDesign( bIsStdDesign);

	QString sRunID			= pszRunID;
	QString sRunAbbrev	= pszRunAbbrev;
	QString sRunIDProcFile = " - " + sRunAbbrev + QString("-cse");		// add '-cse' to ensure no conflicts w/ files to/from other engines
	QString sProjFileAlone = m_sModelFileOnlyNoExt + sRunIDProcFile;

	if (m_bStoreBEMProcDetails)
	{	QString sDbgFileName;
		sDbgFileName = QString( "%1.ibd-b4Evals" ).arg( sProjFileAlone );
		BEMPX_WriteProjectFile( sDbgFileName.toLocal8Bit().constData(), BEMFM_DETAIL /*FALSE*/ );
	}

// -Com:
//		; Analysis Data
//           "RunTitle",                          BEMP_Str,  1,  0,  0,   Opt,  "",                 0,  0,                           1044, "RunTitle",  "" 
//           "AnalysisType",                      BEMP_Sym,  1,  0,  0,   Req,  "",                 0,  0,                           1003, "AnalysisType",  ""   
//           "CompType",                          BEMP_Sym,  1,  0,  0,   Req,  "",                 0,  0,                           1003, "ComplianceType",  ""

	long lRunNumber = (iRunType == CRM_User ? 0 : iRunIdx+1 );
//	BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:RunNumber" ), BEMP_Int, (void*) &lRunNumber );
//
//	QString sOrientLtr, sOrientName;
//	if (iRunType < CRM_StdDesign /*!bIsStdDesign*/ && m_lAnalysisType > 0)
//		iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalPropCompError, "ProposedCompliance", m_bVerbose, m_pCompRuleDebugInfo );
//	else if (iRunType >= CRM_StdDesign)	// SAC 3/27/15 - was:  bIsStdDesign)
//	{	// SAC 3/27/15 - SET 
//		if (iRunType == CRM_DesignRating)
//      	BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:StdDesignBase" ), BEMP_Int, (void*) &m_lDesignRatingRunID );
//		// may at some point need to specifically set  StandardsVersion  if/when multiple standard vintages is supported by a single ruleset
//
//		iRetVal = LocalEvaluateRuleset( sErrorMsg, BEMAnal_CECRes_EvalStdConvError, "BudgetConversion", m_bVerbose, m_pCompRuleDebugInfo );
//	}
//	if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
//		iRetVal = BEMAnal_CECRes_RuleProcAbort;

	pCSERun->SetRunNumber( lRunNumber);
	pCSERun->SetRunID( sRunID);
	pCSERun->SetRunIDProcFile( sRunIDProcFile);
	pCSERun->SetRunAbbrev( sRunAbbrev);

//	if (iRetVal == 0 && iRunType >= CRM_NOrientProp && iRunType <= CRM_WOrientProp)		// SAC 3/27/15 - was:  m_bFullComplianceAnalysis && m_lAllOrientations > 0 && !bLastRun)
//	{  // Set Orientation for this particular run
//		BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:OrientAbbrev" ),  sOrientLtr  );
//		BEMPX_GetString(  BEMPX_GetDatabaseID( "Proj:OrientName"   ),  sOrientName );
//	}
//
//	QString sMissing;
//	if (sRunID.isEmpty())
//		sMissing += "RunID, ";
//	if (sRunAbbrev.isEmpty())
//		sMissing += "RunAbbrev ";
//	if (iRetVal == 0 && !sMissing.isEmpty())
//	{	sMissing = sMissing.left( sMissing.length()-2 );
//		sErrorMsg = "ERROR:  The following Proj properties not set:" + sMissing;
//		iRetVal = BEMAnal_CECRes_GetReqdDataError;
//	}
//	assert( iRunIdx == (lRunNumber-1) );
//
//	if (iRetVal == 0 && (m_sProcessPath.length() + m_sModelFileOnlyNoExt.length() + sRunIDProcFile.length() + 12) > _MAX_PATH)
//	{	sErrorMsg = QString( "ERROR:  CSE processing path too long:  %1%2%3.*" ).arg( m_sProcessPath, m_sModelFileOnlyNoExt, sRunIDProcFile );
//		iRetVal = BEMAnal_CECRes_ProcPathTooLong;
//	}
//
//	if (iRetVal == 0 && iRunIdx == 0 && m_bInitHourlyResults)  // only initialize hourly results for first simulation??
//		BEMPX_InitializeHourlyResults();

	if (iRetVal == 0)
	{
//		// try evaluating prop & postprop rulelists EVERY time
//		iRetVal = LocalEvaluateRuleset(		sErrorMsg, BEMAnal_CECRes_EvalPropInp3Error, "ProposedInput", m_bVerbose, m_pCompRuleDebugInfo );		// generic project defaulting
//		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
//			iRetVal = BEMAnal_CECRes_RuleProcAbort;
//
//		if (iRetVal == 0) // && iRunIdx == 0)
//			iRetVal = LocalEvaluateRuleset(	sErrorMsg, BEMAnal_CECRes_EvalSimChkError, "ProposedModelSimulationCheck", m_bVerbose, m_pCompRuleDebugInfo );		// check user input model for simulation-related errors
//		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
//			iRetVal = BEMAnal_CECRes_RuleProcAbort;
//
//		if (iRetVal == 0)
//			iRetVal = LocalEvaluateRuleset(	sErrorMsg, BEMAnal_CECRes_EvalPostPropError, "PostProposedInput", m_bVerbose, m_pCompRuleDebugInfo );		// setup for Proposed run
//		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
//			iRetVal = BEMAnal_CECRes_RuleProcAbort;

		if (bRemovePVBatt)		// SAC 4/3/19 - added code to remove all 
		{	const char* pszClassesToDel[] = { "Batt", "PVArray", NULL };
			int iCTD=-1;
			while (pszClassesToDel[++iCTD] != NULL)
			{	int iCTDClassID = BEMPX_GetDBComponentID( pszClassesToDel[iCTD] );		assert( iCTDClassID > 0 );
				if (iCTDClassID > 0)
				{	int iCTDError, iNumObjs = BEMPX_GetNumObjects( iCTDClassID );
					for (int iCTDO=iNumObjs-1; iCTDO >= 0; iCTDO--)
					{
						BEMObject* pCTDObj = BEMPX_GetObjectByClass( iCTDClassID, iCTDError, iCTDO );		assert( pCTDObj );
						if (pCTDObj)
							BEMPX_DeleteObject( pCTDObj );
			}	}	}
		}

		if (iRetVal == 0)
			iRetVal = LocalEvaluateRuleset( sErrorMsg, 63, "CSE_SimulationPrep", m_bVerbose, m_pCompRuleDebugInfo );
//											63 : Error evaluating CSE_SimulationPrep rules
		if (iRetVal == 0 && BEMPX_AbortRuleEvaluation())
			iRetVal = 63;

		BEMPX_RefreshLogFile();	// SAC 5/19/14

		if (iRetVal == 0 && psCSEVer)
		{
//		if (iRetVal == 0 && iRunIdx == 0)  // Store various software & ruleset versions prior to first run - SAC 12/19/12
//		{	QString sVerTemp, sVerStr;
//			BEMPX_GetRulesetID( sVerTemp, sVerStr );
//			if (!sVerStr.isEmpty())
//				BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:RulesetVersion" ), BEMP_QStr, (void*) &sVerStr );
//			else
//				BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:RulesetVersion" ), m_iError );
//
			QString sCSEVersion = GetVersionInfo();
			if (!sCSEVersion.isEmpty())
			{	*psCSEVer = sCSEVersion;
				BEMPX_SetBEMData( BEMPX_GetDatabaseID( "Proj:DHWSimEngVersion" ), BEMP_QStr, (void*) &sCSEVersion );	// SAC 8/23/17 - added storage of CSE sim ver (in place of old T24DHW engine)
			}
			else
				BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:DHWSimEngVersion" ), m_iError );

//			// SAC 1/8/13 - DHW engine version stored to BEMBase DURING DHW simulation (immediately following the ...Init() call)
//			BEMPX_DefaultProperty( BEMPX_GetDatabaseID( "Proj:SecSimEngingVer" ), m_iError );
		}

//		// Purge user-defined non-parent/child components which are not referenced  - SAC 1/20/13 - added to prevent simulation of objects that are not referenced in the building model
//		// Purge performed HERE, before objects are cross-referenced (Cons <-> cseCONS & Mat <-> cseMATERIAL), w/ those cross-references preventing any useful purging
//		BEMPX_PurgeUnreferencedComponents();

		QString sOutFiles[CSERun::OutFileCOUNT];
		if (iRetVal == 0)
		{	sOutFiles[CSERun::OutFileCSV] = m_sProcessPath + sProjFileAlone + ".csv";
			sOutFiles[CSERun::OutFileREP] = m_sProcessPath + sProjFileAlone + ".rep";
			sOutFiles[CSERun::OutFileERR] = m_sProcessPath + sProjFileAlone + ".err";
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileCSV], CSERun::OutFileCSV);
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileREP], CSERun::OutFileREP);
			pCSERun->SetOutFile( sOutFiles[CSERun::OutFileERR], CSERun::OutFileERR);

			static const char* pszOutFileDescs[] = { "CSV output", "REP output", "Error output" };
			int i=0;
			for (; (i<CSERun::OutFileCOUNT && iRetVal == 0); i++)
			{	sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
								"application before an updated file can be written.\n\nSelect 'Retry' to proceed "
								"(once the file is closed), or \n'Abort' to abort the analysis." ).arg( pszOutFileDescs[i], sOutFiles[i] );
				if (!OKToWriteOrDeleteFile( sOutFiles[i].toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( pszOutFileDescs[i], sOutFiles[i] );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( pszOutFileDescs[i], sOutFiles[i] );
					iRetVal = 62;		//  Unable to open/delete/write simulation output file (.csv or .rep)
				}
				// code to DELETE existing results files prior to simulation
				else if (FileExists( sOutFiles[i].toLocal8Bit().constData() ))
					DeleteFile( sOutFiles[i].toLocal8Bit().constData() );
			}
		}

		QString sRptIncFile, sProcRptIncFile, sZoneIncFile, sProcZoneIncFile;		QVector<QString> saZoneIncFiles, saProcZoneIncFiles, saZoneNameIncFiles;
		if (iRetVal == 0)
		{	// Write Report Include file statement
			long lProjReportIncludeFileDBID = BEMPX_GetDatabaseID( "Proj:CSE_RptIncFile" );
			if (bAllowReportIncludeFile && lProjReportIncludeFileDBID > 0 && BEMPX_GetString( lProjReportIncludeFileDBID, sRptIncFile ) && !sRptIncFile.isEmpty())
			{	BOOL bOrigRptIncFileLastSlashIdx = sRptIncFile.lastIndexOf('\\');
				if (sRptIncFile.indexOf(':') < 0 && sRptIncFile.indexOf('\\') != 0)
					// report file does not include a FULL path, so expected to be a path relative to the project file - so prepend project path...
					sRptIncFile = m_sModelPathOnly + sRptIncFile;
				if (!FileExists( sRptIncFile.toLocal8Bit().constData() ))
				{	sLogMsg = QString( "CSE report include file not found '%1'" ).arg( sRptIncFile );
					BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
					//WriteToLogFile( WM_LOGUPDATED, sRptFileErr );
					BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
					sRptIncFile.clear();
				}
				else
				{	QString sRptIncFileNoPath = sRptIncFile.right( sRptIncFile.length() - sRptIncFile.lastIndexOf('\\') - 1 );
					if (bOrigRptIncFileLastSlashIdx >= 0)
						// need to CHANGE report include file setting to reference only the filename (no path info)
						BEMPX_SetBEMData( lProjReportIncludeFileDBID, BEMP_QStr, (void*) &sRptIncFileNoPath );
					sProcRptIncFile = m_sProcessPath + sRptIncFileNoPath;
				}
				if (!sRptIncFile.isEmpty() && sRptIncFile.compare( sProcRptIncFile, Qt::CaseInsensitive ) == 0)
				{	// if we have a valid report include file but both the source and destination path\filenames are identical, then empty them, as no copying is required
					sRptIncFile.clear();
					sProcRptIncFile.clear();
				}
			}

//			// SAC 7/2/15 - added logic here to check for and address usage of Zone:TstatIncludeFile
//			long lUseTstatIncFile, lDBID_Zone_UseTstatIncFile = BEMPX_GetDatabaseID( "Zone:UseTstatIncFile" ), lDBID_Zone_TstatIncludeFile = BEMPX_GetDatabaseID( "Zone:TstatIncludeFile" );
//			int iNumZones = (lDBID_Zone_UseTstatIncFile < 1 ? 0 : BEMPX_GetNumObjects( BEMPX_GetClassID( lDBID_Zone_UseTstatIncFile ) ));
//			for (int iZnIdx=0; (iRetVal == 0 && iZnIdx < iNumZones); iZnIdx++)
//			{	if (BEMPX_GetInteger( lDBID_Zone_UseTstatIncFile,  lUseTstatIncFile,    0, -1, iZnIdx ) && lUseTstatIncFile > 0 &&
//					 BEMPX_GetString(  lDBID_Zone_TstatIncludeFile, sZoneIncFile, FALSE, 0, -1, iZnIdx ) && !sZoneIncFile.isEmpty() && StringInArray( saZoneIncFiles, sZoneIncFile ) < 0)
//				{	//BOOL bOrigZoneIncFileLastSlashIdx = sZoneIncFile.lastIndexOf('\\');
//					if (sZoneIncFile.indexOf(':') < 0 && sZoneIncFile.indexOf('\\') != 0)
//						// include file does not include a FULL path, so expected to be a path relative to the project file - so prepend project path...
//						sZoneIncFile = m_sModelPathOnly + sZoneIncFile;
//					if (!FileExists( sZoneIncFile ))
//					{	if (!BEMPX_GetString( BEMPX_GetDatabaseID( "Zone:Name" ), sProcZoneIncFile, FALSE, 0, -1, iZnIdx ) || sProcZoneIncFile.isEmpty())
//					 		sProcZoneIncFile = "<unknown>";
//						sLogMsg = QString( "ERROR:  CSE zone '%1' include file not found '%2'" ).arg( sProcZoneIncFile, sZoneIncFile );
//						BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
//						iRetVal = BEMAnal_CECRes_MissingZnCSEIncFile;
//						//BEMPX_DefaultProperty( lProjReportIncludeFileDBID, m_iError );
//						sZoneIncFile.clear();
//					}
//					else
//					{	QString sZoneIncFileNoPath = sZoneIncFile.right( sZoneIncFile.length() - sZoneIncFile.lastIndexOf('\\') - 1 );
//						sProcZoneIncFile = m_sProcessPath + sZoneIncFileNoPath;
//					}
//					if (!sZoneIncFile.isEmpty() && sZoneIncFile.compare( sProcZoneIncFile, Qt::CaseInsensitive ) == 0)
//					{	// if we have a valid report include file but both the source and destination path\filenames are identical, then empty them, as no copying is required
//						sZoneIncFile.clear();
//						sProcZoneIncFile.clear();
//					}
//					if (!sZoneIncFile.isEmpty() && !sProcZoneIncFile.isEmpty())
//					{	saZoneIncFiles.push_back( sZoneIncFile );
//						saProcZoneIncFiles.push_back( sProcZoneIncFile );
//					 	if (BEMPX_GetString( BEMPX_GetDatabaseID( "Zone:Name" ), sProcZoneIncFile, FALSE, 0, -1, iZnIdx ) && !sProcZoneIncFile.isEmpty())
//					 		saZoneNameIncFiles.push_back( sProcZoneIncFile );
//					 	else
//					 		saZoneNameIncFiles.push_back( "<unknown>" );
//				}	}
//			}
		}

		if (iRetVal == 0)
		{	// Copy weather & report include files into processing directory
			// ??? do this for each & every run ???
			QString sDestWthr;	int i;
			int iLastSlashIdx = m_sCSEWthr.lastIndexOf('\\');
			if (m_sCSEWthr.lastIndexOf('/') > iLastSlashIdx)	// SAC 5/27/16 - compatibility w/ paths having etiher forward or back slashes
				iLastSlashIdx = m_sCSEWthr.lastIndexOf('/');
			sDestWthr     = m_sProcessPath + m_sCSEWthr.right( m_sCSEWthr.length() - iLastSlashIdx - 1 ); // "CTZ12S13.CSW";
			const char* pszFileDescs[] = {	"CSE weather", 	"CSE report include" 												};
			QString* psaCopySrc[ ] = { 		&m_sCSEWthr,  		(sRptIncFile.isEmpty() ? NULL : &sRptIncFile), 		NULL	};
			QString* psaCopyDest[] = {			&sDestWthr, 		&sProcRptIncFile														};
			int      iaCopyError[] = {			 60,					64																			};
			for (i=0; (psaCopySrc[i] != NULL && iRetVal == 0); i++)
			{	sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
									"application before an updated file can be written.\n\nSelect 'Retry' to update the file "
									"(once the file is closed), or \n'Abort' to abort the analysis." ).arg( pszFileDescs[i], *psaCopyDest[i] );
				if (!OKToWriteOrDeleteFile( psaCopyDest[i]->toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( pszFileDescs[i], *psaCopyDest[i] );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( pszFileDescs[i], *psaCopyDest[i] );
					iRetVal = iaCopyError[i];
				}
				else if (!CopyFile( psaCopySrc[i]->toLocal8Bit().constData(), psaCopyDest[i]->toLocal8Bit().constData(), FALSE ))
				{	sErrorMsg = QString( "ERROR:  Unable to copy %1 file:  '%2'  to:  '%3'" ).arg( pszFileDescs[i], *psaCopySrc[i], *psaCopyDest[i] );
					iRetVal = iaCopyError[i]+1;
				}
			}
//		// similar loop as above but for Zone include files
//			for (i=0; (i < saZoneIncFiles.size() && iRetVal == 0); i++)
//			{	sMsg = QString( "The zone '%1' CSE include file '%2' is opened in another application.  This file must be closed in that "
//									"application before an updated file can be written.\n\nSelect 'Retry' to update the file "
//									"(once the file is closed), or \n'Cancel' to abort the analysis." ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
//				if (!OKToWriteOrDeleteFile( saProcZoneIncFiles[i], sMsg, m_bSilent ))
//				{	if (m_bSilent)
//						sErrorMsg = QString( "ERROR:  Unable to overwrite zone '%1' CSE include file:  %2" ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
//					else
//						sErrorMsg = QString( "ERROR:  User chose not to overwrite zone '%1' CSE include file:  %2" ).arg( saZoneNameIncFiles[i], saProcZoneIncFiles[i] );
//					iRetVal = BEMAnal_CECRes_CSEIncFIleWriteError;
//				}
//				else if (!CopyFile( saZoneIncFiles[i], saProcZoneIncFiles[i], FALSE ))
//				{	sErrorMsg = QString( "ERROR:  Unable to copy %1 file:  '%2'  to:  '%3'" ).arg( "CSE include", saZoneIncFiles[i], saProcZoneIncFiles[i] );
//					iRetVal = BEMAnal_CECRes_CSEIncFileCopyError;
//				}
//			}
		}

		if (iRetVal == 0)
		{	// remove path portion of weather filename (to be written to CSE input file) - SAC 5/27/16
			QString sWthrFN;
			long lDBID_CSEwfName = BEMPX_GetDatabaseID( "cseTOP:wfName" );
		 	if (lDBID_CSEwfName > 0 && BEMPX_GetString( lDBID_CSEwfName, sWthrFN ) && !sWthrFN.isEmpty())
		 	{	int iLastSlashIdx = sWthrFN.lastIndexOf('\\');
				if (sWthrFN.lastIndexOf('/') > iLastSlashIdx)	// SAC 5/27/16 - compatibility w/ paths having etiher forward or back slashes
					iLastSlashIdx = sWthrFN.lastIndexOf('/');
				if (iLastSlashIdx >= 0)
				{	sWthrFN = sWthrFN.right( sWthrFN.length() - iLastSlashIdx - 1 );
					BEMPX_SetBEMData( lDBID_CSEwfName, BEMP_QStr, (void*) &sWthrFN );
		}	}	}

	// SAC 7/16/18 - ported writing of TDV CSV file from Res setup to support BATTERY simulation
		// SAC 3/18/17 - storage of TDV CSV file to be made available to the CSE input file
		pCSERun->SetTDVFName( "" );		// SAC 4/16/17
		QString sTDVFile;
		if (iRetVal == 0)
		{	long lCSE_WriteTDV;
			long lDBID_CSEtdvfName = BEMPX_GetDatabaseID( "cseTOP:tdvfName" );	// SAC 8/14/18
			if (lDBID_CSEtdvfName > 0 && BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:CSE_WriteTDV" ), lCSE_WriteTDV ) && lCSE_WriteTDV > 0)
			{	sTDVFile = sProjFileAlone + "-tdv.csv";
				QString sFullTDVFile = m_sProcessPath + sTDVFile;
				sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
				             "application before an updated file can be written.\n\nSelect 'Retry' to update the file "
								 "(once the file is closed), or \n'Abort' to abort the %3." ).arg( "TDV", sFullTDVFile, "compliance report generation" );
				if (!OKToWriteOrDeleteFile( sFullTDVFile.toLocal8Bit().constData(), sMsg, m_bSilent ))
				{	if (m_bSilent)
						sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file (required for CSE simulation):  %2" ).arg( "TDV", sFullTDVFile );
					else
						sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file (required for CSE simulation):  %2" ).arg( "TDV", sFullTDVFile );
					iRetVal = 60;		// BEMAnal_CECRes_TDVFileWriteError;	//  Error writing CSV file w/ TDV data (required for CSE simulation)
				}
				else
				{	double daTDVElec[8760], daTDVFuel[8760];  //, daTDVSecElec[8760], daTDVSecFuel[8760];
					long lCZ=0, lGas=0;
					bool bHaveSecTDVElec=false, bHaveSecTDVFuel=false;
					if (	!BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:CliZnNum" ), lCZ  ) || lCZ  < 1 || lCZ > 16 ||
							!BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:GasType"  ), lGas ) || lGas < 1 || lGas > 2 ||
							BEMPX_GetTableColumn( &daTDVElec[0], 8760, "TDVbyCZandFuel", ((lCZ-1) * 3) + 2       , NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) != 0 ||
							BEMPX_GetTableColumn( &daTDVFuel[0], 8760, "TDVbyCZandFuel", ((lCZ-1) * 3) + 2 + lGas, NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) != 0 )
					{	assert( false );
						sErrorMsg = QString( "ERROR:  Unable to retrieve Electric TDV data for CZ %1 (required for CSE simulation)" ).arg( QString::number(lCZ) );
						iRetVal = 60;		// BEMAnal_CECRes_TDVFileWriteError;	//  Error writing CSV file w/ TDV data (required for CSE simulation)
					}
					else
					{	QDateTime locTime = QDateTime::currentDateTime();
						QString timeStamp = locTime.toString("ddd dd-MMM-yy  hh:mm:ss ap");   // "Wed 14-Dec-16  12:30:29 pm"

		//				// SAC 9/23/17 - added code to summ GHG TDV adders to TDV data stored to 
		//				QString qsTDVSecTblElec, qsTDVSecTblFuel;
		//				bHaveSecTDVElec = ( BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:TDVSecTbl_Elec" ), qsTDVSecTblElec ) && 
		//										  qsTDVSecTblElec.length() > 0 && qsTDVSecTblElec.compare( "none", Qt::CaseInsensitive ) && 
		//										  BEMPX_GetTableColumn( &daTDVSecElec[0], 8760, qsTDVSecTblElec.toLocal8Bit().constData(), lCZ, NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) == 0 );
		//				if (lGas == 1)
		//					BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:TDVSecTbl_NGas" ), qsTDVSecTblFuel );
		//				else
		//					BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:TDVSecTbl_Othr" ), qsTDVSecTblFuel );
		//				bHaveSecTDVFuel = ( qsTDVSecTblFuel.length() > 0 && qsTDVSecTblFuel.compare( "none", Qt::CaseInsensitive ) && 
		//										  BEMPX_GetTableColumn( &daTDVSecFuel[0], 8760, qsTDVSecTblFuel.toLocal8Bit().constData(), lCZ, NULL /*pszErrMsgBuffer*/, 0 /*iErrMsgBufferLen*/ ) == 0 );
		      		QString qsVer, qsGas;
						if (!BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:CompManagerVersion" ), qsVer ) || qsVer.length() < 1)
							qsVer = "(unknown version)";
						switch (lGas)
						{	case  1 :	qsGas = "NatGas";   break;
							case  2 :	qsGas = "Propane";  break;
							default :	qsGas = "(unknown)";  break;
						}

						FILE *fp_CSV;
						int iErrorCode;
						try
						{	iErrorCode = fopen_s( &fp_CSV, sFullTDVFile.toLocal8Bit().constData(), "wb" );
							if (iErrorCode != 0 || fp_CSV == NULL)
							{	assert( false );
								sErrorMsg = QString( "ERROR:  Unable to write TDV data file (required for CSE simulation):  %1" ).arg( sFullTDVFile );
								iRetVal = 60;		// BEMAnal_CECRes_TDVFileWriteError;	//  Error writing CSV file w/ TDV data (required for CSE simulation)
							}
							else
							{	fprintf( fp_CSV, "\"TDV Data (TDV/Btu)\",\"001\"\n" );
								fprintf( fp_CSV, "\"%s\"\n", timeStamp.toLocal8Bit().constData() );
		//						fprintf( fp_CSV, "\"%s, CZ%ld, Fuel Elec-only\",\"Hour\"\n", qsVer.toLocal8Bit().constData(), lCZ );
		//						fprintf( fp_CSV, "\"tdvElec\"\n" );
								fprintf( fp_CSV, "\"%s, CZ%ld, Fuel %s\",\"Hour\"\n", qsVer.toLocal8Bit().constData(), lCZ, qsGas.toLocal8Bit().constData() );
								fprintf( fp_CSV, "\"tdvElec\",\"tdvFuel\"\n" );
								for (int hr=0; hr<8760; hr++)
		//							fprintf( fp_CSV, "%g\n", daTDVElec[hr] );
		//						{	if (bHaveSecTDVElec)
		//								daTDVElec[hr] += (daTDVSecElec[hr] / 3.413);
		//							if (bHaveSecTDVFuel)
		//								daTDVFuel[hr] += (daTDVSecFuel[hr] / 100);
									fprintf( fp_CSV, "%g,%g\n", daTDVElec[hr], daTDVFuel[hr] );
		//						}
								fflush( fp_CSV );
								fclose( fp_CSV );
							}
						}
						catch( ... )
						{	assert( false );
							sErrorMsg = QString( "ERROR:  Exception thrown writing TDV data file (required for CSE simulation):  %1" ).arg( sFullTDVFile );
							iRetVal = 60;		// BEMAnal_CECRes_TDVFileWriteError;	//  Error writing CSV file w/ TDV data (required for CSE simulation)
					// how to handle error writing to TDV CSV file ??
						}
				}	}
				if (iRetVal == 0)
					pCSERun->SetTDVFName( sTDVFile );		// SAC 4/16/17
			}
			if (sTDVFile.length() > 0)
				BEMPX_SetBEMData( lDBID_CSEtdvfName, BEMP_QStr, (void*) &sTDVFile );
			else if (lDBID_CSEtdvfName > 0)
				BEMPX_DefaultProperty( lDBID_CSEtdvfName, m_iError );
		}

	// WRITE CSV FILE of E+ElecUse - SAC 7/25/18
		if (iRetVal == 0)
		{	int iSpecialVal, iError;
			BEMObject* pCSE_ElecUseImpFile = BEMPX_GetObjectPtr( BEMPX_GetDatabaseID( "Proj:CSE_ElecUseIMPORTFILE" ), iSpecialVal, iError );
		 	if (pCSE_ElecUseImpFile && pCSE_ElecUseImpFile->getClass())
		 	{	int iCSE_ElecUseImpFileObjIdx = BEMPX_GetObjectIndex( pCSE_ElecUseImpFile->getClass(), pCSE_ElecUseImpFile );		assert( iCSE_ElecUseImpFileObjIdx >= 0 );
				double* pdHrlyElecUse[ NUM_T24_NRES_EndUses ];
				int iNumHrlyElecUsePtrs = 0, iHrlyElecUsePtrRV, iEU;
				for (iEU=0; iEU < NUM_T24_NRES_EndUses; iEU++)
				{	pdHrlyElecUse[iEU] = NULL;
					if (iEU != IDX_T24_NRES_EU_CompTot && iEU != IDX_T24_NRES_EU_Total)
					{	iHrlyElecUsePtrRV = BEMPX_GetHourlyResultArrayPtr( &pdHrlyElecUse[iNumHrlyElecUsePtrs], NULL, 0, pszRunID, pszaEPlusFuelNames[0/*elec*/],
																							esEUMap_CECNonRes[iEU].sEnduseName, iBEMProcIdx );
						if (pdHrlyElecUse[iNumHrlyElecUsePtrs] && iHrlyElecUsePtrRV==0)
							iNumHrlyElecUsePtrs++;
				}	}

		//		double* pdHrlyTotElecUse = NULL;
		//		int iHrlyTotElecUsePtrRV = BEMPX_GetHourlyResultArrayPtr( &pdHrlyTotElecUse, NULL, 0, pszRunID, pszaEPlusFuelNames[0/*elec*/],
		//																						esEUMap_CECNonRes[IDX_T24_NRES_EU_Total].sEnduseName, iBEMProcIdx );
		//		bool bHrlyTotElecUsePtrOK = (pdHrlyTotElecUse && iHrlyTotElecUsePtrRV==0);			assert( bHrlyTotElecUsePtrOK );  // needs to be valid following split of results processing routine? - SAC 7/23/18
		//		if (!bHrlyTotElecUsePtrOK)
				if (iNumHrlyElecUsePtrs < 1)
				{	assert( false );
					sErrorMsg = QString( "ERROR:  Unable to retrieve E+ Electric Use hourly results (required for CSE PV/Battery simulation):  %1, %2, %3" ).arg( 
																		pszRunID, pszaEPlusFuelNames[0/*elec*/], QString::number(iBEMProcIdx) );
					iRetVal = 70;		// BEMAnal_CECRes_ (no error ID for this to date)
				}
				else
				{	QString sEUseFile = sProjFileAlone + "-elecuse.csv";
					QString sFullEUseFile = m_sProcessPath + sEUseFile;
					sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
					             "application before an updated file can be written.\n\nSelect 'Retry' to update the file "
									 "(once the file is closed), or \n'Abort' to abort the %3." ).arg( "Building Electric Use", sFullEUseFile, "CSE PV/Battery simulation" );
					if (!OKToWriteOrDeleteFile( sFullEUseFile.toLocal8Bit().constData(), sMsg, m_bSilent ))
					{	if (m_bSilent)
							sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file (required for PV/Battery CSE simulation):  %2" ).arg( "Building Electric Use", sFullEUseFile );
						else
							sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file (required for PV/Battery CSE simulation):  %2" ).arg( "Building Electric Use", sFullEUseFile );
						iRetVal = 70;		// BEMAnal_CECRes_ (no error ID for this to date)
					}
					else
					{	QDateTime locTime = QDateTime::currentDateTime();
						QString timeStamp = locTime.toString("ddd dd-MMM-yy  hh:mm:ss ap");   // "Wed 14-Dec-16  12:30:29 pm"
		      		QString qsVer;
						if (!BEMPX_GetString( BEMPX_GetDatabaseID( "Proj:CompManagerVersion" ), qsVer ) || qsVer.length() < 1)
							qsVer = "(CompManagerVersion unknown)";

						FILE *fp_CSV;
						int iErrorCode;
						try
						{	iErrorCode = fopen_s( &fp_CSV, sFullEUseFile.toLocal8Bit().constData(), "wb" );
							if (iErrorCode != 0 || fp_CSV == NULL)
							{	assert( false );
								sErrorMsg = QString( "ERROR:  Unable to write E+ Electric Use CSV file (required for CSE PV/Battery simulation):  %1" ).arg( sFullEUseFile );
								iRetVal = 70;		// BEMAnal_CECRes_ (no error ID for this to date)
							}
							else
							{	fprintf( fp_CSV, "\"E+ Electric Use (kWh) - %s\",1\n", qsVer.toLocal8Bit().constData() );
								fprintf( fp_CSV, "\"%s\"\n", timeStamp.toLocal8Bit().constData() );
							//	fprintf( fp_CSV, "\"%s\",Hour\n", qsVer.toLocal8Bit().constData() );
								fprintf( fp_CSV, "\"EplusElecUse\",Hour\n", qsVer.toLocal8Bit().constData() );	// first column string must be consistent w/ cseIMPORTFILE:imTitle in CSE input
								fprintf( fp_CSV, "\"EplusTotalKWH\"\n" );
								double dVal;
								for (int hr=0; hr<8760; hr++)
								{	dVal = 0;
									for (iEU=0; iEU < iNumHrlyElecUsePtrs; iEU++)
										dVal += pdHrlyElecUse[iEU][hr];
									fprintf( fp_CSV, "%g\n", dVal );
								}
								fprintf( fp_CSV, "\n" );
								fflush( fp_CSV );
								fclose( fp_CSV );
							}
						}
						catch( ... )
						{	assert( false );
							sErrorMsg = QString( "ERROR:  Exception thrown writing E+ Electric Use CSV file (required for CSE PV/Battery simulation):  %1" ).arg( sFullEUseFile );
							iRetVal = 70;		// BEMAnal_CECRes_ (no error ID for this to date)
					// how to handle error writing to EUse CSV file ??
						}
						if (iRetVal == 0)
							BEMPX_SetBEMData( BEMPX_GetDatabaseID( "cseIMPORTFILE:imFileName" ), BEMP_QStr, (void*) &sEUseFile );
					}

// A2030 Example,1
// 5/24/2018 11:35
// EnergyPlusExport,Hour
// Mon,Day,Hr,Spc Heat,Spc Cool,Indr Fans,Heat Rej,Pump&Misc,Dom HW,Lighting,Recept,Process,Othr Ltg,Proc Mtrs,Comp Tot,TotElec kWh,Spc Heat,Spc Cool,Indr Fans,Heat Rej,Pump&Misc,Dom HW,Lighting,Recept,Process,Othr Ltg,Proc Mtrs,Comp Tot,TotNG kBtu,Spc Heat,Spc Cool,Indr Fans,Heat Rej,Pump&Misc,Dom HW,Lighting,Recept,Process,Othr Ltg,Proc Mtrs,Comp Tot,TotProp kBtu,Electric,NatGas,OtherFuel
// 1,1,1,0,0,2.03769,0,0.122414,0.124335,0.57115,1.76452,0,1.36356,0,2.85559,5.98367,0,0,0,0,0,17.5678,0,0,0,0,0,17.5678,17.5678,0,0,0,0,0,0,0,0,0,0,0,0,0,16.28,1712.5,3217.1
// 1,1,2,0,0.0151643,1.82452,0,0.122414,0.124335,0.57115,1.76452,0,1.36356,0,2.65758,5.78566,0,0,0,0,0,24.4135,0,0,0,0,0,24.4135,24.4135,0,0,0,0,0,0,0,0,0,0,0,0,0,15.96,1712.5,3217.1


//	   Proj:CSE_ElecUseIMPORTFILE	= {  CreateComp( cseIMPORTFILE, "EplusElecUseImportFile", "" )  }
//IMPORTFILE "EnergyPlusHourly" 
//   imFileName = "A2030_EnergyPlusExport.csv"
//column name:  "EplusTotalKWH"

				}

//double BEMPROC_API __cdecl BEMPX_GetHourlyResultArray( char* pszErrMsgBuffer, int iErrMsgBufferLen, double* pDbl, const char* pszRunName,
//																					  const char* pszMeterName,    const char* pszEnduse1,      const char* pszEnduse2=NULL,
//																					  const char* pszEnduse3=NULL, const char* pszEnduse4=NULL, const char* pszEnduse5=NULL,
//																					  const char* pszEnduse6=NULL, const char* pszEnduse7=NULL, const char* pszEnduse8=NULL, int iBEMProcIdx=-1 );
//int    BEMPROC_API __cdecl BEMPX_GetHourlyResultArrayPtr( double** ppDbl, char* pszErrMsgBuffer, int iErrMsgBufferLen, const char* pszRunName,
//																						  const char* pszMeterName, const char* pszEnduse, int iBEMProcIdx=-1 );

//int CSERunMgr::SetupRun_NonRes(int iRunIdx, int iRunType, QString& sErrorMsg, bool bAllowReportIncludeFile /*=true*/,		// SAC 5/24/16
//											const char* pszRunID /*=NULL*/, const char* pszRunAbbrev /*=NULL*/, QString* psCSEVer /*=NULL*/, int iBEMProcIdx /*=-1*/ )

//			// SAC 8/18/14 - fixed bug where DHW simulation results NOT being accounted for when applying TDV multipliers to hourly results
//					double* pdBEMHrlyRes	= NULL;
//					int iBEMHrlyResPtrRV = BEMPX_GetHourlyResultArrayPtr( &pdBEMHrlyRes, NULL, 0, osRunInfo.LongRunID().toLocal8Bit().constData(), pszaEPlusFuelNames[iFl], esEUMap_CECNonRes[iEUIdx].sEnduseName, osRunInfo.BEMProcIdx() );
//					bool bBEMHrlyResPtrOK = (pdBEMHrlyRes && iBEMHrlyResPtrRV==0);			assert( bBEMHrlyResPtrOK );  // needs to be valid following split of results processing routine? - SAC 7/23/18
//
//				double* daMtrEUData[NUM_T24_NRES_Fuels][NUM_T24_NRES_EndUses];
//				double  daMtrCTotData[NUM_T24_NRES_Fuels][8760], daMtrTotData[NUM_T24_NRES_Fuels][8760];
//				int iMtr=0, iEU;
//				for (; iMtr < NUM_T24_NRES_Fuels; iMtr++)
//				{	//	for (iYrHr=0; iYrHr<8760; iYrHr++)
//					//		daMtrTotals[iMtr][iYrHr] = 0.0;
//					int iEUOM = (iMtr==0 ? 0 : 1);
//					int iEUCmpTot=-1, iEUTot=-1;	// SAC 2/1/17
//					for (iEU=0; iEU < iNumEUs; iEU++)
//					{	if (BEMPX_GetHourlyResultArrayPtr( &daMtrEUData[iMtr][iEU], NULL, 0, pszModelName, pszaEPlusFuelNames[iMtr],
//	 																esEUMap_CECNonRes[iEUO[iEUOM][iEU]].sEnduseName, iBEMProcIdx ) == 0 && daMtrEUData[iMtr][iEU] != NULL)
//						{	// OK - do nothing
//						}
//						else
//						{	if (iEUO[iEUOM][iEU] == IDX_T24_NRES_EU_CompTot)		// SAC 2/1/17 - used to sum CompTot & Tot results, as they are NOT represented in the meter results retrieved above
//							{	iEUCmpTot = iEU;
//								daMtrEUData[iMtr][iEU] = &daMtrCTotData[iMtr][0];
//								for (iYrHr=0; iYrHr<8760; iYrHr++)
//									daMtrCTotData[iMtr][iYrHr] = 0.0;
//							}
//							else if (iEUO[iEUOM][iEU] == IDX_T24_NRES_EU_Total)


		}	}



// debugging
//bool bVerboseInit = m_bVerbose;
//m_bVerbose = true;
				// SAC 1/29/19 - code to initiate HPWH Sizing run(s) when called for (based on Res)
				long lCSE_HPWHSizingReqd=0, lHPWHSizInProc=0;
				long lDBID_HPWHSizInProc = BEMPX_GetDatabaseID( "Proj:CSE_HPWHSizInProc" );	// SAC 12/18/18
				if (lDBID_HPWHSizInProc > 0 && (!BEMPX_GetInteger( lDBID_HPWHSizInProc, lHPWHSizInProc ) || lHPWHSizInProc < 1) &&
					 BEMPX_GetInteger( BEMPX_GetDatabaseID( "Proj:CSE_HPWHSizingReqd" ), lCSE_HPWHSizingReqd ) && lCSE_HPWHSizingReqd > 0)
				{	// set flag indicating that we are performing HPWHSizing (otherwise we get stuck in an infinite loop during setup of CSE run initiated by T24Res_HPWHSizing()
					lHPWHSizInProc = 1;
					BEMPX_SetBEMData( lDBID_HPWHSizInProc, BEMP_Int, (void*) &lHPWHSizInProc );
				// PERFORM HPWH Sizing
					if (!T24Res_HPWHSizing( sProjFileAlone, sRunID, sErrorMsg, 1 /*iModelType*/ ))
					{	iRetVal = 72;	// Error in sizing small central residential HPWH system(s) using CSE
						if (!sErrorMsg.isEmpty())
							BEMPX_WriteLogFile( sErrorMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
					}
					// re-default (blast) flag indicating that we are performing HPWHSizing
					BEMPX_DefaultProperty( lDBID_HPWHSizInProc, m_iError );
				}
// debugging
//m_bVerbose = bVerboseInit;



		QString sProjCSEFile;
		if (iRetVal == 0)
		{	// Write CSE input file  (and store BEM details file)
			sProjCSEFile = m_sProcessPath + sProjFileAlone + ".cse";
			sMsg = QString( "The %1 file '%2' is opened in another application.  This file must be closed in that "
							 "application before an updated file can be written.\n\nSelect 'Retry' to update the file "
							 "(once the file is closed), or \n'Abort' to abort the analysis." ).arg( "CSE input", sProjCSEFile );
			if (!OKToWriteOrDeleteFile( sProjCSEFile.toLocal8Bit().constData(), sMsg, m_bSilent ))
			{	if (m_bSilent)
					sErrorMsg = QString( "ERROR:  Unable to overwrite %1 file:  %2" ).arg( "CSE input", sProjCSEFile );
				else
					sErrorMsg = QString( "ERROR:  User chose not to overwrite %1 file:  %2" ).arg( "CSE input", sProjCSEFile );
				iRetVal = 58;		//	58 : Unable to open/delete/write simulation input (.cse) file
			}
			else if (!BEMPX_WriteProjectFile( sProjCSEFile.toLocal8Bit().constData(), BEMFM_INPUT /*TRUE*/, FALSE /*bUseLogFileName*/, FALSE /*bWriteAllProperties*/,
										FALSE /*bSupressAllMessageBoxes*/, BEMFT_CSE /*iFileType*/, false /*bAppend*/,
										NULL /*ModelName*/, true /*WriteTerminator*/, -1 /*iBEMProcIdx*/, -1 /*ModDate*/, false /*OnlyValidInputs*/,   // SAC 7/23/18 - added iBEMProcIdx
										true /*AllowCreateDateReset*/, 1 /*PropertyCommentOption*/ ))			// SAC 12/5/16 - added to enable files to include comments: 0-none / 1-units & long name / 
			{	sErrorMsg = QString( "ERROR:  Unable to write %1 file:  %2" ).arg( "CSE input", sProjCSEFile );
				iRetVal = 59;		//	59 : Error writing simulation input (.cse) file
			}
			else
			{	SetCurrentDirectory( m_sProcessPath.toLocal8Bit().constData() );
				if (m_bStoreBEMProcDetails)
				{	QString sDbgFileName = sProjCSEFile.left( sProjCSEFile.length()-3 );
					sDbgFileName += "ibd-Detail";
					BEMPX_WriteProjectFile( sDbgFileName.toLocal8Bit().constData(), BEMFM_DETAIL /*FALSE*/ );
				}
			}
			BEMPX_RefreshLogFile();	// SAC 5/19/14
		}

		if (iRetVal == 0 && m_bPerformSimulations && !m_bBypassCSE)
		{
			// remove trailing '.cse' file extension from path/file passed into CSE
			QString sProjCSEFileForArg = sProjCSEFile;
			if (!sProjCSEFileForArg.right(4).compare(".CSE", Qt::CaseInsensitive))
				sProjCSEFileForArg = sProjCSEFileForArg.left( sProjCSEFileForArg.length()-4 );

			// CSE SIMULATION
			QString sParams;
			//	sParams = QString( "-b \"%1.cse\"" ).arg( sProjFileAlone );		// SAC 8/20/12 - added '-b' (batch) option to prevent user prompt in the event of sim error
			sParams = QString( "-b \"%1\"" ).arg( sProjCSEFileForArg );
	#ifdef _DEBUG
			sLogMsg = QString( "   CSE exePath:  %1" ).arg( m_sCSEexe );
			BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
			sLogMsg = QString( "   CSE cmdLine:  %1" ).arg( sParams );
			BEMPX_WriteLogFile( sLogMsg, NULL /*sLogPathFile*/, FALSE /*bBlankFile*/, TRUE /*bSupressAllMessageBoxes*/, FALSE /*bAllowCopyOfPreviousLog*/ );
	#endif
			pCSERun->SetArgs( sParams);
		}
	}

	if (iPrevBEMProcIdx > 0)	// SAC 7/23/18
		BEMPX_SetActiveModel( iPrevBEMProcIdx );
#else
			iRunIdx;  iRunType;  sErrorMsg;  bAllowReportIncludeFile;  pszRunID;  pszRunAbbrev;  psCSEVer;  iBEMProcIdx;
#endif

	return iRetVal;
}

void CSERunMgr::DoRuns()
{
	CSERun* pCSERun = NULL;
	for(size_t iRun = 0; iRun < m_vCSERun.size(); ++iRun)
	{	pCSERun = m_vCSERun[iRun];
		StartRun( *pCSERun);
		m_vCSEActiveRun.push_back( pCSERun);
	}
	MonitorRuns();
}		// CSERunMgr::DoRuns

void CSERunMgr::DoRunRange( int iFirstRunIdx, int iLastRunIdx )
{
	CSERun* pCSERun = NULL;		int iNumRuns=0;
	for(size_t iRun = (size_t) iFirstRunIdx; iRun < m_vCSERun.size() && iRun <= (size_t) iLastRunIdx; ++iRun)
	{	pCSERun = m_vCSERun[iRun];
		if (pCSERun)
		{	StartRun( *pCSERun);
			m_vCSEActiveRun.push_back( pCSERun);
			iNumRuns++;
	}	}		assert( iNumRuns > 0 );
	if (iNumRuns > 0)
		m_iNumProgressRuns = 1;
	MonitorRuns();
	m_iNumProgressRuns = -1;
}		// CSERunMgr::DoRunRange

void CSERunMgr::MonitorRuns()
{
	CSERun* pCSERun;
	exec_stream_t* pES = NULL;
	std::string sOut;
	bool bFirstException = true;
	while(m_vCSEActiveRun.size() > 0)
	{	for(size_t iRun = 0; iRun < m_vCSEActiveRun.size(); ++iRun)
		{	pCSERun = m_vCSEActiveRun[iRun];
			pES = pCSERun->GetExecStream();
			try
			{	if (pES->running())
				{	ProcessRunOutput( pES, iRun, bFirstException);
					Sleep(10);
				}
				else
				{	bool bFirstExceptionX = true;
					while( ProcessRunOutput( pES, iRun, bFirstExceptionX));
					pES->close();
					pCSERun->SetExitCode( pES->exit_code());
					m_vCSEActiveRun.erase(m_vCSEActiveRun.begin()+iRun);
				}
			}
			catch(exec_stream_t::error_t &e)
			{	if (bFirstException)
				{	std::string sLogMsg=e.what();
					BEMPX_WriteLogFile( sLogMsg.c_str(), NULL, FALSE, TRUE, FALSE );
					bFirstException = false;
				}
			}
		}
	}
}		// CSERunMgr::MonitorRuns

bool CSERunMgr::ProcessRunOutput(exec_stream_t* pES, size_t iRun, bool &bFirstException)
{
	assert( pES);
	std::string sOut;
	try
	{	std::istream& is = pES->out();
		is >> sOut;
		if (!is.good())
			is.clear();
		if (!sOut.empty())
		{	sOut = " " + sOut;
			if (CSE_ProcessMessage( 0, sOut.c_str(), (int) iRun, this) == CSE_ABORT)
				pES->kill(255);
			else if (sOut.find("OpenGL") != std::string::npos)
				IncNumOpenGLErrors();
//			if (pCSERunMgr && (strstr( msg, "OpenGL" ) != NULL))
//				pCSERunMgr->IncNumOpenGLErrors();
		}
	}
	catch(exec_stream_t::error_t &e)
	{	if (bFirstException)
		{	std::string sLogMsg=e.what();
			BEMPX_WriteLogFile( sLogMsg.c_str(), NULL, FALSE, TRUE, FALSE );
			bFirstException = FALSE;
		}
	}
	return !sOut.empty();
}		// CSERunMgr::ProcessRunOutput

void CSERunMgr::StartRun(CSERun& cseRun)
{
	try
	{	exec_stream_t* pES = new exec_stream_t( m_sCSEexe.toLocal8Bit().constData(), cseRun.GetArgs().toLocal8Bit().constData(), CREATE_NO_WINDOW);
		cseRun.SetExecStream( pES);
	}
	catch(exec_stream_t::error_t &e)
	{	std::string sLogMsg=e.what();
		BEMPX_WriteLogFile( sLogMsg.c_str(), NULL, FALSE, TRUE, FALSE );
	}
}		// CSERunMgr::StartRun

QString CSERunMgr::GetVersionInfo()
{	QString sVersion;
	DWORD verHandle = NULL;
	UINT size = 0;
	LPBYTE lpBuffer = NULL;
	DWORD verSize = GetFileVersionInfoSize( m_sCSEexe.toLocal8Bit().constData(), &verHandle);
	if (verSize != NULL)
	{	LPSTR verData = new char[verSize];
		QString qsCSEName = "CSE";
		int iLastSlash = std::max( m_sCSEexe.lastIndexOf('\\'), m_sCSEexe.lastIndexOf('/') );
		int iLastDot   =           m_sCSEexe.lastIndexOf('.');
		if (iLastSlash > 0 && iLastDot > (iLastSlash+1))		// SAC 12/4/17 - report CSEs w/ different executable filenames
			qsCSEName = m_sCSEexe.mid( iLastSlash+1, (iLastDot-iLastSlash-1) );
		if (GetFileVersionInfo( m_sCSEexe.toLocal8Bit().constData(), verHandle, verSize, verData))
		{
			QString sVerNum;  // SAC 11/26/18 - mods to use Product Version STRING if available
			BYTE* lpb;
			if (VerQueryValue( verData, "\\VarFileInfo\\Translation", (void**)&lpb, &size ))
			{	WORD* lpw = (WORD*)lpb;
				std::string sProcVerQuery = boost::str( boost::format( "\\StringFileInfo\\%04x%04x\\ProductVersion" ) % lpw[ 0 ] % lpw[ 1 ] );
				if (VerQueryValue( verData, sProcVerQuery.c_str(), (void**)&lpb, &size ) && size > 0)
				{	sVerNum = (LPCSTR)lpb;
													// testing
													//QString sVerInfo = QString( "   %1 ProductVersion=%2" ).arg( qsCSEName, (LPCSTR)lpb );
													//BEMPX_WriteLogFile( sVerInfo, NULL, false, TRUE, false );
			}	}
			if (sVerNum.isEmpty() && VerQueryValue( verData, "\\", (VOID FAR* FAR*)&lpBuffer, &size ) && size)
			{	VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
				if (verInfo->dwSignature == 0xfeef04bd)
				{	int major = HIWORD(verInfo->dwFileVersionMS);
					int minor = LOWORD(verInfo->dwFileVersionMS);
					sVerNum = QString( "%1.%2" ).arg( QString::number(major), QString::number(minor));
													// testing
													//QString sVerInfo = QString( "   %1 VS_FIXEDFILEINFO:  dwFileVersionMS=%2  dwFileVersionLS=%3  dwProductVersionMS=%4  dwProductVersionLS=%5  dwFileDateMS=%6  dwFileDateLS=%7" ).arg( 
													//		qsCSEName, QString::number( verInfo->dwFileVersionMS ), QString::number( verInfo->dwFileVersionLS ), QString::number( verInfo->dwProductVersionMS ), QString::number( verInfo->dwProductVersionLS ), QString::number( verInfo->dwFileDateMS ), QString::number( verInfo->dwFileDateLS ) );
													//BEMPX_WriteLogFile( sVerInfo, NULL, false, TRUE, false );
			}	}

			if (!sVerNum.isEmpty())
			{	sVersion = QString( "%1 %2" ).arg( qsCSEName, sVerNum );
				int pathLen = m_sCSEexe.length();
				if (pathLen >=4)
				{	if (_stricmp( m_sCSEexe.right(4).toLocal8Bit().constData(), ".exe") == 0)
						sVersion += " EXE";
					else if (_stricmp( m_sCSEexe.right(4).toLocal8Bit().constData(), ".dll") == 0)
						sVersion += " DLL";
			}	}
		}
		delete[] verData;
	}
	return sVersion;
}


static const char pszDashes[] = "--------------------------------------------------------------------------------\n";

// process CSE errors and/or reports into file for user review - SAC 11/7/16
bool CSERunMgr::ArchiveSimOutput( int iRunIdx, QString sSimOutputPathFile, int iOutFileType )
{	bool bRetVal = ((int) m_vCSERun.size() > iRunIdx ? true : false);

	if (bRetVal)
	{	const CSERun& cseRun = GetRun(iRunIdx);
		QString sTemp, sCSEOutFile = cseRun.GetOutFile( (CSERun::OutFile) iOutFileType );		assert( !sCSEOutFile.isEmpty() );
		QStringList slErrors;
		if (iOutFileType == CSERun::OutFileERR && m_iSimErrorOpt == 1)
		{	// make sure Errors exist in the error output file BEFORE 
			QFile errFile( sCSEOutFile );
			if (errFile.open( QIODevice::Text | QIODevice::ReadOnly ))
			{
	         char buff[ 300 ];
	         // read each line from the prev file into the log file
	         int iReadRetVal;   bool bSavingErr = false;   int iErrIdx = -1;
	         while (iReadRetVal = errFile.readLine( buff, 299 ) > 0)
	         {	sTemp = buff;
	         	if (bSavingErr)
	         	{	if (sTemp.left(4).compare("----"))
	         			slErrors[iErrIdx] += sTemp;
	        			else  // end of error
	        				bSavingErr = false;
	         	}
	         	else
	         	{	bool bCSELineErr  = (!sTemp.right(8).compare("Error: \n"));
	         		bool bOtherCSEErr = (!sTemp.left( 5).compare("Error"));
	         		if (bCSELineErr || bOtherCSEErr)
		         	{	// start of new error message
		         		if (bCSELineErr)
		         		{	int iStartParen = sTemp.lastIndexOf('('), iEndParen = sTemp.lastIndexOf(')');		assert( iEndParen > (iStartParen+1) && iStartParen > 0 );
			         		if (iEndParen > (iStartParen+1) && iStartParen > 0)
			         			slErrors.push_back( QString("\nError on line %1: ").arg( sTemp.mid( iStartParen+1, (iEndParen-iStartParen-1) ) ) );
			         		else
			         			slErrors.push_back( QString("\n%1").arg( sTemp ) );
			         	}
		         		else	// added else condition for those errors that don;t track directly back to CSE input line numbers
		         			slErrors.push_back( QString("\n%1").arg( sTemp ) );
		        			iErrIdx++;
		     				bSavingErr = true;
	         	}	}
	         }
	         assert( iReadRetVal==0 );	// if -1, then error reading
				if (iReadRetVal < 0)
					bRetVal = false;
//---------------
// Error: C:\WSF\DEVLIBS\COMPMGR\SRC\BEM-OPEN\BIN\RES\PROJECTS\SAMPLES-2016\1STORYEXAMPLE3-SMATTIC2 - COMP16\1STORYEXAMPLE3-SMATTIC2 - PROP.CSE(1935): Error: 
//  RSYS 'rsys-HVAC System 1': 
//    S0497: 'rsSEER' missing: required when rsType=ACFurnace (cooling is available)
//---------------
// Error: C:\WSF\DEVLIBS\COMPMGR\SRC\BEM-OPEN\BIN\RES\PROJECTS\SAMPLES-2016\1STORYEXAMPLE3-SMATTIC2 - COMP16\1STORYEXAMPLE3-SMATTIC2 - PROP.CSE(1935): Error: 
//  RSYS 'rsys-HVAC System 1': rsSEER (0) must be > rsEER (0.34)
//---------------
// Info: C:\WSF\DEVLIBS\COMPMGR\SRC\BEM-OPEN\BIN\RES\PROJECTS\SAMPLES-2016\1STORYEXAMPLE3-SMATTIC2 - COMP16\1STORYEXAMPLE3-SMATTIC2 - PROP.CSE(1983): Info: 
//  S0214: No run due to error(s) above
//---------------
		}	}

//if (iOutFileType == CSERun::OutFileERR)
//{QString sDbgMsg = QString( "%1 errors found listed in CSE error file: %2" ).arg( QString::number(slErrors.size()), sCSEOutFile );
//BEMMessageBox( sDbgMsg );
//}

		if (bRetVal && (iOutFileType != CSERun::OutFileERR || slErrors.size() > 0) &&
							(iOutFileType != CSERun::OutFileREP || FileExists( sCSEOutFile )))
		{
		try
		{	bool bOutFileAlreadyExists = FileExists( sSimOutputPathFile );
			QFile outFile( sSimOutputPathFile );
			if (!bOutFileAlreadyExists)
				bRetVal = outFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate );
			else
				bRetVal = outFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
																																			assert( bRetVal );
			if (bRetVal)
			{
				if (bOutFileAlreadyExists)
					outFile.write( "\n\n" );

				QString sCSEInpFile = cseRun.GetArgs();
				sCSEInpFile = sCSEInpFile.left( sCSEInpFile.length()-1 );
				int iLastSlash    = std::max( sCSEInpFile.lastIndexOf('\\'),               sCSEInpFile.lastIndexOf('/') );
				int i2ndLastSlash = std::max( sCSEInpFile.lastIndexOf('\\', iLastSlash-1), sCSEInpFile.lastIndexOf('/', iLastSlash-1) );

//--------------------------------------------------------------------------------
//--  PROPOSED model:  Dir\File.cse
//--------------------------------------------------------------------------------
				outFile.write( pszDashes );
				outFile.write( QString("--  %1 model:  %2\n").arg( cseRun.GetRunID().toUpper(), sCSEInpFile.right( (sCSEInpFile.length()-i2ndLastSlash-1) ) ).toLocal8Bit().constData() );
				outFile.write( pszDashes );

				if (iOutFileType == CSERun::OutFileERR)
					for (int i=0; i<slErrors.size(); i++)
					{	outFile.write( slErrors[i].toLocal8Bit().constData() );
						//outFile.write( "\n" );
					}
				else	// report (not error) file
				{
					QFile repFile( sCSEOutFile );
					if (repFile.open( QIODevice::Text | QIODevice::ReadOnly ))
					{
			         char buff[ 300 ];
			         // read each line from the prev file into the log file
			         int iReadRetVal;
			         while (iReadRetVal = repFile.readLine( buff, 299 ) > 0)
			// at some point 
						{	outFile.write( buff );
							//outFile.write( "\n" );
						}
			         assert( iReadRetVal==0 );	// if -1, then error reading
						if (iReadRetVal < 0)
							bRetVal = false;
					}
					else
						bRetVal = false;
				}

//	int m_iSimReportOpt;		// SAC 11/5/16 - 0: no CSE reports / 1: user-specified reports / 2: entire .rpt file
//	int m_iSimErrorOpt;		// SAC 11/5/16 - 0: no CSE errors / 1: always list CSE errors
//	int  iSimReportDetailsOption	=	GetCSVOptionValue( "SimReportDetailsOption",   1,  saCSVOptions );		// SAC 11/5/16 - 0: no CSE reports / 1: user-specified reports / 2: entire .rpt file
//	int  iSimErrorDetailsOption	=	GetCSVOptionValue( "SimErrorDetailsOption" ,   1,  saCSVOptions );		// SAC 11/5/16 - 0: no CSE errors / 1: always list CSE errors

				outFile.close();
			}
		}
		catch( ... )
		{	assert( false );
			bRetVal = false;
		}
	}	}	// end of if (bRetVal)

	return bRetVal;
}