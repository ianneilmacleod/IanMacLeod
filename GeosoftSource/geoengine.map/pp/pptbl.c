//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
// PPTBL.c
//
// Geosoft high-level graphics class table routines.
//
// This part of the class performs rendering to a device.  The TP member
// class performs maintenance of tables.
//
//=============================================================================




#include <sys.h>                        // SYS Object
#include <geosoft.h>                    // Geosoft

#include <col.h>                        // colours
#include <pp.h>                         // PP Object
#include <dd.h>                         // DD Object
#include <gs.h>                         // GS Object
#include <ra.h>                         // RA Object
#include <lpt.h>                        // LPT Object
#include <gfn.h>                        // GFN Object
#include <mp.h>                         // MP Object
#include <ap.h>                         // AP Object
#include <vv.h>                         // VV Object
#include "pp.ih"                // PP Internal Header

//-----------------------------------------------------------------------------

void SetAttName_PP(H_PP*   hPP,         // PP handle
                   short   numb,        // attribute number
                   AttN_PP attrn)       // attribute name
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the attribute name ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))  // Defensive Anyway
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		strcpy_GS(Attr.Name, attrn, sizeof (Attr.Name));
		Attr.Name[15] = '\0';
		SetE_VV(hPP->hVVAttribs, numb, &Attr);
	}

	return;


}


//-----------------------------------------------------------------------------

void GetAttName_PP(H_PP*   hPP,         // PP handle
                   short   numb,        // attribute number
                   AttN_PP attrn)       // attribute name
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Get the attribute name ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))  // Defensive Anyway
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		strcpy_GS(attrn, Attr.Name, sizeof (AttN_PP));
	}

	return;


}


//-----------------------------------------------------------------------------

short GetAttN_PP(H_PP*   hPP,           // PP handle
                 AttN_PP attrn)         // attribute name
{
	DEBUG_STACK;

	short  i;
	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Find the Attribute ---

	for (i = 0; i < hPP->sNAttribs; i++)
	{
		GetE_VV(hPP->hVVAttribs, i, &Attr);
		if (!_stricmp(attrn, Attr.Name))
			return (i);
	}
	return (-1);


}


//-----------------------------------------------------------------------------

short SetAttT_PP(H_PP*   hPP,           // PP handle
                 short   numb,          // Table entry
                 Att_PP* attr)          // Attribute
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP && attr);                 // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the attribute (defensive Anyway) ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		SetE_VV(hPP->hVVAttribs, numb, attr);
		return (0);
	}
	else
		return (1);


}

//-----------------------------------------------------------------------------

short GetAttT_PP(H_PP*   hPP,           // PP handle
                 short   numb,          // Table entry
                 Att_PP* attr)          // Attribute
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP && attr);                 // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the attribute (defensive Anyway) ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, attr);
		return (0);
	}
	else
	{
		GetE_VV(hPP->hVVAttribs, 0, attr);
		return (1);
	}


}

//-----------------------------------------------------------------------------

short SetAtt_PP(H_PP* hPP,      // PP handle
                short numb)     // Table entry
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the current attribute (defensive anyway) ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		SetCurAtt_PP(hPP, &Attr);
		return (0);
	}
	else
		return (1);


}


//-----------------------------------------------------------------------------

short SetCurAtt_PP(H_PP*   hPP,         // PP handle
                   Att_PP* attr)        // Attribute
{
	DEBUG_STACK;

	Rgb rgb;


	// --- Validate Parameters ---

	ASSERT(hPP && attr);                 // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Set the Attribute ---

	hPP->CurAttrib = *attr;


	// --- Reflect Changes ---

	Color_DD(hPP->hDD, hPP->CurAttrib.Lcol);
	if (!hPP->sDoExtents)
		Thick_DD(hPP->hDD, hPP->CurAttrib.Lthk);
	if (sSelect_GFN(hPP->hGFN, hPP->CurAttrib.Font))
		if (sSelect_GFN(hPP->hGFN, "default"))
			return (1);


	// --- Retrieve the line pattern ---

	if (hPP->fDashDD)
	{
		sLineStyle_DD(hPP->hDD, (long) hPP->CurAttrib.Lpat, (double) hPP->CurAttrib.Lptc / 1000.0);
		ASSERT(hPP->pLpt == NULL);
	}
	else
	{
		if (hPP->CurAttrib.Lpat > 1)
		{
			hPP->pLpt = Query_LPT(hPP->hLPT, hPP->CurAttrib.Lpat);
			if ((hPP->pLpt != NULL) && (hPP->pLpt->lItems <= 0))
				hPP->pLpt = NULL;
		}
		else
			hPP->pLpt = NULL;
	}

	// --- Check the fill Color ---

	GetColor(&hPP->CurAttrib.Fcol, (ColorMix*) &rgb, C_RGB);
	hPP->sNoFill = (short) ((rgb.byR == 255) && (rgb.byG == 255) && (rgb.byB == 255));
	GetColor(&hPP->CurAttrib.Lcol, (ColorMix*) &rgb, C_RGB);
	hPP->sNoDraw = (short) ((rgb.byR == 255) && (rgb.byG == 255) && (rgb.byB == 255));

	return (0);


}

//-----------------------------------------------------------------------------

short AddAttr_PP(H_PP*   hPP,           // PP handle
                 Att_PP* pAttr)         // Attribute Data to Add
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP && pAttr);                // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Rellocate the VV object for one more attribute ---

	if (SetLn_VV(hPP->hVVAttribs, hPP->sNAttribs + 1))
		return (-1);


	// --- Add the Attribute ---

	SetE_VV(hPP->hVVAttribs, hPP->sNAttribs, pAttr);
	return (hPP->sNAttribs++);


}

//-----------------------------------------------------------------------------

short QryAttr_PP(H_PP* hPP)   // PP handle
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- return the Current Number of Attributes ---

	return (hPP->sNAttribs);


}


//-----------------------------------------------------------------------------

void SetLcolT_PP(H_PP* hPP, short numb, GeoColor col)
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Set the color (defensive anyway) ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		Attr.Lcol = col;
		SetE_VV(hPP->hVVAttribs, numb, &Attr);
	}

	return;


}

//-----------------------------------------------------------------------------

void SetLthkT_PP(H_PP* hPP, short numb, long thk)     // in microns
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(thk >= 0);                    // Thicknes cannot be negative

	// --- Set line thickness (Defensive Anyway) ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		Attr.Lthk = thk;
		SetE_VV(hPP->hVVAttribs, numb, &Attr);
	}

	return;


}

//-----------------------------------------------------------------------------

void SetLpatT_PP(H_PP* hPP, short numb, short pat)
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(pat >= 0);                    // Line pattern must be positive

	// --- Set the line pattern ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		if (pat == 1)
			pat = 0;
		Attr.Lpat = pat;
		SetE_VV(hPP->hVVAttribs, numb, &Attr);
	}
	return;

}

//-----------------------------------------------------------------------------

void SetLptcT_PP(H_PP* hPP, short numb, long ptc)
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(ptc >= 0);                    // Line pitch must be positive

	// --- Set the line pitch ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		Attr.Lptc = ptc;
		SetE_VV(hPP->hVVAttribs, numb, &Attr);
	}
	return;

}

//-----------------------------------------------------------------------------

void SetFcolT_PP(H_PP* hPP, short numb, GeoColor col)
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the Fill Color ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		Attr.Fcol = col;
		SetE_VV(hPP->hVVAttribs, numb, &Attr);
	}
	return;

}

//-----------------------------------------------------------------------------

void SetTstyT_PP(H_PP* hPP, short numb, Txt_PP sty)
{
	DEBUG_STACK;

	Att_PP Attr;


	// --- Validate Parameters ---

	ASSERT(hPP);                                 // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);            // Was it really a PP object ?
	ASSERT((sty.sl > 0) && (sty.sl < 89.0));     // Must be a valid slant

	// --- Set the text style ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		Attr.Tsty = sty;
		SetE_VV(hPP->hVVAttribs, numb, &Attr);
	}
	return;

}

//-----------------------------------------------------------------------------

void SetFontT_PP(H_PP* hPP, short numb, Font_PP font)
{
	DEBUG_STACK;

	Att_PP Attr;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the Font to Use ---

	if ((numb >= 0) && (numb < hPP->sNAttribs))
	{
		GetE_VV(hPP->hVVAttribs, numb, &Attr);
		strcpy_GS(Attr.Font, font, sizeof (Attr.Font));
		SetE_VV(hPP->hVVAttribs, numb, &Attr);
	}
	return;

}


//-----------------------------------------------------------------------------


void SetLcol_PP(H_PP* hPP, GeoColor col)
{
	DEBUG_STACK;

	Rgb rgb;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set current color ---

	hPP->CurAttrib.Lcol = col;
	Color_DD(hPP->hDD, col);
	GetColor(&hPP->CurAttrib.Lcol, (ColorMix*) &rgb, C_RGB);
	hPP->sNoDraw = (short) ((rgb.byR == 255) && (rgb.byG == 255) && (rgb.byB == 255));
	return;

}

//-----------------------------------------------------------------------------

void SetLthk_PP(H_PP* hPP, long thk)
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(thk >= 0);                    // Line thickness is positive

	// --- Set line thickness ---

	hPP->CurAttrib.Lthk = thk;
	if (!hPP->sDoExtents)
		Thick_DD(hPP->hDD, thk);
	return;

}

//-----------------------------------------------------------------------------

void SetLpat_PP(H_PP* hPP, short pat)
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(pat >= 0);                    // Line pattern is positive


	// --- Retrive the line pattern ---

	if (pat == 1)
		pat = 0;
	hPP->CurAttrib.Lpat = pat;

	if (hPP->fDashDD)
	{
		sLineStyle_DD(hPP->hDD, (long) hPP->CurAttrib.Lpat, (double) hPP->CurAttrib.Lptc / 1000.0);
		ASSERT(hPP->pLpt == NULL);
	}
	else
	{
		if (pat > 0)
		{
			// --- Retrieve the line pattern ---

			hPP->pLpt = Query_LPT(hPP->hLPT, hPP->CurAttrib.Lpat);

			// --- Is this a valid line pattern ? ---

			if ((hPP->pLpt != NULL) && (hPP->pLpt->lItems <= 0))
				hPP->pLpt = NULL;

		}
		else
			hPP->pLpt = NULL;
	}
	return;

}

//-----------------------------------------------------------------------------

void SetLptc_PP(H_PP* hPP, long ptc)
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT(ptc >= 0);                    // Line Pitch is positive


	// --- Set the line pitch ---

	hPP->CurAttrib.Lptc = ptc;
	if (hPP->fDashDD)
	{
		sLineStyle_DD(hPP->hDD, (long) hPP->CurAttrib.Lpat, (double) hPP->CurAttrib.Lptc / 1000.0);
		ASSERT(hPP->pLpt == NULL);
	}

	return;

}

//-----------------------------------------------------------------------------

void SetFcol_PP(H_PP* hPP, GeoColor col)
{
	DEBUG_STACK;

	Rgb rgb;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the fill color ---

	hPP->CurAttrib.Fcol = col;
	GetColor(&hPP->CurAttrib.Fcol, (ColorMix*) &rgb, C_RGB);
	hPP->sNoFill = (short) ((rgb.byR == 255) && (rgb.byG == 255) && (rgb.byB == 255));
	return;

}

//-----------------------------------------------------------------------------

void SetTsty_PP(H_PP* hPP, Txt_PP sty)
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the text style ---

	hPP->CurAttrib.Tsty = sty;
	return;

}

//-----------------------------------------------------------------------------

short SetFont_PP(H_PP* hPP, Font_PP font)
{
	DEBUG_STACK;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the font to use ---

	strcpy_GS(hPP->CurAttrib.Font, font, sizeof (hPP->CurAttrib.Font));
	if (sSelect_GFN(hPP->hGFN, font))
		if (sSelect_GFN(hPP->hGFN, "default"))
			return (1);

	return (0);


}
