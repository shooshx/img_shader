
#include <Windows.h>
#include <stdio.h>

#include <gl/glew.h>



#define CHECK_GL_ERR

#define LOG(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
    case WM_ERASEBKGND:
        LOG("WM_ERASEBKGND");
        return 1;
    case WM_CHAR:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    case WM_CREATE:
        LOG("WM_CREATE");
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

#define MY_WND_CLS "MyOglCls"


#define XSZ 800
#define YSZ 600


const char* def_vshader = R"***(
    attribute vec3 aVtxPos;
    varying vec2  pos;
    void main(void) {
        pos = aVtxPos.xy;
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

bool initOpenGL(bool showWindow)
{
    WNDCLASSEXA wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_OWNDC; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= MY_WND_CLS;

	if (RegisterClassExA(&wcex) == 0) {
        LOG("Failed RegisterClassExA");
        return false;
    }

//    LOG("Creating window");
    HWND hWnd = CreateWindowA(MY_WND_CLS, "OGL", WS_POPUP /*| WS_OVERLAPPEDWINDOW*/, 0, 0, XSZ, YSZ, 0,0,0,0);
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
    LOG("tex loc %d", loc_tex);
    glUseProgram(prog); // meeded for glUniform
    glUniform1i(loc_tex, 0);

    mglCheckErrors("compile");

    return prog;

}

int g_width = 0, g_height = 0;

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

int createTex(int internal_format, int format, int type, const char* buf)
{
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

int inGrayScaleByteImg(int size, const char* buf)
{
    if (size != (g_width * g_height)) {
        LOG("Wrong size! %d != %d (%d*%d)", size, g_width * g_height, g_width, g_height);
        return -1;
    }
    return createTex(GL_R8, GL_RED, GL_UNSIGNED_BYTE, buf);
}

int inRGBAByteImg(int size, const char* buf)
{
    if (size != (g_width * g_height * 4)) {
        LOG("Wrong size! %d != %d", size, g_width * g_height * 4);
        return -1;
    }
    return createTex(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, buf);
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

    SwapBuffers(g_hDC);

};

int outGrayScaleByte(int size, const char* intoBuf)
{
     if (size != (g_width * g_height))
        return false;

    return true;
}

void runWindow()
{
    MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} // TBD process ESC
}