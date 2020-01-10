#pragma once

#define _WINSOCKAPI_
#include <windows.h>
#include <time.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <WinSock2.h>
#include <thread>

#include "vector.h"
#include "math.h"
#include "offsets.h"
#include "junk.h"
#include "log.h"
#include "custom_structs.h"
#include "driver.h"
#include "utils.h"
#include "overlay.h"
//#include "DirectX.h"
#include "Game.h"

struct Matrix {
	float matrix[16];
};