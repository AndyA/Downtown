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
  double prescale;
} tb_convolve;

tb_convolve *tb_convolve_new(unsigned len,
                             const double *coef);

tb_convolve *tb_convolve_new_signed(unsigned len,
                                    const double *pos_coef,
                                    const double *neg_coef);

void tb_convolve_free(tb_convolve *c);
void tb_convolve_set_prescale(tb_convolve *c, double prescale);

double tb_convolve_calc(const tb_convolve *c, const double *in, unsigned len, unsigned pos);
void tb_convolve_apply_linear(const tb_convolve *c, double *out, const double *in, unsigned len);
void tb_convolve_apply(const tb_convolve *c, double *out, const double *in, unsigned len);

double tb_convolve_elapsed(const double *series, unsigned len);
double tb_convolve_translate(const double *in, unsigned ilen, double *out, unsigned olen);

/* Internal */

double tb_convolve__sample(const double *coef, double pos, double sa);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
