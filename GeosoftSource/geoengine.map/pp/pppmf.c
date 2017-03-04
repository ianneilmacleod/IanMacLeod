//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
// PPPMF.c
//
// Geosoft high-level graphics class.
//
// This part of the class performs renderering of a PMF file.
//
//=============================================================================



#include <math.h>

#include <sys.h>                // SYS Header
#include <geosoft.h>            // Geosoft

#include <ap.h>                 // AP Class
#include <mp.h>                 // MP Class
#include <col.h>                // COL Utilities
#include <gs.h>                 // GS Header
#include <pp.h>                 // PP Header
#include <vv.h>                 // VV Header
#include <gs.h>                 // GS Header
#include <pmf.h>                // PMF Header
#include "pp.ih"                // PP Internal Header

#define PMF_PP_ERROR  (100000L + (long) __LINE__)

//-----------------------------------------------------------------------------

long lDoPmf_PP(H_PP*       hPP,         // PP Object
               H_PMF*      hPMF,        // PMF Object
               Entity_PMF* pPmf)        // PMF Entity to Render
{
	DEBUG_STACK;

	VOL_DAT* pTBuff;
	long     lBlocks;
	Att_PP   Attr;
	long     lOveride;
	long     lPP_ERROR = NO_ERROR;

	short    sDone;
	short    sFirst;

	Point_PP p;

	TLoc_PP  TLoc;
	short    sJust;
	double   dScale;
	double   dRotate;



	// --- Validate Parameters ---

	ASSERT(hPP && hPMF && pPmf);         // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Old Style PMF ? ---

	if ((pPmf->sNumber == 3) || (pPmf->sNumber == 2))
	{
		sDone = 0;
		if (pPmf->sNumber == 3)
			sFirst = 0;
		else
			sFirst = 1;
		while (!sDone)
		{
			lBlocks = lReadOld_PMF(hPMF, hPP->pDataBuff, PMF_PP_READ_BUFFER, &sDone);
			if (lBlocks == -1)
				return (PMF_PP_ERROR);
			if (lBlocks > 0)
				IPVect_PP(hPP, hPP->pDataBuff, lBlocks / 4, CONV_SHORT, sFirst);
			sFirst = 1;
		}
		return (NO_ERROR);

	}
	else if ((pPmf->sNumber >= 1000) && (pPmf->sNumber <= 1255))
	{

		SetAtt_PP(hPP, (short) (pPmf->sNumber - 1000));
		return (NO_ERROR);

	}

	// --- Set the Proper Attribute ---

	if (pPmf->sGraphicsEntity)
	{
		if (pPmf->sAttrib > 0)
			SetAtt_PP(hPP, pPmf->sAttrib);
		else
		{
			// --- Interpret the Attribute Information ---

			GetAttT_PP(hPP, (short) -pPmf->sAttrib, &Attr);
			if ((lPP_ERROR = lAttrPmf_PP((CON_BYT*) pPmf->pOveride, pPmf->lOveride, &Attr, &lOveride)) != 0)
				return (lPP_ERROR);
			SetCurAtt_PP(hPP, &Attr);
		}
	}


	// --- What Type of PMF is this ? ---

	switch (pPmf->sNumber)
	{
		case 20:                 // --- A Line ---

			// --- Move to the first point ---

			p.x = (double) ((PMF_20*) pPmf->pData)->rX1;
			p.y = (double) ((PMF_20*) pPmf->pData)->rY1;
			Move_PP(hPP, p);

			// --- Draw the Line to second point ---

			p.x = (double) ((PMF_20*) pPmf->pData)->rX2;
			p.y = (double) ((PMF_20*) pPmf->pData)->rY2;
			Vect_PP(hPP, p);
			break;


		case 21:                 // --- A Poly Line ---
		case 22:                 // --- A Polygon ---

			if (pPmf->lOtherData > 0L)
			{

				// --- Prepare the Buffer ---

				if (pPmf->lOtherData > PMF_PP_READ_BUFFER)
				{
					// --- Try to get a block large enough ---

					pTBuff = Alloc_MP(hPP->hMP, pPmf->lOtherData);
					if (pTBuff == NULL)
					{
						// --- Skip this object ---

						if (Seek_PMF(hPMF, pPmf->lOtherData, BF_SEEK_CURRENT) == -1)
							return (PMF_PP_ERROR);
						break;        // Skip this Entity
					}
					sFirst = 1;

				}
				else
				{

					pTBuff = hPP->pDataBuff;
					sFirst = 0;
				}


				// --- Read the PMF data into Buffer ---

				if ((lBlocks = lReadData_PMF(hPMF, pTBuff, pPmf->lOtherData, pPmf->lOtherData)) == -1)
					return (PMF_PP_ERROR);

				// --- Render Polygon/Polyline ---

				if (pPmf->sNumber == 22)
					VFill_PP(hPP, pTBuff, lBlocks / 8, CONV_FLOAT, 1);
				else
					IPVect_PP(hPP, pTBuff, lBlocks / 8, CONV_FLOAT, 0);

				// --- Free the Buffer we allocated ---

				if (sFirst)
					Free_MP(hPP->hMP, pTBuff);
			}
			break;


		case 26:                 // --- Text ---

			// --- Get Text Information ---

			sJust = ((PMF_26*) pPmf->pData)->sJust;
			TLoc.p1.x = (double) ((PMF_26*) pPmf->pData)->rX1;
			TLoc.p1.y = (double) ((PMF_26*) pPmf->pData)->rY1;
			TLoc.d.p2.x = (double) ((PMF_26*) pPmf->pData)->rX2;

			// --- modes -1 to 2 have angles stored only in X ---

			if (sJust >= 3)
				TLoc.d.p2.y = (double) ((PMF_26*) pPmf->pData)->rY2;
			else
				TLoc.d.p2.y = 0.0;

			// --- Read the PMF data into Buffer ---

			if (pPmf->lOtherData > 0L)
			{

				if ((lBlocks = lReadData_PMF(hPMF, hPP->pDataBuff, pPmf->lOtherData, pPmf->lOtherData)) == -1)
					return (PMF_PP_ERROR);

				// --- Generate the Text ---

				PText_PP(hPP, (CON_STR*) hPP->pDataBuff, TLoc, sJust);
			}
			break;

		case 27:                 // --- Circle ---

			// --- Generate the Circle ---

			p.x = (double) ((PMF_27*) pPmf->pData)->rX;
			p.y = (double) ((PMF_27*) pPmf->pData)->rY;
			Circle_PP(hPP, RoundLong_GS(((PMF_27*) pPmf->pData)->rRadius * 1000.0), (double) ((PMF_27*) pPmf->pData)->rSAngle, (double) ((PMF_27*) pPmf->pData)->rEAngle, ((PMF_27*) pPmf->pData)->sClose, p);
			break;


		case 28:                                          // --- Symbol ---

			// --- Generate the Symbol ---

			p.x = (double) ((PMF_28*) pPmf->pData)->rX;    // To double
			p.y = (double) ((PMF_28*) pPmf->pData)->rY;    // To double

			Symb_PP(hPP, ((PMF_28*) pPmf->pData)->sSymbol, (double) ((PMF_28*) pPmf->pData)->rScale, (double) ((PMF_28*) pPmf->pData)->rRotate, p);
			break;


		case 29:                 // --- Poly Symbol ---

			// --- Remember the information ---

			dScale = (double) ((PMF_28*) pPmf->pData)->rScale;


			dRotate = (double) ((PMF_28*) pPmf->pData)->rRotate;
			sJust = ((PMF_28*) pPmf->pData)->sSymbol;

			// --- Process Blocks ---

			while (pPmf->lOtherData > 0L)
			{
				lBlocks = lReadData_PMF(hPMF, hPP->pDataBuff, PMF_PP_READ_BUFFER, pPmf->lOtherData);
				if (lBlocks == -1)
					return (PMF_PP_ERROR);
				if (lBlocks > 0)
				{
					if (dScale > 0.0)  // ignore symbols of size <= 0.0
						IPSymb_PP(hPP, sJust, lBlocks / 8, dScale, dRotate, hPP->pDataBuff, CONV_FLOAT);
				}
				pPmf->lOtherData -= lBlocks;
			}

			break;


		default:                 // --- Unknown PMF type, pass it on to DD ---

		{
			struct _PMF
			{
				short sFlag;
				short sNumber;
				long lLen;
				CON_DAT* pData;
			} _Pmf;

			_Pmf.sFlag = -32767;
			_Pmf.sNumber = pPmf->sNumber;
			_Pmf.lLen = pPmf->lDataSize / 4;
			_Pmf.pData = (CON_DAT*) pPmf->pData;

			if (sPmf_DD(hPP->hDD, &_Pmf))
				return (PMF_PP_ERROR);

		}

			break;

	}
	return (NO_ERROR);

}


//-----------------------------------------------------------------------------

long lPmf_PP(H_PP*    hPP,      // PP Object
             CON_STR* szName,   // PLT file to render
             long     lOrigin)  // Origin of plot information
{
	DEBUG_STACK;


	register short sCount = 0;
	long           lPP_ERROR = NO_ERROR;
	Entity_PMF     Pmf;
	H_PMF*         hPMF;


	// --- Validate Parameters ---

	ASSERT(hPP && szName);               // Must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Load the Attribute Table ---

	lPP_ERROR = lLoadAtt_PP(hPP->hAP, hPP->hVVAttribs, &hPP->sNAttribs, szName);


	// --- Open the PLOT file ---

	if ((hPMF = hCreat_PMF(hPP->hAP, szName, 0, BF_READ)) == NULL)
		return (PMF_PP_ERROR);


	// --- get the short coordinate origin ---

	if (lOrigin != GS_S4DM)
		hPP->PMFOrigin = lOrigin;

	else
	{


		// --- Read the Header ---

		Header_PMF Header;
		lPP_ERROR = lHeader_PMF(hPMF, &Header);
		if (lPP_ERROR == NO_ERROR)
			hPP->PMFOrigin = Header.lOrigin;
		Seek_PMF(hPMF, 0, BF_SEEK_START);


		// --- Save the Extents ---

		if (Header.sFlags & LOADED_SIZE)
		{


			// --- make sure valid plot area ---

			if ((Header.MinRegion.x > Header.MaxRegion.x) || (Header.MinRegion.y > Header.MaxRegion.y))
			{

				RegistErr_AP(hPP->hAP, 20912, modn);
				SetParmS_AP(hPP->hAP, "%1", szName);
				lPP_ERROR = PMF_PP_ERROR;
				goto GS_EXIT;

			}


			hPP->clipll = Header.MinRegion;
			hPP->clipur = Header.MaxRegion;

		}

	}


	// --- Render all PMF entities ---

	while (lPP_ERROR == NO_ERROR)
	{
		// --- Read the PMF Header ---

		if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
			break;

		// --- Are we done ? ---

		if ((Pmf.sNumber == 9999) || (Pmf.sNumber == 999))
			break;


		// --- Render the Object ---

		if ((lPP_ERROR = lDoPmf_PP(hPP, hPMF, &Pmf)) != 0)
			break;


		// --- Do we need to stop ? ---

		if (sCount++ == 25)
		{
			if (CheckStop_AP(hPP->hAP))
				break;
			sCount = 0;
		}
	}


GS_EXIT:


	// --- Close the Plot File ---

	Destr_PMF(hPMF, BF_KEEP);


	// --- Return PMF_PP_ERROR Code ---

	if (lPP_ERROR != NO_ERROR)
	{
		// --- File Corrupt PMF_PP_ERROR ---

		RegistErr_AP(hPP->hAP, 20909, modn);
		SetParmL_AP(hPP->hAP, "%1", lPP_ERROR);
		SetParmS_AP(hPP->hAP, "%2", szName);
	}
	return (lPP_ERROR);

}



//-----------------------------------------------------------------------------

long lAttrPmf_PP(CON_BYT* pBuff,        // Buffer holding attribute overrides
                 long     lLength,      // Number of bytes in override
                 Att_PP*  attrib,       // Where to put the override info
                 long*    lOveride)     // Attributes that were overwritten
{
	DEBUG_STACK;

	CON_BYT*     pCur;
	ATTRIB_DATA* pAttr;
	short        sFlag;
	VOL_STR*     cp;


	// --- Validate Parameters ----

	ASSERT(attrib && pBuff);     // Must be Real


	// --- Generate Overides ---

	pCur = pBuff;
	*lOveride = 0;

	while (lLength > 2)
	{
		pAttr = (ATTRIB_DATA*) pCur;
		sFlag = pAttr->sFlag;
		switch (sFlag)
		{
			case 1:               // --- Line Color ---

				if (lLength < 6)
					return (PMF_PP_ERROR);
				InitColor(&attrib->Lcol, C_KCMY, pAttr->Data.Bytes[1], pAttr->Data.Bytes[2], pAttr->Data.Bytes[3], pAttr->Data.Bytes[0]);
				pCur += 6;
				lLength -= 6;
				*lOveride |= ATR_LCOL;
				break;

			case 2:               // --- Line Thickness ---

				if (lLength < 4)
					return (PMF_PP_ERROR);
				attrib->Lthk = GS_MAX(RoundLong_GS(pAttr->Data.Int), 1);
				pCur += 4;
				lLength -= 4;
				*lOveride |= ATR_LTHK;
				break;


			case 3:               // --- Line Pattern ---

				if (lLength < 4)
					return (PMF_PP_ERROR);
				attrib->Lpat = pAttr->Data.Int;
				pCur += 4;
				lLength -= 4;
				*lOveride |= ATR_LPAT;
				break;

			case 4:                                                                // --- Line Pitch ---

				if (lLength < 6)
					return (PMF_PP_ERROR);
				attrib->Lptc = GS_MAX(RoundLong_GS(pAttr->Data.Real * 1000.0), 1);  // to microns
				pCur += 6;
				lLength -= 6;
				*lOveride |= ATR_LPTC;
				break;


			case 5:               // --- Fill Color ---

				if (lLength < 6)
					return (PMF_PP_ERROR);
				InitColor(&attrib->Fcol, C_KCMY, pAttr->Data.Bytes[1], pAttr->Data.Bytes[2], pAttr->Data.Bytes[3], pAttr->Data.Bytes[0]);
				pCur += 6;
				lLength -= 6;
				*lOveride |= ATR_FCOL;
				break;

			case 6:                                                                        // --- Text Style ---

				if (lLength < 18)
					return (PMF_PP_ERROR);
				attrib->Tsty.ht = GS_MAX(RoundLong_GS(pAttr->Data.Reals[0] * 1000.0), 1);   // Microns
				attrib->Tsty.wd = GS_MAX(RoundLong_GS(pAttr->Data.Reals[1] * 1000.0), 1);   // Microns
				attrib->Tsty.sp = GS_MAX(RoundLong_GS(pAttr->Data.Reals[2] * 1000.0), 1);   // Microns
				attrib->Tsty.sl = (double) pAttr->Data.Reals[3];
				if (fabs(attrib->Tsty.sl) > 89.0)
					attrib->Tsty.sl = 0.0;
				pCur += 18;
				lLength -= 18;
				*lOveride |= ATR_TSTY;
				break;

			case 7:               // --- Font ---

				if (lLength < 10)
					return (PMF_PP_ERROR);
				memcpy(attrib->Font, pAttr->Data.Font, 8);
				attrib->Font[8] = '\0';
				if ((cp = strchr_GS(attrib->Font, ' ')) != NULL)
					*cp = '\0';    // Remove Spaces
				pCur += 10;
				lLength -= 10;
				*lOveride |= ATR_FONT;
				break;

			default:              // --- Unknown Attributes ---

				return (NO_ERROR);
				break;
		}
	}

	// --- No PMF_PP_ERROR ---

	return (NO_ERROR);

}


//-----------------------------------------------------------------------------

long lLoadAtt_PP(H_AP*    hAP,          // AP Object
                 H_VV*    hVV,          // VV Object
                 short*   psNAttribs,   // Number of Attributes
                 CON_STR* szName)       // Name of Plot File
{
	DEBUG_STACK;

	PMF_ATTRIB* pA;
	short       sCount = 0;
	long        lBSize;
	long        lPP_ERROR = NO_ERROR;
	short       i;
	VOL_BYT*    pSBuff = NULL;
	VOL_STR*    cp;
	Entity_PMF  Pmf;
	Att_PP      Att;
	VOL_STR     pBuff[255];
	H_PMF*      hPMF = NULL;



	// --- Validate Parameters ---

	ASSERT(hAP && hVV && psNAttribs && szName);  // Must be real


	// --- Open the Attribute File ---

	hPMF = hCreat_PMF(hAP, szName, 1, BF_READ);


	// --- if can't create attribute, clear errors and skip it ---

	if (hPMF == NULL)
	{
		ClearErr_AP(hAP);
		*psNAttribs = 0;
		return (NO_ERROR);
	}


	// --- Begin the Main Loop ---

	*psNAttribs = 1;
	pA = (PMF_ATTRIB*) pBuff;
	while (lPP_ERROR == NO_ERROR)
	{
		// --- Read the PMF and check for end of file ---

		lPP_ERROR = lRead_PMF(hPMF, &Pmf, pBuff, sizeof (pBuff));
		if (lPP_ERROR || (Pmf.sNumber == 999) || (Pmf.sNumber == 9999))
			break;


		// --- Update Count ---

		sCount++;
		if ((sCount % 10 == 0) && CheckStop_AP(hAP))
		{
			lPP_ERROR = NO_ERROR;
			break;
		}


		// --- Does this PMF have non-variable data ? ---

		if (Pmf.sNumber == 3000)
		{
			// --- Determine the block size ---

			*psNAttribs = ((PMF_3000*) Pmf.pData)->sNumbAttribs;

			// --- Set length of VV ---

			SetLn_VV(hVV, *psNAttribs + 1);


			// --- Initialize all Attributes ---

			InitColor(&Att.Lcol, C_KCMY, 0, 0, 0, 0);
			InitColor(&Att.Fcol, C_KCMY, 0, 0, 0, 0);
			Att.Lthk = 0;
			Att.Lpat = 0;
			Att.Lptc = 1;
			Att.Tsty.ht = 1;
			Att.Tsty.wd = 1;
			Att.Tsty.sp = 1;
			Att.Tsty.sl = 0.0;
			memcpy(Att.Font, "default", 8);
			strcpy_GS(Att.Name, "default", sizeof (Att.Name));
			for (i = 0; i < *psNAttribs + 1; i++)
				SetE_VV(hVV, i, &Att);


			// --- Read Attributes ---

			if (*psNAttribs > 0)
			{
				lBSize = Pmf.lOtherData / *psNAttribs;
				(*psNAttribs)++;

				// --- Process Blocks ---

				for (i = 1; i < *psNAttribs; i++)
				{
					// --- Read data into buffer ---

					if (lReadData_PMF(hPMF, pBuff, sizeof (pBuff), lBSize) != lBSize)
					{
						lPP_ERROR = PMF_PP_ERROR;
						break;
					}


					// --- Setup the attributes ---

					InitColor(&Att.Lcol, C_KCMY, pA->bColors[1], pA->bColors[2], pA->bColors[3], pA->bColors[0]);
					InitColor(&Att.Fcol, C_KCMY, pA->bFColors[1], pA->bFColors[2], pA->bFColors[3], pA->bFColors[0]);
					Att.Lthk = (long) pA->sThick;
					Att.Lpat = pA->sLinePat;
					Att.Lptc = GS_MAX(RoundLong_GS(pA->rPitch * 1000.0), 1);                 // To Microns
					Att.Tsty.ht = GS_MAX(RoundLong_GS(pA->rTHeight * 1000.0), 1);            // To Microns
					Att.Tsty.wd = GS_MAX(RoundLong_GS(pA->rTWidth * 1000.0), 1);             // To Microns
					Att.Tsty.sp = GS_MAX(RoundLong_GS(pA->rTSpace * 1000.0), 1);             // To Microns

					Att.Tsty.sl = (double) pA->rTSlant;
					if (fabs(Att.Tsty.sl) > 89.0)
						Att.Tsty.sl = 0.0;

					memcpy(Att.Font, pA->szFont, 8);
					Att.Font[8] = '\0';
					if ((cp = strchr_GS(Att.Font, ' ')) != NULL)
						*cp = '\0'; // Remove Spaces
					sFormatStr_GS(Att.Name, sizeof (Att.Name), &i, GS_SHORT, GSF_NORMAL, 0);

					// --- Write out the attribute ---

					SetE_VV(hVV, i, &Att);
				}
			}

		}
		else if (Pmf.sNumber == 3002)
		{

			if (Pmf.lOtherData > 0L)
			{

				// --- Read Data into Stack Buffer ---

				pSBuff = (VOL_BYT*) Alloc_MP(hGethMP_AP(hAP), Pmf.lOtherData);
				if (pSBuff == NULL)
				{
					lPP_ERROR = PMF_PP_ERROR;
					break;
				}
				if (lReadData_PMF(hPMF, pSBuff, Pmf.lOtherData, Pmf.lOtherData) != Pmf.lOtherData)
				{
					lPP_ERROR = PMF_PP_ERROR;
					break;
				}

				// --- Process Names ---

				i = 1;
				cp = (VOL_STR*) pSBuff;

				while (strlen(cp) != 0)
				{
					// --- Copy the Name ---

					GetE_VV(hVV, i, &Att);
					strcpy_GS(Att.Name, cp, sizeof (Att.Name));
					Att.Name[15] = '\0';
					SetE_VV(hVV, i, &Att);

					cp += strlen(cp) + 1;
					i++;
					if (i >= *psNAttribs)
						break;
				}

				// --- Free Memory ---

				Free_MP(hGethMP_AP(hAP), pSBuff);
				pSBuff = NULL;
			}

		}
		else
		{

			// --- Skip Any Other PMFS ---

			if (Pmf.lOtherData > 0)
				if (Seek_PMF(hPMF, Pmf.lOtherData, BF_SEEK_CURRENT) == -1)
				{
					lPP_ERROR = PMF_PP_ERROR;
					break;
				}
		}
	}

	// --- Close the Attribute File ---

	if (hPMF != NULL)
		Destr_PMF(hPMF, BF_KEEP);
	if (pSBuff != NULL)
		Free_MP(hGethMP_AP(hAP), pSBuff);

	// --- Done ---

	if (lPP_ERROR != NO_ERROR)
	{
		RegistErr_AP(hAP, 20902, modn);
		SetParmL_AP(hAP, "%1", lPP_ERROR);
	}

	return (lPP_ERROR);

}

#undef PMF_PP_ERROR
