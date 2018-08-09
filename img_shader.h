#pragma once


void* initOpenGL(bool showWindow, void* ctrl);
void getSize(int* width, int* height, void* pctrl);
int compileFragShader(const char* fshader);
void delProgram(int prog);
int inImg(const char* format, int width, int height, const char* buf, const char* varname);
void render(int prog, int width, int height, void* ctrl);
bool outImg(const char* format, int size, char* intoBuf, void* pctrl);
void runWindow();
int elemSize(const char* format);

