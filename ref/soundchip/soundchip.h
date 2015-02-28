/* soundchip.h */

#ifndef SOUNDCHIP_H_
#define SOUNDCHIP_H_

#ifdef __cplusplus
extern "C" {
#endif

  struct frames {
    int n_frames;
    int n_files;
    char **files;
  };

  typedef double fftw_input_type;
  typedef double fftw_output_type;
  typedef double fftw_spectrum_type;

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
