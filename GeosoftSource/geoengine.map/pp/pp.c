//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
// PP.c
//
// Geosoft high-level graphics class.
//
// This part of the class performs rendering to a device.  The TP member
// class performs maintenance of tables.
//
//=============================================================================



#include <col.h>                // COL Utilities
#include <pp.h>                 // PP Header
#include <gfn.h>                // GFN Object
#include <vv.h>                 // VV Header
#include "pp.ih"                // PP Internal Header


//-----------------------------------------------------------------------------

H_PP* Creat_PP(H_AP*  hAP,      // AP handle
               H_DD*  hDD,      // handle to output device (DD)
               H_GFN* hGFN,     // handle to font object (GFN)
               H_LPT* hLPT)     // Handle to line pattern object (LPT)
{
	DEBUG_STACK;

	short sCheckErr = 0;

	H_PP* hPP = NULL;
	H_MP* hMP;
	I_DD  info;



	// --- Validate Parameters ---

	ASSERT(hAP);
	ASSERT(hDD);
	ASSERT(hGFN);
	ASSERT(hLPT);

	// --- Create the Memory for the Object Storage ---

	hMP = hGethMP_AP(hAP);
	hPP = (H_PP*) Alloc_MP(hMP, sizeof (H_PP));
	CHECK_ERR(hPP == NULL);

	// --- Initialize the Object ---

	hPP->obj_id = PMF_PP_ID;

	hPP->hAP = hAP;
	hPP->hDD = hDD;
	hPP->hMP = hMP;
	hPP->hGFN = hGFN;
	hPP->hLPT = hLPT;
	hPP->pEdit = NULL;

	hPP->fTextDD = false;
	hPP->fDashDD = false;

	hPP->sNoFill = 0;
	hPP->sNoDraw = 0;
	hPP->linacc = 0;
	hPP->pLpt = 0;

	hPP->sDoExtents = 0;
	hPP->sNAttribs = 1;
	hPP->hVVAttribs = NULL;
	hPP->pLpt = NULL;


	// --- Initialize the First Attribute ---

	InitColor(&hPP->CurAttrib.Lcol, C_KCMY, 255, 255, 255, 0);   // All Black
	hPP->CurAttrib.Lthk = 1;
	hPP->CurAttrib.Lptc = 10000;
	hPP->CurAttrib.Lpat = 0;
	InitColor(&hPP->CurAttrib.Fcol, C_KCMY, 0, 0, 0, 0);         // All White
	hPP->CurAttrib.Tsty.ht = 5000;
	hPP->CurAttrib.Tsty.wd = 5000;
	hPP->CurAttrib.Tsty.sp = 5000;
	hPP->CurAttrib.Tsty.sl = 0.0;
	strcpy(hPP->CurAttrib.Font, "DEFAULT");
	strcpy(hPP->CurAttrib.Name, "0");

	// --- Allocate the attribute storage ---

	hPP->hVVAttribs = hCreat_VV(hAP, -(short) (sizeof (Att_PP)), 1);
	CHECK_ERR(hPP->hVVAttribs == NULL);

	// --- Initialize the Attribute 0 ---

	SetE_VV(hPP->hVVAttribs, 0, &hPP->CurAttrib);

	// --- Get Device Maximums ---

	info.Short = 1;
	SetI_DD(hDD, DD_ON, &info);  // Turn Device On
	GetI_DD(hDD, DD_XSIZE, &info);
	hPP->dxmax = info.Long;
	GetI_DD(hDD, DD_YSIZE, &info);
	hPP->dymax = info.Long;

	// --- Setup the default Viewport ---

	hPP->clipll.x = 0;
	hPP->clipll.y = 0;
	hPP->clipur.x = hPP->dxmax;
	hPP->clipur.y = hPP->dymax;
	hPP->origin.x = 0.0;
	hPP->origin.y = 0.0;
	hPP->dloc.x = 0;
	hPP->dloc.y = 0;
	hPP->PMFOrigin = 0L;


	// --- Setup Default Device Configurations ---

	Color_DD(hPP->hDD, hPP->CurAttrib.Lcol);
	Thick_DD(hPP->hDD, hPP->CurAttrib.Lthk);
	if (sSelect_GFN(hPP->hGFN, hPP->CurAttrib.Font))
	{
		RegistErr_AP(hAP, 20903, modn);
		SetParmS_AP(hAP, "%1", hPP->CurAttrib.Font);
		info.Short = 0;
		SetI_DD(hDD, DD_ON, &info);      // Turn Device Off
		sCheckErr = 1;
		goto GS_EXIT;
	}


	// --- Setup Standard Scaling ---

	Unit_PP(hPP);

	// --- DD capabilities ---

	hPP->fTextDD = sTextSupport_DD(hDD) != 0;
	hPP->fDashDD = sDashSupport_DD(hDD) != 0;

GS_EXIT:

	if (sCheckErr)
	{
		if (hPP)
		{
			if (hPP->hVVAttribs)
				Destr_VV(hPP->hVVAttribs);
			Free_MP(hMP, hPP);
		}
		return (NULL);
	}

	// --- Done ---

	return (hPP);

}


//-----------------------------------------------------------------------------

void Destr_PP(H_PP* hPP)      // PP handle
{
	DEBUG_STACK;

	//short sCheckErr = 0;
	I_DD info;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Turn Device OFF ---

	info.Short = 0;
	SetI_DD(hPP->hDD, DD_ON, &info);     // Turn Device On


	// --- Destroy the Attribute tables ---

	Destr_VV(hPP->hVVAttribs);


	// --- Free the object ---

	Free_MP(hPP->hMP, hPP);

}


//-----------------------------------------------------------------------------

short Show_PP(H_PP* hPP)      // PP handle
{
	DEBUG_STACK;

	//short sCheckErr = 0;


	return (Show_DD(hPP->hDD));

}


//-----------------------------------------------------------------------------

short Clear_PP(H_PP* hPP)     // PP handle
{
	DEBUG_STACK;

	//short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Call the DD function ---

	return (Clear_DD(hPP->hDD));

}


//-----------------------------------------------------------------------------

short Flush_PP(H_PP* hPP)     // PP handle
{
	DEBUG_STACK;

	//short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Call the DD function ---

	return (Flush_DD(hPP->hDD));

}


//-----------------------------------------------------------------------------

short GetI_PP(H_PP* hPP,        // PP handle
              short info,       // desired info from PMF_PP_ list of defines.
              I_PP* data)       // info union
{
	DEBUG_STACK;

	//short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Return the data ---

	return (0);

}


//-----------------------------------------------------------------------------

short SetI_PP(H_PP* hPP,        // PP handle
              short info,       // desired info from PMF_PP_ list of defines
              I_PP* data)       // info union
{
	DEBUG_STACK;

	//short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Do something ---

	return (0);

}


//-----------------------------------------------------------------------------

short SetMDF_PP(H_PP*  hPP,     // PP handle
                MDF_PP info)    // MDF info
{
	DEBUG_STACK;

	//short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Do something ---

	return (0);

}



//=============================================================================
//
// viewports/scaling
//
//=============================================================================


//-----------------------------------------------------------------------------

Point_DD Trans_PP(H_PP*    hPP, // PP handle
                  Point_PP pt)  // point in user space
{
	DEBUG_STACK;

	//short sCheckErr = 0;
	Point_DD p;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Convert to Device Coordinates ---

	pt.x += hPP->origin.x;
	pt.y += hPP->origin.y;
	p.x = (long) ((pt.x * hPP->matrix[0][0]) + (pt.y * hPP->matrix[1][0]));
	p.y = (long) ((pt.x * hPP->matrix[0][1]) + (pt.y * hPP->matrix[1][1]));


	// --- Make sure there was not over/under flow ---

#ifdef DEBUG_CODE
	{
		double x1, x2;

		x1 = (pt.x * hPP->matrix[0][0]) + (pt.y * hPP->matrix[1][0]);
		x2 = (pt.x * hPP->matrix[0][1]) + (pt.y * hPP->matrix[1][1]);
		if ((x1 < MIN_LONG) || (x1 > MAX_LONG) || (x2 < MIN_LONG) || (x2 > MAX_LONG))
		{
			RegistErr_AP(hPP->hAP, 20905, modn);
			SetParmR_AP(hPP->hAP, "%1", x1);
			SetParmR_AP(hPP->hAP, "%2", x2);
			if (x1 < MIN_LONG)
				x1 = MIN_LONG + 10000.0;
			if (x1 > MAX_LONG)
				x1 = MAX_LONG - 10000.0;
			if (x2 < MIN_LONG)
				x2 = MIN_LONG + 10000.0;
			if (x2 > MAX_LONG)
				x2 = MAX_LONG - 10000.0;
			p.x = (long) x1;
			p.y = (long) x2;
		}
	}
#endif

	// --- Done ---

	return (p);

}

//-----------------------------------------------------------------------------

void Unit_PP(H_PP* hPP)       // PP handle
{
	DEBUG_STACK;

	//short sCheckErr = 0;
	short i;
	I_DD  info;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Get Device Maximums ---

	GetI_DD(hPP->hDD, DD_XSIZE, &info);
	hPP->dxmax = info.Long;
	GetI_DD(hPP->hDD, DD_YSIZE, &info);
	hPP->dymax = info.Long;


	// --- Reset the matrix to default values ---

	for (i = 0; i < 9; i++)
		hPP->matrix[i / 3][i % 3] = 0.0;
	hPP->matrix[0][0] = 1000.0;  // 1 user unit - 1 millimeter
	hPP->matrix[1][1] = 1000.0;  // 1 user unit - 1 millimeter
	hPP->matrix[2][2] = 1000.0;  // 1 user unit - 1 millimeter

	// --- Set the unit window ---

	hPP->window.ll.x = 0.0;
	hPP->window.ll.y = 0.0;
	hPP->window.ur.x = (double) hPP->dxmax;
	hPP->window.ur.y = (double) hPP->dymax;

}


//-----------------------------------------------------------------------------

void SetVP_PP(H_PP*  hPP,       // PP handle
              Box_PP box)       // viewport box
{
	DEBUG_STACK;

	//short sCheckErr = 0;
	Point_DD ll, ur;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Set the window info ---

	hPP->window = box;


	// --- Change the clipping window ---

	ll = Trans_PP(hPP, box.ll);
	ur = Trans_PP(hPP, box.ur);
	ISetClip_PP(hPP, ll, ur);

}


//-----------------------------------------------------------------------------

void TransVP_PP(H_PP*    hPP,           // PP handle
                Point_PP origin,        // viewport origin in user units
                Point_PP scales,        // X and Y scale factors to millimetres
                double   rotate)        // rotation angle
{
	DEBUG_STACK;

	//short sCheckErr = 0;
	double   m1[3][3];
	double   m2[3][3];
	double   m3[3][3];
	double   cosa, sina;
	short    i;
	Point_DD pt;
	Point_PP nt;

	// --- Validate Parameters ---

	ASSERT(hPP);                                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);                    // Was it really a PP object ?
	ASSERT((scales.x > 0.0) && (scales.y > 0.0));        // Must be a positive number
	ASSERT((rotate > -360.0) && (rotate < 360.0));       // Must be a valid degree



	// --- Change the origin ---

	hPP->origin = origin;
	pt = Trans_PP(hPP, origin);


	// --- Change the location of the window ---

	hPP->window.ll.x += pt.x;
	hPP->window.ll.y += pt.y;
	hPP->window.ur.x += pt.x;
	hPP->window.ur.y += pt.y;


	// --- Initialize the matrixes ---

	for (i = 0; i < 9; i++)
	{
		if (i / 3 == i % 3)
		{
			m1[i / 3][i % 3] = 1.0;
			m2[i / 3][i % 3] = 1.0;
			m3[i / 3][i % 3] = 1.0;
		}
		else
		{
			m1[i / 3][i % 3] = 0.0;
			m2[i / 3][i % 3] = 0.0;
			m3[i / 3][i % 3] = 0.0;
		}
	}


	//--- Setup the first conversion matrix ---

	m1[0][0] *= scales.x;
	m1[1][1] *= scales.y;


	// --- Setup a rotation matrix ---

	if (rotate != 0.0)
	{
		// --- Get rotation values ---

		cosa = cos(rotate / DEG_TO_GRAD);
		sina = sin(rotate / DEG_TO_GRAD);


		// --- Setup the temporary matrix for a rotation ---

		m2[0][0] = cosa;
		m2[1][0] = -sina;
		m2[0][1] = sina;
		m2[1][1] = cosa;
	}

	// --- Generate the complete change matrix into m3 ---

	IMatrix_PP(m1, m2, m3);


	// --- Multiply the current matrix with change matrix ---

	IMatrix_PP(m3, hPP->matrix, m1);
	for (i = 0; i < 9; i++)
		hPP->matrix[i / 3][i % 3] = m1[i / 3][i % 3];


	// --- Rescale the window size ---

	nt.x = (hPP->window.ll.x * m3[0][0]) + (hPP->window.ll.y * m3[1][0]);
	nt.y = (hPP->window.ll.x * m3[0][1]) + (hPP->window.ll.y * m3[1][1]);
	hPP->window.ll = nt;

	nt.x = (hPP->window.ur.x * m3[0][0]) + (hPP->window.ur.y * m3[1][0]);
	nt.y = (hPP->window.ur.x * m3[0][1]) + (hPP->window.ur.y * m3[1][1]);
	hPP->window.ur = nt;


}



//-----------------------------------------------------------------------------

void Wind_PP(H_PP*  hPP,        // PP handle
             Box_PP wind)       // user coordinates to apply to the view port
{
	DEBUG_STACK;

	//short sCheckErr = 0;
	Point_PP scale;
	Point_PP origin;



	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Determine scaling ---

	scale.x = ((hPP->window.ur.x - hPP->window.ll.x) / (wind.ur.x - wind.ll.x)) / 1000.0;
	scale.y = ((hPP->window.ur.y - hPP->window.ll.y) / (wind.ur.y - wind.ll.y)) / 1000.0;
	origin.x = -wind.ll.x * scale.x;
	origin.y = -wind.ll.y * scale.y;


	// --- Translate the origin and change the scaling ---

	TransVP_PP(hPP, origin, scale, 0.0);

}


//-----------------------------------------------------------------------------
//                             INTERNAL FUNCTIONS
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

short IVisible_PP(H_PP*    hPP, // PP Object
                  Point_DD pt)  // Point to check
{
	DEBUG_STACK;

	//short sCheckErr = 0;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Check to see if it is in view ---

	return ((short) (hPP->sDoExtents || ((pt.x >= hPP->clipll.x) && (pt.x <= hPP->clipur.x) && (pt.y >= hPP->clipll.y) && (pt.y <= hPP->clipur.y))));

}


//-----------------------------------------------------------------------------

void ISetClip_PP(H_PP*    hPP,  // PP Object
                 Point_DD ll,   // Lower Left point
                 Point_DD ur)   // Upper right point to clip
{
	DEBUG_STACK;

	//short sCheckErr = 0;


	// --- Change the clipping window ---

	hPP->clipll = ll;
	hPP->clipur = ur;

}


//-----------------------------------------------------------------------------

void IMatrix_PP(double m1[3][3],        // Matrix 1
                double m2[3][3],        // Matrix 2
                double rs[3][3])        // Result
{
	DEBUG_STACK;

	//short sCheckErr = 0;
	short i, j, k;


	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
		{
			rs[i][j] = 0.0;
			for (k = 0; k < 3; k++)
				rs[i][j] += m1[k][j] * m2[i][k];
		}

}

//------------------------------------------------------------------------
