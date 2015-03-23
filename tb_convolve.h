/* tb_convolve.h */

#ifndef TB_CONVOLVE_H_
#define TB_CONVOLVE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned len;
  double *pos_coef;
  double *neg_coef;;
} tb_convolve;

tb_convolve *tb_convolve_new(unsigned len,
                             const double *coef);

tb_convolve *tb_convolve_new_signed(unsigned len,
                                    const double *pos_coef,
                                    const double *neg_coef);


void tb_convolve_free(tb_convolve *c);

double tb_convolve_calc(const tb_convolve *c, const double *in, unsigned len, unsigned pos);
void tb_convolve_apply(const tb_convolve *c, double *out, const double *in, unsigned len);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
