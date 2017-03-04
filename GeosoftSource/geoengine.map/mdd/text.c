//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================





#include <sys.h>
#include <geosoft.h>

#include <ap.h>
#include <gs.h>
#include <mp.h>
#include <mview.h>
#include <geostruc.h>
#include <reg.h>
#include <dd.h>
#include <pmf.h>
#include <map.h>
#include <pply.h>
#include "mdd.ih"


//-----------------------------------------------------------------------------

short sTextSupport_DD(H_DD* hDD)
{
	return (1);
}


//-------------------------------------------------------------------------

short sSetText_DD(H_DD*    hDD,         // MVIEW
                  double   dSize,       // Font size in mm
                  double   dThick,      // Line thickness in mm
                  COL_ANY  Col,         // Color
                  long     lRef,        // Reference (0-8)
                  double   dAngle,      // angle
                  CON_STR* pcName,      // Face Name of the Font
                  bool  fItalics)    // Italics?
{
	DEBUG_STACK;

	short sCheckErr = 0;
	short sWeight;


	ASSERT(hDD);
	ASSERT(hDD->hMVIEW);


	// -- determine the font weight ---

	if ((dThick <= 0.0) || (dSize <= 0.0))
		sWeight = MVIEW_FONTWEIGHT_ULTRA_LIGHT;

	else
	{

		// --- ratio thickness to size ---

		dThick /= dSize;

		if (dThick <= 1.0 / 48.0)
			sWeight = MVIEW_FONTWEIGHT_ULTRA_LIGHT;
		else if (dThick <= 1.0 / 24.0)
			sWeight = MVIEW_FONTWEIGHT_LIGHT;
		else if (dThick <= 1.0 / 16.0)
			sWeight = MVIEW_FONTWEIGHT_MEDIUM;
		else if (dThick <= 1.0 / 12.0)
			sWeight = MVIEW_FONTWEIGHT_BOLD;
		else if (dThick <= 0.145)
			sWeight = MVIEW_FONTWEIGHT_XBOLD;
		else if (dThick <= 1.0 / 4.0)
			sWeight = MVIEW_FONTWEIGHT_XXBOLD;
		else
			sWeight = MVIEW_FONTWEIGHT_MEDIUM;

	}


	// --- is there a true-type map? ---

	if (hDD->hRegTT)
	{

		REG_Entry* hEntry;
		VOL_STR    szName[64];
		strcpy_s(szName, sizeof (szName), "TRUETYPE.");
		strcat_GS(szName, pcName, sizeof (szName));

		hEntry = hFind_REG(hDD->hRegTT, szName);
		if (hEntry)
			CHECK_ERR(sFontName_MVIEW(hDD->hMVIEW, szGetEntryValueRaw_REG(hEntry), false, sWeight, fItalics));
		else
			CHECK_ERR(sFontName_MVIEW(hDD->hMVIEW, pcName, true, sWeight, fItalics));

	}
	else
		CHECK_ERR(sFontName_MVIEW(hDD->hMVIEW, pcName, true, sWeight, fItalics));



	// --- set the other font parameters ---

	if (dSize <= 0.0)
		dSize = 0.0001;
	CHECK_ERR(sFontSize_MVIEW(hDD->hMVIEW, dSize * dScaleMM_MVIEW(hDD->hMVIEW)));

	CHECK_ERR(sFontColor_MVIEW(hDD->hMVIEW, Col));

	if (lRef < 0 || lRef > 8)
		lRef = 0;
	CHECK_ERR(sFontRef_MVIEW(hDD->hMVIEW, lRef));

	while (dAngle < -360.0)
		dAngle += 360.0;
	while (dAngle > 360.0)
		dAngle -= 360.0;
	CHECK_ERR(sFontAngle_MVIEW(hDD->hMVIEW, dAngle));


	// --- done ---

GS_EXIT:

	return (sCheckErr);


}


//-------------------------------------------------------------------------

GS_EXPORT short sText_DD(H_DD*           hDD,    // DD
                         const Point_DD* pP,     // Point
                         CON_STR*        pcText) // Text
{
	DEBUG_STACK;

	short      sCheckErr = 0;
	GS_D2POINT D2;
	CON_STR*   pcI;
	VOL_STR*   pcO;



	// --- Validate Parameters ---

	ASSERT(hDD);
	ASSERT(hDD->obj_id == OBJ_ID);
	ASSERT(hDD->hMVIEW);
	VERIFY_STRING(pcText);

	PT_VIEW(hDD, pP, &D2);
	hDD->D2_cur = D2;


	// --- make sure filter buffer is long enough for worst case ---

	if (hDD->lFilt <= ((long) strlen(pcText) * 3))
	{

		hDD->lFilt = (strlen(pcText) * 3) + 256;
		if (hDD->pcFilt)
			Free_MP(hDD->hMP, hDD->pcFilt);
		hDD->pcFilt = (VOL_STR*) Alloc_MP(hDD->hMP, hDD->lFilt);
		CHECK_ERR(hDD->pcFilt == NULL);
		strcpy_GS(hDD->pcFilt, "", hDD->lFilt);
	}


	if (hDD->lFiltU <= ((long) strlen(pcText) * 3))
	{

		hDD->lFiltU = (strlen(pcText) * 3) + 256;
		if (hDD->pcFiltU)
			Free_MP(hDD->hMP, hDD->pcFiltU);
		hDD->pcFiltU = (VOL_STR*) Alloc_MP(hDD->hMP, hDD->lFiltU);
		CHECK_ERR(hDD->pcFiltU == NULL);
		strcpy_GS(hDD->pcFiltU, "", hDD->lFiltU);
	}



	// --- just in case we don't have a buffer, check it ---

	if (bHaveSpecialChars_GS(pcText) && hDD->pcFilt)
	{
		// --- replace special characters with escape sequences ---

		pcI = pcText;
		pcO = hDD->pcFilt;

		while (*pcI)
		{
			switch (*pcI)
			{

				case 0x1d:         // diameter
					*pcO++ = '%';
					*pcO++ = '%';
					*pcO++ = 'c';
					break;

				case 0x1e:         // plus-minus
					*pcO++ = '%';
					*pcO++ = '%';
					*pcO++ = 'p';
					break;

				case 0x1f:         // degree
					*pcO++ = '%';
					*pcO++ = '%';
					*pcO++ = 'd';
					break;

				default:
					*pcO++ = *pcI;
			}
			pcI++;
		}
		*pcO = '\0';


		// --- Update the Pointer ---

		pcText = hDD->pcFilt;
	}


	// --- Do we still have special characters ? ---

	if (bHaveSpecialChars_GS(pcText) && hDD->pcFiltU)
	{
		// --- Convert from ACP to UTF8 ---

		ACP2strcpy_GS(hDD->pcFiltU, pcText, hDD->lFiltU);
		pcText = hDD->pcFiltU;
	}



	// --- Draw the Text ---

	CHECK_ERR(sText_MVIEW(hDD->hMVIEW, &D2, pcText));


	// --- Done ---

GS_EXIT:
	return (sCheckErr);


}
