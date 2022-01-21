// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

#ifdef RCPP_USE_GLOBAL_ROSTREAM
Rcpp::Rostream<true>&  Rcpp::Rcout = Rcpp::Rcpp_cout_get();
Rcpp::Rostream<false>& Rcpp::Rcerr = Rcpp::Rcpp_cerr_get();
#endif

// rcpp_binseg_normal
Rcpp::List rcpp_binseg_normal(const Rcpp::NumericVector data_vec, const int kmax, const Rcpp::LogicalVector is_validation_vec, const Rcpp::NumericVector position_vec);
RcppExport SEXP _binsegRcpp_rcpp_binseg_normal(SEXP data_vecSEXP, SEXP kmaxSEXP, SEXP is_validation_vecSEXP, SEXP position_vecSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< const Rcpp::NumericVector >::type data_vec(data_vecSEXP);
    Rcpp::traits::input_parameter< const int >::type kmax(kmaxSEXP);
    Rcpp::traits::input_parameter< const Rcpp::LogicalVector >::type is_validation_vec(is_validation_vecSEXP);
    Rcpp::traits::input_parameter< const Rcpp::NumericVector >::type position_vec(position_vecSEXP);
    rcpp_result_gen = Rcpp::wrap(rcpp_binseg_normal(data_vec, kmax, is_validation_vec, position_vec));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_binsegRcpp_rcpp_binseg_normal", (DL_FUNC) &_binsegRcpp_rcpp_binseg_normal, 4},
    {NULL, NULL, 0}
};

RcppExport void R_init_binsegRcpp(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
