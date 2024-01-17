#include <vector>
#include <Types.h>
#include <Constants.h>
#include <Window.h>

/**
 * Gaussian Filter Taps.
 *
 * @param Ts Symbol Period (Span), any value in range 1 to infinite.
 * @param BTb Bandwidth Time Product, any `float` in range 0.0 to infinite.
 * @param Fs Oversampling Rate (Sps), any value in range 1 to infinite.
 * @return vector of floats containing the filter coefficients.
 * @note Ts is usually set to 3.0
 * @note GSM uses Ts = 3.0, BTb = 0.3, Fs = Sps
 */
std::vector<F32> Generate_Gaussian_LPF(float Ts, float BTb, float Fs);


/**
 * LowPass Filter Taps.
 *
 * @param SampleRate Sampling Rate in hertz, any value in range 1 to infinite.
 * @param Cutoff Cuttof fequency in hertz, any value in range 1 to Samplerate/2.
 * @param Span Filter span in symbols, any `int` in range 2 to infinite, defaults to 11.
 * @param window Windowing function type, default is Hamming.
 * @return vector of floats containing the filter coefficients.
 * @note Span Should be > 3 for most cases!
 */
std::vector<F32> Generate_Generic_LPF(float SampleRate, float Cutoff, int Span = 11, WindowType window=Hamming);