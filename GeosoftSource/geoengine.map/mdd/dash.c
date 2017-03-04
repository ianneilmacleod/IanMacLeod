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
#include <dd.h>
#include <pmf.h>
#include <map.h>
#include <pply.h>
#include "mdd.ih"


//-----------------------------------------------------------------------------

short sDashSupport_DD(H_DD* hDD)
{
	short sCheckErr = 0;

	return (1);
}


//-------------------------------------------------------------------------

GS_EXPORT short sLineStyle_DD(H_DD*  hDD,       // DD
                              long   lStyle,    // Line Style
                              double dPitch)    // Pitch in mm
{
	DEBUG_STACK;

	short sCheckErr = 0;


	if (lStyle > 0)
		lStyle--;
	return (sLineStyle_MVIEW(hDD->hMVIEW, lStyle, dPitch * dScaleMM_MVIEW(hDD->hMVIEW)));

}


//-----------------------------------------------------------------------------
