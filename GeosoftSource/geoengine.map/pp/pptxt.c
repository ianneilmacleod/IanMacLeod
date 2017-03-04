//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
// PPTXT.c
//
// Geosoft high-level graphics class.
//
// This part of the PP class handles the text manipulation.
//
//=============================================================================




#include <math.h>
#include <ctype.h>

#include <sys.h>                // SYS Header
#include <geosoft.h>            // Geosoft

#include <ap.h>                 // AP HEader
#include <col.h>                // colours
#include <gs.h>                 // GX Header
#include <pp.h>                 // PP Header
#include <dd.h>                 // DD Header
#include <mp.h>                 // MP Header
#include <gfn.h>                // GFN Object
#include "pp.ih"                // PP Internal Header
#include "../gfn/gfn.ih"


//-----------------------------------------------------------------------------

void Symb_PP(H_PP*    hPP,      // PP handle
             short    symb,     // symbol to plot
             double   scale,    // scale relative to current text attribute
             double   rangle,   // rotation angle
             Point_PP loc)      // symbol location
{
	short    sCheckErr = 0;

	double   rotate;
	text_ob  tinfo;

	VOL_STR* pszSymb;
	VOL_STR  szSymb[2] = { 0 };

	DEBUG_STACK;

	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// Valid symbol to plot

	if ((symb < 0) || (symb > 255))
		return;


	// --- Make sure scale is valid ---

	if (scale == 0.0)
		return;                   // Invalid scaling

	tinfo.height = (double) hPP->CurAttrib.Tsty.ht * scale;
	tinfo.width = (double) hPP->CurAttrib.Tsty.wd * scale;
	tinfo.space = (double) hPP->CurAttrib.Tsty.sp;
	tinfo.flags = 0;


	// --- Determine the text slant ---

	if (hPP->CurAttrib.Tsty.sl > 89.0)
		tinfo.slant = 0.0;
	else
		tinfo.slant = tan((double) hPP->CurAttrib.Tsty.sl / DEG_TO_GRAD);


	// --- Setup the rotation ---

	rotate = rangle / DEG_TO_GRAD;
	tinfo.sin = sin(rotate);
	tinfo.cos = cos(rotate);
	tinfo.xs = tinfo.width / STROKE_RES;
	tinfo.ys = tinfo.height / STROKE_RES;


	// --- Setup Location ---

	tinfo.orig = Trans_PP(hPP, loc);


	// --- Plot the symbol ---

	szSymb[0] = (char) symb;
	pszSymb = szSymb;
	IChar_PP(hPP, (CON_STR**) &pszSymb, &tinfo);         // Plot the symbol

	return;

}


//-----------------------------------------------------------------------------

void PText_PP(H_PP*    hPP,     // PP handle
              CON_STR* text,    // text to plot
              TLoc_PP  loc,     // text location
              short    just)    // justification
{
	DEBUG_STACK;

	short    sCheckErr = 0;
	VOL_STR  txt[255];
	double   rotate;
	double   dx, dy;
	double   slen;
	double   ratio;
	Point_DD center;
	Point_DD loc1, loc2;
	text_ob  tinfo;
	long     lRef;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Valid justification mode ---

	if ((just < -1) || (just > 8))
		return;


	// --- Get a copy of the current drawing information ---

	tinfo.height = (double) hPP->CurAttrib.Tsty.ht;
	tinfo.width = (double) hPP->CurAttrib.Tsty.wd;
	tinfo.space = (double) hPP->CurAttrib.Tsty.sp;
	loc1 = Trans_PP(hPP, loc.p1);

	// --- Determine the text slant ---

	if (fabs(hPP->CurAttrib.Tsty.sl) > 89.0)
		tinfo.slant = 0.0;
	else
		tinfo.slant = tan((double) hPP->CurAttrib.Tsty.sl / DEG_TO_GRAD);


	// --- Filter the text for special characters ---

	strcpy_GS(txt, text, sizeof (txt));
	tinfo.flags = IFilter_PP(txt);
	if (!strlen(txt))
		return;
	slen = TLen_PP(hPP, txt, 0); // No filtering


	// --- Deal with non-base justified text ---

	if (just <= PMF_PP_ALL_CENTERED)
	{


		// --- Generate Rotation for text ---

		rotate = loc.d.angle / DEG_TO_GRAD;

		// --- DD text ? ---

		if (hPP->fTextDD)
		{
			tinfo.orig = loc1;
			switch (just)
			{
				case PMF_PP_LEFT:
					lRef = 0;
					break;

				case PMF_PP_CENTERED:
					lRef = 1;
					break;

				case PMF_PP_RIGHT:
					lRef = 2;
					break;

				case PMF_PP_ALL_CENTERED:
					lRef = 4;
					break;
			}

		}
		else
		{

			tinfo.sin = sin(rotate);
			tinfo.cos = cos(rotate);

			switch (just)
			{

				case PMF_PP_LEFT:

					tinfo.orig = loc1;
					lRef = 0;
					break;

				case PMF_PP_CENTERED:

					tinfo.orig.x = loc1.x - (long) ((slen * tinfo.cos) / 2.0);
					tinfo.orig.y = loc1.y - (long) ((slen * tinfo.sin) / 2.0);
					break;

				case PMF_PP_RIGHT:

					tinfo.orig.x = loc1.x - (long) (slen * tinfo.cos);
					tinfo.orig.y = loc1.y - (long) (slen * tinfo.sin);
					break;

				case PMF_PP_ALL_CENTERED:

					tinfo.orig.x = loc1.x - (long) (((slen * tinfo.cos) - (tinfo.height * tinfo.sin)) / 2.0);
					tinfo.orig.y = loc1.y - (long) (((slen * tinfo.sin) + (tinfo.height * tinfo.cos)) / 2.0);
					break;
			}
		}

	}
	else
	{


		// --- Based Justified Text ---

		loc2 = Trans_PP(hPP, loc.d.p2);
		dx = (double) (loc2.x - loc1.x);  // Delta in X
		dy = (double) (loc2.y - loc1.y);  // Delta in Y


		// --- Determine base center point ---

		center.x = loc1.x + (long) (dx / 2.0);
		center.y = loc1.y + (long) (dy / 2.0);


		// --- Check to make sure we don't have a vertical line ---

		if (fabs(dx) < 0.0001)
		{
			// --- Vertical line, setup a standard rotation ---

			if (loc1.y > loc2.y)
				rotate = -M_PI_2;
			else
				rotate = M_PI_2;

		}
		else
		{

			// --- Determine actual rotation of text ---

			rotate = atan(dy / dx);
			if ((rotate > 0.0) && (dx < 0.0))
				rotate -= PI;
			else if ((rotate < 0.0) && (dx < 0.0))
				rotate += PI;
		}


		// --- Determine sin/cos ---

		tinfo.sin = sin(rotate);
		tinfo.cos = cos(rotate);


		// --- Determine scaling ratio ---

		ratio = sqrt(dx * dx + dy * dy) / slen;


		if (hPP->fTextDD)
		{

			switch (just)
			{

				case PMF_PP_BASE_LEFT:

					tinfo.orig = loc1;
					lRef = 0;
					break;

				case PMF_PP_BASE_CENTERED:

					tinfo.orig = center;
					lRef = 1;
					break;

				case PMF_PP_BASE_RIGHT:

					tinfo.orig = loc2;
					lRef = 2;
					break;

				case PMF_PP_BASE_ALL_CENTERED:

					tinfo.height = GS_MIN(tinfo.space, tinfo.height);
					tinfo.orig.x = center.x - (long) ((tinfo.height * tinfo.sin) / 2.0);
					tinfo.orig.y = center.y + (long) ((tinfo.height * tinfo.cos) / 2.0);
					lRef = 4;
					break;

				case PMF_PP_BASE_ALIGNED_HEIGHT:   // Variable height

					tinfo.height *= ratio;
					tinfo.width *= ratio;
					tinfo.space *= ratio;
					tinfo.orig = loc1;
					lRef = 0;
					break;

				case PMF_PP_BASE_ALIGNED_WIDTH:    // Fixed height

					tinfo.width *= ratio;
					tinfo.space *= ratio;
					tinfo.orig = loc1;
					lRef = 0;
			}

		}
		else
		{

			switch (just)
			{

				case PMF_PP_BASE_LEFT:

					tinfo.orig = loc1;
					break;

				case PMF_PP_BASE_CENTERED:

					tinfo.orig.x = center.x - (long) ((slen * tinfo.cos) / 2.0);
					tinfo.orig.y = center.y - (long) ((slen * tinfo.sin) / 2.0);
					break;

				case PMF_PP_BASE_RIGHT:

					tinfo.orig.x = loc2.x - (long) (slen * tinfo.cos);
					tinfo.orig.y = loc2.y - (long) (slen * tinfo.sin);
					break;

				case PMF_PP_BASE_ALL_CENTERED:

					tinfo.orig.x = center.x - (long) (((slen * tinfo.cos) - (tinfo.height * tinfo.sin)) / 2.0);
					tinfo.orig.y = center.y - (long) (((slen * tinfo.sin) + (tinfo.height * tinfo.cos)) / 2.0);
					break;

				case PMF_PP_BASE_ALIGNED_HEIGHT:   // Variable height

					tinfo.height *= ratio;
					tinfo.width *= ratio;
					tinfo.space *= ratio;
					tinfo.orig = loc1;
					break;

				case PMF_PP_BASE_ALIGNED_WIDTH:    // Fixed height

					tinfo.width *= ratio;
					tinfo.space *= ratio;
					tinfo.orig = loc1;
			}
		}
	}

	// --- Plot the text ---

	if (hPP->fTextDD)
	{

		// --- use DD rendering, set text parameters ---

		tinfo.height = GS_MIN(tinfo.space, tinfo.height);
		sSetText_DD(hPP->hDD, tinfo.height / 1000.0, (double) hPP->CurAttrib.Lthk / 1000.0, AnyColor(hPP->CurAttrib.Lcol), lRef, rotate * DEG_TO_GRAD, hPP->CurAttrib.Font, (bool) (hPP->CurAttrib.Tsty.sl > 0.0));


		// --- plot the text ---

		sText_DD(hPP->hDD, &tinfo.orig, text);


	}
	else
	{

		// --- use stroke rendering ---

		tinfo.xs = tinfo.width / STROKE_RES;
		tinfo.ys = tinfo.height / STROKE_RES;
		IText_PP(hPP, txt, &tinfo);

		if (just == PMF_PP_CENTERED)
			Move_PP(hPP, loc.p1);

	}

	return;

}


//-----------------------------------------------------------------------------

long TLen_PP(H_PP*    hPP,      // PP Handle
             CON_STR* str,      // string to measure
             short    filter)   // Filter the string before measuring ?
{
	DEBUG_STACK;

	short     sCheckErr = 0;
	long      length;
	VOL_STR   st[255];
	CON_STR*  p;
	long      gap;
	GFN_CHAR* ch;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Filter out special characters ---

	if (filter)
	{
		strcpy_GS(st, str, sizeof (st));
		IFilter_PP(st);
		p = st;
	}
	else
		p = str;


	// --- Get the length of each character ---

	length = 0;
	gap = hPP->CurAttrib.Tsty.sp - hPP->CurAttrib.Tsty.wd;
	while (*p != '\0')
	{
		// --- Get the character the evaluate its length ---

		ch = pTextCharGFN(hPP->hGFN, &p);
		if (ch != NULL)
			length += (long) ((double) (ch->sRight - ch->sLeft) / STROKE_RES * (double) hPP->CurAttrib.Tsty.wd) + gap;
	}
	if (length > 0)
		length -= gap;


	// --- Done ---

	return (length);

}

//-----------------------------------------------------------------------------

#define PSymb_PP(A,B,C,D,E,F) IPSymb_PP((A),(B),(C),(D),(E),(F),CONV_DOUBLE);


//-----------------------------------------------------------------------------
//                             INTERNAL FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

Point_DD ILocate_PP(short    x,         // X value of stroke
                    short    y,         // Y value of stroke
                    text_ob* info)      // Information on how to draw the character
{
	DEBUG_STACK;

	short    sCheckErr = 0;
	Point_PP pt;
	Point_DD res;



	// --- Validate Parameters ---

	ASSERT(info);                // Object must be real


	// --- Calculate scaled x and y ---

	pt.x = ((double) x + ((double) y * info->slant)) * info->xs;
	pt.y = (double) y * info->ys;

	// --- Calculate the rotated x and y ---

	res.x = info->orig.x + (long) ((pt.x * info->cos) - (pt.y * info->sin));
	res.y = info->orig.y + (long) ((pt.y * info->cos) + (pt.x * info->sin));


	// --- Done --

	return (res);

}



//-----------------------------------------------------------------------------

short IChar_PP(H_PP*     hPP,           // PP Handle
               CON_STR** pszStream,     // Character to plot
               text_ob*  tinfo)         // Text Rendering information
{
	DEBUG_STACK;

	short       sCheckErr = 0;
	short       i;
	GFN_CHAR*   ch;
	short       nstrks;
	GFN_STROKE* stp;
	GFN_STROKE  sp;
	short       allin;
	Point_DD*   buff;


	// --- Validate Parameters ---

	ASSERT(hPP && tinfo);                // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Get the character from the font ---

	ch = pTextCharGFN(hPP->hGFN, pszStream);
	if (!ch->sStrokes)
		return (ch->sRight - ch->sLeft);
	stp = ch->pStrokes;
	nstrks = 0;
	allin = 1;


	// --- Make a vector buffer on the stack ---

	buff = (Point_DD*) AllocTemp_MP(hPP->hMP, sizeof (Point_DD) * ch->sStrokes * 2);
	if (buff == NULL)
		return (0);            // No Character Drawn


	// --- Plot the character ---

	for (i = 0; i < ch->sStrokes; i++)
	{
		// --- Get Stroke Information ---

		sp = stp[i];

		// --- What kind of stroke is this ? ---

		if (sp.lTest == GFN_STROKE_UP)
		{
			// --- Any vectors to finish ? ---

			if (nstrks > 1)
				IMVect_PP(hPP, buff, nstrks, allin, 0);

			nstrks = 0;
			allin = 1;

		}
		else if (sp.lTest == GFN_STROKE_FILL)
		{

			// --- Fill the vectors ---

			if (nstrks > 1)
			{
				// --- Do an Internal Fill ---

				Move_DD(hPP->hDD, buff[0]);
				if (!hPP->sNoFill)
					IFill_PP(hPP, buff, nstrks, allin);
				IMVect_PP(hPP, buff, nstrks, allin, 0);
			}
			nstrks = 0;
			allin = 1;

		}
		else if (sp.lTest != GFN_STROKE_END)
		{

			// --- Save the vector ---

			buff[nstrks] = ILocate_PP(sp.A.sX, sp.A.sY, tinfo);
			allin &= IVisible_PP(hPP, buff[nstrks]);
			nstrks++;
		}
	}

	// --- Draw the last vector if more than one stroke ---

	if (nstrks > 1)
	{
		IMVect_PP(hPP, buff, nstrks, allin, 0);
		nstrks = 0;
		allin = 1;
	}


	// --- Destroy the buffer on the stack ---

	Free_MP(hPP->hMP, buff);

	// --- Return the width ---

	return (ch->sRight - ch->sLeft);
}


//-----------------------------------------------------------------------------

void IText_PP(H_PP*    hPP,     // PP Object
              CON_STR* txt,     // Text to draw
              text_ob* tinfo)   // Location to plot the text
{
	DEBUG_STACK;

	short    sCheckErr = 0;

	CON_STR* p;

	short    width;
	double   space;
	double   gap;
	Point_DD p1, p2, pt;
	Point_DD begloc;


	// --- Validate Parameters ---

	ASSERT(hPP && txt && tinfo);         // Object must be real
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Save Begining Location ---

	begloc = tinfo->orig;


	// --- Draw each character ---

	gap = tinfo->space - tinfo->width;
	p = txt;
	while (*p != '\0')
	{
		// --- Draw the character ---

		width = IChar_PP(hPP, &p, tinfo);


		// --- Move to start of next character ---

		space = ((double) width * tinfo->width / STROKE_RES) + gap;
		tinfo->orig.x += (long) (space * tinfo->cos);
		tinfo->orig.y += (long) (space * tinfo->sin);
	}
	pt = tinfo->orig;


	// --- Draw Underscore if there ---

	if (tinfo->flags & TXT_UNDER_SCORE)
	{
		// --- Draw a Line ---

		tinfo->orig = begloc;
		p1 = ILocate_PP(0, -10, tinfo);

		tinfo->orig = pt;
		p2 = ILocate_PP(0, -10, tinfo);

		IVect_PP(hPP, p1, p2);
	}

	// --- Draw Overscore if there ---

	if (tinfo->flags & TXT_OVER_SCORE)
	{
		// --- Draw a Line ---

		tinfo->orig = begloc;
		p1 = ILocate_PP(0, 74, tinfo);

		tinfo->orig = pt;
		p2 = ILocate_PP(0, 74, tinfo);

		IVect_PP(hPP, p1, p2);
	}

}


//-----------------------------------------------------------------------------
// IPSymb:   Internal poly symbol rendering function that can use multiple
//           input types.
//
// Returns:  Nothing

void IPSymb_PP(H_PP*    hPP,    // PP handle
               short    symb,   // symbols to plot
               long     nsymbs, // number of symbols
               double   scale,  // scale relative to current text attribute
               double   rangle, // rotation angle
               CON_DAT* loc,    // symbol locations
               short    sType)  // Type of locations
{
	short           sCheckErr = 0;

	double          rotate;
	long            i;
	text_ob         tinfo;
	Point_PP        dpt;

	const Point_PP* dp;
	const float*    rp;

	VOL_STR*        pszSymb;
	VOL_STR         szSymb[2] = { 0 };
	bool         bDpt = false;

	DEBUG_STACK;

	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	if ((nsymbs <= 0) ||                 // Valid number of symbols
	    (scale <= 0.0))                  // Valid scale is non-zero
		return;

	rp = (const float*) loc;
	dp = (const Point_PP*) loc;
	tinfo.height = (double) hPP->CurAttrib.Tsty.ht * scale;
	tinfo.width = (double) hPP->CurAttrib.Tsty.wd * scale;
	tinfo.space = (double) hPP->CurAttrib.Tsty.sp;
	tinfo.orig = hPP->dloc;
	tinfo.flags = 0;


	// --- Determine the text slant ---

	if (hPP->CurAttrib.Tsty.sl > 89.0)
		tinfo.slant = 0.0;
	else
		tinfo.slant = tan((double) hPP->CurAttrib.Tsty.sl / DEG_TO_GRAD);


	// --- Setup the rotation ---

	rotate = rangle / DEG_TO_GRAD;
	tinfo.sin = sin(rotate);
	tinfo.cos = cos(rotate);
	tinfo.xs = tinfo.width / STROKE_RES;
	tinfo.ys = tinfo.height / STROKE_RES;


	// --- Plot the symbols ---

	for (i = 0; i < nsymbs; i++)
	{
		if (sType == CONV_DOUBLE)
			tinfo.orig = Trans_PP(hPP, dp[i]);
		else if (sType == CONV_FLOAT)
		{
			dpt.x = (double) rp[i * 2];
			dpt.y = (double) rp[(i * 2) + 1];
			tinfo.orig = Trans_PP(hPP, dpt);
			bDpt = true;        // We have set dpt
		}
		else
		{
			continue;              // Should not happen
		}
		szSymb[0] = (char) symb;
		pszSymb = szSymb;
		IChar_PP(hPP, (CON_STR**) &pszSymb, &tinfo);
	}


	// --- Reset Point ---

	if (bDpt)
		Move_PP(hPP, dpt);        // Go back to center of last symb

	return;


}

//-----------------------------------------------------------------------------
// FilTxt_PP:     Filters a text string for special codes such as:
//
//-----------------------------------------------------------------------------

short IFilter_PP(VOL_STR* text)       // Text to filter
{
	DEBUG_STACK;

	short    sCheckErr = 0;
	short    flags = 0;
	VOL_BYT* p, * q;



	ASSERT(text != NULL);


	// --- Setup pointers ---

	p = (VOL_BYT*) text;
	q = (VOL_BYT*) text;


	// --- Scan the string for special directives ---

	while (*p != '\0')
	{
		if ((*p != '%') || (*(p + 1) != '%'))
		{
			// --- Nothing Special ---

			if (q != p)
				*q = *p;
			p++;
			q++;

		}
		else
		{

			// --- Special characters ---

			p += 2;
			switch (toupper(*p))
			{
				case 'O':
					flags |= TXT_OVER_SCORE;
					p++;
					break;

				case 'U':
					flags |= TXT_UNDER_SCORE;
					p++;
					break;

				case 'D':
					*q = 31;
					p++;
					q++;
					break;

				case 'P':
					*q = 30;
					p++;
					q++;
					break;

				case 'C':
					*q = 29;
					p++;
					q++;
					break;

				case '%':
					*q = '%';
					p++;
					q++;
					break;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					*q = (VOL_BYT) ((*p - 48) * 100 + (*(p + 1) - 48) * 10 + (*(p + 2) - 48));
					p += 3;
					q++;
					break;
			}
		}
	}

	// --- Done ---

	*q = '\0';
	return (flags);

}
