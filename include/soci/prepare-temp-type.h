//
// Copyright (C) 2004-2008 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_PREPARE_TEMP_TYPE_INCLUDED
#define SOCI_PREPARE_TEMP_TYPE_INCLUDED

#include "soci/into-type.h"
#include "soci/use-type.h"
#include "soci/use.h"
#include "soci/ref-counted-prepare-info.h"

namespace soci
{

namespace details
{

// this needs to be lightweight and copyable
class SOCI_DECL prepare_temp_type
{
public:
    prepare_temp_type(session &);
    prepare_temp_type(prepare_temp_type const &);
    prepare_temp_type & operator=(prepare_temp_type const &);

    ~prepare_temp_type();

    template <typename T>
    prepare_temp_type & operator<<(T const & t)
    {
        rcpi_->accumulate(t);
        return *this;
    }

    prepare_temp_type & operator,(into_type_ptr const & i);

    template <typename T, typename Indicator>
    prepare_temp_type &operator,(into_container<T, Indicator> const &ic)
    {
        rcpi_->exchange(ic);
        return *this;
    }
    template <typename T, typename Indicator>
    prepare_temp_type &operator,(use_container<T, Indicator> const &uc)
    {
        rcpi_->exchange(uc);
        return *this;
    }

    ref_counted_prepare_info * get_prepare_info() const { return rcpi_; }

#if __cplusplus >= 201402L

private:

    template<std::size_t> struct int_{};

    template <typename T0, typename... Trest>
    prepare_temp_type &tuple_use(std::tuple<T0,Trest...> const &uc, int_<0>)
    {
        auto p = std::make_unique<details::c_use_container<T0>>(std::get<0>(uc), std::string());
        rcpi_->exchange(*p);
        uses_.push_back(std::move(p));
        return *this;
    }

    template <typename T0, typename... Trest, std::size_t I>
    prepare_temp_type &tuple_use(std::tuple<T0,Trest...> const &uc, int_<I>)
    {
        tuple_use(uc, int_<I-1>());

        auto p = std::make_unique<details::c_use_container<typename std::tuple_element<I, std::tuple<T0,Trest...>>::type>>(std::get<I>(uc), std::string());
        rcpi_->exchange(*p);
        uses_.push_back(std::move(p));
        return *this;
    }

public:

    template <typename T0, typename... Trest>
    prepare_temp_type &operator,(use_container<std::tuple<T0,Trest...>, no_indicator> const &uc)
    {
        tuple_use(uc.t, int_<sizeof...(Trest)>());
        return *this;
    }

#endif

private:

    std::vector<std::unique_ptr<details::use_container_base>> uses_;

private:
    ref_counted_prepare_info * rcpi_;
};

} // namespace details

} // namespace soci

#endif
