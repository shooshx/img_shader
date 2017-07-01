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
    case WM_CREATE:
        LOG("WM_CREATE");
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

#define MY_WND_CLS "MyOglCls"



#define XSZ 800
#define YSZ 600


const char* vshader = R"***(
    attribute vec3 aVtxPos;
//    uniform mat4 uMVMatrix;
//    uniform mat4 uPMatrix;
    void main(void) {
        gl_Position = vec4(aVtxPos, 1.0); //uPMatrix * uMVMatrix * vec4(aVtxPos, 1.0);
    }
)***";

const char* fshader = R"***(
    void main(void) {
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

#define ATTR_VTX 0


int main()
{
    WNDCLASSEXA wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_OWNDC; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= MY_WND_CLS;

	if (RegisterClassExA(&wcex) == 0)
        return 1;

    LOG("Creating window");
    HWND hWnd = CreateWindowA(MY_WND_CLS, "OGL", WS_POPUP | WS_OVERLAPPEDWINDOW, 0, 0, XSZ, YSZ, 0,0,0,0);
    if (hWnd == NULL)
        return 1;

    HDC hDC = GetDC(hWnd);

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
        return 1;
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
   // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // SwapBuffers(hDC);

    ShowWindow(hWnd, SW_SHOW);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    int prog = glCreateProgram();
    int vs = makeShader(GL_VERTEX_SHADER, vshader);
    int fs = makeShader(GL_FRAGMENT_SHADER, fshader);
    if (vs == 0 || fs == 0) {
        LOG("Failed compiling shader");
        return 1;
    }
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);

    glBindAttribLocation(prog, ATTR_VTX, "aVtxPos");

    glLinkProgram(prog);
    // TBD get errors

    glUseProgram(prog);

   // 

    mglCheckErrors("attr");

  //  float vtxData = {
  //        1.0f, -1.0f,  0.0f,    
  //       -1.0f,  1.0f,  0.0f,    
  //       -1.0f, -1.0f,  0.0f,    
  //        1.0f,  1.0f,  0.0f  
  //  };

    float vtxData[] = {
          0.5f, -0.5f,  0.0f,    
         -0.5f,  0.5f,  0.0f,    
         -0.5f, -0.5f,  0.0f,    
          0.5f,  0.5f,  0.0f  
    };
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
    
    glDrawElements(GL_TRIANGLES, indCount, GL_UNSIGNED_SHORT, 0);

    SwapBuffers(hDC);


    MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    return 0;
};