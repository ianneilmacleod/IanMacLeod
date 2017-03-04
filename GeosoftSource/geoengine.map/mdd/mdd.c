//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
//  MDD  - MVIEW based implementation of DD.
//
//=============================================================================

// --- INCLUDES ---





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


H_DD* hCreatM_DD(H_MAP* hMAP, CON_STR* szPlt)
{
	DEBUG_STACK;

	short         sCheckErr = 0;

	H_MP*         hMP = NULL;
	H_DD*         hDD = NULL;
	H_AP*         hAP = NULL;
	H_PMF*        hPMF = NULL;

	Header_PMF    Header;
	long          lDx, lDy;

	GS_FILE_PARTS Fparts;

	VOL_STR       szName[GS_MAX_PATH];
	VOL_STR       szData[MVIEW_NAME_LENGTH];


	ASSERT(hMAP && szPlt);


	hAP = hGethAP_MAP(hMAP);


	// --- Allocate memory for the structure ---

	hMP = hGethMP_AP(hAP);
	hDD = (H_DD*) Alloc_MP(hMP, sizeof (H_DD));
	CHECK_ERR(hDD == NULL);


	// --- Initialize the structure ---

	hDD->obj_id = OBJ_ID;

	hDD->hMAP = hMAP;
	hDD->hMVIEW = NULL;
	hDD->hMP = hMP;
	hDD->hAP = hAP;
	hDD->D2_cur = GS_D2Point(0.0, 0.0);
	hDD->C_curL = SetAnyColor(C_ANY_NONE, 1, 1, 2);
	hDD->C_curF = hDD->C_curL;
	hDD->C_new = SetAnyColor(C_ANY_RGB, 0, 0, 0);
	hDD->dT_cur = -1e32;
	hDD->dT_new = 0.0;
	hDD->dXo = 0.0;
	hDD->dYo = 0.0;
	hDD->dXs = 1.0;
	hDD->dYs = 1.0;
	hDD->fNewViews = false;
	hDD->hRegTT = NULL;
	hDD->pcFilt = NULL;
	hDD->pcFiltU = NULL;
	hDD->lFilt = 0;
	hDD->lFiltU = 0;


	// --- open file as a PMF and get header ---

	hPMF = hCreat_PMF(hAP, szPlt, 0, BF_READ);
	CHECK_ERR(hPMF == NULL);
	CHECK_ERR(lHeader_PMF(hPMF, &Header));
	Destr_PMF(hPMF, BF_KEEP);
	hPMF = NULL;


	// --- Is there enough Header Information ---

	if (!(Header.sFlags & LOADED_SIZE))
	{
		RegistErr_AP(hAP, 24570, modn);
		SetParmS_AP(hAP, "%1", szPlt);
		sCheckErr = 1;
		goto GS_EXIT;
	}


	// --- Validate the Header Information ---

	lDx = Header.MaxRegion.x - Header.MinRegion.x;
	lDy = Header.MaxRegion.y - Header.MinRegion.y;
	if ((lDx == 0) || (lDy == 0))
	{
		RegistErr_AP(hAP, 24570, modn);
		SetParmS_AP(hAP, "%1", szPlt);
		sCheckErr = 1;
		goto GS_EXIT;
	}


	// --- determine plotting scale ---

	hDD->dXo = Header.dOrigin.x;
	hDD->dYo = Header.dOrigin.y;
	hDD->dXs = Header.dScale * Header.dConv / 1000000.0;
	hDD->dYs = hDD->dXs;


	// --- get data view ---

	CHECK_ERR(sGetDataView_MAP(hDD->hMAP, szData, sizeof (szData)));
	strcpy_GS(hDD->szName, szData, sizeof (hDD->szName));
	if (!fExistView_MAP(hMAP, hDD->szName))
	{
		hDD->hMVIEW = hCreat_MVIEW(hMAP, hDD->szName, MVIEW_WRITENEW);
		CHECK_ERR(hDD->hMVIEW == NULL);
		CHECK_ERR(sGroupClipMode_MVIEW(hDD->hMVIEW, MVIEW_CLIP_MODE_OFF));
	}
	else
	{
		hDD->hMVIEW = hCreat_MVIEW(hMAP, hDD->szName, MVIEW_WRITEOLD);
		CHECK_ERR(hDD->hMVIEW == NULL);
		CHECK_ERR(sGroupClipMode_MVIEW(hDD->hMVIEW, MVIEW_CLIP_MODE_ON));
	}


	// --- group name ---

	strcpy_GS(szName, szPlt, sizeof (szName));
	FileParts_GS(szName, &Fparts);
	strlwr_GS(Fparts.pcName);
	strcpy_GS(hDD->szGrp, Fparts.pcName, sizeof (hDD->szGrp));
	strcat_GS(hDD->szGrp, ".", sizeof (hDD->szGrp));
	strcat_GS(hDD->szGrp, Fparts.pcExt, sizeof (hDD->szGrp));
	strcpy_GS(szName, hDD->szGrp, sizeof (szName));
	CHECK_ERR(sGenNewGroupName_MVIEW(hDD->hMVIEW, szName, hDD->szGrp, sizeof (hDD->szGrp)));
	CHECK_ERR(sGroupStream_MVIEW(hDD->hMVIEW, hDD->szGrp, MVIEW_GROUP_NEW));


	// --- get True-Type reg ---


	if (sFindG_GS("truetype.ini", "", GS_FIND_LOCAL_GEOSOFT, GS_DIRECTORY_INI, GS_NAME_FULL, szName, sizeof (szName)) == 0)
	{
		hDD->hRegTT = hCreat_REG(hAP);
		CHECK_ERR(hDD->hRegTT == NULL);
		CHECK_ERR(sLoadINI_REG(hDD->hRegTT, szName));
	}


	// --- create a starting text buffer ---

	hDD->lFilt = 256;
	hDD->pcFilt = (VOL_STR*) Alloc_MP(hMP, hDD->lFilt);
	CHECK_ERR(hDD->pcFilt == NULL);

	hDD->lFiltU = 256;
	hDD->pcFiltU = (VOL_STR*) Alloc_MP(hMP, hDD->lFiltU);
	CHECK_ERR(hDD->pcFiltU == NULL);

	// --- Done ---

GS_EXIT:

	if (sCheckErr)
	{

		if (hPMF)
			Destr_PMF(hPMF, BF_KEEP);
		if (hDD->hMVIEW)
			Destr_MVIEW(hDD->hMVIEW);
		if (hDD->hRegTT)
			Destr_REG(hDD->hRegTT);
		if (hDD)
			Free_MP(hMP, hDD);
		return (NULL);

	}
	else
		return (hDD);

}



//-----------------------------------------------------------------------------

void Destr_DD(H_DD* hDD)      // DD handle
{
	DEBUG_STACK;

	short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hDD);                         // Object must be real
	ASSERT(hDD->obj_id == OBJ_ID);       // Is an Actual DD object


	// --- close the current view ---

	if (hDD->hMVIEW)
		Destr_MVIEW(hDD->hMVIEW);


	// --- Free the data ---

	if (hDD->hRegTT)
		Destr_REG(hDD->hRegTT);
	if (hDD->pcFilt)
		Free_MP(hDD->hMP, hDD->pcFilt);
	if (hDD->pcFiltU)
		Free_MP(hDD->hMP, hDD->pcFiltU);
	Free_MP(hDD->hMP, hDD);

}


//-----------------------------------------------------------------------------

short Color_DD(H_DD*    hDD,    // DD handle
               GeoColor col)    // GeoColor to set.
{
	DEBUG_STACK;

	short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hDD);                         // Object must be real
	ASSERT(hDD->obj_id == OBJ_ID);       // Is an Actual DD object


	// --- set new colour ---

	hDD->C_new = AnyColor(col);

	return (0);

}


//-----------------------------------------------------------------------------

void Sat_DD(H_DD*    hDD,       // DD handle
            GeoColor col)       // Maximum KCMY colour to allow
{
	DEBUG_STACK;

	short sCheckErr = 0;

	return;
}


//-----------------------------------------------------------------------------

void Thick_DD(H_DD* hDD,        // DD handle
              long  thk)        // line thickness in microns
{
	DEBUG_STACK;

	short sCheckErr = 0;


	ASSERT(hDD);                         // Object must be real
	ASSERT(hDD->obj_id == OBJ_ID);       // Is an Actual DD object

	hDD->dT_new = thk * hDD->dXs;

	return;

}

//-----------------------------------------------------------------------------

short Move_DD(H_DD*    hDD,     // DD handle
              Point_DD pt)      // move to this point
{
	DEBUG_STACK;

	short      sCheckErr = 0;
	GS_D2POINT D2;


	// --- Validate Parameters ---

	ASSERT(hDD);                         // Object must be real
	ASSERT(hDD->obj_id == OBJ_ID);       // Is an Actual DD object

	PT_VIEW(hDD, &pt, &D2);
	hDD->D2_cur = D2;

	return (0);

}



//-----------------------------------------------------------------------------

short Vect_DD(H_DD*    hDD,     // DD handle
              Point_DD pt)      // vector to this point
{
	DEBUG_STACK;

	short      sCheckErr = 0;
	GS_D2POINT D2;
	GS_D2LINE  L2;


	// --- Validate Parameters ---

	ASSERT(hDD);                         // Object must be real
	ASSERT(hDD->obj_id == OBJ_ID);       // Is an Actual DD object


	// --- convert to view units ---

	PT_VIEW(hDD, &pt, &D2);


	// --- draw the line ---

	L2 = GS_D2Line(hDD->D2_cur.dX, hDD->D2_cur.dY, D2.dX, D2.dY);

	CHECK_LINE(hDD);
	sLine_MVIEW(hDD->hMVIEW, &L2);


	// --- save current point ---

	hDD->D2_cur = D2;

	return (0);

}



//-----------------------------------------------------------------------------


short PVect_DD(H_DD*     hDD,   // DD handle
               Point_DD* pts,   // chain of points
               long      numb)  // number of points in the polyline
{
	DEBUG_STACK;

	short       sCheckErr = 0;

	long        s;
	GS_D2POINT* pD2 = NULL;
	GS_D2POINT* pD;
	Point_DD*   pP;


	// --- Validate Parameters ---

	ASSERT(hDD && pts);                  // Objects must be real
	ASSERT(hDD->obj_id == OBJ_ID);       // Is an Actual DD object

	if (numb < 1)
		goto GS_EXIT;                     // Must have at least one point


	// --- Allocate memory ---

	pD2 = (GS_D2POINT*) Alloc_MP(hDD->hMP, (numb + 1) * sizeof (*pD2));
	CHECK_ERR(pD2 == NULL);


	// --- copy/convert points ---

	*pD2 = hDD->D2_cur;
	for (s = 0, pP = pts, pD = pD2 + 1; s < numb; s++, pP++, pD++)
		PT_VIEW(hDD, pP, pD);


	// --- draw the line ---

	CHECK_LINE(hDD);
	CHECK_ERR(sPolyDraw_MVIEW(hDD->hMVIEW, MVIEW_DRAW_POLYLINE, (long) (numb + 1), pD2));


	// --- save current point ---

	hDD->D2_cur = pD2[numb];


	// --- done ---

GS_EXIT:


	// --- Free the memory ---

	if (pD2)
		Free_MP(hDD->hMP, pD2);

	return (0);

}


//-----------------------------------------------------------------------------

short Fill_DD(H_DD*     hDD,    // DD handle
              Point_DD* pts,    // chain of points bounding area to be filled
              long      numb)   // number of points in the polyline
{
	DEBUG_STACK;

	short       sCheckErr = 0;

	long        s;
	GS_D2POINT* pD2 = NULL;
	GS_D2POINT* pD;
	Point_DD*   pP;



	// --- Validate Parameters ---

	ASSERT(hDD && pts);                  // Objects must be real
	ASSERT(hDD->obj_id == OBJ_ID);       // Is an Actual DD object
	ASSERT(numb > 0);                    // Must have at least one point


	// --- Allocate memory ---

	pD2 = (GS_D2POINT*) Alloc_MP(hDD->hMP, (long) numb * sizeof (*pD2));
	CHECK_ERR(pD2 == NULL);


	// --- copy/convert points ---

	for (s = 0, pP = pts, pD = pD2; s < numb; s++, pP++, pD++)
		PT_VIEW(hDD, pP, pD);


	// --- draw polygon ---

	CHECK_LINE(hDD);
	CHECK_FILL(hDD);
	CHECK_ERR(sPolyDraw_MVIEW(hDD->hMVIEW, MVIEW_DRAW_POLYGON, numb, pD2));


	// --- save current point ---

	hDD->D2_cur = pD2[numb - 1];


	// --- done ---

GS_EXIT:


	// --- Free the memory ---

	if (pD2)
		Free_MP(hDD->hMP, pD2);

	return (sCheckErr);


}

//-----------------------------------------------------------------------------

short Show_DD(H_DD* hDD)      // DD handle
{
	DEBUG_STACK;

	short sCheckErr = 0;


	return (0);


}

//-----------------------------------------------------------------------------

short SetI_DD(H_DD* hDD,        // DD handle
              short info,       // desired info from DD_I_ list of defines
              I_DD* data)       // info union
{
	DEBUG_STACK;

	short sCheckErr = 0;


	return (0);


}


//-----------------------------------------------------------------------------

short GetI_DD(H_DD* hDD,        // DD handle
              short info,       // desired info from DD_I_ list of defines.
              I_DD* data)       // info union
{
	DEBUG_STACK;

	short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hDD && data);                                 // Objects must be real
	ASSERT(hDD->obj_id == OBJ_ID);                       // Is an Actual DD object
	ASSERT((info > 0) && (info < MAX_DD_ATTRIBS));       // Valid Info Request


	// --- Return an attribute ---

	if ((info > 0) && (info < MAX_DD_ATTRIBS))
	{
		// --- Do we have to do anything ? ---

		switch (info)
		{
			case DD_XSIZE:
				data->Long = GS_S4MX;
				break;

			case DD_YSIZE:
				data->Long = GS_S4MX;
				break;

			default:
				return (1);
		}
		return (0);
	}
	else
		return (1);


}

//-----------------------------------------------------------------------------

short Clear_DD(H_DD* hDD)     // DD handle
{
	DEBUG_STACK;

	short sCheckErr = 0;


	return (0);


}


//-----------------------------------------------------------------------------

short Flush_DD(H_DD* hDD)     // DD handle
{
	DEBUG_STACK;

	short sCheckErr = 0;


	return (0);

}


//-----------------------------------------------------------------------------

short sPmf_DD(H_DD*    hDD,     // DD
              CON_DAT* pPmf)    // PMF data
{
	DEBUG_STACK;

	short sCheckErr = 0;

	// --- PMF data block ---

	struct P50
	{
		short sFlag;
		short sPmf;
		long lLen;
		VOL_DAT* pData;
	}*        pP;

	MVIEW_ATT Matt;
	bool   fAtt = false;
	bool   fObj = false;

	VOL_STR*  cp, * cp2;
	VOL_STR   szGroup[255];


	ASSERT(hDD && pPmf);

	pP = (struct P50*) pPmf;

	if (pP->sFlag != -32767)
		return (0);

	switch (pP->sPmf)
	{

		case 50:
		case 51:
		{


			// --- new view ---

			struct P50D
			{
				double dXo;
				double dYo;
				double dXs;
				double dYs;
				GS_D2AREA D2;
				CON_STR szName[32];
			}* p50;

			if (pP->lLen != (long) sizeof (*p50) / 4)
				break;
			p50 = (struct P50D*) pP->pData;


			// --- set translation mm -> view ---

			hDD->dXo = p50->dXo;
			hDD->dYo = p50->dYo;
			hDD->dXs = 1.0 / p50->dXs / 1000.0;    // microns to user units
			hDD->dYs = 1.0 / p50->dYs / 1000.0;


			// --- close the current view ---

			if (hDD->hMVIEW)
			{

				fObj = bIsCurObj_MVIEW(hDD->hMVIEW);
				if (fObj)
				{
					GetAtt_MVIEW(hDD->hMVIEW, &Matt);
					fAtt = true;
				}
				else
					fAtt = false;
				Destr_MVIEW(hDD->hMVIEW);
				hDD->hMVIEW = NULL;
			}
			else
				fAtt = false;


			// --- does view already exist? ---

			if (!fExistView_MAP(hDD->hMAP, p50->szName))
			{


				// --- create a new view ---

				hDD->hMVIEW = hCreat_MVIEW(hDD->hMAP, p50->szName, MVIEW_WRITENEW);
				CHECK_ERR(hDD->hMVIEW == NULL);
				CHECK_ERR(sGroupStream_MVIEW(hDD->hMVIEW, hDD->szGrp, MVIEW_GROUP_APPEND));


				// --- set the new view translation ---

				CHECK_ERR(sSetTrans_MVIEW(hDD->hMVIEW, p50->dXo, p50->dYo, p50->dXs, p50->dYs));


				// --- set the clip window if one is provided

				if (((p50->D2.dMaxX - p50->D2.dMinX) > 0.0) && ((p50->D2.dMaxY - p50->D2.dMinY) > 0.0))
					CHECK_ERR(sRectangle_PPLY(hGetPPLY_MVIEW(hDD->hMVIEW), &p50->D2));


			}
			else
			{


				// --- open the view ---

				hDD->hMVIEW = hCreat_MVIEW(hDD->hMAP, p50->szName, MVIEW_WRITEOLD);
				CHECK_ERR(hDD->hMVIEW == NULL);
				CHECK_ERR(sGroupStream_MVIEW(hDD->hMVIEW, hDD->szGrp, MVIEW_GROUP_APPEND));


			}


			// --- we have new views ---

			hDD->fNewViews = true;


		}
			break;


		case 52:                 // new group

			cp = NULL;
			cp2 = NULL;
			strcpy_GS(szGroup, (CON_STR*) (pP->pData), sizeof (szGroup));

			// --- Ensure we have valid group name ---

			cp = strrchr_GS(szGroup, '\\');
			if (cp == NULL)
				cp = szGroup;
			else
				cp++;

			cp2 = strrchr_GS(cp, ':');
			if (cp2 == NULL)
				cp2 = cp;
			else
				cp2++;

			strcpy_GS(hDD->szGrp, cp2, sizeof (hDD->szGrp));

			CHECK_ERR(sGroupStream_MVIEW(hDD->hMVIEW, hDD->szGrp, MVIEW_GROUP_NEW));

			// --- set the attributes ---

			if (fAtt)
				CHECK_ERR(sSetAtt_MVIEW(hDD->hMVIEW, &Matt));

			break;


		case 53:                 // change group

			cp = NULL;
			cp2 = NULL;
			strcpy_GS(szGroup, (CON_STR*) (pP->pData), sizeof (szGroup));

			// --- Ensure we have valid group name ---

			cp = strrchr_GS(szGroup, '\\');
			if (cp == NULL)
				cp = szGroup;
			else
				cp++;

			cp2 = strrchr_GS(cp, ':');
			if (cp2 == NULL)
				cp2 = cp;
			else
				cp2++;

			fObj = bIsCurObj_MVIEW(hDD->hMVIEW);
			if (fObj)
			{
				GetAtt_MVIEW(hDD->hMVIEW, &Matt);
				fAtt = true;
			}
			strcpy_GS(hDD->szGrp, cp2, sizeof (hDD->szGrp));

			CHECK_ERR(sGroupStream_MVIEW(hDD->hMVIEW, hDD->szGrp, MVIEW_GROUP_APPEND));

			// --- set the attributes ---

			if (fAtt)
				CHECK_ERR(sSetAtt_MVIEW(hDD->hMVIEW, &Matt));

			break;

	}

GS_EXIT:

	return (sCheckErr);


}

//-------------------------------------------------------------------------

GS_EXPORT short sEllipse_DD(H_DD*           hDD,        // DD
                            const Point_DD* pCenter,    // Circle center in microns
                            long            lRadius,    // Circle radius in microns
                            GeoColor        col,        // Line colour
                            long            dThk)       // line thickness in microns
{
	DEBUG_STACK;

	short      sCheckErr = 0;

	Point_DD   D1, D2;
	GS_D2POINT P1, P2;
	GS_D2AREA  D2Area;
	long       lT;


	// --- Validate Parameters ---

	ASSERT(hDD);
	ASSERT(hDD->obj_id == OBJ_ID);
	ASSERT(hDD->hMVIEW);

	// --- convert to view units ---

	lT = GS_MAX((long) (dThk / 2. + 0.5), 1);

	D1.x = pCenter->x - lRadius - lT;
	D1.y = pCenter->y - lRadius - lT;

	D2.x = pCenter->x + lRadius + lT;
	D2.y = pCenter->y + lRadius + lT;

	PT_VIEW(hDD, &D1, &P1);
	PT_VIEW(hDD, &D2, &P2);

	D2Area = GS_D2Area(P1.dX, P1.dY, P2.dX, P2.dY);

	CHECK_ERR(sLineColor_MVIEW(hDD->hMVIEW, AnyColor(col)));
	CHECK_ERR(sLineThick_MVIEW(hDD->hMVIEW, dThk));

	CHECK_ERR(sEllipse_MVIEW(hDD->hMVIEW, &D2Area));

GS_EXIT:

	return (sCheckErr);


}

//-----------------------------------------------------------------------------
