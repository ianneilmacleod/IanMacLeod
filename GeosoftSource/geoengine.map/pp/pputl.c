//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
// PPUTL.c
//
// Geosoft high-level graphics class.
//
// Utitities for PMF control.
//
//=============================================================================




#include <ap.h>                 // AP Class
#include <mp.h>                 // MP Class
#include <col.h>                // COL Utilities
#include <pp.h>                 // PP Header
#include <zon.h>                // ZON HEeader
#include <gs.h>                 // GS Header
#include <pp.h>                 // PP Header
#include <vv.h>                 // VV Header
#include <vvv.h>                // VVV Header
#include <bf.h>                 // RPF Header
#include <geosoft.h>            // GEOSOFT Header
#include <ma.h>                 // MA Header
#include "pp.ih"                // PP Internal Header

#define PMF_PP_ERROR (120000L + __LINE__)



//*****************************************************************************
//                      SPECIAL PMF UTILITIES
//*****************************************************************************

#define SCALEBAR_DEF       0.5                  // Anotated Text Default Size (% bar)
#define SCALEBAR_MIN       1.5                  // Miminum size

#define SCALEBAR_TDEF      1.5                  // Title Text Default Size (% bar)
#define SCALEBAR_TMIN      2.0                  // Title Text Default Size (% bar)

#define SCALEBAR_SDEF      0.75                 // Title Text Default Size (% bar)
#define SCALEBAR_SMIN      1.5                  // Minimum Size

//-----------------------------------------------------------------------------

long lScaleBar_PP(H_PP*    hPP,                 // PP Object
                  CON_STR* szTitle,             // Title of the Scale Bar
                  CON_STR* szSubTitle,          // SubTitle of the Scale Bar
                  short    sDecimals,           // Decimals in the Anotations
                  float    fHeight,             // Height of the Anotation Text (mm's)
                  float    fBoxHeight,          // Height of the Box
                  float    fBoxWidth,           // Width of the Box in mm's
                  Point_PP dPoint,              // Lower left hand of the box
                  CON_STR* szZone,              // Zone File
                  short    sAttrib,             // Attribute to use
                  H_VV*    hVV,                 // VV where the ID's for the created objects
                  long*    plSize)              // Number of objects created
{
	DEBUG_STACK;

	H_ZON*  hZON = NULL;
	long    lPP_ERROR = NO_ERROR;

	short   sZones;
	short   sZoneColor;
	zone_ob Zone;
	double  dPrevLevel = 0.0;

	SYS_ALIGNED_STACK_BUFFER(pBuff, 70);

	VOL_STR szText[255];
	float*  fBuff = (float*) pBuff;
	Att_PP  Overide;

	short   i, t;
	long    lPmfs = 0;
	long    lId;
	short   sType;
	float   fSize, fTextY;


	// --- Validate Parameters ---

	ASSERT(hPP && hVV && plSize);        // Real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(sDecimals >= 0);              // Valid Decimals
	ASSERT(sAttrib > 0);                 // Valid Attribute

#ifdef DEBUG_CODE
	{
		I_VV Info;
		Info_VV(hVV, &Info);
		ASSERT(Info.Type == GS_LONG);
	}
#endif

	// --- Create a Zone Object ---

	hZON = hCreat_ZON(hPP->hAP, 256, C_KCMY);
	if (hZON == NULL)
	{
		lPP_ERROR = PMF_PP_ERROR;
		goto SCALE_END;
	}


	// --- Load the Zone File ---

	sType = 2;
	if (Load_ZON(hZON, szZone, &sType))
		goto SCALE_END;


	// --- Count number of zones in the file ---

	sZones = Numb_ZON(hZON);
	if (sZones == 0)
		goto SCALE_END;           // No Zones
	sZoneColor = GetColor_ZON(hZON);



	// --- Compute the size of the the color zone boxes ---

	fSize = (float) (fBoxHeight * (double) sZones);

	// --- Box Locations ---

	fBuff[0] = (float) dPoint.x;
	fBuff[1] = (float) dPoint.y;

	fBuff[2] = (float) dPoint.x;
	fBuff[3] = (float) dPoint.y + fBoxHeight;

	fBuff[4] = (float) dPoint.x + fBoxWidth;
	fBuff[5] = (float) dPoint.y + fBoxHeight;

	fBuff[6] = (float) dPoint.x + fBoxWidth;
	fBuff[7] = (float) dPoint.y;

	fBuff[8] = (float) dPoint.x;
	fBuff[9] = (float) dPoint.y;


	// --- Text Size ---

	Overide.Tsty.ht = Overide.Tsty.wd = Overide.Tsty.sp = RoundLong_GS(fHeight * 1000.0);
	Overide.Tsty.sl = 0.0;


	// --- Text Locations ---

	fBuff[10] = (float) (dPoint.x - fHeight);
	fBuff[11] = (float) dPoint.y - (fHeight / (float) 2.0);
	fBuff[12] = (float) 0.0;
	fBuff[13] = (float) 0.0;
	(*(short*) &fBuff[14]) = PMF_PP_RIGHT;
	fTextY = fBuff[11] - (fHeight * (float) 2.0);




	// --- Generate the Bar ---

	for (i = 0; i < sZones; i++)
	{
		// --- Update the Overide Color ---

		Zone = Get_ZON(hZON, i);
		Overide.Fcol.mix = Zone.color;
		Overide.Fcol.sType = (BYTE) sZoneColor;

		// --- Generate the Text ---

		sFormatStr_GS(szText, sizeof (szText), &dPrevLevel, GS_DOUBLE, GSF_NORMAL, sDecimals);
		dPrevLevel = Zone.level;


		// --- Make the Box PMF ---

		lPP_ERROR = lNewExtPmf_PP(hPP, &lId, 22, 0, NULL, 40, (VOL_BYT*) fBuff, 1, (short) -sAttrib, ATR_FCOL, &Overide);
		if (lPP_ERROR)
			goto SCALE_END;
		SetE_VV(hVV, lPmfs, &lId);
		lPmfs++;


		// --- Make the Text PMF ---

		if ((fBuff[11] - (fHeight / 2.0) > fTextY) && (i != 0))
		{
			lPP_ERROR = lNewExtPmf_PP(hPP, &lId, 26, 20, (VOL_BYT*) (fBuff + 10), strlen(szText) + 1, (CON_BYT*) szText, 1, (short) -sAttrib, ATR_TSTY, &Overide);
			if (lPP_ERROR)
				goto SCALE_END;
			SetE_VV(hVV, lPmfs, &lId);
			lPmfs++;
			fTextY = fBuff[11] + (fHeight / (float) 2.0);
		}


		// --- Move the Box Up ---

		for (t = 0; t < 6; t++)
			fBuff[(t * 2) + 1] += fBoxHeight;
	}


	// --- Title Size ---

	fHeight = (float) (fHeight * SCALEBAR_TDEF);
	if (fHeight < (float) SCALEBAR_TMIN)
		fHeight = (float) SCALEBAR_TMIN;
	Overide.Tsty.ht = Overide.Tsty.wd = Overide.Tsty.sp = RoundLong_GS(fHeight * 1000.0);

	fBuff[10] = (float) (dPoint.x);
	fBuff[11] = (float) (dPoint.y - (fHeight * 2.0));
	(*(short*) &fBuff[14]) = PMF_PP_CENTERED;


	// --- Generate the Title ---

	lPP_ERROR = lNewExtPmf_PP(hPP, &lId, 26, 20, (VOL_BYT*) (fBuff + 10), strlen(szTitle) + 1, (CON_BYT*) szTitle, 1, (short) -sAttrib, ATR_TSTY, &Overide);
	if (lPP_ERROR)
		goto SCALE_END;
	SetE_VV(hVV, lPmfs, &lId);
	lPmfs++;


	// --- SubTitle Size ---

	fBuff[11] -= (float) (fHeight * 1.5);
	fHeight = fHeight * (float) SCALEBAR_SDEF;
	if (fHeight < (float) SCALEBAR_SMIN)
		fHeight = (float) SCALEBAR_SMIN;
	Overide.Tsty.ht = Overide.Tsty.wd = Overide.Tsty.sp = RoundLong_GS(fHeight * 1000.0);


	// --- Generate the SubTitle ---

	lPP_ERROR = lNewExtPmf_PP(hPP, &lId, 26, 20, (VOL_BYT*) (fBuff + 10), strlen(szSubTitle) + 1, (CON_BYT*) szSubTitle, 1, (short) -sAttrib, ATR_TSTY, &Overide);
	if (lPP_ERROR)
		goto SCALE_END;
	SetE_VV(hVV, lPmfs, &lId);
	lPmfs++;


	// --- Done ---

SCALE_END:

	if (hZON)
		Destr_ZON(hZON);
	if (lPP_ERROR)
	{
		RegistErr_AP(hPP->hAP, 20910, modn);
		SetParmL_AP(hPP->hAP, "%1", lPP_ERROR);
	}


	// --- Return ---

	*plSize = lPmfs;
	return (lPP_ERROR);

}


//-----------------------------------------------------------------------------


//*****************************************************************************
//                      EXTENT/PMF MODIFICATION FUNCTIONS
//*****************************************************************************


long lModifyExtPmf_PP(H_PP*     hPP,       // PP Object
                      H_VV*     hVV,       // VV Objects holding data
                      long      lObjects,  // Number of Objects to Modify
                      short     sAttrMode, // Attribute Mode Operation
                      // 0 - No Modifications
                      // 1 - Add to Overides
                      // 2 - Replace Overides
                      // 3 - Change Attribute
                      short     sNewAttr,       // New Attribute       (MODE 2)
                      long      lOveride,       // Attribute Overide   (MODE 1,2,3)
                      Att_PP*   pAttrData,      // Attribute Data      (MODE 1,2,3)
                      Point_DD* lOldOrigin,     // Old Origin of Object
                      Point_DD* lNewOrigin,     // New Origin of Object
                      Point_PP* dScale)         // New Scale
{
	DEBUG_STACK;

	register short i;
	long           t;
	H_PMF*         hPMF;

	long           lPP_ERROR = NO_ERROR;
	long           lId;
	long           lNewId;
	long           lPmfOveride = 0L;

	short          sScaleText = 0;
	short          sDone;
	long           lBlocks;

	PMF_BUFF       OutBuff;
	unsigned long  lOffset;

	extent_ob      extent;
	extent_ob      pExt;
	Entity_PMF     Pmf;
	Att_PP         PmfAttr;

	Point_PP       dTextScale;
	double         dAngle, dPerc;
	double         dX, dY;


	// --- Validate Parameters ---

	ASSERT(hPP && hVV && lOldOrigin && lNewOrigin && dScale);    // Real
	ASSERT(hPP->obj_id == PMF_PP_ID);                            // Was it really a PP object ?
	ASSERT(lObjects > 0L);                                       // Must have objects to modify
	ASSERT((sAttrMode >= 0) || (sAttrMode <= 3));                // Valid Attribute Mode
#ifdef DEBUG_CODE
	{
		I_VV Info;
		Info_VV(hVV, &Info);
		ASSERT(Info.Type == GS_LONG);
	}
#endif


	// --- Allocate a Write Buffer ---

	hPP->pEdit->pWriteBuffer = (VOL_BYT*) AllocTemp_MP(hPP->hMP, PMF_PP_WRITE_BUFFER);
	hPP->pEdit->lBuffCounter = 0;
	OutBuff.sId = VALID_PMF;


	// --- Start the Main Loop ---

	for (t = 0L; t < lObjects; t++)
	{

		// --- Get the Entity ID Number ---

		if (GetE_VV(hVV, t, &lId))
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto END_PP_ERROR;
		}


		// --- Get the Information ---

		if (GetE_VV(hPP->pEdit->hVV, lId, &extent))
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto END_PP_ERROR;
		}
		extent.lOffset |= EXT_ERASED;     // Mark Entity as Erased
		if (SetE_VV(hPP->pEdit->hVV, lId, &extent))
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto END_PP_ERROR;
		}


		// --- Get offset of the new PMF ---

		lOffset = (unsigned long) Seek_PMF(hPP->pEdit->hSPMF, 0, BF_SEEK_EOF);


		// --- What file do we use ? ---

		if (lId < hPP->pEdit->lSaveExtent)
			hPMF = hPP->pEdit->hPMF;
		else
			hPMF = hPP->pEdit->hSPMF;


		// --- Seek to proper location ---

		if (Seek_PMF(hPMF, extent.lOffset & EXT_LOWER, BF_SEEK_START) == -1)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto END_PP_ERROR;
		}


		// --- Is this an Old PMF ? ---

		if (extent.lOffset & EXT_OLD)
		{
			// --- Copy the entity, move, scale and change attributes ---

			for (i = 0; i < extent.sNumber; i++)
			{
				// --- Read the PMF Header ---

				if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
					goto END_PP_ERROR;


				// --- Is this an attribute Entity ---

				if ((Pmf.sNumber == 3) || (Pmf.sNumber == 2))
				{
					// --- Write the Header Out ---

					OutBuff.sNumber = Pmf.sNumber;
					if ((lPP_ERROR = lIData_PP(hPP, 4, (VOL_BYT*) &OutBuff)) != 0)
						goto END_PP_ERROR;

					// --- Translate and rescale everything ---

					sDone = 0;
					while (!sDone)
					{
						lBlocks = lReadOld_PMF(hPMF, hPP->pDataBuff, PMF_PP_READ_BUFFER, &sDone);
						if (lBlocks > 0)
						{
							ITrans_PP(hPP, lBlocks / 4, hPP->pDataBuff, CONV_SHORT, lOldOrigin, lNewOrigin, dScale);
							if ((lPP_ERROR = lIData_PP(hPP, lBlocks, hPP->pDataBuff)) != 0)
								goto END_PP_ERROR;
						}
						if (lBlocks == -1)
						{
							lPP_ERROR = PMF_PP_ERROR;
							goto END_PP_ERROR;
						}
					}

				}
				else
				{

					// --- Create the output PMF ---

					if (sAttrMode == 3)
						OutBuff.sNumber = 1000 + sNewAttr;    // Replace Attribute
					else
						OutBuff.sNumber = Pmf.sNumber;


					// --- Write the Data to the buffer ---

					if ((lPP_ERROR = lIData_PP(hPP, 4, (VOL_BYT*) &OutBuff)) != 0)
						goto END_PP_ERROR;
				}
			}

			// --- Compute new extents directly from old extents since ---
			// --- attributes cannot modify size of OLD Pmf's ---

			pExt.lOffset = ((unsigned long) lOffset | EXT_OLD | EXT_GRAPHIC);
			pExt.sNumber = extent.sNumber;
			pExt.lMin.x = lNewOrigin->x;
			pExt.lMin.y = lNewOrigin->y;
			pExt.lMax.x = lNewOrigin->x + RoundLong_GS((double) (extent.lMax.x - extent.lMin.x) * dScale->x);
			pExt.lMax.y = lNewOrigin->y + RoundLong_GS((double) (extent.lMax.y - extent.lMin.y) * dScale->y);

		}
		else
		{                         // --- Regular Pmf's ---

			// --- Read the PMF Header ---

			if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
				goto END_PP_ERROR;



			// --- Load the Old Attribute ---

			if (Pmf.sAttrib <= 0)
			{
				// --- Load Overide ---

				GetAttT_PP(hPP, (short) -Pmf.sAttrib, &PmfAttr);
				if ((lPP_ERROR = lAttrPmf_PP((CON_BYT*) Pmf.pOveride, Pmf.lOveride, &PmfAttr, &lPmfOveride)) != 0)
					goto END_PP_ERROR;
			}
			else
				GetAttT_PP(hPP, Pmf.sAttrib, &PmfAttr);


			// --- Do work with the appropriate Pmf's Data (no variable pmf's) ---

			switch (Pmf.sNumber)
			{
				case 20:           // --- Line ---

					ITrans_PP(hPP, 2, (VOL_BYT*) &((PMF_20*) hPP->pDataBuff)->rX1, CONV_FLOAT, lOldOrigin, lNewOrigin, dScale);
					break;

				case 26:           // --- Text ---

					if (((dScale->x != 1.0) || (dScale->y != 1.0)) && (((PMF_26*) hPP->pDataBuff)->sJust <= PMF_PP_BASE_ALL_CENTERED))
					{
						// --- Compute Sin and Cos ---

						if (((PMF_26*) hPP->pDataBuff)->sJust < PMF_PP_BASE_LEFT)
						{
							// --- Convert the Angle to Radians ---

							dAngle = (double) (((PMF_26*) hPP->pDataBuff)->rX2 / DEG_TO_GRAD);
							while (dAngle > PI2)
								dAngle -= PI2;
							while (dAngle < -PI2)
								dAngle += PI2;
							if ((dAngle == PI5) || (dAngle == 3 * PI5) || (dAngle == -PI5) || (dAngle == -3 * PI5))
								dAngle += 0.0001;

							dAngle = dAngle + (atan(tan(dAngle) * (dScale->y / dScale->x)) - atan(tan(dAngle)));
							((PMF_26*) hPP->pDataBuff)->rX2 = (float) (dAngle * DEG_TO_GRAD);

						}
						else
						{

							// --- Compute the Angle ---

							dX = ((PMF_26*) hPP->pDataBuff)->rX2 - ((PMF_26*) hPP->pDataBuff)->rX1;
							dY = ((PMF_26*) hPP->pDataBuff)->rY2 - ((PMF_26*) hPP->pDataBuff)->rY1;
							dAngle = atan(dY / dX);
						}

						// --- Localize the Angle ---

						if (dAngle < 0.0)
							dAngle = -dAngle;
						while (dAngle >= PI2)
							dAngle -= PI2;
						if (dAngle >= PI)
							dAngle = PI2 - dAngle;
						if (dAngle >= PI5)
							dAngle = PI - dAngle;


						// --- Determine Scaling ---

						dPerc = fabs(dAngle / PI5);
						dTextScale.x = ((1.0 - dPerc) * dScale->x) + (dPerc * dScale->y);
						dTextScale.y = (dPerc * dScale->x) + ((1.0 - dPerc) * dScale->y);
						sScaleText = 1;
					}

					// --- Scale the Points ---

					if (((PMF_26*) hPP->pDataBuff)->sJust < PMF_PP_BASE_LEFT)
						ITrans_PP(hPP, 1, (VOL_BYT*) &((PMF_26*) hPP->pDataBuff)->rX1, CONV_FLOAT, lOldOrigin, lNewOrigin, dScale);
					else
						ITrans_PP(hPP, 2, (VOL_BYT*) &((PMF_26*) hPP->pDataBuff)->rX1, CONV_FLOAT, lOldOrigin, lNewOrigin, dScale);


					break;

				case 27:           // --- Circle ---

					ITrans_PP(hPP, 1, (VOL_BYT*) &((PMF_27*) hPP->pDataBuff)->rX, CONV_FLOAT, lOldOrigin, lNewOrigin, dScale);
					((PMF_27*) hPP->pDataBuff)->rRadius *= (float) dScale->x;
					break;

				case 28:           // --- Symbol ---

					if ((dScale->x != 1.0) || (dScale->y != 1.0))
					{
						sScaleText = 1;
						dTextScale = *dScale;
					}

					ITrans_PP(hPP, 1, (VOL_BYT*) &((PMF_28*) hPP->pDataBuff)->rX, CONV_FLOAT, lOldOrigin, lNewOrigin, dScale);
					break;

				case 29:           // --- Poly Symbol ---

					if ((dScale->x != 1.0) || (dScale->y != 1.0))
					{
						sScaleText = 1;
						dTextScale = *dScale;
					}
					break;
			}


			// --- Scale Text and Symbols ---

			if (sScaleText)
			{
				// --- Create an Attribute Scale ---

				if (sAttrMode == 0)
					sAttrMode = 1;
				lOveride |= ATR_TSTY;
				pAttrData->Tsty.ht = RoundLong_GS((double) PmfAttr.Tsty.ht * dTextScale.y);
				pAttrData->Tsty.wd = RoundLong_GS((double) PmfAttr.Tsty.wd * dTextScale.x);
				pAttrData->Tsty.sp = RoundLong_GS((double) PmfAttr.Tsty.sp * dTextScale.x);
			}

			// --- Change the Attribute itself ? ---

			if (sAttrMode == 3)
			{
				Pmf.sAttrib = sNewAttr;
				lPmfOveride = 0L;
			}

			// --- Clear attributes ? ---

			if ((sAttrMode == 2) && (Pmf.sAttrib != 0))
				lPmfOveride = 0L;


			// --- Add all the overides ---

			if (lOveride & ATR_LCOL)
				PmfAttr.Lcol = pAttrData->Lcol;
			if (lOveride & ATR_LTHK)
				PmfAttr.Lthk = pAttrData->Lthk;
			if (lOveride & ATR_LPAT)
				PmfAttr.Lpat = pAttrData->Lpat;
			if (lOveride & ATR_LPTC)
				PmfAttr.Lptc = pAttrData->Lptc;
			if (lOveride & ATR_FCOL)
				PmfAttr.Fcol = pAttrData->Fcol;
			if (lOveride & ATR_TSTY)
				PmfAttr.Tsty = pAttrData->Tsty;
			if (lOveride & ATR_FONT)
				strcpy_GS(PmfAttr.Font, pAttrData->Font, sizeof (PmfAttr.Font));
			lPmfOveride |= lOveride;


			// --- Do we need to change the attribute ---

			if (lPmfOveride)
				((PMF_21*) hPP->pDataBuff)->sAttrib = (short) (-abs(Pmf.sAttrib));
			else
				((PMF_21*) hPP->pDataBuff)->sAttrib = (short) (abs(Pmf.sAttrib));


			// --- Write a Pmf Header ---

			OutBuff.sNumber = Pmf.sNumber;
			OutBuff.lSize = Pmf.lDataSize / 4;
			if ((lPP_ERROR = lIData_PP(hPP, 8, (VOL_BYT*) &OutBuff)) != 0)
				goto END_PP_ERROR;
			if ((lPP_ERROR = lIData_PP(hPP, Pmf.lDataSize, hPP->pDataBuff)) != 0)
				goto END_PP_ERROR;


			// --- Create the Attribute Overide PMF ---

			if (lPmfOveride)
			{
				lBlocks = lMakeAttrPmf_PP(&PmfAttr, hPP->pDataBuff, lPmfOveride);
				OutBuff.sNumber = 2000;
				OutBuff.lSize = lBlocks / 4L;

				if ((lPP_ERROR = lIData_PP(hPP, 8, (VOL_BYT*) &OutBuff)) != 0)
					goto END_PP_ERROR;
				if ((lPP_ERROR = lIData_PP(hPP, lBlocks, hPP->pDataBuff)) != 0)
					goto END_PP_ERROR;
			}

			// --- Write out Data PMF ---

			if ((Pmf.sNumber == 21) || (Pmf.sNumber == 22) || (Pmf.sNumber == 29) || (Pmf.sNumber == 26))
			{
				// --- Write out the Header ---

				OutBuff.sNumber = 2001;
				OutBuff.lSize = Pmf.lOtherData / 4L;
				if ((lPP_ERROR = lIData_PP(hPP, 8, (VOL_BYT*) &OutBuff)) != 0)
					goto END_PP_ERROR;


				// --- Is the a text entity ? ---

				if (Pmf.sNumber == 26)
				{

					// --- Read the PMF data into Buffer ---

					if ((lBlocks = lReadData_PMF(hPMF, hPP->pDataBuff, Pmf.lOtherData, Pmf.lOtherData)) == -1)
					{
						lPP_ERROR = PMF_PP_ERROR;
						goto END_PP_ERROR;
					}
					if ((lPP_ERROR = lIData_PP(hPP, lBlocks, hPP->pDataBuff)) != 0)
						goto END_PP_ERROR;

				}
				else
				{

					// --- Process all the Data ---

					while (Pmf.lOtherData > 0L)
					{
						lBlocks = lReadData_PMF(hPMF, hPP->pDataBuff, PMF_PP_READ_BUFFER, Pmf.lOtherData);
						if (lBlocks == -1)
						{
							lPP_ERROR = PMF_PP_ERROR;
							goto END_PP_ERROR;
						}
						if (lBlocks > 0)
						{
							ITrans_PP(hPP, lBlocks / 8, hPP->pDataBuff, CONV_FLOAT, lOldOrigin, lNewOrigin, dScale);
							if ((lPP_ERROR = lIData_PP(hPP, lBlocks, hPP->pDataBuff)) != 0)
								goto END_PP_ERROR;
						}
						Pmf.lOtherData -= lBlocks;
					}
				}
			}

			// --- Flush the last part of the buffer ---

			if ((lPP_ERROR = lIData_PP(hPP, -1, NULL)) != 0)
				goto END_PP_ERROR;


			// --- Re-Read the new entity ---

			hPMF = hPP->pEdit->hSPMF;
			if (Seek_PMF(hPMF, lOffset, BF_SEEK_START) == -1)
			{
				lPP_ERROR = PMF_PP_ERROR;
				goto END_PP_ERROR;
			}
			if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
				goto END_PP_ERROR;


			// --- Generate New Extents ---

			if ((lPP_ERROR = lExtPmf_PP(hPP, hPMF, &Pmf, lOffset, &pExt)) != 0)
				goto END_PP_ERROR;

		}


		// --- Add the Extent to the list ---

		if ((lNewId = lAddExt_PP(hPP, &pExt)) == -1)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto END_PP_ERROR;
		}
		SetE_VV(hVV, t, &lNewId);

	}                            // For Loop

	// --- Flush the last part of the buffer ---

	if ((lPP_ERROR = lIData_PP(hPP, -1, NULL)) != 0)
		goto END_PP_ERROR;

	// --- In Case of Errror ---

END_PP_ERROR:

	if (hPP->pEdit->pWriteBuffer)
		Free_MP(hPP->hMP, hPP->pEdit->pWriteBuffer);
	hPP->pEdit->pWriteBuffer = NULL;
	return (lPP_ERROR);

}


//-----------------------------------------------------------------------------

long lNewExtPmf_PP(H_PP*    hPP,                // PP Object
                   long*    pId,                // Id of the PMF to be returned
                   short    sNumber,            // PMF Number
                   long     lData,              // Size of Data for PMF
                   CON_BYT* pData,              // Data of the PMF
                   long     lOtherData,         // Size of Other Data in PMF (-1 for NON)
                   // -2 - pOtherData contains a VV with data
                   CON_BYT* pOtherData,         // Other Data Pointer (or VV handler)
                   short    sGraphics,          // Is this a graphics entity ? (attributes)
                   short    sAttrib,            // Attribute of PMF
                   long     lOverides,          // Overides to the attribute
                   Att_PP*  pAttr)              // Attribute to use for overides (null if non)
{
	DEBUG_STACK;

	long       lPP_ERROR = NO_ERROR;
	H_PMF*     hPMF;
	long       lOffset;
	Entity_PMF Pmf;
	SYS_ALIGNED_STACK_BUFFER(pBuff1, 100);
	SYS_ALIGNED_STACK_BUFFER(pBuff2, 100);
	extent_ob  pExt;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Create the PMF Object ---

	hPMF = hPP->pEdit->hSPMF;
	memset(&Pmf, 0, sizeof (Pmf));
	Pmf.sNumber = sNumber;
	Pmf.sGraphicsEntity = sGraphics;
	Pmf.lOtherData = lOtherData;
	if (Pmf.lOtherData == -2)
		Pmf.lOtherData = Len_VV((H_VV*) pOtherData)* sDiskSize_VV((H_VV*) pOtherData);

	// --- Is this a graphics entity, then add attribute information ---

	if (sGraphics)
	{
		// --- Attribute information ---

		Pmf.sAttrib = sAttrib;
		Pmf.pData = (VOL_STR*) pBuff2;
		Pmf.lDataSize = lData + 4;

		// --- Add space and attribute to data ---

		((short*) pBuff2)[0] = 0;
		((short*) pBuff2)[1] = sAttrib;
		if (pData)
			memcpy(((VOL_BYT*) pBuff2) + 4, pData, lData);

	}
	else
	{

		// --- No change to data ---

		Pmf.lDataSize = lData;
		Pmf.pData = (VOL_STR*) pData;
	}




	// --- If a graphics entity and an overide, the create the overide ---

	if (sGraphics && lOverides)
	{
		// --- Create the Overide ---

		Pmf.pOveride = (VOL_BYT*) pBuff1;
		Pmf.lOveride = lMakeAttrPmf_PP(pAttr, (VOL_BYT*) pBuff1, lOverides);

	}
	else
	{

		// --- No Overide ---

		Pmf.pOveride = NULL;
		Pmf.lOveride = 0;
	}

	// --- Write the PMF to the Save File ---

	Mode_PMF(hPMF, WRITE_MODE);
	if ((lOffset = Seek_PMF(hPMF, 0, BF_SEEK_EOF)) == -1)
		return (PMF_PP_ERROR);
	lWrite_PMF(hPMF, &Pmf);


	// --- Write the Data to the Save File ---

	if (lOtherData == -2)
	{
		// --- Write the VV Data Out ---

		if (WtBF_VV((H_VV*) pOtherData, hPMF->ipBF, sEType_VV((H_VV*) pOtherData)))
			return (PMF_PP_ERROR);

	}
	else if (lOtherData != -1)
	{

		// --- Write Other Data ---

		if (sWriteData_PMF(hPMF, pOtherData, Pmf.lOtherData))
			return (PMF_PP_ERROR);
	}


	// --- Generate an Extents for this Pmf ---

	Mode_PMF(hPMF, READ_MODE);
	if (Seek_PMF(hPMF, lOffset, BF_SEEK_START) == -1)
		return (PMF_PP_ERROR);
	if ((lPP_ERROR = lRead_PMF(hPMF, &Pmf, hPP->pDataBuff, PMF_PP_READ_BUFFER)) != 0)
		return (lPP_ERROR);
	if ((lPP_ERROR = lExtPmf_PP(hPP, hPMF, &Pmf, lOffset, &pExt)) != 0)
		return (lPP_ERROR);


	// --- Add the Extent to the list ---

	if ((*pId = lAddExt_PP(hPP, &pExt)) == -1)
		return (lPP_ERROR);

	// --- Return No PMF_PP_ERROR ---

	return (NO_ERROR);

}

//-----------------------------------------------------------------------------

void EraseExt_PP(H_PP* hPP,             // PP Object
                 H_VV* hVV,             // VV object holding ID's to duplicate
                 long  lSize,           // Number of entities to duplicate
                 short sErased)         // 0 - Not Erased, 1 - Erased
{
	DEBUG_STACK;

	unsigned long lVal = 0L;
	long          lId;
	long*         pId;
	extent_ob*    pExt;
	H_VVV*        hVVV = NULL;
	ppedt_ob*     pEdit = hPP->pEdit;


	// --- Validate Parameters ---

	ASSERT(hPP && hVV);                  // Real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
#ifdef DEBUG_CODE
	{
		I_VV Info;
		Info_VV(hVV, &Info);
		ASSERT(Info.Type == GS_LONG);
	}
#endif

	// --- Turning ON or OFF ? ---

	if (sErased)
		lVal = EXT_ERASED;


	// --- Start Scanning Through the entities ---

	hVVV = hCreat_VVV(hVV);
	if (hVVV == NULL)
		return;
	sStartNext_VVV(hVVV, 0L, 0);


	// --- Start Changing ---

	for (lId = 0L; lId < lSize; lId++)
	{
		// --- Get the Pointers ---

		pId = (long*) pNext_VVV(hVVV);
		pExt = (extent_ob*) pPoint_VVV(pEdit->hVVV, *pId);

		// --- Change the Entity ---

		pExt->lOffset = (pExt->lOffset & (~(EXT_ERASED))) | lVal;
	}
	Destr_VVV(hVVV);

}



//-----------------------------------------------------------------------------

long lDupExt_PP(H_PP* hPP,      // PP Object
                H_VV* hOVV,     // VV object holding ID's to duplicate
                long  lSize,    // Number of entities to duplicate
                H_VV* hVV,      // Destination VV
                short sErase)   // Erase the copy ?
{
	DEBUG_STACK;

	H_VVV*     hOVVV = NULL;
	H_VVV*     hVVV = NULL;
	H_PMF*     hPMF;
	long       lId;
	long*      pId;
	long*      pDId;
	long       lOffset;
	long       lCopySize;
	long       lRSize;
	long       lPP_ERROR;
	long       lBlocks;
	extent_ob* pExt;
	extent_ob  Ext;
	ppedt_ob*  pEdit = hPP->pEdit;


	// --- Validate Parameters ---

	ASSERT(hPP && hVV && hOVV);          // Real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
#ifdef DEBUG_CODE
	{
		I_VV Info;
		Info_VV(hVV, &Info);
		ASSERT(Info.Type == GS_LONG);
		Info_VV(hOVV, &Info);
		ASSERT(Info.Type == GS_LONG);
	}
#endif


	// --- Get the Offset ---

	lOffset = Seek_PMF(pEdit->hSPMF, 0, BF_SEEK_EOF);
	if (lOffset == -1)
		return (PMF_PP_ERROR);


	// --- Copy the VV ---

	Copy_VV(hVV, hOVV);
	hOVVV = hCreat_VVV(hOVV);
	hVVV = hCreat_VVV(hVV);
	if ((hOVVV == NULL) || (hVVV == NULL))
	{
		lPP_ERROR = PMF_PP_ERROR;
		goto DUP_PP_ERROR;
	}


	// --- Create the Write Buffer ---

	pEdit->pWriteBuffer = (VOL_BYT*) AllocTemp_MP(hPP->hMP, PMF_PP_WRITE_BUFFER);
	pEdit->lBuffCounter = 0;


	// --- Start Scanning Through the entities ---

	sStartNext_VVV(hVVV, 0, 0);
	sStartNext_VVV(hOVVV, 0, 0);

	for (lId = 0L; lId < lSize; lId++)
	{
		// --- Get the pointers ---

		pId = (long*) pNext_VVV(hOVVV);
		pDId = (long*) pNext_VVV(hVVV);
		Ext = *(extent_ob*) pPoint_VVV(pEdit->hVVV, *pId);

		// --- Get the Size of the Object ---

		if (*pId == pEdit->lSaveExtent - 1)
		{
			// --- End of save file, get the size ---

			lCopySize = pEdit->lPlotFileSize - (Ext.lOffset & EXT_LOWER);

		}
		else if (*pId == pEdit->lTotalExtents - 1)
		{

			lCopySize = lOffset - (Ext.lOffset & EXT_LOWER);

		}
		else
		{

			pExt = (extent_ob*) pPoint_VVV(pEdit->hVVV, *pId + 1L);
			lCopySize = (pExt->lOffset & EXT_LOWER) - (Ext.lOffset & EXT_LOWER);
		}

		// --- What file do we use ? ---

		if (*pId < pEdit->lSaveExtent)
			hPMF = pEdit->hPMF;
		else
			hPMF = pEdit->hSPMF;
		if (Seek_PMF(hPMF, (Ext.lOffset & EXT_LOWER), BF_SEEK_START) == -1)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto DUP_PP_ERROR;
		}


		// --- Copy the Object ---

		lRSize = lCopySize;
		while (lCopySize > 0L)
		{
			// --- Read and write the data ---

			lBlocks = lReadData_PMF(hPMF, hPP->pDataBuff, PMF_PP_READ_BUFFER, lCopySize);
			if (lBlocks > 0)
				if ((lPP_ERROR = lIData_PP(hPP, lBlocks, hPP->pDataBuff)) != 0)
					goto DUP_PP_ERROR;
			lCopySize -= lBlocks;
			if (lBlocks <= 0)
				break;
		}

		// --- Change the Entity ---

		Ext.lOffset = lOffset + (Ext.lOffset & (EXT_GRAPHIC | EXT_OLD | EXT_ATTRIB));
		if (sErase)
			Ext.lOffset |= EXT_ERASED;
		if ((*pDId = lAddExt_PP(hPP, &Ext)) == -1)
		{
			lPP_ERROR = PMF_PP_ERROR;
			goto DUP_PP_ERROR;
		}
		lOffset += lRSize;
	}


	// --- Flush the last part of the buffer ---

	if ((lPP_ERROR = lIData_PP(hPP, -1, NULL)) != 0)
		goto DUP_PP_ERROR;


	// --- In Case of Errror ---

DUP_PP_ERROR:
	Destr_VVV(hVVV);
	Destr_VVV(hOVVV);
	if (pEdit->pWriteBuffer)
		Free_MP(hPP->hMP, pEdit->pWriteBuffer);
	pEdit->pWriteBuffer = NULL;

	return (lPP_ERROR);

}


//--------------------------------------------------------------------
