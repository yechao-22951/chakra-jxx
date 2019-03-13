#pragma once
#define CXX_EXCEPTION_IF(exp)			do { if(exp) jxx_throw_error( 0,0 ); } while(0);
#include "jxx.var.value.h"
#include "jxx.var.object.h"
#include "jxx.var.function.h"