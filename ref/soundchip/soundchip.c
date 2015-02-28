/* soundchip.c */

#if 0

int fft(const struct frames &frames) {
  int n_points = frames.n_frames;

  fprintf(stderr, "In fft on %d samples\n", n_points);

  fftw_input_type *in;
  fftw_output_type *out;
  fftw_complex *comout;
  fftw_plan plan;

  in = (fftw_input_type *) fftw_malloc(sizeof(fftw_input_type) * n_points);
  out = (fftw_output_type *) fftw_malloc(sizeof(fftw_output_type) * n_points);
  comout = (fftw_complex *) fftw_malloc(sizeof(*comout) * n_points);

  plan = fftw_plan_r2r_1d(n_points, in, out, FFTW_R2HC, FFTW_FORWARD);

  channeldata *data1 = (channeldata *)frames.files[0];
  memset((void *)out, 0, sizeof(fftw_output_type) * n_points);
  for (int i = 0; i < n_points; i++) {
    // fixme, channel 0 only
    in[i] = (double)data1[i].channels[0];
  }

  fftw_execute(plan);

  int m = 0;
  double correction = (double)sample_frequency / (double)n_points;
  for (int i = 0; i < (n_points / 2) + 1; i++) {
    double absval = out[i] * out[i] + out[(n_points / 2) + 1 - i] * out[(n_points / 2) + 1 - i];
    double cc = (double)m * correction;
    printf("%f %f\n", cc, absval);
    m++;
  }

  fftw_destroy_plan(plan);
  fftw_free(in);
  fftw_free(out);

  return 0;
}

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
