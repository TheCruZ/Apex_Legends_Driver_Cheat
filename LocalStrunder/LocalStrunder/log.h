#pragma once
//LOGs
#ifdef _DEBUG
#define log(text,...) //printf(text,__VA_ARGS__)
#else
#define log(text,...)
#endif