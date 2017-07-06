#pragma once


bool initOpenGL(bool showWindow);
void getSize(int* width, int* height);
int compileFragShader(const char* fshader);
void delProgram(int prog);
int inImg(const char* format, int width, int height, const char* buf, const char* varname);
void render(int prog, int width, int height);
bool outImg(const char* format, int size, char* intoBuf);
void runWindow();
int elemSize(const char* format);

