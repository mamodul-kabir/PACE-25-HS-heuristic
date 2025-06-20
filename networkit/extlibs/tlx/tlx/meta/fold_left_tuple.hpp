/*******************************************************************************
 * tlx/meta/fold_left_tuple.hpp
 *
 * Part of tlx - http://panthema.net/tlx
 *
 * Copyright (C) 2018 Hung Tran <hung@ae.cs.uni-frankfurt.de>
 *
 * All rights reserved. Published under the Boost Software License, Version 1.0
 ******************************************************************************/

#ifndef TLX_META_FOLD_LEFT_TUPLE_HEADER
#define TLX_META_FOLD_LEFT_TUPLE_HEADER

#include <tlx/meta/fold_left.hpp>
#include <tlx/meta/index_sequence.hpp>
#include <cstddef>
#include <tuple>
#include <type_traits>

namespace tlx {

//! \addtogroup tlx_meta
//! \{

/******************************************************************************/
// Variadic Template Expander: Implements fold_left() on the components of a
// tuple.

namespace meta_detail {

//! helper for fold_left_tuple: forwards tuple entries
template <typename Reduce, typename Initial, typename Tuple, std::size_t... Is>
auto fold_left_tuple_impl(Reduce&& r, Initial&& init, Tuple&& t,
                          index_sequence<Is...>)
{
    return fold_left(std::forward<Reduce>(r), std::forward<Initial>(init),
                     std::get<Is>(std::forward<Tuple>(t))...);
}

} // namespace meta_detail

//! Implements fold_left() -- ((a * b) * c) -- with a binary Reduce operation
//! and initial value on a tuple.
template <typename Reduce, typename Initial, typename Tuple>
auto fold_left_tuple(Reduce&& r, Initial&& init, Tuple&& t)
{
    using Indices = make_index_sequence<
        std::tuple_size<typename std::decay<Tuple>::type>::value>;
    return meta_detail::fold_left_tuple_impl(std::forward<Reduce>(r),
                                             std::forward<Initial>(init),
                                             std::forward<Tuple>(t), Indices());
}

//! \}

} // namespace tlx

#endif // !TLX_META_FOLD_LEFT_TUPLE_HEADER

/******************************************************************************/
