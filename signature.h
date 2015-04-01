/* signature.h */

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define sig_SIGNATURE_BITS  256

double sig_normalise(double x);
double *sig_scale_length(double *dst, const double *src, size_t len);
double *sig_smooth(double *dst, const double *src, size_t len);
char *sig_signature(char *sig, const double *src, size_t len);

#ifdef __cplusplus
}
#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
