/************************************************************
 *
 *   console.cpp
 *   a crude message console for queuing and writing
 *   messages to the user
 *
 */

#include "game.h"
#include "console.h"
#include "poly.h"
#include "fallen/DDLibrary/headers/D3DTexture.h"
#include "fallen/DDLibrary/headers/GDisplay.h"
//#include "font3d.h"
#include "font2d.h"
#include "panel.h"
#include <winsock.h>

#ifdef TARGET_DC
#include "target.h"
#endif

#define CONSOLE_LINES	15
#define	CONSOLE_WIDTH	50

struct ConsoleText {
	CBYTE Text[CONSOLE_WIDTH];
	SLONG Age;
};

//static Font3D *font=0;
static ConsoleText Data[CONSOLE_LINES];
static CBYTE status_text[_MAX_PATH];

//
// The messages at specific place on the screen.
//

typedef struct
{
	SLONG delay;					// (delay == 0) => Don't display message.
	SLONG x;
	SLONG y;
	CBYTE mess[CONSOLE_WIDTH];

} CONSOLE_Mess;

#define CONSOLE_MAX_MESSES 16

CONSOLE_Mess CONSOLE_mess[CONSOLE_MAX_MESSES];




void CONSOLE_font(CBYTE *fontpath, float scale) {
//	if (font) delete font;
//	font = new Font3D(fontpath,scale);
}


//
// Returns TRUE if there is no text to write to the screen.
//

SLONG CONSOLE_no_text()
{
	SLONG i;

	CONSOLE_Mess *cm;
/*
	if (GAME_STATE & GS_PLAYBACK)
	{
		return TRUE;
	}
*/
	for (i = 0; i < CONSOLE_MAX_MESSES; i++)
	{
		cm = &CONSOLE_mess[i];

		if (cm->delay)
		{
			//
			// There is a text_at message to display.
			//

			return FALSE;
		}
	}
/*
	if (Data[0].Age)
	{
		return FALSE;
	}
*/
	return TRUE;
}

static SLONG last_tick;
static SLONG this_tick;

void CONSOLE_draw() {


#ifndef TARGET_DC


	SLONG i;

	CONSOLE_Mess *cm;


	last_tick = this_tick;
	this_tick = GetTickCount();
	if (this_tick-last_tick>4000) {
		// it's prolly lying. ish.
		last_tick=this_tick-1000;
	}

//	if (!font) return;


	if (CONSOLE_no_text()&&!Data[0].Age)
	{
/*		static moo=0;
		POLY_frame_init(FALSE,FALSE);
		FONT2D_DrawString("YADDA YADDA...",10,10,0xFF00FF,16,POLY_PAGE_FONT2D,moo>>2);
		moo++;
		POLY_frame_draw(FALSE,FALSE);*/
		//
		// Nothing to draw.
		//
		if (*status_text) {
			POLY_frame_init(FALSE, FALSE);
			FONT2D_DrawString(status_text,10,10,0x7f00ff00,256,POLY_PAGE_FONT2D);
			POLY_frame_draw(FALSE,TRUE);
		}

		return;

		//	if (!(GAME_STATE & GS_PLAYBACK) && !Data[0].Age) return;  // quick return when there's no text to write
	}

	// probably want to change this to only blank areas we'll be writing to
//	the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);

	POLY_frame_init(FALSE, FALSE);
	if (*status_text) FONT2D_DrawString(status_text,10,10,0x7f00ff00,256,POLY_PAGE_FONT2D);
/*
	if (GAME_STATE & GS_PLAYBACK)
	{
void	draw_a_3d_menu(Font3D &font, SLONG	menu);

		draw_a_3d_menu((*font),1);
	}
*/

	for (i=0;i<CONSOLE_LINES;i++)
		if (Data[i].Age) {
			if (Data[i].Age>512) {
				FONT2D_DrawString(Data[i].Text,135,370+(17*i),0x00ff00,256,POLY_PAGE_FONT2D);
//				FONT2D_DrawString(Data[i].Text,2,44+(18*i),0x20ffd0,11,POLY_PAGE_FONT2D);
			} else {
				FONT2D_DrawString(Data[i].Text,135,370+(17*i),0x00ff00,256,POLY_PAGE_FONT2D,(512-Data[i].Age)>>3);
//				FONT2D_DrawString(Data[i].Text,2,44+(18*i),0x20ffd0,11,POLY_PAGE_FONT2D,(512-Data[i].Age)>>3);
			}
/*			if (Data[i].Age>500) {
				font->DrawString(Data[i].Text,320,200+(55*i),0xff9f9f,2.0,0);
			} else {
				font->DrawString(Data[i].Text,320,200+(55*i),0xff9f9f,2.0,0,(500-Data[i].Age)*0.0015);
			}*/
			Data[i].Age -= this_tick - last_tick;

			if (Data[i].Age < 0)
			{
				Data[i].Age = 0;
			}
		}

	//
	// The messages at specific points on the screen.
	//

/*	if (!CONSOLE_no_text()) {
		the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);
		for (i = 0; i < CONSOLE_MAX_MESSES; i++)
		{
			cm = &CONSOLE_mess[i];

			if (cm->delay)
			{
				font->DrawString(
							cm->mess,
							cm->x,
							cm->y,
							0xffffff,
							1.0);

				cm->delay -= this_tick - last_tick;

				if (cm->delay < 0)
				{
					cm->delay = 0;
				}
			}
		}
	}*/

	POLY_frame_draw(FALSE,TRUE);
	if (!Data[0].Age) CONSOLE_scroll();


#endif //#ifndef TARGET_DC

}

void CONSOLE_text(CBYTE *text, SLONG delay)
{
	PANEL_new_text(NULL, delay, text);

	return;


	SLONG i;
	CBYTE *ch;
	CBYTE *ch1;
	CBYTE *ch2;
	CBYTE *pipe;

	CBYTE temp[1024];

	//
	// Early out on NULL strings or strings with just spaces in them.
	//

	if (text == NULL)
	{
		return;
	}

	for (ch = text; *ch; ch++)
	{
		if (!isspace(*ch))
		{
			goto found_non_white_space;
		}
	}

	return;

  found_non_white_space:;

	//
	// Check for pipes that split up the text.
	//

	pipe = strchr(text, '|');

	if (pipe)
	{
		//
		// Create two strings...
		//

		strcpy(temp, text);

		//
		// Terminate the first string but overwriting '|'.
		//

		temp[pipe - text] = '\000';

		//
		// Print each half separately.
		//

		CONSOLE_text( temp,                  delay);
		CONSOLE_text(&temp[pipe + 1 - text], delay);

		return;
	}

	//
	// Make sure the the string is not too long to fit on one line.
	//

	if (strlen(text) <= CONSOLE_WIDTH)
	{
		ch1 = text;
		ch2 = NULL;
	}
	else
	{
		strcpy(temp, text);

		//
		// Find the first SPACE character <= CONSOLE_WIDTH
		//

		for (i = CONSOLE_WIDTH - 1; i > 0; i--)
		{
			if (isspace(temp[i]))
			{
				//
				// This is where to break up the line.
				//

				temp[i] = '\000';

				ch1 = temp;
				ch2 = temp + i + 1;

				goto split_up_line;
			}
		}

		//
		// Too long and nowhere to split it up!
		//

		return;

	  split_up_line:;
	}

	//
	// Add 'ch1' to the message list.
	//

	i=0;
	while ((i<CONSOLE_LINES)&&(Data[i].Age)) i++;
	if (i==CONSOLE_LINES) {
		CONSOLE_scroll();
		i=CONSOLE_LINES-1;
	}
	Data[i].Age=delay;
	strncpy(Data[i].Text,ch1,CONSOLE_WIDTH-1);
	Data[i].Text[CONSOLE_WIDTH-1]=0;

	//
	// Convert message to uppercase.
	//

	for (ch = Data[i].Text; *ch; *ch++ = toupper(*ch));

	//
	// And add the second part of our message...
	//

	CONSOLE_text(ch2, delay);
}


void CONSOLE_scroll() {
	UBYTE i;

	while (Data[0].Text[0]&&!Data[0].Age) {

		for (i=1;i<CONSOLE_LINES;i++) {
			Data[i-1]=Data[i];
		}
		Data[CONSOLE_LINES-1].Age=0;
		Data[CONSOLE_LINES-1].Text[0]=0;
	}
}


void CONSOLE_clear() {
	memset(Data,0,sizeof(Data));
	if (!this_tick) this_tick=GetTickCount();
}



void CONSOLE_text_at(
		SLONG  x,
		SLONG  y,
		SLONG  delay,
		CBYTE *fmt, ...)
{
	SLONG i;

	CONSOLE_Mess *cm;

	//
	// Work out the real message.
	//

	CBYTE   message[FONT_MAX_LENGTH];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	//
	// Convert to uppercase.
	//

	for (CBYTE *ch = message; *ch; *ch++ = toupper(*ch));

	//
	// Look for an message at the same place.
	//

	for (i = 0; i < CONSOLE_MAX_MESSES; i++)
	{
		cm = &CONSOLE_mess[i];

		if (cm->delay && cm->x == x && cm->y == y)
		{
			//
			// Found an message at the same place.
			//

			strncpy(cm->mess, message, CONSOLE_WIDTH);

			cm->delay = delay;
			cm->x     = x;
			cm->y     = y;

			return;
		}
	}

	//
	// Look for an unused message.
	//

	for (i = 0; i < CONSOLE_MAX_MESSES; i++)
	{
		cm = &CONSOLE_mess[i];

		if (cm->delay <= 0)
		{
			//
			// Found an unused message.
			//

			strncpy(cm->mess, message, CONSOLE_WIDTH);

			cm->delay = delay;
			cm->x     = x;
			cm->y     = y;

			return;
		}
	}
}

void CONSOLE_status(CBYTE *msg) {
	strcpy(status_text,msg);
	for (CBYTE *ch = status_text; *ch; *ch++ = toupper(*ch));
}

#if 0
#ifndef FINAL

// TCP/IP console

#pragma comment ( lib, "ws2_32.lib" )

static DWORD WINAPI ThreadRun(LPVOID arg);
static int GetPort(SOCKET& s);
static void ReadMessage(SOCKET s);

HANDLE	hThread = NULL;

void CONSOLE_TCP()
{
	if (hThread)	return;

	DWORD	tid;
	hThread = CreateThread(NULL, 0, ThreadRun, NULL, 0, &tid);
}

static DWORD WINAPI ThreadRun(LPVOID arg)
{
	// begin Windows sockets
	WORD	wVersionRequested = MAKEWORD(2, 0);
	WSADATA	wsaData;

	if (WSAStartup(wVersionRequested, &wsaData))
	{
		TRACE("FATAL: Cannot initialize WinSock\n");
		return 1;
	}

	// get socket
	SOCKET	s;
	int		rc = GetPort(s);

	if (rc == -1)
	{
		TRACE("FATAL: Cannot open socket\n");
		return 1;
	}
	else if (rc == -2)
	{
		TRACE("FATAL: Address in use\n");
		return 1;
	}

	for (;;)
	{
		struct sockaddr raddr;
		struct sockaddr_in *rap = (struct sockaddr_in *)&raddr;

		int	raddrlen = sizeof(raddr);

		SOCKET as;

		// accept connection
		for (;;)
		{
			as = accept(s, &raddr, &raddrlen);
			if (as != INVALID_SOCKET)					break;		// connected
			if (WSAGetLastError() == WSAEINTR)			continue;	// harmless - try again
			if (WSAGetLastError() != WSAEWOULDBLOCK)	break;		// actual error
			Sleep(0);
		}

		if (as == INVALID_SOCKET)
		{
			TRACE("Accept failed\n");
		}
		else
		{
			TRACE("Accept: %s:%d\n", inet_ntoa(rap->sin_addr), 23);

			// set new socket to non-blocking
			u_long	one = 1;
			if (ioctlsocket(as, FIONBIO, &one) != SOCKET_ERROR)
			{
				// read message from socket
				ReadMessage(as);
			}

			closesocket(as);
		}
	}
}

static int GetPort(SOCKET& s)
{
	// create socket
	struct sockaddr_in inaddr;

	memset(&inaddr, 0, sizeof(inaddr));

	inaddr.sin_family		= AF_INET;
	inaddr.sin_addr.s_addr	= INADDR_ANY;
	inaddr.sin_port			= htons(23);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)	return -1;

	// bind socket
	if (bind(s, (struct sockaddr *)&inaddr, sizeof(inaddr)) == SOCKET_ERROR)
	{
		closesocket(s);
		if (WSAGetLastError() == WSAEADDRINUSE)	return -2;
		else									return -1;
	}

	// listen
	while (listen(s, 5) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEINTR)
		{
			closesocket(s);
			return -1;
		}
	}

	// set socket to non-blocking
	u_long	one = 1;
	if (ioctlsocket(s, FIONBIO, &one) == SOCKET_ERROR)
	{
		closesocket(s);
		return -1;
	}

	return 0;
}

static int Send(SOCKET s, const char* dbuf, int dlen, int flags)
{
	int olen = dlen;
	int	slen;

	for (;;)
	{
		slen = send(s, dbuf, dlen, flags);
		if (slen != SOCKET_ERROR)
		{
			if (slen == dlen)	return olen;
			dbuf += slen;
			dlen -= slen;
		}
		else
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)	break;	// proper error
		}
		Sleep(0);
	}

	return SOCKET_ERROR;
}

static int Recv(SOCKET s, char* buf, int dlen, int flags)
{
	int	rlen;

	for (;;)
	{
		rlen = recv(s, buf, 512, 0);
		if (!rlen)									return SOCKET_ERROR;	// connection is closed
		if (rlen != SOCKET_ERROR)					return rlen;
		if (WSAGetLastError() != WSAEWOULDBLOCK)	break;					// real error
		Sleep(0);
	}

	return SOCKET_ERROR;
}

extern void parse_console(CBYTE* str);

static void ReadMessage(SOCKET s)
{
	char	buf[512];		// buffer for data
	char	cmd[512];
	char*	cptr;

	cptr = cmd;

	char*	welcome = "Welcome to the Urban Chaos embedded Telnet server\x0D\n(c) Mucky Foot 1999\x0D\n\x0D\n> ";
	Send(s, welcome, strlen(welcome), 0);

	for (;;)
	{
		int	rlen = Recv(s, buf, 512, 0);

		if (rlen == SOCKET_ERROR)	return;

		// echo
		Send(s, buf, rlen, 0);

		for (int ii = 0; ii < rlen; ii++)
		{
			if (buf[ii] == 0x0D)	continue;
			if (buf[ii] == 0x08)
			{
				if (cptr > cmd)	cptr--;
				continue;
			}
			if (buf[ii] == 0x0A)
			{
				*cptr = 0;
				TRACE("Command: %s\n", cmd);
				parse_console(cmd);
				cptr = cmd;
				send(s, "> ", 2, 0);
				continue;
			}
			*cptr++ = buf[ii];
		}
	}
}

#endif
#endif
