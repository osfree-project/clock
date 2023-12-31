/*
 *  Clock (winclock.c)
 *
 *  Copyright 1998 by Marcel Baur <mbaur@g26.ethz.ch>
 *
 *  This file is based on  rolex.c  by Jim Peterson.
 *
 *  I just managed to move the relevant parts into the Clock application
 *  and made it look like the original Windows one. You can find the original
 *  rolex.c in the wine /libtest directory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

// C lib headers
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

// Windows headers
#include "windows.h"

// Application headers
#include "winclock.h"
#include "main.h"

#define M_PI 3.1415926535 


typedef struct
{
    POINT Start;
    POINT End;
    POINT p2;
    POINT p3;
    POINT p4;
} HandData;

static HandData HourHand, MinuteHand, SecondHand;

static void DrawTicks(HDC dc, const POINT* centre, int radius)
{
    int t;
    HPEN oldhPen, hPen;
	HBRUSH oldhBrush, hBrush;
	int hourWidth=0.09*radius;

    /* Minute divisions */
    if (radius>64)
    {
        hPen=CreatePen(PS_SOLID, 2, TickColor);
        oldhPen=SelectObject(dc, hPen);
        for(t=0; t<60; t++) {
            MoveToEx(dc,
                     centre->x + sin(t*M_PI/30)*0.95*radius,
                     centre->y - cos(t*M_PI/30)*0.95*radius,
                     NULL);
	    LineTo(dc,
		   centre->x + sin(t*M_PI/30)*0.94*radius,
		   centre->y - cos(t*M_PI/30)*0.94*radius);
	}
        SelectObject(dc, oldhPen);
        DeleteObject(hPen);
    }

	hBrush=CreateSolidBrush(RGB(00, 128,128));
	oldhBrush=SelectObject(dc, hBrush);
    /* Hour divisions */
    for(t=0; t<12; t++) {
		hPen=CreatePen(PS_SOLID, 1, RGB(0,0,0));
		oldhPen=SelectObject(dc, hPen);
		Rectangle(dc, 
			centre->x + sin(t*M_PI/6)*0.95*radius-hourWidth/2,
			centre->y - cos(t*M_PI/6)*0.95*radius-hourWidth/2,
			centre->x + sin(t*M_PI/6)*0.95*radius+hourWidth/2,
			centre->y - cos(t*M_PI/6)*0.95*radius+hourWidth/2);
		SelectObject(dc, oldhPen);
		DeleteObject(hPen);
		hPen=CreatePen(PS_SOLID, 1, RGB(0,255,255));
		oldhPen=SelectObject(dc, hPen);
        MoveToEx(dc,
                 centre->x + sin(t*M_PI/6)*0.95*radius-hourWidth/2,
                 centre->y - cos(t*M_PI/6)*0.95*radius+hourWidth/2,
                 NULL);
        LineTo(dc,
               centre->x + sin(t*M_PI/6)*0.95*radius-hourWidth/2,
               centre->y - cos(t*M_PI/6)*0.95*radius-hourWidth/2);
        LineTo(dc,
               centre->x + sin(t*M_PI/6)*0.95*radius+hourWidth/2,
               centre->y - cos(t*M_PI/6)*0.95*radius-hourWidth/2);
		SelectObject(dc, oldhPen);
		DeleteObject(hPen);
    }
    SelectObject(dc, oldhBrush);
    DeleteObject(hBrush);
}


static void DrawHand(HDC dc,HandData* hand)
{
	MoveToEx(dc, hand->End.x, hand->End.y, NULL);
	LineTo(dc, hand->p2.x, hand->p2.y);
	LineTo(dc, hand->p3.x, hand->p3.y);
	LineTo(dc, hand->p4.x, hand->p4.y);
	LineTo(dc, hand->End.x, hand->End.y);
}

static void DrawHands(HDC dc, BOOL bSeconds)
{
    if (bSeconds) {
		SelectObject(dc, CreatePen(PS_SOLID, 1, HandColor));
		MoveToEx(dc, SecondHand.Start.x, SecondHand.Start.y, NULL);
		LineTo(dc, SecondHand.End.x, SecondHand.End.y);
		DeleteObject(SelectObject(dc, GetStockObject(NULL_PEN)));
    }

    SelectObject(dc, CreatePen(PS_SOLID, 1, HandColor));
    DrawHand(dc, &MinuteHand);
    DrawHand(dc, &HourHand);
    DeleteObject(SelectObject(dc, GetStockObject(NULL_PEN)));
}

static void PositionHand(const POINT* centre, double length, double angle, HandData* hand)
{
    hand->Start = *centre;
    hand->End.x = centre->x + sin(angle)*length;
    hand->End.y = centre->y - cos(angle)*length;
	hand->p2.x=centre->x + sin(angle+M_PI/2)*length*0.07;
	hand->p2.y=centre->y - cos(angle+M_PI/2)*length*0.07;
	hand->p3.x=centre->x + sin(angle+M_PI)*length*0.2;
	hand->p3.y=centre->y - cos(angle+M_PI)*length*0.2;
	hand->p4.x=centre->x + sin(angle-M_PI/2)*length*0.07;
	hand->p4.y=centre->y - cos(angle-M_PI/2)*length*0.07;
}

static void PositionHands(const POINT* centre, int radius, BOOL bSeconds)
{
//    SYSTEMTIME st;
    double hour, minute, second;
    struct dostime_t t;

    _dos_gettime (&t);
    hour = t.hour%12;
	minute = t.minute;
	second = t.second;

    /* 0 <= hour,minute,second < 2pi */


    PositionHand(centre, radius * 0.6,  ((hour*5+minute/12)/(12*5)) * 2*M_PI, &HourHand);
    PositionHand(centre, radius * 0.79, minute/60 * 2*M_PI, &MinuteHand);
    if (bSeconds)
        PositionHand(centre, radius * 0.79, second/60 * 2*M_PI, &SecondHand);  
}

void AnalogClock(HDC dc, int x, int y, BOOL bSeconds)
{
    POINT centre;
    int radius;
    
    radius = min(x, y)/2;
    if (radius < 0)
	return;

    centre.x = x/2;
    centre.y = y/2;

    DrawTicks(dc, &centre, radius);
    PositionHands(&centre, radius, bSeconds);
    DrawHands(dc, bSeconds);
}


void IconAnalogClock(HDC dc, int x, int y)
{
    POINT centre;
    int radius;
    int t;
    
    radius = min(x, y)/2;
    if (radius < 0)
	return;

    centre.x = x/2;
    centre.y = y/2;

    for(t=0; t<12; t++) {
		SetPixel(dc, centre.x + sin(t*M_PI/6)*0.8*radius, centre.y - cos(t*M_PI/6)*0.8*radius, RGB(255,255,255));
		SetPixel(dc, centre.x + sin(t*M_PI/6)*0.8*radius+1, centre.y - cos(t*M_PI/6)*0.8*radius, RGB(0,0,0));
		SetPixel(dc, centre.x + sin(t*M_PI/6)*0.8*radius+1, centre.y - cos(t*M_PI/6)*0.8*radius+1, RGB(0,0,0));
		SetPixel(dc, centre.x + sin(t*M_PI/6)*0.8*radius, centre.y - cos(t*M_PI/6)*0.8*radius+1, RGB(0,0,0));
    }

    PositionHands(&centre, radius, FALSE);

    SelectObject(dc, CreatePen(PS_SOLID, 1, HandColor));
	MoveToEx(dc, MinuteHand.Start.x, MinuteHand.Start.y, NULL);
	LineTo(dc, MinuteHand.End.x, MinuteHand.End.y);
//    DrawHand(dc, &MinuteHand);
	MoveToEx(dc, HourHand.Start.x, HourHand.Start.y, NULL);
	LineTo(dc, HourHand.End.x, HourHand.End.y);
    //DrawHand(dc, &HourHand);
    DeleteObject(SelectObject(dc, GetStockObject(NULL_PEN)));
}

void FormatDate(char * szDate, BOOL bFull)
{
    struct dosdate_t d;
    char f[3][5]={"","",""};
	int i;
	char dFormat[20]="";
	char buf[20]="";
/*	
	strcpy(dFormat,"%[dMy]");
	strcat(dFormat, Globals.sDate);
	strcat(dFormat, "%[dMy]");
	strcat(dFormat, Globals.sDate);
	strcat(dFormat, "%[dMy]");
*/
	strcpy(dFormat,"%[dMy]/%[dMy]/%[dMy]");

    _dos_getdate (&d);

	if (3==sscanf(Globals.sShortDate, dFormat, &f[0],&f[1],&f[2]))
	{
		for (i=0; i<3; i++)
		{
			if (!strcmp("d", f[i])) {
				sprintf(buf, "%d", d.day);
				strcat(szDate, buf);
			}
			else if (!strcmp("dd", f[i])) {
				sprintf(buf, "%02d", d.day);
				strcat(szDate, buf);
			}
			else if (!strcmp("M", f[i])) {
				sprintf(buf, "%d", d.month);
				strcat(szDate, buf);
			}
			else if (!strcmp("MM", f[i])) {
				sprintf(buf, "%02d", d.month);
				strcat(szDate, buf);
			}
			else if (!strcmp("yy", f[i]) && bFull) {
				sprintf(buf, "%02d", d.year % 100);
				strcat(szDate, buf);
			}
			else if (!strcmp("yyyy", f[i]) && bFull) {
				sprintf(buf, "%04d", d.year);
				strcat(szDate, buf);
			} else {
				//strcat(szDate, "error");
			};
			if ((i<2 && bFull) || (i<1 && !bFull)) strcat(szDate, Globals.sDate);
		}
	}
}
 
void FormatTime(char * szTime, BOOL bFull)
{
	char tFormat[20]="";
	struct dostime_t t;
	int hour;
	char buf[3]="";

	_dos_gettime (&t);

	if (Globals.iTime) // 24h
	{
		hour=t.hour;
	} else {  // 12h
		hour=(t.hour % 12)?(t.hour % 12):12;
	}

	if (Globals.iTLZero) 
	{
		sprintf(tFormat, "%02d%s%02d%s", hour, Globals.sTime, t.minute, (Globals.bSeconds && bFull)?Globals.sTime:"");
	} else {
		if (hour<10)
		{
			sprintf(tFormat, "  %d%s%02d%s", hour, Globals.sTime, t.minute, (Globals.bSeconds && bFull)?Globals.sTime:"");
		} else {
			sprintf(tFormat, "%d%s%02d%s", hour, Globals.sTime, t.minute, (Globals.bSeconds && bFull)?Globals.sTime:"");
		}
	}

	if (bFull)
	{
		if (Globals.bSeconds) strcat(tFormat, "%02d");
		if (!Globals.iTime)
		{
			strcat(tFormat, " ");
			strcat(tFormat, (t.hour<12)?Globals.s1159:Globals.s2359);
		}
	}

	sprintf(szTime, tFormat, t.second);
}


HFONT SizeFont(HDC dc, int x, int y, BOOL bSeconds, const LOGFONT* font)
{
    SIZE extent;
    LOGFONT lf;
    double xscale, yscale;
    HFONT oldFont, newFont;
    char szTime[255];
    int chars;

    FormatTime(szTime, TRUE);
    chars=lstrlen(szTime);

    lf = *font;
    lf.lfHeight = -20;

    oldFont = SelectObject(dc, CreateFontIndirect(&lf));
    GetTextExtentPoint(dc, szTime, chars, &extent);
    DeleteObject(SelectObject(dc, oldFont));

    xscale = (double)x/extent.cx;
    yscale = (double)y/extent.cy;
    lf.lfHeight *= min(xscale, yscale);    
    newFont = CreateFontIndirect(&lf);

    return newFont;
}


void DigitalClock(HDC dc, int x, int y, BOOL bSeconds)
{
    SIZE extent;
    HFONT oldFont;
    char szTime[255]="";
    char szDate[255]="";
    int tchars;
    int dchars;
	int upshift = 0;
	BOOL shift = FALSE;

	FormatDate(szDate, TRUE);
    dchars=lstrlen(szDate);

	FormatTime(szTime, TRUE);
    tchars=lstrlen(szTime);


    oldFont = SelectObject(dc, Globals.hFont);

    GetTextExtentPoint(dc, szTime, tchars, &extent);
    if (extent.cy>63) {
		shift=TRUE;
	}
	
	if (Globals.bDate) upshift=extent.cy/2;
	
	SetBkColor(dc, BackgroundColor);
    SetBkMode(dc, TRANSPARENT);

	if (shift) {
		SetTextColor(dc, RGB(255,255,255));
		TextOut(dc, (x - extent.cx)/2-2 , (y - extent.cy)/2-2 - upshift , szTime, tchars);
		SetTextColor(dc, ShadowColor);
		TextOut(dc, (x - extent.cx)/2+2 , (y - extent.cy)/2+2 - upshift , szTime, tchars);
	}

	if (shift)
	{
		SetTextColor(dc, FaceColor);
	} else {
		SetTextColor(dc, HandColor);
	}
	
    TextOut(dc, (x - extent.cx)/2, (y - extent.cy)/2 - upshift, szTime, tchars);

	if (Globals.bDate)
	{
		SelectObject(dc, Globals.hDateFont);
		SetTextColor(dc, HandColor);
		GetTextExtentPoint(dc, szDate, dchars, &extent);
		TextOut(dc, (x - extent.cx)/2, y/2, szDate, dchars);
	}

    SelectObject(dc, oldFont);
}

void IconDigitalClock(HDC dc, int x, int y)
{
    SIZE extent;
    HFONT oldFont, font;
	LOGFONT logfont;
    char szTime[255]="";
    int tchars;

	FormatTime(szTime, FALSE);
    tchars=lstrlen(szTime);

    memset(&logfont, 0, sizeof(logfont));
    logfont.lfHeight = 16;
	logfont.lfWidth = 7;
	lstrcpy(logfont.lfFaceName, "Symbol");
    font = CreateFontIndirect(&logfont);

    oldFont = SelectObject(dc, font);
    GetTextExtentPoint(dc, szTime, tchars, &extent);

    SetBkColor(dc, BackgroundColor);
    SetBkMode(dc, TRANSPARENT);

    SetTextColor(dc, RGB(0,0,0));
    TextOut(dc, 4, (y - extent.cy)/2, szTime, tchars);

    SelectObject(dc, oldFont);
	DeleteObject(font);

}
