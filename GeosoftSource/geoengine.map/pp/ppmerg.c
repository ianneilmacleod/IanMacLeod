//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
// PPMERG.c
//
// Geosoft high-level graphics class.
//
// This part of the class performs merging of PMF files.
//
//=============================================================================



#include <col.h>                // COL Utilities
#include <pp.h>                 // PP Header
#include <vv.h>                 // VV Header
#include <vvv.h>                // VVV Header
#include <gs.h>                 // GS Header
#include <pmf.h>                // PMF Header
#include "pp.ih"


#define PMF_PP_ERROR (130000l + __LINE__)


//-----------------------------------------------------------------------------

void IConvPP(Conv_PP* pConv,    // Conversion Control
             long     lEntries, // Number of Entries to Convert
             short    sType,    // Type of Conversion
             // 0 - New to New
             // 1 - Old to Old
             // 1 - Old to New
             VOL_BYT* pIData,   // Location of Input data
             VOL_BYT* pOData)   // Ouput Data
{
	DEBUG_STACK;

	register long i;
	Point_PP      d1, d2;
	Point_DD      lP;


	// --- Main Loop ---

	for (i = 0; i < lEntries; i++)
	{
		// --- Get the Point ---

		if (sType == 0)
		{
			// --- New PMF ---

			d1.x = (double) (((FPoint*) pIData)[i].fX * 1000.0);
			d1.y = (double) (((FPoint*) pIData)[i].fY * 1000.0);

		}
		else
		{

			// --- Old PMF ---

			d1.x = (double) ((((SPoint*) pIData)[i].sX * 50L) - pConv->lOldOrigin);
			d1.y = (double) ((((SPoint*) pIData)[i].sY * 50L) - pConv->lOldOrigin);
		}



		// --- Convert the Point to New System ---

		d2.x = d1.x * pConv->dDeltaScale + pConv->dDeltaOrig.x;
		d2.y = d1.y * pConv->dDeltaScale + pConv->dDeltaOrig.y;



		// --- Store in appropriate location ---

		if (sType == 1)
		{
			// --- Round to New Long ---

			lP.x = RoundLong_GS((d2.x + (double) pConv->lNewOrigin) / 50.0);
			lP.y = RoundLong_GS((d2.y + (double) pConv->lNewOrigin) / 50.0);

			// --- Check Limits ---

			if (lP.x > GS_S2MX)
				lP.x = GS_S2MX;
			if (lP.x < GS_S2MN)
				lP.x = GS_S2MN;
			if (lP.y > GS_S2MX)
				lP.y = GS_S2MX;
			if (lP.y < GS_S2MN)
				lP.y = GS_S2MN;

			// --- Save Data ---

			((SPoint*) pOData)[i].sX = (short) lP.x;
			((SPoint*) pOData)[i].sY = (short) lP.y;

		}
		else
		{

			// --- Save Data in Output Buffer ---

			((FPoint*) pOData)[i].fX = (float) (d2.x / 1000.0);
			((FPoint*) pOData)[i].fY = (float) (d2.y / 1000.0);
		}
	}

}




//-----------------------------------------------------------------------------

long lMergePmfs_PP(H_PMF*   hOPMF,      // Output PMF
                   H_PMF*   hIPMF,      // Input  PMF
                   Conv_PP* pConv,      // Conversion Data
                   short    sIBuff,     // Input Buffer Size (Output is 2*x)
                   VOL_BYT* pIBuff,     // Input Buffer
                   VOL_BYT* pOBuff)     // Ouput Buffer
{
	DEBUG_STACK;

	Entity_PMF Pmf;
	SYS_ALIGNED_STACK_BUFFER(pBuff, 100);

	long       lPP_ERROR = 0;
	long       lBlocks;

	short      sOldAttrib;       // Old Attribute
	short      sDone;

	long       lOveride;
	Att_PP     Attrib;


	// --- Validate Paramters ---

	ASSERT(hIPMF && hOPMF);


	// --- Start the Loop ---

	Seek_PMF(hIPMF, 0L, BF_SEEK_START);
	do
	{

		// --- Read the PMF Header ---

		if ((lPP_ERROR = lRead_PMF(hIPMF, &Pmf, pIBuff, sIBuff)) != 0)
			goto END_PP_ERROR;
		if ((Pmf.sNumber == 9999) || (Pmf.sNumber == 999))
			break;


		// --- What type of PMF is it ? ---

		if ((Pmf.sNumber == 2) || (Pmf.sNumber == 3))
		{
			// --- Convert to New ? ---

			if (pConv->sOldToNew)
			{

				// --- Read the Old Data ---

				sDone = 0;
				lBlocks = lReadOld_PMF(hIPMF, pIBuff, sIBuff, &sDone);
				if (lBlocks == -1)
				{
					lPP_ERROR = PMF_PP_ERROR;
					goto END_PP_ERROR;
				}
				if (lBlocks > 4)
				{
					// --- Set PMF Header ---

					Pmf.sGraphicsEntity = 1;
					Pmf.sAttrib = sOldAttrib;
					Pmf.lOveride = 0;
					Pmf.pOveride = NULL;
					Pmf.pData = (VOL_STR*) pBuff;
					((short*) pBuff)[0] = 0;
					((short*) pBuff)[1] = Pmf.sAttrib;


					// --- How Much Data ? ---

					if (lBlocks > 8)
					{
						// --- Generate a Polyline PMF ---

						Pmf.sNumber = 21;
						Pmf.lDataSize = 4;

						do
						{

							// --- Write out the PMF Header ---

							Pmf.lOtherData = lBlocks * 2;
							lPP_ERROR = lWrite_PMF(hOPMF, &Pmf);
							if (lPP_ERROR)
								goto END_PP_ERROR;

							// --- Translate the Data ---

							IConvPP(pConv, lBlocks / 4, 2, pIBuff, pOBuff);

							// --- Write out the Data ---

							sWriteData_PMF(hOPMF, pOBuff, lBlocks * 2);
							if (sDone)
								break;

							// --- Move last point over ---

							((short*) pIBuff)[0] = ((short*) pIBuff)[(lBlocks / 2) - 2];
							((short*) pIBuff)[1] = ((short*) pIBuff)[(lBlocks / 2) - 1];

							// --- Read more data ---

							lBlocks = lReadOld_PMF(hIPMF, pIBuff + 4, sIBuff - 4, &sDone);
							if (lBlocks == -1)
							{
								lPP_ERROR = PMF_PP_ERROR;
								goto END_PP_ERROR;
							}
							if (lBlocks > 0)
								lBlocks += 4;

						} while (lBlocks > 0);

					}
					else
					{

						// --- Generate a Line PMF ---

						Pmf.sNumber = 20;
						Pmf.lDataSize = 20;
						Pmf.lOtherData = -1;

						// --- Translate the Data ---

						IConvPP(pConv, lBlocks / 4, 2, pIBuff, ((VOL_BYT*) pBuff) + 4);

						// --- Write out the PMF Header ---

						lPP_ERROR = lWrite_PMF(hOPMF, &Pmf);
						if (lPP_ERROR)
							goto END_PP_ERROR;
					}
				}
			}
			else
			{

				// --- Write out the Old Header ---

				if ((lPP_ERROR = lWriteOld_PMF(hOPMF, 3)) != 0)
					goto END_PP_ERROR;


				// --- Copy the Data while shifting it ---

				do
				{
					lBlocks = lReadOld_PMF(hIPMF, pIBuff, sIBuff, &sDone);
					if (lBlocks > 0)
					{
						IConvPP(pConv, lBlocks / 4, 1, pIBuff, pOBuff);
						sWriteData_PMF(hOPMF, pOBuff, lBlocks);
					}
				} while (lBlocks > 0);
			}


		}
		else if ((Pmf.sNumber >= 1000) && (Pmf.sNumber <= 1255))
		{

			// --- Old PMF Pen Changes ---

			if (pConv->sOldToNew)
			{
				// --- Update the Attribute, entity not coppied ---

				if (Pmf.sNumber - 1000 < pConv->sMaxAttribs)
					sOldAttrib = pConv->psAttribs[Pmf.sNumber - 1000];
				else
					sOldAttrib = 0;

			}
			else
			{

				// --- Shift attribute header ---

				if (Pmf.sNumber - 1000 < pConv->sMaxAttribs)
					Pmf.sNumber = (short) (pConv->psAttribs[(short) (Pmf.sNumber - 1000)] + 1000);
				else
					Pmf.sNumber = 1000;

				// --- Write out the PMF ---

				if ((lPP_ERROR = lWriteOld_PMF(hOPMF, Pmf.sNumber)) != 0)
					goto END_PP_ERROR;
			}

		}
		else if ((Pmf.sNumber == 20) || (Pmf.sNumber == 21) || (Pmf.sNumber == 22) || (Pmf.sNumber == 26) || (Pmf.sNumber == 27) || (Pmf.sNumber == 28) || (Pmf.sNumber == 29))
		{

			// --- Modify the Attribute Overide ---

			if (Pmf.sAttrib < 0)
			{
				// --- Set the Attribute to overide ---

				if (-Pmf.sAttrib < pConv->sMaxAttribs)
					Pmf.sAttrib = -pConv->psAttribs[-Pmf.sAttrib];
				else
					Pmf.sAttrib = 0;


				// --- Read the Attribute Overide ---

				if ((lPP_ERROR = lAttrPmf_PP((CON_BYT*) Pmf.pOveride, Pmf.lOveride, &Attrib, &lOveride)) != 0)
					goto END_PP_ERROR;


				// --- Update the Attribute Overide for Text Styles ---

				if (lOveride & (ATR_TSTY | ATR_LTHK | ATR_LPTC))
				{
					Attrib.Tsty.ht = RoundLong_GS(Attrib.Tsty.ht * pConv->dDeltaScale);
					Attrib.Tsty.wd = RoundLong_GS(Attrib.Tsty.wd * pConv->dDeltaScale);
					Attrib.Tsty.sp = RoundLong_GS(Attrib.Tsty.sp * pConv->dDeltaScale);
					Attrib.Lthk = RoundLong_GS(Attrib.Lthk * pConv->dDeltaScale);
					Attrib.Lptc = RoundLong_GS(Attrib.Lptc * pConv->dDeltaScale);
					Pmf.lOveride = lMakeAttrPmf_PP(&Attrib, (VOL_BYT*) Pmf.pOveride, lOveride);
				}

			}
			else
			{

				if (Pmf.sAttrib < pConv->sMaxAttribs)
					Pmf.sAttrib = pConv->psAttribs[Pmf.sAttrib];
				else
					Pmf.sAttrib = 0;
			}


			// --- Change the Attribute Data ---

			((short*) pIBuff)[1] = Pmf.sAttrib;

			// --- Modify the PMF's based on the PMF type ---

			switch (Pmf.sNumber)
			{
				case 20:           // --- Line ---

					IConvPP(pConv, 2, 0, (VOL_BYT*) &((PMF_20*) pIBuff)->rX1, (VOL_BYT*) &((PMF_20*) pIBuff)->rX1);
					Pmf.lOtherData = -1;
					break;

				case 26:           // --- Text ---

					// --- Do we need to rotate ? ---

					if (((PMF_26*) pIBuff)->sJust <= PMF_PP_ALL_CENTERED)
						((PMF_26*) pIBuff)->rX2 += (float) pConv->fDeltaAngle;

					// --- Translate the Points ---

					if (((PMF_26*) pIBuff)->sJust < PMF_PP_BASE_LEFT)
						IConvPP(pConv, 1, 0, (VOL_BYT*) &((PMF_26*) pIBuff)->rX1, (VOL_BYT*) &((PMF_26*) pIBuff)->rX1);
					else
						IConvPP(pConv, 2, 0, (VOL_BYT*) &((PMF_26*) pIBuff)->rX1, (VOL_BYT*) &((PMF_26*) pIBuff)->rX1);
					break;

				case 27:           // --- Circle ---

					IConvPP(pConv, 1, 0, (VOL_BYT*) &((PMF_27*) pIBuff)->rX, (VOL_BYT*) &((PMF_27*) pIBuff)->rX);
					((PMF_27*) pIBuff)->rRadius *= (float) pConv->dDeltaScale;
					((PMF_27*) pIBuff)->rSAngle += (float) pConv->fDeltaAngle;
					((PMF_27*) pIBuff)->rEAngle += (float) pConv->fDeltaAngle;
					Pmf.lOtherData = -1;
					break;

				case 28:           // --- Symbol ---

					IConvPP(pConv, 1, 0, (VOL_BYT*) &((PMF_28*) pIBuff)->rX, (VOL_BYT*) &((PMF_28*) pIBuff)->rX);
					((PMF_28*) pIBuff)->rScale *= (float) pConv->dDeltaScale;
					((PMF_28*) pIBuff)->rRotate += (float) pConv->fDeltaAngle;
					Pmf.lOtherData = -1;
					break;

				case 29:           // --- Poly Symbol ---

					((PMF_28*) pIBuff)->rScale *= (float) pConv->dDeltaScale;
					break;
			}


			// --- Output the Header PMF ---

			if ((lPP_ERROR = lWrite_PMF(hOPMF, &Pmf)) != 0)
				goto END_PP_ERROR;


			// --- Write out Data PMF ---

			if ((Pmf.sNumber == 21) || (Pmf.sNumber == 22) || (Pmf.sNumber == 29) || (Pmf.sNumber == 26))
			{
				// --- Is the a text entity ? ---

				if (Pmf.sNumber == 26)
				{
					// --- Read the PMF data into Buffer ---

					if ((lBlocks = lReadData_PMF(hIPMF, pIBuff, Pmf.lOtherData, Pmf.lOtherData)) == -1)
					{
						lPP_ERROR = PMF_PP_ERROR;
						goto END_PP_ERROR;
					}

					// --- Write Data Out ---

					if (sWriteData_PMF(hOPMF, pIBuff, lBlocks))
					{
						lPP_ERROR = PMF_PP_ERROR;
						goto END_PP_ERROR;
					}

				}
				else
				{

					// --- Process all the Data ---

					while (Pmf.lOtherData > 0L)
					{
						lBlocks = lReadData_PMF(hIPMF, pIBuff, sIBuff, Pmf.lOtherData);
						if (lBlocks == -1)
						{
							lPP_ERROR = PMF_PP_ERROR;
							goto END_PP_ERROR;
						}
						if (lBlocks > 0)
						{
							IConvPP(pConv, lBlocks / 8, 0, pIBuff, pOBuff);
							if (sWriteData_PMF(hOPMF, pOBuff, lBlocks))
							{
								lPP_ERROR = PMF_PP_ERROR;
								goto END_PP_ERROR;
							}
						}
						Pmf.lOtherData -= lBlocks;
					}
				}
			}
		}
	} while (!lPP_ERROR);


	// --- Done ---

END_PP_ERROR:
	return (lPP_ERROR);

}



//-----------------------------------------------------------------------------

short sMergeAttribs_PP(H_VV*    hVV,            // VV holding Attributes
                       H_VV*    hIVV,           // Input VV holding attributes
                       short    sFile,          // File number to use for -a
                       Conv_PP* pConv,          // Conversion
                       short    sRename)        // Merge Attributs (0 - no, 1 -yes)
{
	DEBUG_STACK;

	Att_PP* pA, * pB;
	short   i, t, u;
	short   sIAtts;
	short   sAtts;

	VOL_STR szBuff[50];

	VOL_STR sz2[30];

	H_VVV*  hVVV = NULL;
	H_VVV*  hIVVV = NULL;


	// --- Validate Paramters ---

	ASSERT(hVV && hIVV && pConv);        // Real
	sAtts = (short) Len_VV(hVV);
	sIAtts = (short) Len_VV(hIVV);
	SetLn_VV(hVV, sAtts + sIAtts - 1);
	pConv->psAttribs[0] = 0;


	// --- Create VVV's ---

	hVVV = hCreat_VVV(hVV);
	if (hVVV == NULL)
		return (1);

	hIVVV = hCreat_VVV(hIVV);
	if (hIVVV == NULL)
	{
		Destr_VVV(hVVV);
		return (1);
	}


	// --- Go through the new attribute list ---

	sStartNext_VVV(hIVVV, 1, 0);
	for (i = 1; i < sIAtts; i++)
	{
		// ---- Get the Attribute ---

		pA = (Att_PP*) pNext_VVV(hIVVV);


		// --- Scale the Attribute ---

		pA->Tsty.ht = RoundLong_GS(pA->Tsty.ht * pConv->dDeltaScale);
		pA->Tsty.wd = RoundLong_GS(pA->Tsty.wd * pConv->dDeltaScale);
		pA->Tsty.sp = RoundLong_GS(pA->Tsty.sp * pConv->dDeltaScale);
		pA->Lthk = RoundLong_GS(pA->Lthk * pConv->dDeltaScale);
		pA->Lptc = RoundLong_GS(pA->Lptc * pConv->dDeltaScale);


		// --- Scan through the Attribute list for the name ---

		if (!sRename)
		{
			u = 0;
			sStartNext_VVV(hVVV, 1, 0);
			for (t = 1; t < sAtts; t++)
			{
				pB = (Att_PP*) pNext_VVV(hVVV);
				if (!_stricmp(pB->Name, pA->Name))
				{
					u = 2;
					break;
				}
			}
			
		}

		// --- Did we find it ? ---

		if (!u || sRename)
		{
			// --- Do we have to change the Name ? ---

			if (sRename)
			{
				strcpy_GS(szBuff, pA->Name, sizeof (szBuff));
				strcat_GS(szBuff, "_", sizeof (szBuff));
				sFormatStr_GS(sz2, sizeof (sz2), &sFile, GS_SHORT, GSF_NORMAL, 0);
				strcat_GS(szBuff, sz2, sizeof (szBuff));
				memcpy(pA->Name, szBuff, 15);
				pA->Name[15] = '\0';
			}

			// --- Add to the end ---

			*(Att_PP*) pPoint_VVV(hVVV, sAtts) = *pA;
			pConv->psAttribs[i] = sAtts;
			sAtts++;

		}
		else
		{

			// --- Just Remember the ID ---

			pConv->psAttribs[i] = t;
		}
	}
	


	// --- Set the VV length ---

	SetLn_VV(hVV, sAtts);


	// --- Destroy the VVV's ---

	Destr_VVV(hVVV);
	Destr_VVV(hIVVV);
	return (0);

}




//-----------------------------------------------------------------------------

short sRenderExt_PP(H_PP*     hPP,      // PP Object to use
                    CON_STR*  szFile,   // File Name of PMF
                    Point_DD* plMin,    // Min Region
                    Point_DD* plMax)    // Max Region
{
	DEBUG_STACK;

	H_PMF*     hPMF = NULL;
	Entity_PMF Pmf;
	Att_PP     Attr;
	long       lOveride;
	long       lPP_ERROR = NO_ERROR;
	Header_PMF Header;



	// --- Validate Paramters ---

	ASSERT(hPP && szFile && plMin && plMax);     // Real
	ASSERT(hPP->obj_id == PMF_PP_ID);            // Was it really a PP object ?

	// --- Go to a standard PP ---

	Unit_PP(hPP);


	// --- Load the Attribute File ---

	lPP_ERROR = lLoadAtt_PP(hPP->hAP, hPP->hVVAttribs, &hPP->sNAttribs, szFile);
	if (lPP_ERROR != NO_ERROR)
	{
		// --- File Corrupt PMF_PP_ERROR ---

		RegistErr_AP(hPP->hAP, 20902, modn);
		SetParmL_AP(hPP->hAP, "%1", lPP_ERROR);
		return (1);
	}


	// --- Open the Plot File ---

	hPMF = hCreat_PMF(hPP->hAP, szFile, 0, BF_READ);
	if (hPMF == NULL)
		return (1);
	if (lHeader_PMF(hPMF, &Header))
	{
		Destr_PMF(hPMF, BF_KEEP);
		return (1);
	}
	Seek_PMF(hPMF, 0, BF_SEEK_START);
	hPP->PMFOrigin = Header.lOrigin;

	// --- Set the PP rendering Extents ---

	hPP->lMin.x = GS_S4MX;
	hPP->lMin.y = GS_S4MX;
	hPP->lMax.x = GS_S4MN;
	hPP->lMax.y = GS_S4MN;
	hPP->sDoExtents = 1;


	// --- Go through all the PMF's ---

	for (;;)
	{

		// --- Read the PMF ---

		if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
			goto END_EXT;
		if ((Pmf.sNumber == 999) || (Pmf.sNumber == 9999))
			break;


		// --- Flip the Attribute ---

		if (Pmf.sGraphicsEntity)
		{
			if (Pmf.sAttrib > 0)
			{
				// --- This Entity is dependent on the attribute ---

				SetAtt_PP(hPP, Pmf.sAttrib);

			}
			else
			{

				// --- Interpret the Attribute Information ---

				GetAttT_PP(hPP, (short) -Pmf.sAttrib, &Attr);
				if ((lPP_ERROR = lAttrPmf_PP((CON_BYT*) Pmf.pOveride, Pmf.lOveride, &Attr, &lOveride)) != 0)
					goto END_EXT;
				SetCurAtt_PP(hPP, &Attr);
			}
		}


		// --- Render the PMF ---

		if ((lPP_ERROR = lDoPmf_PP(hPP, hPMF, &Pmf)) != 0)
			goto END_EXT;

	}


	// --- Get the Extents ---

END_EXT:
	hPP->sDoExtents = 0;         // Turn it off
	plMin->x = hPP->lMin.x - 500;
	plMin->y = hPP->lMin.y - 500;
	plMax->x = hPP->lMax.x + 500;
	plMax->y = hPP->lMax.y + 500;

	// --- Destroy the PMF ---

	if (hPMF)
		Destr_PMF(hPMF, BF_KEEP);

	// --- Generate PMF_PP_ERRORs ---

	if (lPP_ERROR != NO_ERROR)
	{
		RegistErr_AP(hPP->hAP, 20901, modn);
		SetParmS_AP(hPP->hAP, "%1", szFile);
		SetParmL_AP(hPP->hAP, "%2", lPP_ERROR);
		return (1);
	}

	// --- Done ---

	return (0);

}
