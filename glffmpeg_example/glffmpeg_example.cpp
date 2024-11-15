/*
* Simple sample for the glFFmpeg library
* Copyright (c) 2006 Marco Ippolito
*
* Demonstrates how to utilize glFFmpeg
*
* This file is part of glFFmpeg.
*/
#include "glffmpeg.h"
#include <GL/gl.h>

#ifdef _WIN32 
#include <windows.h>	
#include <io.h>
#include <fcntl.h>
#include <conio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <GL/glx.h> // This includes the necessary X headers
#include <GL/glu.h>
#include <X11/keysym.h>
#endif

#define FAILURE -1
#define SUCCESS 1

const unsigned int WINDOW_WIDTH = 1920;
const unsigned int WINDOW_HEIGHT = 1080;
const unsigned int FRAME_NUMBER = 600;
const char* STREAM_NAME = "glFFmpeg_example.mov";

static int s_argc = 0;
static char** s_argv = NULL;

double colors[3][4] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}
};
float c = 0.5f;
bool rising = true;

#ifdef _WIN32 
HDC                 s_hDC;				/* device context */
HGLRC               s_hRC;				/* opengl context */
HWND                s_hWnd;				/* window */
MSG                 msg;				/* message */
static PAINTSTRUCT  ps;
#else
Window              s_hWnd;
Display* s_pDisplay = NULL;
XEvent 		    event;
#endif // OS

void display()
{
	// Fade a triangles in and out
	if (rising) {
		c += 0.01f;
		if (c > 1.0f) {
			rising = false;
			c -= 0.02f;
		}
	}
	else {
		c -= 0.01f;
		if (c < 0.5f) {
			rising = true;
			c += 0.02f;
		}
	}

	// disable the depth test and set up the projection and model view
	// matrices
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-512.0, 512.0, -512.0, 512.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//Clear the color buffer if rendering locally
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw the triangle
	glBegin(GL_TRIANGLES);
	glColor3f(c, 0.0f, 0.0f);
	glVertex2f(0.0f, 307.4f);
	glColor3f(0.0f, c, 0.0f);
	glVertex2f(-306.4f, -306.4f);
	glColor3f(0.0f, 0.0f, c);
	glVertex2f(307.4f, -306.4f);
	glEnd();

	glFlush();
}



#ifdef _WIN32
// Window message handler used when rendering locally instead of remotely
LRESULT WINAPI
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_PAINT:
		display();
		SwapBuffers(s_hDC);

		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_SIZE:
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		PostMessage(hWnd, WM_PAINT, 0, 0);
		return 0;

	case WM_CHAR:
		switch (wParam) {
		case 27:			/* ESC key */
			PostQuitMessage(0);
			break;
		}
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	default:
		PostMessage(hWnd, WM_PAINT, 0, 0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif // OS 

int CreateOpenGLWindow(char* title, int x, int y, int width, int height)
{

#ifdef _WIN32

	int         pf;
	WNDCLASS    wc;
	PIXELFORMATDESCRIPTOR pfd;
	static HINSTANCE hInstance = 0;

	// only register the window class once - use hInstance as a flag.
	if (!hInstance)
	{
		hInstance = GetModuleHandle(NULL);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = "glFFmpeg";

		if (!RegisterClass(&wc))
		{
			MessageBox(NULL, "CreateOpenGLWindow: RegisterClass() failed - "
				"Cannot register window class.", "Error", MB_OK);
			return FAILURE;
		}
	}

	RECT windowRect;
	windowRect.left = 0;
	windowRect.right = windowRect.left + width;
	windowRect.top = 0;
	windowRect.bottom = windowRect.top + height;

	// Accounts for border sizes
	DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	AdjustWindowRect(&windowRect, style, FALSE);

	width = windowRect.right - windowRect.left;
	height = windowRect.bottom - windowRect.top;

	s_hWnd = CreateWindow("glFFmpeg", title, style,
		x, y, width, height, NULL, NULL, hInstance, NULL);

	if (s_hWnd == NULL)
	{
		MessageBox(NULL, "CreateOpenGLWindow: CreateWindow() failed - Cannot create a window.",
			"Error", MB_OK);
		return FAILURE;
	}

	s_hDC = GetDC(s_hWnd);

	/* there is no guarantee that the contents of the stack that become
	   the pfd are zeroed, therefore _make sure_ to clear these bits. */
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | 0;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	pf = ChoosePixelFormat(s_hDC, &pfd);
	if (pf == 0)
	{
		MessageBox(NULL, "CreateOpenGLWindow: ChoosePixelFormat() failed - "
			"Cannot find a suitable pixel format.", "Error", MB_OK);
		return FAILURE;
	}

	if (SetPixelFormat(s_hDC, pf, &pfd) == FALSE)
	{
		MessageBox(NULL, "CreateOpenGLWindow: SetPixelFormat() failed - "
			"Cannot set format specified.", "Error", MB_OK);
		return FAILURE;
	}

	DescribePixelFormat(s_hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	ReleaseDC(s_hWnd, s_hDC);

	s_hRC = wglCreateContext(s_hDC);
	wglMakeCurrent(s_hDC, s_hRC);
	ShowWindow(s_hWnd, true);

#else // OS 

	XSetWindowAttributes windowAttributes;
	XVisualInfo* visualInfo = NULL;
	Colormap colorMap;
	GLXContext glxContext;
	int errorBase;
	int eventBase;

	// Open a connection to the X server
	s_pDisplay = XOpenDisplay(NULL);

	if (s_pDisplay == NULL)
	{
		fprintf(stderr, "CreateOpenGLWindow: %s\n", "could not open display");
		return FAILURE;
	}

	// Make sure OpenGL's GLX extension supported
	if (!glXQueryExtension(s_pDisplay, &errorBase, &eventBase))
	{
		fprintf(stderr, "CreateOpenGLWindow: %s\n", "X server has no OpenGL GLX extension");
		return FAILURE;
	}

	// Find an appropriate visual

	int doubleBufferVisual[] =
	{
		GLX_RGBA,           // Needs to support OpenGL
			GLX_DEPTH_SIZE, 16, // Needs to support a 16 bit depth buffer
			GLX_DOUBLEBUFFER,   // Needs to support double-buffering
			None                // end of list
	};

	int singleBufferVisual[] =
	{
		GLX_RGBA,           // Needs to support OpenGL
			GLX_DEPTH_SIZE, 16, // Needs to support a 16 bit depth buffer
			None                // end of list
	};

	// Try for the double-bufferd visual first
	visualInfo = glXChooseVisual(s_pDisplay, DefaultScreen(s_pDisplay), doubleBufferVisual);

	// Create an OpenGL rendering context
	glxContext = glXCreateContext(s_pDisplay,
		visualInfo,
		NULL,      // No sharing of display lists
		GL_TRUE); // Direct rendering if possible

	if (glxContext == NULL)
	{
		fprintf(stderr, "CreateOpenGLWindow: %s\n", "could not create rendering context");
		return FAILURE;
	}

	// Create an X colormap since we're probably not using the default visual 
	colorMap = XCreateColormap(s_pDisplay,
		RootWindow(s_pDisplay, visualInfo->screen),
		visualInfo->visual,
		AllocNone);

	windowAttributes.colormap = colorMap;
	windowAttributes.border_pixel = 0;
	windowAttributes.event_mask = ExposureMask |
		VisibilityChangeMask |
		KeyPressMask |
		KeyReleaseMask |
		ButtonPressMask |
		ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask |
		SubstructureNotifyMask |
		FocusChangeMask;

	// Create an X window with the selected visual
	s_hWnd = XCreateWindow(s_pDisplay,
		RootWindow(s_pDisplay, visualInfo->screen),
		x, y,     // x/y position of top-left outside corner of the window
		width, height, // Width and height of window
		0,        // Border width
		visualInfo->depth,
		InputOutput,
		visualInfo->visual,
		CWBorderPixel | CWColormap | CWEventMask,
		&windowAttributes);

	XSetStandardProperties(s_pDisplay,
		s_hWnd,
		title,
		title,
		None,
		s_argv,
		s_argc,
		NULL);

	// Bind the rendering context to the window
	glXMakeCurrent(s_pDisplay, s_hWnd, glxContext);

	// Request the X window to be displayed on the screen
	XMapWindow(s_pDisplay, s_hWnd);

#endif // OS

	return SUCCESS;
}

int main(int argc, char* argv[])
{
	int frameNum = 0;
	void* imageBuffer = NULL;
	s_argc = argc;
	s_argv = argv;

	// Initialize a GL context
	if (CreateOpenGLWindow("glFFmpeg", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT) != SUCCESS)
	{
		return FAILURE;
	}

	// Initialize glFFmpeg
	if (initializeGLFFMPEG() != SUCCESS)
	{
		return FAILURE;
	}

	imageBuffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 3);

	if (initializeStream(STREAM_NAME, 60, WINDOW_WIDTH,
		WINDOW_HEIGHT, imageBuffer) != 0)
	{
		free(imageBuffer);
		return FAILURE;
	}

#ifdef _WIN32

	do {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			display();
			SwapBuffers(s_hDC);
		}

		// capture the frame
		glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, imageBuffer);
		// encode the frame to the video strem  
		encodeFrame(STREAM_NAME);

		if (++frameNum > FRAME_NUMBER)
		{
			break;
		}
	} while (msg.message != WM_QUIT);

	wglMakeCurrent(NULL, NULL);
	ReleaseDC(s_hWnd, s_hDC);
	wglDeleteContext(s_hRC);
	DestroyWindow(s_hWnd);

#else // OS 

	bool done = false;
	while (!done) {
		XPeekEvent(s_pDisplay, &event);

		if (event.type == KeyPress) {
			switch (XKeycodeToKeysym(s_pDisplay, event.xkey.keycode, 0)) {
			case XK_Escape:
				done = true;
			}
		}
		display();

		glXSwapBuffers(s_pDisplay, s_hWnd);

		// capture the frame
		glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, imageBuffer);
		// encode the frame to the video strem  
		encodeFrame(STREAM_NAME);

		frameNum++;
		if (frameNum > 300)
			done = true;
	}
#endif // OS

	//shutdown glFFmpeg
	shutdownStream(STREAM_NAME);

	free(imageBuffer);
	shutdownGLFFMPEG();
	return 0;
}