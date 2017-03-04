//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
// PPEDT.c
//
// Geosoft high-level graphics class.
//
// Allows Editing of PLOT Files.
//
//=============================================================================


#include <col.h>                // COL Utilities
#include <pp.h>                 // PP Header
#include "pp.ih"
#include <vv.h>                 // VV Header
#include <vvv.h>                // VVV Header
#include <bf.h>                 // RPF Header
#include <geosoft.h>            // GEOSOFT Header
#include <ma.h>                 // MA Header

#define PMF_PP_ERROR (110000L + __LINE__)

//*****************************************************************************
//                                EDITING FUNCTIONS
//*****************************************************************************


//-----------------------------------------------------------------------------

long lStartEdit_PP(H_PP*    hPP,         // PP Object
                   CON_STR* szPlot,      // Plot file name to edit
                   CON_STR* szSave,      // Plot file save name
                   long     lPlotOrigin) // Origin of Plot
{
	DEBUG_STACK;

	short         sCheckErr = 0;
	ppedt_ob*     pEdit = NULL;
	short         sCount;
	short         sDone;
	long          lBlocks;
	unsigned long lOffset;
	Entity_PMF    Pmf;
	extent_ob     Ext;



	// --- Validate Parameters ---

	ASSERT(hPP && szPlot && szSave);     // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Setup the Device Coordinate/Clipping ---

	hPP->PMFOrigin = lPlotOrigin;

	// --- Create the Edit Structure ---

	hPP->pEdit = pEdit = (ppedt_ob*) Alloc_MP(hPP->hMP, sizeof (ppedt_ob));
	CHECK_ERR(pEdit == NULL);
	pEdit->hVVV = NULL;
	pEdit->hVV = NULL;
	pEdit->hPMF = NULL;
	pEdit->hSPMF = NULL;
	pEdit->szPlotName = NULL;
	pEdit->szSaveName = NULL;

	// --- Initialize the VV Extent Storage ---

	pEdit->hVV = hCreat_VV(hPP->hAP, -((short) sizeof (extent_ob)), 100);
	CHECK_ERR(pEdit->hVV == NULL);


	// --- Create the VVV Object ---

	pEdit->hVVV = hCreat_VVV(pEdit->hVV);
	CHECK_ERR(pEdit->hVVV == NULL);

	// --- Create the PLOT Object ---

	pEdit->hPMF = hCreat_PMF(hPP->hAP, szPlot, 0, BF_READWRITE_OLD);
	CHECK_ERR(pEdit->hPMF == NULL);

	// --- Create the SAVE Object ---

	pEdit->hSPMF = hCreat_PMF(hPP->hAP, szSave, 0, BF_READWRITE_NEW);
	CHECK_ERR(pEdit->hPMF == NULL);

	// --- Load the Attribute File ---

	if (lLoadAtt_PP(hPP->hAP, hPP->hVVAttribs, &hPP->sNAttribs, szPlot) != 0)
	{
		// --- File Corrupt PMF_PP_ERROR ---

		RegistErr_AP(hPP->hAP, 20902, modn);
		SetParmL_AP(hPP->hAP, "%1", 1);
		sCheckErr = 1;
		goto GS_EXIT;
	}


	// --- Remeber the Names ---

	pEdit->szPlotName = (VOL_STR*) Alloc_MP(hPP->hMP, strlen(szPlot) + 1);
	CHECK_ERR(pEdit->szPlotName == NULL);
	pEdit->szSaveName = (VOL_STR*) Alloc_MP(hPP->hMP, strlen(szSave) + 1);
	CHECK_ERR(pEdit->szSaveName == NULL);

	strcpy(pEdit->szPlotName, szPlot);
	strcpy(pEdit->szSaveName, szSave);


	// --- Initialize the First extents table ---

	pEdit->lMinExt.x = GS_S4MX;
	pEdit->lMinExt.y = GS_S4MX;
	pEdit->lMaxExt.x = GS_S4MN;
	pEdit->lMaxExt.y = GS_S4MN;

	pEdit->lTotalExtents = 0L;
	pEdit->lAlocatedExtents = 100L;

	pEdit->lLastExtent = 0L;
	pEdit->lSaveExtent = 0L;
	pEdit->lFileSize = Seek_PMF(pEdit->hPMF, 0L, BF_SEEK_EOF);
	Seek_PMF(pEdit->hPMF, 0L, BF_SEEK_START);    // Go Back to begining.


	// --- Start Generating the Extents Table ---

	do
	{

		// --- Do we need to stop ? ---

		if (CheckStop_AP(hPP->hAP))
		{
			sCheckErr = 1;
			goto GS_EXIT;
		}


		// --- Get the Offset from the file ---

		lOffset = (unsigned long) Seek_PMF(pEdit->hPMF, 0L, BF_SEEK_CURRENT);


		// --- Read the PMF Header ---

		CHECK_ERR(lRead_PMF(pEdit->hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER) != 0);

		// --- Old Style PMF's ? ---

		if ((Pmf.sNumber == 3) || (Pmf.sNumber == 2) || ((Pmf.sNumber >= 1000) && (Pmf.sNumber <= 1255)))
		{
			// --- Read all the OLD PMF entities until we hit another type ---

			Ext.lOffset = lOffset | EXT_GRAPHIC | EXT_OLD;
			hPP->lMin.x = GS_S4MX;
			hPP->lMin.y = GS_S4MX;
			hPP->lMax.x = GS_S4MN;
			hPP->lMax.y = GS_S4MN;
			sCount = 0;
			hPP->sDoExtents = 1;   // Turn extents on

			do
			{
				// --- Skip To next Entity (if PMF2/3) ---

				if ((Pmf.sNumber == 3) || (Pmf.sNumber == 2))
				{
					sDone = 0;
					while (!sDone)
					{
						lBlocks = lReadOld_PMF(pEdit->hPMF, hPP->pDataBuff, PMF_PP_READ_BUFFER, &sDone);
						if (lBlocks > 0)
							IPVect_PP(hPP, hPP->pDataBuff, lBlocks / 4, CONV_SHORT, 0);
						CHECK_ERR(lBlocks == -1);
					}
				}

				// --- Increment Count ---

				sCount++;

				// --- Get the Offset from the file ---

				lOffset = (unsigned long) Seek_PMF(pEdit->hPMF, 0L, BF_SEEK_CURRENT);

				// --- Read the PMF Header ---

				CHECK_ERR(lRead_PMF(pEdit->hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER) != 0);

			} while ((Pmf.sNumber == 3) || (Pmf.sNumber == 2) || ((Pmf.sNumber >= 1000) && (Pmf.sNumber <= 1255)));

			// --- Generate a Simple Extent ---

			hPP->sDoExtents = 0;   // Turn it off
			Ext.sNumber = sCount;
			Ext.lMin = hPP->lMin;
			Ext.lMax = hPP->lMax;
			CHECK_ERR(lAddExt_PP(hPP, &Ext) == -1);
			pEdit->lSaveExtent++;
		}

		// --- What kind of entity is this ? ---

		if ((Pmf.sNumber != 9999) && (Pmf.sNumber != 999))
		{
			// --- Compute the Extents ---

			CHECK_ERR(lExtPmf_PP(hPP, pEdit->hPMF, &Pmf, lOffset, &Ext) != 0);

			// --- Add the Extents ---

			CHECK_ERR(lAddExt_PP(hPP, &Ext) == -1);

			// --- Update the Save Extents List ---

			pEdit->lSaveExtent++;

		}
		else
		{

			// --- Remeber the EOF ---

			pEdit->lPlotFileSize = lOffset;
			break;
		}

	} while (!sCheckErr);


	// --- Done ---

GS_EXIT:

	if (sCheckErr)
	{
		if (pEdit)
		{
			if (pEdit->hVVV)
				Destr_VVV(pEdit->hVVV);
			if (pEdit->hVV)
				Destr_VV(pEdit->hVV);
			if (pEdit->hPMF)
				Destr_PMF(pEdit->hPMF, BF_KEEP);
			if (pEdit->hSPMF)
				Destr_PMF(pEdit->hSPMF, BF_DELETE);
			if (pEdit->szPlotName)
				Free_MP(hPP->hMP, pEdit->szPlotName);
			if (pEdit->szSaveName)
				Free_MP(hPP->hMP, pEdit->szSaveName);
			Free_MP(hPP->hMP, pEdit);
			hPP->pEdit = NULL;
		}
	}

	return (sCheckErr);

}


//-----------------------------------------------------------------------------

long lEndEdit_PP(H_PP* hPP)   // PP Object
{
	DEBUG_STACK;

	long lPP_ERROR = NO_ERROR;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode


	// --- Destroy the Save/Plot file ---

	Destr_PMF(hPP->pEdit->hSPMF, BF_DELETE);
	Destr_PMF(hPP->pEdit->hPMF, BF_KEEP);


	// --- Destroy the Extents List ---

	Destr_VVV(hPP->pEdit->hVVV);
	Destr_VV(hPP->pEdit->hVV);


	// --- Destroy the Edit Object ---

	Free_MP(hPP->hMP, hPP->pEdit->szPlotName);
	Free_MP(hPP->hMP, hPP->pEdit->szSaveName);
	Free_MP(hPP->hMP, hPP->pEdit);


	// --- Turn Edit Mode OFF ---

	hPP->pEdit = NULL;


	// --- Done ---

	return (lPP_ERROR);

}


//-----------------------------------------------------------------------------

long lSaveEdit_PP(H_PP*       hPP,      // PP Object
                  Header_PMF* Header)   // Header to Place back
{
	DEBUG_STACK;

	long       lPP_ERROR;
	long       lCurId;
	long       lAtId;
	long       lStartId;
	long       lLastOffset;
	VOL_STR    pBuff[50];
	short      i;

	ppedt_ob*  pEdit;
	extent_ob* pExt = NULL;
	PMF_BUFF   Buff;



	// --- Validate Parameters ---

	ASSERT(hPP && Header);               // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode
	pEdit = hPP->pEdit;


	// --- Save the New Attribute File ---

	if ((lPP_ERROR = lCreateAtt_PP(hPP->hAP, hPP->hVVAttribs, hPP->sNAttribs, pEdit->szPlotName)) != 0)
		return (lPP_ERROR);


	// --- Prepare Plot File for Compression ---

	Mode_PMF(pEdit->hPMF, WRITE_MODE);
	Mode_PMF(pEdit->hSPMF, WRITE_MODE);
	lAtId = 0L;


	// --- Skip all Valid Plot Entities ---

	sStartNext_VVV(pEdit->hVVV, 0, 0);
	lCurId = -1L;
	do
	{
		lCurId++;
		if (lCurId >= pEdit->lSaveExtent)
			break;
		pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);
	} while ((pExt->lOffset & EXT_ERASED) == 0);
	
	lAtId = lCurId;


	// --- Update the File Size ---

	if (lCurId == pEdit->lSaveExtent)
		pEdit->lNewFileSize = pEdit->lPlotFileSize;
	else
		pEdit->lNewFileSize = pExt->lOffset & EXT_LOWER;



	// --- Plot File Compression ---

	while (lCurId < pEdit->lSaveExtent)
	{
		// --- Skip until we hit a valid entry ---

		sStartNext_VVV(pEdit->hVVV, lCurId + 1, 0);
		do
		{
			lCurId++;
			if (lCurId >= pEdit->lSaveExtent)
				break;
			pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);
		} while (pExt->lOffset & EXT_ERASED);


		// --- Count all the Valid Entries ---

		if (lCurId != pEdit->lSaveExtent)
		{
			lStartId = lCurId;
			do
			{
				pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);
				lCurId++;
			} while (((pExt->lOffset & EXT_ERASED) == 0) && (lCurId < pEdit->lSaveExtent));
		}
		else
			lStartId = -1;
		


		// --- Nothing to copy ? ---

		if (lStartId == -1)
			break;


		// --- Copy the Valid entries in the plot file ---

		if ((lPP_ERROR = lICopy_PP(hPP, &lAtId, lStartId, lCurId, 0)) != 0)
			goto SAVE_PP_ERROR;
	}


	// --- Skip All Invalid Entities ---

	lCurId = pEdit->lSaveExtent;
	sStartNext_VVV(pEdit->hVVV, lCurId, 0);
	do
	{
		pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);
		if ((pExt->lOffset & EXT_ERASED) == 0)
			break;
		lCurId++;
	} while (lCurId < pEdit->lTotalExtents);
	


	// --- Copy All Entities from the Save file to Plot file ---

	while (lCurId < pEdit->lTotalExtents)
	{
		// --- Get All the Valid Entities ---

		lStartId = lCurId;
		sStartNext_VVV(pEdit->hVVV, lCurId, 0);
		do
		{
			pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);
			if (pExt->lOffset & EXT_ERASED)
				break;
			lCurId++;
		} while (lCurId < pEdit->lTotalExtents);
		


		// --- Copy Them to the Plot File ---

		if ((lPP_ERROR = lICopy_PP(hPP, &lAtId, lStartId, lCurId, 1)) != 0)
			goto SAVE_PP_ERROR;


		// --- Skip all Invalid Entities ---

		if (lCurId == pEdit->lTotalExtents)
			break;
		sStartNext_VVV(pEdit->hVVV, lCurId, 0);
		do
		{
			pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);
			if ((pExt->lOffset & EXT_ERASED) == 0)
				break;
			lCurId++;
		} while (lCurId < pEdit->lTotalExtents);
		
	}


	// --- Set the Last and Save Extents ---

	pEdit->lSaveExtent = pEdit->lTotalExtents = lAtId;
	pEdit->lLastExtent = 0L;


	// --- Write EOF to Plot File ---

	lLastOffset = pEdit->lNewFileSize;
	Buff.sId = VALID_PMF;
	Buff.sNumber = 9999;
	Buff.lSize = 0L;
	((H_PMF*) pEdit->hPMF)->ipBF->lSeek(pEdit->lNewFileSize, BF_SEEK_START);
	if (((H_PMF*) pEdit->hPMF)->ipBF->lWrite(&Buff, 8) != 8)
	{
		lPP_ERROR = PMF_PP_ERROR;
		goto SAVE_PP_ERROR;
	}
	pEdit->lNewFileSize += 8;


	// --- Change the sizes of the files ---

	((H_PMF*) pEdit->hPMF)->ipBF->sChSize(pEdit->lNewFileSize);
	((H_PMF*) pEdit->hSPMF)->ipBF->sChSize(0L);
	pEdit->lFileSize = pEdit->lNewFileSize;
	pEdit->lPlotFileSize = pEdit->lNewFileSize - 8;



	// --- Update the End of File PMF ---

	if ((Header->sVersion == 2) && (Header->sRevision == 1))
	{
		pExt = (extent_ob*) pPoint_VVV(pEdit->hVVV, 1);
		if (pExt->sNumber != 2001)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto SAVE_PP_ERROR;
		}
		((H_PMF*) pEdit->hPMF)->ipBF->lSeek((pExt->lOffset & EXT_LOWER) + 8, BF_SEEK_START);
		if (((H_PMF*) pEdit->hPMF)->ipBF->lWrite(&lLastOffset, 4) != 4)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto SAVE_PP_ERROR;
		}
	}

	// --- Update the Map Registration Information ---

	if (Header->sFlags & LOADED_OLD)
	{
		// --- Find the MAP (32767) PMF ---

		i = 0;
		do
			pExt = (extent_ob*) pPoint_VVV(pEdit->hVVV, i++);
		while ((pExt->sNumber != 32767) && (i < 20));
		ASSERT(pExt->sNumber == 32767);  // Must have found it

		// --- Create the New PMF data ---

		((PMF_32767*) pBuff)->sOrigin = (short) (Header->lOrigin / 50L);
		((PMF_32767*) pBuff)->sMaxAttribs = (short) (Header->sNAttribs + 1000);
		((PMF_32767*) pBuff)->sMinX = (short) ((Header->MinRegion.x + Header->lOrigin) / 50L);
		((PMF_32767*) pBuff)->sMinY = (short) ((Header->MinRegion.y + Header->lOrigin) / 50L);
		((PMF_32767*) pBuff)->sMaxX = (short) ((Header->MaxRegion.x + Header->lOrigin) / 50L);
		((PMF_32767*) pBuff)->sMaxY = (short) ((Header->MaxRegion.y + Header->lOrigin) / 50L);

		// --- Write the New Data ---

		((H_PMF*) pEdit->hPMF)->ipBF->lSeek((pExt->lOffset & EXT_LOWER) + 8, BF_SEEK_START);
		if (((H_PMF*) pEdit->hPMF)->ipBF->lWrite(&pBuff, 12) != 12)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto SAVE_PP_ERROR;
		}

	}


	// --- New PMF ? ---

	if (Header->sFlags & LOADED_NEW)
	{
		// --- Find the MAP (5) PMF ---

		i = 0;
		do
			pExt = (extent_ob*) pPoint_VVV(pEdit->hVVV, i++);
		while ((pExt->sNumber != 5) && (i < 20));
		ASSERT(pExt->sNumber == 5);

		// --- Create the New PMF data ---

		((PMF_5*) pBuff)->rMinX = (float) Header->MinRegion.x / (float) 1000.0;
		((PMF_5*) pBuff)->rMinY = (float) Header->MinRegion.y / (float) 1000.0;
		((PMF_5*) pBuff)->rMaxX = (float) Header->MaxRegion.x / (float) 1000.0;
		((PMF_5*) pBuff)->rMaxY = (float) Header->MaxRegion.y / (float) 1000.0;

		// --- Write the New Data ---

		((H_PMF*) pEdit->hPMF)->ipBF->lSeek((pExt->lOffset & EXT_LOWER) + 8, BF_SEEK_START);
		if (((H_PMF*) pEdit->hPMF)->ipBF->lWrite(&pBuff, 16) != 16)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto SAVE_PP_ERROR;
		}
	}


	// --- Update the World Map Registration Information ---

	if (Header->sFlags & LOADED_MAP)
	{
		// --- Find the MAP (4) PMF ---

		i = 0;
		do
			pExt = (extent_ob*) pPoint_VVV(pEdit->hVVV, i++);
		while ((pExt->sNumber != 4) && (i < 20));
		ASSERT(pExt->sNumber == 4);


		// --- Create the New PMF data ---

		((PMF_4*) pBuff)->dScale = Header->dScale;
		((PMF_4*) pBuff)->dConv = Header->dConv;
		((PMF_4*) pBuff)->dXloc = Header->dOrigin.x;
		((PMF_4*) pBuff)->dYloc = Header->dOrigin.y;
		((PMF_4*) pBuff)->dRotate = Header->dRotation;

		// --- Write the New Data ---

		((H_PMF*) pEdit->hPMF)->ipBF->lSeek((pExt->lOffset & EXT_LOWER) + 8, BF_SEEK_START);
		if (((H_PMF*) pEdit->hPMF)->ipBF->lWrite(&pBuff, 40) != 40)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto SAVE_PP_ERROR;
		}
	}


	// --- Turn Regular Mode on ---

SAVE_PP_ERROR:
	Mode_PMF(pEdit->hPMF, READ_MODE);
	Mode_PMF(pEdit->hSPMF, READ_MODE);

	// --- Return PMF_PP_ERROR Code ---

	if (lPP_ERROR)
	{
		// --- File Corrupt PMF_PP_ERROR ---

		RegistErr_AP(hPP->hAP, 20907, modn);
		SetParmL_AP(hPP->hAP, "%1", lPP_ERROR);
	}

	// --- Done ---

	return (lPP_ERROR);

}



//-----------------------------------------------------------------------------

void EditInfo_PP(H_PP*     hPP,         // PP Object
                 long*     plFileSize,  // Size of the PLOT
                 long*     plMemUsed,   // Amount of Memory Used
                 long*     plExtUsed,   // Number of Extents Generated
                 Point_DD* lMin,        // Mimimum Extents
                 Point_DD* lMax)        // Maximum Extents
{
	DEBUG_STACK;

	long lSaveSize;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Must be Real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode


	// --- Return the Information ---

	if (plFileSize)
	{
		lSaveSize = Seek_PMF(hPP->pEdit->hSPMF, 0L, BF_SEEK_EOF);
		*plFileSize = hPP->pEdit->lFileSize + lSaveSize;
	}
	if (plMemUsed)
		*plMemUsed = hPP->pEdit->lAlocatedExtents * sizeof (extent_ob);
	if (plExtUsed)
		*plExtUsed = hPP->pEdit->lTotalExtents;
	if (lMin)
		*lMin = hPP->pEdit->lMinExt;
	if (lMax)
		*lMax = hPP->pEdit->lMaxExt;

}




//-----------------------------------------------------------------------------

long lRenderEdit_PP(H_PP* hPP)        // PP Object
{
	DEBUG_STACK;

	short      sCount = 0;
	long       lPP_ERROR = NO_ERROR;
	long       lCurExt;
	ppedt_ob*  pEdit;
	extent_ob* pExt;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode
	pEdit = hPP->pEdit;


	// --- Start at the begining ---

	sStartNext_VVV(pEdit->hVVV, 0L, 0);


	// --- Scan through all the PMF'S ---

	for (lCurExt = 0L; lCurExt < pEdit->lTotalExtents; lCurExt++)
	{
		// --- Get the pointer ---

		pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);


		// --- Is valid extent ? ---

		if ((pExt->lOffset & (EXT_GRAPHIC | EXT_ERASED)) == EXT_GRAPHIC)
			lPP_ERROR = lIDoPMF_PP(hPP, pExt, (short) (lCurExt >= pEdit->lSaveExtent), &sCount);
		if (sCount == -1)
			break;
	}
	

	// --- Return PMF_PP_ERROR Code ---

	if (lPP_ERROR)
	{
		// --- File Corrupt PMF_PP_ERROR ---

		RegistErr_AP(hPP->hAP, 20907, modn);
		SetParmL_AP(hPP->hAP, "%1", lPP_ERROR);
	}
	return (lPP_ERROR);

}


//-----------------------------------------------------------------------------

long lRecomputeExt_PP(H_PP* hPP)      // PP Object
{
	DEBUG_STACK;

	long       lPP_ERROR = NO_ERROR;
	long       lCurExt;
	ppedt_ob*  pEdit;
	H_PMF*     hPMF;
	extent_ob* pExt;
	Entity_PMF Pmf;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode
	pEdit = hPP->pEdit;

	// --- Prepare to scan trough ---

	pEdit->lMinExt.x = GS_S4MX;
	pEdit->lMinExt.y = GS_S4MX;
	pEdit->lMaxExt.x = GS_S4MN;
	pEdit->lMaxExt.y = GS_S4MN;
	sStartNext_VVV(pEdit->hVVV, 0L, 0);


	// --- Scan through all the PMF'S ---

	for (lCurExt = 0L; lCurExt < pEdit->lTotalExtents; lCurExt++)
	{
		// --- Get the Pointer ---

		pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);

		// --- Does this extent require recomputation ? ---

		if ((pExt->lOffset & (EXT_GRAPHIC | EXT_ERASED | EXT_ATTRIB)) == (EXT_GRAPHIC | EXT_ATTRIB))
		{
			// --- What file do we use ? ---

			if (lCurExt < hPP->pEdit->lSaveExtent)
				hPMF = hPP->pEdit->hPMF;
			else
				hPMF = hPP->pEdit->hSPMF;


			// --- Seek to proper location ---

			if (Seek_PMF(hPMF, pExt->lOffset & EXT_LOWER, BF_SEEK_START) == -1)
				return (PMF_PP_ERROR);


			// --- Read the PMF Header ---

			if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
				return (lPP_ERROR);


			// --- Modify the Extents ---

			lPP_ERROR = lExtPmf_PP(hPP, hPMF, &Pmf, pExt->lOffset, pExt);
		}


		// --- Update the Global Extents ---

		if ((pExt->lOffset & (EXT_GRAPHIC | EXT_ERASED)) == EXT_GRAPHIC)
		{
			if (pExt->lMin.x < hPP->pEdit->lMinExt.x)
				hPP->pEdit->lMinExt.x = pExt->lMin.x;
			if (pExt->lMin.y < hPP->pEdit->lMinExt.y)
				hPP->pEdit->lMinExt.y = pExt->lMin.y;
			if (pExt->lMax.x > hPP->pEdit->lMaxExt.x)
				hPP->pEdit->lMaxExt.x = pExt->lMax.x;
			if (pExt->lMax.y > hPP->pEdit->lMaxExt.y)
				hPP->pEdit->lMaxExt.y = pExt->lMax.y;
		}
	}

	// --- Return PMF_PP_ERROR Code ---

	if (lPP_ERROR)
	{
		// --- File Corrupt PMF_PP_ERROR ---

		RegistErr_AP(hPP->hAP, 20907, modn);
		SetParmL_AP(hPP->hAP, "%1", lPP_ERROR);
	}
	return (lPP_ERROR);

}



//*****************************************************************************
//                              EXTENT MAINTENANCE FUNCTIONS
//*****************************************************************************

//-----------------------------------------------------------------------------

long lFindExt_PP(H_PP*    hPP,          // PP Object
                 Point_DD lMin,         // Lower Limit of Region To Check
                 Point_DD lMax)         // Upper Limit of Region To Check
{
	DEBUG_STACK;

	long       lCurExt;
	long       lLastExt;
	extent_ob* pExt;
	ppedt_ob*  pEdit;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode
	pEdit = hPP->pEdit;

	// --- System Protection ---

	if (pEdit->lTotalExtents == 0)
		return (-1);


	// --- Prepare to scan trough ---

	lLastExt = lCurExt = pEdit->lLastExtent;
	if (lCurExt >= pEdit->lTotalExtents)
	{
		lCurExt = lLastExt = 0L;
	}
	sStartNext_VVV(pEdit->hVVV, lCurExt, 0);

	// --- Scan Through the Extents list for the proper extent ---

	do
	{
		// --- Get Pointer ---

		pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);

		// --- Does this Entity Match Search ? ---

		if ((pExt->lOffset & (EXT_GRAPHIC | EXT_ERASED)) == EXT_GRAPHIC)
		{
			// --- Do the Exents Fit ? ---

			if (IRect_PP(&lMin, &lMax, &pExt->lMin, &pExt->lMax) > 0)
			{
				// --- We have a Match !! ---

				pEdit->lLastExtent = lCurExt + 1;
				
				return (lCurExt);
			}
		}

		// --- Did we go past the end ? ---

		lCurExt++;
		if (lCurExt >= pEdit->lTotalExtents)
		{
			lCurExt = 0;
			sStartNext_VVV(pEdit->hVVV, 0L, 0);
		}

	} while (lCurExt != lLastExt);

	
	return (-1L);                // Not Found

}

//-----------------------------------------------------------------------------

long lListExt_PP(H_PP*     hPP,         // PP Object
                 Point_DD* lMin,        // Minimum Region to Check
                 Point_DD* lMax,        // Maximum Region to Check
                 H_VV*     hVV,         // VV to place the data into
                 long*     lNumb)       // Number of Elements in VV used
{
	DEBUG_STACK;

	long       lPP_ERROR = NO_ERROR;
	long       lCurExt = 0L;
	ppedt_ob*  pEdit = hPP->pEdit;
	extent_ob* pExt;
	Point_DD   lExtMin;
	Point_DD   lExtMax;



	// --- Validate Parameters ---

	ASSERT(hPP && hVV && lMin && lMax && lNumb); // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);            // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);                  // Must be In Edit Mode
#ifdef DEBUG_CODE
	{
		I_VV Info;
		Info_VV(hVV, &Info);
		ASSERT(Info.Type == GS_LONG);
	}
#endif
	*lNumb = 0L;


	// --- Setup our own extents ---

	lExtMin.x = GS_S4MX;
	lExtMin.y = GS_S4MX;
	lExtMax.x = GS_S4MN;
	lExtMax.y = GS_S4MN;


	// --- Set List Size to Zero ---

	if (SetLn_VV(hVV, 0))
		return (PMF_PP_ERROR);


	// --- Prepare to scan trough ---

	sStartNext_VVV(pEdit->hVVV, 0L, 0);


	// --- Scan through all the PMF'S ---

	for (lCurExt = 0L; lCurExt < pEdit->lTotalExtents; lCurExt++)
	{
		// --- Is valid extent ? ---

		pExt = (extent_ob*) pNext_VVV(pEdit->hVVV);
		if ((pExt->lOffset & (EXT_GRAPHIC | EXT_ERASED)) == EXT_GRAPHIC)
		{

			// --- Do the Exents Fit ? ---

			if (IRect_PP(lMin, lMax, &pExt->lMin, &pExt->lMax) == 2)
			{
				// --- We have a Match !! ---

				if (SetLn_VV(hVV, (*lNumb) + 1L))
				{
					lPP_ERROR = PMF_PP_ERROR;
					goto END_LIST;
				}
				if (SetE_VV(hVV, *lNumb, &lCurExt))
				{
					lPP_ERROR = PMF_PP_ERROR;
					goto END_LIST;
				}
				(*lNumb)++;

				// --- Check the Extents ---

				if (pExt->lMin.x < lExtMin.x)
					lExtMin.x = pExt->lMin.x;
				if (pExt->lMin.y < lExtMin.y)
					lExtMin.y = pExt->lMin.y;
				if (pExt->lMax.x > lExtMax.x)
					lExtMax.x = pExt->lMax.x;
				if (pExt->lMax.y > lExtMax.y)
					lExtMax.y = pExt->lMax.y;
			}
		}
	}

END_LIST:

	


	// --- Update the Region ---

	*lMin = lExtMin;
	*lMax = lExtMax;


	// --- Return PMF_PP_ERROR Code ---

	if (lPP_ERROR)
	{
		// --- File Corrupt PMF_PP_ERROR ---

		RegistErr_AP(hPP->hAP, 20907, modn);
		SetParmL_AP(hPP->hAP, "%1", lPP_ERROR);
	}
	return (lPP_ERROR);

}


//-----------------------------------------------------------------------------

short sLimitExt_PP(H_PP*     hPP,       // PP Object
                   Point_DD* lMin,      // Minimum Region to Check
                   Point_DD* lMax,      // Maximum Region to Check
                   H_VV*     hVV,       // VV holding object ID's
                   long      lNumb)     // Number of Elements in the VV
{
	DEBUG_STACK;

	ppedt_ob*  pEdit = hPP->pEdit;
	long       lCur = 0L;
	long       lId;
	H_VVV*     hVVV;
	extent_ob* pExt;



	// --- Validate Parameters ---

	ASSERT(hPP && hVV && lMin && lMax);  // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode
#ifdef DEBUG_CODE
	{
		I_VV Info;
		Info_VV(hVV, &Info);
		ASSERT(Info.Type == GS_LONG);
	}
#endif

	// --- Setup our own extents ---

	lMin->x = GS_S4MX;
	lMin->y = GS_S4MX;
	lMax->x = GS_S4MN;
	lMax->y = GS_S4MN;


	// --- Prepare to scan trough ---

	hVVV = hCreat_VVV(hVV);
	if (hVVV == NULL)
		return (1);
	sStartNext_VVV(hVVV, 0L, 0);


	// --- Scan through all the PMF'S ---

	for (lCur = 0L; lCur < lNumb; lCur++)
	{
		// --- Is valid extent ? ---

		lId = Next_VVV<int32_t>(hVVV);
		pExt = (extent_ob*) pPoint_VVV(pEdit->hVVV, lId);

		// --- Check the Extents ---

		if (pExt->lMin.x < lMin->x)
			lMin->x = pExt->lMin.x;
		if (pExt->lMin.y < lMin->y)
			lMin->y = pExt->lMin.y;
		if (pExt->lMax.x > lMax->x)
			lMax->x = pExt->lMax.x;
		if (pExt->lMax.y > lMax->y)
			lMax->y = pExt->lMax.y;
	}

	Destr_VVV(hVVV);
	return (0);

}



//-----------------------------------------------------------------------------

long lDoEditPmf_PP(H_PP* hPP,           // PP Object
                   long  lExtId)        // Extent ID of PMF to Render
{
	DEBUG_STACK;

	long      lPP_ERROR = NO_ERROR;
	extent_ob extent;
	short     sCount = 0;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode
	ASSERT(lExtId >= 0);                 // Must be a valid Extent

	// --- Get the Appropriate Extent ---

	if (GetE_VV(hPP->pEdit->hVV, lExtId, &extent))
		return (PMF_PP_ERROR);

	// --- Render the Entity ---

	lPP_ERROR = lIDoPMF_PP(hPP, &extent, (short) (lExtId >= hPP->pEdit->lSaveExtent), &sCount);

	// --- Return the PMF_PP_ERROR ---

	return (lPP_ERROR);

}


//-----------------------------------------------------------------------------

long lGetExtInfo_PP(H_PP*     hPP,      // PP Object
                    long      lId,      // ID of Extent
                    Point_DD* lMin,     // Minimum Extent
                    Point_DD* lMax,     // Maximum Extent
                    short*    sNumber,  // Number of PMF
                    short*    sGraphic, // Graphic PMF?
                    short*    sOld)     // Old Pmf ?
{
	DEBUG_STACK;

	extent_ob extent;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);          // Must be In Edit Mode


	// --- Get the Information ---

	if (GetE_VV(hPP->pEdit->hVV, lId, &extent))
		return (PMF_PP_ERROR);


	// --- Return the Information ---

	if (lMin)
		*lMin = extent.lMin;
	if (lMax)
		*lMax = extent.lMax;
	if (sNumber)
		*sNumber = extent.sNumber;
	if (sGraphic)
		*sGraphic = (short) ((extent.lOffset & EXT_GRAPHIC) && 1);
	if (sOld)
		*sOld = (short) ((extent.lOffset & EXT_OLD) && 1);

	return (NO_ERROR);

}

//-----------------------------------------------------------------------------

long lGetExtAttr_PP(H_PP*   hPP,        // PP Object
                    long    lId,        // ID of Extent
                    short*  pAttr,      // Attribute of Object
                    long*   pOver,      // Overides of Object
                    Att_PP* pAtt)       // Attribute Structure
{
	DEBUG_STACK;

	extent_ob  extent;
	long       lPP_ERROR;
	H_PMF*     hPMF;
	Entity_PMF Pmf;


	// --- Validate Parameters ---

	ASSERT(hPP && pAttr && pOver && pAtt);       // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);            // Was it really a PP object ?
	ASSERT(hPP->pEdit != NULL);                  // Must be In Edit Mode


	// --- Get the Information ---

	if (GetE_VV(hPP->pEdit->hVV, lId, &extent))
		return (PMF_PP_ERROR);


	// --- What file do we use ? ---

	if (lId < hPP->pEdit->lSaveExtent)
		hPMF = hPP->pEdit->hPMF;
	else
		hPMF = hPP->pEdit->hSPMF;


	// --- Seek to proper location ---

	if (Seek_PMF(hPMF, extent.lOffset & EXT_LOWER, BF_SEEK_START) == -1)
		return (PMF_PP_ERROR);


	// --- Is this a series of objects ? ---

	if (extent.lOffset & EXT_OLD)
	{
		// --- Old Pmf's are attribute 0 ---

		*pAttr = 0;
		*pOver = 0;

	}
	else
	{

		// --- Read the PMF ---

		if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
			return (lPP_ERROR);


		// --- Get the Proper Attribute ---

		*pAttr = Pmf.sAttrib;

		if (Pmf.sAttrib <= 0)
		{
			// --- Interpret the Attribute Information ---

			GetAttT_PP(hPP, (short) -Pmf.sAttrib, pAtt);
			if ((lPP_ERROR = lAttrPmf_PP((CON_BYT*) Pmf.pOveride, Pmf.lOveride, pAtt, pOver)) != 0)
				return (lPP_ERROR);
			*pAttr = -Pmf.sAttrib;

		}
		else
		{

			// --- No Overides ---

			GetAttT_PP(hPP, Pmf.sAttrib, pAtt);
			*pAttr = Pmf.sAttrib;
			*pOver = 0;
		}
	}
	return (NO_ERROR);

}





//*****************************************************************************
//                                INTERNAL FUNCTIONS
//*****************************************************************************


//-----------------------------------------------------------------------------

long lMakeAttrPmf_PP(Att_PP*  Attr,     // Attribute overide to use
                     VOL_BYT* pBuff,    // Buffer to put the data
                     long     lOveride) // Which attributes to write ?
{
	DEBUG_STACK;

	long     lCur = 0;
	ColorMix Mix;


	// --- Validate Parameters ----

	ASSERT(Attr && pBuff);       // Must be Real


	// --- Generate LINE COLOR Overide ---

	if (lOveride & ATR_LCOL)
	{
		GetColor(&Attr->Lcol, &Mix, C_KCMY);
		*((short*) pBuff) = 1;
		pBuff += 2;               // LINE COLOR
		*pBuff = Mix.indx[3];
		pBuff++;
		*pBuff = Mix.indx[0];
		pBuff++;
		*pBuff = Mix.indx[1];
		pBuff++;
		*pBuff = Mix.indx[2];
		pBuff++;
		lCur += 6;
	}

	// --- Generate LINE THICKNESS Overide ---

	if (lOveride & ATR_LTHK)
	{
		*((short*) pBuff) = 2;
		pBuff += 2;               // LINE THICKNESS
		*((short*) pBuff) = (short) Attr->Lthk;
		pBuff += 2;
		lCur += 4;
	}

	// --- Generate LINE PATTERN Overide ---

	if (lOveride & ATR_LPAT)
	{
		*((short*) pBuff) = 3;
		pBuff += 2;               // LINE PATTERN
		*((short*) pBuff) = Attr->Lpat;
		pBuff += 2;
		lCur += 4;
	}

	// --- Generate LINE PITCH Overide ---

	if (lOveride & ATR_LPTC)
	{
		*((short*) pBuff) = 4;
		pBuff += 2;               // LINE PITCH
		*((float*) pBuff) = (float) Attr->Lptc / (float) 1000.0;
		pBuff += 4;
		lCur += 6;
	}

	// --- Generate FILL COLOR Overide ---

	if (lOveride & ATR_FCOL)
	{
		GetColor(&Attr->Fcol, &Mix, C_KCMY);
		*((short*) pBuff) = 5;
		pBuff += 2;               // FILL COLOR
		*pBuff = Mix.indx[3];
		pBuff++;
		*pBuff = Mix.indx[0];
		pBuff++;
		*pBuff = Mix.indx[1];
		pBuff++;
		*pBuff = Mix.indx[2];
		pBuff++;
		lCur += 6;
	}

	// --- Generate TEXT STYLE Overide ---

	if (lOveride & ATR_TSTY)
	{
		*((short*) pBuff) = 6;
		pBuff += 2;               // TEXT STYLE
		*((float*) pBuff) = (float) Attr->Tsty.ht / (float) 1000.0;
		pBuff += 4;
		*((float*) pBuff) = (float) Attr->Tsty.wd / (float) 1000.0;
		pBuff += 4;
		*((float*) pBuff) = (float) Attr->Tsty.sp / (float) 1000.0;
		pBuff += 4;
		*((float*) pBuff) = (float) Attr->Tsty.sl;
		pBuff += 4;
		lCur += 18;
	}


	// --- Generate FONT Overide ---

	if (lOveride & ATR_FONT)
	{
		*((short*) pBuff) = 7;
		pBuff += 2;               // FONT
		memset(pBuff, 32, 8);
		memcpy(pBuff, Attr->Font, strlen(Attr->Font));
		pBuff += 8;
		lCur += 10;
	}

	// --- Return Bytes Used ---

	return ((((lCur + 3) / 4) * 4));

}





//-----------------------------------------------------------------------------

long lAddExt_PP(H_PP*      hPP,         // PP Object
                extent_ob* pExt)        // Extent to Add
{
	DEBUG_STACK;


	// --- Validate Parameters ---

	ASSERT(hPP && pExt);                 // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit);                  // Must be in EDIT mode

	// --- Reallocate the VV object ---

	if (hPP->pEdit->lTotalExtents >= hPP->pEdit->lAlocatedExtents)
	{
		// --- Alocate More Extents Storage ---

		hPP->pEdit->lAlocatedExtents += 100L;
		if (SetLn_VV(hPP->pEdit->hVV, hPP->pEdit->lAlocatedExtents))
			return (-1);
	}


	// --- Determine the Extents if it is graphics ---

	if (pExt->lOffset & EXT_GRAPHIC)
	{
		// --- Update the Maximal Extents ---

		if (pExt->lMin.x < hPP->pEdit->lMinExt.x)
			hPP->pEdit->lMinExt.x = pExt->lMin.x;
		if (pExt->lMin.y < hPP->pEdit->lMinExt.y)
			hPP->pEdit->lMinExt.y = pExt->lMin.y;
		if (pExt->lMax.x > hPP->pEdit->lMaxExt.x)
			hPP->pEdit->lMaxExt.x = pExt->lMax.x;
		if (pExt->lMax.y > hPP->pEdit->lMaxExt.y)
			hPP->pEdit->lMaxExt.y = pExt->lMax.y;
	}


	// --- Set the Element ---

	SetE_VV(hPP->pEdit->hVV, hPP->pEdit->lTotalExtents, pExt);


	// --- Return the Element ---

	return (hPP->pEdit->lTotalExtents++);

}


//-----------------------------------------------------------------------------

long lCreateAtt_PP(H_AP*    hAP,        // AP Handle
                   H_VV*    hVV,        // VV holding attributes
                   short    sAttribs,   // Number of attributs to write
                   CON_STR* szName)     // Name of Plot File
{
	DEBUG_STACK;

	H_PMF*     hPMF;
	Entity_PMF Pmf;
	Att_PP     Attr;

	SYS_ALIGNED_STACK_BUFFER(pBuff, 100);

	long       lNameSize = 0L;
	long       lPP_ERROR = NO_ERROR;
	long       lEndLoc = 0L;
	short      i;
	PMF_ATTRIB IAttr;
	ColorMix   Mix;


	// --- Validate Parameters ---

	ASSERT(hAP && szName);

	// --- Create the Attribute File ---

	hPMF = hCreat_PMF(hAP, szName, 1, BF_READWRITE_NEW);
	if (hPMF == NULL)
	{
		RegistErr_AP(hAP, 20906, modn);
		SetParmS_AP(hAP, "%1", szName);
		return (1);
	}

	// --- Change to Write Mode ---

	Mode_PMF(hPMF, WRITE_MODE);


	// --- Generate the First PMF ---

	Pmf.sNumber = 1;
	Pmf.lDataSize = 4L;
	Pmf.pData = (VOL_STR*) pBuff;
	Pmf.lOtherData = -1;
	Pmf.sGraphicsEntity = 0;
	Pmf.pOveride = NULL;

	((PMF_1*) pBuff)->sVersion = 2;
	((PMF_1*) pBuff)->sRevision = 1;
	lPP_ERROR = lWrite_PMF(hPMF, &Pmf);


	// --- Write PMF 2001 ---

	if (!lPP_ERROR)
	{
		Pmf.sNumber = 2001;
		Pmf.pData = (VOL_STR*) &lEndLoc;
		Pmf.lOtherData = -1;
		lPP_ERROR = lWrite_PMF(hPMF, &Pmf);
	}


	// --- Write PMF 3000 ---

	if (hVV && (sAttribs > 0))
	{
		if (!lPP_ERROR)
		{
			Pmf.sNumber = 3000;
			Pmf.pData = (VOL_STR*) pBuff;
			Pmf.lDataSize = 4L;
			Pmf.lOtherData = (long) ATTRIBUTE_SIZE * (long) (sAttribs - 1);
			((PMF_3000*) pBuff)->sMaxAttrib = ATTRIBS_IN_BLOCK;
			((PMF_3000*) pBuff)->sNumbAttribs = (short) (sAttribs - 1);
			lPP_ERROR = lWrite_PMF(hPMF, &Pmf);
		}

		// --- Write Data To PMF 2000 ---

		for (i = 1; i < sAttribs; i++)
		{
			if (lPP_ERROR)
				break;

			// --- Convert The Attribute ---

			GetE_VV(hVV, i, &Attr);


			// --- Convert the Forground Color ---

			GetColor(&Attr.Lcol, &Mix, C_KCMY);
			IAttr.bColors[0] = Mix.K.byK;
			IAttr.bColors[1] = Mix.K.byC;
			IAttr.bColors[2] = Mix.K.byM;
			IAttr.bColors[3] = Mix.K.byY;


			// --- Convert the Background color ---

			GetColor(&Attr.Fcol, &Mix, C_KCMY);
			IAttr.bFColors[0] = Mix.K.byK;
			IAttr.bFColors[1] = Mix.K.byC;
			IAttr.bFColors[2] = Mix.K.byM;
			IAttr.bFColors[3] = Mix.K.byY;


			// --- Set all the Other Attributes ---

			IAttr.sThick = (short) Attr.Lthk;
			IAttr.sLinePat = Attr.Lpat;
			IAttr.rPitch = (float) Attr.Lptc / (float) 1000.0;
			IAttr.rTHeight = (float) Attr.Tsty.ht / (float) 1000.0;
			IAttr.rTWidth = (float) Attr.Tsty.wd / (float) 1000.0;
			IAttr.rTSpace = (float) Attr.Tsty.sp / (float) 1000.0;
			if (fabs(Attr.Tsty.sl) > 89.0)
				Attr.Tsty.sl = 0.0;
			IAttr.rTSlant = (float) Attr.Tsty.sl;
			memcpy(IAttr.szFont, Attr.Font, 8);


			// --- Write out an attribute ---

			if (sWriteData_PMF(hPMF, &IAttr, ATTRIBUTE_SIZE) == -1)
				lPP_ERROR = PMF_PP_ERROR;


			// --- Compute the size of the name ---

			lNameSize += strlen(Attr.Name) + 1;
		}
		lNameSize++;              // Remember the last NULL


		// --- Write PMF 3002 ---

		if (!lPP_ERROR)
		{
			Pmf.sNumber = 3002;
			Pmf.lDataSize = 0L;
			Pmf.lOtherData = lNameSize;
			lPP_ERROR = lWrite_PMF(hPMF, &Pmf);
		}


		// --- Write Data To PMF 2000 ---

		for (i = 1; i < sAttribs; i++)
		{
			if (lPP_ERROR)
				break;

			// --- Write out an attribute ---

			GetE_VV(hVV, i, &Attr);
			if (sWriteData_PMF(hPMF, Attr.Name, strlen(Attr.Name) + 1) == -1)
				lPP_ERROR = PMF_PP_ERROR;
			Pmf.lOtherData -= strlen(Attr.Name) + 1;
		}

		// --- Write Last NULL to data ---

		if (!lPP_ERROR)
		{
			memset(pBuff, 0, 10);
			if (sWriteData_PMF(hPMF, pBuff, Pmf.lOtherData) == -1)
				lPP_ERROR = PMF_PP_ERROR;
		}
	}

	// --- Remember the Last Data Location ---

	lEndLoc = Seek_PMF(hPMF, 0, BF_SEEK_CURRENT);


	// --- Write PMF 9999 ---

	if (!lPP_ERROR)
	{
		Pmf.sNumber = 9999;
		Pmf.lDataSize = 0L;
		Pmf.lOtherData = -1L;
		lPP_ERROR = lWrite_PMF(hPMF, &Pmf);
	}

	// --- Update the End Of File ---

	if (Seek_PMF(hPMF, 20L, BF_SEEK_START) == -1)
		lPP_ERROR = PMF_PP_ERROR;
	if (!lPP_ERROR)
		lPP_ERROR = sWriteData_PMF(hPMF, &lEndLoc, 4);

	// --- Close the File ---

	if (hPMF)
		Destr_PMF(hPMF, BF_KEEP);


	// --- Report any PMF_PP_ERRORs ---

	if (lPP_ERROR)
	{
		RegistErr_AP(hAP, 20904, modn);
		SetParmS_AP(hAP, "%1", szName);
		SetParmL_AP(hAP, "%2", lPP_ERROR);
	}

	return (lPP_ERROR);

}


//-----------------------------------------------------------------------------

short IRect_PP(Point_DD* lMin,          // Minimum Region
               Point_DD* lMax,          // Maximum Region
               Point_DD* lOMin,         // Object Minimum Region
               Point_DD* lOMax)         // Object Maximum Region
{
	DEBUG_STACK;

	short x1, x2, y1, y2, res, sx, sy;


	// --- Validate Parameters ---

	ASSERT(lMin && lMax && lOMin && lOMax);      // Must Be real


	// --- Determine the Object Point Information ---

	x1 = (short) ((lOMin->x >= lMin->x) && (lOMin->x <= lMax->x));
	x2 = (short) ((lOMax->x >= lMin->x) && (lOMax->x <= lMax->x));

	y1 = (short) (((lOMin->y >= lMin->y) && (lOMin->y <= lMax->y)) * 8);
	y2 = (short) (((lOMax->y >= lMin->y) && (lOMax->y <= lMax->y)) * 8);

	res = x1 + x2 + y1 + y2;


	// --- Is the Object Visible ? ---

	if (res == 18)
		return (2);               // All Inside
	else if ((res > 8) && (res != 16))
		return (1);               // Patially Inside


	// --- The Object Span the Region ? ---

	sx = (short) ((lOMin->x < lMin->x) && (lOMax->x > lMax->x));
	sy = (short) ((lOMin->y < lMin->y) && (lOMax->y > lMax->y));


	// --- Is the Object Visible ? ---

	if ((sx && (y1 + y2)) || (sy && (x1 + x2)) || (sx && sy))
		return (1);


	// --- Object Not Visible ---

	return ((short) NO_ERROR);

}



//*****************************************************************************
//                           EXTENT COMPUTATION FUNCTIONS
//*****************************************************************************


//-----------------------------------------------------------------------------

long lExtPmf_PP(H_PP*         hPP,      // PP Object
                H_PMF*        hPMF,     // PMF Object
                Entity_PMF*   pPmf,     // PMF to add to extents list
                unsigned long lOffset,  // Offset Into the File
                extent_ob*    pExt)     // Where to put the extent
{
	DEBUG_STACK;

	Att_PP Attr;
	long   lOveride;
	long   lPP_ERROR = NO_ERROR;


	// --- Validate Parameters ---

	ASSERT(hPP && pPmf && pExt);         // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(hPP->pEdit);                  // Must be in EDIT mode


	// --- Prepare a generic Extents Object ---

	hPP->sDoExtents = 1;         // Turn it on
	pExt->lOffset = lOffset;
	pExt->sNumber = pPmf->sNumber;


	// --- Get the Attribute Information ---

	if (pPmf->sGraphicsEntity)
	{
		if (pPmf->sAttrib > 0)
		{
			// --- This Entity is dependent on the attribute ---

			if ((pPmf->sNumber == 26) || (pPmf->sNumber == 28) || (pPmf->sNumber == 29))
				pExt->lOffset |= EXT_ATTRIB;
			SetAtt_PP(hPP, pPmf->sAttrib);

		}
		else
		{

			// --- Interpret the Attribute Information ---

			GetAttT_PP(hPP, (short) -pPmf->sAttrib, &Attr);
			if ((lPP_ERROR = lAttrPmf_PP((CON_BYT*) pPmf->pOveride, pPmf->lOveride, &Attr, &lOveride)) != 0)
				goto END_EXT;
			SetCurAtt_PP(hPP, &Attr);
		}
	}

	// --- Set the Internal PP Rendering to exetents generation ---

	hPP->lMin.x = GS_S4MX;
	hPP->lMin.y = GS_S4MX;
	hPP->lMax.x = GS_S4MN;
	hPP->lMax.y = GS_S4MN;

	// --- Render the PMF ---

	if ((lPP_ERROR = lDoPmf_PP(hPP, hPMF, pPmf)) != 0)
		goto END_EXT;

	// --- Get the Extents ---

	pExt->lMin = hPP->lMin;
	pExt->lMax = hPP->lMax;


	// --- Add the Extent ---

	if (pExt->lMin.x != GS_S4MX)
		pExt->lOffset |= EXT_GRAPHIC;

END_EXT:
	hPP->sDoExtents = 0;         // Turn it off
	return (lPP_ERROR);

}


//-----------------------------------------------------------------------------

long lIDoPMF_PP(H_PP*      hPP,         // PP Object
                extent_ob* extent,      // Extent object
                short      sSave,       // In a Save File ?
                short*     sCount)      // Count Value
{
	DEBUG_STACK;

	H_PMF*     hPMF;
	long       lBlocks;
	long       lCurBlock;
	long       lPP_ERROR = NO_ERROR;
	Entity_PMF Pmf;


	// --- Validate Parameters ---

	ASSERT(hPP && extent && sCount);             // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);            // Was it really a PP object ?
	ASSERT((sSave == 0) || (sSave == 1));        // Valid Save ?


	// --- What file do we use ? ---

	if (sSave)
		hPMF = hPP->pEdit->hSPMF;
	else
		hPMF = hPP->pEdit->hPMF;


	// --- Seek to proper location ---

	if (Seek_PMF(hPMF, (extent->lOffset & EXT_LOWER), BF_SEEK_START) == -1)
		return (PMF_PP_ERROR);


	// --- Is this a series of objects ? ---

	if (extent->lOffset & EXT_OLD)
		lBlocks = (unsigned short) extent->sNumber;
	else
		lBlocks = 1L;


	// --- Render the PMF's in this block ---

	for (lCurBlock = 0L; lCurBlock < lBlocks; lCurBlock++)
	{

		// --- Read the PMF Header ---

		if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
			return (lPP_ERROR);


		// --- Render the Object ---

		if ((lPP_ERROR = lDoPmf_PP(hPP, hPMF, &Pmf)) != 0)
			return (lPP_ERROR);


		// --- Do we need to stop ? ---

		if ((*sCount)++ == 25)
		{
			if (CheckStop_AP(hPP->hAP))
			{
				(*sCount) = -1;
				return (NO_ERROR);
			}
			(*sCount) = 0;
		}
	}
	return (NO_ERROR);

}


//-----------------------------------------------------------------------------

void IPExt_PP(H_PP*     hPP,    // PP handle
              Point_DD* pts,    // Points
              long      npts)   // number of points in the chain
{
	DEBUG_STACK;

	long l;


	// --- Validate Parameters ---

	ASSERT(hPP && pts);                  // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT((npts > 0) && (pts != NULL)); // Valid point table passed in


	// --- Check all the Points ---

	for (l = 0; l < npts; l++)
	{
		// --- Check the Limits ---

		if (pts[l].x < hPP->lMin.x)
			hPP->lMin.x = pts[l].x;
		if (pts[l].y < hPP->lMin.y)
			hPP->lMin.y = pts[l].y;
		if (pts[l].x > hPP->lMax.x)
			hPP->lMax.x = pts[l].x;
		if (pts[l].y > hPP->lMax.y)
			hPP->lMax.y = pts[l].y;
	}

}

//-----------------------------------------------------------------------------

void ITrans_PP(H_PP*     hPP,           // PP Object
               long      lEntries,      // Number of entries to write
               VOL_BYT*  pData,         // Location of data
               short     sConv,         // Conversion Factor
               Point_DD* lOldOrigin,    // Old Origin
               Point_DD* lNewOrigin,    // New Origin
               Point_PP* dScale)        // New Scale
{
	DEBUG_STACK;

	register long i;
	Point_PP      dP;
	Point_DD      lP;


	// --- Validate Parameters ---

	ASSERT(hPP && pData);                // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- What kind of data are we working with ? ---

	if (sConv == CONV_SHORT)
	{
		// --- Convert and shift ---

		for (i = 0; i < lEntries; i++)
		{
			// --- Add the Points to the Buffer ---

			lP.x = RoundLong_GS(((SPoint*) pData)[i].sX * 50L) - hPP->PMFOrigin;
			lP.y = RoundLong_GS(((SPoint*) pData)[i].sY * 50L) - hPP->PMFOrigin;

			dP.x = ((double) (lP.x - lOldOrigin->x) * dScale->x) + (double) lNewOrigin->x;
			dP.y = ((double) (lP.y - lOldOrigin->y) * dScale->y) + (double) lNewOrigin->y;

			((SPoint*) pData)[i].sX = (short) ((dP.x + (double) hPP->PMFOrigin) / 50.0);
			((SPoint*) pData)[i].sY = (short) ((dP.y + (double) hPP->PMFOrigin) / 50.0);
		}

	}
	else
	{

		// --- Convert and shift ---

		for (i = 0; i < lEntries; i++)
		{
			// --- Add the Points to the Buffer ---

			dP.x = (double) ((FPoint*) pData)[i].fX;
			dP.y = (double) ((FPoint*) pData)[i].fY;
			lP = Trans_PP(hPP, dP);

			((FPoint*) pData)[i].fX = (float) ((((double) lP.x - (double) lOldOrigin->x) * dScale->x) + (double) lNewOrigin->x) / (float) 1000.0;
			((FPoint*) pData)[i].fY = (float) ((((double) lP.y - (double) lOldOrigin->y) * dScale->y) + (double) lNewOrigin->y) / (float) 1000.0;
		}
	}

}



//-----------------------------------------------------------------------------

long lIData_PP(H_PP*    hPP,            // PP Object
               long     lEntries,       // Number of entries to write
               VOL_BYT* pData)          // Location of data
{
	DEBUG_STACK;

	long lOldSeek;


	// --- Validate Parameters ---

	ASSERT(hPP);
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Does it fit in the buffer ? ---

	if ((hPP->pEdit->lBuffCounter + lEntries >= PMF_PP_WRITE_BUFFER) || (lEntries >= PMF_PP_WRITE_BUFFER) || (lEntries < 0))
	{

		// --- Flush the Buffer ---

		lOldSeek = Seek_PMF(hPP->pEdit->hSPMF, 0, BF_SEEK_CURRENT);
		Mode_PMF(hPP->pEdit->hSPMF, WRITE_MODE);
		if (Seek_PMF(hPP->pEdit->hSPMF, 0, BF_SEEK_EOF) == -1)
			return (PMF_PP_ERROR);

		if (hPP->pEdit->lBuffCounter > 0)
			if (sWriteData_PMF(hPP->pEdit->hSPMF, hPP->pEdit->pWriteBuffer, hPP->pEdit->lBuffCounter) == -1)
				return (PMF_PP_ERROR);
		if (lEntries > 0)
			if (sWriteData_PMF(hPP->pEdit->hSPMF, pData, lEntries) == -1)
				return (PMF_PP_ERROR);


		Mode_PMF(hPP->pEdit->hSPMF, READ_MODE);
		if (Seek_PMF(hPP->pEdit->hSPMF, lOldSeek, BF_SEEK_START) == -1)
			return (PMF_PP_ERROR);
		hPP->pEdit->lBuffCounter = 0;

	}
	else
	{

		// --- Copy the data into the buffer ---

		memcpy(hPP->pEdit->pWriteBuffer + hPP->pEdit->lBuffCounter, pData, lEntries);
		hPP->pEdit->lBuffCounter += lEntries;

	}

	// --- Done ---

	return (NO_ERROR);

}


//-----------------------------------------------------------------------------

long lICopy_PP(H_PP* hPP,       // PP Object
               long* lAt,       // Current At Location
               long  lStart,    // Starting Extent to copy
               long  lEnd,      // Ending Extent to copy
               short sMode)     // 0 - Plot File, 1 - Save File
{
	DEBUG_STACK;

	ppedt_ob* pEdit = hPP->pEdit;
	long      lS, lSize, lShift;
	long      lRead;
	short     sSpecialEnd = 0;
	extent_ob Ext;
	IBF*      hOBF;
	long      lId;
	long      lESize;

	SYS_ALIGNED_STACK_BUFFER(Buff, MIN_BLOCK_SIZE);


	// --- Validate Parameters ---

	ASSERT(hPP);
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- What Mode ? ---

	if (sMode == 0)
	{
		hOBF = ((H_PMF*) pEdit->hPMF)->ipBF;
		if (lEnd == pEdit->lSaveExtent)
		{
			sSpecialEnd = 1;
			lEnd--;
		}
	}
	else
	{
		hOBF = ((H_PMF*) pEdit->hSPMF)->ipBF;
		if (lEnd == pEdit->lTotalExtents)
		{
			sSpecialEnd = 1;
			lEnd--;
		}
	}



	// --- Compute the Copy Locations ---

	if (GetE_VV(pEdit->hVV, lStart, &Ext))
		return (PMF_PP_ERROR);
	lS = Ext.lOffset & EXT_LOWER;

	if (GetE_VV(pEdit->hVV, lEnd, &Ext))
		return (PMF_PP_ERROR);
	if (sSpecialEnd)
	{
		if (sMode == 0)
			lSize = pEdit->lPlotFileSize - lS;
		else
			lSize = hOBF->lSeek(0L, BF_SEEK_EOF) - lS;     // Get the End of File
		lEnd++;
	}
	else
		lSize = (Ext.lOffset & EXT_LOWER) - lS;


	// --- Compute Shifting Factor ---

	lShift = pEdit->lNewFileSize - lS;
	lESize = lEnd - lStart;
	if (lESize <= 0)
		lESize = 1;


	// --- Do the Copying ---

	while (lSize > 0L)
	{
		// --- Compute Block Size ---

		if (lSize > MIN_BLOCK_SIZE)
			lRead = MIN_BLOCK_SIZE;
		else
			lRead = lSize;

		// --- Read the Block ---

		if (hOBF->lSeek(lS, BF_SEEK_START) == -1)
			return (PMF_PP_ERROR);
		if (hOBF->lRead(Buff, lRead) != lRead)
			return (PMF_PP_ERROR);

		// --- Write the Block ---

		if (((H_PMF*) pEdit->hPMF)->ipBF->lSeek(pEdit->lNewFileSize, BF_SEEK_START) == -1)
			return (PMF_PP_ERROR);
		if (((H_PMF*) pEdit->hPMF)->ipBF->lWrite(Buff, lRead) != lRead)
			return (PMF_PP_ERROR);

		// --- Update Pointers ---

		pEdit->lNewFileSize += lRead;
		lS += lRead;
		lSize -= lRead;
	}

	// --- Copy the Extents and Shift at same time ---

	for (lId = lStart; lId < lStart + lESize; lId++)
	{
		if (GetE_VV(pEdit->hVV, lId, &Ext))
			return (PMF_PP_ERROR);
		Ext.lOffset = (Ext.lOffset & EXT_UPPER) + ((Ext.lOffset & EXT_LOWER) + lShift);
		if (SetE_VV(pEdit->hVV, *lAt, &Ext))
			return (PMF_PP_ERROR);
		(*lAt)++;
	}

	// --- Done ---

	return (NO_ERROR);

}

#undef PMF_PP_ERROR
