/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file util/HighsMatrixUtils.h
 * @brief Class-independent utilities for HiGHS
 */
#ifndef UTIL_HIGHSMATRIXUTILS_H_
#define UTIL_HIGHSMATRIXUTILS_H_

#include <cassert>
#include <vector>

// #include "lp_data/HighsStatus.h"
#include "lp_data/HighsOptions.h"

using std::vector;

HighsStatus assessMatrix(const HighsLogOptions& log_options,
                         const std::string matrix_name, const HighsInt vec_dim,
                         const HighsInt num_vec, vector<HighsInt>& matrix_start,
                         vector<HighsInt>& matrix_index,
                         vector<double>& matrix_value,
                         const double small_matrix_value,
                         const double large_matrix_value);

HighsStatus assessMatrix(const HighsLogOptions& log_options,
                         const std::string matrix_name, const HighsInt vec_dim,
                         const HighsInt num_vec, vector<HighsInt>& matrix_start,
                         vector<HighsInt>& matrix_p_end,
                         vector<HighsInt>& matrix_index,
                         vector<double>& matrix_value,
                         const double small_matrix_value,
                         const double large_matrix_value);

HighsStatus assessMatrix(
    const HighsLogOptions& log_options, const std::string matrix_name,
    const HighsInt vec_dim, const HighsInt num_vec, const bool partitioned,
    vector<HighsInt>& matrix_start, vector<HighsInt>& matrix_p_end,
    vector<HighsInt>& matrix_index, vector<double>& matrix_value,
    const double small_matrix_value, const double large_matrix_value);

HighsStatus assessMatrixDimensions(const HighsLogOptions& log_options,
                                   const HighsInt num_vec,
                                   const bool partitioned,
                                   const vector<HighsInt>& matrix_start,
                                   const vector<HighsInt>& matrix_p_end,
                                   const vector<HighsInt>& matrix_index,
                                   const vector<double>& matrix_value);

#endif  // UTIL_HIGHSMATRIXUTILS_H_
