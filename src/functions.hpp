#ifndef SCHNEK_FUNCTIONS_H
#define SCHNEK_FUNCTIONS_H

namespace schnek {

template<typename T>
T min(T a, T b) { return a>b?b:a; }

template<typename T>
T max(T a, T b) { return a>b?a:b; }

template<typename T>
int signum0(T x) { return x>0?1:(x<0?-1:0); }

template<typename T>
int signum(T x) { return x<0?-1:1; }

double drand();

int irand(int range);

}

#endif
