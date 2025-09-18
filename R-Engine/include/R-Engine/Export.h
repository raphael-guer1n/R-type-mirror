#pragma once

#if defined(_WIN32)
  #if defined(R_ENGINE_BUILD)
    #define R_ENGINE_API __declspec(dllexport)
  #else
    #define R_ENGINE_API __declspec(dllimport)
  #endif
#else
  #define R_ENGINE_API
#endif
