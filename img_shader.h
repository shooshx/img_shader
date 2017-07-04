#pragma once


bool initOpenGL(bool showWindow, int width, int height);
void getSize(int* width, int* height);
int compileFragShader(const char* fshader);
void delProgram(int prog);
int inImg(const char* format, int size, const char* buf, const char* varname);
void render(int prog);
bool outImg(const char* format, int size, char* intoBuf);
void runWindow();
int elemSize(const char* format);

