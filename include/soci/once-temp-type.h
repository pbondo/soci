//
// Copyright (C) 2004-2016 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_ONCE_TEMP_TYPE_H_INCLUDED
#define SOCI_ONCE_TEMP_TYPE_H_INCLUDED

#include "soci/soci-platform.h"
#include "soci/ref-counted-statement.h"
#include "soci/prepare-temp-type.h"

namespace soci
{

class session;

namespace details
{

class ref_counted_statement;

// this needs to be lightweight and copyable
class SOCI_DECL once_temp_type
{
public:

    once_temp_type(session & s);
    once_temp_type(once_temp_type const & o);
    once_temp_type & operator=(once_temp_type const & o);

    ~once_temp_type() SOCI_NOEXCEPT_FALSE;

    template <typename T>
    once_temp_type & operator<<(T const & t)
    {
        rcst_->accumulate(t);
        return *this;
    }

    once_temp_type & operator,(into_type_ptr const &);
    once_temp_type & operator,(use_type_ptr const &);
    
    template <typename T, typename Indicator>
    once_temp_type &operator,(into_container<T, Indicator> const &ic)
    {
        rcst_->exchange(ic);
        return *this;
    }
    template <typename T, typename Indicator>
    once_temp_type &operator,(use_container<T, Indicator> const &uc)
    {
        rcst_->exchange(uc);
        return *this;
    }

#if __cplusplus >= 201402L

private:

    template<std::size_t> struct int_{};

    template <typename T0, typename... Trest>
    once_temp_type &tuple_use(std::tuple<T0,Trest...> const &uc, int_<0>)
    {
        auto p = std::make_unique<details::c_use_container<T0>>(std::get<0>(uc), std::string());
        rcst_->exchange(*p);
        uses_.push_back(std::move(p));
        return *this;
    }

    template <typename T0, typename... Trest, std::size_t I>
    once_temp_type &tuple_use(std::tuple<T0,Trest...> const &uc, int_<I>)
    {
        tuple_use(uc, int_<I-1>());

        auto p = std::make_unique<details::c_use_container<typename std::tuple_element<I, std::tuple<T0,Trest...>>::type>>(std::get<I>(uc), std::string());
        rcst_->exchange(*p);
        uses_.push_back(std::move(p));
        return *this;
    }

public:

    template <typename T0, typename... Trest>
    once_temp_type &operator,(use_container<std::tuple<T0,Trest...>, no_indicator> const &uc)
    {
        tuple_use(uc.t, int_<sizeof...(Trest)>());
        return *this;
    }

#endif

private:

    std::vector<std::unique_ptr<details::use_container_base>> uses_;

private:
    ref_counted_statement * rcst_;
};

// this needs to be lightweight and copyable
class once_type
{
public:
    once_type() : session_(NULL) {}
    once_type(session * s) : session_(s) {}

    void set_session(session * s)
    {
        session_ = s;
    }

    template <typename T>
    once_temp_type operator<<(T const & t)
    {
        once_temp_type o(*session_);
        o << t;
        return o;
    }

private:
    session * session_;
};


// this needs to be lightweight and copyable
class prepare_type
{
public:
    prepare_type() : session_(NULL) {}
    prepare_type(session * s) : session_(s) {}

    void set_session(session * s)
    {
        session_ = s;
    }

    template <typename T>
    prepare_temp_type operator<<(T const & t)
    {
        prepare_temp_type p(*session_);
        p << t;
        return p;
    }

private:
    session * session_;
};

} // namespace details

// Note: ddl_type is intended to be used just as once_temp_type,
// but since it can be also used directly (explicitly) by the user code,
// it is declared outside of the namespace details.
class SOCI_DECL ddl_type
{
public:

    ddl_type(session & s);
    ddl_type(const ddl_type & d);
    ddl_type & operator=(const ddl_type & d);

    ~ddl_type() SOCI_NOEXCEPT_FALSE;

    void create_table(const std::string & tableName);
    void add_column(const std::string & tableName,
        const std::string & columnName, data_type dt,
        int precision, int scale);
    void alter_column(const std::string & tableName,
        const std::string & columnName, data_type dt,
        int precision, int scale);
    void drop_column(const std::string & tableName,
        const std::string & columnName);
    ddl_type & column(const std::string & columnName, data_type dt,
        int precision = 0, int scale = 0);
    ddl_type & unique(const std::string & name,
        const std::string & columnNames);
    ddl_type & primary_key(const std::string & name,
        const std::string & columnNames);
    ddl_type & foreign_key(const std::string & name,
        const std::string & columnNames,
        const std::string & refTableName,
        const std::string & refColumnNames);

    ddl_type & operator()(const std::string & arbitrarySql);

    // helper function for handling delimiters
    // between various parts of DDL statements
    void set_tail(const std::string & tail);

private:
    session * s_;
    details::ref_counted_statement * rcst_;
};

} // namespace soci

#endif
