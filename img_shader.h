#pragma once


bool initOpenGL(bool showWindow, int width, int height);
void getSize(int* width, int* height);
int compileFragShader(const char* fshader);
int inImg(const char* format, int size, const char* buf);
void render(int prog, int img);
bool outImg(const char* format, int size, char* intoBuf);
void runWindow();
int elemSize(const char* format);

