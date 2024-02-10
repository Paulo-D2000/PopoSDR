#include <Types.h>
#include <Constants.h>

/**
 * Window Functions
 * 
 * Based on https://en.wikipedia.org/wiki/Window_function
*/

/**
 * Sinc [sin(pi*x)/pi*x] Function
 * @param x time, any `float` in range -infinite to infinite.
 * @return `F32` representing the function [sin(pi*x)/pi*x] at point x.
 * @note at point `x=0` the return is `1.0`
 */
inline F32 sinc(const F32& x){
    if(x == 0.0f){
        return 1.0f;
    }
    return sinf(x*M_PI_F) / (x*M_PI_F);
}

enum WindowType {Rect, Hanning, Hamming, Blackman, Kaiser};


/**
 * Window Function name getter for logging
 * @param window any `WindowType`.
 * @return `std::string` representing the window name.
 */
inline std::string getWindowName(WindowType window){
    static std::string windowLabels[] = {"Rect", "Hanning", "Hamming", "Blackman", "Kaiser"};
    return windowLabels[window];
}

inline F32 I0(const F32& x){
    return (F32)(std::cyl_bessel_i(0, x));
}

/**
 * Evaulate window function at x
 * @param x any `F32` in range -1.0 < x < 1.0
 * @param type any `WindowType`.
 * @param alpha Kaise window parameter, any `float` in range 0 to infinite
 * @return `F32` representing the window function evaulated at x.
 */
inline F32 getWindow(const F32& x, const WindowType& type, float alpha=6.76){
    F32 w = 1.0f;
    float a = 1.0f;
    float b = alpha * M_PI_F;
    float a0 = 1.0f;
    float a1 = 1.0f;
    float a2 = 1.0f;
    switch (type)
    {
    case Rect:
        w = 1.0f;
        break;

    case Hanning:
        w = 0.5f * 0.5f * cosf(x);
        break;

    case Hamming:
        a0 = 25.0f/46.0f;
        a1 = 1.0f - a0;
        w = a0 * a1 * cosf(x);
        break;
    
    case Blackman:
        a = 0.16f;
        a0 = 0.5f*(1-a);
        a1 = 0.5f;
        a2 = 0.5f*a;
        w = a0 - a1 * cosf(M_TWOPI_F*(0.5f+0.5f*x)) + a2*cosf(2.0f*M_TWOPI_F*(0.5f+0.5f*x));
        break;

    case Kaiser:
        w = I0(x) * (b*sqrtf(1.0f-x*x)) / (I0(x)*b);
        break;
    }
    return w;
}