#pragma once
//#include <ntddk.h>

#ifdef DEBUG
//#define log(text) DbgPrintEx(0, 0, text);
#define log(text,...) DbgPrintEx(0, 0, text,__VA_ARGS__);
#else
//#define log(text)
#define log(text,...)
#endif

