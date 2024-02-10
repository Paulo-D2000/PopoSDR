#include <vector>
#include <Types.h>
#include <Constants.h>
#include <Window.h>

/**
 * Generates Gaussian Filter Taps.
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
 * Generates LowPass Filter Taps.
 *
 * @param SampleRate Sampling Rate in hertz, any value in range 1 to infinite.
 * @param Cutoff Cutoff fequency in hertz, any value in range 1 to Samplerate/2.
 * @param Gain Output gain, any value, except zero, in range -infinite to infinite.
 * @param Span Filter span in symbols, any `int` in range 2 to infinite.
 * @param window Windowing function type, default is Kaiser.
 * @return vector of floats containing the filter coefficients.
 * @note Span Should be > 3 for most cases!
 */
std::vector<F32> Generate_Generic_LPF(float SampleRate, float Cutoff, float Gain = 1.0f, int Span = 6, WindowType window=Kaiser);

/**
 * Generates Square Root Raised Cosine Filter Taps.
 *
 * @param SampleRate Sampling Rate in hertz, any value in range 1 to infinite.
 * @param SymbolRate SYmbol Rate in hertz, any value in range 1 to Samplerate/2.
 * @param ExcessBw Excess Bandwidth (Alpha or Beta), any float, greater than 0.0f smaller than 1.0f.
 * @param Gain Output gain, any value, except zero, in range -infinite to infinite.
 * @param Span Filter span in symbols, any `int` in range 2 to infinite.
 * @return vector of floats containing the filter coefficients.
 * @note Span Should be > 3 for most cases!
 */
std::vector<F32> Generate_Root_Raised_Cosine(float SampleRate, float SymbolRate, float ExcessBw = 0.35f, float Gain = 1.0f, int Span = 6);