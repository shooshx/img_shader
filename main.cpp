
#include <Windows.h>
#include <stdio.h>

#include <gl/glew.h>
#include "img_shader.h"


#define CHECK_GL_ERR

#define LOG(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

LRESULT CALLBACK OglWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
    case WM_ERASEBKGND:
        //LOG("WM_ERASEBKGND");
        return 1;
    case WM_CHAR:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    case WM_CREATE:
        //LOG("WM_CREATE");
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

#define MY_WND_CLS "MyOglCls"



const char* def_vshader = R"***(
    attribute vec3 aVtxPos;
    varying vec2  pos;
    void main(void) {
        pos = vec2(aVtxPos.x * 0.5 + 0.5, aVtxPos.y * 0.5 + 0.5);
        gl_Position = vec4(aVtxPos, 1.0);
    }
)***";

const char* def_fshader = R"***(
    uniform sampler2D intex;
    varying vec2  pos;
    void main(void) {
        vec4 t1 = texture2D(intex, pos);
        gl_FragColor = vec4(0.7, 0.7, 0.7, 1.0);
    }
)***";


int makeShader(int type, const char* src) {
    int s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
       
    int status = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        int loglen = 0, wrotelen = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &loglen);
        char* buf = (char*)malloc(loglen);
        glGetShaderInfoLog(s, loglen, &wrotelen, buf);

        LOG("FAILED compiling shader\n%s", buf);
        free(buf);
        return 0;
    }
    return s;
}

const char* errorText(int code)
{
    switch(code)
    {
    case GL_NO_ERROR: return "GL_NO_ERROR";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    default: return "-Unknown Error Code-";
    }
}

void mglCheckErrors(const char* place)
{
#ifdef CHECK_GL_ERR 
    GLenum code;
    int count = 0;
    while ((code = glGetError()) != GL_NO_ERROR && count++ < 10) 
        LOG("GL-ERROR (%s) 0x%x : %s", place, code, errorText(code));
#endif
}


HDC g_hDC = NULL;
int g_width = 0;
int g_height = 0;
bool g_showWindow = false;

void getSize(int* width, int* height) {
    *width = g_width;
    *height = g_height;
}

bool initOpenGL(bool showWindow, int width, int height)
{
    WNDCLASSEXA wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_OWNDC; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)OglWndProc;
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= MY_WND_CLS;

	if (RegisterClassExA(&wcex) == 0) {
        LOG("Failed RegisterClassExA");
        return false;
    }

    g_width = width;
    g_height = height;
    g_showWindow = showWindow;

    DWORD style = WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED | WS_CAPTION;
    if (width < 150)
        style = WS_POPUP; // the title bar would streach it
    RECT rect = {0,0,width,height };
    AdjustWindowRect(&rect, style, FALSE);
    //LOG("rect %d,%d,%d,%d", rect.top, rect.bottom, rect.left, rect.right);

    HWND hWnd = CreateWindowA(MY_WND_CLS, "OGL", style, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL,0,0,0);
    if (hWnd == NULL) {
        LOG("Failed CreateWindowA");
        return false;
    }

    HDC hDC = GetDC(hWnd);
    g_hDC = hDC;

    PIXELFORMATDESCRIPTOR pfd = {
	        sizeof(PIXELFORMATDESCRIPTOR),
	        1,
	        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
	        PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
	        32,                        //Colordepth of the framebuffer.
	        0, 0, 0, 0, 0, 0,
	        0,
	        0,
	        0,
	        0, 0, 0, 0,
	        24,                        //Number of bits for the depthbuffer
	        8,                        //Number of bits for the stencilbuffer
	        0,                        //Number of Aux buffers in the framebuffer.
	        PFD_MAIN_PLANE,
	        0,
	        0, 0, 0
        };
    int pf = ChoosePixelFormat(hDC, &pfd);

    if (!SetPixelFormat(hDC, pf, &pfd))
        return 1;
    HGLRC rc = wglCreateContext(hDC);
    wglMakeCurrent(hDC, rc);

    auto err = glewInit();
    if (err != GLEW_OK) {
        LOG("glewInit error %d", err);
        return false;
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glActiveTexture(GL_TEXTURE0);
   // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // SwapBuffers(hDC);

    if (showWindow)
        ShowWindow(hWnd, SW_SHOW);
    return true;
}

#define ATTR_VTX 0
#define UNIFORM_TEX 1

int compileFragShader(const char* fshader)
{
    int prog = glCreateProgram();
    int vs = makeShader(GL_VERTEX_SHADER, def_vshader);
    int fs = makeShader(GL_FRAGMENT_SHADER, fshader);
    if (vs == 0 || fs == 0) {
        LOG("Failed compiling shader");
        return -1;
    }
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);

    // before link
    glBindAttribLocation(prog, ATTR_VTX, "aVtxPos"); 

    glLinkProgram(prog);

    int loc_tex = glGetUniformLocation(prog, "intex");
    //LOG("tex loc %d", loc_tex);
    glUseProgram(prog); // meeded for glUniform
    glUniform1i(loc_tex, 0);

    mglCheckErrors("compile");

    return prog;

}


/*
bool setImgSize(int width, int height)
{
    if (g_width != 0 || g_height != 0)
        return false;
    if (width == 0 || height == 0)
        return false;
    g_width = width;
    g_height = height;
    return true;
}
*/


// converting to RGBA because loading a single channel texture of non power of 2 has a problem
char *g_convertBuf = NULL;
int g_convBufSz = 0;

void ensureConvBuf() {
    if (g_convertBuf == NULL) {
        g_convBufSz = g_width * g_height * 4;
        g_convertBuf = (char*)malloc(g_convBufSz);
    }
}

int elemSize(const char* fmtname) {
    if (strcmp(fmtname, "RGBA") == 0)
        return 4;
    if (strcmp(fmtname, "RGB") == 0)
        return 3;
    if (strcmp(fmtname, "L") == 0)
        return 1;

    return 0;
}

int inImg(const char* fmtname, int size, const char* buf)
{
    int needSz = g_width * g_height * elemSize(fmtname);
    if (size != needSz) {
        LOG("Wrong size! %d != %d (%d*%d)", size, needSz, g_width, g_height);
        return -1;
    }

    int internal_format = 0, format = 0, type = 0;
    if (strcmp(fmtname, "L") == 0)
    {
        ensureConvBuf();
        memset(g_convertBuf, 0, g_convBufSz);
        for(int i = 0; i < size; ++i)
            g_convertBuf[i*4] = buf[i];
        buf = g_convertBuf;
        internal_format = GL_RGBA8;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    }
    else if (strcmp(fmtname, "RGBA") == 0)
    {
        internal_format = GL_RGBA8;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    }
    else if (strcmp(fmtname, "RGB") == 0)
    {
        internal_format = GL_RGB8;
        format = GL_RGB;
        type = GL_UNSIGNED_BYTE;
    }
    else
        return false;

    unsigned int tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // single mipmap level (https://www.khronos.org/opengl/wiki/Common_Mistakes)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, g_width, g_height, 0, format, type, buf);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    mglCheckErrors("tex");

    return (int)tex;
}




void render(int prog, int img)
{
     glUseProgram(prog);

    float vtxData[] = { 1.0f, -1.0f,  0.0f, -1.0f,  1.0f,  0.0f, -1.0f, -1.0f,  0.0f, 1.0f,  1.0f,  0.0f };
    //float vtxData[] = { 0.5f, -0.5f,  0.0f, -0.5f,  0.5f,  0.0f, -0.5f, -0.5f,  0.0f, 0.5f,  0.5f,  0.0f };
    int vtxCount = 4;
    short indData[] = {
        1, 2, 3,
        3, 2, 0
    };
    int indCount = 6;

    unsigned int vtxBuf = 0;
    glGenBuffers(1, &vtxBuf);
    glBindBuffer(GL_ARRAY_BUFFER, vtxBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtxData), vtxData, GL_STATIC_DRAW);
    unsigned int indBuf = 0;
    glGenBuffers(1, &indBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indData), indData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(ATTR_VTX);
    glVertexAttribPointer(ATTR_VTX, 3, GL_FLOAT, FALSE, 0, 0);

    //LOG("tex obj %d", img);
    glBindTexture(GL_TEXTURE_2D, img);
    
    glDrawElements(GL_TRIANGLES, indCount, GL_UNSIGNED_SHORT, 0);

   // SwapBuffers(g_hDC);


};

bool outImg(const char* fmtname, int size, char* intoBuf)
{
    int needSz = g_width * g_height * elemSize(fmtname);
    if (size != needSz) {
        LOG("Wrong size! %d != %d", size, needSz);
        return false;
    }
    char *buf = intoBuf;
    bool singleChan = false;
    int format = 0, type = 0;

    if (strcmp(fmtname, "L") == 0) {
        ensureConvBuf();
        buf = g_convertBuf;
        singleChan = true;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    }
    else if (strcmp(fmtname, "RGBA") == 0)
    {
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    }
    else if (strcmp(fmtname, "RGB") == 0)
    {
        format = GL_RGB;
        type = GL_UNSIGNED_BYTE;
    }
    else
        return false;

    glReadPixels(0, 0, g_width, g_height, format, type, (void*)buf);

    if (singleChan)
        for (int i = 0; i < size; ++i)
            intoBuf[i] = g_convertBuf[i * 4];

    return true;
}


void runWindow()
{
    SwapBuffers(g_hDC); // back buffer to front so it's visible

    MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} // TBD process ESC

    SwapBuffers(g_hDC); // front buffer to back so it could be read when saving to file
}


