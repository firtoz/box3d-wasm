       
       
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;
typedef long int __quad_t;
typedef unsigned long int __u_quad_t;
typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;
typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;
typedef int __daddr_t;
typedef int __key_t;
typedef int __clockid_t;
typedef void * __timer_t;
typedef long int __blksize_t;
typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;
typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;
typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;
typedef long int __fsword_t;
typedef long int __ssize_t;
typedef long int __syscall_slong_t;
typedef unsigned long int __syscall_ulong_t;
typedef __off64_t __loff_t;
typedef char *__caddr_t;
typedef long int __intptr_t;
typedef unsigned int __socklen_t;
typedef int __sig_atomic_t;
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;
typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;
typedef signed char int_fast8_t;
typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
typedef unsigned char uint_fast8_t;
typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
typedef long int intptr_t;
typedef unsigned long int uintptr_t;
typedef __intmax_t intmax_t;
typedef __uintmax_t uintmax_t;
#pragma GCC diagnostic pop
       
typedef void* b3AllocFcn( int32_t size, int32_t alignment );
typedef void b3FreeFcn( void* mem );
typedef int b3AssertFcn( const char* condition, const char* fileName, int lineNumber );
typedef void b3LogFcn( const char* message );
extern "C" void b3SetAllocator( b3AllocFcn* allocFcn, b3FreeFcn* freeFcn );
extern "C" int b3GetByteCount( void );
extern "C" void b3SetAssertFcn( b3AssertFcn* assertFcn );
extern "C" int b3InternalAssert( const char* condition, const char* fileName, int lineNumber );
extern "C" void b3SetLogFcn( b3LogFcn* logFcn );
typedef struct b3Version
{
 int major;
 int minor;
 int revision;
} b3Version;
extern "C" b3Version b3GetVersion( void );
extern "C" bool b3IsDoublePrecision( void );
extern "C" uint64_t b3GetTicks( void );
extern "C" float b3GetMilliseconds( uint64_t ticks );
extern "C" float b3GetMillisecondsAndReset( uint64_t* ticks );
extern "C" void b3Yield( void );
extern "C" void b3Sleep( int milliseconds );
extern "C" uint32_t b3Hash( uint32_t hash, const uint8_t* data, int count );
       
       
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wc++11-extensions"
#pragma GCC diagnostic ignored "-Wc++23-extensions"
namespace std
{
  typedef long unsigned int size_t;
  typedef long int ptrdiff_t;
  typedef decltype(nullptr) nullptr_t;
#pragma GCC visibility push(default)
  extern "C++" __attribute__ ((__noreturn__, __always_inline__))
  inline void __terminate() noexcept
  {
    void terminate() noexcept __attribute__ ((__noreturn__,__cold__));
    terminate();
  }
#pragma GCC visibility pop
}
namespace std
{
  inline namespace __cxx11 __attribute__((__abi_tag__ ("cxx11"))) { }
}
namespace __gnu_cxx
{
  inline namespace __cxx11 __attribute__((__abi_tag__ ("cxx11"))) { }
}
namespace std
{
#pragma GCC visibility push(default)
  __attribute__((__always_inline__))
  constexpr inline bool
  __is_constant_evaluated() noexcept
  {
    return __builtin_is_constant_evaluated();
  }
#pragma GCC visibility pop
}
namespace std
{
#pragma GCC visibility push(default)
  extern "C++" __attribute__ ((__noreturn__)) __attribute__((__cold__))
  void
  __glibcxx_assert_fail
    (const char* __file, int __line, const char* __function,
     const char* __condition)
  noexcept;
#pragma GCC visibility pop
}
namespace __gnu_cxx
{
  typedef __decltype(0.0bf16) __bfloat16_t;
}
#pragma GCC diagnostic pop
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{

  template<typename _Tp>
    class reference_wrapper;
  template<typename _Tp, _Tp __v>
    struct integral_constant
    {
      static constexpr _Tp value = __v;
      using value_type = _Tp;
      using type = integral_constant<_Tp, __v>;
      constexpr operator value_type() const noexcept { return value; }
      constexpr value_type operator()() const noexcept { return value; }
    };
  template<bool __v>
    using __bool_constant = integral_constant<bool, __v>;
  using true_type = __bool_constant<true>;
  using false_type = __bool_constant<false>;
  template<bool __v>
    using bool_constant = __bool_constant<__v>;
  template<bool, typename _Tp = void>
    struct enable_if
    { };
  template<typename _Tp>
    struct enable_if<true, _Tp>
    { using type = _Tp; };
  template<bool _Cond, typename _Tp = void>
    using __enable_if_t = typename enable_if<_Cond, _Tp>::type;
  template<bool>
    struct __conditional
    {
      template<typename _Tp, typename>
 using type = _Tp;
    };
  template<>
    struct __conditional<false>
    {
      template<typename, typename _Up>
 using type = _Up;
    };
  template<bool _Cond, typename _If, typename _Else>
    using __conditional_t
      = typename __conditional<_Cond>::template type<_If, _Else>;
  template <typename _Type>
    struct __type_identity
    { using type = _Type; };
  template<typename _Tp>
    using __type_identity_t = typename __type_identity<_Tp>::type;
  namespace __detail
  {
    template<typename _Tp, typename...>
      using __first_t = _Tp;
    template<typename... _Bn>
      auto __or_fn(int) -> __first_t<false_type,
         __enable_if_t<!bool(_Bn::value)>...>;
    template<typename... _Bn>
      auto __or_fn(...) -> true_type;
    template<typename... _Bn>
      auto __and_fn(int) -> __first_t<true_type,
          __enable_if_t<bool(_Bn::value)>...>;
    template<typename... _Bn>
      auto __and_fn(...) -> false_type;
  }
  template<typename... _Bn>
    struct __or_
    : decltype(__detail::__or_fn<_Bn...>(0))
    { };
  template<typename... _Bn>
    struct __and_
    : decltype(__detail::__and_fn<_Bn...>(0))
    { };
  template<typename _Pp>
    struct __not_
    : __bool_constant<!bool(_Pp::value)>
    { };
  template<typename... _Bn>
    inline constexpr bool __or_v = __or_<_Bn...>::value;
  template<typename... _Bn>
    inline constexpr bool __and_v = __and_<_Bn...>::value;
  namespace __detail
  {
    template<typename , typename _B1, typename... _Bn>
      struct __disjunction_impl
      { using type = _B1; };
    template<typename _B1, typename _B2, typename... _Bn>
      struct __disjunction_impl<__enable_if_t<!bool(_B1::value)>, _B1, _B2, _Bn...>
      { using type = typename __disjunction_impl<void, _B2, _Bn...>::type; };
    template<typename , typename _B1, typename... _Bn>
      struct __conjunction_impl
      { using type = _B1; };
    template<typename _B1, typename _B2, typename... _Bn>
      struct __conjunction_impl<__enable_if_t<bool(_B1::value)>, _B1, _B2, _Bn...>
      { using type = typename __conjunction_impl<void, _B2, _Bn...>::type; };
  }
  template<typename... _Bn>
    struct conjunction
    : __detail::__conjunction_impl<void, _Bn...>::type
    { };
  template<>
    struct conjunction<>
    : true_type
    { };
  template<typename... _Bn>
    struct disjunction
    : __detail::__disjunction_impl<void, _Bn...>::type
    { };
  template<>
    struct disjunction<>
    : false_type
    { };
  template<typename _Pp>
    struct negation
    : __not_<_Pp>::type
    { };
  template<typename... _Bn>
    inline constexpr bool conjunction_v = conjunction<_Bn...>::value;
  template<typename... _Bn>
    inline constexpr bool disjunction_v = disjunction<_Bn...>::value;
  template<typename _Pp>
    inline constexpr bool negation_v = negation<_Pp>::value;
  template<typename>
    struct is_object;
  template<typename>
    struct remove_cv;
  template<typename>
    struct is_const;
  template<typename>
    struct __is_array_unknown_bounds;
  template<typename _Tp>
    using __maybe_complete_object_type
      = __and_<is_object<_Tp>, __not_<__is_array_unknown_bounds<_Tp>>>;
  template<typename _Tp,
    typename = __enable_if_t<__maybe_complete_object_type<_Tp>::value>,
    size_t = sizeof(_Tp)>
    constexpr true_type
    __is_complete_or_unbounded(__type_identity<_Tp>)
    { return {}; };
  template<typename _TypeIdentity,
    typename _NestedType = typename _TypeIdentity::type>
    constexpr typename __not_<__maybe_complete_object_type<_NestedType>>::type
    __is_complete_or_unbounded(_TypeIdentity)
    { return {}; }
  template<typename _Tp>
    using __remove_cv_t = typename remove_cv<_Tp>::type;
  template<typename _Tp>
    struct is_void
    : public false_type { };
  template<>
    struct is_void<void>
    : public true_type { };
  template<>
    struct is_void<const void>
    : public true_type { };
  template<>
    struct is_void<volatile void>
    : public true_type { };
  template<>
    struct is_void<const volatile void>
    : public true_type { };
  enum class _Integer_kind { _None, _Signed, _Unsigned };
  template<typename>
    struct __is_integral_helper
    : public false_type
    { static constexpr auto _S_kind = _Integer_kind::_None; };
  template<>
    struct __is_integral_helper<bool>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_None; };
  template<>
    struct __is_integral_helper<char>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_None; };
  template<>
    struct __is_integral_helper<signed char>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Signed; };
  template<>
    struct __is_integral_helper<unsigned char>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Unsigned; };
  template<>
    struct __is_integral_helper<wchar_t>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_None; };
  template<>
    struct __is_integral_helper<char16_t>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_None; };
  template<>
    struct __is_integral_helper<char32_t>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_None; };
  template<>
    struct __is_integral_helper<short>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Signed; };
  template<>
    struct __is_integral_helper<unsigned short>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Unsigned; };
  template<>
    struct __is_integral_helper<int>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Signed; };
  template<>
    struct __is_integral_helper<unsigned int>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Unsigned; };
  template<>
    struct __is_integral_helper<long>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Signed; };
  template<>
    struct __is_integral_helper<unsigned long>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Unsigned; };
  template<>
    struct __is_integral_helper<long long>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Signed; };
  template<>
    struct __is_integral_helper<unsigned long long>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Unsigned; };
  __extension__
  template<>
    struct __is_integral_helper<__int128>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Signed; };
  __extension__
  template<>
    struct __is_integral_helper<unsigned __int128>
    : public true_type
    { static constexpr auto _S_kind = _Integer_kind::_Unsigned; };
  template<typename _Tp>
    using __is_signed_integer
      = __bool_constant<__is_integral_helper<_Tp>::_S_kind
     == _Integer_kind::_Signed>;
  template<typename _Tp>
    using __is_unsigned_integer
      = __bool_constant<__is_integral_helper<_Tp>::_S_kind
     == _Integer_kind::_Unsigned>;
  template<typename _Tp>
    using __is_signed_or_unsigned_integer
      = __bool_constant<__is_integral_helper<_Tp>::_S_kind
     != _Integer_kind::_None>;
  template<typename _Tp>
    struct is_integral
    : public __is_integral_helper<__remove_cv_t<_Tp>>::type
    { };
  template<typename>
    struct __is_floating_point_helper
    : public false_type { };
  template<>
    struct __is_floating_point_helper<float>
    : public true_type { };
  template<>
    struct __is_floating_point_helper<double>
    : public true_type { };
  template<>
    struct __is_floating_point_helper<long double>
    : public true_type { };
  template<>
    struct __is_floating_point_helper<__float128>
    : public true_type { };
  template<typename _Tp>
    struct is_floating_point
    : public __is_floating_point_helper<__remove_cv_t<_Tp>>::type
    { };
  template<typename _Tp>
    struct is_array
    : public __bool_constant<__is_array(_Tp)>
    { };
  template<typename _Tp>
    struct is_pointer
    : public __bool_constant<__is_pointer(_Tp)>
    { };
  template<typename>
    struct is_lvalue_reference
    : public false_type { };
  template<typename _Tp>
    struct is_lvalue_reference<_Tp&>
    : public true_type { };
  template<typename>
    struct is_rvalue_reference
    : public false_type { };
  template<typename _Tp>
    struct is_rvalue_reference<_Tp&&>
    : public true_type { };
  template<typename _Tp>
    struct is_member_object_pointer
    : public __bool_constant<__is_member_object_pointer(_Tp)>
    { };
  template<typename _Tp>
    struct is_member_function_pointer
    : public __bool_constant<__is_member_function_pointer(_Tp)>
    { };
  template<typename _Tp>
    struct is_enum
    : public __bool_constant<__is_enum(_Tp)>
    { };
  template<typename _Tp>
    struct is_union
    : public __bool_constant<__is_union(_Tp)>
    { };
  template<typename _Tp>
    struct is_class
    : public __bool_constant<__is_class(_Tp)>
    { };
  template<typename _Tp>
    struct is_function
    : public __bool_constant<__is_function(_Tp)>
    { };
  template<typename _Tp>
    struct is_null_pointer
    : public false_type { };
  template<>
    struct is_null_pointer<std::nullptr_t>
    : public true_type { };
  template<>
    struct is_null_pointer<const std::nullptr_t>
    : public true_type { };
  template<>
    struct is_null_pointer<volatile std::nullptr_t>
    : public true_type { };
  template<>
    struct is_null_pointer<const volatile std::nullptr_t>
    : public true_type { };
  template<typename _Tp>
    struct __is_nullptr_t
    : public is_null_pointer<_Tp>
    { } __attribute__ ((__deprecated__ ("use '" "std::is_null_pointer" "' instead")));
  template<typename _Tp>
    struct is_reference
    : public __bool_constant<__is_reference(_Tp)>
    { };
  template<typename _Tp>
    struct is_arithmetic
    : public __or_<is_integral<_Tp>, is_floating_point<_Tp>>::type
    { };
  template<typename _Tp>
    struct is_fundamental
    : public __or_<is_arithmetic<_Tp>, is_void<_Tp>,
     is_null_pointer<_Tp>
     >::type
    { };
  template<typename _Tp>
    struct is_object
    : public __bool_constant<__is_object(_Tp)>
    { };
  template<typename>
    struct is_member_pointer;
  template<typename _Tp>
    struct is_scalar
    : public __or_<is_arithmetic<_Tp>, is_enum<_Tp>, is_pointer<_Tp>,
                   is_member_pointer<_Tp>, is_null_pointer<_Tp>
     >::type
    { };
  template<typename _Tp>
    struct is_compound
    : public __bool_constant<!is_fundamental<_Tp>::value> { };
  template<typename _Tp>
    struct is_member_pointer
    : public __bool_constant<__is_member_pointer(_Tp)>
    { };
  template<typename, typename>
    struct is_same;
  template<typename _Tp, typename... _Types>
    using __is_one_of = __or_<is_same<_Tp, _Types>...>;
  template<typename...> using __void_t = void;
  template<typename _Tp>
    struct is_const
    : public __bool_constant<__is_const(_Tp)>
    { };
  template<typename _Tp>
    struct is_volatile
    : public __bool_constant<__is_volatile(_Tp)>
    { };
  template<typename _Tp>
    struct
   
    is_trivial
    : public __bool_constant<__is_trivial(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_trivially_copyable
    : public __bool_constant<__is_trivially_copyable(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_standard_layout
    : public __bool_constant<__is_standard_layout(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct
   
    is_pod
    : public __bool_constant<__is_pod(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct
    [[__deprecated__]]
    is_literal_type
    : public __bool_constant<__is_literal_type(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_empty
    : public __bool_constant<__is_empty(_Tp)>
    { };
  template<typename _Tp>
    struct is_polymorphic
    : public __bool_constant<__is_polymorphic(_Tp)>
    { };
  template<typename _Tp>
    struct is_final
    : public __bool_constant<__is_final(_Tp)>
    { };
  template<typename _Tp>
    struct is_abstract
    : public __bool_constant<__is_abstract(_Tp)>
    { };
  template<typename _Tp,
    bool = is_arithmetic<_Tp>::value>
    struct __is_signed_helper
    : public false_type { };
  template<typename _Tp>
    struct __is_signed_helper<_Tp, true>
    : public __bool_constant<_Tp(-1) < _Tp(0)>
    { };
  template<typename _Tp>
    struct is_signed
    : public __is_signed_helper<_Tp>::type
    { };
  template<typename _Tp>
    struct is_unsigned
    : public __and_<is_arithmetic<_Tp>, __not_<is_signed<_Tp>>>::type
    { };
  template<typename _Tp, typename _Up = _Tp&&>
    _Up
    __declval(int);
  template<typename _Tp>
    _Tp
    __declval(long);
  template<typename _Tp>
    auto declval() noexcept -> decltype(__declval<_Tp>(0));
  template<typename>
    struct remove_all_extents;
  template<typename _Tp>
    struct __is_array_known_bounds
    : public false_type
    { };
  template<typename _Tp, size_t _Size>
    struct __is_array_known_bounds<_Tp[_Size]>
    : public true_type
    { };
  template<typename _Tp>
    struct __is_array_unknown_bounds
    : public false_type
    { };
  template<typename _Tp>
    struct __is_array_unknown_bounds<_Tp[]>
    : public true_type
    { };
  template<typename _Tp>
    struct is_destructible
    : public __bool_constant<__is_destructible(_Tp)>
    { };
  template<typename _Tp>
    struct is_nothrow_destructible
    : public __bool_constant<__is_nothrow_destructible(_Tp)>
    { };
  template<typename _Tp, typename... _Args>
    using __is_constructible_impl
      = __bool_constant<__is_constructible(_Tp, _Args...)>;
  template<typename _Tp, typename... _Args>
    struct is_constructible
      : public __is_constructible_impl<_Tp, _Args...>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_default_constructible
    : public __is_constructible_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    using __add_lval_ref_t = __add_lvalue_reference(_Tp);
  template<typename _Tp>
    struct is_copy_constructible
    : public __is_constructible_impl<_Tp, __add_lval_ref_t<const _Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    using __add_rval_ref_t = __add_rvalue_reference(_Tp);
  template<typename _Tp>
    struct is_move_constructible
    : public __is_constructible_impl<_Tp, __add_rval_ref_t<_Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp, typename... _Args>
    using __is_nothrow_constructible_impl
      = __bool_constant<__is_nothrow_constructible(_Tp, _Args...)>;
  template<typename _Tp, typename... _Args>
    struct is_nothrow_constructible
    : public __is_nothrow_constructible_impl<_Tp, _Args...>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_nothrow_default_constructible
    : public __is_nothrow_constructible_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_nothrow_copy_constructible
    : public __is_nothrow_constructible_impl<_Tp, __add_lval_ref_t<const _Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_nothrow_move_constructible
    : public __is_nothrow_constructible_impl<_Tp, __add_rval_ref_t<_Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp, typename _Up>
    using __is_assignable_impl = __bool_constant<__is_assignable(_Tp, _Up)>;
  template<typename _Tp, typename _Up>
    struct is_assignable
    : public __is_assignable_impl<_Tp, _Up>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_copy_assignable
    : public __is_assignable_impl<__add_lval_ref_t<_Tp>,
      __add_lval_ref_t<const _Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_move_assignable
    : public __is_assignable_impl<__add_lval_ref_t<_Tp>, __add_rval_ref_t<_Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp, typename _Up>
    using __is_nothrow_assignable_impl
      = __bool_constant<__is_nothrow_assignable(_Tp, _Up)>;
  template<typename _Tp, typename _Up>
    struct is_nothrow_assignable
    : public __is_nothrow_assignable_impl<_Tp, _Up>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_nothrow_copy_assignable
    : public __is_nothrow_assignable_impl<__add_lval_ref_t<_Tp>,
       __add_lval_ref_t<const _Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_nothrow_move_assignable
    : public __is_nothrow_assignable_impl<__add_lval_ref_t<_Tp>,
       __add_rval_ref_t<_Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp, typename... _Args>
    using __is_trivially_constructible_impl
      = __bool_constant<__is_trivially_constructible(_Tp, _Args...)>;
  template<typename _Tp, typename... _Args>
    struct is_trivially_constructible
    : public __is_trivially_constructible_impl<_Tp, _Args...>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_trivially_default_constructible
    : public __is_trivially_constructible_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  struct __do_is_implicitly_default_constructible_impl
  {
    template <typename _Tp>
    static void __helper(const _Tp&);
    template <typename _Tp>
    static true_type __test(const _Tp&,
                            decltype(__helper<const _Tp&>({}))* = 0);
    static false_type __test(...);
  };
  template<typename _Tp>
    struct __is_implicitly_default_constructible_impl
    : public __do_is_implicitly_default_constructible_impl
    {
      using type = decltype(__test(declval<_Tp>()));
    };
  template<typename _Tp>
    struct __is_implicitly_default_constructible_safe
    : public __is_implicitly_default_constructible_impl<_Tp>::type
    { };
  template <typename _Tp>
    struct __is_implicitly_default_constructible
    : public __and_<__is_constructible_impl<_Tp>,
      __is_implicitly_default_constructible_safe<_Tp>>::type
    { };
  template<typename _Tp>
    struct is_trivially_copy_constructible
    : public __is_trivially_constructible_impl<_Tp, __add_lval_ref_t<const _Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_trivially_move_constructible
    : public __is_trivially_constructible_impl<_Tp, __add_rval_ref_t<_Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp, typename _Up>
    using __is_trivially_assignable_impl
      = __bool_constant<__is_trivially_assignable(_Tp, _Up)>;
  template<typename _Tp, typename _Up>
    struct is_trivially_assignable
    : public __is_trivially_assignable_impl<_Tp, _Up>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_trivially_copy_assignable
    : public __is_trivially_assignable_impl<__add_lval_ref_t<_Tp>,
         __add_lval_ref_t<const _Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_trivially_move_assignable
    : public __is_trivially_assignable_impl<__add_lval_ref_t<_Tp>,
         __add_rval_ref_t<_Tp>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_trivially_destructible
    : public __bool_constant<__is_trivially_destructible(_Tp)>
    { };
  template<typename _Tp>
    struct has_virtual_destructor
    : public __bool_constant<__has_virtual_destructor(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct alignment_of
    : public integral_constant<std::size_t, alignof(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct rank
    : public integral_constant<std::size_t, __array_rank(_Tp)> { };
  template<typename, unsigned _Uint = 0>
    struct extent
    : public integral_constant<size_t, 0> { };
  template<typename _Tp, size_t _Size>
    struct extent<_Tp[_Size], 0>
    : public integral_constant<size_t, _Size> { };
  template<typename _Tp, unsigned _Uint, size_t _Size>
    struct extent<_Tp[_Size], _Uint>
    : public extent<_Tp, _Uint - 1>::type { };
  template<typename _Tp>
    struct extent<_Tp[], 0>
    : public integral_constant<size_t, 0> { };
  template<typename _Tp, unsigned _Uint>
    struct extent<_Tp[], _Uint>
    : public extent<_Tp, _Uint - 1>::type { };
  template<typename _Tp, typename _Up>
    struct is_same
    : public __bool_constant<__is_same(_Tp, _Up)>
    { };
  template<typename _Base, typename _Derived>
    struct is_base_of
    : public __bool_constant<__is_base_of(_Base, _Derived)>
    { };
  template<typename _From, typename _To>
    struct is_convertible
    : public __bool_constant<__is_convertible(_From, _To)>
    { };
  template<typename _ToElementType, typename _FromElementType>
    using __is_array_convertible
      = is_convertible<_FromElementType(*)[], _ToElementType(*)[]>;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++14-extensions"
  template<typename _Tp, typename... _Args>
    struct __is_nothrow_new_constructible_impl
    : __bool_constant<
 noexcept(::new(std::declval<void*>()) _Tp(std::declval<_Args>()...))
      >
    { };
  template<typename _Tp, typename... _Args>
    inline constexpr bool __is_nothrow_new_constructible
      = __and_<is_constructible<_Tp, _Args...>,
        __is_nothrow_new_constructible_impl<_Tp, _Args...>>::value;
#pragma GCC diagnostic pop
  template<typename _Tp>
    struct remove_const
    { using type = _Tp; };
  template<typename _Tp>
    struct remove_const<_Tp const>
    { using type = _Tp; };
  template<typename _Tp>
    struct remove_volatile
    { using type = _Tp; };
  template<typename _Tp>
    struct remove_volatile<_Tp volatile>
    { using type = _Tp; };
  template<typename _Tp>
    struct remove_cv
    { using type = __remove_cv(_Tp); };
  template<typename _Tp>
    struct add_const
    { using type = _Tp const; };
  template<typename _Tp>
    struct add_volatile
    { using type = _Tp volatile; };
  template<typename _Tp>
    struct add_cv
    { using type = _Tp const volatile; };
  template<typename _Tp>
    using remove_const_t = typename remove_const<_Tp>::type;
  template<typename _Tp>
    using remove_volatile_t = typename remove_volatile<_Tp>::type;
  template<typename _Tp>
    using remove_cv_t = typename remove_cv<_Tp>::type;
  template<typename _Tp>
    using add_const_t = typename add_const<_Tp>::type;
  template<typename _Tp>
    using add_volatile_t = typename add_volatile<_Tp>::type;
  template<typename _Tp>
    using add_cv_t = typename add_cv<_Tp>::type;
  template<typename _Tp>
    struct remove_reference
    { using type = __remove_reference(_Tp); };
  template<typename _Tp>
    struct add_lvalue_reference
    { using type = __add_lval_ref_t<_Tp>; };
  template<typename _Tp>
    struct add_rvalue_reference
    { using type = __add_rval_ref_t<_Tp>; };
  template<typename _Tp>
    using remove_reference_t = typename remove_reference<_Tp>::type;
  template<typename _Tp>
    using add_lvalue_reference_t = typename add_lvalue_reference<_Tp>::type;
  template<typename _Tp>
    using add_rvalue_reference_t = typename add_rvalue_reference<_Tp>::type;
  template<typename _Unqualified, bool _IsConst, bool _IsVol>
    struct __cv_selector;
  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, false, false>
    { using __type = _Unqualified; };
  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, false, true>
    { using __type = volatile _Unqualified; };
  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, true, false>
    { using __type = const _Unqualified; };
  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, true, true>
    { using __type = const volatile _Unqualified; };
  template<typename _Qualified, typename _Unqualified,
    bool _IsConst = is_const<_Qualified>::value,
    bool _IsVol = is_volatile<_Qualified>::value>
    class __match_cv_qualifiers
    {
      using __match = __cv_selector<_Unqualified, _IsConst, _IsVol>;
    public:
      using __type = typename __match::__type;
    };
  template<typename _Tp>
    struct __make_unsigned
    { using __type = _Tp; };
  template<>
    struct __make_unsigned<char>
    { using __type = unsigned char; };
  template<>
    struct __make_unsigned<signed char>
    { using __type = unsigned char; };
  template<>
    struct __make_unsigned<short>
    { using __type = unsigned short; };
  template<>
    struct __make_unsigned<int>
    { using __type = unsigned int; };
  template<>
    struct __make_unsigned<long>
    { using __type = unsigned long; };
  template<>
    struct __make_unsigned<long long>
    { using __type = unsigned long long; };
  __extension__
  template<>
    struct __make_unsigned<__int128>
    { using __type = unsigned __int128; };
  template<typename _Tp,
    bool _IsInt = is_integral<_Tp>::value,
    bool _IsEnum = __is_enum(_Tp)>
    class __make_unsigned_selector;
  template<typename _Tp>
    class __make_unsigned_selector<_Tp, true, false>
    {
      using __unsigned_type
 = typename __make_unsigned<__remove_cv_t<_Tp>>::__type;
    public:
      using __type
 = typename __match_cv_qualifiers<_Tp, __unsigned_type>::__type;
    };
  class __make_unsigned_selector_base
  {
  protected:
    template<typename...> struct _List { };
    template<typename _Tp, typename... _Up>
      struct _List<_Tp, _Up...> : _List<_Up...>
      { static constexpr size_t __size = sizeof(_Tp); };
    template<size_t _Sz, typename _Tp, bool = (_Sz <= _Tp::__size)>
      struct __select;
    template<size_t _Sz, typename _Uint, typename... _UInts>
      struct __select<_Sz, _List<_Uint, _UInts...>, true>
      { using __type = _Uint; };
    template<size_t _Sz, typename _Uint, typename... _UInts>
      struct __select<_Sz, _List<_Uint, _UInts...>, false>
      : __select<_Sz, _List<_UInts...>>
      { };
  };
  template<typename _Tp>
    class __make_unsigned_selector<_Tp, false, true>
    : __make_unsigned_selector_base
    {
      __extension__
      using _UInts = _List<unsigned char, unsigned short, unsigned int,
      unsigned long, unsigned long long
      , unsigned __int128
     >;
      using __unsigned_type = typename __select<sizeof(_Tp), _UInts>::__type;
    public:
      using __type
 = typename __match_cv_qualifiers<_Tp, __unsigned_type>::__type;
    };
  template<>
    struct __make_unsigned<wchar_t>
    {
      using __type
 = typename __make_unsigned_selector<wchar_t, false, true>::__type;
    };
  template<>
    struct __make_unsigned<char16_t>
    {
      using __type
 = typename __make_unsigned_selector<char16_t, false, true>::__type;
    };
  template<>
    struct __make_unsigned<char32_t>
    {
      using __type
 = typename __make_unsigned_selector<char32_t, false, true>::__type;
    };
  template<typename _Tp>
    struct make_unsigned
    { using type = typename __make_unsigned_selector<_Tp>::__type; };
  template<> struct make_unsigned<bool>;
  template<> struct make_unsigned<bool const>;
  template<> struct make_unsigned<bool volatile>;
  template<> struct make_unsigned<bool const volatile>;
  template<typename _Tp>
    struct __make_signed
    { using __type = _Tp; };
  template<>
    struct __make_signed<char>
    { using __type = signed char; };
  template<>
    struct __make_signed<unsigned char>
    { using __type = signed char; };
  template<>
    struct __make_signed<unsigned short>
    { using __type = signed short; };
  template<>
    struct __make_signed<unsigned int>
    { using __type = signed int; };
  template<>
    struct __make_signed<unsigned long>
    { using __type = signed long; };
  template<>
    struct __make_signed<unsigned long long>
    { using __type = signed long long; };
  __extension__
  template<>
    struct __make_signed<unsigned __int128>
    { using __type = __int128; };
  template<typename _Tp,
    bool _IsInt = is_integral<_Tp>::value,
    bool _IsEnum = __is_enum(_Tp)>
    class __make_signed_selector;
  template<typename _Tp>
    class __make_signed_selector<_Tp, true, false>
    {
      using __signed_type
 = typename __make_signed<__remove_cv_t<_Tp>>::__type;
    public:
      using __type
 = typename __match_cv_qualifiers<_Tp, __signed_type>::__type;
    };
  template<typename _Tp>
    class __make_signed_selector<_Tp, false, true>
    {
      using __unsigned_type = typename __make_unsigned_selector<_Tp>::__type;
    public:
      using __type = typename __make_signed_selector<__unsigned_type>::__type;
    };
  template<>
    struct __make_signed<wchar_t>
    {
      using __type
 = typename __make_signed_selector<wchar_t, false, true>::__type;
    };
  template<>
    struct __make_signed<char16_t>
    {
      using __type
 = typename __make_signed_selector<char16_t, false, true>::__type;
    };
  template<>
    struct __make_signed<char32_t>
    {
      using __type
 = typename __make_signed_selector<char32_t, false, true>::__type;
    };
  template<typename _Tp>
    struct make_signed
    { using type = typename __make_signed_selector<_Tp>::__type; };
  template<> struct make_signed<bool>;
  template<> struct make_signed<bool const>;
  template<> struct make_signed<bool volatile>;
  template<> struct make_signed<bool const volatile>;
  template<typename _Tp>
    using make_signed_t = typename make_signed<_Tp>::type;
  template<typename _Tp>
    using make_unsigned_t = typename make_unsigned<_Tp>::type;
  template<typename _Tp>
    struct remove_extent
    { using type = __remove_extent(_Tp); };
  template<typename _Tp>
    struct remove_all_extents
    { using type = __remove_all_extents(_Tp); };
  template<typename _Tp>
    using remove_extent_t = typename remove_extent<_Tp>::type;
  template<typename _Tp>
    using remove_all_extents_t = typename remove_all_extents<_Tp>::type;
  template<typename _Tp>
    struct remove_pointer
    { using type = __remove_pointer(_Tp); };
  template<typename _Tp>
    struct add_pointer
    { using type = __add_pointer(_Tp); };
  template<typename _Tp>
    using remove_pointer_t = typename remove_pointer<_Tp>::type;
  template<typename _Tp>
    using add_pointer_t = typename add_pointer<_Tp>::type;
  struct __attribute__((__aligned__)) __aligned_storage_max_align_t
  { };
  constexpr size_t
  __aligned_storage_default_alignment([[__maybe_unused__]] size_t __len)
  {
    return alignof(__aligned_storage_max_align_t);
  }
  template<size_t _Len,
    size_t _Align = __aligned_storage_default_alignment(_Len)>
    struct
   
    aligned_storage
    {
      struct type
      {
 alignas(_Align) unsigned char __data[_Len];
      };
    };
  template <typename... _Types>
    struct __strictest_alignment
    {
      static const size_t _S_alignment = 0;
      static const size_t _S_size = 0;
    };
  template <typename _Tp, typename... _Types>
    struct __strictest_alignment<_Tp, _Types...>
    {
      static const size_t _S_alignment =
        alignof(_Tp) > __strictest_alignment<_Types...>::_S_alignment
 ? alignof(_Tp) : __strictest_alignment<_Types...>::_S_alignment;
      static const size_t _S_size =
        sizeof(_Tp) > __strictest_alignment<_Types...>::_S_size
 ? sizeof(_Tp) : __strictest_alignment<_Types...>::_S_size;
    };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template <size_t _Len, typename... _Types>
    struct
   
    aligned_union
    {
    private:
      static_assert(sizeof...(_Types) != 0, "At least one type is required");
      using __strictest = __strictest_alignment<_Types...>;
      static const size_t _S_len = _Len > __strictest::_S_size
 ? _Len : __strictest::_S_size;
    public:
      static const size_t alignment_value = __strictest::_S_alignment;
      using type = typename aligned_storage<_S_len, alignment_value>::type;
    };
  template <size_t _Len, typename... _Types>
    const size_t aligned_union<_Len, _Types...>::alignment_value;
#pragma GCC diagnostic pop
  template<typename _Tp>
    struct decay
    { using type = __decay(_Tp); };
  template<typename _Tp>
    struct __strip_reference_wrapper
    {
      using __type = _Tp;
    };
  template<typename _Tp>
    struct __strip_reference_wrapper<reference_wrapper<_Tp> >
    {
      using __type = _Tp&;
    };
  template<typename _Tp>
    using __decay_t = typename decay<_Tp>::type;
  template<typename _Tp>
    using __decay_and_strip = __strip_reference_wrapper<__decay_t<_Tp>>;
  template<typename... _Cond>
    using _Require = __enable_if_t<__and_<_Cond...>::value>;
  template<typename _Tp>
    using __remove_cvref_t
     = typename remove_cv<typename remove_reference<_Tp>::type>::type;
  template<bool _Cond, typename _Iftrue, typename _Iffalse>
    struct conditional
    { using type = _Iftrue; };
  template<typename _Iftrue, typename _Iffalse>
    struct conditional<false, _Iftrue, _Iffalse>
    { using type = _Iffalse; };
  template<typename... _Tp>
    struct common_type;
  template<typename _Tp>
    struct __success_type
    { using type = _Tp; };
  struct __failure_type
  { };
  struct __do_common_type_impl
  {
    template<typename _Tp, typename _Up>
      using __cond_t
 = decltype(true ? std::declval<_Tp>() : std::declval<_Up>());
    template<typename _Tp, typename _Up>
      static __success_type<__decay_t<__cond_t<_Tp, _Up>>>
      _S_test(int);
    template<typename, typename>
      static __failure_type
      _S_test_2(...);
    template<typename _Tp, typename _Up>
      static decltype(_S_test_2<_Tp, _Up>(0))
      _S_test(...);
  };
  template<>
    struct common_type<>
    { };
  template<typename _Tp0>
    struct common_type<_Tp0>
    : public common_type<_Tp0, _Tp0>
    { };
  template<typename _Tp1, typename _Tp2,
    typename _Dp1 = __decay_t<_Tp1>, typename _Dp2 = __decay_t<_Tp2>>
    struct __common_type_impl
    {
      using type = common_type<_Dp1, _Dp2>;
    };
  template<typename _Tp1, typename _Tp2>
    struct __common_type_impl<_Tp1, _Tp2, _Tp1, _Tp2>
    : private __do_common_type_impl
    {
      using type = decltype(_S_test<_Tp1, _Tp2>(0));
    };
  template<typename _Tp1, typename _Tp2>
    struct common_type<_Tp1, _Tp2>
    : public __common_type_impl<_Tp1, _Tp2>::type
    { };
  template<typename...>
    struct __common_type_pack
    { };
  template<typename, typename, typename = void>
    struct __common_type_fold;
  template<typename _Tp1, typename _Tp2, typename... _Rp>
    struct common_type<_Tp1, _Tp2, _Rp...>
    : public __common_type_fold<common_type<_Tp1, _Tp2>,
    __common_type_pack<_Rp...>>
    { };
  template<typename _CTp, typename... _Rp>
    struct __common_type_fold<_CTp, __common_type_pack<_Rp...>,
         __void_t<typename _CTp::type>>
    : public common_type<typename _CTp::type, _Rp...>
    { };
  template<typename _CTp, typename _Rp>
    struct __common_type_fold<_CTp, _Rp, void>
    { };
  template<typename _Tp, bool = __is_enum(_Tp)>
    struct __underlying_type_impl
    {
      using type = __underlying_type(_Tp);
    };
  template<typename _Tp>
    struct __underlying_type_impl<_Tp, false>
    { };
  template<typename _Tp>
    struct underlying_type
    : public __underlying_type_impl<_Tp>
    { };
  template<typename _Tp>
    struct __declval_protector
    {
      static const bool __stop = false;
    };
  template<typename _Tp>
    auto declval() noexcept -> decltype(__declval<_Tp>(0))
    {
      static_assert(__declval_protector<_Tp>::__stop,
      "declval() must not be used!");
      return __declval<_Tp>(0);
    }
  template<typename _Signature>
    struct result_of;
  struct __invoke_memfun_ref { };
  struct __invoke_memfun_deref { };
  struct __invoke_memobj_ref { };
  struct __invoke_memobj_deref { };
  struct __invoke_other { };
  template<typename _Tp, typename _Tag>
    struct __result_of_success : __success_type<_Tp>
    { using __invoke_type = _Tag; };
  struct __result_of_memfun_ref_impl
  {
    template<typename _Fp, typename _Tp1, typename... _Args>
      static __result_of_success<decltype(
      (std::declval<_Tp1>().*std::declval<_Fp>())(std::declval<_Args>()...)
      ), __invoke_memfun_ref> _S_test(int);
    template<typename...>
      static __failure_type _S_test(...);
  };
  template<typename _MemPtr, typename _Arg, typename... _Args>
    struct __result_of_memfun_ref
    : private __result_of_memfun_ref_impl
    {
      using type = decltype(_S_test<_MemPtr, _Arg, _Args...>(0));
    };
  struct __result_of_memfun_deref_impl
  {
    template<typename _Fp, typename _Tp1, typename... _Args>
      static __result_of_success<decltype(
      ((*std::declval<_Tp1>()).*std::declval<_Fp>())(std::declval<_Args>()...)
      ), __invoke_memfun_deref> _S_test(int);
    template<typename...>
      static __failure_type _S_test(...);
  };
  template<typename _MemPtr, typename _Arg, typename... _Args>
    struct __result_of_memfun_deref
    : private __result_of_memfun_deref_impl
    {
      using type = decltype(_S_test<_MemPtr, _Arg, _Args...>(0));
    };
  struct __result_of_memobj_ref_impl
  {
    template<typename _Fp, typename _Tp1>
      static __result_of_success<decltype(
      std::declval<_Tp1>().*std::declval<_Fp>()
      ), __invoke_memobj_ref> _S_test(int);
    template<typename, typename>
      static __failure_type _S_test(...);
  };
  template<typename _MemPtr, typename _Arg>
    struct __result_of_memobj_ref
    : private __result_of_memobj_ref_impl
    {
      using type = decltype(_S_test<_MemPtr, _Arg>(0));
    };
  struct __result_of_memobj_deref_impl
  {
    template<typename _Fp, typename _Tp1>
      static __result_of_success<decltype(
      (*std::declval<_Tp1>()).*std::declval<_Fp>()
      ), __invoke_memobj_deref> _S_test(int);
    template<typename, typename>
      static __failure_type _S_test(...);
  };
  template<typename _MemPtr, typename _Arg>
    struct __result_of_memobj_deref
    : private __result_of_memobj_deref_impl
    {
      using type = decltype(_S_test<_MemPtr, _Arg>(0));
    };
  template<typename _MemPtr, typename _Arg>
    struct __result_of_memobj;
  template<typename _Res, typename _Class, typename _Arg>
    struct __result_of_memobj<_Res _Class::*, _Arg>
    {
      using _Argval = __remove_cvref_t<_Arg>;
      using _MemPtr = _Res _Class::*;
      using type = typename __conditional_t<__or_<is_same<_Argval, _Class>,
        is_base_of<_Class, _Argval>>::value,
        __result_of_memobj_ref<_MemPtr, _Arg>,
        __result_of_memobj_deref<_MemPtr, _Arg>
      >::type;
    };
  template<typename _MemPtr, typename _Arg, typename... _Args>
    struct __result_of_memfun;
  template<typename _Res, typename _Class, typename _Arg, typename... _Args>
    struct __result_of_memfun<_Res _Class::*, _Arg, _Args...>
    {
      using _Argval = typename remove_reference<_Arg>::type;
      using _MemPtr = _Res _Class::*;
      using type = typename __conditional_t<is_base_of<_Class, _Argval>::value,
        __result_of_memfun_ref<_MemPtr, _Arg, _Args...>,
        __result_of_memfun_deref<_MemPtr, _Arg, _Args...>
      >::type;
    };
  template<typename _Tp, typename _Up = __remove_cvref_t<_Tp>>
    struct __inv_unwrap
    {
      using type = _Tp;
    };
  template<typename _Tp, typename _Up>
    struct __inv_unwrap<_Tp, reference_wrapper<_Up>>
    {
      using type = _Up&;
    };
  template<bool, bool, typename _Functor, typename... _ArgTypes>
    struct __result_of_impl
    {
      using type = __failure_type;
    };
  template<typename _MemPtr, typename _Arg>
    struct __result_of_impl<true, false, _MemPtr, _Arg>
    : public __result_of_memobj<__decay_t<_MemPtr>,
    typename __inv_unwrap<_Arg>::type>
    { };
  template<typename _MemPtr, typename _Arg, typename... _Args>
    struct __result_of_impl<false, true, _MemPtr, _Arg, _Args...>
    : public __result_of_memfun<__decay_t<_MemPtr>,
    typename __inv_unwrap<_Arg>::type, _Args...>
    { };
  struct __result_of_other_impl
  {
    template<typename _Fn, typename... _Args>
      static __result_of_success<decltype(
      std::declval<_Fn>()(std::declval<_Args>()...)
      ), __invoke_other> _S_test(int);
    template<typename...>
      static __failure_type _S_test(...);
  };
  template<typename _Functor, typename... _ArgTypes>
    struct __result_of_impl<false, false, _Functor, _ArgTypes...>
    : private __result_of_other_impl
    {
      using type = decltype(_S_test<_Functor, _ArgTypes...>(0));
    };
  template<typename _Functor, typename... _ArgTypes>
    struct __invoke_result
    : public __result_of_impl<
        is_member_object_pointer<
          typename remove_reference<_Functor>::type
        >::value,
        is_member_function_pointer<
          typename remove_reference<_Functor>::type
        >::value,
 _Functor, _ArgTypes...
      >::type
    { };
  template<typename _Fn, typename... _Args>
    using __invoke_result_t = typename __invoke_result<_Fn, _Args...>::type;
  template<typename _Functor, typename... _ArgTypes>
    struct result_of<_Functor(_ArgTypes...)>
    : public __invoke_result<_Functor, _ArgTypes...>
    { } __attribute__ ((__deprecated__ ("use '" "std::invoke_result" "' instead")));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template<size_t _Len,
    size_t _Align = __aligned_storage_default_alignment(_Len)>
    using aligned_storage_t = typename aligned_storage<_Len, _Align>::type;
  template <size_t _Len, typename... _Types>
    using aligned_union_t = typename aligned_union<_Len, _Types...>::type;
#pragma GCC diagnostic pop
  template<typename _Tp>
    using decay_t = typename decay<_Tp>::type;
  template<bool _Cond, typename _Tp = void>
    using enable_if_t = typename enable_if<_Cond, _Tp>::type;
  template<bool _Cond, typename _Iftrue, typename _Iffalse>
    using conditional_t = typename conditional<_Cond, _Iftrue, _Iffalse>::type;
  template<typename... _Tp>
    using common_type_t = typename common_type<_Tp...>::type;
  template<typename _Tp>
    using underlying_type_t = typename underlying_type<_Tp>::type;
  template<typename _Tp>
    using result_of_t = typename result_of<_Tp>::type;
  template<typename...> using void_t = void;
  template<typename _Default, typename _AlwaysVoid,
    template<typename...> class _Op, typename... _Args>
    struct __detector
    {
      using type = _Default;
      using __is_detected = false_type;
    };
  template<typename _Default, template<typename...> class _Op,
     typename... _Args>
    struct __detector<_Default, __void_t<_Op<_Args...>>, _Op, _Args...>
    {
      using type = _Op<_Args...>;
      using __is_detected = true_type;
    };
  template<typename _Default, template<typename...> class _Op,
    typename... _Args>
    using __detected_or = __detector<_Default, void, _Op, _Args...>;
  template<typename _Default, template<typename...> class _Op,
    typename... _Args>
    using __detected_or_t
      = typename __detected_or<_Default, _Op, _Args...>::type;
  template <typename _Tp>
    struct __is_swappable;
  template <typename _Tp>
    struct __is_nothrow_swappable;
  template<typename>
    struct __is_tuple_like_impl : false_type
    { };
  template<typename _Tp>
    struct __is_tuple_like
    : public __is_tuple_like_impl<__remove_cvref_t<_Tp>>::type
    { };
  template<typename _Tp>
   
    inline
    _Require<__not_<__is_tuple_like<_Tp>>,
      is_move_constructible<_Tp>,
      is_move_assignable<_Tp>>
    swap(_Tp&, _Tp&)
    noexcept(__and_<is_nothrow_move_constructible<_Tp>,
             is_nothrow_move_assignable<_Tp>>::value);
  template<typename _Tp, size_t _Nm>
   
    inline
    __enable_if_t<__is_swappable<_Tp>::value>
    swap(_Tp (&__a)[_Nm], _Tp (&__b)[_Nm])
    noexcept(__is_nothrow_swappable<_Tp>::value);
  namespace __swappable_details {
    using std::swap;
    struct __do_is_swappable_impl
    {
      template<typename _Tp, typename
               = decltype(swap(std::declval<_Tp&>(), std::declval<_Tp&>()))>
        static true_type __test(int);
      template<typename>
        static false_type __test(...);
    };
    struct __do_is_nothrow_swappable_impl
    {
      template<typename _Tp>
        static __bool_constant<
          noexcept(swap(std::declval<_Tp&>(), std::declval<_Tp&>()))
        > __test(int);
      template<typename>
        static false_type __test(...);
    };
  }
  template<typename _Tp>
    struct __is_swappable_impl
    : public __swappable_details::__do_is_swappable_impl
    {
      using type = decltype(__test<_Tp>(0));
    };
  template<typename _Tp>
    struct __is_nothrow_swappable_impl
    : public __swappable_details::__do_is_nothrow_swappable_impl
    {
      using type = decltype(__test<_Tp>(0));
    };
  template<typename _Tp>
    struct __is_swappable
    : public __is_swappable_impl<_Tp>::type
    { };
  template<typename _Tp>
    struct __is_nothrow_swappable
    : public __is_nothrow_swappable_impl<_Tp>::type
    { };
  template<typename _Tp>
    struct is_swappable
    : public __is_swappable_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    struct is_nothrow_swappable
    : public __is_nothrow_swappable_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    inline constexpr bool is_swappable_v =
      is_swappable<_Tp>::value;
  template<typename _Tp>
    inline constexpr bool is_nothrow_swappable_v =
      is_nothrow_swappable<_Tp>::value;
  namespace __swappable_with_details {
    using std::swap;
    struct __do_is_swappable_with_impl
    {
      template<typename _Tp, typename _Up, typename
               = decltype(swap(std::declval<_Tp>(), std::declval<_Up>())),
               typename
               = decltype(swap(std::declval<_Up>(), std::declval<_Tp>()))>
        static true_type __test(int);
      template<typename, typename>
        static false_type __test(...);
    };
    struct __do_is_nothrow_swappable_with_impl
    {
      template<typename _Tp, typename _Up>
        static __bool_constant<
          noexcept(swap(std::declval<_Tp>(), std::declval<_Up>()))
          &&
          noexcept(swap(std::declval<_Up>(), std::declval<_Tp>()))
        > __test(int);
      template<typename, typename>
        static false_type __test(...);
    };
  }
  template<typename _Tp, typename _Up>
    struct __is_swappable_with_impl
    : public __swappable_with_details::__do_is_swappable_with_impl
    {
      using type = decltype(__test<_Tp, _Up>(0));
    };
  template<typename _Tp>
    struct __is_swappable_with_impl<_Tp&, _Tp&>
    : public __swappable_details::__do_is_swappable_impl
    {
      using type = decltype(__test<_Tp&>(0));
    };
  template<typename _Tp, typename _Up>
    struct __is_nothrow_swappable_with_impl
    : public __swappable_with_details::__do_is_nothrow_swappable_with_impl
    {
      using type = decltype(__test<_Tp, _Up>(0));
    };
  template<typename _Tp>
    struct __is_nothrow_swappable_with_impl<_Tp&, _Tp&>
    : public __swappable_details::__do_is_nothrow_swappable_impl
    {
      using type = decltype(__test<_Tp&>(0));
    };
  template<typename _Tp, typename _Up>
    struct is_swappable_with
    : public __is_swappable_with_impl<_Tp, _Up>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "first template argument must be a complete class or an unbounded array");
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Up>{}),
 "second template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp, typename _Up>
    struct is_nothrow_swappable_with
    : public __is_nothrow_swappable_with_impl<_Tp, _Up>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "first template argument must be a complete class or an unbounded array");
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Up>{}),
 "second template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp, typename _Up>
    inline constexpr bool is_swappable_with_v =
      is_swappable_with<_Tp, _Up>::value;
  template<typename _Tp, typename _Up>
    inline constexpr bool is_nothrow_swappable_with_v =
      is_nothrow_swappable_with<_Tp, _Up>::value;
  template<typename _Result, typename _Ret,
    bool = is_void<_Ret>::value, typename = void>
    struct __is_invocable_impl
    : false_type
    {
      using __nothrow_conv = false_type;
    };
  template<typename _Result, typename _Ret>
    struct __is_invocable_impl<_Result, _Ret,
                                true,
          __void_t<typename _Result::type>>
    : true_type
    {
      using __nothrow_conv = true_type;
    };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
  template<typename _Result, typename _Ret>
    struct __is_invocable_impl<_Result, _Ret,
                                false,
          __void_t<typename _Result::type>>
    {
    private:
      using _Res_t = typename _Result::type;
      static _Res_t _S_get() noexcept;
      template<typename _Tp>
 static void _S_conv(__type_identity_t<_Tp>) noexcept;
      template<typename _Tp,
        bool _Nothrow = noexcept(_S_conv<_Tp>(_S_get())),
        typename = decltype(_S_conv<_Tp>(_S_get())),
        bool _Dangle = __reference_converts_from_temporary(_Tp, _Res_t)
       >
 static __bool_constant<_Nothrow && !_Dangle>
 _S_test(int);
      template<typename _Tp, bool = false>
 static false_type
 _S_test(...);
    public:
      using type = decltype(_S_test<_Ret, true>(1));
      using __nothrow_conv = decltype(_S_test<_Ret>(1));
    };
#pragma GCC diagnostic pop
  template<typename _Fn, typename... _ArgTypes>
    struct __is_invocable
    : __bool_constant<__is_invocable(_Fn, _ArgTypes...)>
    { };
  template<typename _Fn, typename _Tp, typename... _Args>
    constexpr bool __call_is_nt(__invoke_memfun_ref)
    {
      using _Up = typename __inv_unwrap<_Tp>::type;
      return noexcept((std::declval<_Up>().*std::declval<_Fn>())(
     std::declval<_Args>()...));
    }
  template<typename _Fn, typename _Tp, typename... _Args>
    constexpr bool __call_is_nt(__invoke_memfun_deref)
    {
      return noexcept(((*std::declval<_Tp>()).*std::declval<_Fn>())(
     std::declval<_Args>()...));
    }
  template<typename _Fn, typename _Tp>
    constexpr bool __call_is_nt(__invoke_memobj_ref)
    {
      using _Up = typename __inv_unwrap<_Tp>::type;
      return noexcept(std::declval<_Up>().*std::declval<_Fn>());
    }
  template<typename _Fn, typename _Tp>
    constexpr bool __call_is_nt(__invoke_memobj_deref)
    {
      return noexcept((*std::declval<_Tp>()).*std::declval<_Fn>());
    }
  template<typename _Fn, typename... _Args>
    constexpr bool __call_is_nt(__invoke_other)
    {
      return noexcept(std::declval<_Fn>()(std::declval<_Args>()...));
    }
  template<typename _Result, typename _Fn, typename... _Args>
    struct __call_is_nothrow
    : __bool_constant<
 std::__call_is_nt<_Fn, _Args...>(typename _Result::__invoke_type{})
      >
    { };
  template<typename _Fn, typename... _Args>
    using __call_is_nothrow_
      = __call_is_nothrow<__invoke_result<_Fn, _Args...>, _Fn, _Args...>;
  template<typename _Fn, typename... _Args>
    struct __is_nothrow_invocable
    : __bool_constant<__is_nothrow_invocable(_Fn, _Args...)>
    { };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
  struct __nonesuchbase {};
  struct __nonesuch : private __nonesuchbase {
    ~__nonesuch() = delete;
    __nonesuch(__nonesuch const&) = delete;
    void operator=(__nonesuch const&) = delete;
  };
#pragma GCC diagnostic pop
  template<typename _Functor, typename... _ArgTypes>
    struct invoke_result
    : public __invoke_result<_Functor, _ArgTypes...>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Functor>{}),
 "_Functor must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
    };
  template<typename _Fn, typename... _Args>
    using invoke_result_t = typename invoke_result<_Fn, _Args...>::type;
  template<typename _Fn, typename... _ArgTypes>
    struct is_invocable
    : public __bool_constant<__is_invocable(_Fn, _ArgTypes...)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Fn>{}),
 "_Fn must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
    };
  template<typename _Ret, typename _Fn, typename... _ArgTypes>
    struct is_invocable_r
    : __is_invocable_impl<__invoke_result<_Fn, _ArgTypes...>, _Ret>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Fn>{}),
 "_Fn must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Ret>{}),
 "_Ret must be a complete class or an unbounded array");
    };
  template<typename _Fn, typename... _ArgTypes>
    struct is_nothrow_invocable
    : public __bool_constant<__is_nothrow_invocable(_Fn, _ArgTypes...)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Fn>{}),
 "_Fn must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
    };
  template<typename _Result, typename _Ret>
    using __is_nt_invocable_impl
      = typename __is_invocable_impl<_Result, _Ret>::__nothrow_conv;
  template<typename _Ret, typename _Fn, typename... _ArgTypes>
    struct is_nothrow_invocable_r
    : __and_<__is_nt_invocable_impl<__invoke_result<_Fn, _ArgTypes...>, _Ret>,
             __call_is_nothrow_<_Fn, _ArgTypes...>>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Fn>{}),
 "_Fn must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Ret>{}),
 "_Ret must be a complete class or an unbounded array");
    };
template <typename _Tp>
  inline constexpr bool is_void_v = is_void<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_null_pointer_v = is_null_pointer<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_integral_v = is_integral<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_floating_point_v = is_floating_point<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_array_v = __is_array(_Tp);
template <typename _Tp>
  inline constexpr bool is_pointer_v = __is_pointer(_Tp);
template <typename _Tp>
  inline constexpr bool is_lvalue_reference_v = false;
template <typename _Tp>
  inline constexpr bool is_lvalue_reference_v<_Tp&> = true;
template <typename _Tp>
  inline constexpr bool is_rvalue_reference_v = false;
template <typename _Tp>
  inline constexpr bool is_rvalue_reference_v<_Tp&&> = true;
template <typename _Tp>
  inline constexpr bool is_member_object_pointer_v =
    __is_member_object_pointer(_Tp);
template <typename _Tp>
  inline constexpr bool is_member_function_pointer_v =
    __is_member_function_pointer(_Tp);
template <typename _Tp>
  inline constexpr bool is_enum_v = __is_enum(_Tp);
template <typename _Tp>
  inline constexpr bool is_union_v = __is_union(_Tp);
template <typename _Tp>
  inline constexpr bool is_class_v = __is_class(_Tp);
template <typename _Tp>
  inline constexpr bool is_reference_v = __is_reference(_Tp);
template <typename _Tp>
  inline constexpr bool is_arithmetic_v = is_arithmetic<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_fundamental_v = is_fundamental<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_object_v = __is_object(_Tp);
template <typename _Tp>
  inline constexpr bool is_scalar_v = is_scalar<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_compound_v = !is_fundamental_v<_Tp>;
template <typename _Tp>
  inline constexpr bool is_member_pointer_v = __is_member_pointer(_Tp);
template <typename _Tp>
  inline constexpr bool is_const_v = __is_const(_Tp);
template <typename _Tp>
  inline constexpr bool is_function_v = __is_function(_Tp);
template <typename _Tp>
  inline constexpr bool is_volatile_v = __is_volatile(_Tp);
template <typename _Tp>
 
  inline constexpr bool is_trivial_v = __is_trivial(_Tp);
template <typename _Tp>
  inline constexpr bool is_trivially_copyable_v = __is_trivially_copyable(_Tp);
template <typename _Tp>
  inline constexpr bool is_standard_layout_v = __is_standard_layout(_Tp);
template <typename _Tp>
 
  inline constexpr bool is_pod_v = __is_pod(_Tp);
template <typename _Tp>
  [[__deprecated__]]
  inline constexpr bool is_literal_type_v = __is_literal_type(_Tp);
template <typename _Tp>
  inline constexpr bool is_empty_v = __is_empty(_Tp);
template <typename _Tp>
  inline constexpr bool is_polymorphic_v = __is_polymorphic(_Tp);
template <typename _Tp>
  inline constexpr bool is_abstract_v = __is_abstract(_Tp);
template <typename _Tp>
  inline constexpr bool is_final_v = __is_final(_Tp);
template <typename _Tp>
  inline constexpr bool is_signed_v = is_signed<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_unsigned_v = is_unsigned<_Tp>::value;
template <typename _Tp, typename... _Args>
  inline constexpr bool is_constructible_v = __is_constructible(_Tp, _Args...);
template <typename _Tp>
  inline constexpr bool is_default_constructible_v = __is_constructible(_Tp);
template <typename _Tp>
  inline constexpr bool is_copy_constructible_v
    = __is_constructible(_Tp, __add_lval_ref_t<const _Tp>);
template <typename _Tp>
  inline constexpr bool is_move_constructible_v
    = __is_constructible(_Tp, __add_rval_ref_t<_Tp>);
template <typename _Tp, typename _Up>
  inline constexpr bool is_assignable_v = __is_assignable(_Tp, _Up);
template <typename _Tp>
  inline constexpr bool is_copy_assignable_v
    = __is_assignable(__add_lval_ref_t<_Tp>, __add_lval_ref_t<const _Tp>);
template <typename _Tp>
  inline constexpr bool is_move_assignable_v
    = __is_assignable(__add_lval_ref_t<_Tp>, __add_rval_ref_t<_Tp>);
template <typename _Tp>
  inline constexpr bool is_destructible_v = __is_destructible(_Tp);
template <typename _Tp, typename... _Args>
  inline constexpr bool is_trivially_constructible_v
    = __is_trivially_constructible(_Tp, _Args...);
template <typename _Tp>
  inline constexpr bool is_trivially_default_constructible_v
    = __is_trivially_constructible(_Tp);
template <typename _Tp>
  inline constexpr bool is_trivially_copy_constructible_v
    = __is_trivially_constructible(_Tp, __add_lval_ref_t<const _Tp>);
template <typename _Tp>
  inline constexpr bool is_trivially_move_constructible_v
    = __is_trivially_constructible(_Tp, __add_rval_ref_t<_Tp>);
template <typename _Tp, typename _Up>
  inline constexpr bool is_trivially_assignable_v
    = __is_trivially_assignable(_Tp, _Up);
template <typename _Tp>
  inline constexpr bool is_trivially_copy_assignable_v
    = __is_trivially_assignable(__add_lval_ref_t<_Tp>,
    __add_lval_ref_t<const _Tp>);
template <typename _Tp>
  inline constexpr bool is_trivially_move_assignable_v
    = __is_trivially_assignable(__add_lval_ref_t<_Tp>,
    __add_rval_ref_t<_Tp>);
template <typename _Tp>
  inline constexpr bool is_trivially_destructible_v
    = __is_trivially_destructible(_Tp);
template <typename _Tp, typename... _Args>
  inline constexpr bool is_nothrow_constructible_v
    = __is_nothrow_constructible(_Tp, _Args...);
template <typename _Tp>
  inline constexpr bool is_nothrow_default_constructible_v
    = __is_nothrow_constructible(_Tp);
template <typename _Tp>
  inline constexpr bool is_nothrow_copy_constructible_v
    = __is_nothrow_constructible(_Tp, __add_lval_ref_t<const _Tp>);
template <typename _Tp>
  inline constexpr bool is_nothrow_move_constructible_v
    = __is_nothrow_constructible(_Tp, __add_rval_ref_t<_Tp>);
template <typename _Tp, typename _Up>
  inline constexpr bool is_nothrow_assignable_v
    = __is_nothrow_assignable(_Tp, _Up);
template <typename _Tp>
  inline constexpr bool is_nothrow_copy_assignable_v
    = __is_nothrow_assignable(__add_lval_ref_t<_Tp>,
         __add_lval_ref_t<const _Tp>);
template <typename _Tp>
  inline constexpr bool is_nothrow_move_assignable_v
    = __is_nothrow_assignable(__add_lval_ref_t<_Tp>, __add_rval_ref_t<_Tp>);
template <typename _Tp>
  inline constexpr bool is_nothrow_destructible_v
    = __is_nothrow_destructible(_Tp);
template <typename _Tp>
  inline constexpr bool has_virtual_destructor_v
    = __has_virtual_destructor(_Tp);
template <typename _Tp>
  inline constexpr size_t alignment_of_v = alignment_of<_Tp>::value;
template <typename _Tp>
  inline constexpr size_t rank_v = __array_rank(_Tp);
template <typename _Tp, unsigned _Idx = 0>
  inline constexpr size_t extent_v = 0;
template <typename _Tp, size_t _Size>
  inline constexpr size_t extent_v<_Tp[_Size], 0> = _Size;
template <typename _Tp, unsigned _Idx, size_t _Size>
  inline constexpr size_t extent_v<_Tp[_Size], _Idx> = extent_v<_Tp, _Idx - 1>;
template <typename _Tp>
  inline constexpr size_t extent_v<_Tp[], 0> = 0;
template <typename _Tp, unsigned _Idx>
  inline constexpr size_t extent_v<_Tp[], _Idx> = extent_v<_Tp, _Idx - 1>;
template <typename _Tp, typename _Up>
  inline constexpr bool is_same_v = __is_same(_Tp, _Up);
template <typename _Base, typename _Derived>
  inline constexpr bool is_base_of_v = __is_base_of(_Base, _Derived);
template <typename _From, typename _To>
  inline constexpr bool is_convertible_v = __is_convertible(_From, _To);
template<typename _Fn, typename... _Args>
  inline constexpr bool is_invocable_v
    = __is_invocable(_Fn, _Args...);
template<typename _Fn, typename... _Args>
  inline constexpr bool is_nothrow_invocable_v
    = __is_nothrow_invocable(_Fn, _Args...);
template<typename _Ret, typename _Fn, typename... _Args>
  inline constexpr bool is_invocable_r_v
    = is_invocable_r<_Ret, _Fn, _Args...>::value;
template<typename _Ret, typename _Fn, typename... _Args>
  inline constexpr bool is_nothrow_invocable_r_v
    = is_nothrow_invocable_r<_Ret, _Fn, _Args...>::value;
  template<typename _Tp>
    struct has_unique_object_representations
    : bool_constant<__has_unique_object_representations(
      remove_cv_t<remove_all_extents_t<_Tp>>
      )>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };
  template<typename _Tp>
    inline constexpr bool has_unique_object_representations_v
      = has_unique_object_representations<_Tp>::value;
  template<typename _Tp>
    struct is_aggregate
    : bool_constant<__is_aggregate(remove_cv_t<_Tp>)>
    { };
  template<typename _Tp>
    inline constexpr bool is_aggregate_v = __is_aggregate(remove_cv_t<_Tp>);
  template<size_t... _Indexes> struct _Index_tuple { };
  template<size_t _Num>
    struct _Build_index_tuple
    {
      using __type = _Index_tuple<__integer_pack(_Num)...>;
    };

}
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
extern "C++" {
namespace std __attribute__ ((__visibility__ ("default")))
{

  struct __true_type { };
  struct __false_type { };
  template<bool>
    struct __truth_type
    { typedef __false_type __type; };
  template<>
    struct __truth_type<true>
    { typedef __true_type __type; };
  template<class _Sp, class _Tp>
    struct __traitor
    {
      enum { __value = bool(_Sp::__value) || bool(_Tp::__value) };
      typedef typename __truth_type<__value>::__type __type;
    };
  template<typename, typename>
    struct __are_same
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };
  template<typename _Tp>
    struct __are_same<_Tp, _Tp>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<typename _Tp>
    struct __is_integer
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };
  template<>
    struct __is_integer<bool>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<signed char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<unsigned char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<wchar_t>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<char16_t>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<char32_t>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<short>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<unsigned short>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<int>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<unsigned int>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<long>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<unsigned long>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<long long>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_integer<unsigned long long>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
__extension__ template<> struct __is_integer<__int128> { enum { __value = 1 }; typedef __true_type __type; }; __extension__ template<> struct __is_integer<unsigned __int128> { enum { __value = 1 }; typedef __true_type __type; };
  template<typename _Tp>
    struct __is_floating
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };
  template<>
    struct __is_floating<float>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_floating<double>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_floating<long double>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_floating<__float128>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<typename _Tp>
    struct __is_arithmetic
    : public __traitor<__is_integer<_Tp>, __is_floating<_Tp> >
    { };
  template<typename _Tp>
    struct __is_char
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };
  template<>
    struct __is_char<char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_char<wchar_t>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<typename _Tp>
    struct __is_byte
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };
  template<>
    struct __is_byte<char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_byte<signed char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<>
    struct __is_byte<unsigned char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  enum class byte : unsigned char;
  template<>
    struct __is_byte<byte>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
  template<typename _Tp>
    struct __is_nonvolatile_trivially_copyable
    {
      enum { __value = __is_trivially_copyable(_Tp) };
    };
  template<typename _Tp>
    struct __is_nonvolatile_trivially_copyable<volatile _Tp>
    {
      enum { __value = 0 };
    };
  template<typename _OutputIter, typename _InputIter>
    struct __memcpyable
    {
      enum { __value = 0 };
    };
  template<typename _Tp>
    struct __memcpyable<_Tp*, _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };
  template<typename _Tp>
    struct __memcpyable<_Tp*, const _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };
  template<typename _Tp> struct __memcpyable_integer;
  template<typename _Tp, typename _Up>
    struct __memcpyable<_Tp*, _Up*>
    {
      enum {
 __value = __memcpyable_integer<_Tp>::__width != 0
      && ((int)__memcpyable_integer<_Tp>::__width
     == (int)__memcpyable_integer<_Up>::__width)
      };
    };
  template<typename _Tp, typename _Up>
    struct __memcpyable<_Tp*, const _Up*>
    : __memcpyable<_Tp*, _Up*>
    { };
  template<typename _Tp>
    struct __memcpyable_integer
    {
      enum {
 __width = __is_integer<_Tp>::__value ? (sizeof(_Tp) * 8) : 0
      };
    };
  template<typename _Tp>
    struct __memcpyable_integer<volatile _Tp>
    { enum { __width = 0 }; };
  template<>
    struct __memcpyable_integer<bool>
    { enum { __width = 0 }; };
  template<typename _Iter1, typename _Iter2>
    struct __memcmpable
    {
      enum { __value = 0 };
    };
  template<typename _Tp>
    struct __memcmpable<_Tp*, _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };
  template<typename _Tp>
    struct __memcmpable<const _Tp*, _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };
  template<typename _Tp>
    struct __memcmpable<_Tp*, const _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };
  template<typename _Tp, bool _TreatAsBytes =
 __is_byte<_Tp>::__value
    >
    struct __is_memcmp_ordered
    {
      static const bool __value = _Tp(-1) > _Tp(1);
    };
  template<typename _Tp>
    struct __is_memcmp_ordered<_Tp, false>
    {
      static const bool __value = false;
    };
  template<typename _Tp, typename _Up, bool = sizeof(_Tp) == sizeof(_Up)>
    struct __is_memcmp_ordered_with
    {
      static const bool __value = __is_memcmp_ordered<_Tp>::__value
 && __is_memcmp_ordered<_Up>::__value;
    };
  template<typename _Tp, typename _Up>
    struct __is_memcmp_ordered_with<_Tp, _Up, false>
    {
      static const bool __value = false;
    };
  template<>
    struct __is_memcmp_ordered_with<std::byte, std::byte, true>
    { static constexpr bool __value = true; };
  template<typename _Tp, bool _SameSize>
    struct __is_memcmp_ordered_with<_Tp, std::byte, _SameSize>
    { static constexpr bool __value = false; };
  template<typename _Up, bool _SameSize>
    struct __is_memcmp_ordered_with<std::byte, _Up, _SameSize>
    { static constexpr bool __value = false; };
  template<typename _ValT, typename _Tp>
    constexpr bool __can_use_memchr_for_find
      = __is_byte<_ValT>::__value
   && (is_same_v<_Tp, _ValT> || is_integral_v<_Tp>);
  template<typename _Tp>
    struct __is_move_iterator
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };
  template<typename _Iterator>
   
    inline _Iterator
    __miter_base(_Iterator __it)
    { return __it; }

}
}
#pragma GCC diagnostic pop
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
extern "C++" {
namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{

  template<bool, typename>
    struct __enable_if
    { };
  template<typename _Tp>
    struct __enable_if<true, _Tp>
    { typedef _Tp __type; };
  template<bool _Cond, typename _Iftrue, typename _Iffalse>
    struct __conditional_type
    { typedef _Iftrue __type; };
  template<typename _Iftrue, typename _Iffalse>
    struct __conditional_type<false, _Iftrue, _Iffalse>
    { typedef _Iffalse __type; };
  template<typename _Tp>
    struct __add_unsigned
    {
    private:
      typedef __enable_if<std::__is_integer<_Tp>::__value, _Tp> __if_type;
    public:
      typedef typename __if_type::__type __type;
    };
  template<>
    struct __add_unsigned<char>
    { typedef unsigned char __type; };
  template<>
    struct __add_unsigned<signed char>
    { typedef unsigned char __type; };
  template<>
    struct __add_unsigned<short>
    { typedef unsigned short __type; };
  template<>
    struct __add_unsigned<int>
    { typedef unsigned int __type; };
  template<>
    struct __add_unsigned<long>
    { typedef unsigned long __type; };
  template<>
    struct __add_unsigned<long long>
    { typedef unsigned long long __type; };
  template<>
    struct __add_unsigned<bool>;
  template<>
    struct __add_unsigned<wchar_t>;
  template<typename _Tp>
    struct __remove_unsigned
    {
    private:
      typedef __enable_if<std::__is_integer<_Tp>::__value, _Tp> __if_type;
    public:
      typedef typename __if_type::__type __type;
    };
  template<>
    struct __remove_unsigned<char>
    { typedef signed char __type; };
  template<>
    struct __remove_unsigned<unsigned char>
    { typedef signed char __type; };
  template<>
    struct __remove_unsigned<unsigned short>
    { typedef short __type; };
  template<>
    struct __remove_unsigned<unsigned int>
    { typedef int __type; };
  template<>
    struct __remove_unsigned<unsigned long>
    { typedef long __type; };
  template<>
    struct __remove_unsigned<unsigned long long>
    { typedef long long __type; };
  template<>
    struct __remove_unsigned<bool>;
  template<>
    struct __remove_unsigned<wchar_t>;
  template<typename _Type>
    constexpr
    inline bool
    __is_null_pointer(_Type* __ptr)
    { return __ptr == 0; }
  template<typename _Type>
    constexpr
    inline bool
    __is_null_pointer(_Type)
    { return false; }
  constexpr bool
  __is_null_pointer(std::nullptr_t)
  { return true; }
  template<typename _Tp, bool = std::__is_integer<_Tp>::__value>
    struct __promote
    { typedef double __type; };
  template<typename _Tp>
    struct __promote<_Tp, false>
    { };
  template<>
    struct __promote<long double>
    { typedef long double __type; };
  template<>
    struct __promote<double>
    { typedef double __type; };
  template<>
    struct __promote<float>
    { typedef float __type; };
  template<typename... _Tp>
    using __promoted_t = decltype((typename __promote<_Tp>::__type(0) + ...));
  template<typename _Tp, typename _Up>
    using __promote_2 = __promote<__promoted_t<_Tp, _Up>>;
  template<typename _Tp, typename _Up, typename _Vp>
    using __promote_3 = __promote<__promoted_t<_Tp, _Up, _Vp>>;
  template<typename _Tp, typename _Up, typename _Vp, typename _Wp>
    using __promote_4 = __promote<__promoted_t<_Tp, _Up, _Vp, _Wp>>;

}
}
#pragma GCC diagnostic pop
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
extern "C" {
typedef float float_t;
typedef double double_t;
typedef long double long_double_t;
typedef _Float32 _Float32_t;
typedef _Float64 _Float64_t;
typedef _Float128 _Float128_t;
enum
  {
    FP_INT_UPWARD =
      0,
    FP_INT_DOWNWARD =
      1,
    FP_INT_TOWARDZERO =
      2,
    FP_INT_TONEARESTFROMZERO =
      3,
    FP_INT_TONEAREST =
      4,
  };
extern int __fpclassify (double __value) noexcept (true)
     __attribute__ ((__const__));
extern int __signbit (double __value) noexcept (true)
     __attribute__ ((__const__));
extern int __isinf (double __value) noexcept (true)
  __attribute__ ((__const__));
extern int __finite (double __value) noexcept (true)
  __attribute__ ((__const__));
extern int __isnan (double __value) noexcept (true)
  __attribute__ ((__const__));
extern int __iseqsig (double __x, double __y) noexcept (true);
extern int __issignaling (double __value) noexcept (true)
     __attribute__ ((__const__));
 extern double acos (double __x) noexcept (true); extern double __acos (double __x) noexcept (true);
 extern double asin (double __x) noexcept (true); extern double __asin (double __x) noexcept (true);
 extern double atan (double __x) noexcept (true); extern double __atan (double __x) noexcept (true);
 extern double atan2 (double __y, double __x) noexcept (true); extern double __atan2 (double __y, double __x) noexcept (true);
 extern double cos (double __x) noexcept (true); extern double __cos (double __x) noexcept (true);
 extern double sin (double __x) noexcept (true); extern double __sin (double __x) noexcept (true);
 extern double tan (double __x) noexcept (true); extern double __tan (double __x) noexcept (true);
extern double acospi (double __x) noexcept (true); extern double __acospi (double __x) noexcept (true);
 extern double acospi (double __x) noexcept (true); extern double __acospi (double __x) noexcept (true);
extern double asinpi (double __x) noexcept (true); extern double __asinpi (double __x) noexcept (true);
 extern double asinpi (double __x) noexcept (true); extern double __asinpi (double __x) noexcept (true);
extern double atanpi (double __x) noexcept (true); extern double __atanpi (double __x) noexcept (true);
 extern double atanpi (double __x) noexcept (true); extern double __atanpi (double __x) noexcept (true);
extern double atan2pi (double __y, double __x) noexcept (true); extern double __atan2pi (double __y, double __x) noexcept (true);
 extern double atan2pi (double __y, double __x) noexcept (true); extern double __atan2pi (double __y, double __x) noexcept (true);
 extern double cospi (double __x) noexcept (true); extern double __cospi (double __x) noexcept (true);
 extern double sinpi (double __x) noexcept (true); extern double __sinpi (double __x) noexcept (true);
 extern double tanpi (double __x) noexcept (true); extern double __tanpi (double __x) noexcept (true);
 extern double cosh (double __x) noexcept (true); extern double __cosh (double __x) noexcept (true);
 extern double sinh (double __x) noexcept (true); extern double __sinh (double __x) noexcept (true);
 extern double tanh (double __x) noexcept (true); extern double __tanh (double __x) noexcept (true);
 extern void sincos (double __x, double *__sinx, double *__cosx) noexcept (true); extern void __sincos (double __x, double *__sinx, double *__cosx) noexcept (true);
 extern double acosh (double __x) noexcept (true); extern double __acosh (double __x) noexcept (true);
 extern double asinh (double __x) noexcept (true); extern double __asinh (double __x) noexcept (true);
 extern double atanh (double __x) noexcept (true); extern double __atanh (double __x) noexcept (true);
 extern double exp (double __x) noexcept (true); extern double __exp (double __x) noexcept (true);
extern double frexp (double __x, int *__exponent) noexcept (true); extern double __frexp (double __x, int *__exponent) noexcept (true);
extern double ldexp (double __x, int __exponent) noexcept (true); extern double __ldexp (double __x, int __exponent) noexcept (true);
 extern double log (double __x) noexcept (true); extern double __log (double __x) noexcept (true);
 extern double log10 (double __x) noexcept (true); extern double __log10 (double __x) noexcept (true);
extern double modf (double __x, double *__iptr) noexcept (true); extern double __modf (double __x, double *__iptr) noexcept (true) __attribute__ ((__nonnull__ (2)));
 extern double exp10 (double __x) noexcept (true); extern double __exp10 (double __x) noexcept (true);
 extern double exp2m1 (double __x) noexcept (true); extern double __exp2m1 (double __x) noexcept (true);
 extern double exp10m1 (double __x) noexcept (true); extern double __exp10m1 (double __x) noexcept (true);
 extern double log2p1 (double __x) noexcept (true); extern double __log2p1 (double __x) noexcept (true);
 extern double log10p1 (double __x) noexcept (true); extern double __log10p1 (double __x) noexcept (true);
 extern double logp1 (double __x) noexcept (true); extern double __logp1 (double __x) noexcept (true);
 extern double expm1 (double __x) noexcept (true); extern double __expm1 (double __x) noexcept (true);
 extern double log1p (double __x) noexcept (true); extern double __log1p (double __x) noexcept (true);
extern double logb (double __x) noexcept (true); extern double __logb (double __x) noexcept (true);
 extern double exp2 (double __x) noexcept (true); extern double __exp2 (double __x) noexcept (true);
 extern double log2 (double __x) noexcept (true); extern double __log2 (double __x) noexcept (true);
 extern double pow (double __x, double __y) noexcept (true); extern double __pow (double __x, double __y) noexcept (true);
extern double sqrt (double __x) noexcept (true); extern double __sqrt (double __x) noexcept (true);
 extern double hypot (double __x, double __y) noexcept (true); extern double __hypot (double __x, double __y) noexcept (true);
 extern double cbrt (double __x) noexcept (true); extern double __cbrt (double __x) noexcept (true);
extern double compoundn (double __x, long long int __y) noexcept (true); extern double __compoundn (double __x, long long int __y) noexcept (true);
extern double pown (double __x, long long int __y) noexcept (true); extern double __pown (double __x, long long int __y) noexcept (true);
 extern double powr (double __x, double __y) noexcept (true); extern double __powr (double __x, double __y) noexcept (true);
extern double powr (double __x, double __y) noexcept (true); extern double __powr (double __x, double __y) noexcept (true);
extern double rootn (double __x, long long int __y) noexcept (true); extern double __rootn (double __x, long long int __y) noexcept (true);
 extern double rsqrt (double __x) noexcept (true); extern double __rsqrt (double __x) noexcept (true);
extern double ceil (double __x) noexcept (true) __attribute__ ((__const__));
extern double fabs (double __x) noexcept (true) __attribute__ ((__const__));
extern double floor (double __x) noexcept (true) __attribute__ ((__const__));
extern double fmod (double __x, double __y) noexcept (true); extern double __fmod (double __x, double __y) noexcept (true);
extern int finite (double __value) noexcept (true)
  __attribute__ ((__const__));
extern double drem (double __x, double __y) noexcept (true); extern double __drem (double __x, double __y) noexcept (true);
extern double significand (double __x) noexcept (true); extern double __significand (double __x) noexcept (true);
extern double copysign (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double nan (const char *__tagb) noexcept (true); extern double __nan (const char *__tagb) noexcept (true);
extern double j0 (double) noexcept (true); extern double __j0 (double) noexcept (true);
extern double j1 (double) noexcept (true); extern double __j1 (double) noexcept (true);
extern double jn (int, double) noexcept (true); extern double __jn (int, double) noexcept (true);
extern double y0 (double) noexcept (true); extern double __y0 (double) noexcept (true);
extern double y1 (double) noexcept (true); extern double __y1 (double) noexcept (true);
extern double yn (int, double) noexcept (true); extern double __yn (int, double) noexcept (true);
 extern double erf (double) noexcept (true); extern double __erf (double) noexcept (true);
 extern double erfc (double) noexcept (true); extern double __erfc (double) noexcept (true);
extern double lgamma (double) noexcept (true); extern double __lgamma (double) noexcept (true);
extern double tgamma (double) noexcept (true); extern double __tgamma (double) noexcept (true);
extern double gamma (double) noexcept (true); extern double __gamma (double) noexcept (true);
extern double lgamma_r (double, int *__signgamp) noexcept (true); extern double __lgamma_r (double, int *__signgamp) noexcept (true);
extern double rint (double __x) noexcept (true); extern double __rint (double __x) noexcept (true);
extern double nextafter (double __x, double __y) noexcept (true); extern double __nextafter (double __x, double __y) noexcept (true);
extern double nexttoward (double __x, long double __y) noexcept (true); extern double __nexttoward (double __x, long double __y) noexcept (true);
extern double nextdown (double __x) noexcept (true); extern double __nextdown (double __x) noexcept (true);
extern double nextup (double __x) noexcept (true); extern double __nextup (double __x) noexcept (true);
extern double remainder (double __x, double __y) noexcept (true); extern double __remainder (double __x, double __y) noexcept (true);
extern double scalbn (double __x, int __n) noexcept (true); extern double __scalbn (double __x, int __n) noexcept (true);
extern int ilogb (double __x) noexcept (true); extern int __ilogb (double __x) noexcept (true);
extern long int llogb (double __x) noexcept (true); extern long int __llogb (double __x) noexcept (true);
extern double scalbln (double __x, long int __n) noexcept (true); extern double __scalbln (double __x, long int __n) noexcept (true);
extern double nearbyint (double __x) noexcept (true); extern double __nearbyint (double __x) noexcept (true);
extern double round (double __x) noexcept (true) __attribute__ ((__const__));
extern double trunc (double __x) noexcept (true) __attribute__ ((__const__));
extern double remquo (double __x, double __y, int *__quo) noexcept (true); extern double __remquo (double __x, double __y, int *__quo) noexcept (true);
extern long int lrint (double __x) noexcept (true); extern long int __lrint (double __x) noexcept (true);
__extension__
extern long long int llrint (double __x) noexcept (true); extern long long int __llrint (double __x) noexcept (true);
extern long int lround (double __x) noexcept (true); extern long int __lround (double __x) noexcept (true);
__extension__
extern long long int llround (double __x) noexcept (true); extern long long int __llround (double __x) noexcept (true);
extern double fdim (double __x, double __y) noexcept (true); extern double __fdim (double __x, double __y) noexcept (true);
extern double fmax (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fmin (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fma (double __x, double __y, double __z) noexcept (true); extern double __fma (double __x, double __y, double __z) noexcept (true);
extern double roundeven (double __x) noexcept (true) __attribute__ ((__const__));
extern double fromfp (double __x, int __round, unsigned int __width) noexcept (true); extern double __fromfp (double __x, int __round, unsigned int __width) noexcept (true);
extern double ufromfp (double __x, int __round, unsigned int __width) noexcept (true); extern double __ufromfp (double __x, int __round, unsigned int __width) noexcept (true);
extern double fromfpx (double __x, int __round, unsigned int __width) noexcept (true); extern double __fromfpx (double __x, int __round, unsigned int __width) noexcept (true);
extern double ufromfpx (double __x, int __round, unsigned int __width) noexcept (true); extern double __ufromfpx (double __x, int __round, unsigned int __width) noexcept (true);
extern int canonicalize (double *__cx, const double *__x) noexcept (true);
extern double fmaxmag (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fminmag (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fmaximum (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fminimum (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fmaximum_num (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fminimum_num (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fmaximum_mag (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fminimum_mag (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fmaximum_mag_num (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern double fminimum_mag_num (double __x, double __y) noexcept (true) __attribute__ ((__const__));
extern int totalorder (const double *__x, const double *__y) noexcept (true)
     __attribute__ ((__pure__));
extern int totalordermag (const double *__x, const double *__y) noexcept (true)
     __attribute__ ((__pure__));
extern double getpayload (const double *__x) noexcept (true); extern double __getpayload (const double *__x) noexcept (true);
extern int setpayload (double *__x, double __payload) noexcept (true);
extern int setpayloadsig (double *__x, double __payload) noexcept (true);
extern double scalb (double __x, double __n) noexcept (true); extern double __scalb (double __x, double __n) noexcept (true);
extern int __fpclassifyf (float __value) noexcept (true)
     __attribute__ ((__const__));
extern int __signbitf (float __value) noexcept (true)
     __attribute__ ((__const__));
extern int __isinff (float __value) noexcept (true)
  __attribute__ ((__const__));
extern int __finitef (float __value) noexcept (true)
  __attribute__ ((__const__));
extern int __isnanf (float __value) noexcept (true)
  __attribute__ ((__const__));
extern int __iseqsigf (float __x, float __y) noexcept (true);
extern int __issignalingf (float __value) noexcept (true)
     __attribute__ ((__const__));
 extern float acosf (float __x) noexcept (true); extern float __acosf (float __x) noexcept (true);
 extern float asinf (float __x) noexcept (true); extern float __asinf (float __x) noexcept (true);
 extern float atanf (float __x) noexcept (true); extern float __atanf (float __x) noexcept (true);
 extern float atan2f (float __y, float __x) noexcept (true); extern float __atan2f (float __y, float __x) noexcept (true);
 extern float cosf (float __x) noexcept (true); extern float __cosf (float __x) noexcept (true);
 extern float sinf (float __x) noexcept (true); extern float __sinf (float __x) noexcept (true);
 extern float tanf (float __x) noexcept (true); extern float __tanf (float __x) noexcept (true);
extern float acospif (float __x) noexcept (true); extern float __acospif (float __x) noexcept (true);
 extern float acospif (float __x) noexcept (true); extern float __acospif (float __x) noexcept (true);
extern float asinpif (float __x) noexcept (true); extern float __asinpif (float __x) noexcept (true);
 extern float asinpif (float __x) noexcept (true); extern float __asinpif (float __x) noexcept (true);
extern float atanpif (float __x) noexcept (true); extern float __atanpif (float __x) noexcept (true);
 extern float atanpif (float __x) noexcept (true); extern float __atanpif (float __x) noexcept (true);
extern float atan2pif (float __y, float __x) noexcept (true); extern float __atan2pif (float __y, float __x) noexcept (true);
 extern float atan2pif (float __y, float __x) noexcept (true); extern float __atan2pif (float __y, float __x) noexcept (true);
 extern float cospif (float __x) noexcept (true); extern float __cospif (float __x) noexcept (true);
 extern float sinpif (float __x) noexcept (true); extern float __sinpif (float __x) noexcept (true);
 extern float tanpif (float __x) noexcept (true); extern float __tanpif (float __x) noexcept (true);
 extern float coshf (float __x) noexcept (true); extern float __coshf (float __x) noexcept (true);
 extern float sinhf (float __x) noexcept (true); extern float __sinhf (float __x) noexcept (true);
 extern float tanhf (float __x) noexcept (true); extern float __tanhf (float __x) noexcept (true);
 extern void sincosf (float __x, float *__sinx, float *__cosx) noexcept (true); extern void __sincosf (float __x, float *__sinx, float *__cosx) noexcept (true);
 extern float acoshf (float __x) noexcept (true); extern float __acoshf (float __x) noexcept (true);
 extern float asinhf (float __x) noexcept (true); extern float __asinhf (float __x) noexcept (true);
 extern float atanhf (float __x) noexcept (true); extern float __atanhf (float __x) noexcept (true);
 extern float expf (float __x) noexcept (true); extern float __expf (float __x) noexcept (true);
extern float frexpf (float __x, int *__exponent) noexcept (true); extern float __frexpf (float __x, int *__exponent) noexcept (true);
extern float ldexpf (float __x, int __exponent) noexcept (true); extern float __ldexpf (float __x, int __exponent) noexcept (true);
 extern float logf (float __x) noexcept (true); extern float __logf (float __x) noexcept (true);
 extern float log10f (float __x) noexcept (true); extern float __log10f (float __x) noexcept (true);
extern float modff (float __x, float *__iptr) noexcept (true); extern float __modff (float __x, float *__iptr) noexcept (true) __attribute__ ((__nonnull__ (2)));
 extern float exp10f (float __x) noexcept (true); extern float __exp10f (float __x) noexcept (true);
 extern float exp2m1f (float __x) noexcept (true); extern float __exp2m1f (float __x) noexcept (true);
 extern float exp10m1f (float __x) noexcept (true); extern float __exp10m1f (float __x) noexcept (true);
 extern float log2p1f (float __x) noexcept (true); extern float __log2p1f (float __x) noexcept (true);
 extern float log10p1f (float __x) noexcept (true); extern float __log10p1f (float __x) noexcept (true);
 extern float logp1f (float __x) noexcept (true); extern float __logp1f (float __x) noexcept (true);
 extern float expm1f (float __x) noexcept (true); extern float __expm1f (float __x) noexcept (true);
 extern float log1pf (float __x) noexcept (true); extern float __log1pf (float __x) noexcept (true);
extern float logbf (float __x) noexcept (true); extern float __logbf (float __x) noexcept (true);
 extern float exp2f (float __x) noexcept (true); extern float __exp2f (float __x) noexcept (true);
 extern float log2f (float __x) noexcept (true); extern float __log2f (float __x) noexcept (true);
 extern float powf (float __x, float __y) noexcept (true); extern float __powf (float __x, float __y) noexcept (true);
extern float sqrtf (float __x) noexcept (true); extern float __sqrtf (float __x) noexcept (true);
 extern float hypotf (float __x, float __y) noexcept (true); extern float __hypotf (float __x, float __y) noexcept (true);
 extern float cbrtf (float __x) noexcept (true); extern float __cbrtf (float __x) noexcept (true);
extern float compoundnf (float __x, long long int __y) noexcept (true); extern float __compoundnf (float __x, long long int __y) noexcept (true);
extern float pownf (float __x, long long int __y) noexcept (true); extern float __pownf (float __x, long long int __y) noexcept (true);
 extern float powrf (float __x, float __y) noexcept (true); extern float __powrf (float __x, float __y) noexcept (true);
extern float powrf (float __x, float __y) noexcept (true); extern float __powrf (float __x, float __y) noexcept (true);
extern float rootnf (float __x, long long int __y) noexcept (true); extern float __rootnf (float __x, long long int __y) noexcept (true);
 extern float rsqrtf (float __x) noexcept (true); extern float __rsqrtf (float __x) noexcept (true);
extern float ceilf (float __x) noexcept (true) __attribute__ ((__const__));
extern float fabsf (float __x) noexcept (true) __attribute__ ((__const__));
extern float floorf (float __x) noexcept (true) __attribute__ ((__const__));
extern float fmodf (float __x, float __y) noexcept (true); extern float __fmodf (float __x, float __y) noexcept (true);
extern int isinff (float __value) noexcept (true)
  __attribute__ ((__const__));
extern int finitef (float __value) noexcept (true)
  __attribute__ ((__const__));
extern float dremf (float __x, float __y) noexcept (true); extern float __dremf (float __x, float __y) noexcept (true);
extern float significandf (float __x) noexcept (true); extern float __significandf (float __x) noexcept (true);
extern float copysignf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float nanf (const char *__tagb) noexcept (true); extern float __nanf (const char *__tagb) noexcept (true);
extern int isnanf (float __value) noexcept (true)
  __attribute__ ((__const__));
extern float j0f (float) noexcept (true); extern float __j0f (float) noexcept (true);
extern float j1f (float) noexcept (true); extern float __j1f (float) noexcept (true);
extern float jnf (int, float) noexcept (true); extern float __jnf (int, float) noexcept (true);
extern float y0f (float) noexcept (true); extern float __y0f (float) noexcept (true);
extern float y1f (float) noexcept (true); extern float __y1f (float) noexcept (true);
extern float ynf (int, float) noexcept (true); extern float __ynf (int, float) noexcept (true);
 extern float erff (float) noexcept (true); extern float __erff (float) noexcept (true);
 extern float erfcf (float) noexcept (true); extern float __erfcf (float) noexcept (true);
extern float lgammaf (float) noexcept (true); extern float __lgammaf (float) noexcept (true);
extern float tgammaf (float) noexcept (true); extern float __tgammaf (float) noexcept (true);
extern float gammaf (float) noexcept (true); extern float __gammaf (float) noexcept (true);
extern float lgammaf_r (float, int *__signgamp) noexcept (true); extern float __lgammaf_r (float, int *__signgamp) noexcept (true);
extern float rintf (float __x) noexcept (true); extern float __rintf (float __x) noexcept (true);
extern float nextafterf (float __x, float __y) noexcept (true); extern float __nextafterf (float __x, float __y) noexcept (true);
extern float nexttowardf (float __x, long double __y) noexcept (true); extern float __nexttowardf (float __x, long double __y) noexcept (true);
extern float nextdownf (float __x) noexcept (true); extern float __nextdownf (float __x) noexcept (true);
extern float nextupf (float __x) noexcept (true); extern float __nextupf (float __x) noexcept (true);
extern float remainderf (float __x, float __y) noexcept (true); extern float __remainderf (float __x, float __y) noexcept (true);
extern float scalbnf (float __x, int __n) noexcept (true); extern float __scalbnf (float __x, int __n) noexcept (true);
extern int ilogbf (float __x) noexcept (true); extern int __ilogbf (float __x) noexcept (true);
extern long int llogbf (float __x) noexcept (true); extern long int __llogbf (float __x) noexcept (true);
extern float scalblnf (float __x, long int __n) noexcept (true); extern float __scalblnf (float __x, long int __n) noexcept (true);
extern float nearbyintf (float __x) noexcept (true); extern float __nearbyintf (float __x) noexcept (true);
extern float roundf (float __x) noexcept (true) __attribute__ ((__const__));
extern float truncf (float __x) noexcept (true) __attribute__ ((__const__));
extern float remquof (float __x, float __y, int *__quo) noexcept (true); extern float __remquof (float __x, float __y, int *__quo) noexcept (true);
extern long int lrintf (float __x) noexcept (true); extern long int __lrintf (float __x) noexcept (true);
__extension__
extern long long int llrintf (float __x) noexcept (true); extern long long int __llrintf (float __x) noexcept (true);
extern long int lroundf (float __x) noexcept (true); extern long int __lroundf (float __x) noexcept (true);
__extension__
extern long long int llroundf (float __x) noexcept (true); extern long long int __llroundf (float __x) noexcept (true);
extern float fdimf (float __x, float __y) noexcept (true); extern float __fdimf (float __x, float __y) noexcept (true);
extern float fmaxf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fminf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fmaf (float __x, float __y, float __z) noexcept (true); extern float __fmaf (float __x, float __y, float __z) noexcept (true);
extern float roundevenf (float __x) noexcept (true) __attribute__ ((__const__));
extern float fromfpf (float __x, int __round, unsigned int __width) noexcept (true); extern float __fromfpf (float __x, int __round, unsigned int __width) noexcept (true);
extern float ufromfpf (float __x, int __round, unsigned int __width) noexcept (true); extern float __ufromfpf (float __x, int __round, unsigned int __width) noexcept (true);
extern float fromfpxf (float __x, int __round, unsigned int __width) noexcept (true); extern float __fromfpxf (float __x, int __round, unsigned int __width) noexcept (true);
extern float ufromfpxf (float __x, int __round, unsigned int __width) noexcept (true); extern float __ufromfpxf (float __x, int __round, unsigned int __width) noexcept (true);
extern int canonicalizef (float *__cx, const float *__x) noexcept (true);
extern float fmaxmagf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fminmagf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fmaximumf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fminimumf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fmaximum_numf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fminimum_numf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fmaximum_magf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fminimum_magf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fmaximum_mag_numf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern float fminimum_mag_numf (float __x, float __y) noexcept (true) __attribute__ ((__const__));
extern int totalorderf (const float *__x, const float *__y) noexcept (true)
     __attribute__ ((__pure__));
extern int totalordermagf (const float *__x, const float *__y) noexcept (true)
     __attribute__ ((__pure__));
extern float getpayloadf (const float *__x) noexcept (true); extern float __getpayloadf (const float *__x) noexcept (true);
extern int setpayloadf (float *__x, float __payload) noexcept (true);
extern int setpayloadsigf (float *__x, float __payload) noexcept (true);
extern float scalbf (float __x, float __n) noexcept (true); extern float __scalbf (float __x, float __n) noexcept (true);
extern int __fpclassifyl (long double __value) noexcept (true)
     __attribute__ ((__const__));
extern int __signbitl (long double __value) noexcept (true)
     __attribute__ ((__const__));
extern int __isinfl (long double __value) noexcept (true)
  __attribute__ ((__const__));
extern int __finitel (long double __value) noexcept (true)
  __attribute__ ((__const__));
extern int __isnanl (long double __value) noexcept (true)
  __attribute__ ((__const__));
extern int __iseqsigl (long double __x, long double __y) noexcept (true);
extern int __issignalingl (long double __value) noexcept (true)
     __attribute__ ((__const__));
 extern long double acosl (long double __x) noexcept (true); extern long double __acosl (long double __x) noexcept (true);
 extern long double asinl (long double __x) noexcept (true); extern long double __asinl (long double __x) noexcept (true);
 extern long double atanl (long double __x) noexcept (true); extern long double __atanl (long double __x) noexcept (true);
 extern long double atan2l (long double __y, long double __x) noexcept (true); extern long double __atan2l (long double __y, long double __x) noexcept (true);
 extern long double cosl (long double __x) noexcept (true); extern long double __cosl (long double __x) noexcept (true);
 extern long double sinl (long double __x) noexcept (true); extern long double __sinl (long double __x) noexcept (true);
 extern long double tanl (long double __x) noexcept (true); extern long double __tanl (long double __x) noexcept (true);
extern long double acospil (long double __x) noexcept (true); extern long double __acospil (long double __x) noexcept (true);
 extern long double acospil (long double __x) noexcept (true); extern long double __acospil (long double __x) noexcept (true);
extern long double asinpil (long double __x) noexcept (true); extern long double __asinpil (long double __x) noexcept (true);
 extern long double asinpil (long double __x) noexcept (true); extern long double __asinpil (long double __x) noexcept (true);
extern long double atanpil (long double __x) noexcept (true); extern long double __atanpil (long double __x) noexcept (true);
 extern long double atanpil (long double __x) noexcept (true); extern long double __atanpil (long double __x) noexcept (true);
extern long double atan2pil (long double __y, long double __x) noexcept (true); extern long double __atan2pil (long double __y, long double __x) noexcept (true);
 extern long double atan2pil (long double __y, long double __x) noexcept (true); extern long double __atan2pil (long double __y, long double __x) noexcept (true);
 extern long double cospil (long double __x) noexcept (true); extern long double __cospil (long double __x) noexcept (true);
 extern long double sinpil (long double __x) noexcept (true); extern long double __sinpil (long double __x) noexcept (true);
 extern long double tanpil (long double __x) noexcept (true); extern long double __tanpil (long double __x) noexcept (true);
 extern long double coshl (long double __x) noexcept (true); extern long double __coshl (long double __x) noexcept (true);
 extern long double sinhl (long double __x) noexcept (true); extern long double __sinhl (long double __x) noexcept (true);
 extern long double tanhl (long double __x) noexcept (true); extern long double __tanhl (long double __x) noexcept (true);
 extern void sincosl (long double __x, long double *__sinx, long double *__cosx) noexcept (true); extern void __sincosl (long double __x, long double *__sinx, long double *__cosx) noexcept (true);
 extern long double acoshl (long double __x) noexcept (true); extern long double __acoshl (long double __x) noexcept (true);
 extern long double asinhl (long double __x) noexcept (true); extern long double __asinhl (long double __x) noexcept (true);
 extern long double atanhl (long double __x) noexcept (true); extern long double __atanhl (long double __x) noexcept (true);
 extern long double expl (long double __x) noexcept (true); extern long double __expl (long double __x) noexcept (true);
extern long double frexpl (long double __x, int *__exponent) noexcept (true); extern long double __frexpl (long double __x, int *__exponent) noexcept (true);
extern long double ldexpl (long double __x, int __exponent) noexcept (true); extern long double __ldexpl (long double __x, int __exponent) noexcept (true);
 extern long double logl (long double __x) noexcept (true); extern long double __logl (long double __x) noexcept (true);
 extern long double log10l (long double __x) noexcept (true); extern long double __log10l (long double __x) noexcept (true);
extern long double modfl (long double __x, long double *__iptr) noexcept (true); extern long double __modfl (long double __x, long double *__iptr) noexcept (true) __attribute__ ((__nonnull__ (2)));
 extern long double exp10l (long double __x) noexcept (true); extern long double __exp10l (long double __x) noexcept (true);
 extern long double exp2m1l (long double __x) noexcept (true); extern long double __exp2m1l (long double __x) noexcept (true);
 extern long double exp10m1l (long double __x) noexcept (true); extern long double __exp10m1l (long double __x) noexcept (true);
 extern long double log2p1l (long double __x) noexcept (true); extern long double __log2p1l (long double __x) noexcept (true);
 extern long double log10p1l (long double __x) noexcept (true); extern long double __log10p1l (long double __x) noexcept (true);
 extern long double logp1l (long double __x) noexcept (true); extern long double __logp1l (long double __x) noexcept (true);
 extern long double expm1l (long double __x) noexcept (true); extern long double __expm1l (long double __x) noexcept (true);
 extern long double log1pl (long double __x) noexcept (true); extern long double __log1pl (long double __x) noexcept (true);
extern long double logbl (long double __x) noexcept (true); extern long double __logbl (long double __x) noexcept (true);
 extern long double exp2l (long double __x) noexcept (true); extern long double __exp2l (long double __x) noexcept (true);
 extern long double log2l (long double __x) noexcept (true); extern long double __log2l (long double __x) noexcept (true);
 extern long double powl (long double __x, long double __y) noexcept (true); extern long double __powl (long double __x, long double __y) noexcept (true);
extern long double sqrtl (long double __x) noexcept (true); extern long double __sqrtl (long double __x) noexcept (true);
 extern long double hypotl (long double __x, long double __y) noexcept (true); extern long double __hypotl (long double __x, long double __y) noexcept (true);
 extern long double cbrtl (long double __x) noexcept (true); extern long double __cbrtl (long double __x) noexcept (true);
extern long double compoundnl (long double __x, long long int __y) noexcept (true); extern long double __compoundnl (long double __x, long long int __y) noexcept (true);
extern long double pownl (long double __x, long long int __y) noexcept (true); extern long double __pownl (long double __x, long long int __y) noexcept (true);
 extern long double powrl (long double __x, long double __y) noexcept (true); extern long double __powrl (long double __x, long double __y) noexcept (true);
extern long double powrl (long double __x, long double __y) noexcept (true); extern long double __powrl (long double __x, long double __y) noexcept (true);
extern long double rootnl (long double __x, long long int __y) noexcept (true); extern long double __rootnl (long double __x, long long int __y) noexcept (true);
 extern long double rsqrtl (long double __x) noexcept (true); extern long double __rsqrtl (long double __x) noexcept (true);
extern long double ceill (long double __x) noexcept (true) __attribute__ ((__const__));
extern long double fabsl (long double __x) noexcept (true) __attribute__ ((__const__));
extern long double floorl (long double __x) noexcept (true) __attribute__ ((__const__));
extern long double fmodl (long double __x, long double __y) noexcept (true); extern long double __fmodl (long double __x, long double __y) noexcept (true);
extern int isinfl (long double __value) noexcept (true)
  __attribute__ ((__const__));
extern int finitel (long double __value) noexcept (true)
  __attribute__ ((__const__));
extern long double dreml (long double __x, long double __y) noexcept (true); extern long double __dreml (long double __x, long double __y) noexcept (true);
extern long double significandl (long double __x) noexcept (true); extern long double __significandl (long double __x) noexcept (true);
extern long double copysignl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double nanl (const char *__tagb) noexcept (true); extern long double __nanl (const char *__tagb) noexcept (true);
extern int isnanl (long double __value) noexcept (true)
  __attribute__ ((__const__));
extern long double j0l (long double) noexcept (true); extern long double __j0l (long double) noexcept (true);
extern long double j1l (long double) noexcept (true); extern long double __j1l (long double) noexcept (true);
extern long double jnl (int, long double) noexcept (true); extern long double __jnl (int, long double) noexcept (true);
extern long double y0l (long double) noexcept (true); extern long double __y0l (long double) noexcept (true);
extern long double y1l (long double) noexcept (true); extern long double __y1l (long double) noexcept (true);
extern long double ynl (int, long double) noexcept (true); extern long double __ynl (int, long double) noexcept (true);
 extern long double erfl (long double) noexcept (true); extern long double __erfl (long double) noexcept (true);
 extern long double erfcl (long double) noexcept (true); extern long double __erfcl (long double) noexcept (true);
extern long double lgammal (long double) noexcept (true); extern long double __lgammal (long double) noexcept (true);
extern long double tgammal (long double) noexcept (true); extern long double __tgammal (long double) noexcept (true);
extern long double gammal (long double) noexcept (true); extern long double __gammal (long double) noexcept (true);
extern long double lgammal_r (long double, int *__signgamp) noexcept (true); extern long double __lgammal_r (long double, int *__signgamp) noexcept (true);
extern long double rintl (long double __x) noexcept (true); extern long double __rintl (long double __x) noexcept (true);
extern long double nextafterl (long double __x, long double __y) noexcept (true); extern long double __nextafterl (long double __x, long double __y) noexcept (true);
extern long double nexttowardl (long double __x, long double __y) noexcept (true); extern long double __nexttowardl (long double __x, long double __y) noexcept (true);
extern long double nextdownl (long double __x) noexcept (true); extern long double __nextdownl (long double __x) noexcept (true);
extern long double nextupl (long double __x) noexcept (true); extern long double __nextupl (long double __x) noexcept (true);
extern long double remainderl (long double __x, long double __y) noexcept (true); extern long double __remainderl (long double __x, long double __y) noexcept (true);
extern long double scalbnl (long double __x, int __n) noexcept (true); extern long double __scalbnl (long double __x, int __n) noexcept (true);
extern int ilogbl (long double __x) noexcept (true); extern int __ilogbl (long double __x) noexcept (true);
extern long int llogbl (long double __x) noexcept (true); extern long int __llogbl (long double __x) noexcept (true);
extern long double scalblnl (long double __x, long int __n) noexcept (true); extern long double __scalblnl (long double __x, long int __n) noexcept (true);
extern long double nearbyintl (long double __x) noexcept (true); extern long double __nearbyintl (long double __x) noexcept (true);
extern long double roundl (long double __x) noexcept (true) __attribute__ ((__const__));
extern long double truncl (long double __x) noexcept (true) __attribute__ ((__const__));
extern long double remquol (long double __x, long double __y, int *__quo) noexcept (true); extern long double __remquol (long double __x, long double __y, int *__quo) noexcept (true);
extern long int lrintl (long double __x) noexcept (true); extern long int __lrintl (long double __x) noexcept (true);
__extension__
extern long long int llrintl (long double __x) noexcept (true); extern long long int __llrintl (long double __x) noexcept (true);
extern long int lroundl (long double __x) noexcept (true); extern long int __lroundl (long double __x) noexcept (true);
__extension__
extern long long int llroundl (long double __x) noexcept (true); extern long long int __llroundl (long double __x) noexcept (true);
extern long double fdiml (long double __x, long double __y) noexcept (true); extern long double __fdiml (long double __x, long double __y) noexcept (true);
extern long double fmaxl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fminl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fmal (long double __x, long double __y, long double __z) noexcept (true); extern long double __fmal (long double __x, long double __y, long double __z) noexcept (true);
extern long double roundevenl (long double __x) noexcept (true) __attribute__ ((__const__));
extern long double fromfpl (long double __x, int __round, unsigned int __width) noexcept (true); extern long double __fromfpl (long double __x, int __round, unsigned int __width) noexcept (true);
extern long double ufromfpl (long double __x, int __round, unsigned int __width) noexcept (true); extern long double __ufromfpl (long double __x, int __round, unsigned int __width) noexcept (true);
extern long double fromfpxl (long double __x, int __round, unsigned int __width) noexcept (true); extern long double __fromfpxl (long double __x, int __round, unsigned int __width) noexcept (true);
extern long double ufromfpxl (long double __x, int __round, unsigned int __width) noexcept (true); extern long double __ufromfpxl (long double __x, int __round, unsigned int __width) noexcept (true);
extern int canonicalizel (long double *__cx, const long double *__x) noexcept (true);
extern long double fmaxmagl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fminmagl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fmaximuml (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fminimuml (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fmaximum_numl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fminimum_numl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fmaximum_magl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fminimum_magl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fmaximum_mag_numl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern long double fminimum_mag_numl (long double __x, long double __y) noexcept (true) __attribute__ ((__const__));
extern int totalorderl (const long double *__x, const long double *__y) noexcept (true)
     __attribute__ ((__pure__));
extern int totalordermagl (const long double *__x, const long double *__y) noexcept (true)
     __attribute__ ((__pure__));
extern long double getpayloadl (const long double *__x) noexcept (true); extern long double __getpayloadl (const long double *__x) noexcept (true);
extern int setpayloadl (long double *__x, long double __payload) noexcept (true);
extern int setpayloadsigl (long double *__x, long double __payload) noexcept (true);
extern long double scalbl (long double __x, long double __n) noexcept (true); extern long double __scalbl (long double __x, long double __n) noexcept (true);
 extern _Float32 acosf32 (_Float32 __x) noexcept (true); extern _Float32 __acosf32 (_Float32 __x) noexcept (true);
 extern _Float32 asinf32 (_Float32 __x) noexcept (true); extern _Float32 __asinf32 (_Float32 __x) noexcept (true);
 extern _Float32 atanf32 (_Float32 __x) noexcept (true); extern _Float32 __atanf32 (_Float32 __x) noexcept (true);
 extern _Float32 atan2f32 (_Float32 __y, _Float32 __x) noexcept (true); extern _Float32 __atan2f32 (_Float32 __y, _Float32 __x) noexcept (true);
 extern _Float32 cosf32 (_Float32 __x) noexcept (true); extern _Float32 __cosf32 (_Float32 __x) noexcept (true);
 extern _Float32 sinf32 (_Float32 __x) noexcept (true); extern _Float32 __sinf32 (_Float32 __x) noexcept (true);
 extern _Float32 tanf32 (_Float32 __x) noexcept (true); extern _Float32 __tanf32 (_Float32 __x) noexcept (true);
extern _Float32 acospif32 (_Float32 __x) noexcept (true); extern _Float32 __acospif32 (_Float32 __x) noexcept (true);
 extern _Float32 acospif32 (_Float32 __x) noexcept (true); extern _Float32 __acospif32 (_Float32 __x) noexcept (true);
extern _Float32 asinpif32 (_Float32 __x) noexcept (true); extern _Float32 __asinpif32 (_Float32 __x) noexcept (true);
 extern _Float32 asinpif32 (_Float32 __x) noexcept (true); extern _Float32 __asinpif32 (_Float32 __x) noexcept (true);
extern _Float32 atanpif32 (_Float32 __x) noexcept (true); extern _Float32 __atanpif32 (_Float32 __x) noexcept (true);
 extern _Float32 atanpif32 (_Float32 __x) noexcept (true); extern _Float32 __atanpif32 (_Float32 __x) noexcept (true);
extern _Float32 atan2pif32 (_Float32 __y, _Float32 __x) noexcept (true); extern _Float32 __atan2pif32 (_Float32 __y, _Float32 __x) noexcept (true);
 extern _Float32 atan2pif32 (_Float32 __y, _Float32 __x) noexcept (true); extern _Float32 __atan2pif32 (_Float32 __y, _Float32 __x) noexcept (true);
 extern _Float32 cospif32 (_Float32 __x) noexcept (true); extern _Float32 __cospif32 (_Float32 __x) noexcept (true);
 extern _Float32 sinpif32 (_Float32 __x) noexcept (true); extern _Float32 __sinpif32 (_Float32 __x) noexcept (true);
 extern _Float32 tanpif32 (_Float32 __x) noexcept (true); extern _Float32 __tanpif32 (_Float32 __x) noexcept (true);
 extern _Float32 coshf32 (_Float32 __x) noexcept (true); extern _Float32 __coshf32 (_Float32 __x) noexcept (true);
 extern _Float32 sinhf32 (_Float32 __x) noexcept (true); extern _Float32 __sinhf32 (_Float32 __x) noexcept (true);
 extern _Float32 tanhf32 (_Float32 __x) noexcept (true); extern _Float32 __tanhf32 (_Float32 __x) noexcept (true);
 extern void sincosf32 (_Float32 __x, _Float32 *__sinx, _Float32 *__cosx) noexcept (true); extern void __sincosf32 (_Float32 __x, _Float32 *__sinx, _Float32 *__cosx) noexcept (true);
 extern _Float32 acoshf32 (_Float32 __x) noexcept (true); extern _Float32 __acoshf32 (_Float32 __x) noexcept (true);
 extern _Float32 asinhf32 (_Float32 __x) noexcept (true); extern _Float32 __asinhf32 (_Float32 __x) noexcept (true);
 extern _Float32 atanhf32 (_Float32 __x) noexcept (true); extern _Float32 __atanhf32 (_Float32 __x) noexcept (true);
 extern _Float32 expf32 (_Float32 __x) noexcept (true); extern _Float32 __expf32 (_Float32 __x) noexcept (true);
extern _Float32 frexpf32 (_Float32 __x, int *__exponent) noexcept (true); extern _Float32 __frexpf32 (_Float32 __x, int *__exponent) noexcept (true);
extern _Float32 ldexpf32 (_Float32 __x, int __exponent) noexcept (true); extern _Float32 __ldexpf32 (_Float32 __x, int __exponent) noexcept (true);
 extern _Float32 logf32 (_Float32 __x) noexcept (true); extern _Float32 __logf32 (_Float32 __x) noexcept (true);
 extern _Float32 log10f32 (_Float32 __x) noexcept (true); extern _Float32 __log10f32 (_Float32 __x) noexcept (true);
extern _Float32 modff32 (_Float32 __x, _Float32 *__iptr) noexcept (true); extern _Float32 __modff32 (_Float32 __x, _Float32 *__iptr) noexcept (true) __attribute__ ((__nonnull__ (2)));
 extern _Float32 exp10f32 (_Float32 __x) noexcept (true); extern _Float32 __exp10f32 (_Float32 __x) noexcept (true);
 extern _Float32 exp2m1f32 (_Float32 __x) noexcept (true); extern _Float32 __exp2m1f32 (_Float32 __x) noexcept (true);
 extern _Float32 exp10m1f32 (_Float32 __x) noexcept (true); extern _Float32 __exp10m1f32 (_Float32 __x) noexcept (true);
 extern _Float32 log2p1f32 (_Float32 __x) noexcept (true); extern _Float32 __log2p1f32 (_Float32 __x) noexcept (true);
 extern _Float32 log10p1f32 (_Float32 __x) noexcept (true); extern _Float32 __log10p1f32 (_Float32 __x) noexcept (true);
 extern _Float32 logp1f32 (_Float32 __x) noexcept (true); extern _Float32 __logp1f32 (_Float32 __x) noexcept (true);
 extern _Float32 expm1f32 (_Float32 __x) noexcept (true); extern _Float32 __expm1f32 (_Float32 __x) noexcept (true);
 extern _Float32 log1pf32 (_Float32 __x) noexcept (true); extern _Float32 __log1pf32 (_Float32 __x) noexcept (true);
extern _Float32 logbf32 (_Float32 __x) noexcept (true); extern _Float32 __logbf32 (_Float32 __x) noexcept (true);
 extern _Float32 exp2f32 (_Float32 __x) noexcept (true); extern _Float32 __exp2f32 (_Float32 __x) noexcept (true);
 extern _Float32 log2f32 (_Float32 __x) noexcept (true); extern _Float32 __log2f32 (_Float32 __x) noexcept (true);
 extern _Float32 powf32 (_Float32 __x, _Float32 __y) noexcept (true); extern _Float32 __powf32 (_Float32 __x, _Float32 __y) noexcept (true);
extern _Float32 sqrtf32 (_Float32 __x) noexcept (true); extern _Float32 __sqrtf32 (_Float32 __x) noexcept (true);
 extern _Float32 hypotf32 (_Float32 __x, _Float32 __y) noexcept (true); extern _Float32 __hypotf32 (_Float32 __x, _Float32 __y) noexcept (true);
 extern _Float32 cbrtf32 (_Float32 __x) noexcept (true); extern _Float32 __cbrtf32 (_Float32 __x) noexcept (true);
extern _Float32 compoundnf32 (_Float32 __x, long long int __y) noexcept (true); extern _Float32 __compoundnf32 (_Float32 __x, long long int __y) noexcept (true);
extern _Float32 pownf32 (_Float32 __x, long long int __y) noexcept (true); extern _Float32 __pownf32 (_Float32 __x, long long int __y) noexcept (true);
 extern _Float32 powrf32 (_Float32 __x, _Float32 __y) noexcept (true); extern _Float32 __powrf32 (_Float32 __x, _Float32 __y) noexcept (true);
extern _Float32 powrf32 (_Float32 __x, _Float32 __y) noexcept (true); extern _Float32 __powrf32 (_Float32 __x, _Float32 __y) noexcept (true);
extern _Float32 rootnf32 (_Float32 __x, long long int __y) noexcept (true); extern _Float32 __rootnf32 (_Float32 __x, long long int __y) noexcept (true);
 extern _Float32 rsqrtf32 (_Float32 __x) noexcept (true); extern _Float32 __rsqrtf32 (_Float32 __x) noexcept (true);
extern _Float32 ceilf32 (_Float32 __x) noexcept (true) __attribute__ ((__const__));
extern _Float32 fabsf32 (_Float32 __x) noexcept (true) __attribute__ ((__const__));
extern _Float32 floorf32 (_Float32 __x) noexcept (true) __attribute__ ((__const__));
extern _Float32 fmodf32 (_Float32 __x, _Float32 __y) noexcept (true); extern _Float32 __fmodf32 (_Float32 __x, _Float32 __y) noexcept (true);
extern _Float32 copysignf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 nanf32 (const char *__tagb) noexcept (true); extern _Float32 __nanf32 (const char *__tagb) noexcept (true);
extern _Float32 j0f32 (_Float32) noexcept (true); extern _Float32 __j0f32 (_Float32) noexcept (true);
extern _Float32 j1f32 (_Float32) noexcept (true); extern _Float32 __j1f32 (_Float32) noexcept (true);
extern _Float32 jnf32 (int, _Float32) noexcept (true); extern _Float32 __jnf32 (int, _Float32) noexcept (true);
extern _Float32 y0f32 (_Float32) noexcept (true); extern _Float32 __y0f32 (_Float32) noexcept (true);
extern _Float32 y1f32 (_Float32) noexcept (true); extern _Float32 __y1f32 (_Float32) noexcept (true);
extern _Float32 ynf32 (int, _Float32) noexcept (true); extern _Float32 __ynf32 (int, _Float32) noexcept (true);
 extern _Float32 erff32 (_Float32) noexcept (true); extern _Float32 __erff32 (_Float32) noexcept (true);
 extern _Float32 erfcf32 (_Float32) noexcept (true); extern _Float32 __erfcf32 (_Float32) noexcept (true);
extern _Float32 lgammaf32 (_Float32) noexcept (true); extern _Float32 __lgammaf32 (_Float32) noexcept (true);
extern _Float32 tgammaf32 (_Float32) noexcept (true); extern _Float32 __tgammaf32 (_Float32) noexcept (true);
extern _Float32 lgammaf32_r (_Float32, int *__signgamp) noexcept (true); extern _Float32 __lgammaf32_r (_Float32, int *__signgamp) noexcept (true);
extern _Float32 rintf32 (_Float32 __x) noexcept (true); extern _Float32 __rintf32 (_Float32 __x) noexcept (true);
extern _Float32 nextafterf32 (_Float32 __x, _Float32 __y) noexcept (true); extern _Float32 __nextafterf32 (_Float32 __x, _Float32 __y) noexcept (true);
extern _Float32 nextdownf32 (_Float32 __x) noexcept (true); extern _Float32 __nextdownf32 (_Float32 __x) noexcept (true);
extern _Float32 nextupf32 (_Float32 __x) noexcept (true); extern _Float32 __nextupf32 (_Float32 __x) noexcept (true);
extern _Float32 remainderf32 (_Float32 __x, _Float32 __y) noexcept (true); extern _Float32 __remainderf32 (_Float32 __x, _Float32 __y) noexcept (true);
extern _Float32 scalbnf32 (_Float32 __x, int __n) noexcept (true); extern _Float32 __scalbnf32 (_Float32 __x, int __n) noexcept (true);
extern int ilogbf32 (_Float32 __x) noexcept (true); extern int __ilogbf32 (_Float32 __x) noexcept (true);
extern long int llogbf32 (_Float32 __x) noexcept (true); extern long int __llogbf32 (_Float32 __x) noexcept (true);
extern _Float32 scalblnf32 (_Float32 __x, long int __n) noexcept (true); extern _Float32 __scalblnf32 (_Float32 __x, long int __n) noexcept (true);
extern _Float32 nearbyintf32 (_Float32 __x) noexcept (true); extern _Float32 __nearbyintf32 (_Float32 __x) noexcept (true);
extern _Float32 roundf32 (_Float32 __x) noexcept (true) __attribute__ ((__const__));
extern _Float32 truncf32 (_Float32 __x) noexcept (true) __attribute__ ((__const__));
extern _Float32 remquof32 (_Float32 __x, _Float32 __y, int *__quo) noexcept (true); extern _Float32 __remquof32 (_Float32 __x, _Float32 __y, int *__quo) noexcept (true);
extern long int lrintf32 (_Float32 __x) noexcept (true); extern long int __lrintf32 (_Float32 __x) noexcept (true);
__extension__
extern long long int llrintf32 (_Float32 __x) noexcept (true); extern long long int __llrintf32 (_Float32 __x) noexcept (true);
extern long int lroundf32 (_Float32 __x) noexcept (true); extern long int __lroundf32 (_Float32 __x) noexcept (true);
__extension__
extern long long int llroundf32 (_Float32 __x) noexcept (true); extern long long int __llroundf32 (_Float32 __x) noexcept (true);
extern _Float32 fdimf32 (_Float32 __x, _Float32 __y) noexcept (true); extern _Float32 __fdimf32 (_Float32 __x, _Float32 __y) noexcept (true);
extern _Float32 fmaxf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fminf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fmaf32 (_Float32 __x, _Float32 __y, _Float32 __z) noexcept (true); extern _Float32 __fmaf32 (_Float32 __x, _Float32 __y, _Float32 __z) noexcept (true);
extern _Float32 roundevenf32 (_Float32 __x) noexcept (true) __attribute__ ((__const__));
extern _Float32 fromfpf32 (_Float32 __x, int __round, unsigned int __width) noexcept (true); extern _Float32 __fromfpf32 (_Float32 __x, int __round, unsigned int __width) noexcept (true);
extern _Float32 ufromfpf32 (_Float32 __x, int __round, unsigned int __width) noexcept (true); extern _Float32 __ufromfpf32 (_Float32 __x, int __round, unsigned int __width) noexcept (true);
extern _Float32 fromfpxf32 (_Float32 __x, int __round, unsigned int __width) noexcept (true); extern _Float32 __fromfpxf32 (_Float32 __x, int __round, unsigned int __width) noexcept (true);
extern _Float32 ufromfpxf32 (_Float32 __x, int __round, unsigned int __width) noexcept (true); extern _Float32 __ufromfpxf32 (_Float32 __x, int __round, unsigned int __width) noexcept (true);
extern int canonicalizef32 (_Float32 *__cx, const _Float32 *__x) noexcept (true);
extern _Float32 fmaxmagf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fminmagf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fmaximumf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fminimumf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fmaximum_numf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fminimum_numf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fmaximum_magf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fminimum_magf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fmaximum_mag_numf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern _Float32 fminimum_mag_numf32 (_Float32 __x, _Float32 __y) noexcept (true) __attribute__ ((__const__));
extern int totalorderf32 (const _Float32 *__x, const _Float32 *__y) noexcept (true)
     __attribute__ ((__pure__));
extern int totalordermagf32 (const _Float32 *__x, const _Float32 *__y) noexcept (true)
     __attribute__ ((__pure__));
extern _Float32 getpayloadf32 (const _Float32 *__x) noexcept (true); extern _Float32 __getpayloadf32 (const _Float32 *__x) noexcept (true);
extern int setpayloadf32 (_Float32 *__x, _Float32 __payload) noexcept (true);
extern int setpayloadsigf32 (_Float32 *__x, _Float32 __payload) noexcept (true);
 extern _Float64 acosf64 (_Float64 __x) noexcept (true); extern _Float64 __acosf64 (_Float64 __x) noexcept (true);
 extern _Float64 asinf64 (_Float64 __x) noexcept (true); extern _Float64 __asinf64 (_Float64 __x) noexcept (true);
 extern _Float64 atanf64 (_Float64 __x) noexcept (true); extern _Float64 __atanf64 (_Float64 __x) noexcept (true);
 extern _Float64 atan2f64 (_Float64 __y, _Float64 __x) noexcept (true); extern _Float64 __atan2f64 (_Float64 __y, _Float64 __x) noexcept (true);
 extern _Float64 cosf64 (_Float64 __x) noexcept (true); extern _Float64 __cosf64 (_Float64 __x) noexcept (true);
 extern _Float64 sinf64 (_Float64 __x) noexcept (true); extern _Float64 __sinf64 (_Float64 __x) noexcept (true);
 extern _Float64 tanf64 (_Float64 __x) noexcept (true); extern _Float64 __tanf64 (_Float64 __x) noexcept (true);
extern _Float64 acospif64 (_Float64 __x) noexcept (true); extern _Float64 __acospif64 (_Float64 __x) noexcept (true);
 extern _Float64 acospif64 (_Float64 __x) noexcept (true); extern _Float64 __acospif64 (_Float64 __x) noexcept (true);
extern _Float64 asinpif64 (_Float64 __x) noexcept (true); extern _Float64 __asinpif64 (_Float64 __x) noexcept (true);
 extern _Float64 asinpif64 (_Float64 __x) noexcept (true); extern _Float64 __asinpif64 (_Float64 __x) noexcept (true);
extern _Float64 atanpif64 (_Float64 __x) noexcept (true); extern _Float64 __atanpif64 (_Float64 __x) noexcept (true);
 extern _Float64 atanpif64 (_Float64 __x) noexcept (true); extern _Float64 __atanpif64 (_Float64 __x) noexcept (true);
extern _Float64 atan2pif64 (_Float64 __y, _Float64 __x) noexcept (true); extern _Float64 __atan2pif64 (_Float64 __y, _Float64 __x) noexcept (true);
 extern _Float64 atan2pif64 (_Float64 __y, _Float64 __x) noexcept (true); extern _Float64 __atan2pif64 (_Float64 __y, _Float64 __x) noexcept (true);
 extern _Float64 cospif64 (_Float64 __x) noexcept (true); extern _Float64 __cospif64 (_Float64 __x) noexcept (true);
 extern _Float64 sinpif64 (_Float64 __x) noexcept (true); extern _Float64 __sinpif64 (_Float64 __x) noexcept (true);
 extern _Float64 tanpif64 (_Float64 __x) noexcept (true); extern _Float64 __tanpif64 (_Float64 __x) noexcept (true);
 extern _Float64 coshf64 (_Float64 __x) noexcept (true); extern _Float64 __coshf64 (_Float64 __x) noexcept (true);
 extern _Float64 sinhf64 (_Float64 __x) noexcept (true); extern _Float64 __sinhf64 (_Float64 __x) noexcept (true);
 extern _Float64 tanhf64 (_Float64 __x) noexcept (true); extern _Float64 __tanhf64 (_Float64 __x) noexcept (true);
 extern void sincosf64 (_Float64 __x, _Float64 *__sinx, _Float64 *__cosx) noexcept (true); extern void __sincosf64 (_Float64 __x, _Float64 *__sinx, _Float64 *__cosx) noexcept (true);
 extern _Float64 acoshf64 (_Float64 __x) noexcept (true); extern _Float64 __acoshf64 (_Float64 __x) noexcept (true);
 extern _Float64 asinhf64 (_Float64 __x) noexcept (true); extern _Float64 __asinhf64 (_Float64 __x) noexcept (true);
 extern _Float64 atanhf64 (_Float64 __x) noexcept (true); extern _Float64 __atanhf64 (_Float64 __x) noexcept (true);
 extern _Float64 expf64 (_Float64 __x) noexcept (true); extern _Float64 __expf64 (_Float64 __x) noexcept (true);
extern _Float64 frexpf64 (_Float64 __x, int *__exponent) noexcept (true); extern _Float64 __frexpf64 (_Float64 __x, int *__exponent) noexcept (true);
extern _Float64 ldexpf64 (_Float64 __x, int __exponent) noexcept (true); extern _Float64 __ldexpf64 (_Float64 __x, int __exponent) noexcept (true);
 extern _Float64 logf64 (_Float64 __x) noexcept (true); extern _Float64 __logf64 (_Float64 __x) noexcept (true);
 extern _Float64 log10f64 (_Float64 __x) noexcept (true); extern _Float64 __log10f64 (_Float64 __x) noexcept (true);
extern _Float64 modff64 (_Float64 __x, _Float64 *__iptr) noexcept (true); extern _Float64 __modff64 (_Float64 __x, _Float64 *__iptr) noexcept (true) __attribute__ ((__nonnull__ (2)));
 extern _Float64 exp10f64 (_Float64 __x) noexcept (true); extern _Float64 __exp10f64 (_Float64 __x) noexcept (true);
 extern _Float64 exp2m1f64 (_Float64 __x) noexcept (true); extern _Float64 __exp2m1f64 (_Float64 __x) noexcept (true);
 extern _Float64 exp10m1f64 (_Float64 __x) noexcept (true); extern _Float64 __exp10m1f64 (_Float64 __x) noexcept (true);
 extern _Float64 log2p1f64 (_Float64 __x) noexcept (true); extern _Float64 __log2p1f64 (_Float64 __x) noexcept (true);
 extern _Float64 log10p1f64 (_Float64 __x) noexcept (true); extern _Float64 __log10p1f64 (_Float64 __x) noexcept (true);
 extern _Float64 logp1f64 (_Float64 __x) noexcept (true); extern _Float64 __logp1f64 (_Float64 __x) noexcept (true);
 extern _Float64 expm1f64 (_Float64 __x) noexcept (true); extern _Float64 __expm1f64 (_Float64 __x) noexcept (true);
 extern _Float64 log1pf64 (_Float64 __x) noexcept (true); extern _Float64 __log1pf64 (_Float64 __x) noexcept (true);
extern _Float64 logbf64 (_Float64 __x) noexcept (true); extern _Float64 __logbf64 (_Float64 __x) noexcept (true);
 extern _Float64 exp2f64 (_Float64 __x) noexcept (true); extern _Float64 __exp2f64 (_Float64 __x) noexcept (true);
 extern _Float64 log2f64 (_Float64 __x) noexcept (true); extern _Float64 __log2f64 (_Float64 __x) noexcept (true);
 extern _Float64 powf64 (_Float64 __x, _Float64 __y) noexcept (true); extern _Float64 __powf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float64 sqrtf64 (_Float64 __x) noexcept (true); extern _Float64 __sqrtf64 (_Float64 __x) noexcept (true);
 extern _Float64 hypotf64 (_Float64 __x, _Float64 __y) noexcept (true); extern _Float64 __hypotf64 (_Float64 __x, _Float64 __y) noexcept (true);
 extern _Float64 cbrtf64 (_Float64 __x) noexcept (true); extern _Float64 __cbrtf64 (_Float64 __x) noexcept (true);
extern _Float64 compoundnf64 (_Float64 __x, long long int __y) noexcept (true); extern _Float64 __compoundnf64 (_Float64 __x, long long int __y) noexcept (true);
extern _Float64 pownf64 (_Float64 __x, long long int __y) noexcept (true); extern _Float64 __pownf64 (_Float64 __x, long long int __y) noexcept (true);
 extern _Float64 powrf64 (_Float64 __x, _Float64 __y) noexcept (true); extern _Float64 __powrf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float64 powrf64 (_Float64 __x, _Float64 __y) noexcept (true); extern _Float64 __powrf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float64 rootnf64 (_Float64 __x, long long int __y) noexcept (true); extern _Float64 __rootnf64 (_Float64 __x, long long int __y) noexcept (true);
 extern _Float64 rsqrtf64 (_Float64 __x) noexcept (true); extern _Float64 __rsqrtf64 (_Float64 __x) noexcept (true);
extern _Float64 ceilf64 (_Float64 __x) noexcept (true) __attribute__ ((__const__));
extern _Float64 fabsf64 (_Float64 __x) noexcept (true) __attribute__ ((__const__));
extern _Float64 floorf64 (_Float64 __x) noexcept (true) __attribute__ ((__const__));
extern _Float64 fmodf64 (_Float64 __x, _Float64 __y) noexcept (true); extern _Float64 __fmodf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float64 copysignf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 nanf64 (const char *__tagb) noexcept (true); extern _Float64 __nanf64 (const char *__tagb) noexcept (true);
extern _Float64 j0f64 (_Float64) noexcept (true); extern _Float64 __j0f64 (_Float64) noexcept (true);
extern _Float64 j1f64 (_Float64) noexcept (true); extern _Float64 __j1f64 (_Float64) noexcept (true);
extern _Float64 jnf64 (int, _Float64) noexcept (true); extern _Float64 __jnf64 (int, _Float64) noexcept (true);
extern _Float64 y0f64 (_Float64) noexcept (true); extern _Float64 __y0f64 (_Float64) noexcept (true);
extern _Float64 y1f64 (_Float64) noexcept (true); extern _Float64 __y1f64 (_Float64) noexcept (true);
extern _Float64 ynf64 (int, _Float64) noexcept (true); extern _Float64 __ynf64 (int, _Float64) noexcept (true);
 extern _Float64 erff64 (_Float64) noexcept (true); extern _Float64 __erff64 (_Float64) noexcept (true);
 extern _Float64 erfcf64 (_Float64) noexcept (true); extern _Float64 __erfcf64 (_Float64) noexcept (true);
extern _Float64 lgammaf64 (_Float64) noexcept (true); extern _Float64 __lgammaf64 (_Float64) noexcept (true);
extern _Float64 tgammaf64 (_Float64) noexcept (true); extern _Float64 __tgammaf64 (_Float64) noexcept (true);
extern _Float64 lgammaf64_r (_Float64, int *__signgamp) noexcept (true); extern _Float64 __lgammaf64_r (_Float64, int *__signgamp) noexcept (true);
extern _Float64 rintf64 (_Float64 __x) noexcept (true); extern _Float64 __rintf64 (_Float64 __x) noexcept (true);
extern _Float64 nextafterf64 (_Float64 __x, _Float64 __y) noexcept (true); extern _Float64 __nextafterf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float64 nextdownf64 (_Float64 __x) noexcept (true); extern _Float64 __nextdownf64 (_Float64 __x) noexcept (true);
extern _Float64 nextupf64 (_Float64 __x) noexcept (true); extern _Float64 __nextupf64 (_Float64 __x) noexcept (true);
extern _Float64 remainderf64 (_Float64 __x, _Float64 __y) noexcept (true); extern _Float64 __remainderf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float64 scalbnf64 (_Float64 __x, int __n) noexcept (true); extern _Float64 __scalbnf64 (_Float64 __x, int __n) noexcept (true);
extern int ilogbf64 (_Float64 __x) noexcept (true); extern int __ilogbf64 (_Float64 __x) noexcept (true);
extern long int llogbf64 (_Float64 __x) noexcept (true); extern long int __llogbf64 (_Float64 __x) noexcept (true);
extern _Float64 scalblnf64 (_Float64 __x, long int __n) noexcept (true); extern _Float64 __scalblnf64 (_Float64 __x, long int __n) noexcept (true);
extern _Float64 nearbyintf64 (_Float64 __x) noexcept (true); extern _Float64 __nearbyintf64 (_Float64 __x) noexcept (true);
extern _Float64 roundf64 (_Float64 __x) noexcept (true) __attribute__ ((__const__));
extern _Float64 truncf64 (_Float64 __x) noexcept (true) __attribute__ ((__const__));
extern _Float64 remquof64 (_Float64 __x, _Float64 __y, int *__quo) noexcept (true); extern _Float64 __remquof64 (_Float64 __x, _Float64 __y, int *__quo) noexcept (true);
extern long int lrintf64 (_Float64 __x) noexcept (true); extern long int __lrintf64 (_Float64 __x) noexcept (true);
__extension__
extern long long int llrintf64 (_Float64 __x) noexcept (true); extern long long int __llrintf64 (_Float64 __x) noexcept (true);
extern long int lroundf64 (_Float64 __x) noexcept (true); extern long int __lroundf64 (_Float64 __x) noexcept (true);
__extension__
extern long long int llroundf64 (_Float64 __x) noexcept (true); extern long long int __llroundf64 (_Float64 __x) noexcept (true);
extern _Float64 fdimf64 (_Float64 __x, _Float64 __y) noexcept (true); extern _Float64 __fdimf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float64 fmaxf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fminf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fmaf64 (_Float64 __x, _Float64 __y, _Float64 __z) noexcept (true); extern _Float64 __fmaf64 (_Float64 __x, _Float64 __y, _Float64 __z) noexcept (true);
extern _Float64 roundevenf64 (_Float64 __x) noexcept (true) __attribute__ ((__const__));
extern _Float64 fromfpf64 (_Float64 __x, int __round, unsigned int __width) noexcept (true); extern _Float64 __fromfpf64 (_Float64 __x, int __round, unsigned int __width) noexcept (true);
extern _Float64 ufromfpf64 (_Float64 __x, int __round, unsigned int __width) noexcept (true); extern _Float64 __ufromfpf64 (_Float64 __x, int __round, unsigned int __width) noexcept (true);
extern _Float64 fromfpxf64 (_Float64 __x, int __round, unsigned int __width) noexcept (true); extern _Float64 __fromfpxf64 (_Float64 __x, int __round, unsigned int __width) noexcept (true);
extern _Float64 ufromfpxf64 (_Float64 __x, int __round, unsigned int __width) noexcept (true); extern _Float64 __ufromfpxf64 (_Float64 __x, int __round, unsigned int __width) noexcept (true);
extern int canonicalizef64 (_Float64 *__cx, const _Float64 *__x) noexcept (true);
extern _Float64 fmaxmagf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fminmagf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fmaximumf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fminimumf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fmaximum_numf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fminimum_numf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fmaximum_magf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fminimum_magf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fmaximum_mag_numf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern _Float64 fminimum_mag_numf64 (_Float64 __x, _Float64 __y) noexcept (true) __attribute__ ((__const__));
extern int totalorderf64 (const _Float64 *__x, const _Float64 *__y) noexcept (true)
     __attribute__ ((__pure__));
extern int totalordermagf64 (const _Float64 *__x, const _Float64 *__y) noexcept (true)
     __attribute__ ((__pure__));
extern _Float64 getpayloadf64 (const _Float64 *__x) noexcept (true); extern _Float64 __getpayloadf64 (const _Float64 *__x) noexcept (true);
extern int setpayloadf64 (_Float64 *__x, _Float64 __payload) noexcept (true);
extern int setpayloadsigf64 (_Float64 *__x, _Float64 __payload) noexcept (true);
extern int __fpclassifyf128 (_Float128 __value) noexcept (true)
     __attribute__ ((__const__));
extern int __signbitf128 (_Float128 __value) noexcept (true)
     __attribute__ ((__const__));
extern int __isinff128 (_Float128 __value) noexcept (true)
  __attribute__ ((__const__));
extern int __finitef128 (_Float128 __value) noexcept (true)
  __attribute__ ((__const__));
extern int __isnanf128 (_Float128 __value) noexcept (true)
  __attribute__ ((__const__));
extern int __iseqsigf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern int __issignalingf128 (_Float128 __value) noexcept (true)
     __attribute__ ((__const__));
 extern _Float128 acosf128 (_Float128 __x) noexcept (true); extern _Float128 __acosf128 (_Float128 __x) noexcept (true);
 extern _Float128 asinf128 (_Float128 __x) noexcept (true); extern _Float128 __asinf128 (_Float128 __x) noexcept (true);
 extern _Float128 atanf128 (_Float128 __x) noexcept (true); extern _Float128 __atanf128 (_Float128 __x) noexcept (true);
 extern _Float128 atan2f128 (_Float128 __y, _Float128 __x) noexcept (true); extern _Float128 __atan2f128 (_Float128 __y, _Float128 __x) noexcept (true);
 extern _Float128 cosf128 (_Float128 __x) noexcept (true); extern _Float128 __cosf128 (_Float128 __x) noexcept (true);
 extern _Float128 sinf128 (_Float128 __x) noexcept (true); extern _Float128 __sinf128 (_Float128 __x) noexcept (true);
 extern _Float128 tanf128 (_Float128 __x) noexcept (true); extern _Float128 __tanf128 (_Float128 __x) noexcept (true);
extern _Float128 acospif128 (_Float128 __x) noexcept (true); extern _Float128 __acospif128 (_Float128 __x) noexcept (true);
 extern _Float128 acospif128 (_Float128 __x) noexcept (true); extern _Float128 __acospif128 (_Float128 __x) noexcept (true);
extern _Float128 asinpif128 (_Float128 __x) noexcept (true); extern _Float128 __asinpif128 (_Float128 __x) noexcept (true);
 extern _Float128 asinpif128 (_Float128 __x) noexcept (true); extern _Float128 __asinpif128 (_Float128 __x) noexcept (true);
extern _Float128 atanpif128 (_Float128 __x) noexcept (true); extern _Float128 __atanpif128 (_Float128 __x) noexcept (true);
 extern _Float128 atanpif128 (_Float128 __x) noexcept (true); extern _Float128 __atanpif128 (_Float128 __x) noexcept (true);
extern _Float128 atan2pif128 (_Float128 __y, _Float128 __x) noexcept (true); extern _Float128 __atan2pif128 (_Float128 __y, _Float128 __x) noexcept (true);
 extern _Float128 atan2pif128 (_Float128 __y, _Float128 __x) noexcept (true); extern _Float128 __atan2pif128 (_Float128 __y, _Float128 __x) noexcept (true);
 extern _Float128 cospif128 (_Float128 __x) noexcept (true); extern _Float128 __cospif128 (_Float128 __x) noexcept (true);
 extern _Float128 sinpif128 (_Float128 __x) noexcept (true); extern _Float128 __sinpif128 (_Float128 __x) noexcept (true);
 extern _Float128 tanpif128 (_Float128 __x) noexcept (true); extern _Float128 __tanpif128 (_Float128 __x) noexcept (true);
 extern _Float128 coshf128 (_Float128 __x) noexcept (true); extern _Float128 __coshf128 (_Float128 __x) noexcept (true);
 extern _Float128 sinhf128 (_Float128 __x) noexcept (true); extern _Float128 __sinhf128 (_Float128 __x) noexcept (true);
 extern _Float128 tanhf128 (_Float128 __x) noexcept (true); extern _Float128 __tanhf128 (_Float128 __x) noexcept (true);
 extern void sincosf128 (_Float128 __x, _Float128 *__sinx, _Float128 *__cosx) noexcept (true); extern void __sincosf128 (_Float128 __x, _Float128 *__sinx, _Float128 *__cosx) noexcept (true);
 extern _Float128 acoshf128 (_Float128 __x) noexcept (true); extern _Float128 __acoshf128 (_Float128 __x) noexcept (true);
 extern _Float128 asinhf128 (_Float128 __x) noexcept (true); extern _Float128 __asinhf128 (_Float128 __x) noexcept (true);
 extern _Float128 atanhf128 (_Float128 __x) noexcept (true); extern _Float128 __atanhf128 (_Float128 __x) noexcept (true);
 extern _Float128 expf128 (_Float128 __x) noexcept (true); extern _Float128 __expf128 (_Float128 __x) noexcept (true);
extern _Float128 frexpf128 (_Float128 __x, int *__exponent) noexcept (true); extern _Float128 __frexpf128 (_Float128 __x, int *__exponent) noexcept (true);
extern _Float128 ldexpf128 (_Float128 __x, int __exponent) noexcept (true); extern _Float128 __ldexpf128 (_Float128 __x, int __exponent) noexcept (true);
 extern _Float128 logf128 (_Float128 __x) noexcept (true); extern _Float128 __logf128 (_Float128 __x) noexcept (true);
 extern _Float128 log10f128 (_Float128 __x) noexcept (true); extern _Float128 __log10f128 (_Float128 __x) noexcept (true);
extern _Float128 modff128 (_Float128 __x, _Float128 *__iptr) noexcept (true); extern _Float128 __modff128 (_Float128 __x, _Float128 *__iptr) noexcept (true) __attribute__ ((__nonnull__ (2)));
 extern _Float128 exp10f128 (_Float128 __x) noexcept (true); extern _Float128 __exp10f128 (_Float128 __x) noexcept (true);
 extern _Float128 exp2m1f128 (_Float128 __x) noexcept (true); extern _Float128 __exp2m1f128 (_Float128 __x) noexcept (true);
 extern _Float128 exp10m1f128 (_Float128 __x) noexcept (true); extern _Float128 __exp10m1f128 (_Float128 __x) noexcept (true);
 extern _Float128 log2p1f128 (_Float128 __x) noexcept (true); extern _Float128 __log2p1f128 (_Float128 __x) noexcept (true);
 extern _Float128 log10p1f128 (_Float128 __x) noexcept (true); extern _Float128 __log10p1f128 (_Float128 __x) noexcept (true);
 extern _Float128 logp1f128 (_Float128 __x) noexcept (true); extern _Float128 __logp1f128 (_Float128 __x) noexcept (true);
 extern _Float128 expm1f128 (_Float128 __x) noexcept (true); extern _Float128 __expm1f128 (_Float128 __x) noexcept (true);
 extern _Float128 log1pf128 (_Float128 __x) noexcept (true); extern _Float128 __log1pf128 (_Float128 __x) noexcept (true);
extern _Float128 logbf128 (_Float128 __x) noexcept (true); extern _Float128 __logbf128 (_Float128 __x) noexcept (true);
 extern _Float128 exp2f128 (_Float128 __x) noexcept (true); extern _Float128 __exp2f128 (_Float128 __x) noexcept (true);
 extern _Float128 log2f128 (_Float128 __x) noexcept (true); extern _Float128 __log2f128 (_Float128 __x) noexcept (true);
 extern _Float128 powf128 (_Float128 __x, _Float128 __y) noexcept (true); extern _Float128 __powf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float128 sqrtf128 (_Float128 __x) noexcept (true); extern _Float128 __sqrtf128 (_Float128 __x) noexcept (true);
 extern _Float128 hypotf128 (_Float128 __x, _Float128 __y) noexcept (true); extern _Float128 __hypotf128 (_Float128 __x, _Float128 __y) noexcept (true);
 extern _Float128 cbrtf128 (_Float128 __x) noexcept (true); extern _Float128 __cbrtf128 (_Float128 __x) noexcept (true);
extern _Float128 compoundnf128 (_Float128 __x, long long int __y) noexcept (true); extern _Float128 __compoundnf128 (_Float128 __x, long long int __y) noexcept (true);
extern _Float128 pownf128 (_Float128 __x, long long int __y) noexcept (true); extern _Float128 __pownf128 (_Float128 __x, long long int __y) noexcept (true);
 extern _Float128 powrf128 (_Float128 __x, _Float128 __y) noexcept (true); extern _Float128 __powrf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float128 powrf128 (_Float128 __x, _Float128 __y) noexcept (true); extern _Float128 __powrf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float128 rootnf128 (_Float128 __x, long long int __y) noexcept (true); extern _Float128 __rootnf128 (_Float128 __x, long long int __y) noexcept (true);
 extern _Float128 rsqrtf128 (_Float128 __x) noexcept (true); extern _Float128 __rsqrtf128 (_Float128 __x) noexcept (true);
extern _Float128 ceilf128 (_Float128 __x) noexcept (true) __attribute__ ((__const__));
extern _Float128 fabsf128 (_Float128 __x) noexcept (true) __attribute__ ((__const__));
extern _Float128 floorf128 (_Float128 __x) noexcept (true) __attribute__ ((__const__));
extern _Float128 fmodf128 (_Float128 __x, _Float128 __y) noexcept (true); extern _Float128 __fmodf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float128 copysignf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 nanf128 (const char *__tagb) noexcept (true); extern _Float128 __nanf128 (const char *__tagb) noexcept (true);
extern _Float128 j0f128 (_Float128) noexcept (true); extern _Float128 __j0f128 (_Float128) noexcept (true);
extern _Float128 j1f128 (_Float128) noexcept (true); extern _Float128 __j1f128 (_Float128) noexcept (true);
extern _Float128 jnf128 (int, _Float128) noexcept (true); extern _Float128 __jnf128 (int, _Float128) noexcept (true);
extern _Float128 y0f128 (_Float128) noexcept (true); extern _Float128 __y0f128 (_Float128) noexcept (true);
extern _Float128 y1f128 (_Float128) noexcept (true); extern _Float128 __y1f128 (_Float128) noexcept (true);
extern _Float128 ynf128 (int, _Float128) noexcept (true); extern _Float128 __ynf128 (int, _Float128) noexcept (true);
 extern _Float128 erff128 (_Float128) noexcept (true); extern _Float128 __erff128 (_Float128) noexcept (true);
 extern _Float128 erfcf128 (_Float128) noexcept (true); extern _Float128 __erfcf128 (_Float128) noexcept (true);
extern _Float128 lgammaf128 (_Float128) noexcept (true); extern _Float128 __lgammaf128 (_Float128) noexcept (true);
extern _Float128 tgammaf128 (_Float128) noexcept (true); extern _Float128 __tgammaf128 (_Float128) noexcept (true);
extern _Float128 lgammaf128_r (_Float128, int *__signgamp) noexcept (true); extern _Float128 __lgammaf128_r (_Float128, int *__signgamp) noexcept (true);
extern _Float128 rintf128 (_Float128 __x) noexcept (true); extern _Float128 __rintf128 (_Float128 __x) noexcept (true);
extern _Float128 nextafterf128 (_Float128 __x, _Float128 __y) noexcept (true); extern _Float128 __nextafterf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float128 nextdownf128 (_Float128 __x) noexcept (true); extern _Float128 __nextdownf128 (_Float128 __x) noexcept (true);
extern _Float128 nextupf128 (_Float128 __x) noexcept (true); extern _Float128 __nextupf128 (_Float128 __x) noexcept (true);
extern _Float128 remainderf128 (_Float128 __x, _Float128 __y) noexcept (true); extern _Float128 __remainderf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float128 scalbnf128 (_Float128 __x, int __n) noexcept (true); extern _Float128 __scalbnf128 (_Float128 __x, int __n) noexcept (true);
extern int ilogbf128 (_Float128 __x) noexcept (true); extern int __ilogbf128 (_Float128 __x) noexcept (true);
extern long int llogbf128 (_Float128 __x) noexcept (true); extern long int __llogbf128 (_Float128 __x) noexcept (true);
extern _Float128 scalblnf128 (_Float128 __x, long int __n) noexcept (true); extern _Float128 __scalblnf128 (_Float128 __x, long int __n) noexcept (true);
extern _Float128 nearbyintf128 (_Float128 __x) noexcept (true); extern _Float128 __nearbyintf128 (_Float128 __x) noexcept (true);
extern _Float128 roundf128 (_Float128 __x) noexcept (true) __attribute__ ((__const__));
extern _Float128 truncf128 (_Float128 __x) noexcept (true) __attribute__ ((__const__));
extern _Float128 remquof128 (_Float128 __x, _Float128 __y, int *__quo) noexcept (true); extern _Float128 __remquof128 (_Float128 __x, _Float128 __y, int *__quo) noexcept (true);
extern long int lrintf128 (_Float128 __x) noexcept (true); extern long int __lrintf128 (_Float128 __x) noexcept (true);
__extension__
extern long long int llrintf128 (_Float128 __x) noexcept (true); extern long long int __llrintf128 (_Float128 __x) noexcept (true);
extern long int lroundf128 (_Float128 __x) noexcept (true); extern long int __lroundf128 (_Float128 __x) noexcept (true);
__extension__
extern long long int llroundf128 (_Float128 __x) noexcept (true); extern long long int __llroundf128 (_Float128 __x) noexcept (true);
extern _Float128 fdimf128 (_Float128 __x, _Float128 __y) noexcept (true); extern _Float128 __fdimf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float128 fmaxf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fminf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fmaf128 (_Float128 __x, _Float128 __y, _Float128 __z) noexcept (true); extern _Float128 __fmaf128 (_Float128 __x, _Float128 __y, _Float128 __z) noexcept (true);
extern _Float128 roundevenf128 (_Float128 __x) noexcept (true) __attribute__ ((__const__));
extern _Float128 fromfpf128 (_Float128 __x, int __round, unsigned int __width) noexcept (true); extern _Float128 __fromfpf128 (_Float128 __x, int __round, unsigned int __width) noexcept (true);
extern _Float128 ufromfpf128 (_Float128 __x, int __round, unsigned int __width) noexcept (true); extern _Float128 __ufromfpf128 (_Float128 __x, int __round, unsigned int __width) noexcept (true);
extern _Float128 fromfpxf128 (_Float128 __x, int __round, unsigned int __width) noexcept (true); extern _Float128 __fromfpxf128 (_Float128 __x, int __round, unsigned int __width) noexcept (true);
extern _Float128 ufromfpxf128 (_Float128 __x, int __round, unsigned int __width) noexcept (true); extern _Float128 __ufromfpxf128 (_Float128 __x, int __round, unsigned int __width) noexcept (true);
extern int canonicalizef128 (_Float128 *__cx, const _Float128 *__x) noexcept (true);
extern _Float128 fmaxmagf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fminmagf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fmaximumf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fminimumf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fmaximum_numf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fminimum_numf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fmaximum_magf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fminimum_magf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fmaximum_mag_numf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern _Float128 fminimum_mag_numf128 (_Float128 __x, _Float128 __y) noexcept (true) __attribute__ ((__const__));
extern int totalorderf128 (const _Float128 *__x, const _Float128 *__y) noexcept (true)
     __attribute__ ((__pure__));
extern int totalordermagf128 (const _Float128 *__x, const _Float128 *__y) noexcept (true)
     __attribute__ ((__pure__));
extern _Float128 getpayloadf128 (const _Float128 *__x) noexcept (true); extern _Float128 __getpayloadf128 (const _Float128 *__x) noexcept (true);
extern int setpayloadf128 (_Float128 *__x, _Float128 __payload) noexcept (true);
extern int setpayloadsigf128 (_Float128 *__x, _Float128 __payload) noexcept (true);
 extern _Float32x acosf32x (_Float32x __x) noexcept (true); extern _Float32x __acosf32x (_Float32x __x) noexcept (true);
 extern _Float32x asinf32x (_Float32x __x) noexcept (true); extern _Float32x __asinf32x (_Float32x __x) noexcept (true);
 extern _Float32x atanf32x (_Float32x __x) noexcept (true); extern _Float32x __atanf32x (_Float32x __x) noexcept (true);
 extern _Float32x atan2f32x (_Float32x __y, _Float32x __x) noexcept (true); extern _Float32x __atan2f32x (_Float32x __y, _Float32x __x) noexcept (true);
 extern _Float32x cosf32x (_Float32x __x) noexcept (true); extern _Float32x __cosf32x (_Float32x __x) noexcept (true);
 extern _Float32x sinf32x (_Float32x __x) noexcept (true); extern _Float32x __sinf32x (_Float32x __x) noexcept (true);
 extern _Float32x tanf32x (_Float32x __x) noexcept (true); extern _Float32x __tanf32x (_Float32x __x) noexcept (true);
extern _Float32x acospif32x (_Float32x __x) noexcept (true); extern _Float32x __acospif32x (_Float32x __x) noexcept (true);
 extern _Float32x acospif32x (_Float32x __x) noexcept (true); extern _Float32x __acospif32x (_Float32x __x) noexcept (true);
extern _Float32x asinpif32x (_Float32x __x) noexcept (true); extern _Float32x __asinpif32x (_Float32x __x) noexcept (true);
 extern _Float32x asinpif32x (_Float32x __x) noexcept (true); extern _Float32x __asinpif32x (_Float32x __x) noexcept (true);
extern _Float32x atanpif32x (_Float32x __x) noexcept (true); extern _Float32x __atanpif32x (_Float32x __x) noexcept (true);
 extern _Float32x atanpif32x (_Float32x __x) noexcept (true); extern _Float32x __atanpif32x (_Float32x __x) noexcept (true);
extern _Float32x atan2pif32x (_Float32x __y, _Float32x __x) noexcept (true); extern _Float32x __atan2pif32x (_Float32x __y, _Float32x __x) noexcept (true);
 extern _Float32x atan2pif32x (_Float32x __y, _Float32x __x) noexcept (true); extern _Float32x __atan2pif32x (_Float32x __y, _Float32x __x) noexcept (true);
 extern _Float32x cospif32x (_Float32x __x) noexcept (true); extern _Float32x __cospif32x (_Float32x __x) noexcept (true);
 extern _Float32x sinpif32x (_Float32x __x) noexcept (true); extern _Float32x __sinpif32x (_Float32x __x) noexcept (true);
 extern _Float32x tanpif32x (_Float32x __x) noexcept (true); extern _Float32x __tanpif32x (_Float32x __x) noexcept (true);
 extern _Float32x coshf32x (_Float32x __x) noexcept (true); extern _Float32x __coshf32x (_Float32x __x) noexcept (true);
 extern _Float32x sinhf32x (_Float32x __x) noexcept (true); extern _Float32x __sinhf32x (_Float32x __x) noexcept (true);
 extern _Float32x tanhf32x (_Float32x __x) noexcept (true); extern _Float32x __tanhf32x (_Float32x __x) noexcept (true);
 extern void sincosf32x (_Float32x __x, _Float32x *__sinx, _Float32x *__cosx) noexcept (true); extern void __sincosf32x (_Float32x __x, _Float32x *__sinx, _Float32x *__cosx) noexcept (true);
 extern _Float32x acoshf32x (_Float32x __x) noexcept (true); extern _Float32x __acoshf32x (_Float32x __x) noexcept (true);
 extern _Float32x asinhf32x (_Float32x __x) noexcept (true); extern _Float32x __asinhf32x (_Float32x __x) noexcept (true);
 extern _Float32x atanhf32x (_Float32x __x) noexcept (true); extern _Float32x __atanhf32x (_Float32x __x) noexcept (true);
 extern _Float32x expf32x (_Float32x __x) noexcept (true); extern _Float32x __expf32x (_Float32x __x) noexcept (true);
extern _Float32x frexpf32x (_Float32x __x, int *__exponent) noexcept (true); extern _Float32x __frexpf32x (_Float32x __x, int *__exponent) noexcept (true);
extern _Float32x ldexpf32x (_Float32x __x, int __exponent) noexcept (true); extern _Float32x __ldexpf32x (_Float32x __x, int __exponent) noexcept (true);
 extern _Float32x logf32x (_Float32x __x) noexcept (true); extern _Float32x __logf32x (_Float32x __x) noexcept (true);
 extern _Float32x log10f32x (_Float32x __x) noexcept (true); extern _Float32x __log10f32x (_Float32x __x) noexcept (true);
extern _Float32x modff32x (_Float32x __x, _Float32x *__iptr) noexcept (true); extern _Float32x __modff32x (_Float32x __x, _Float32x *__iptr) noexcept (true) __attribute__ ((__nonnull__ (2)));
 extern _Float32x exp10f32x (_Float32x __x) noexcept (true); extern _Float32x __exp10f32x (_Float32x __x) noexcept (true);
 extern _Float32x exp2m1f32x (_Float32x __x) noexcept (true); extern _Float32x __exp2m1f32x (_Float32x __x) noexcept (true);
 extern _Float32x exp10m1f32x (_Float32x __x) noexcept (true); extern _Float32x __exp10m1f32x (_Float32x __x) noexcept (true);
 extern _Float32x log2p1f32x (_Float32x __x) noexcept (true); extern _Float32x __log2p1f32x (_Float32x __x) noexcept (true);
 extern _Float32x log10p1f32x (_Float32x __x) noexcept (true); extern _Float32x __log10p1f32x (_Float32x __x) noexcept (true);
 extern _Float32x logp1f32x (_Float32x __x) noexcept (true); extern _Float32x __logp1f32x (_Float32x __x) noexcept (true);
 extern _Float32x expm1f32x (_Float32x __x) noexcept (true); extern _Float32x __expm1f32x (_Float32x __x) noexcept (true);
 extern _Float32x log1pf32x (_Float32x __x) noexcept (true); extern _Float32x __log1pf32x (_Float32x __x) noexcept (true);
extern _Float32x logbf32x (_Float32x __x) noexcept (true); extern _Float32x __logbf32x (_Float32x __x) noexcept (true);
 extern _Float32x exp2f32x (_Float32x __x) noexcept (true); extern _Float32x __exp2f32x (_Float32x __x) noexcept (true);
 extern _Float32x log2f32x (_Float32x __x) noexcept (true); extern _Float32x __log2f32x (_Float32x __x) noexcept (true);
 extern _Float32x powf32x (_Float32x __x, _Float32x __y) noexcept (true); extern _Float32x __powf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32x sqrtf32x (_Float32x __x) noexcept (true); extern _Float32x __sqrtf32x (_Float32x __x) noexcept (true);
 extern _Float32x hypotf32x (_Float32x __x, _Float32x __y) noexcept (true); extern _Float32x __hypotf32x (_Float32x __x, _Float32x __y) noexcept (true);
 extern _Float32x cbrtf32x (_Float32x __x) noexcept (true); extern _Float32x __cbrtf32x (_Float32x __x) noexcept (true);
extern _Float32x compoundnf32x (_Float32x __x, long long int __y) noexcept (true); extern _Float32x __compoundnf32x (_Float32x __x, long long int __y) noexcept (true);
extern _Float32x pownf32x (_Float32x __x, long long int __y) noexcept (true); extern _Float32x __pownf32x (_Float32x __x, long long int __y) noexcept (true);
 extern _Float32x powrf32x (_Float32x __x, _Float32x __y) noexcept (true); extern _Float32x __powrf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32x powrf32x (_Float32x __x, _Float32x __y) noexcept (true); extern _Float32x __powrf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32x rootnf32x (_Float32x __x, long long int __y) noexcept (true); extern _Float32x __rootnf32x (_Float32x __x, long long int __y) noexcept (true);
 extern _Float32x rsqrtf32x (_Float32x __x) noexcept (true); extern _Float32x __rsqrtf32x (_Float32x __x) noexcept (true);
extern _Float32x ceilf32x (_Float32x __x) noexcept (true) __attribute__ ((__const__));
extern _Float32x fabsf32x (_Float32x __x) noexcept (true) __attribute__ ((__const__));
extern _Float32x floorf32x (_Float32x __x) noexcept (true) __attribute__ ((__const__));
extern _Float32x fmodf32x (_Float32x __x, _Float32x __y) noexcept (true); extern _Float32x __fmodf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32x copysignf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x nanf32x (const char *__tagb) noexcept (true); extern _Float32x __nanf32x (const char *__tagb) noexcept (true);
extern _Float32x j0f32x (_Float32x) noexcept (true); extern _Float32x __j0f32x (_Float32x) noexcept (true);
extern _Float32x j1f32x (_Float32x) noexcept (true); extern _Float32x __j1f32x (_Float32x) noexcept (true);
extern _Float32x jnf32x (int, _Float32x) noexcept (true); extern _Float32x __jnf32x (int, _Float32x) noexcept (true);
extern _Float32x y0f32x (_Float32x) noexcept (true); extern _Float32x __y0f32x (_Float32x) noexcept (true);
extern _Float32x y1f32x (_Float32x) noexcept (true); extern _Float32x __y1f32x (_Float32x) noexcept (true);
extern _Float32x ynf32x (int, _Float32x) noexcept (true); extern _Float32x __ynf32x (int, _Float32x) noexcept (true);
 extern _Float32x erff32x (_Float32x) noexcept (true); extern _Float32x __erff32x (_Float32x) noexcept (true);
 extern _Float32x erfcf32x (_Float32x) noexcept (true); extern _Float32x __erfcf32x (_Float32x) noexcept (true);
extern _Float32x lgammaf32x (_Float32x) noexcept (true); extern _Float32x __lgammaf32x (_Float32x) noexcept (true);
extern _Float32x tgammaf32x (_Float32x) noexcept (true); extern _Float32x __tgammaf32x (_Float32x) noexcept (true);
extern _Float32x lgammaf32x_r (_Float32x, int *__signgamp) noexcept (true); extern _Float32x __lgammaf32x_r (_Float32x, int *__signgamp) noexcept (true);
extern _Float32x rintf32x (_Float32x __x) noexcept (true); extern _Float32x __rintf32x (_Float32x __x) noexcept (true);
extern _Float32x nextafterf32x (_Float32x __x, _Float32x __y) noexcept (true); extern _Float32x __nextafterf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32x nextdownf32x (_Float32x __x) noexcept (true); extern _Float32x __nextdownf32x (_Float32x __x) noexcept (true);
extern _Float32x nextupf32x (_Float32x __x) noexcept (true); extern _Float32x __nextupf32x (_Float32x __x) noexcept (true);
extern _Float32x remainderf32x (_Float32x __x, _Float32x __y) noexcept (true); extern _Float32x __remainderf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32x scalbnf32x (_Float32x __x, int __n) noexcept (true); extern _Float32x __scalbnf32x (_Float32x __x, int __n) noexcept (true);
extern int ilogbf32x (_Float32x __x) noexcept (true); extern int __ilogbf32x (_Float32x __x) noexcept (true);
extern long int llogbf32x (_Float32x __x) noexcept (true); extern long int __llogbf32x (_Float32x __x) noexcept (true);
extern _Float32x scalblnf32x (_Float32x __x, long int __n) noexcept (true); extern _Float32x __scalblnf32x (_Float32x __x, long int __n) noexcept (true);
extern _Float32x nearbyintf32x (_Float32x __x) noexcept (true); extern _Float32x __nearbyintf32x (_Float32x __x) noexcept (true);
extern _Float32x roundf32x (_Float32x __x) noexcept (true) __attribute__ ((__const__));
extern _Float32x truncf32x (_Float32x __x) noexcept (true) __attribute__ ((__const__));
extern _Float32x remquof32x (_Float32x __x, _Float32x __y, int *__quo) noexcept (true); extern _Float32x __remquof32x (_Float32x __x, _Float32x __y, int *__quo) noexcept (true);
extern long int lrintf32x (_Float32x __x) noexcept (true); extern long int __lrintf32x (_Float32x __x) noexcept (true);
__extension__
extern long long int llrintf32x (_Float32x __x) noexcept (true); extern long long int __llrintf32x (_Float32x __x) noexcept (true);
extern long int lroundf32x (_Float32x __x) noexcept (true); extern long int __lroundf32x (_Float32x __x) noexcept (true);
__extension__
extern long long int llroundf32x (_Float32x __x) noexcept (true); extern long long int __llroundf32x (_Float32x __x) noexcept (true);
extern _Float32x fdimf32x (_Float32x __x, _Float32x __y) noexcept (true); extern _Float32x __fdimf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32x fmaxf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fminf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fmaf32x (_Float32x __x, _Float32x __y, _Float32x __z) noexcept (true); extern _Float32x __fmaf32x (_Float32x __x, _Float32x __y, _Float32x __z) noexcept (true);
extern _Float32x roundevenf32x (_Float32x __x) noexcept (true) __attribute__ ((__const__));
extern _Float32x fromfpf32x (_Float32x __x, int __round, unsigned int __width) noexcept (true); extern _Float32x __fromfpf32x (_Float32x __x, int __round, unsigned int __width) noexcept (true);
extern _Float32x ufromfpf32x (_Float32x __x, int __round, unsigned int __width) noexcept (true); extern _Float32x __ufromfpf32x (_Float32x __x, int __round, unsigned int __width) noexcept (true);
extern _Float32x fromfpxf32x (_Float32x __x, int __round, unsigned int __width) noexcept (true); extern _Float32x __fromfpxf32x (_Float32x __x, int __round, unsigned int __width) noexcept (true);
extern _Float32x ufromfpxf32x (_Float32x __x, int __round, unsigned int __width) noexcept (true); extern _Float32x __ufromfpxf32x (_Float32x __x, int __round, unsigned int __width) noexcept (true);
extern int canonicalizef32x (_Float32x *__cx, const _Float32x *__x) noexcept (true);
extern _Float32x fmaxmagf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fminmagf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fmaximumf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fminimumf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fmaximum_numf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fminimum_numf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fmaximum_magf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fminimum_magf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fmaximum_mag_numf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern _Float32x fminimum_mag_numf32x (_Float32x __x, _Float32x __y) noexcept (true) __attribute__ ((__const__));
extern int totalorderf32x (const _Float32x *__x, const _Float32x *__y) noexcept (true)
     __attribute__ ((__pure__));
extern int totalordermagf32x (const _Float32x *__x, const _Float32x *__y) noexcept (true)
     __attribute__ ((__pure__));
extern _Float32x getpayloadf32x (const _Float32x *__x) noexcept (true); extern _Float32x __getpayloadf32x (const _Float32x *__x) noexcept (true);
extern int setpayloadf32x (_Float32x *__x, _Float32x __payload) noexcept (true);
extern int setpayloadsigf32x (_Float32x *__x, _Float32x __payload) noexcept (true);
 extern _Float64x acosf64x (_Float64x __x) noexcept (true); extern _Float64x __acosf64x (_Float64x __x) noexcept (true);
 extern _Float64x asinf64x (_Float64x __x) noexcept (true); extern _Float64x __asinf64x (_Float64x __x) noexcept (true);
 extern _Float64x atanf64x (_Float64x __x) noexcept (true); extern _Float64x __atanf64x (_Float64x __x) noexcept (true);
 extern _Float64x atan2f64x (_Float64x __y, _Float64x __x) noexcept (true); extern _Float64x __atan2f64x (_Float64x __y, _Float64x __x) noexcept (true);
 extern _Float64x cosf64x (_Float64x __x) noexcept (true); extern _Float64x __cosf64x (_Float64x __x) noexcept (true);
 extern _Float64x sinf64x (_Float64x __x) noexcept (true); extern _Float64x __sinf64x (_Float64x __x) noexcept (true);
 extern _Float64x tanf64x (_Float64x __x) noexcept (true); extern _Float64x __tanf64x (_Float64x __x) noexcept (true);
extern _Float64x acospif64x (_Float64x __x) noexcept (true); extern _Float64x __acospif64x (_Float64x __x) noexcept (true);
 extern _Float64x acospif64x (_Float64x __x) noexcept (true); extern _Float64x __acospif64x (_Float64x __x) noexcept (true);
extern _Float64x asinpif64x (_Float64x __x) noexcept (true); extern _Float64x __asinpif64x (_Float64x __x) noexcept (true);
 extern _Float64x asinpif64x (_Float64x __x) noexcept (true); extern _Float64x __asinpif64x (_Float64x __x) noexcept (true);
extern _Float64x atanpif64x (_Float64x __x) noexcept (true); extern _Float64x __atanpif64x (_Float64x __x) noexcept (true);
 extern _Float64x atanpif64x (_Float64x __x) noexcept (true); extern _Float64x __atanpif64x (_Float64x __x) noexcept (true);
extern _Float64x atan2pif64x (_Float64x __y, _Float64x __x) noexcept (true); extern _Float64x __atan2pif64x (_Float64x __y, _Float64x __x) noexcept (true);
 extern _Float64x atan2pif64x (_Float64x __y, _Float64x __x) noexcept (true); extern _Float64x __atan2pif64x (_Float64x __y, _Float64x __x) noexcept (true);
 extern _Float64x cospif64x (_Float64x __x) noexcept (true); extern _Float64x __cospif64x (_Float64x __x) noexcept (true);
 extern _Float64x sinpif64x (_Float64x __x) noexcept (true); extern _Float64x __sinpif64x (_Float64x __x) noexcept (true);
 extern _Float64x tanpif64x (_Float64x __x) noexcept (true); extern _Float64x __tanpif64x (_Float64x __x) noexcept (true);
 extern _Float64x coshf64x (_Float64x __x) noexcept (true); extern _Float64x __coshf64x (_Float64x __x) noexcept (true);
 extern _Float64x sinhf64x (_Float64x __x) noexcept (true); extern _Float64x __sinhf64x (_Float64x __x) noexcept (true);
 extern _Float64x tanhf64x (_Float64x __x) noexcept (true); extern _Float64x __tanhf64x (_Float64x __x) noexcept (true);
 extern void sincosf64x (_Float64x __x, _Float64x *__sinx, _Float64x *__cosx) noexcept (true); extern void __sincosf64x (_Float64x __x, _Float64x *__sinx, _Float64x *__cosx) noexcept (true);
 extern _Float64x acoshf64x (_Float64x __x) noexcept (true); extern _Float64x __acoshf64x (_Float64x __x) noexcept (true);
 extern _Float64x asinhf64x (_Float64x __x) noexcept (true); extern _Float64x __asinhf64x (_Float64x __x) noexcept (true);
 extern _Float64x atanhf64x (_Float64x __x) noexcept (true); extern _Float64x __atanhf64x (_Float64x __x) noexcept (true);
 extern _Float64x expf64x (_Float64x __x) noexcept (true); extern _Float64x __expf64x (_Float64x __x) noexcept (true);
extern _Float64x frexpf64x (_Float64x __x, int *__exponent) noexcept (true); extern _Float64x __frexpf64x (_Float64x __x, int *__exponent) noexcept (true);
extern _Float64x ldexpf64x (_Float64x __x, int __exponent) noexcept (true); extern _Float64x __ldexpf64x (_Float64x __x, int __exponent) noexcept (true);
 extern _Float64x logf64x (_Float64x __x) noexcept (true); extern _Float64x __logf64x (_Float64x __x) noexcept (true);
 extern _Float64x log10f64x (_Float64x __x) noexcept (true); extern _Float64x __log10f64x (_Float64x __x) noexcept (true);
extern _Float64x modff64x (_Float64x __x, _Float64x *__iptr) noexcept (true); extern _Float64x __modff64x (_Float64x __x, _Float64x *__iptr) noexcept (true) __attribute__ ((__nonnull__ (2)));
 extern _Float64x exp10f64x (_Float64x __x) noexcept (true); extern _Float64x __exp10f64x (_Float64x __x) noexcept (true);
 extern _Float64x exp2m1f64x (_Float64x __x) noexcept (true); extern _Float64x __exp2m1f64x (_Float64x __x) noexcept (true);
 extern _Float64x exp10m1f64x (_Float64x __x) noexcept (true); extern _Float64x __exp10m1f64x (_Float64x __x) noexcept (true);
 extern _Float64x log2p1f64x (_Float64x __x) noexcept (true); extern _Float64x __log2p1f64x (_Float64x __x) noexcept (true);
 extern _Float64x log10p1f64x (_Float64x __x) noexcept (true); extern _Float64x __log10p1f64x (_Float64x __x) noexcept (true);
 extern _Float64x logp1f64x (_Float64x __x) noexcept (true); extern _Float64x __logp1f64x (_Float64x __x) noexcept (true);
 extern _Float64x expm1f64x (_Float64x __x) noexcept (true); extern _Float64x __expm1f64x (_Float64x __x) noexcept (true);
 extern _Float64x log1pf64x (_Float64x __x) noexcept (true); extern _Float64x __log1pf64x (_Float64x __x) noexcept (true);
extern _Float64x logbf64x (_Float64x __x) noexcept (true); extern _Float64x __logbf64x (_Float64x __x) noexcept (true);
 extern _Float64x exp2f64x (_Float64x __x) noexcept (true); extern _Float64x __exp2f64x (_Float64x __x) noexcept (true);
 extern _Float64x log2f64x (_Float64x __x) noexcept (true); extern _Float64x __log2f64x (_Float64x __x) noexcept (true);
 extern _Float64x powf64x (_Float64x __x, _Float64x __y) noexcept (true); extern _Float64x __powf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64x sqrtf64x (_Float64x __x) noexcept (true); extern _Float64x __sqrtf64x (_Float64x __x) noexcept (true);
 extern _Float64x hypotf64x (_Float64x __x, _Float64x __y) noexcept (true); extern _Float64x __hypotf64x (_Float64x __x, _Float64x __y) noexcept (true);
 extern _Float64x cbrtf64x (_Float64x __x) noexcept (true); extern _Float64x __cbrtf64x (_Float64x __x) noexcept (true);
extern _Float64x compoundnf64x (_Float64x __x, long long int __y) noexcept (true); extern _Float64x __compoundnf64x (_Float64x __x, long long int __y) noexcept (true);
extern _Float64x pownf64x (_Float64x __x, long long int __y) noexcept (true); extern _Float64x __pownf64x (_Float64x __x, long long int __y) noexcept (true);
 extern _Float64x powrf64x (_Float64x __x, _Float64x __y) noexcept (true); extern _Float64x __powrf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64x powrf64x (_Float64x __x, _Float64x __y) noexcept (true); extern _Float64x __powrf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64x rootnf64x (_Float64x __x, long long int __y) noexcept (true); extern _Float64x __rootnf64x (_Float64x __x, long long int __y) noexcept (true);
 extern _Float64x rsqrtf64x (_Float64x __x) noexcept (true); extern _Float64x __rsqrtf64x (_Float64x __x) noexcept (true);
extern _Float64x ceilf64x (_Float64x __x) noexcept (true) __attribute__ ((__const__));
extern _Float64x fabsf64x (_Float64x __x) noexcept (true) __attribute__ ((__const__));
extern _Float64x floorf64x (_Float64x __x) noexcept (true) __attribute__ ((__const__));
extern _Float64x fmodf64x (_Float64x __x, _Float64x __y) noexcept (true); extern _Float64x __fmodf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64x copysignf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x nanf64x (const char *__tagb) noexcept (true); extern _Float64x __nanf64x (const char *__tagb) noexcept (true);
extern _Float64x j0f64x (_Float64x) noexcept (true); extern _Float64x __j0f64x (_Float64x) noexcept (true);
extern _Float64x j1f64x (_Float64x) noexcept (true); extern _Float64x __j1f64x (_Float64x) noexcept (true);
extern _Float64x jnf64x (int, _Float64x) noexcept (true); extern _Float64x __jnf64x (int, _Float64x) noexcept (true);
extern _Float64x y0f64x (_Float64x) noexcept (true); extern _Float64x __y0f64x (_Float64x) noexcept (true);
extern _Float64x y1f64x (_Float64x) noexcept (true); extern _Float64x __y1f64x (_Float64x) noexcept (true);
extern _Float64x ynf64x (int, _Float64x) noexcept (true); extern _Float64x __ynf64x (int, _Float64x) noexcept (true);
 extern _Float64x erff64x (_Float64x) noexcept (true); extern _Float64x __erff64x (_Float64x) noexcept (true);
 extern _Float64x erfcf64x (_Float64x) noexcept (true); extern _Float64x __erfcf64x (_Float64x) noexcept (true);
extern _Float64x lgammaf64x (_Float64x) noexcept (true); extern _Float64x __lgammaf64x (_Float64x) noexcept (true);
extern _Float64x tgammaf64x (_Float64x) noexcept (true); extern _Float64x __tgammaf64x (_Float64x) noexcept (true);
extern _Float64x lgammaf64x_r (_Float64x, int *__signgamp) noexcept (true); extern _Float64x __lgammaf64x_r (_Float64x, int *__signgamp) noexcept (true);
extern _Float64x rintf64x (_Float64x __x) noexcept (true); extern _Float64x __rintf64x (_Float64x __x) noexcept (true);
extern _Float64x nextafterf64x (_Float64x __x, _Float64x __y) noexcept (true); extern _Float64x __nextafterf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64x nextdownf64x (_Float64x __x) noexcept (true); extern _Float64x __nextdownf64x (_Float64x __x) noexcept (true);
extern _Float64x nextupf64x (_Float64x __x) noexcept (true); extern _Float64x __nextupf64x (_Float64x __x) noexcept (true);
extern _Float64x remainderf64x (_Float64x __x, _Float64x __y) noexcept (true); extern _Float64x __remainderf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64x scalbnf64x (_Float64x __x, int __n) noexcept (true); extern _Float64x __scalbnf64x (_Float64x __x, int __n) noexcept (true);
extern int ilogbf64x (_Float64x __x) noexcept (true); extern int __ilogbf64x (_Float64x __x) noexcept (true);
extern long int llogbf64x (_Float64x __x) noexcept (true); extern long int __llogbf64x (_Float64x __x) noexcept (true);
extern _Float64x scalblnf64x (_Float64x __x, long int __n) noexcept (true); extern _Float64x __scalblnf64x (_Float64x __x, long int __n) noexcept (true);
extern _Float64x nearbyintf64x (_Float64x __x) noexcept (true); extern _Float64x __nearbyintf64x (_Float64x __x) noexcept (true);
extern _Float64x roundf64x (_Float64x __x) noexcept (true) __attribute__ ((__const__));
extern _Float64x truncf64x (_Float64x __x) noexcept (true) __attribute__ ((__const__));
extern _Float64x remquof64x (_Float64x __x, _Float64x __y, int *__quo) noexcept (true); extern _Float64x __remquof64x (_Float64x __x, _Float64x __y, int *__quo) noexcept (true);
extern long int lrintf64x (_Float64x __x) noexcept (true); extern long int __lrintf64x (_Float64x __x) noexcept (true);
__extension__
extern long long int llrintf64x (_Float64x __x) noexcept (true); extern long long int __llrintf64x (_Float64x __x) noexcept (true);
extern long int lroundf64x (_Float64x __x) noexcept (true); extern long int __lroundf64x (_Float64x __x) noexcept (true);
__extension__
extern long long int llroundf64x (_Float64x __x) noexcept (true); extern long long int __llroundf64x (_Float64x __x) noexcept (true);
extern _Float64x fdimf64x (_Float64x __x, _Float64x __y) noexcept (true); extern _Float64x __fdimf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64x fmaxf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fminf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fmaf64x (_Float64x __x, _Float64x __y, _Float64x __z) noexcept (true); extern _Float64x __fmaf64x (_Float64x __x, _Float64x __y, _Float64x __z) noexcept (true);
extern _Float64x roundevenf64x (_Float64x __x) noexcept (true) __attribute__ ((__const__));
extern _Float64x fromfpf64x (_Float64x __x, int __round, unsigned int __width) noexcept (true); extern _Float64x __fromfpf64x (_Float64x __x, int __round, unsigned int __width) noexcept (true);
extern _Float64x ufromfpf64x (_Float64x __x, int __round, unsigned int __width) noexcept (true); extern _Float64x __ufromfpf64x (_Float64x __x, int __round, unsigned int __width) noexcept (true);
extern _Float64x fromfpxf64x (_Float64x __x, int __round, unsigned int __width) noexcept (true); extern _Float64x __fromfpxf64x (_Float64x __x, int __round, unsigned int __width) noexcept (true);
extern _Float64x ufromfpxf64x (_Float64x __x, int __round, unsigned int __width) noexcept (true); extern _Float64x __ufromfpxf64x (_Float64x __x, int __round, unsigned int __width) noexcept (true);
extern int canonicalizef64x (_Float64x *__cx, const _Float64x *__x) noexcept (true);
extern _Float64x fmaxmagf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fminmagf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fmaximumf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fminimumf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fmaximum_numf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fminimum_numf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fmaximum_magf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fminimum_magf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fmaximum_mag_numf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern _Float64x fminimum_mag_numf64x (_Float64x __x, _Float64x __y) noexcept (true) __attribute__ ((__const__));
extern int totalorderf64x (const _Float64x *__x, const _Float64x *__y) noexcept (true)
     __attribute__ ((__pure__));
extern int totalordermagf64x (const _Float64x *__x, const _Float64x *__y) noexcept (true)
     __attribute__ ((__pure__));
extern _Float64x getpayloadf64x (const _Float64x *__x) noexcept (true); extern _Float64x __getpayloadf64x (const _Float64x *__x) noexcept (true);
extern int setpayloadf64x (_Float64x *__x, _Float64x __payload) noexcept (true);
extern int setpayloadsigf64x (_Float64x *__x, _Float64x __payload) noexcept (true);
extern float fadd (double __x, double __y) noexcept (true);
extern float fdiv (double __x, double __y) noexcept (true);
extern float ffma (double __x, double __y, double __z) noexcept (true);
extern float fmul (double __x, double __y) noexcept (true);
extern float fsqrt (double __x) noexcept (true);
extern float fsub (double __x, double __y) noexcept (true);
extern float faddl (long double __x, long double __y) noexcept (true);
extern float fdivl (long double __x, long double __y) noexcept (true);
extern float ffmal (long double __x, long double __y, long double __z) noexcept (true);
extern float fmull (long double __x, long double __y) noexcept (true);
extern float fsqrtl (long double __x) noexcept (true);
extern float fsubl (long double __x, long double __y) noexcept (true);
extern double daddl (long double __x, long double __y) noexcept (true);
extern double ddivl (long double __x, long double __y) noexcept (true);
extern double dfmal (long double __x, long double __y, long double __z) noexcept (true);
extern double dmull (long double __x, long double __y) noexcept (true);
extern double dsqrtl (long double __x) noexcept (true);
extern double dsubl (long double __x, long double __y) noexcept (true);
extern _Float32 f32addf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32 f32divf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32 f32fmaf32x (_Float32x __x, _Float32x __y, _Float32x __z) noexcept (true);
extern _Float32 f32mulf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32 f32sqrtf32x (_Float32x __x) noexcept (true);
extern _Float32 f32subf32x (_Float32x __x, _Float32x __y) noexcept (true);
extern _Float32 f32addf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float32 f32divf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float32 f32fmaf64 (_Float64 __x, _Float64 __y, _Float64 __z) noexcept (true);
extern _Float32 f32mulf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float32 f32sqrtf64 (_Float64 __x) noexcept (true);
extern _Float32 f32subf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float32 f32addf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float32 f32divf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float32 f32fmaf64x (_Float64x __x, _Float64x __y, _Float64x __z) noexcept (true);
extern _Float32 f32mulf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float32 f32sqrtf64x (_Float64x __x) noexcept (true);
extern _Float32 f32subf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float32 f32addf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float32 f32divf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float32 f32fmaf128 (_Float128 __x, _Float128 __y, _Float128 __z) noexcept (true);
extern _Float32 f32mulf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float32 f32sqrtf128 (_Float128 __x) noexcept (true);
extern _Float32 f32subf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float32x f32xaddf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float32x f32xdivf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float32x f32xfmaf64 (_Float64 __x, _Float64 __y, _Float64 __z) noexcept (true);
extern _Float32x f32xmulf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float32x f32xsqrtf64 (_Float64 __x) noexcept (true);
extern _Float32x f32xsubf64 (_Float64 __x, _Float64 __y) noexcept (true);
extern _Float32x f32xaddf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float32x f32xdivf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float32x f32xfmaf64x (_Float64x __x, _Float64x __y, _Float64x __z) noexcept (true);
extern _Float32x f32xmulf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float32x f32xsqrtf64x (_Float64x __x) noexcept (true);
extern _Float32x f32xsubf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float32x f32xaddf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float32x f32xdivf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float32x f32xfmaf128 (_Float128 __x, _Float128 __y, _Float128 __z) noexcept (true);
extern _Float32x f32xmulf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float32x f32xsqrtf128 (_Float128 __x) noexcept (true);
extern _Float32x f32xsubf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float64 f64addf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64 f64divf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64 f64fmaf64x (_Float64x __x, _Float64x __y, _Float64x __z) noexcept (true);
extern _Float64 f64mulf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64 f64sqrtf64x (_Float64x __x) noexcept (true);
extern _Float64 f64subf64x (_Float64x __x, _Float64x __y) noexcept (true);
extern _Float64 f64addf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float64 f64divf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float64 f64fmaf128 (_Float128 __x, _Float128 __y, _Float128 __z) noexcept (true);
extern _Float64 f64mulf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float64 f64sqrtf128 (_Float128 __x) noexcept (true);
extern _Float64 f64subf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float64x f64xaddf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float64x f64xdivf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float64x f64xfmaf128 (_Float128 __x, _Float128 __y, _Float128 __z) noexcept (true);
extern _Float64x f64xmulf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern _Float64x f64xsqrtf128 (_Float128 __x) noexcept (true);
extern _Float64x f64xsubf128 (_Float128 __x, _Float128 __y) noexcept (true);
extern int signgam;
enum
  {
    FP_NAN =
      0,
    FP_INFINITE =
      1,
    FP_ZERO =
      2,
    FP_SUBNORMAL =
      3,
    FP_NORMAL =
      4
  };
extern int __iscanonicall (long double __x)
     noexcept (true) __attribute__ ((__const__));
extern "C++" {
inline int iscanonical (float __val) { return ((void) (__typeof (__val)) (__val), 1); }
inline int iscanonical (double __val) { return ((void) (__typeof (__val)) (__val), 1); }
inline int iscanonical (long double __val) { return __iscanonicall (__val); }
inline int iscanonical (_Float128 __val) { return ((void) (__typeof (__val)) (__val), 1); }
}
extern "C++" {
inline int issignaling (float __val) { return __issignalingf (__val); }
inline int issignaling (double __val) { return __issignaling (__val); }
inline int
issignaling (long double __val)
{
  return __issignalingl (__val);
}
inline int issignaling (_Float128 __val) { return __issignalingf128 (__val); }
}
extern "C++" {
template <class __T> inline bool
iszero (__T __val)
{
  return __val == 0;
}
}
extern "C++" {
template<typename> struct __iseqsig_type;
template<> struct __iseqsig_type<float>
{
  static int __call (float __x, float __y) throw ()
  {
    return __iseqsigf (__x, __y);
  }
};
template<> struct __iseqsig_type<double>
{
  static int __call (double __x, double __y) throw ()
  {
    return __iseqsig (__x, __y);
  }
};
template<> struct __iseqsig_type<long double>
{
  static int __call (long double __x, long double __y) throw ()
  {
    return __iseqsigl (__x, __y);
  }
};
template<> struct __iseqsig_type<_Float32>
{
  static int __call (_Float32 __x, _Float32 __y) throw ()
  {
    return __iseqsigf (__x, __y);
  }
};
template<> struct __iseqsig_type<_Float64>
{
  static int __call (_Float64 __x, _Float64 __y) throw ()
  {
    return __iseqsig (__x, __y);
  }
};
template<> struct __iseqsig_type<_Float128>
{
  static int __call (_Float128 __x, _Float128 __y) throw ()
  {
    return __iseqsigf128 (__x, __y);
  }
};
template<> struct __iseqsig_type<_Float32x>
{
  static int __call (_Float32x __x, _Float32x __y) throw ()
  {
    return __iseqsig (__x, __y);
  }
};
template<> struct __iseqsig_type<_Float64x>
{
  static int __call (_Float64x __x, _Float64x __y) throw ()
  {
    return __iseqsigl (__x, __y);
  }
};
template<typename _T1, typename _T2>
inline int
iseqsig (_T1 __x, _T2 __y) throw ()
{
  typedef decltype (((__x) + (__y) + 0.0f)) _T3;
  return __iseqsig_type<_T3>::__call (__x, __y);
}
}
}
 
#pragma GCC diagnostic pop
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wlong-long"
typedef long unsigned int size_t;
extern "C" {
typedef struct
  {
    int quot;
    int rem;
  } div_t;
typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;
__extension__ typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;
extern size_t __ctype_get_mb_cur_max (void) noexcept (true) ;
extern double atof (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;
extern int atoi (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;
extern long int atol (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;
__extension__ extern long long int atoll (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;
extern double strtod (const char *__restrict __nptr,
        char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern float strtof (const char *__restrict __nptr,
       char **__restrict __endptr) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern long double strtold (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern _Float32 strtof32 (const char *__restrict __nptr,
     char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern _Float64 strtof64 (const char *__restrict __nptr,
     char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern _Float128 strtof128 (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern _Float32x strtof32x (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern _Float64x strtof64x (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern long int strtol (const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern unsigned long int strtoul (const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
__extension__
extern long long int strtoq (const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
__extension__
extern long long int strtoll (const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern long int strtol (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtol")
     __attribute__ ((__nonnull__ (1)));
extern unsigned long int strtoul (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoul")
     __attribute__ ((__nonnull__ (1)));
__extension__
extern long long int strtoq (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoll")
     __attribute__ ((__nonnull__ (1)));
__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoull")
     __attribute__ ((__nonnull__ (1)));
__extension__
extern long long int strtoll (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoll")
     __attribute__ ((__nonnull__ (1)));
__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr, char **__restrict __endptr, int __base) noexcept (true) __asm__ ("" "__isoc23_strtoull")
     __attribute__ ((__nonnull__ (1)));
extern int strfromd (char *__dest, size_t __size, const char *__format,
       double __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
extern int strfromf (char *__dest, size_t __size, const char *__format,
       float __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
extern int strfroml (char *__dest, size_t __size, const char *__format,
       long double __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
extern int strfromf32 (char *__dest, size_t __size, const char * __format,
         _Float32 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
extern int strfromf64 (char *__dest, size_t __size, const char * __format,
         _Float64 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
extern int strfromf128 (char *__dest, size_t __size, const char * __format,
   _Float128 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
extern int strfromf32x (char *__dest, size_t __size, const char * __format,
   _Float32x __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
extern int strfromf64x (char *__dest, size_t __size, const char * __format,
   _Float64x __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
struct __locale_struct
{
  struct __locale_data *__locales[13];
  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;
  const char *__names[13];
};
typedef struct __locale_struct *__locale_t;
typedef __locale_t locale_t;
extern long int strtol_l (const char *__restrict __nptr,
     char **__restrict __endptr, int __base,
     locale_t __loc) noexcept (true) __attribute__ ((__nonnull__ (1, 4)));
extern unsigned long int strtoul_l (const char *__restrict __nptr,
        char **__restrict __endptr,
        int __base, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));
__extension__
extern long long int strtoll_l (const char *__restrict __nptr,
    char **__restrict __endptr, int __base,
    locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));
__extension__
extern unsigned long long int strtoull_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       int __base, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));
extern long int strtol_l (const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) noexcept (true) __asm__ ("" "__isoc23_strtol_l")
     __attribute__ ((__nonnull__ (1, 4)));
extern unsigned long int strtoul_l (const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) noexcept (true) __asm__ ("" "__isoc23_strtoul_l")
     __attribute__ ((__nonnull__ (1, 4)));
__extension__
extern long long int strtoll_l (const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) noexcept (true) __asm__ ("" "__isoc23_strtoll_l")
     __attribute__ ((__nonnull__ (1, 4)));
__extension__
extern unsigned long long int strtoull_l (const char *__restrict __nptr, char **__restrict __endptr, int __base, locale_t __loc) noexcept (true) __asm__ ("" "__isoc23_strtoull_l")
     __attribute__ ((__nonnull__ (1, 4)));
extern double strtod_l (const char *__restrict __nptr,
   char **__restrict __endptr, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
extern float strtof_l (const char *__restrict __nptr,
         char **__restrict __endptr, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
extern long double strtold_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
extern _Float32 strtof32_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
extern _Float64 strtof64_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
extern _Float128 strtof128_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
extern _Float32x strtof32x_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
extern _Float64x strtof64x_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
extern char *l64a (long int __n) noexcept (true) ;
extern long int a64l (const char *__s)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;
extern "C" {
typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;
typedef __loff_t loff_t;
typedef __ino_t ino_t;
typedef __ino64_t ino64_t;
typedef __dev_t dev_t;
typedef __gid_t gid_t;
typedef __mode_t mode_t;
typedef __nlink_t nlink_t;
typedef __uid_t uid_t;
typedef __off_t off_t;
typedef __off64_t off64_t;
typedef __pid_t pid_t;
typedef __id_t id_t;
typedef __ssize_t ssize_t;
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;
typedef __key_t key_t;
typedef __clock_t clock_t;
typedef __clockid_t clockid_t;
typedef __time_t time_t;
typedef __timer_t timer_t;
typedef __useconds_t useconds_t;
typedef __suseconds_t suseconds_t;
typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef __uint8_t u_int8_t;
typedef __uint16_t u_int16_t;
typedef __uint32_t u_int32_t;
typedef __uint64_t u_int64_t;
typedef int register_t __attribute__ ((__mode__ (__word__)));
static __inline __uint16_t
__bswap_16 (__uint16_t __bsx)
{
  return __builtin_bswap16 (__bsx);
}
static __inline __uint32_t
__bswap_32 (__uint32_t __bsx)
{
  return __builtin_bswap32 (__bsx);
}
__extension__ static __inline __uint64_t
__bswap_64 (__uint64_t __bsx)
{
  return __builtin_bswap64 (__bsx);
}
static __inline __uint16_t
__uint16_identity (__uint16_t __x)
{
  return __x;
}
static __inline __uint32_t
__uint32_identity (__uint32_t __x)
{
  return __x;
}
static __inline __uint64_t
__uint64_identity (__uint64_t __x)
{
  return __x;
}
typedef struct
{
  unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
} __sigset_t;
typedef __sigset_t sigset_t;
struct timeval
{
  __time_t tv_sec;
  __suseconds_t tv_usec;
};
struct timespec
{
  __time_t tv_sec;
  __syscall_slong_t tv_nsec;
};
typedef long int __fd_mask;
typedef struct
  {
    __fd_mask fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];
  } fd_set;
typedef __fd_mask fd_mask;
extern "C" {
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);
}
typedef __blksize_t blksize_t;
typedef __blkcnt_t blkcnt_t;
typedef __fsblkcnt_t fsblkcnt_t;
typedef __fsfilcnt_t fsfilcnt_t;
typedef __blkcnt64_t blkcnt64_t;
typedef __fsblkcnt64_t fsblkcnt64_t;
typedef __fsfilcnt64_t fsfilcnt64_t;
typedef union
{
  __extension__ unsigned long long int __value64;
  struct
  {
    unsigned int __low;
    unsigned int __high;
  } __value32;
} __atomic_wide_counter;
typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;
typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;
struct __pthread_mutex_s
{
  int __lock;
  unsigned int __count;
  int __owner;
  unsigned int __nusers;
  int __kind;
  short __spins;
  short __glibc_reserved;
  __pthread_list_t __list;
};
struct __pthread_rwlock_arch_t
{
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;
  int __cur_writer;
  int __shared;
  unsigned long int __pad1;
  unsigned long int __pad2;
  unsigned int __flags;
};
struct __pthread_cond_s
{
  __atomic_wide_counter __wseq;
  __atomic_wide_counter __g1_start;
  unsigned int __g_size[2] ;
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
  unsigned int __unused_initialized_1;
  unsigned int __unused_initialized_2;
};
typedef unsigned int __tss_t;
typedef unsigned long int __thrd_t;
typedef struct
{
  int __data ;
} __once_flag;
typedef unsigned long int pthread_t;
typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;
typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;
typedef unsigned int pthread_key_t;
typedef int pthread_once_t;
union pthread_attr_t
{
  char __size[56];
  long int __align;
};
typedef union pthread_attr_t pthread_attr_t;
typedef union
{
  struct __pthread_mutex_s __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;
typedef union
{
  struct __pthread_cond_s __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;
typedef union
{
  struct __pthread_rwlock_arch_t __data;
  char __size[56];
  long int __align;
} pthread_rwlock_t;
typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;
typedef volatile int pthread_spinlock_t;
typedef union
{
  char __size[32];
  long int __align;
} pthread_barrier_t;
typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
}
extern long int random (void) noexcept (true);
extern void srandom (unsigned int __seed) noexcept (true);
extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) noexcept (true) __attribute__ ((__nonnull__ (2)));
extern char *setstate (char *__statebuf) noexcept (true) __attribute__ ((__nonnull__ (1)));
struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };
extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (2)));
extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     noexcept (true) __attribute__ ((__nonnull__ (2, 4)));
extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int rand (void) noexcept (true);
extern void srand (unsigned int __seed) noexcept (true);
extern int rand_r (unsigned int *__seed) noexcept (true);
extern double drand48 (void) noexcept (true);
extern double erand48 (unsigned short int __xsubi[3]) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern long int lrand48 (void) noexcept (true);
extern long int nrand48 (unsigned short int __xsubi[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern long int mrand48 (void) noexcept (true);
extern long int jrand48 (unsigned short int __xsubi[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void srand48 (long int __seedval) noexcept (true);
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) noexcept (true) __attribute__ ((__nonnull__ (1)));
struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    __extension__ unsigned long long int __a;
  };
extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     noexcept (true) __attribute__ ((__nonnull__ (2)));
extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern __uint32_t arc4random (void)
     noexcept (true) ;
extern void arc4random_buf (void *__buf, size_t __size)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern __uint32_t arc4random_uniform (__uint32_t __upper_bound)
     noexcept (true) ;
extern void *malloc (size_t __size) noexcept (true) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) ;
extern void *calloc (size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__alloc_size__ (1, 2))) ;
extern void *realloc (void *__ptr, size_t __size)
     noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__alloc_size__ (2)));
extern void free (void *__ptr) noexcept (true);
extern void free_sized (void *__ptr, size_t __size) noexcept (true);
extern void free_aligned_sized (void *__ptr, size_t __alignment, size_t __size)
     noexcept (true);
extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__warn_unused_result__))
     __attribute__ ((__alloc_size__ (2, 3)))
    __attribute__ ((__malloc__ (__builtin_free, 1)));
extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__malloc__ (reallocarray, 1)));
extern "C" {
extern void *alloca (size_t __size) noexcept (true);
}
extern void *valloc (size_t __size) noexcept (true) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) ;
extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     noexcept (true) __attribute__ ((__nonnull__ (1))) ;
extern void *aligned_alloc (size_t __alignment, size_t __size)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__alloc_align__ (1)))
     __attribute__ ((__alloc_size__ (2))) ;
extern void abort (void) noexcept (true) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));
extern int atexit (void (*__func) (void)) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern "C++" int at_quick_exit (void (*__func) (void))
     noexcept (true) __asm ("at_quick_exit") __attribute__ ((__nonnull__ (1)));
extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void exit (int __status) noexcept (true) __attribute__ ((__noreturn__));
extern void quick_exit (int __status) noexcept (true) __attribute__ ((__noreturn__));
extern void _Exit (int __status) noexcept (true) __attribute__ ((__noreturn__));
extern char *getenv (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1))) ;
extern char *secure_getenv (const char *__name)
     noexcept (true) __attribute__ ((__nonnull__ (1))) ;
extern int putenv (char *__string) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int setenv (const char *__name, const char *__value, int __replace)
     noexcept (true) __attribute__ ((__nonnull__ (2)));
extern int unsetenv (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int clearenv (void) noexcept (true);
extern char *mktemp (char *__template) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) ;
extern int mkstemp64 (char *__template) __attribute__ ((__nonnull__ (1))) ;
extern int mkstemps (char *__template, int __suffixlen) __attribute__ ((__nonnull__ (1))) ;
extern int mkstemps64 (char *__template, int __suffixlen)
     __attribute__ ((__nonnull__ (1))) ;
extern char *mkdtemp (char *__template) noexcept (true) __attribute__ ((__nonnull__ (1))) ;
extern int mkostemp (char *__template, int __flags) __attribute__ ((__nonnull__ (1))) ;
extern int mkostemp64 (char *__template, int __flags) __attribute__ ((__nonnull__ (1))) ;
extern int mkostemps (char *__template, int __suffixlen, int __flags)
     __attribute__ ((__nonnull__ (1))) ;
extern int mkostemps64 (char *__template, int __suffixlen, int __flags)
     __attribute__ ((__nonnull__ (1))) ;
extern int system (const char *__command) ;
extern char *canonicalize_file_name (const char *__name)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__malloc__))
     __attribute__ ((__malloc__ (__builtin_free, 1))) ;
extern char *realpath (const char *__restrict __name,
         char *__restrict __resolved) noexcept (true) ;
typedef int (*__compar_fn_t) (const void *, const void *);
typedef __compar_fn_t comparison_fn_t;
typedef int (*__compar_d_fn_t) (const void *, const void *, void *);
extern void *bsearch (const void *__key, const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;
extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));
extern void qsort_r (void *__base, size_t __nmemb, size_t __size,
       __compar_d_fn_t __compar, void *__arg)
  __attribute__ ((__nonnull__ (1, 4)));
extern int abs (int __x) noexcept (true) __attribute__ ((__const__)) ;
extern long int labs (long int __x) noexcept (true) __attribute__ ((__const__)) ;
__extension__ extern long long int llabs (long long int __x)
     noexcept (true) __attribute__ ((__const__)) ;
extern unsigned int uabs (int __x) noexcept (true) __attribute__ ((__const__)) ;
extern unsigned long int ulabs (long int __x) noexcept (true) __attribute__ ((__const__)) ;
__extension__ extern unsigned long long int ullabs (long long int __x)
     noexcept (true) __attribute__ ((__const__)) ;
extern div_t div (int __numer, int __denom)
     noexcept (true) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     noexcept (true) __attribute__ ((__const__)) ;
__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     noexcept (true) __attribute__ ((__const__)) ;
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) noexcept (true) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) noexcept (true) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *gcvt (double __value, int __ndigit, char *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (3))) ;
extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (3))) ;
extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int mblen (const char *__s, size_t __n) noexcept (true);
extern int mbtowc (wchar_t *__restrict __pwc,
     const char *__restrict __s, size_t __n) noexcept (true);
extern int wctomb (char *__s, wchar_t __wchar) noexcept (true);
extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   const char *__restrict __s, size_t __n) noexcept (true)
    __attribute__ ((__access__ (__read_only__, 2)));
extern size_t wcstombs (char *__restrict __s,
   const wchar_t *__restrict __pwcs, size_t __n)
     noexcept (true)
  __attribute__ ((__access__ (__write_only__, 1, 3)))
  __attribute__ ((__access__ (__read_only__, 2)));
extern int rpmatch (const char *__response) noexcept (true) __attribute__ ((__nonnull__ (1))) ;
extern int getsubopt (char **__restrict __optionp,
        char *const *__restrict __tokens,
        char **__restrict __valuep)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2, 3))) ;
extern int posix_openpt (int __oflag) ;
extern int grantpt (int __fd) noexcept (true);
extern int unlockpt (int __fd) noexcept (true);
extern char *ptsname (int __fd) noexcept (true) ;
extern int ptsname_r (int __fd, char *__buf, size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 2, 3)));
extern int getpt (void);
extern int getloadavg (double __loadavg[], int __nelem)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern size_t memalignment (const void *__p);
}
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{

  using ::abs;
  inline long
  abs(long __i) { return __builtin_labs(__i); }
  inline long long
  abs(long long __x) { return __builtin_llabs (__x); }
  inline constexpr double
  abs(double __x)
  { return __builtin_fabs(__x); }
  inline constexpr float
  abs(float __x)
  { return __builtin_fabsf(__x); }
  inline constexpr long double
  abs(long double __x)
  { return __builtin_fabsl(__x); }
  __extension__ inline constexpr __int128
  abs(__int128 __x) { return __x >= 0 ? __x : -__x; }
  __extension__ inline constexpr
  __float128
  abs(__float128 __x)
  {
    return __builtin_fabsf128(__x);
  }

}
}
#pragma GCC diagnostic pop
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{

  using ::acos;
  inline constexpr float
  acos(float __x)
  { return __builtin_acosf(__x); }
  inline constexpr long double
  acos(long double __x)
  { return __builtin_acosl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    acos(_Tp __x)
    { return __builtin_acos(__x); }
  using ::asin;
  inline constexpr float
  asin(float __x)
  { return __builtin_asinf(__x); }
  inline constexpr long double
  asin(long double __x)
  { return __builtin_asinl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    asin(_Tp __x)
    { return __builtin_asin(__x); }
  using ::atan;
  inline constexpr float
  atan(float __x)
  { return __builtin_atanf(__x); }
  inline constexpr long double
  atan(long double __x)
  { return __builtin_atanl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    atan(_Tp __x)
    { return __builtin_atan(__x); }
  using ::atan2;
  inline constexpr float
  atan2(float __y, float __x)
  { return __builtin_atan2f(__y, __x); }
  inline constexpr long double
  atan2(long double __y, long double __x)
  { return __builtin_atan2l(__y, __x); }
  using ::ceil;
  inline constexpr float
  ceil(float __x)
  { return __builtin_ceilf(__x); }
  inline constexpr long double
  ceil(long double __x)
  { return __builtin_ceill(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    ceil(_Tp __x)
    { return __builtin_ceil(__x); }
  using ::cos;
  inline constexpr float
  cos(float __x)
  { return __builtin_cosf(__x); }
  inline constexpr long double
  cos(long double __x)
  { return __builtin_cosl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    cos(_Tp __x)
    { return __builtin_cos(__x); }
  using ::cosh;
  inline constexpr float
  cosh(float __x)
  { return __builtin_coshf(__x); }
  inline constexpr long double
  cosh(long double __x)
  { return __builtin_coshl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    cosh(_Tp __x)
    { return __builtin_cosh(__x); }
  using ::exp;
  inline constexpr float
  exp(float __x)
  { return __builtin_expf(__x); }
  inline constexpr long double
  exp(long double __x)
  { return __builtin_expl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    exp(_Tp __x)
    { return __builtin_exp(__x); }
  using ::fabs;
  inline constexpr float
  fabs(float __x)
  { return __builtin_fabsf(__x); }
  inline constexpr long double
  fabs(long double __x)
  { return __builtin_fabsl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    fabs(_Tp __x)
    { return __builtin_fabs(__x); }
  using ::floor;
  inline constexpr float
  floor(float __x)
  { return __builtin_floorf(__x); }
  inline constexpr long double
  floor(long double __x)
  { return __builtin_floorl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    floor(_Tp __x)
    { return __builtin_floor(__x); }
  using ::fmod;
  inline constexpr float
  fmod(float __x, float __y)
  { return __builtin_fmodf(__x, __y); }
  inline constexpr long double
  fmod(long double __x, long double __y)
  { return __builtin_fmodl(__x, __y); }
  using ::frexp;
  inline float
  frexp(float __x, int* __exp)
  { return __builtin_frexpf(__x, __exp); }
  inline long double
  frexp(long double __x, int* __exp)
  { return __builtin_frexpl(__x, __exp); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    frexp(_Tp __x, int* __exp)
    { return __builtin_frexp(__x, __exp); }
  using ::ldexp;
  inline constexpr float
  ldexp(float __x, int __exp)
  { return __builtin_ldexpf(__x, __exp); }
  inline constexpr long double
  ldexp(long double __x, int __exp)
  { return __builtin_ldexpl(__x, __exp); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    ldexp(_Tp __x, int __exp)
    { return __builtin_ldexp(__x, __exp); }
  using ::log;
  inline constexpr float
  log(float __x)
  { return __builtin_logf(__x); }
  inline constexpr long double
  log(long double __x)
  { return __builtin_logl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    log(_Tp __x)
    { return __builtin_log(__x); }
  using ::log10;
  inline constexpr float
  log10(float __x)
  { return __builtin_log10f(__x); }
  inline constexpr long double
  log10(long double __x)
  { return __builtin_log10l(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    log10(_Tp __x)
    { return __builtin_log10(__x); }
  using ::modf;
  inline float
  modf(float __x, float* __iptr)
  { return __builtin_modff(__x, __iptr); }
  inline long double
  modf(long double __x, long double* __iptr)
  { return __builtin_modfl(__x, __iptr); }
  using ::pow;
  inline constexpr float
  pow(float __x, float __y)
  { return __builtin_powf(__x, __y); }
  inline constexpr long double
  pow(long double __x, long double __y)
  { return __builtin_powl(__x, __y); }
  using ::sin;
  inline constexpr float
  sin(float __x)
  { return __builtin_sinf(__x); }
  inline constexpr long double
  sin(long double __x)
  { return __builtin_sinl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    sin(_Tp __x)
    { return __builtin_sin(__x); }
  using ::sinh;
  inline constexpr float
  sinh(float __x)
  { return __builtin_sinhf(__x); }
  inline constexpr long double
  sinh(long double __x)
  { return __builtin_sinhl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    sinh(_Tp __x)
    { return __builtin_sinh(__x); }
  using ::sqrt;
  inline constexpr float
  sqrt(float __x)
  { return __builtin_sqrtf(__x); }
  inline constexpr long double
  sqrt(long double __x)
  { return __builtin_sqrtl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    sqrt(_Tp __x)
    { return __builtin_sqrt(__x); }
  using ::tan;
  inline constexpr float
  tan(float __x)
  { return __builtin_tanf(__x); }
  inline constexpr long double
  tan(long double __x)
  { return __builtin_tanl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    tan(_Tp __x)
    { return __builtin_tan(__x); }
  using ::tanh;
  inline constexpr float
  tanh(float __x)
  { return __builtin_tanhf(__x); }
  inline constexpr long double
  tanh(long double __x)
  { return __builtin_tanhl(__x); }
  template<typename _Tp>
    inline constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    double>::__type
    tanh(_Tp __x)
    { return __builtin_tanh(__x); }
  template<typename _Tp, typename _Up>
    inline constexpr
    typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    atan2(_Tp __y, _Up __x)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return atan2(__type(__y), __type(__x));
    }
  template<typename _Tp, typename _Up>
    inline constexpr
    typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    fmod(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return fmod(__type(__x), __type(__y));
    }
  template<typename _Tp, typename _Up>
    inline constexpr
    typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    pow(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return pow(__type(__x), __type(__y));
    }
  constexpr int
  fpclassify(float __x)
  { return __builtin_fpclassify(0, 1, 4,
    3, 2, __x); }
  constexpr int
  fpclassify(double __x)
  { return __builtin_fpclassify(0, 1, 4,
    3, 2, __x); }
  constexpr int
  fpclassify(long double __x)
  { return __builtin_fpclassify(0, 1, 4,
    3, 2, __x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              int>::__type
    fpclassify(_Tp __x)
    { return __x != 0 ? 4 : 2; }
  constexpr bool
  isfinite(float __x)
  { return __builtin_isfinite(__x); }
  constexpr bool
  isfinite(double __x)
  { return __builtin_isfinite(__x); }
  constexpr bool
  isfinite(long double __x)
  { return __builtin_isfinite(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              bool>::__type
    isfinite(_Tp)
    { return true; }
  constexpr bool
  isinf(float __x)
  { return __builtin_isinf(__x); }
  constexpr bool
  isinf(double __x)
  { return __builtin_isinf(__x); }
  constexpr bool
  isinf(long double __x)
  { return __builtin_isinf(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              bool>::__type
    isinf(_Tp)
    { return false; }
  constexpr bool
  isnan(float __x)
  { return __builtin_isnan(__x); }
  constexpr bool
  isnan(double __x)
  { return __builtin_isnan(__x); }
  constexpr bool
  isnan(long double __x)
  { return __builtin_isnan(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              bool>::__type
    isnan(_Tp)
    { return false; }
  constexpr bool
  isnormal(float __x)
  { return __builtin_isnormal(__x); }
  constexpr bool
  isnormal(double __x)
  { return __builtin_isnormal(__x); }
  constexpr bool
  isnormal(long double __x)
  { return __builtin_isnormal(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              bool>::__type
    isnormal(_Tp __x)
    { return __x != 0 ? true : false; }
  constexpr bool
  signbit(float __x)
  { return __builtin_signbit(__x); }
  constexpr bool
  signbit(double __x)
  { return __builtin_signbit(__x); }
  constexpr bool
  signbit(long double __x)
  { return __builtin_signbit(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              bool>::__type
    signbit(_Tp __x)
    { return __x < 0 ? true : false; }
  constexpr bool
  isgreater(float __x, float __y)
  { return __builtin_isgreater(__x, __y); }
  constexpr bool
  isgreater(double __x, double __y)
  { return __builtin_isgreater(__x, __y); }
  constexpr bool
  isgreater(long double __x, long double __y)
  { return __builtin_isgreater(__x, __y); }
  template<typename _Tp, typename _Up>
    constexpr typename
    __gnu_cxx::__enable_if<(__is_arithmetic<_Tp>::__value
       && __is_arithmetic<_Up>::__value), bool>::__type
    isgreater(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return __builtin_isgreater(__type(__x), __type(__y));
    }
  constexpr bool
  isgreaterequal(float __x, float __y)
  { return __builtin_isgreaterequal(__x, __y); }
  constexpr bool
  isgreaterequal(double __x, double __y)
  { return __builtin_isgreaterequal(__x, __y); }
  constexpr bool
  isgreaterequal(long double __x, long double __y)
  { return __builtin_isgreaterequal(__x, __y); }
  template<typename _Tp, typename _Up>
    constexpr typename
    __gnu_cxx::__enable_if<(__is_arithmetic<_Tp>::__value
       && __is_arithmetic<_Up>::__value), bool>::__type
    isgreaterequal(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return __builtin_isgreaterequal(__type(__x), __type(__y));
    }
  constexpr bool
  isless(float __x, float __y)
  { return __builtin_isless(__x, __y); }
  constexpr bool
  isless(double __x, double __y)
  { return __builtin_isless(__x, __y); }
  constexpr bool
  isless(long double __x, long double __y)
  { return __builtin_isless(__x, __y); }
  template<typename _Tp, typename _Up>
    constexpr typename
    __gnu_cxx::__enable_if<(__is_arithmetic<_Tp>::__value
       && __is_arithmetic<_Up>::__value), bool>::__type
    isless(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return __builtin_isless(__type(__x), __type(__y));
    }
  constexpr bool
  islessequal(float __x, float __y)
  { return __builtin_islessequal(__x, __y); }
  constexpr bool
  islessequal(double __x, double __y)
  { return __builtin_islessequal(__x, __y); }
  constexpr bool
  islessequal(long double __x, long double __y)
  { return __builtin_islessequal(__x, __y); }
  template<typename _Tp, typename _Up>
    constexpr typename
    __gnu_cxx::__enable_if<(__is_arithmetic<_Tp>::__value
       && __is_arithmetic<_Up>::__value), bool>::__type
    islessequal(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return __builtin_islessequal(__type(__x), __type(__y));
    }
  constexpr bool
  islessgreater(float __x, float __y)
  { return __builtin_islessgreater(__x, __y); }
  constexpr bool
  islessgreater(double __x, double __y)
  { return __builtin_islessgreater(__x, __y); }
  constexpr bool
  islessgreater(long double __x, long double __y)
  { return __builtin_islessgreater(__x, __y); }
  template<typename _Tp, typename _Up>
    constexpr typename
    __gnu_cxx::__enable_if<(__is_arithmetic<_Tp>::__value
       && __is_arithmetic<_Up>::__value), bool>::__type
    islessgreater(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return __builtin_islessgreater(__type(__x), __type(__y));
    }
  constexpr bool
  isunordered(float __x, float __y)
  { return __builtin_isunordered(__x, __y); }
  constexpr bool
  isunordered(double __x, double __y)
  { return __builtin_isunordered(__x, __y); }
  constexpr bool
  isunordered(long double __x, long double __y)
  { return __builtin_isunordered(__x, __y); }
  template<typename _Tp, typename _Up>
    constexpr typename
    __gnu_cxx::__enable_if<(__is_arithmetic<_Tp>::__value
       && __is_arithmetic<_Up>::__value), bool>::__type
    isunordered(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return __builtin_isunordered(__type(__x), __type(__y));
    }
  using ::acosf;
  using ::acosl;
  using ::asinf;
  using ::asinl;
  using ::atanf;
  using ::atanl;
  using ::atan2f;
  using ::atan2l;
  using ::ceilf;
  using ::ceill;
  using ::cosf;
  using ::cosl;
  using ::coshf;
  using ::coshl;
  using ::expf;
  using ::expl;
  using ::fabsf;
  using ::fabsl;
  using ::floorf;
  using ::floorl;
  using ::fmodf;
  using ::fmodl;
  using ::frexpf;
  using ::frexpl;
  using ::ldexpf;
  using ::ldexpl;
  using ::logf;
  using ::logl;
  using ::log10f;
  using ::log10l;
  using ::modff;
  using ::modfl;
  using ::powf;
  using ::powl;
  using ::sinf;
  using ::sinl;
  using ::sinhf;
  using ::sinhl;
  using ::sqrtf;
  using ::sqrtl;
  using ::tanf;
  using ::tanl;
  using ::tanhf;
  using ::tanhl;
  using ::double_t;
  using ::float_t;
  using ::acosh;
  using ::acoshf;
  using ::acoshl;
  using ::asinh;
  using ::asinhf;
  using ::asinhl;
  using ::atanh;
  using ::atanhf;
  using ::atanhl;
  using ::cbrt;
  using ::cbrtf;
  using ::cbrtl;
  using ::copysign;
  using ::copysignf;
  using ::copysignl;
  using ::erf;
  using ::erff;
  using ::erfl;
  using ::erfc;
  using ::erfcf;
  using ::erfcl;
  using ::exp2;
  using ::exp2f;
  using ::exp2l;
  using ::expm1;
  using ::expm1f;
  using ::expm1l;
  using ::fdim;
  using ::fdimf;
  using ::fdiml;
  using ::fma;
  using ::fmaf;
  using ::fmal;
  using ::fmax;
  using ::fmaxf;
  using ::fmaxl;
  using ::fmin;
  using ::fminf;
  using ::fminl;
  using ::hypot;
  using ::hypotf;
  using ::hypotl;
  using ::ilogb;
  using ::ilogbf;
  using ::ilogbl;
  using ::lgamma;
  using ::lgammaf;
  using ::lgammal;
  using ::llrint;
  using ::llrintf;
  using ::llrintl;
  using ::llround;
  using ::llroundf;
  using ::llroundl;
  using ::log1p;
  using ::log1pf;
  using ::log1pl;
  using ::log2;
  using ::log2f;
  using ::log2l;
  using ::logb;
  using ::logbf;
  using ::logbl;
  using ::lrint;
  using ::lrintf;
  using ::lrintl;
  using ::lround;
  using ::lroundf;
  using ::lroundl;
  using ::nan;
  using ::nanf;
  using ::nanl;
  using ::nearbyint;
  using ::nearbyintf;
  using ::nearbyintl;
  using ::nextafter;
  using ::nextafterf;
  using ::nextafterl;
  using ::nexttoward;
  using ::nexttowardf;
  using ::nexttowardl;
  using ::remainder;
  using ::remainderf;
  using ::remainderl;
  using ::remquo;
  using ::remquof;
  using ::remquol;
  using ::rint;
  using ::rintf;
  using ::rintl;
  using ::round;
  using ::roundf;
  using ::roundl;
  using ::scalbln;
  using ::scalblnf;
  using ::scalblnl;
  using ::scalbn;
  using ::scalbnf;
  using ::scalbnl;
  using ::tgamma;
  using ::tgammaf;
  using ::tgammal;
  using ::trunc;
  using ::truncf;
  using ::truncl;
  constexpr float
  acosh(float __x)
  { return __builtin_acoshf(__x); }
  constexpr long double
  acosh(long double __x)
  { return __builtin_acoshl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    acosh(_Tp __x)
    { return __builtin_acosh(__x); }
  constexpr float
  asinh(float __x)
  { return __builtin_asinhf(__x); }
  constexpr long double
  asinh(long double __x)
  { return __builtin_asinhl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    asinh(_Tp __x)
    { return __builtin_asinh(__x); }
  constexpr float
  atanh(float __x)
  { return __builtin_atanhf(__x); }
  constexpr long double
  atanh(long double __x)
  { return __builtin_atanhl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    atanh(_Tp __x)
    { return __builtin_atanh(__x); }
  constexpr float
  cbrt(float __x)
  { return __builtin_cbrtf(__x); }
  constexpr long double
  cbrt(long double __x)
  { return __builtin_cbrtl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    cbrt(_Tp __x)
    { return __builtin_cbrt(__x); }
  constexpr float
  copysign(float __x, float __y)
  { return __builtin_copysignf(__x, __y); }
  constexpr long double
  copysign(long double __x, long double __y)
  { return __builtin_copysignl(__x, __y); }
  constexpr float
  erf(float __x)
  { return __builtin_erff(__x); }
  constexpr long double
  erf(long double __x)
  { return __builtin_erfl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    erf(_Tp __x)
    { return __builtin_erf(__x); }
  constexpr float
  erfc(float __x)
  { return __builtin_erfcf(__x); }
  constexpr long double
  erfc(long double __x)
  { return __builtin_erfcl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    erfc(_Tp __x)
    { return __builtin_erfc(__x); }
  constexpr float
  exp2(float __x)
  { return __builtin_exp2f(__x); }
  constexpr long double
  exp2(long double __x)
  { return __builtin_exp2l(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    exp2(_Tp __x)
    { return __builtin_exp2(__x); }
  constexpr float
  expm1(float __x)
  { return __builtin_expm1f(__x); }
  constexpr long double
  expm1(long double __x)
  { return __builtin_expm1l(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    expm1(_Tp __x)
    { return __builtin_expm1(__x); }
  constexpr float
  fdim(float __x, float __y)
  { return __builtin_fdimf(__x, __y); }
  constexpr long double
  fdim(long double __x, long double __y)
  { return __builtin_fdiml(__x, __y); }
  constexpr float
  fma(float __x, float __y, float __z)
  { return __builtin_fmaf(__x, __y, __z); }
  constexpr long double
  fma(long double __x, long double __y, long double __z)
  { return __builtin_fmal(__x, __y, __z); }
  constexpr float
  fmax(float __x, float __y)
  { return __builtin_fmaxf(__x, __y); }
  constexpr long double
  fmax(long double __x, long double __y)
  { return __builtin_fmaxl(__x, __y); }
  constexpr float
  fmin(float __x, float __y)
  { return __builtin_fminf(__x, __y); }
  constexpr long double
  fmin(long double __x, long double __y)
  { return __builtin_fminl(__x, __y); }
  constexpr float
  hypot(float __x, float __y)
  { return __builtin_hypotf(__x, __y); }
  constexpr long double
  hypot(long double __x, long double __y)
  { return __builtin_hypotl(__x, __y); }
  constexpr int
  ilogb(float __x)
  { return __builtin_ilogbf(__x); }
  constexpr int
  ilogb(long double __x)
  { return __builtin_ilogbl(__x); }
  template<typename _Tp>
    constexpr
    typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                    int>::__type
    ilogb(_Tp __x)
    { return __builtin_ilogb(__x); }
  constexpr float
  lgamma(float __x)
  { return __builtin_lgammaf(__x); }
  constexpr long double
  lgamma(long double __x)
  { return __builtin_lgammal(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    lgamma(_Tp __x)
    { return __builtin_lgamma(__x); }
  constexpr long long
  llrint(float __x)
  { return __builtin_llrintf(__x); }
  constexpr long long
  llrint(long double __x)
  { return __builtin_llrintl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              long long>::__type
    llrint(_Tp __x)
    { return __builtin_llrint(__x); }
  constexpr long long
  llround(float __x)
  { return __builtin_llroundf(__x); }
  constexpr long long
  llround(long double __x)
  { return __builtin_llroundl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              long long>::__type
    llround(_Tp __x)
    { return __builtin_llround(__x); }
  constexpr float
  log1p(float __x)
  { return __builtin_log1pf(__x); }
  constexpr long double
  log1p(long double __x)
  { return __builtin_log1pl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    log1p(_Tp __x)
    { return __builtin_log1p(__x); }
  constexpr float
  log2(float __x)
  { return __builtin_log2f(__x); }
  constexpr long double
  log2(long double __x)
  { return __builtin_log2l(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    log2(_Tp __x)
    { return __builtin_log2(__x); }
  constexpr float
  logb(float __x)
  { return __builtin_logbf(__x); }
  constexpr long double
  logb(long double __x)
  { return __builtin_logbl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    logb(_Tp __x)
    { return __builtin_logb(__x); }
  constexpr long
  lrint(float __x)
  { return __builtin_lrintf(__x); }
  constexpr long
  lrint(long double __x)
  { return __builtin_lrintl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              long>::__type
    lrint(_Tp __x)
    { return __builtin_lrint(__x); }
  constexpr long
  lround(float __x)
  { return __builtin_lroundf(__x); }
  constexpr long
  lround(long double __x)
  { return __builtin_lroundl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              long>::__type
    lround(_Tp __x)
    { return __builtin_lround(__x); }
  constexpr float
  nearbyint(float __x)
  { return __builtin_nearbyintf(__x); }
  constexpr long double
  nearbyint(long double __x)
  { return __builtin_nearbyintl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    nearbyint(_Tp __x)
    { return __builtin_nearbyint(__x); }
  constexpr float
  nextafter(float __x, float __y)
  { return __builtin_nextafterf(__x, __y); }
  constexpr long double
  nextafter(long double __x, long double __y)
  { return __builtin_nextafterl(__x, __y); }
  constexpr float
  nexttoward(float __x, long double __y)
  { return __builtin_nexttowardf(__x, __y); }
  constexpr long double
  nexttoward(long double __x, long double __y)
  { return __builtin_nexttowardl(__x, __y); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    nexttoward(_Tp __x, long double __y)
    { return __builtin_nexttoward(__x, __y); }
  constexpr float
  remainder(float __x, float __y)
  { return __builtin_remainderf(__x, __y); }
  constexpr long double
  remainder(long double __x, long double __y)
  { return __builtin_remainderl(__x, __y); }
  inline float
  remquo(float __x, float __y, int* __pquo)
  { return __builtin_remquof(__x, __y, __pquo); }
  inline long double
  remquo(long double __x, long double __y, int* __pquo)
  { return __builtin_remquol(__x, __y, __pquo); }
  constexpr float
  rint(float __x)
  { return __builtin_rintf(__x); }
  constexpr long double
  rint(long double __x)
  { return __builtin_rintl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    rint(_Tp __x)
    { return __builtin_rint(__x); }
  constexpr float
  round(float __x)
  { return __builtin_roundf(__x); }
  constexpr long double
  round(long double __x)
  { return __builtin_roundl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    round(_Tp __x)
    { return __builtin_round(__x); }
  constexpr float
  scalbln(float __x, long __ex)
  { return __builtin_scalblnf(__x, __ex); }
  constexpr long double
  scalbln(long double __x, long __ex)
  { return __builtin_scalblnl(__x, __ex); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    scalbln(_Tp __x, long __ex)
    { return __builtin_scalbln(__x, __ex); }
  constexpr float
  scalbn(float __x, int __ex)
  { return __builtin_scalbnf(__x, __ex); }
  constexpr long double
  scalbn(long double __x, int __ex)
  { return __builtin_scalbnl(__x, __ex); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    scalbn(_Tp __x, int __ex)
    { return __builtin_scalbn(__x, __ex); }
  constexpr float
  tgamma(float __x)
  { return __builtin_tgammaf(__x); }
  constexpr long double
  tgamma(long double __x)
  { return __builtin_tgammal(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    tgamma(_Tp __x)
    { return __builtin_tgamma(__x); }
  constexpr float
  trunc(float __x)
  { return __builtin_truncf(__x); }
  constexpr long double
  trunc(long double __x)
  { return __builtin_truncl(__x); }
  template<typename _Tp>
    constexpr typename __gnu_cxx::__enable_if<__is_integer<_Tp>::__value,
                                              double>::__type
    trunc(_Tp __x)
    { return __builtin_trunc(__x); }
  template<typename _Tp, typename _Up>
    constexpr typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    copysign(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return copysign(__type(__x), __type(__y));
    }
  template<typename _Tp, typename _Up>
    constexpr typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    fdim(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return fdim(__type(__x), __type(__y));
    }
  template<typename _Tp, typename _Up>
    constexpr typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    fmax(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return fmax(__type(__x), __type(__y));
    }
  template<typename _Tp, typename _Up>
    constexpr typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    fmin(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return fmin(__type(__x), __type(__y));
    }
  template<typename _Tp, typename _Up>
    constexpr typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    hypot(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return hypot(__type(__x), __type(__y));
    }
  template<typename _Tp, typename _Up>
    constexpr typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    nextafter(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return nextafter(__type(__x), __type(__y));
    }
  template<typename _Tp, typename _Up>
    constexpr typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    remainder(_Tp __x, _Up __y)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return remainder(__type(__x), __type(__y));
    }
  template<typename _Tp, typename _Up>
    inline typename __gnu_cxx::__promote_2<_Tp, _Up>::__type
    remquo(_Tp __x, _Up __y, int* __pquo)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Up>::__type __type;
      return remquo(__type(__x), __type(__y), __pquo);
    }
  template<typename _Tp, typename _Up, typename _Vp>
    constexpr typename __gnu_cxx::__promote_3<_Tp, _Up, _Vp>::__type
    fma(_Tp __x, _Up __y, _Vp __z)
    {
      typedef typename __gnu_cxx::__promote_3<_Tp, _Up, _Vp>::__type __type;
      return fma(__type(__x), __type(__y), __type(__z));
    }
  template<typename _Tp>
    inline _Tp
    __hypot3(_Tp __x, _Tp __y, _Tp __z)
    {
      __x = std::abs(__x);
      __y = std::abs(__y);
      __z = std::abs(__z);
      if (_Tp __a = __x < __y ? __y < __z ? __z : __y : __x < __z ? __z : __x)
 return __a * std::sqrt((__x / __a) * (__x / __a)
          + (__y / __a) * (__y / __a)
          + (__z / __a) * (__z / __a));
      else
 return {};
    }
  inline float
  hypot(float __x, float __y, float __z)
  { return std::__hypot3<float>(__x, __y, __z); }
  inline double
  hypot(double __x, double __y, double __z)
  { return std::__hypot3<double>(__x, __y, __z); }
  inline long double
  hypot(long double __x, long double __y, long double __z)
  { return std::__hypot3<long double>(__x, __y, __z); }
  template<typename _Tp, typename _Up, typename _Vp>
    typename __gnu_cxx::__promote_3<_Tp, _Up, _Vp>::__type
    hypot(_Tp __x, _Up __y, _Vp __z)
    {
      using __type = typename __gnu_cxx::__promote_3<_Tp, _Up, _Vp>::__type;
      return std::__hypot3<__type>(__x, __y, __z);
    }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  void
  __throw_logic_error(const char*) __attribute__((__noreturn__,__cold__));
  void
  __throw_domain_error(const char*) __attribute__((__noreturn__,__cold__));
  void
  __throw_invalid_argument(const char*) __attribute__((__noreturn__,__cold__));
  void
  __throw_length_error(const char*) __attribute__((__noreturn__,__cold__));
  void
  __throw_out_of_range(const char*) __attribute__((__noreturn__,__cold__));
  void
  __throw_out_of_range_fmt(const char*, ...)
    __attribute__((__noreturn__,__cold__,__format__(__gnu_printf__, 1, 2)));
  void
  __throw_runtime_error(const char*) __attribute__((__noreturn__,__cold__));
  void
  __throw_overflow_error(const char*) __attribute__((__noreturn__,__cold__));
  void
  __throw_underflow_error(const char*) __attribute__((__noreturn__,__cold__));

}
namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{

  template<typename _Tp>
    struct __is_integer_nonstrict
    : public std::__is_integer<_Tp>
    {
      using std::__is_integer<_Tp>::__value;
      enum { __width = __value ? sizeof(_Tp) * 8 : 0 };
    };
  template<typename _Value>
    struct __numeric_traits_integer
    {
      static_assert(__is_integer_nonstrict<_Value>::__value,
      "invalid specialization");
      static const bool __is_signed = (_Value)(-1) < 0;
      static const int __digits
 = __is_integer_nonstrict<_Value>::__width - __is_signed;
      static const _Value __max = __is_signed
 ? (((((_Value)1 << (__digits - 1)) - 1) << 1) + 1)
 : ~(_Value)0;
      static const _Value __min = __is_signed ? -__max - 1 : (_Value)0;
    };
  template<typename _Value>
    const _Value __numeric_traits_integer<_Value>::__min;
  template<typename _Value>
    const _Value __numeric_traits_integer<_Value>::__max;
  template<typename _Value>
    const bool __numeric_traits_integer<_Value>::__is_signed;
  template<typename _Value>
    const int __numeric_traits_integer<_Value>::__digits;
  template<typename _Tp>
    using __int_traits = __numeric_traits_integer<_Tp>;
  template<typename _Value>
    struct __numeric_traits_floating
    {
      static const int __max_digits10 = (2 + (std::__are_same<_Value, float>::__value ? 24 : std::__are_same<_Value, double>::__value ? 53 : 64) * 643L / 2136);
      static const bool __is_signed = true;
      static const int __digits10 = (std::__are_same<_Value, float>::__value ? 6 : std::__are_same<_Value, double>::__value ? 15 : 18);
      static const int __max_exponent10 = (std::__are_same<_Value, float>::__value ? 38 : std::__are_same<_Value, double>::__value ? 308 : 4932);
    };
  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__max_digits10;
  template<typename _Value>
    const bool __numeric_traits_floating<_Value>::__is_signed;
  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__digits10;
  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__max_exponent10;
  template<typename _Value>
    struct __numeric_traits
    : public __numeric_traits_integer<_Value>
    { };
  template<>
    struct __numeric_traits<float>
    : public __numeric_traits_floating<float>
    { };
  template<>
    struct __numeric_traits<double>
    : public __numeric_traits_floating<double>
    { };
  template<>
    struct __numeric_traits<long double>
    : public __numeric_traits_floating<long double>
    { };

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  template<typename _Tp>
    __attribute__((__always_inline__))
    inline constexpr _Tp*
    __addressof(_Tp& __r) noexcept
    { return __builtin_addressof(__r); }
  template<typename _Tp>
    [[__nodiscard__,__gnu__::__always_inline__]]
    constexpr _Tp&&
    forward(typename std::remove_reference<_Tp>::type& __t) noexcept
    { return static_cast<_Tp&&>(__t); }
  template<typename _Tp>
    [[__nodiscard__,__gnu__::__always_inline__]]
    constexpr _Tp&&
    forward(typename std::remove_reference<_Tp>::type&& __t) noexcept
    {
      static_assert(!std::is_lvalue_reference<_Tp>::value,
   "std::forward must not be used to convert an rvalue to an lvalue");
      return static_cast<_Tp&&>(__t);
    }
  template<typename _Tp, typename _Up>
  struct __like_impl;
  template<typename _Tp, typename _Up>
  struct __like_impl<_Tp&, _Up&>
  { using type = _Up&; };
  template<typename _Tp, typename _Up>
  struct __like_impl<const _Tp&, _Up&>
  { using type = const _Up&; };
  template<typename _Tp, typename _Up>
  struct __like_impl<_Tp&&, _Up&>
  { using type = _Up&&; };
  template<typename _Tp, typename _Up>
  struct __like_impl<const _Tp&&, _Up&>
  { using type = const _Up&&; };
  template<typename _Tp, typename _Up>
    using __like_t = typename __like_impl<_Tp&&, _Up&>::type;
  template<typename _Tp>
    [[__nodiscard__,__gnu__::__always_inline__]]
    constexpr typename std::remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept
    { return static_cast<typename std::remove_reference<_Tp>::type&&>(__t); }
  template<typename _Tp>
    struct __move_if_noexcept_cond
    : public __and_<__not_<is_nothrow_move_constructible<_Tp>>,
                    is_copy_constructible<_Tp>>::type { };
  template<typename _Tp>
    [[__nodiscard__,__gnu__::__always_inline__]]
    constexpr
    __conditional_t<__move_if_noexcept_cond<_Tp>::value, const _Tp&, _Tp&&>
    move_if_noexcept(_Tp& __x) noexcept
    { return std::move(__x); }
  template<typename _Tp>
    [[__nodiscard__,__gnu__::__always_inline__]]
    inline constexpr _Tp*
    addressof(_Tp& __r) noexcept
    { return __builtin_addressof(__r); }
  template<typename _Tp>
    const _Tp* addressof(const _Tp&&) = delete;
  template <typename _Tp, typename _Up = _Tp>
   
    inline _Tp
    __exchange(_Tp& __obj, _Up&& __new_val)
    {
      _Tp __old_val = std::move(__obj);
      __obj = std::forward<_Up>(__new_val);
      return __old_val;
    }
  template<typename _Tp>
    inline
    __enable_if_t<__and_<__not_<__is_tuple_like<_Tp>>,
    is_move_constructible<_Tp>,
    is_move_assignable<_Tp>>::value>
    swap(_Tp& __a, _Tp& __b)
    noexcept(__and_<is_nothrow_move_constructible<_Tp>, is_nothrow_move_assignable<_Tp>>::value)
    {
      _Tp __tmp = std::move(__a);
      __a = std::move(__b);
      __b = std::move(__tmp);
    }
  template<typename _Tp, size_t _Nm>
    inline
    __enable_if_t<__is_swappable<_Tp>::value>
    swap(_Tp (&__a)[_Nm], _Tp (&__b)[_Nm])
    noexcept(__is_nothrow_swappable<_Tp>::value)
    {
      for (size_t __n = 0; __n < _Nm; ++__n)
 swap(__a[__n], __b[__n]);
    }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  template<typename _Tp>
    struct tuple_size;
  template<typename _Tp,
    typename _Up = typename remove_cv<_Tp>::type,
    typename = typename enable_if<is_same<_Tp, _Up>::value>::type,
    size_t = tuple_size<_Tp>::value>
    using __enable_if_has_tuple_size = _Tp;
  template<typename _Tp>
    struct tuple_size<const __enable_if_has_tuple_size<_Tp>>
    : public tuple_size<_Tp> { };
  template<typename _Tp>
    struct tuple_size<volatile __enable_if_has_tuple_size<_Tp>>
    : public tuple_size<_Tp> { };
  template<typename _Tp>
    struct tuple_size<const volatile __enable_if_has_tuple_size<_Tp>>
    : public tuple_size<_Tp> { };
  template<typename _Tp>
    inline constexpr size_t tuple_size_v = tuple_size<_Tp>::value;
  template<size_t __i, typename _Tp>
    struct tuple_element;
  template<size_t __i, typename _Tp>
    using __tuple_element_t = typename tuple_element<__i, _Tp>::type;
  template<size_t __i, typename _Tp>
    struct tuple_element<__i, const _Tp>
    {
      using type = const __tuple_element_t<__i, _Tp>;
    };
  template<size_t __i, typename _Tp>
    struct tuple_element<__i, volatile _Tp>
    {
      using type = volatile __tuple_element_t<__i, _Tp>;
    };
  template<size_t __i, typename _Tp>
    struct tuple_element<__i, const volatile _Tp>
    {
      using type = const volatile __tuple_element_t<__i, _Tp>;
    };
  template<typename _Tp, typename... _Types>
    constexpr size_t
    __find_uniq_type_in_pack()
    {
      constexpr size_t __sz = sizeof...(_Types);
      constexpr bool __found[__sz] = { __is_same(_Tp, _Types) ... };
      size_t __n = __sz;
      for (size_t __i = 0; __i < __sz; ++__i)
 {
   if (__found[__i])
     {
       if (__n < __sz)
  return __sz;
       __n = __i;
     }
 }
      return __n;
    }
  template<size_t __i, typename _Tp>
    using tuple_element_t = typename tuple_element<__i, _Tp>::type;
  template<typename _Tp, _Tp... _Idx>
    struct integer_sequence
    {
      typedef _Tp value_type;
      static constexpr size_t size() noexcept { return sizeof...(_Idx); }
    };
  template<typename _Tp, _Tp _Num>
    using make_integer_sequence
      = integer_sequence<_Tp, __integer_pack(_Num)...>;
  template<size_t... _Idx>
    using index_sequence = integer_sequence<size_t, _Idx...>;
  template<size_t _Num>
    using make_index_sequence = make_integer_sequence<size_t, _Num>;
  template<typename... _Types>
    using index_sequence_for = make_index_sequence<sizeof...(_Types)>;
  struct in_place_t {
    explicit in_place_t() = default;
  };
  inline constexpr in_place_t in_place{};
  template<typename _Tp> struct in_place_type_t
  {
    explicit in_place_type_t() = default;
  };
  template<typename _Tp>
    inline constexpr in_place_type_t<_Tp> in_place_type{};
  template<size_t _Idx> struct in_place_index_t
  {
    explicit in_place_index_t() = default;
  };
  template<size_t _Idx>
    inline constexpr in_place_index_t<_Idx> in_place_index{};
  template<typename>
    inline constexpr bool __is_in_place_type_v = false;
  template<typename _Tp>
    inline constexpr bool __is_in_place_type_v<in_place_type_t<_Tp>> = true;
  template<typename>
    inline constexpr bool __is_in_place_index_v = false;
  template<size_t _Nm>
    inline constexpr bool __is_in_place_index_v<in_place_index_t<_Nm>> = true;
  template<size_t _Np, typename... _Types>
    struct _Nth_type
    { using type = __type_pack_element<_Np, _Types...>; };
  struct _Swallow_assign
  {
    template<class _Tp>
      constexpr const _Swallow_assign&
      operator=(const _Tp&) const noexcept
      { return *this; }
  };
  inline constexpr _Swallow_assign ignore{};

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  struct piecewise_construct_t { explicit piecewise_construct_t() = default; };
  inline constexpr piecewise_construct_t piecewise_construct =
    piecewise_construct_t();
  template<typename _T1, typename _T2>
    struct pair;
  template<typename...>
    class tuple;
  template<typename _Tp, size_t _Nm>
    struct array;
  template<size_t...>
    struct _Index_tuple;
  template<size_t _Int, class _Tp1, class _Tp2>
    constexpr typename tuple_element<_Int, pair<_Tp1, _Tp2>>::type&
    get(pair<_Tp1, _Tp2>& __in) noexcept;
  template<size_t _Int, class _Tp1, class _Tp2>
    constexpr typename tuple_element<_Int, pair<_Tp1, _Tp2>>::type&&
    get(pair<_Tp1, _Tp2>&& __in) noexcept;
  template<size_t _Int, class _Tp1, class _Tp2>
    constexpr const typename tuple_element<_Int, pair<_Tp1, _Tp2>>::type&
    get(const pair<_Tp1, _Tp2>& __in) noexcept;
  template<size_t _Int, class _Tp1, class _Tp2>
    constexpr const typename tuple_element<_Int, pair<_Tp1, _Tp2>>::type&&
    get(const pair<_Tp1, _Tp2>&& __in) noexcept;
  template<size_t __i, typename... _Elements>
    constexpr __tuple_element_t<__i, tuple<_Elements...>>&
    get(tuple<_Elements...>& __t) noexcept;
  template<size_t __i, typename... _Elements>
    constexpr const __tuple_element_t<__i, tuple<_Elements...>>&
    get(const tuple<_Elements...>& __t) noexcept;
  template<size_t __i, typename... _Elements>
    constexpr __tuple_element_t<__i, tuple<_Elements...>>&&
    get(tuple<_Elements...>&& __t) noexcept;
  template<size_t __i, typename... _Elements>
    constexpr const __tuple_element_t<__i, tuple<_Elements...>>&&
    get(const tuple<_Elements...>&& __t) noexcept;
  template<size_t _Int, typename _Tp, size_t _Nm>
    constexpr _Tp&
    get(array<_Tp, _Nm>&) noexcept;
  template<size_t _Int, typename _Tp, size_t _Nm>
    constexpr _Tp&&
    get(array<_Tp, _Nm>&&) noexcept;
  template<size_t _Int, typename _Tp, size_t _Nm>
    constexpr const _Tp&
    get(const array<_Tp, _Nm>&) noexcept;
  template<size_t _Int, typename _Tp, size_t _Nm>
    constexpr const _Tp&&
    get(const array<_Tp, _Nm>&&) noexcept;
  template <bool, typename _T1, typename _T2>
    struct _PCC
    {
      template <typename _U1, typename _U2>
      static constexpr bool _ConstructiblePair()
      {
 return __and_<is_constructible<_T1, const _U1&>,
        is_constructible<_T2, const _U2&>>::value;
      }
      template <typename _U1, typename _U2>
      static constexpr bool _ImplicitlyConvertiblePair()
      {
 return __and_<is_convertible<const _U1&, _T1>,
        is_convertible<const _U2&, _T2>>::value;
      }
      template <typename _U1, typename _U2>
      static constexpr bool _MoveConstructiblePair()
      {
 return __and_<is_constructible<_T1, _U1&&>,
        is_constructible<_T2, _U2&&>>::value;
      }
      template <typename _U1, typename _U2>
      static constexpr bool _ImplicitlyMoveConvertiblePair()
      {
 return __and_<is_convertible<_U1&&, _T1>,
        is_convertible<_U2&&, _T2>>::value;
      }
    };
  template <typename _T1, typename _T2>
    struct _PCC<false, _T1, _T2>
    {
      template <typename _U1, typename _U2>
      static constexpr bool _ConstructiblePair()
      {
 return false;
      }
      template <typename _U1, typename _U2>
      static constexpr bool _ImplicitlyConvertiblePair()
      {
 return false;
      }
      template <typename _U1, typename _U2>
      static constexpr bool _MoveConstructiblePair()
      {
 return false;
      }
      template <typename _U1, typename _U2>
      static constexpr bool _ImplicitlyMoveConvertiblePair()
      {
 return false;
      }
    };
  template<typename _U1, typename _U2> class __pair_base
  {
    template<typename _T1, typename _T2> friend struct pair;
    __pair_base() = default;
    ~__pair_base() = default;
    __pair_base(const __pair_base&) = default;
    __pair_base& operator=(const __pair_base&) = delete;
  };
  template<typename _T1, typename _T2>
    struct pair
    : public __pair_base<_T1, _T2>
    {
      typedef _T1 first_type;
      typedef _T2 second_type;
      _T1 first;
      _T2 second;
      constexpr pair(const pair&) = default;
      constexpr pair(pair&&) = default;
      template<typename... _Args1, typename... _Args2>

 pair(piecewise_construct_t, tuple<_Args1...>, tuple<_Args2...>);
      void
      swap(pair& __p)
      noexcept(__and_<__is_nothrow_swappable<_T1>,
        __is_nothrow_swappable<_T2>>::value)
      {
 using std::swap;
 swap(first, __p.first);
 swap(second, __p.second);
      }
    private:
      template<typename... _Args1, size_t... _Indexes1,
        typename... _Args2, size_t... _Indexes2>

 pair(tuple<_Args1...>&, tuple<_Args2...>&,
      _Index_tuple<_Indexes1...>, _Index_tuple<_Indexes2...>);
    public:
      template <typename _U1 = _T1,
                typename _U2 = _T2,
                typename enable_if<__and_<
                                     __is_implicitly_default_constructible<_U1>,
                                     __is_implicitly_default_constructible<_U2>>
                                   ::value, bool>::type = true>
      constexpr pair()
      : first(), second() { }
      template <typename _U1 = _T1,
                typename _U2 = _T2,
                typename enable_if<__and_<
                       is_default_constructible<_U1>,
                       is_default_constructible<_U2>,
                       __not_<
                         __and_<__is_implicitly_default_constructible<_U1>,
                                __is_implicitly_default_constructible<_U2>>>>
                                   ::value, bool>::type = false>
      explicit constexpr pair()
      : first(), second() { }
      using _PCCP = _PCC<true, _T1, _T2>;
      template<typename _U1 = _T1, typename _U2=_T2, typename
        enable_if<_PCCP::template
      _ConstructiblePair<_U1, _U2>()
                  && _PCCP::template
      _ImplicitlyConvertiblePair<_U1, _U2>(),
                         bool>::type=true>
      constexpr pair(const _T1& __a, const _T2& __b)
      : first(__a), second(__b) { }
       template<typename _U1 = _T1, typename _U2=_T2, typename
  enable_if<_PCCP::template
       _ConstructiblePair<_U1, _U2>()
                   && !_PCCP::template
       _ImplicitlyConvertiblePair<_U1, _U2>(),
                         bool>::type=false>
      explicit constexpr pair(const _T1& __a, const _T2& __b)
      : first(__a), second(__b) { }
      template <typename _U1, typename _U2>
        using _PCCFP = _PCC<!is_same<_T1, _U1>::value
       || !is_same<_T2, _U2>::value,
       _T1, _T2>;
      template<typename _U1, typename _U2, typename
        enable_if<_PCCFP<_U1, _U2>::template
      _ConstructiblePair<_U1, _U2>()
                  && _PCCFP<_U1, _U2>::template
      _ImplicitlyConvertiblePair<_U1, _U2>(),
     bool>::type=true>
 constexpr pair(const pair<_U1, _U2>& __p)
 : first(__p.first), second(__p.second)
 { ; }
      template<typename _U1, typename _U2, typename
        enable_if<_PCCFP<_U1, _U2>::template
      _ConstructiblePair<_U1, _U2>()
    && !_PCCFP<_U1, _U2>::template
      _ImplicitlyConvertiblePair<_U1, _U2>(),
                         bool>::type=false>
 explicit constexpr pair(const pair<_U1, _U2>& __p)
 : first(__p.first), second(__p.second)
 { ; }
    private:
      struct __zero_as_null_pointer_constant
      {
 __zero_as_null_pointer_constant(int __zero_as_null_pointer_constant::*)
 { }
 template<typename _Tp,
   typename = __enable_if_t<is_null_pointer<_Tp>::value>>
 __zero_as_null_pointer_constant(_Tp) = delete;
      };
    public:
      template<typename _U1,
        __enable_if_t<__and_<__not_<is_reference<_U1>>,
        is_pointer<_T2>,
        is_constructible<_T1, _U1>,
        __not_<is_constructible<_T1, const _U1&>>,
        is_convertible<_U1, _T1>>::value,
        bool> = true>
 __attribute__ ((__deprecated__ ("use 'nullptr' instead of '0' to " "initialize std::pair of move-only " "type and pointer")))
 constexpr
 pair(_U1&& __x, __zero_as_null_pointer_constant, ...)
 : first(std::forward<_U1>(__x)), second(nullptr)
 { ; }
      template<typename _U1,
        __enable_if_t<__and_<__not_<is_reference<_U1>>,
        is_pointer<_T2>,
        is_constructible<_T1, _U1>,
        __not_<is_constructible<_T1, const _U1&>>,
        __not_<is_convertible<_U1, _T1>>>::value,
        bool> = false>
 __attribute__ ((__deprecated__ ("use 'nullptr' instead of '0' to " "initialize std::pair of move-only " "type and pointer")))
 explicit constexpr
 pair(_U1&& __x, __zero_as_null_pointer_constant, ...)
 : first(std::forward<_U1>(__x)), second(nullptr)
 { ; }
      template<typename _U2,
        __enable_if_t<__and_<is_pointer<_T1>,
        __not_<is_reference<_U2>>,
        is_constructible<_T2, _U2>,
        __not_<is_constructible<_T2, const _U2&>>,
        is_convertible<_U2, _T2>>::value,
        bool> = true>
 __attribute__ ((__deprecated__ ("use 'nullptr' instead of '0' to " "initialize std::pair of move-only " "type and pointer")))
 constexpr
 pair(__zero_as_null_pointer_constant, _U2&& __y, ...)
 : first(nullptr), second(std::forward<_U2>(__y))
 { ; }
      template<typename _U2,
        __enable_if_t<__and_<is_pointer<_T1>,
        __not_<is_reference<_U2>>,
        is_constructible<_T2, _U2>,
        __not_<is_constructible<_T2, const _U2&>>,
        __not_<is_convertible<_U2, _T2>>>::value,
        bool> = false>
 __attribute__ ((__deprecated__ ("use 'nullptr' instead of '0' to " "initialize std::pair of move-only " "type and pointer")))
 explicit constexpr
 pair(__zero_as_null_pointer_constant, _U2&& __y, ...)
 : first(nullptr), second(std::forward<_U2>(__y))
 { ; }
      template<typename _U1, typename _U2, typename
        enable_if<_PCCP::template
      _MoveConstructiblePair<_U1, _U2>()
     && _PCCP::template
      _ImplicitlyMoveConvertiblePair<_U1, _U2>(),
                         bool>::type=true>
 constexpr pair(_U1&& __x, _U2&& __y)
 : first(std::forward<_U1>(__x)), second(std::forward<_U2>(__y))
 { ; }
      template<typename _U1, typename _U2, typename
        enable_if<_PCCP::template
      _MoveConstructiblePair<_U1, _U2>()
     && !_PCCP::template
      _ImplicitlyMoveConvertiblePair<_U1, _U2>(),
                         bool>::type=false>
 explicit constexpr pair(_U1&& __x, _U2&& __y)
 : first(std::forward<_U1>(__x)), second(std::forward<_U2>(__y))
 { ; }
      template<typename _U1, typename _U2, typename
        enable_if<_PCCFP<_U1, _U2>::template
      _MoveConstructiblePair<_U1, _U2>()
     && _PCCFP<_U1, _U2>::template
      _ImplicitlyMoveConvertiblePair<_U1, _U2>(),
                         bool>::type=true>
 constexpr pair(pair<_U1, _U2>&& __p)
 : first(std::forward<_U1>(__p.first)),
   second(std::forward<_U2>(__p.second))
 { ; }
      template<typename _U1, typename _U2, typename
        enable_if<_PCCFP<_U1, _U2>::template
      _MoveConstructiblePair<_U1, _U2>()
     && !_PCCFP<_U1, _U2>::template
      _ImplicitlyMoveConvertiblePair<_U1, _U2>(),
                         bool>::type=false>
 explicit constexpr pair(pair<_U1, _U2>&& __p)
 : first(std::forward<_U1>(__p.first)),
   second(std::forward<_U2>(__p.second))
 { ; }
      pair&
      operator=(__conditional_t<__and_<is_copy_assignable<_T1>,
           is_copy_assignable<_T2>>::value,
    const pair&, const __nonesuch&> __p)
      {
 first = __p.first;
 second = __p.second;
 return *this;
      }
      pair&
      operator=(__conditional_t<__and_<is_move_assignable<_T1>,
           is_move_assignable<_T2>>::value,
    pair&&, __nonesuch&&> __p)
      noexcept(__and_<is_nothrow_move_assignable<_T1>,
        is_nothrow_move_assignable<_T2>>::value)
      {
 first = std::forward<first_type>(__p.first);
 second = std::forward<second_type>(__p.second);
 return *this;
      }
      template<typename _U1, typename _U2>
 typename enable_if<__and_<is_assignable<_T1&, const _U1&>,
      is_assignable<_T2&, const _U2&>>::value,
      pair&>::type
 operator=(const pair<_U1, _U2>& __p)
 {
   first = __p.first;
   second = __p.second;
   return *this;
 }
      template<typename _U1, typename _U2>
 typename enable_if<__and_<is_assignable<_T1&, _U1&&>,
      is_assignable<_T2&, _U2&&>>::value,
      pair&>::type
 operator=(pair<_U1, _U2>&& __p)
 {
   first = std::forward<_U1>(__p.first);
   second = std::forward<_U2>(__p.second);
   return *this;
 }
    };
  template<typename _T1, typename _T2> pair(_T1, _T2) -> pair<_T1, _T2>;
  template<typename _T1, typename _T2>
    [[__nodiscard__]]
    inline constexpr bool
    operator==(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __x.first == __y.first && __x.second == __y.second; }
  template<typename _T1, typename _T2>
    [[__nodiscard__]]
    inline constexpr bool
    operator<(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __x.first < __y.first
      || (!(__y.first < __x.first) && __x.second < __y.second); }
  template<typename _T1, typename _T2>
    [[__nodiscard__]]
    inline constexpr bool
    operator!=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__x == __y); }
  template<typename _T1, typename _T2>
    [[__nodiscard__]]
    inline constexpr bool
    operator>(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __y < __x; }
  template<typename _T1, typename _T2>
    [[__nodiscard__]]
    inline constexpr bool
    operator<=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__y < __x); }
  template<typename _T1, typename _T2>
    [[__nodiscard__]]
    inline constexpr bool
    operator>=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__x < __y); }
  template<typename _T1, typename _T2>
    inline
    typename enable_if<__and_<__is_swappable<_T1>,
                              __is_swappable<_T2>>::value>::type
    swap(pair<_T1, _T2>& __x, pair<_T1, _T2>& __y)
    noexcept(noexcept(__x.swap(__y)))
    { __x.swap(__y); }
  template<typename _T1, typename _T2>
    typename enable_if<!__and_<__is_swappable<_T1>,
          __is_swappable<_T2>>::value>::type
    swap(pair<_T1, _T2>&, pair<_T1, _T2>&) = delete;
  template<typename _T1, typename _T2>
    constexpr pair<typename __decay_and_strip<_T1>::__type,
                   typename __decay_and_strip<_T2>::__type>
    make_pair(_T1&& __x, _T2&& __y)
    {
      typedef typename __decay_and_strip<_T1>::__type __ds_type1;
      typedef typename __decay_and_strip<_T2>::__type __ds_type2;
      typedef pair<__ds_type1, __ds_type2> __pair_type;
      return __pair_type(std::forward<_T1>(__x), std::forward<_T2>(__y));
    }
  template<typename _T1, typename _T2>
    struct __is_tuple_like_impl<pair<_T1, _T2>> : true_type
    { };
  template<class _Tp1, class _Tp2>
    struct tuple_size<pair<_Tp1, _Tp2>>
    : public integral_constant<size_t, 2> { };
  template<class _Tp1, class _Tp2>
    struct tuple_element<0, pair<_Tp1, _Tp2>>
    { typedef _Tp1 type; };
  template<class _Tp1, class _Tp2>
    struct tuple_element<1, pair<_Tp1, _Tp2>>
    { typedef _Tp2 type; };
  template<typename _Tp1, typename _Tp2>
    inline constexpr size_t tuple_size_v<pair<_Tp1, _Tp2>> = 2;
  template<typename _Tp1, typename _Tp2>
    inline constexpr size_t tuple_size_v<const pair<_Tp1, _Tp2>> = 2;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++14-extensions"
#pragma GCC diagnostic ignored "-Wc++17-extensions"
  template<typename _Tp>
    inline constexpr bool __is_pair = false;
  template<typename _Tp, typename _Up>
    inline constexpr bool __is_pair<pair<_Tp, _Up>> = true;
#pragma GCC diagnostic pop
  template<size_t _Int>
    struct __pair_get;
  template<>
    struct __pair_get<0>
    {
      template<typename _Tp1, typename _Tp2>
 static constexpr _Tp1&
 __get(pair<_Tp1, _Tp2>& __pair) noexcept
 { return __pair.first; }
      template<typename _Tp1, typename _Tp2>
 static constexpr _Tp1&&
 __move_get(pair<_Tp1, _Tp2>&& __pair) noexcept
 { return std::forward<_Tp1>(__pair.first); }
      template<typename _Tp1, typename _Tp2>
 static constexpr const _Tp1&
 __const_get(const pair<_Tp1, _Tp2>& __pair) noexcept
 { return __pair.first; }
      template<typename _Tp1, typename _Tp2>
 static constexpr const _Tp1&&
 __const_move_get(const pair<_Tp1, _Tp2>&& __pair) noexcept
 { return std::forward<const _Tp1>(__pair.first); }
    };
  template<>
    struct __pair_get<1>
    {
      template<typename _Tp1, typename _Tp2>
 static constexpr _Tp2&
 __get(pair<_Tp1, _Tp2>& __pair) noexcept
 { return __pair.second; }
      template<typename _Tp1, typename _Tp2>
 static constexpr _Tp2&&
 __move_get(pair<_Tp1, _Tp2>&& __pair) noexcept
 { return std::forward<_Tp2>(__pair.second); }
      template<typename _Tp1, typename _Tp2>
 static constexpr const _Tp2&
 __const_get(const pair<_Tp1, _Tp2>& __pair) noexcept
 { return __pair.second; }
      template<typename _Tp1, typename _Tp2>
 static constexpr const _Tp2&&
 __const_move_get(const pair<_Tp1, _Tp2>&& __pair) noexcept
 { return std::forward<const _Tp2>(__pair.second); }
    };
  template<size_t _Int, class _Tp1, class _Tp2>
    constexpr typename tuple_element<_Int, pair<_Tp1, _Tp2>>::type&
    get(pair<_Tp1, _Tp2>& __in) noexcept
    { return __pair_get<_Int>::__get(__in); }
  template<size_t _Int, class _Tp1, class _Tp2>
    constexpr typename tuple_element<_Int, pair<_Tp1, _Tp2>>::type&&
    get(pair<_Tp1, _Tp2>&& __in) noexcept
    { return __pair_get<_Int>::__move_get(std::move(__in)); }
  template<size_t _Int, class _Tp1, class _Tp2>
    constexpr const typename tuple_element<_Int, pair<_Tp1, _Tp2>>::type&
    get(const pair<_Tp1, _Tp2>& __in) noexcept
    { return __pair_get<_Int>::__const_get(__in); }
  template<size_t _Int, class _Tp1, class _Tp2>
    constexpr const typename tuple_element<_Int, pair<_Tp1, _Tp2>>::type&&
    get(const pair<_Tp1, _Tp2>&& __in) noexcept
    { return __pair_get<_Int>::__const_move_get(std::move(__in)); }
  template <typename _Tp, typename _Up>
    constexpr _Tp&
    get(pair<_Tp, _Up>& __p) noexcept
    { return __p.first; }
  template <typename _Tp, typename _Up>
    constexpr const _Tp&
    get(const pair<_Tp, _Up>& __p) noexcept
    { return __p.first; }
  template <typename _Tp, typename _Up>
    constexpr _Tp&&
    get(pair<_Tp, _Up>&& __p) noexcept
    { return std::forward<_Tp>(__p.first); }
  template <typename _Tp, typename _Up>
    constexpr const _Tp&&
    get(const pair<_Tp, _Up>&& __p) noexcept
    { return std::forward<const _Tp>(__p.first); }
  template <typename _Tp, typename _Up>
    constexpr _Tp&
    get(pair<_Up, _Tp>& __p) noexcept
    { return __p.second; }
  template <typename _Tp, typename _Up>
    constexpr const _Tp&
    get(const pair<_Up, _Tp>& __p) noexcept
    { return __p.second; }
  template <typename _Tp, typename _Up>
    constexpr _Tp&&
    get(pair<_Up, _Tp>&& __p) noexcept
    { return std::forward<_Tp>(__p.second); }
  template <typename _Tp, typename _Up>
    constexpr const _Tp&&
    get(const pair<_Up, _Tp>&& __p) noexcept
    { return std::forward<const _Tp>(__p.second); }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  struct input_iterator_tag { };
  struct output_iterator_tag { };
  struct forward_iterator_tag : public input_iterator_tag { };
  struct bidirectional_iterator_tag : public forward_iterator_tag { };
  struct random_access_iterator_tag : public bidirectional_iterator_tag { };
  template<typename _Category, typename _Tp, typename _Distance = ptrdiff_t,
           typename _Pointer = _Tp*, typename _Reference = _Tp&>
    struct [[__deprecated__]] iterator
    {
      typedef _Category iterator_category;
      typedef _Tp value_type;
      typedef _Distance difference_type;
      typedef _Pointer pointer;
      typedef _Reference reference;
    };
  template<typename _Iterator>
    struct iterator_traits;
  template<typename _Iterator, typename = __void_t<>>
    struct __iterator_traits { };
  template<typename _Iterator>
    struct __iterator_traits<_Iterator,
        __void_t<typename _Iterator::iterator_category,
          typename _Iterator::value_type,
          typename _Iterator::difference_type,
          typename _Iterator::pointer,
          typename _Iterator::reference>>
    {
      typedef typename _Iterator::iterator_category iterator_category;
      typedef typename _Iterator::value_type value_type;
      typedef typename _Iterator::difference_type difference_type;
      typedef typename _Iterator::pointer pointer;
      typedef typename _Iterator::reference reference;
    };
  template<typename _Iterator>
    struct iterator_traits
    : public __iterator_traits<_Iterator> { };
  template<typename _Tp>
    struct iterator_traits<_Tp*>
    {
      typedef random_access_iterator_tag iterator_category;
      typedef _Tp value_type;
      typedef ptrdiff_t difference_type;
      typedef _Tp* pointer;
      typedef _Tp& reference;
    };
  template<typename _Tp>
    struct iterator_traits<const _Tp*>
    {
      typedef random_access_iterator_tag iterator_category;
      typedef _Tp value_type;
      typedef ptrdiff_t difference_type;
      typedef const _Tp* pointer;
      typedef const _Tp& reference;
    };
  template<typename _Iter>
    __attribute__((__always_inline__))
    inline constexpr
    typename iterator_traits<_Iter>::iterator_category
    __iterator_category(const _Iter&)
    { return typename iterator_traits<_Iter>::iterator_category(); }
  template<typename _Iter>
    using __iter_category_t
      = typename iterator_traits<_Iter>::iterator_category;
  template<typename _InIter>
    using _RequireInputIter =
      __enable_if_t<is_convertible<__iter_category_t<_InIter>,
       input_iterator_tag>::value>;
  template<typename _It,
    typename _Cat = __iter_category_t<_It>>
    struct __is_random_access_iter
      : is_base_of<random_access_iterator_tag, _Cat>
    {
      typedef is_base_of<random_access_iterator_tag, _Cat> _Base;
      enum { __value = _Base::value };
    };

}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic pop
namespace std __attribute__ ((__visibility__ ("default")))
{


  template <typename> struct _List_iterator;
  template <typename> struct _List_const_iterator;

  template<typename _InputIterator>
    inline constexpr
    typename iterator_traits<_InputIterator>::difference_type
    __distance(_InputIterator __first, _InputIterator __last,
               input_iterator_tag)
    {
     
      typename iterator_traits<_InputIterator>::difference_type __n = 0;
      while (__first != __last)
 {
   ++__first;
   ++__n;
 }
      return __n;
    }
  template<typename _RandomAccessIterator>
    __attribute__((__always_inline__))
    inline constexpr
    typename iterator_traits<_RandomAccessIterator>::difference_type
    __distance(_RandomAccessIterator __first, _RandomAccessIterator __last,
               random_access_iterator_tag)
    {
     
      return __last - __first;
    }
  template<typename _Tp>
    ptrdiff_t
    __distance(std::_List_iterator<_Tp>,
        std::_List_iterator<_Tp>,
        input_iterator_tag);
  template<typename _Tp>
    ptrdiff_t
    __distance(std::_List_const_iterator<_Tp>,
        std::_List_const_iterator<_Tp>,
        input_iterator_tag);
  template<typename _OutputIterator>
    void
    __distance(_OutputIterator, _OutputIterator, output_iterator_tag) = delete;
  template<typename _InputIterator>
    [[__nodiscard__]] __attribute__((__always_inline__))
    inline constexpr
    typename iterator_traits<_InputIterator>::difference_type
    distance(_InputIterator __first, _InputIterator __last)
    {
      return std::__distance(__first, __last,
        std::__iterator_category(__first));
    }
  template<typename _InputIterator, typename _Distance>
    inline constexpr void
    __advance(_InputIterator& __i, _Distance __n, input_iterator_tag)
    {
     
      do { if (__builtin_expect(!bool(__n >= 0), false)) std::__glibcxx_assert_fail("/usr/include/c++/16/bits/stl_iterator_base_funcs.h", 203, __PRETTY_FUNCTION__, "__n >= 0"); } while (false);
      while (__n-- > 0)
 ++__i;
    }
  template<typename _BidirectionalIterator, typename _Distance>
    inline constexpr void
    __advance(_BidirectionalIterator& __i, _Distance __n,
       bidirectional_iterator_tag)
    {
     
      if (__n > 0)
        while (__n--)
   ++__i;
      else
        while (__n++)
   --__i;
    }
  template<typename _RandomAccessIterator, typename _Distance>
    inline constexpr void
    __advance(_RandomAccessIterator& __i, _Distance __n,
              random_access_iterator_tag)
    {
     
      if (__builtin_constant_p(__n) && __n == 1)
 ++__i;
      else if (__builtin_constant_p(__n) && __n == -1)
 --__i;
      else
 __i += __n;
    }
  template<typename _OutputIterator, typename _Distance>
    void
    __advance(_OutputIterator&, _Distance, output_iterator_tag) = delete;
  template<typename _InputIterator, typename _Distance>
    __attribute__((__always_inline__))
    inline constexpr void
    advance(_InputIterator& __i, _Distance __n)
    {
 {
   typename iterator_traits<_InputIterator>::difference_type __d = __n;
   std::__advance(__i, __d, std::__iterator_category(__i));
 }
    }
  template<typename _InputIterator>
    [[__nodiscard__]] [[__gnu__::__always_inline__]]
    inline constexpr _InputIterator
    next(_InputIterator __x, typename
  iterator_traits<_InputIterator>::difference_type __n = 1)
    {
     
      std::advance(__x, __n);
      return __x;
    }
  template<typename _BidirectionalIterator>
    [[__nodiscard__]] [[__gnu__::__always_inline__]]
    inline constexpr _BidirectionalIterator
    prev(_BidirectionalIterator __x, typename
  iterator_traits<_BidirectionalIterator>::difference_type __n = 1)
    {
     
      std::advance(__x, -__n);
      return __x;
    }
  template<typename _Iter>
    __attribute__((__always_inline__))
    inline constexpr
    typename iterator_traits<_Iter>::iterator_category
    __iter_concept_or_category()
    { return typename iterator_traits<_Iter>::iterator_category(); }
  template<typename _Iter>
    __attribute__((__always_inline__))
    inline constexpr
    typename iterator_traits<_Iter>::iterator_category
    __iter_concept_or_category(const _Iter&)
    { return typename iterator_traits<_Iter>::iterator_category(); }
  template<typename _Iter,
    typename _Cat = __decltype(__iter_concept_or_category<_Iter>())>
    struct __is_any_random_access_iter
      : is_base_of<random_access_iterator_tag, _Cat>
    { enum { __value = __is_base_of(random_access_iterator_tag, _Cat) }; };

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  class __undefined;
  template<typename _Tp>
    struct __get_first_arg
    { using type = __undefined; };
  template<template<typename, typename...> class _SomeTemplate, typename _Tp,
           typename... _Types>
    struct __get_first_arg<_SomeTemplate<_Tp, _Types...>>
    { using type = _Tp; };
  template<typename _Tp, typename _Up>
    struct __replace_first_arg
    { };
  template<template<typename, typename...> class _SomeTemplate, typename _Up,
           typename _Tp, typename... _Types>
    struct __replace_first_arg<_SomeTemplate<_Tp, _Types...>, _Up>
    { using type = _SomeTemplate<_Up, _Types...>; };
  template<typename _Ptr, typename = void>
    struct __ptr_traits_elem : __get_first_arg<_Ptr>
    { };
  template<typename _Ptr>
    struct __ptr_traits_elem<_Ptr, __void_t<typename _Ptr::element_type>>
    { using type = typename _Ptr::element_type; };
  template<typename _Ptr>
    using __ptr_traits_elem_t = typename __ptr_traits_elem<_Ptr>::type;
  template<typename _Ptr, typename _Elt, bool = is_void<_Elt>::value>
    struct __ptr_traits_ptr_to
    {
      using pointer = _Ptr;
      using element_type = _Elt;
      static pointer
      pointer_to(element_type& __r)
      { return pointer::pointer_to(__r); }
    };
  template<typename _Ptr, typename _Elt>
    struct __ptr_traits_ptr_to<_Ptr, _Elt, true>
    { };
  template<typename _Tp>
    struct __ptr_traits_ptr_to<_Tp*, _Tp, false>
    {
      using pointer = _Tp*;
      using element_type = _Tp;
      static pointer
      pointer_to(element_type& __r) noexcept
      { return std::addressof(__r); }
    };
  template<typename _Ptr, typename _Elt>
    struct __ptr_traits_impl : __ptr_traits_ptr_to<_Ptr, _Elt>
    {
    private:
      template<typename _Tp>
 using __diff_t = typename _Tp::difference_type;
      template<typename _Tp, typename _Up>
 using __rebind = __type_identity<typename _Tp::template rebind<_Up>>;
    public:
      using pointer = _Ptr;
      using element_type = _Elt;
      using difference_type = __detected_or_t<ptrdiff_t, __diff_t, _Ptr>;
      template<typename _Up>
 using rebind = typename __detected_or_t<__replace_first_arg<_Ptr, _Up>,
      __rebind, _Ptr, _Up>::type;
    };
  template<typename _Ptr>
    struct __ptr_traits_impl<_Ptr, __undefined>
    { };
  template<typename _Ptr>
    struct pointer_traits : __ptr_traits_impl<_Ptr, __ptr_traits_elem_t<_Ptr>>
    { };
  template<typename _Tp>
    struct pointer_traits<_Tp*> : __ptr_traits_ptr_to<_Tp*, _Tp>
    {
      typedef _Tp* pointer;
      typedef _Tp element_type;
      typedef ptrdiff_t difference_type;
      template<typename _Up> using rebind = _Up*;
    };
  template<typename _Ptr, typename _Tp>
    using __ptr_rebind = typename pointer_traits<_Ptr>::template rebind<_Tp>;
  template<typename _Tp>
    [[__gnu__::__always_inline__]]
    constexpr _Tp*
    __to_address(_Tp* __ptr) noexcept
    {
      static_assert(!std::is_function<_Tp>::value, "std::to_address argument "
      "must not be a function pointer");
      return __ptr;
    }
  template<typename _Ptr>
    constexpr typename std::pointer_traits<_Ptr>::element_type*
    __to_address(const _Ptr& __ptr)
    { return std::__to_address(__ptr.operator->()); }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template<typename _Iterator>
    class reverse_iterator
    : public iterator<typename iterator_traits<_Iterator>::iterator_category,
        typename iterator_traits<_Iterator>::value_type,
        typename iterator_traits<_Iterator>::difference_type,
        typename iterator_traits<_Iterator>::pointer,
                      typename iterator_traits<_Iterator>::reference>
    {
      template<typename _Iter>
 friend class reverse_iterator;
    protected:
      _Iterator current;
      typedef iterator_traits<_Iterator> __traits_type;
    public:
      typedef _Iterator iterator_type;
      typedef typename __traits_type::pointer pointer;
      typedef typename __traits_type::difference_type difference_type;
      typedef typename __traits_type::reference reference;
      constexpr
      reverse_iterator()
      noexcept(noexcept(_Iterator()))
      : current()
      { }
      explicit constexpr
      reverse_iterator(iterator_type __x)
      noexcept(noexcept(_Iterator(__x)))
      : current(__x)
      { }
      constexpr
      reverse_iterator(const reverse_iterator& __x)
      noexcept(noexcept(_Iterator(__x.current)))
      : current(__x.current)
      { }
      reverse_iterator& operator=(const reverse_iterator&) = default;
      template<typename _Iter>
 constexpr
        reverse_iterator(const reverse_iterator<_Iter>& __x)
 noexcept(noexcept(_Iterator(__x.current)))
 : current(__x.current)
 { }
      template<typename _Iter>
 constexpr
 reverse_iterator&
 operator=(const reverse_iterator<_Iter>& __x)
 noexcept(noexcept(current = __x.current))
 {
   current = __x.current;
   return *this;
 }
      [[__nodiscard__]]
      constexpr iterator_type
      base() const
      noexcept(noexcept(_Iterator(current)))
      { return current; }
      [[__nodiscard__]]
      constexpr reference
      operator*() const
      {
 _Iterator __tmp = current;
 return *--__tmp;
      }
      [[__nodiscard__]]
      constexpr pointer
      operator->() const
      {
 _Iterator __tmp = current;
 --__tmp;
 return _S_to_pointer(__tmp);
      }
      constexpr reverse_iterator&
      operator++()
      {
 --current;
 return *this;
      }
      constexpr reverse_iterator
      operator++(int)
      {
 reverse_iterator __tmp = *this;
 --current;
 return __tmp;
      }
      constexpr reverse_iterator&
      operator--()
      {
 ++current;
 return *this;
      }
      constexpr reverse_iterator
      operator--(int)
      {
 reverse_iterator __tmp = *this;
 ++current;
 return __tmp;
      }
      [[__nodiscard__]]
      constexpr reverse_iterator
      operator+(difference_type __n) const
      { return reverse_iterator(current - __n); }
      constexpr reverse_iterator&
      operator+=(difference_type __n)
      {
 current -= __n;
 return *this;
      }
      [[__nodiscard__]]
      constexpr reverse_iterator
      operator-(difference_type __n) const
      { return reverse_iterator(current + __n); }
      constexpr reverse_iterator&
      operator-=(difference_type __n)
      {
 current += __n;
 return *this;
      }
      [[__nodiscard__]]
      constexpr reference
      operator[](difference_type __n) const
      { return *(*this + __n); }
    private:
      template<typename _Tp>
 static constexpr _Tp*
 _S_to_pointer(_Tp* __p)
        { return __p; }
      template<typename _Tp>
 static constexpr pointer
 _S_to_pointer(_Tp __t)
        { return __t.operator->(); }
    };
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator==(const reverse_iterator<_Iterator>& __x,
        const reverse_iterator<_Iterator>& __y)
    { return __x.base() == __y.base(); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator<(const reverse_iterator<_Iterator>& __x,
       const reverse_iterator<_Iterator>& __y)
    { return __y.base() < __x.base(); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator!=(const reverse_iterator<_Iterator>& __x,
        const reverse_iterator<_Iterator>& __y)
    { return !(__x == __y); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator>(const reverse_iterator<_Iterator>& __x,
       const reverse_iterator<_Iterator>& __y)
    { return __y < __x; }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator<=(const reverse_iterator<_Iterator>& __x,
        const reverse_iterator<_Iterator>& __y)
    { return !(__y < __x); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator>=(const reverse_iterator<_Iterator>& __x,
        const reverse_iterator<_Iterator>& __y)
    { return !(__x < __y); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator==(const reverse_iterator<_IteratorL>& __x,
        const reverse_iterator<_IteratorR>& __y)
    { return __x.base() == __y.base(); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator<(const reverse_iterator<_IteratorL>& __x,
       const reverse_iterator<_IteratorR>& __y)
    { return __x.base() > __y.base(); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator!=(const reverse_iterator<_IteratorL>& __x,
        const reverse_iterator<_IteratorR>& __y)
    { return __x.base() != __y.base(); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator>(const reverse_iterator<_IteratorL>& __x,
       const reverse_iterator<_IteratorR>& __y)
    { return __x.base() < __y.base(); }
  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator<=(const reverse_iterator<_IteratorL>& __x,
        const reverse_iterator<_IteratorR>& __y)
    { return __x.base() >= __y.base(); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator>=(const reverse_iterator<_IteratorL>& __x,
        const reverse_iterator<_IteratorR>& __y)
    { return __x.base() <= __y.base(); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr auto
    operator-(const reverse_iterator<_IteratorL>& __x,
       const reverse_iterator<_IteratorR>& __y)
    -> decltype(__y.base() - __x.base())
    { return __y.base() - __x.base(); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr reverse_iterator<_Iterator>
    operator+(typename reverse_iterator<_Iterator>::difference_type __n,
       const reverse_iterator<_Iterator>& __x)
    { return reverse_iterator<_Iterator>(__x.base() - __n); }
  template<typename _Iterator>
    inline constexpr reverse_iterator<_Iterator>
    __make_reverse_iterator(_Iterator __i)
    { return reverse_iterator<_Iterator>(__i); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr reverse_iterator<_Iterator>
    make_reverse_iterator(_Iterator __i)
    { return reverse_iterator<_Iterator>(__i); }
  template<typename _Iterator>
    struct __is_move_iterator<reverse_iterator<_Iterator> >
      : __is_move_iterator<_Iterator>
    { };
  template<typename _Container>
    class back_insert_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
    {
    protected:
      _Container* container;
    public:
      typedef _Container container_type;
      explicit
      back_insert_iterator(_Container& __x)
      : container(std::__addressof(__x)) { }
     
      back_insert_iterator&
      operator=(const typename _Container::value_type& __value)
      {
 container->push_back(__value);
 return *this;
      }
     
      back_insert_iterator&
      operator=(typename _Container::value_type&& __value)
      {
 container->push_back(std::move(__value));
 return *this;
      }
      [[__nodiscard__]]
      back_insert_iterator&
      operator*()
      { return *this; }
     
      back_insert_iterator&
      operator++()
      { return *this; }
     
      back_insert_iterator
      operator++(int)
      { return *this; }
    };
  template<typename _Container>
    [[__nodiscard__]]
    inline back_insert_iterator<_Container>
    back_inserter(_Container& __x)
    { return back_insert_iterator<_Container>(__x); }
  template<typename _Container>
    class front_insert_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
    {
    protected:
      _Container* container;
    public:
      typedef _Container container_type;
      explicit
      front_insert_iterator(_Container& __x)
      : container(std::__addressof(__x)) { }
     
      front_insert_iterator&
      operator=(const typename _Container::value_type& __value)
      {
 container->push_front(__value);
 return *this;
      }
     
      front_insert_iterator&
      operator=(typename _Container::value_type&& __value)
      {
 container->push_front(std::move(__value));
 return *this;
      }
      [[__nodiscard__]]
      front_insert_iterator&
      operator*()
      { return *this; }
     
      front_insert_iterator&
      operator++()
      { return *this; }
     
      front_insert_iterator
      operator++(int)
      { return *this; }
    };
  template<typename _Container>
    [[__nodiscard__]]
    inline front_insert_iterator<_Container>
    front_inserter(_Container& __x)
    { return front_insert_iterator<_Container>(__x); }
  template<typename _Container>
    class insert_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
    {
      typedef typename _Container::iterator _Iter;
    protected:
      _Container* container;
      _Iter iter;
    public:
      typedef _Container container_type;
     
      insert_iterator(_Container& __x, _Iter __i)
      : container(std::__addressof(__x)), iter(__i) {}
     
      insert_iterator&
      operator=(const typename _Container::value_type& __value)
      {
 iter = container->insert(iter, __value);
 ++iter;
 return *this;
      }
     
      insert_iterator&
      operator=(typename _Container::value_type&& __value)
      {
 iter = container->insert(iter, std::move(__value));
 ++iter;
 return *this;
      }
      [[__nodiscard__]]
      insert_iterator&
      operator*()
      { return *this; }
     
      insert_iterator&
      operator++()
      { return *this; }
     
      insert_iterator&
      operator++(int)
      { return *this; }
    };
#pragma GCC diagnostic pop
  template<typename _Container>
    [[__nodiscard__]]
    inline insert_iterator<_Container>
    inserter(_Container& __x, typename _Container::iterator __i)
    { return insert_iterator<_Container>(__x, __i); }

}
namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{

  template<typename _Iterator, typename _Container>
    class __normal_iterator
    {
    protected:
      _Iterator _M_current;
      typedef std::iterator_traits<_Iterator> __traits_type;
      template<typename _Iter>
 using __convertible_from
   = std::__enable_if_t<std::is_convertible<_Iter, _Iterator>::value>;
    public:
      typedef _Iterator iterator_type;
      typedef typename __traits_type::iterator_category iterator_category;
      typedef typename __traits_type::value_type value_type;
      typedef typename __traits_type::difference_type difference_type;
      typedef typename __traits_type::reference reference;
      typedef typename __traits_type::pointer pointer;
      __attribute__((__always_inline__))
      constexpr
      __normal_iterator() noexcept
      : _M_current() { }
      __attribute__((__always_inline__))
      explicit constexpr
      __normal_iterator(const _Iterator& __i) noexcept
      : _M_current(__i) { }
      template<typename _Iter, typename = __convertible_from<_Iter>>
 [[__gnu__::__always_inline__]]
 constexpr
 __normal_iterator(const __normal_iterator<_Iter, _Container>& __i)
 noexcept
        : _M_current(__i.base()) { }
      [[__nodiscard__]] __attribute__((__always_inline__))
      constexpr
      reference
      operator*() const noexcept
      { return *_M_current; }
      [[__nodiscard__]] __attribute__((__always_inline__))
      constexpr
      pointer
      operator->() const noexcept
      { return _M_current; }
      __attribute__((__always_inline__))
      constexpr
      __normal_iterator&
      operator++() noexcept
      {
 ++_M_current;
 return *this;
      }
      __attribute__((__always_inline__))
      constexpr
      __normal_iterator
      operator++(int) noexcept
      { return __normal_iterator(_M_current++); }
      __attribute__((__always_inline__))
      constexpr
      __normal_iterator&
      operator--() noexcept
      {
 --_M_current;
 return *this;
      }
      __attribute__((__always_inline__))
      constexpr
      __normal_iterator
      operator--(int) noexcept
      { return __normal_iterator(_M_current--); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      constexpr
      reference
      operator[](difference_type __n) const noexcept
      { return _M_current[__n]; }
      __attribute__((__always_inline__))
      constexpr
      __normal_iterator&
      operator+=(difference_type __n) noexcept
      { _M_current += __n; return *this; }
      [[__nodiscard__]] __attribute__((__always_inline__))
      constexpr
      __normal_iterator
      operator+(difference_type __n) const noexcept
      { return __normal_iterator(_M_current + __n); }
      __attribute__((__always_inline__))
      constexpr
      __normal_iterator&
      operator-=(difference_type __n) noexcept
      { _M_current -= __n; return *this; }
      [[__nodiscard__]] __attribute__((__always_inline__))
      constexpr
      __normal_iterator
      operator-(difference_type __n) const noexcept
      { return __normal_iterator(_M_current - __n); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      constexpr
      const _Iterator&
      base() const noexcept
      { return _M_current; }
    private:
      template<typename _Iter>
 [[__nodiscard__]] __attribute__((__always_inline__))
 friend
 constexpr
 bool
 operator==(const __normal_iterator& __lhs,
     const __normal_iterator<_Iter, _Container>& __rhs)
 noexcept
 { return __lhs.base() == __rhs.base(); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      friend
      constexpr
      bool
      operator==(const __normal_iterator& __lhs, const __normal_iterator& __rhs)
      noexcept
      { return __lhs.base() == __rhs.base(); }
      template<typename _Iter>
 [[__nodiscard__]] __attribute__((__always_inline__))
 friend
 constexpr
 bool
 operator!=(const __normal_iterator& __lhs,
     const __normal_iterator<_Iter, _Container>& __rhs)
 noexcept
 { return __lhs.base() != __rhs.base(); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      friend
      constexpr
      bool
      operator!=(const __normal_iterator& __lhs, const __normal_iterator& __rhs)
      noexcept
      { return __lhs.base() != __rhs.base(); }
      template<typename _Iter>
 [[__nodiscard__]] __attribute__((__always_inline__))
 friend
 constexpr
 inline bool
 operator<(const __normal_iterator& __lhs,
    const __normal_iterator<_Iter, _Container>& __rhs)
 noexcept
 { return __lhs.base() < __rhs.base(); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      friend
     
      bool
      operator<(const __normal_iterator& __lhs, const __normal_iterator& __rhs)
      noexcept
      { return __lhs.base() < __rhs.base(); }
      template<typename _Iter>
 [[__nodiscard__]] __attribute__((__always_inline__))
 friend
 constexpr
 bool
 operator>(const __normal_iterator& __lhs,
    const __normal_iterator<_Iter, _Container>& __rhs)
 noexcept
 { return __lhs.base() > __rhs.base(); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      friend
      constexpr
      bool
      operator>(const __normal_iterator& __lhs, const __normal_iterator& __rhs)
      noexcept
      { return __lhs.base() > __rhs.base(); }
      template<typename _Iter>
 [[__nodiscard__]] __attribute__((__always_inline__))
 friend
 constexpr
 bool
 operator<=(const __normal_iterator& __lhs,
     const __normal_iterator<_Iter, _Container>& __rhs)
 noexcept
 { return __lhs.base() <= __rhs.base(); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      friend
      constexpr
      bool
      operator<=(const __normal_iterator& __lhs, const __normal_iterator& __rhs)
      noexcept
      { return __lhs.base() <= __rhs.base(); }
      template<typename _Iter>
 [[__nodiscard__]] __attribute__((__always_inline__))
 friend
 constexpr
 bool
 operator>=(const __normal_iterator& __lhs,
     const __normal_iterator<_Iter, _Container>& __rhs)
 noexcept
 { return __lhs.base() >= __rhs.base(); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      friend
      constexpr
      bool
      operator>=(const __normal_iterator& __lhs, const __normal_iterator& __rhs)
      noexcept
      { return __lhs.base() >= __rhs.base(); }
      template<typename _Iter>
 [[__nodiscard__, __gnu__::__always_inline__]]
 friend
 constexpr auto
 operator-(const __normal_iterator& __lhs,
    const __normal_iterator<_Iter, _Container>& __rhs) noexcept
 -> decltype(__lhs.base() - __rhs.base())
 { return __lhs.base() - __rhs.base(); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      friend
      constexpr
      difference_type
      operator-(const __normal_iterator& __lhs, const __normal_iterator& __rhs)
 noexcept
      { return __lhs.base() - __rhs.base(); }
      [[__nodiscard__]] __attribute__((__always_inline__))
      friend
      constexpr
      __normal_iterator
      operator+(difference_type __n, const __normal_iterator& __i)
 noexcept
      { return __normal_iterator(__i.base() + __n); }
    };

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  template<typename _Iterator>
    class move_iterator
    {
      _Iterator _M_current;
      using __traits_type = iterator_traits<_Iterator>;
      using __base_ref = typename __traits_type::reference;
      template<typename _Iter2>
 friend class move_iterator;
    public:
      using iterator_type = _Iterator;
      typedef typename __traits_type::iterator_category iterator_category;
      typedef typename __traits_type::value_type value_type;
      typedef typename __traits_type::difference_type difference_type;
      typedef _Iterator pointer;
      using reference
 = __conditional_t<is_reference<__base_ref>::value,
     typename remove_reference<__base_ref>::type&&,
     __base_ref>;
      constexpr
      move_iterator()
      : _M_current() { }
      explicit constexpr
      move_iterator(iterator_type __i)
      : _M_current(std::move(__i)) { }
      template<typename _Iter>
 constexpr
 move_iterator(const move_iterator<_Iter>& __i)
 : _M_current(__i._M_current) { }
      template<typename _Iter>
 constexpr
 move_iterator& operator=(const move_iterator<_Iter>& __i)
 {
   _M_current = __i._M_current;
   return *this;
 }
      [[__nodiscard__]]
      constexpr iterator_type
      base() const
      { return _M_current; }
      [[__nodiscard__]]
      constexpr reference
      operator*() const
      { return static_cast<reference>(*_M_current); }
      [[__nodiscard__]]
      constexpr pointer
      operator->() const
      { return _M_current; }
      constexpr move_iterator&
      operator++()
      {
 ++_M_current;
 return *this;
      }
      constexpr move_iterator
      operator++(int)
      {
 move_iterator __tmp = *this;
 ++_M_current;
 return __tmp;
      }
      constexpr move_iterator&
      operator--()
      {
 --_M_current;
 return *this;
      }
      constexpr move_iterator
      operator--(int)
      {
 move_iterator __tmp = *this;
 --_M_current;
 return __tmp;
      }
      [[__nodiscard__]]
      constexpr move_iterator
      operator+(difference_type __n) const
      { return move_iterator(_M_current + __n); }
      constexpr move_iterator&
      operator+=(difference_type __n)
      {
 _M_current += __n;
 return *this;
      }
      [[__nodiscard__]]
      constexpr move_iterator
      operator-(difference_type __n) const
      { return move_iterator(_M_current - __n); }
      constexpr move_iterator&
      operator-=(difference_type __n)
      {
 _M_current -= __n;
 return *this;
      }
      [[__nodiscard__]]
      constexpr reference
      operator[](difference_type __n) const
      { return std::move(_M_current[__n]); }
    };
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator==(const move_iterator<_IteratorL>& __x,
        const move_iterator<_IteratorR>& __y)
    { return __x.base() == __y.base(); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator!=(const move_iterator<_IteratorL>& __x,
        const move_iterator<_IteratorR>& __y)
    { return !(__x == __y); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator<(const move_iterator<_IteratorL>& __x,
       const move_iterator<_IteratorR>& __y)
    { return __x.base() < __y.base(); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator<=(const move_iterator<_IteratorL>& __x,
        const move_iterator<_IteratorR>& __y)
    { return !(__y < __x); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator>(const move_iterator<_IteratorL>& __x,
       const move_iterator<_IteratorR>& __y)
    { return __y < __x; }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr bool
    operator>=(const move_iterator<_IteratorL>& __x,
        const move_iterator<_IteratorR>& __y)
    { return !(__x < __y); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator==(const move_iterator<_Iterator>& __x,
        const move_iterator<_Iterator>& __y)
    { return __x.base() == __y.base(); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator!=(const move_iterator<_Iterator>& __x,
        const move_iterator<_Iterator>& __y)
    { return !(__x == __y); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator<(const move_iterator<_Iterator>& __x,
       const move_iterator<_Iterator>& __y)
    { return __x.base() < __y.base(); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator<=(const move_iterator<_Iterator>& __x,
        const move_iterator<_Iterator>& __y)
    { return !(__y < __x); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator>(const move_iterator<_Iterator>& __x,
       const move_iterator<_Iterator>& __y)
    { return __y < __x; }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr bool
    operator>=(const move_iterator<_Iterator>& __x,
        const move_iterator<_Iterator>& __y)
    { return !(__x < __y); }
  template<typename _IteratorL, typename _IteratorR>
    [[__nodiscard__]]
    inline constexpr auto
    operator-(const move_iterator<_IteratorL>& __x,
       const move_iterator<_IteratorR>& __y)
    -> decltype(__x.base() - __y.base())
    { return __x.base() - __y.base(); }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr move_iterator<_Iterator>
    operator+(typename move_iterator<_Iterator>::difference_type __n,
       const move_iterator<_Iterator>& __x)
    { return __x + __n; }
  template<typename _Iterator>
    [[__nodiscard__]]
    inline constexpr move_iterator<_Iterator>
    make_move_iterator(_Iterator __i)
    { return move_iterator<_Iterator>(std::move(__i)); }
  template<typename _Iterator, typename _ReturnType
    = __conditional_t<__move_if_noexcept_cond
      <typename iterator_traits<_Iterator>::value_type>::value,
  _Iterator, move_iterator<_Iterator>>>
    [[__nodiscard__]]
    constexpr _ReturnType
    __make_move_if_noexcept_iterator(_Iterator __i)
    { return _ReturnType(__i); }
  template<typename _Tp, typename _ReturnType
    = __conditional_t<__move_if_noexcept_cond<_Tp>::value,
        const _Tp*, move_iterator<_Tp*>>>
    [[__nodiscard__]]
    constexpr _ReturnType
    __make_move_if_noexcept_iterator(_Tp* __i)
    { return _ReturnType(__i); }
  template<typename _Iterator>
    struct __is_move_iterator<move_iterator<_Iterator> >
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

}
namespace __gnu_debug
{
  template<typename _Iterator, typename _Sequence, typename _Category>
    class _Safe_iterator;
}
namespace std __attribute__ ((__visibility__ ("default")))
{

  template<typename _Iterator, typename _Container>
    [[__nodiscard__]] __attribute__((__always_inline__))
   
    inline _Iterator
    __niter_base(__gnu_cxx::__normal_iterator<_Iterator, _Container> __it)
    noexcept(std::is_nothrow_copy_constructible<_Iterator>::value)
    { return __it.base(); }
  template<typename _Iterator>
    [[__nodiscard__]] __attribute__((__always_inline__))
   
    inline _Iterator
    __niter_base(_Iterator __it)
    noexcept(std::is_nothrow_copy_constructible<_Iterator>::value)
    { return __it; }
  template<typename _Ite, typename _Seq>
   
    decltype(std::__niter_base(std::declval<_Ite>()))
    __niter_base(const ::__gnu_debug::_Safe_iterator<_Ite, _Seq,
   std::random_access_iterator_tag>&)
    noexcept(std::is_nothrow_copy_constructible<_Ite>::value);
  template<typename _Iterator>
   
    inline auto
    __niter_base(reverse_iterator<_Iterator> __it)
    -> decltype(__make_reverse_iterator(__niter_base(__it.base())))
    { return __make_reverse_iterator(__niter_base(__it.base())); }
  template<typename _Iterator>
   
    inline auto
    __niter_base(move_iterator<_Iterator> __it)
    -> decltype(make_move_iterator(__niter_base(__it.base())))
    { return make_move_iterator(__niter_base(__it.base())); }
  template<typename _Iterator>
   
    inline auto
    __miter_base(reverse_iterator<_Iterator> __it)
    -> decltype(__make_reverse_iterator(__miter_base(__it.base())))
    { return __make_reverse_iterator(__miter_base(__it.base())); }
  template<typename _Iterator>
   
    inline auto
    __miter_base(move_iterator<_Iterator> __it)
    -> decltype(__miter_base(__it.base()))
    { return __miter_base(__it.base()); }
  template<typename _From, typename _To>
    [[__nodiscard__]]
   
    inline _From
    __niter_wrap(_From __from, _To __res)
    { return __from + (std::__niter_base(__res) - std::__niter_base(__from)); }
  template<typename _Iterator>
    [[__nodiscard__]] __attribute__((__always_inline__))
   
    inline _Iterator
    __niter_wrap(const _Iterator&, _Iterator __res)
    { return __res; }
  template<typename _InputIterator>
    using __iter_key_t = __remove_cvref_t<
      typename iterator_traits<_InputIterator>::value_type::first_type>;
  template<typename _InputIterator>
    using __iter_val_t = __remove_cvref_t<
      typename iterator_traits<_InputIterator>::value_type::second_type>;
  template<typename _T1, typename _T2>
    struct pair;
  template<typename _InputIterator>
    using __iter_to_alloc_t
      = pair<const __iter_key_t<_InputIterator>, __iter_val_t<_InputIterator>>;

}
namespace std
{
  namespace __debug { }
}
namespace __gnu_debug
{
  using namespace std::__debug;
  template<typename _Ite, typename _Seq, typename _Cat>
    class _Safe_iterator;
}
namespace std __attribute__ ((__visibility__ ("default")))
{

  template<typename _Arg, typename _Result>
    struct unary_function
    {
      typedef _Arg argument_type;
      typedef _Result result_type;
    } __attribute__ ((__deprecated__));
  template<typename _Arg1, typename _Arg2, typename _Result>
    struct binary_function
    {
      typedef _Arg1 first_argument_type;
      typedef _Arg2 second_argument_type;
      typedef _Result result_type;
    } __attribute__ ((__deprecated__));
  struct __is_transparent;
  template<typename _Tp = void>
    struct plus;
  template<typename _Tp = void>
    struct minus;
  template<typename _Tp = void>
    struct multiplies;
  template<typename _Tp = void>
    struct divides;
  template<typename _Tp = void>
    struct modulus;
  template<typename _Tp = void>
    struct negate;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template<typename _Tp>
    struct plus : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x + __y; }
    };
  template<typename _Tp>
    struct minus : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x - __y; }
    };
  template<typename _Tp>
    struct multiplies : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x * __y; }
    };
  template<typename _Tp>
    struct divides : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x / __y; }
    };
  template<typename _Tp>
    struct modulus : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x % __y; }
    };
  template<typename _Tp>
    struct negate : public unary_function<_Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x) const
      { return -__x; }
    };
#pragma GCC diagnostic pop
  template<>
    struct plus<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) + std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) + std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) + std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct minus<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) - std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) - std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) - std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct multiplies<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) * std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) * std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) * std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct divides<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) / std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) / std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) / std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct modulus<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) % std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) % std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) % std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct negate<void>
    {
      template <typename _Tp>
 constexpr
 auto
 operator()(_Tp&& __t) const
 noexcept(noexcept(-std::forward<_Tp>(__t)))
 -> decltype(-std::forward<_Tp>(__t))
 { return -std::forward<_Tp>(__t); }
      typedef __is_transparent is_transparent;
    };
  template<typename _Tp = void>
    struct equal_to;
  template<typename _Tp = void>
    struct not_equal_to;
  template<typename _Tp = void>
    struct greater;
  template<typename _Tp = void>
    struct less;
  template<typename _Tp = void>
    struct greater_equal;
  template<typename _Tp = void>
    struct less_equal;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template<typename _Tp>
    struct equal_to : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x == __y; }
    };
  template<typename _Tp>
    struct not_equal_to : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x != __y; }
    };
  template<typename _Tp>
    struct greater : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x > __y; }
    };
  template<typename _Tp>
    struct less : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x < __y; }
    };
  template<typename _Tp>
    struct greater_equal : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x >= __y; }
    };
  template<typename _Tp>
    struct less_equal : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x <= __y; }
    };
  template<typename _Tp>
    struct greater<_Tp*> : public binary_function<_Tp*, _Tp*, bool>
    {
      constexpr bool
      operator()(_Tp* __x, _Tp* __y) const noexcept
      {
 if (std::__is_constant_evaluated())
   return __x > __y;
 return (long unsigned int)__x > (long unsigned int)__y;
      }
    };
  template<typename _Tp>
    struct less<_Tp*> : public binary_function<_Tp*, _Tp*, bool>
    {
      constexpr bool
      operator()(_Tp* __x, _Tp* __y) const noexcept
      {
 if (std::__is_constant_evaluated())
   return __x < __y;
 return (long unsigned int)__x < (long unsigned int)__y;
      }
    };
  template<typename _Tp>
    struct greater_equal<_Tp*> : public binary_function<_Tp*, _Tp*, bool>
    {
      constexpr bool
      operator()(_Tp* __x, _Tp* __y) const noexcept
      {
 if (std::__is_constant_evaluated())
   return __x >= __y;
 return (long unsigned int)__x >= (long unsigned int)__y;
      }
    };
  template<typename _Tp>
    struct less_equal<_Tp*> : public binary_function<_Tp*, _Tp*, bool>
    {
      constexpr bool
      operator()(_Tp* __x, _Tp* __y) const noexcept
      {
 if (std::__is_constant_evaluated())
   return __x <= __y;
 return (long unsigned int)__x <= (long unsigned int)__y;
      }
    };
#pragma GCC diagnostic pop
  template<>
    struct equal_to<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) == std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) == std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) == std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct not_equal_to<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) != std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) != std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) != std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct greater<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) > std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) > std::forward<_Up>(__u))
 {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
   if constexpr (__ptr_cmp<_Tp, _Up>)
     return greater<const volatile void*>{}(
       static_cast<const volatile void*>(std::forward<_Tp>(__t)),
       static_cast<const volatile void*>(std::forward<_Up>(__u)));
   else
     return std::forward<_Tp>(__t) > std::forward<_Up>(__u);
#pragma GCC diagnostic pop
 }
      template<typename _Tp, typename _Up>
 constexpr bool
 operator()(_Tp* __t, _Up* __u) const noexcept
 { return greater<common_type_t<_Tp*, _Up*>>{}(__t, __u); }
      typedef __is_transparent is_transparent;
    private:
      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded2 : true_type { };
      template<typename _Tp, typename _Up>
 struct __not_overloaded2<_Tp, _Up, __void_t<
   decltype(std::declval<_Tp>().operator>(std::declval<_Up>()))>>
 : false_type { };
      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded : __not_overloaded2<_Tp, _Up> { };
      template<typename _Tp, typename _Up>
 struct __not_overloaded<_Tp, _Up, __void_t<
   decltype(operator>(std::declval<_Tp>(), std::declval<_Up>()))>>
 : false_type { };
      template<typename _Tp, typename _Up>
 static constexpr bool __ptr_cmp = __and_<
   __not_overloaded<_Tp, _Up>,
   is_convertible<_Tp, const volatile void*>,
   is_convertible<_Up, const volatile void*>>::value;
    };
  template<>
    struct less<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) < std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) < std::forward<_Up>(__u))
 {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
   if constexpr (__ptr_cmp<_Tp, _Up>)
     return less<const volatile void*>{}(
       static_cast<const volatile void*>(std::forward<_Tp>(__t)),
       static_cast<const volatile void*>(std::forward<_Up>(__u)));
   else
     return std::forward<_Tp>(__t) < std::forward<_Up>(__u);
#pragma GCC diagnostic pop
 }
      template<typename _Tp, typename _Up>
 constexpr bool
 operator()(_Tp* __t, _Up* __u) const noexcept
 { return less<common_type_t<_Tp*, _Up*>>{}(__t, __u); }
      typedef __is_transparent is_transparent;
    private:
      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded2 : true_type { };
      template<typename _Tp, typename _Up>
 struct __not_overloaded2<_Tp, _Up, __void_t<
   decltype(std::declval<_Tp>().operator<(std::declval<_Up>()))>>
 : false_type { };
      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded : __not_overloaded2<_Tp, _Up> { };
      template<typename _Tp, typename _Up>
 struct __not_overloaded<_Tp, _Up, __void_t<
   decltype(operator<(std::declval<_Tp>(), std::declval<_Up>()))>>
 : false_type { };
      template<typename _Tp, typename _Up>
 static constexpr bool __ptr_cmp = __and_<
   __not_overloaded<_Tp, _Up>,
   is_convertible<_Tp, const volatile void*>,
   is_convertible<_Up, const volatile void*>>::value;
    };
  template<>
    struct greater_equal<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) >= std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) >= std::forward<_Up>(__u))
 {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
   if constexpr (__ptr_cmp<_Tp, _Up>)
     return greater_equal<const volatile void*>{}(
       static_cast<const volatile void*>(std::forward<_Tp>(__t)),
       static_cast<const volatile void*>(std::forward<_Up>(__u)));
   else
     return std::forward<_Tp>(__t) >= std::forward<_Up>(__u);
#pragma GCC diagnostic pop
 }
      template<typename _Tp, typename _Up>
 constexpr bool
 operator()(_Tp* __t, _Up* __u) const noexcept
 { return greater_equal<common_type_t<_Tp*, _Up*>>{}(__t, __u); }
      typedef __is_transparent is_transparent;
    private:
      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded2 : true_type { };
      template<typename _Tp, typename _Up>
 struct __not_overloaded2<_Tp, _Up, __void_t<
   decltype(std::declval<_Tp>().operator>=(std::declval<_Up>()))>>
 : false_type { };
      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded : __not_overloaded2<_Tp, _Up> { };
      template<typename _Tp, typename _Up>
 struct __not_overloaded<_Tp, _Up, __void_t<
   decltype(operator>=(std::declval<_Tp>(), std::declval<_Up>()))>>
 : false_type { };
      template<typename _Tp, typename _Up>
 static constexpr bool __ptr_cmp = __and_<
   __not_overloaded<_Tp, _Up>,
   is_convertible<_Tp, const volatile void*>,
   is_convertible<_Up, const volatile void*>>::value;
    };
  template<>
    struct less_equal<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) <= std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) <= std::forward<_Up>(__u))
 {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
   if constexpr (__ptr_cmp<_Tp, _Up>)
     return less_equal<const volatile void*>{}(
       static_cast<const volatile void*>(std::forward<_Tp>(__t)),
       static_cast<const volatile void*>(std::forward<_Up>(__u)));
   else
     return std::forward<_Tp>(__t) <= std::forward<_Up>(__u);
#pragma GCC diagnostic pop
 }
      template<typename _Tp, typename _Up>
 constexpr bool
 operator()(_Tp* __t, _Up* __u) const noexcept
 { return less_equal<common_type_t<_Tp*, _Up*>>{}(__t, __u); }
      typedef __is_transparent is_transparent;
    private:
      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded2 : true_type { };
      template<typename _Tp, typename _Up>
 struct __not_overloaded2<_Tp, _Up, __void_t<
   decltype(std::declval<_Tp>().operator<=(std::declval<_Up>()))>>
 : false_type { };
      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded : __not_overloaded2<_Tp, _Up> { };
      template<typename _Tp, typename _Up>
 struct __not_overloaded<_Tp, _Up, __void_t<
   decltype(operator<=(std::declval<_Tp>(), std::declval<_Up>()))>>
 : false_type { };
      template<typename _Tp, typename _Up>
 static constexpr bool __ptr_cmp = __and_<
   __not_overloaded<_Tp, _Up>,
   is_convertible<_Tp, const volatile void*>,
   is_convertible<_Up, const volatile void*>>::value;
    };
  template<typename _Tp = void>
    struct logical_and;
  template<typename _Tp = void>
    struct logical_or;
  template<typename _Tp = void>
    struct logical_not;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template<typename _Tp>
    struct logical_and : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x && __y; }
    };
  template<typename _Tp>
    struct logical_or : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x || __y; }
    };
  template<typename _Tp>
    struct logical_not : public unary_function<_Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x) const
      { return !__x; }
    };
#pragma GCC diagnostic pop
  template<>
    struct logical_and<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) && std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) && std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) && std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct logical_or<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) || std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) || std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) || std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template<>
    struct logical_not<void>
    {
      template <typename _Tp>
 constexpr
 auto
 operator()(_Tp&& __t) const
 noexcept(noexcept(!std::forward<_Tp>(__t)))
 -> decltype(!std::forward<_Tp>(__t))
 { return !std::forward<_Tp>(__t); }
      typedef __is_transparent is_transparent;
    };
  template<typename _Tp = void>
    struct bit_and;
  template<typename _Tp = void>
    struct bit_or;
  template<typename _Tp = void>
    struct bit_xor;
  template<typename _Tp = void>
    struct bit_not;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template<typename _Tp>
    struct bit_and : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x & __y; }
    };
  template<typename _Tp>
    struct bit_or : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x | __y; }
    };
  template<typename _Tp>
    struct bit_xor : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x ^ __y; }
    };
  template<typename _Tp>
    struct bit_not : public unary_function<_Tp, _Tp>
    {
    constexpr
      _Tp
      operator()(const _Tp& __x) const
      { return ~__x; }
    };
#pragma GCC diagnostic pop
  template <>
    struct bit_and<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) & std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) & std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) & std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template <>
    struct bit_or<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) | std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) | std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) | std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template <>
    struct bit_xor<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) ^ std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) ^ std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) ^ std::forward<_Up>(__u); }
      typedef __is_transparent is_transparent;
    };
  template <>
    struct bit_not<void>
    {
      template <typename _Tp>
 constexpr
 auto
 operator()(_Tp&& __t) const
 noexcept(noexcept(~std::forward<_Tp>(__t)))
 -> decltype(~std::forward<_Tp>(__t))
 { return ~std::forward<_Tp>(__t); }
      typedef __is_transparent is_transparent;
    };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template<typename _Predicate>
    class [[__deprecated__]] unary_negate
    : public unary_function<typename _Predicate::argument_type, bool>
    {
    protected:
      _Predicate _M_pred;
    public:
      constexpr
      explicit
      unary_negate(const _Predicate& __x) : _M_pred(__x) { }
      constexpr
      bool
      operator()(const typename _Predicate::argument_type& __x) const
      { return !_M_pred(__x); }
    };
  template<typename _Predicate>
    __attribute__ ((__deprecated__ ("use '" "std::not_fn" "' instead")))
    constexpr
    inline unary_negate<_Predicate>
    not1(const _Predicate& __pred)
    { return unary_negate<_Predicate>(__pred); }
  template<typename _Predicate>
    class [[__deprecated__]] binary_negate
    : public binary_function<typename _Predicate::first_argument_type,
        typename _Predicate::second_argument_type, bool>
    {
    protected:
      _Predicate _M_pred;
    public:
      constexpr
      explicit
      binary_negate(const _Predicate& __x) : _M_pred(__x) { }
      constexpr
      bool
      operator()(const typename _Predicate::first_argument_type& __x,
   const typename _Predicate::second_argument_type& __y) const
      { return !_M_pred(__x, __y); }
    };
  template<typename _Predicate>
    __attribute__ ((__deprecated__ ("use '" "std::not_fn" "' instead")))
    constexpr
    inline binary_negate<_Predicate>
    not2(const _Predicate& __pred)
    { return binary_negate<_Predicate>(__pred); }
  template<typename _Arg, typename _Result>
    class pointer_to_unary_function : public unary_function<_Arg, _Result>
    {
    protected:
      _Result (*_M_ptr)(_Arg);
    public:
      pointer_to_unary_function() { }
      explicit
      pointer_to_unary_function(_Result (*__x)(_Arg))
      : _M_ptr(__x) { }
      _Result
      operator()(_Arg __x) const
      { return _M_ptr(__x); }
    } __attribute__ ((__deprecated__));
  template<typename _Arg, typename _Result>
    __attribute__ ((__deprecated__ ("use '" "std::function" "' instead")))
    inline pointer_to_unary_function<_Arg, _Result>
    ptr_fun(_Result (*__x)(_Arg))
    { return pointer_to_unary_function<_Arg, _Result>(__x); }
  template<typename _Arg1, typename _Arg2, typename _Result>
    class pointer_to_binary_function
    : public binary_function<_Arg1, _Arg2, _Result>
    {
    protected:
      _Result (*_M_ptr)(_Arg1, _Arg2);
    public:
      pointer_to_binary_function() { }
      explicit
      pointer_to_binary_function(_Result (*__x)(_Arg1, _Arg2))
      : _M_ptr(__x) { }
      _Result
      operator()(_Arg1 __x, _Arg2 __y) const
      { return _M_ptr(__x, __y); }
    } __attribute__ ((__deprecated__));
  template<typename _Arg1, typename _Arg2, typename _Result>
    __attribute__ ((__deprecated__ ("use '" "std::function" "' instead")))
    inline pointer_to_binary_function<_Arg1, _Arg2, _Result>
    ptr_fun(_Result (*__x)(_Arg1, _Arg2))
    { return pointer_to_binary_function<_Arg1, _Arg2, _Result>(__x); }
  template<typename _Tp>
    struct _Identity
    : public unary_function<_Tp, _Tp>
    {
      _Tp&
      operator()(_Tp& __x) const
      { return __x; }
      const _Tp&
      operator()(const _Tp& __x) const
      { return __x; }
    };
  template<typename _Tp> struct _Identity<const _Tp> : _Identity<_Tp> { };
  template<typename _Pair>
    struct _Select1st
    : public unary_function<_Pair, typename _Pair::first_type>
    {
      typename _Pair::first_type&
      operator()(_Pair& __x) const
      { return __x.first; }
      const typename _Pair::first_type&
      operator()(const _Pair& __x) const
      { return __x.first; }
      template<typename _Pair2>
        typename _Pair2::first_type&
        operator()(_Pair2& __x) const
        { return __x.first; }
      template<typename _Pair2>
        const typename _Pair2::first_type&
        operator()(const _Pair2& __x) const
        { return __x.first; }
    };
  template<typename _Pair>
    struct _Select2nd
    : public unary_function<_Pair, typename _Pair::second_type>
    {
      typename _Pair::second_type&
      operator()(_Pair& __x) const
      { return __x.second; }
      const typename _Pair::second_type&
      operator()(const _Pair& __x) const
      { return __x.second; }
    };
  template<typename _Ret, typename _Tp>
    class mem_fun_t : public unary_function<_Tp*, _Ret>
    {
    public:
      explicit
      mem_fun_t(_Ret (_Tp::*__pf)())
      : _M_f(__pf) { }
      _Ret
      operator()(_Tp* __p) const
      { return (__p->*_M_f)(); }
    private:
      _Ret (_Tp::*_M_f)();
    } __attribute__ ((__deprecated__));
  template<typename _Ret, typename _Tp>
    class const_mem_fun_t : public unary_function<const _Tp*, _Ret>
    {
    public:
      explicit
      const_mem_fun_t(_Ret (_Tp::*__pf)() const)
      : _M_f(__pf) { }
      _Ret
      operator()(const _Tp* __p) const
      { return (__p->*_M_f)(); }
    private:
      _Ret (_Tp::*_M_f)() const;
    } __attribute__ ((__deprecated__));
  template<typename _Ret, typename _Tp>
    class mem_fun_ref_t : public unary_function<_Tp, _Ret>
    {
    public:
      explicit
      mem_fun_ref_t(_Ret (_Tp::*__pf)())
      : _M_f(__pf) { }
      _Ret
      operator()(_Tp& __r) const
      { return (__r.*_M_f)(); }
    private:
      _Ret (_Tp::*_M_f)();
    } __attribute__ ((__deprecated__));
  template<typename _Ret, typename _Tp>
    class const_mem_fun_ref_t : public unary_function<_Tp, _Ret>
    {
    public:
      explicit
      const_mem_fun_ref_t(_Ret (_Tp::*__pf)() const)
      : _M_f(__pf) { }
      _Ret
      operator()(const _Tp& __r) const
      { return (__r.*_M_f)(); }
    private:
      _Ret (_Tp::*_M_f)() const;
    } __attribute__ ((__deprecated__));
  template<typename _Ret, typename _Tp, typename _Arg>
    class mem_fun1_t : public binary_function<_Tp*, _Arg, _Ret>
    {
    public:
      explicit
      mem_fun1_t(_Ret (_Tp::*__pf)(_Arg))
      : _M_f(__pf) { }
      _Ret
      operator()(_Tp* __p, _Arg __x) const
      { return (__p->*_M_f)(__x); }
    private:
      _Ret (_Tp::*_M_f)(_Arg);
    } __attribute__ ((__deprecated__));
  template<typename _Ret, typename _Tp, typename _Arg>
    class const_mem_fun1_t : public binary_function<const _Tp*, _Arg, _Ret>
    {
    public:
      explicit
      const_mem_fun1_t(_Ret (_Tp::*__pf)(_Arg) const)
      : _M_f(__pf) { }
      _Ret
      operator()(const _Tp* __p, _Arg __x) const
      { return (__p->*_M_f)(__x); }
    private:
      _Ret (_Tp::*_M_f)(_Arg) const;
    } __attribute__ ((__deprecated__));
  template<typename _Ret, typename _Tp, typename _Arg>
    class mem_fun1_ref_t : public binary_function<_Tp, _Arg, _Ret>
    {
    public:
      explicit
      mem_fun1_ref_t(_Ret (_Tp::*__pf)(_Arg))
      : _M_f(__pf) { }
      _Ret
      operator()(_Tp& __r, _Arg __x) const
      { return (__r.*_M_f)(__x); }
    private:
      _Ret (_Tp::*_M_f)(_Arg);
    } __attribute__ ((__deprecated__));
  template<typename _Ret, typename _Tp, typename _Arg>
    class const_mem_fun1_ref_t : public binary_function<_Tp, _Arg, _Ret>
    {
    public:
      explicit
      const_mem_fun1_ref_t(_Ret (_Tp::*__pf)(_Arg) const)
      : _M_f(__pf) { }
      _Ret
      operator()(const _Tp& __r, _Arg __x) const
      { return (__r.*_M_f)(__x); }
    private:
      _Ret (_Tp::*_M_f)(_Arg) const;
    } __attribute__ ((__deprecated__));
  template<typename _Ret, typename _Tp>
    __attribute__ ((__deprecated__ ("use '" "std::mem_fn" "' instead")))
    inline mem_fun_t<_Ret, _Tp>
    mem_fun(_Ret (_Tp::*__f)())
    { return mem_fun_t<_Ret, _Tp>(__f); }
  template<typename _Ret, typename _Tp>
    __attribute__ ((__deprecated__ ("use '" "std::mem_fn" "' instead")))
    inline const_mem_fun_t<_Ret, _Tp>
    mem_fun(_Ret (_Tp::*__f)() const)
    { return const_mem_fun_t<_Ret, _Tp>(__f); }
  template<typename _Ret, typename _Tp>
    __attribute__ ((__deprecated__ ("use '" "std::mem_fn" "' instead")))
    inline mem_fun_ref_t<_Ret, _Tp>
    mem_fun_ref(_Ret (_Tp::*__f)())
    { return mem_fun_ref_t<_Ret, _Tp>(__f); }
  template<typename _Ret, typename _Tp>
    __attribute__ ((__deprecated__ ("use '" "std::mem_fn" "' instead")))
    inline const_mem_fun_ref_t<_Ret, _Tp>
    mem_fun_ref(_Ret (_Tp::*__f)() const)
    { return const_mem_fun_ref_t<_Ret, _Tp>(__f); }
  template<typename _Ret, typename _Tp, typename _Arg>
    __attribute__ ((__deprecated__ ("use '" "std::mem_fn" "' instead")))
    inline mem_fun1_t<_Ret, _Tp, _Arg>
    mem_fun(_Ret (_Tp::*__f)(_Arg))
    { return mem_fun1_t<_Ret, _Tp, _Arg>(__f); }
  template<typename _Ret, typename _Tp, typename _Arg>
    __attribute__ ((__deprecated__ ("use '" "std::mem_fn" "' instead")))
    inline const_mem_fun1_t<_Ret, _Tp, _Arg>
    mem_fun(_Ret (_Tp::*__f)(_Arg) const)
    { return const_mem_fun1_t<_Ret, _Tp, _Arg>(__f); }
  template<typename _Ret, typename _Tp, typename _Arg>
    __attribute__ ((__deprecated__ ("use '" "std::mem_fn" "' instead")))
    inline mem_fun1_ref_t<_Ret, _Tp, _Arg>
    mem_fun_ref(_Ret (_Tp::*__f)(_Arg))
    { return mem_fun1_ref_t<_Ret, _Tp, _Arg>(__f); }
  template<typename _Ret, typename _Tp, typename _Arg>
    __attribute__ ((__deprecated__ ("use '" "std::mem_fn" "' instead")))
    inline const_mem_fun1_ref_t<_Ret, _Tp, _Arg>
    mem_fun_ref(_Ret (_Tp::*__f)(_Arg) const)
    { return const_mem_fun1_ref_t<_Ret, _Tp, _Arg>(__f); }
#pragma GCC diagnostic pop
  template<typename _Func, typename _SfinaeType, typename = __void_t<>>
    struct __has_is_transparent
    { };
  template<typename _Func, typename _SfinaeType>
    struct __has_is_transparent<_Func, _SfinaeType,
    __void_t<typename _Func::is_transparent>>
    { typedef void type; };
  template<typename _Func, typename _SfinaeType>
    using __has_is_transparent_t
      = typename __has_is_transparent<_Func, _SfinaeType>::type;

}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
namespace std __attribute__ ((__visibility__ ("default")))
{

  template<typename _Operation>
    class binder1st
    : public unary_function<typename _Operation::second_argument_type,
       typename _Operation::result_type>
    {
    protected:
      _Operation op;
      typename _Operation::first_argument_type value;
    public:
      binder1st(const _Operation& __x,
  const typename _Operation::first_argument_type& __y)
      : op(__x), value(__y) { }
      typename _Operation::result_type
      operator()(const typename _Operation::second_argument_type& __x) const
      { return op(value, __x); }
      typename _Operation::result_type
      operator()(typename _Operation::second_argument_type& __x) const
      { return op(value, __x); }
    } __attribute__ ((__deprecated__ ("use '" "std::bind" "' instead")));
  template<typename _Operation, typename _Tp>
    __attribute__ ((__deprecated__ ("use '" "std::bind" "' instead")))
    inline binder1st<_Operation>
    bind1st(const _Operation& __fn, const _Tp& __x)
    {
      typedef typename _Operation::first_argument_type _Arg1_type;
      return binder1st<_Operation>(__fn, _Arg1_type(__x));
    }
  template<typename _Operation>
    class binder2nd
    : public unary_function<typename _Operation::first_argument_type,
       typename _Operation::result_type>
    {
    protected:
      _Operation op;
      typename _Operation::second_argument_type value;
    public:
      binder2nd(const _Operation& __x,
  const typename _Operation::second_argument_type& __y)
      : op(__x), value(__y) { }
      typename _Operation::result_type
      operator()(const typename _Operation::first_argument_type& __x) const
      { return op(__x, value); }
      typename _Operation::result_type
      operator()(typename _Operation::first_argument_type& __x) const
      { return op(__x, value); }
    } __attribute__ ((__deprecated__ ("use '" "std::bind" "' instead")));
  template<typename _Operation, typename _Tp>
    __attribute__ ((__deprecated__ ("use '" "std::bind" "' instead")))
    inline binder2nd<_Operation>
    bind2nd(const _Operation& __fn, const _Tp& __x)
    {
      typedef typename _Operation::second_argument_type _Arg2_type;
      return binder2nd<_Operation>(__fn, _Arg2_type(__x));
    }

}
#pragma GCC diagnostic pop
namespace __gnu_cxx
{
namespace __ops
{
  typedef std::equal_to<void> equal_to;
  typedef std::less<void> less;
  template<typename _Fn>
    using __by_ref_or_value_fn
      = std::__conditional_t<std::__or_<std::is_empty<_Fn>,
     std::is_scalar<_Fn>>::value,
        _Fn, _Fn&>;
  template<typename _Func, typename _Value, bool _Val_2nd = false>
    struct _Comp_with_val
    {
      using _Fn = __by_ref_or_value_fn<_Func>;
      explicit constexpr
      _Comp_with_val(_Fn __f, const _Value& __v)
      : _M_f(__f), _M_val(__v) { }
      [[__no_unique_address__]] _Fn _M_f;
      const _Value& _M_val;
      template<typename _Tp>
 constexpr bool
 operator()(_Tp&& __arg)
 {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
   if constexpr (_Val_2nd)
     return _M_f(__arg, _M_val);
   else
     return _M_f(_M_val, __arg);
#pragma GCC diagnostic pop
 }
    };
  template<typename _Func, typename _Value>
    using _Comp_with_val_1st = _Comp_with_val<_Func, _Value, false>;
  template<typename _Func, typename _Value>
    using _Comp_with_val_2nd = _Comp_with_val<_Func, _Value, true>;
    template<typename _Func>
    struct _Unary_negate
    {
      using _Fn = __by_ref_or_value_fn<_Func>;
      explicit constexpr
      _Unary_negate(_Fn __f) : _M_f(__f) { }
      [[__no_unique_address__]] _Fn _M_f;
      template<typename _Tp>
 constexpr bool
 operator()(_Tp&& __arg) { return !_M_f(__arg); }
    };
  template<typename _Func>
    constexpr _Unary_negate<_Func>
    not1(_Func& __f)
    { return _Unary_negate<_Func>(__f); }
  template<typename _Func, typename _Value>
    constexpr inline _Comp_with_val_1st<_Func, _Value>
    bind1st(_Func& __f, const _Value& __val)
    { return _Comp_with_val_1st<_Func, _Value>(__f, __val); }
  template<typename _Func, typename _Value>
    constexpr inline _Comp_with_val_2nd<_Func, _Value>
    bind2nd(_Func& __f, const _Value& __val)
    { return _Comp_with_val_2nd<_Func, _Value>(__f, __val); }
  template<typename _Value>
    constexpr inline _Comp_with_val_2nd<equal_to, _Value>
    __equal_to(const _Value& __val)
    { return _Comp_with_val_2nd<equal_to, _Value>(equal_to(), __val); }
}
}
namespace std __attribute__ ((__visibility__ ("default")))
{

 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
  template<typename _Tp>
    constexpr _Tp
    __rotl(_Tp __x, int __s) noexcept
    {
      constexpr auto _Nd = __gnu_cxx::__int_traits<_Tp>::__digits;
      if constexpr ((_Nd & (_Nd - 1)) == 0)
 {
   constexpr unsigned __uNd = _Nd;
   const auto __r = static_cast<unsigned>(__s);
   return (__x << (__r % __uNd)) | (__x >> ((-__r) % __uNd));
 }
      const int __r = __s % _Nd;
      if (__r == 0)
 return __x;
      else if (__r > 0)
 return (__x << __r) | (__x >> ((_Nd - __r) % _Nd));
      else
 return (__x >> -__r) | (__x << ((_Nd + __r) % _Nd));
    }
  template<typename _Tp>
    constexpr _Tp
    __rotr(_Tp __x, int __s) noexcept
    {
      constexpr auto _Nd = __gnu_cxx::__int_traits<_Tp>::__digits;
      if constexpr ((_Nd & (_Nd - 1)) == 0)
 {
   constexpr unsigned __uNd = _Nd;
   const auto __r = static_cast<unsigned>(__s);
   return (__x >> (__r % __uNd)) | (__x << ((-__r) % __uNd));
 }
      const int __r = __s % _Nd;
      if (__r == 0)
 return __x;
      else if (__r > 0)
 return (__x >> __r) | (__x << ((_Nd - __r) % _Nd));
      else
 return (__x << -__r) | (__x >> ((_Nd + __r) % _Nd));
    }
  template<typename _Tp>
    constexpr int
    __countl_zero(_Tp __x) noexcept
    {
      using __gnu_cxx::__int_traits;
      constexpr auto _Nd = __int_traits<_Tp>::__digits;
      return __builtin_clzg(__x, _Nd);
    }
  template<typename _Tp>
    constexpr int
    __countl_one(_Tp __x) noexcept
    {
      return std::__countl_zero<_Tp>((_Tp)~__x);
    }
  template<typename _Tp>
    constexpr int
    __countr_zero(_Tp __x) noexcept
    {
      using __gnu_cxx::__int_traits;
      constexpr auto _Nd = __int_traits<_Tp>::__digits;
      return __builtin_ctzg(__x, _Nd);
    }
  template<typename _Tp>
    constexpr int
    __countr_one(_Tp __x) noexcept
    {
      return std::__countr_zero((_Tp)~__x);
    }
  template<typename _Tp>
    constexpr int
    __popcount(_Tp __x) noexcept
    {
      return __builtin_popcountg(__x);
    }
  template<typename _Tp>
    constexpr bool
    __has_single_bit(_Tp __x) noexcept
    { return std::__popcount(__x) == 1; }
  template<typename _Tp>
    constexpr _Tp
    __bit_ceil(_Tp __x) noexcept
    {
      using __gnu_cxx::__int_traits;
      constexpr auto _Nd = __int_traits<_Tp>::__digits;
      if (__x == 0 || __x == 1)
        return 1;
      auto __shift_exponent = _Nd - std::__countl_zero((_Tp)(__x - 1u));
      if (!std::__is_constant_evaluated())
 {
   do { if (__builtin_expect(!bool(__shift_exponent != __int_traits<_Tp>::__digits), false)) std::__glibcxx_assert_fail("/usr/include/c++/16/bit", 358, __PRETTY_FUNCTION__, "__shift_exponent != __int_traits<_Tp>::__digits"); } while (false);
 }
      using __promoted_type = decltype(__x << 1);
      if constexpr (!is_same<__promoted_type, _Tp>::value)
 {
   const int __extra_exp = sizeof(__promoted_type) / sizeof(_Tp) / 2;
   __shift_exponent |= (__shift_exponent & _Nd) << __extra_exp;
 }
      return (_Tp)1u << __shift_exponent;
    }
  template<typename _Tp>
    constexpr _Tp
    __bit_floor(_Tp __x) noexcept
    {
      constexpr auto _Nd = __gnu_cxx::__int_traits<_Tp>::__digits;
      if (__x == 0)
        return 0;
      return (_Tp)1u << (_Nd - std::__countl_zero((_Tp)(__x >> 1)));
    }
  template<typename _Tp>
    constexpr int
    __bit_width(_Tp __x) noexcept
    {
      constexpr auto _Nd = __gnu_cxx::__int_traits<_Tp>::__digits;
      return _Nd - std::__countl_zero(__x);
    }
#pragma GCC diagnostic pop

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  template<typename _Tp, typename _Up>
    constexpr
    inline int
    __memcmp(const _Tp* __first1, const _Up* __first2, size_t __num)
    {
      static_assert(sizeof(_Tp) == sizeof(_Up), "can be compared with memcmp");
 return __builtin_memcmp(__first1, __first2, sizeof(_Tp) * __num);
    }
  template<typename _ForwardIterator1, typename _ForwardIterator2>
   
    inline void
    iter_swap(_ForwardIterator1 __a, _ForwardIterator2 __b)
    {
     
     
      swap(*__a, *__b);
    }
  template<typename _ForwardIterator1, typename _ForwardIterator2>
   
    _ForwardIterator2
    swap_ranges(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
  _ForwardIterator2 __first2)
    {
     
     
      ;
      for (; __first1 != __last1; ++__first1, (void)++__first2)
 std::iter_swap(__first1, __first2);
      return __first2;
    }
  template<typename _Tp>
    [[__nodiscard__]] constexpr
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b)
    {
     
      if (__b < __a)
 return __b;
      return __a;
    }
  template<typename _Tp>
    [[__nodiscard__]] constexpr
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b)
    {
     
      if (__a < __b)
 return __b;
      return __a;
    }
  template<typename _Tp, typename _Compare>
    [[__nodiscard__]] constexpr
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b, _Compare __comp)
    {
      if (__comp(__b, __a))
 return __b;
      return __a;
    }
  template<typename _Tp, typename _Compare>
    [[__nodiscard__]] constexpr
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b, _Compare __comp)
    {
      if (__comp(__a, __b))
 return __b;
      return __a;
    }

  template<typename _Tp, typename _Ref, typename _Ptr>
    struct _Deque_iterator;
  struct _Bit_iterator;

  template<typename _CharT>
    struct char_traits;
  template<typename _CharT, typename _Traits>
    class istreambuf_iterator;
  template<typename _CharT, typename _Traits>
    class ostreambuf_iterator;
  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
      ostreambuf_iterator<_CharT, char_traits<_CharT> > >::__type
    __copy_move_a2(_CharT*, _CharT*,
     ostreambuf_iterator<_CharT, char_traits<_CharT> >);
  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
      ostreambuf_iterator<_CharT, char_traits<_CharT> > >::__type
    __copy_move_a2(const _CharT*, const _CharT*,
     ostreambuf_iterator<_CharT, char_traits<_CharT> >);
  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
        _CharT*>::__type
    __copy_move_a2(istreambuf_iterator<_CharT, char_traits<_CharT> >,
     istreambuf_iterator<_CharT, char_traits<_CharT> >, _CharT*);
  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<
      __is_char<_CharT>::__value,
      std::_Deque_iterator<_CharT, _CharT&, _CharT*> >::__type
    __copy_move_a2(
 istreambuf_iterator<_CharT, char_traits<_CharT> >,
 istreambuf_iterator<_CharT, char_traits<_CharT> >,
 std::_Deque_iterator<_CharT, _CharT&, _CharT*>);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
  template<bool _IsMove, typename _OutIter, typename _InIter>
    __attribute__((__always_inline__))
    inline void
    __assign_one(_OutIter& __out, _InIter& __in)
    {
      if constexpr (_IsMove)
 *__out = std::move(*__in);
      else
 *__out = *__in;
    }
  template<bool _IsMove, typename _InIter, typename _Sent, typename _OutIter>
   
    inline _OutIter
    __copy_move_a2(_InIter __first, _Sent __last, _OutIter __result)
    {
      typedef __decltype(*__first) _InRef;
      typedef __decltype(*__result) _OutRef;
      if constexpr (!__is_trivially_assignable(_OutRef, _InRef))
 { }
      else if (std::__is_constant_evaluated())
 { }
      else if constexpr (__memcpyable<_OutIter, _InIter>::__value)
 {
   ptrdiff_t __n = std::distance(__first, __last);
   if (__builtin_expect(__n > 1, true))
     {
       __builtin_memmove(__result,
    __first,
    __n * sizeof(*__first));
       __result += __n;
     }
   else if (__n == 1)
     {
       std::__assign_one<_IsMove>(__result, __first);
       ++__result;
     }
   return __result;
 }
      for (; __first != __last; ++__result, (void)++__first)
 std::__assign_one<_IsMove>(__result, __first);
      return __result;
    }
#pragma GCC diagnostic pop
  template<bool _IsMove,
    typename _Tp, typename _Ref, typename _Ptr, typename _OI>
    _OI
    __copy_move_a1(std::_Deque_iterator<_Tp, _Ref, _Ptr>,
     std::_Deque_iterator<_Tp, _Ref, _Ptr>,
     _OI);
  template<bool _IsMove,
    typename _ITp, typename _IRef, typename _IPtr, typename _OTp>
    std::_Deque_iterator<_OTp, _OTp&, _OTp*>
    __copy_move_a1(std::_Deque_iterator<_ITp, _IRef, _IPtr>,
     std::_Deque_iterator<_ITp, _IRef, _IPtr>,
     std::_Deque_iterator<_OTp, _OTp&, _OTp*>);
  template<bool _IsMove, typename _II, typename _Tp>
    typename __gnu_cxx::__enable_if<
      __is_any_random_access_iter<_II>::__value,
      std::_Deque_iterator<_Tp, _Tp&, _Tp*> >::__type
    __copy_move_a1(_II, _II, std::_Deque_iterator<_Tp, _Tp&, _Tp*>);
  template<bool _IsMove, typename _II, typename _OI>
    __attribute__((__always_inline__))
   
    inline _OI
    __copy_move_a1(_II __first, _II __last, _OI __result)
    { return std::__copy_move_a2<_IsMove>(__first, __last, __result); }
  template<bool _IsMove, typename _II, typename _OI>
    __attribute__((__always_inline__))
   
    inline _OI
    __copy_move_a(_II __first, _II __last, _OI __result)
    {
      return std::__niter_wrap(__result,
  std::__copy_move_a1<_IsMove>(std::__niter_base(__first),
          std::__niter_base(__last),
          std::__niter_base(__result)));
    }
  template<bool _IsMove,
    typename _Ite, typename _Seq, typename _Cat, typename _OI>
   
    _OI
    __copy_move_a(const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
    const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
    _OI);
  template<bool _IsMove,
    typename _II, typename _Ite, typename _Seq, typename _Cat>
   
    __gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>
    __copy_move_a(_II, _II,
    const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&);
  template<bool _IsMove,
    typename _IIte, typename _ISeq, typename _ICat,
    typename _OIte, typename _OSeq, typename _OCat>
   
    ::__gnu_debug::_Safe_iterator<_OIte, _OSeq, _OCat>
    __copy_move_a(const ::__gnu_debug::_Safe_iterator<_IIte, _ISeq, _ICat>&,
    const ::__gnu_debug::_Safe_iterator<_IIte, _ISeq, _ICat>&,
    const ::__gnu_debug::_Safe_iterator<_OIte, _OSeq, _OCat>&);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
  template<typename _InputIterator, typename _Size, typename _OutputIterator>
   
    _OutputIterator
    __copy_n_a(_InputIterator __first, _Size __n, _OutputIterator __result,
        bool)
    {
      typedef __decltype(*__first) _InRef;
      typedef __decltype(*__result) _OutRef;
      if constexpr (!__is_trivially_assignable(_OutRef, _InRef))
 { }
      else if constexpr (__memcpyable<_OutputIterator,
            _InputIterator>::__value)
 {
   if (__builtin_expect(__n > 1, true))
     {
       __builtin_memmove(__result,
    __first,
    __n * sizeof(*__first));
       __result += __n;
     }
   else if (__n == 1)
     *__result++ = *__first;
   return __result;
 }
      if (__n > 0)
 {
   while (true)
     {
       *__result = *__first;
       ++__result;
       if (--__n > 0)
  ++__first;
       else
  break;
     }
 }
      return __result;
    }
#pragma GCC diagnostic pop
  template<typename _CharT, typename _Size>
    typename __gnu_cxx::__enable_if<
      __is_char<_CharT>::__value, _CharT*>::__type
    __copy_n_a(istreambuf_iterator<_CharT, char_traits<_CharT> >,
        _Size, _CharT*, bool);
  template<typename _CharT, typename _Size>
    typename __gnu_cxx::__enable_if<
      __is_char<_CharT>::__value,
      std::_Deque_iterator<_CharT, _CharT&, _CharT*> >::__type
    __copy_n_a(istreambuf_iterator<_CharT, char_traits<_CharT> >, _Size,
        std::_Deque_iterator<_CharT, _CharT&, _CharT*>,
        bool);
  template<typename _II, typename _OI>
   
    inline _OI
    copy(_II __first, _II __last, _OI __result)
    {
     
     
      ;
      return std::__copy_move_a<__is_move_iterator<_II>::__value>
      (std::__miter_base(__first), std::__miter_base(__last), __result);
    }
  template<typename _II, typename _OI>
   
    inline _OI
    move(_II __first, _II __last, _OI __result)
    {
     
     
      ;
      return std::__copy_move_a<true>(std::__miter_base(__first),
          std::__miter_base(__last), __result);
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
  template<bool _IsMove, typename _BI1, typename _BI2>
   
    inline _BI2
    __copy_move_backward_a2(_BI1 __first, _BI1 __last, _BI2 __result)
    {
      typedef __decltype(*__first) _InRef;
      typedef __decltype(*__result) _OutRef;
      if constexpr (!__is_trivially_assignable(_OutRef, _InRef))
       { }
      else if constexpr (__memcpyable<_BI2, _BI1>::__value)
 {
   ptrdiff_t __n = std::distance(__first, __last);
   std::advance(__result, -__n);
   if (__builtin_expect(__n > 1, true))
     {
       __builtin_memmove(__result,
    __first,
    __n * sizeof(*__first));
     }
   else if (__n == 1)
     std::__assign_one<_IsMove>(__result, __first);
   return __result;
 }
      while (__first != __last)
 {
   --__last;
   --__result;
   std::__assign_one<_IsMove>(__result, __last);
 }
      return __result;
    }
#pragma GCC diagnostic pop
  template<bool _IsMove, typename _BI1, typename _BI2>
    __attribute__((__always_inline__))
   
    inline _BI2
    __copy_move_backward_a1(_BI1 __first, _BI1 __last, _BI2 __result)
    { return std::__copy_move_backward_a2<_IsMove>(__first, __last, __result); }
  template<bool _IsMove,
    typename _Tp, typename _Ref, typename _Ptr, typename _OI>
    _OI
    __copy_move_backward_a1(std::_Deque_iterator<_Tp, _Ref, _Ptr>,
       std::_Deque_iterator<_Tp, _Ref, _Ptr>,
       _OI);
  template<bool _IsMove,
    typename _ITp, typename _IRef, typename _IPtr, typename _OTp>
    std::_Deque_iterator<_OTp, _OTp&, _OTp*>
    __copy_move_backward_a1(
   std::_Deque_iterator<_ITp, _IRef, _IPtr>,
   std::_Deque_iterator<_ITp, _IRef, _IPtr>,
   std::_Deque_iterator<_OTp, _OTp&, _OTp*>);
  template<bool _IsMove, typename _II, typename _Tp>
    typename __gnu_cxx::__enable_if<
      __is_any_random_access_iter<_II>::__value,
      std::_Deque_iterator<_Tp, _Tp&, _Tp*> >::__type
    __copy_move_backward_a1(_II, _II,
       std::_Deque_iterator<_Tp, _Tp&, _Tp*>);
  template<bool _IsMove, typename _II, typename _OI>
    __attribute__((__always_inline__))
   
    inline _OI
    __copy_move_backward_a(_II __first, _II __last, _OI __result)
    {
      return std::__niter_wrap(__result,
  std::__copy_move_backward_a1<_IsMove>
    (std::__niter_base(__first), std::__niter_base(__last),
     std::__niter_base(__result)));
    }
  template<bool _IsMove,
    typename _Ite, typename _Seq, typename _Cat, typename _OI>
   
    _OI
    __copy_move_backward_a(
  const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
  const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
  _OI);
  template<bool _IsMove,
    typename _II, typename _Ite, typename _Seq, typename _Cat>
   
    __gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>
    __copy_move_backward_a(_II, _II,
  const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&);
  template<bool _IsMove,
    typename _IIte, typename _ISeq, typename _ICat,
    typename _OIte, typename _OSeq, typename _OCat>
   
    ::__gnu_debug::_Safe_iterator<_OIte, _OSeq, _OCat>
    __copy_move_backward_a(
  const ::__gnu_debug::_Safe_iterator<_IIte, _ISeq, _ICat>&,
  const ::__gnu_debug::_Safe_iterator<_IIte, _ISeq, _ICat>&,
  const ::__gnu_debug::_Safe_iterator<_OIte, _OSeq, _OCat>&);
  template<typename _BI1, typename _BI2>
    __attribute__((__always_inline__))
   
    inline _BI2
    copy_backward(_BI1 __first, _BI1 __last, _BI2 __result)
    {
     
     
     
      ;
      return std::__copy_move_backward_a<__is_move_iterator<_BI1>::__value>
      (std::__miter_base(__first), std::__miter_base(__last), __result);
    }
  template<typename _BI1, typename _BI2>
    __attribute__((__always_inline__))
   
    inline _BI2
    move_backward(_BI1 __first, _BI1 __last, _BI2 __result)
    {
     
     
     
      ;
      return std::__copy_move_backward_a<true>(std::__miter_base(__first),
            std::__miter_base(__last),
            __result);
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
  template<typename _ForwardIterator, typename _Tp>
   
    inline void
    __fill_a1(_ForwardIterator __first, _ForwardIterator __last,
       const _Tp& __value)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
      const bool __load_outside_loop =
     __is_trivially_constructible(_Tp, const _Tp&)
     && __is_trivially_assignable(__decltype(*__first), const _Tp&)
     && sizeof(_Tp) <= sizeof(long long);
#pragma GCC diagnostic pop
      typedef typename __gnu_cxx::__conditional_type<__load_outside_loop,
           const _Tp,
           const _Tp&>::__type _Up;
      _Up __val(__value);
      for (; __first != __last; ++__first)
 *__first = __val;
    }
#pragma GCC diagnostic pop
  template<typename _Up, typename _Tp>
   
    inline typename
    __gnu_cxx::__enable_if<__is_byte<_Up>::__value
        && (__are_same<_Up, _Tp>::__value
       || __memcpyable_integer<_Tp>::__width),
      void>::__type
    __fill_a1(_Up* __first, _Up* __last, const _Tp& __x)
    {
      const _Up __val = __x;
      if (const size_t __len = __last - __first)
 __builtin_memset(__first, static_cast<unsigned char>(__val), __len);
    }
  template<typename _Ite, typename _Cont, typename _Tp>
    __attribute__((__always_inline__))
   
    inline void
    __fill_a1(::__gnu_cxx::__normal_iterator<_Ite, _Cont> __first,
       ::__gnu_cxx::__normal_iterator<_Ite, _Cont> __last,
       const _Tp& __value)
    { std::__fill_a1(__first.base(), __last.base(), __value); }
  template<typename _Tp, typename _VTp>
    void
    __fill_a1(const std::_Deque_iterator<_Tp, _Tp&, _Tp*>&,
       const std::_Deque_iterator<_Tp, _Tp&, _Tp*>&,
       const _VTp&);
 
  void
  __fill_a1(std::_Bit_iterator, std::_Bit_iterator,
     const bool&);
  template<typename _FIte, typename _Tp>
    __attribute__((__always_inline__))
   
    inline void
    __fill_a(_FIte __first, _FIte __last, const _Tp& __value)
    { std::__fill_a1(__first, __last, __value); }
  template<typename _Ite, typename _Seq, typename _Cat, typename _Tp>
   
    void
    __fill_a(const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
      const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
      const _Tp&);
  template<typename _ForwardIterator, typename _Tp>
    __attribute__((__always_inline__))
   
    inline void
    fill(_ForwardIterator __first, _ForwardIterator __last, const _Tp& __value)
    {
     
      ;
      std::__fill_a(__first, __last, __value);
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
  inline constexpr int
  __size_to_integer(int __n) { return __n; }
  inline constexpr unsigned
  __size_to_integer(unsigned __n) { return __n; }
  inline constexpr long
  __size_to_integer(long __n) { return __n; }
  inline constexpr unsigned long
  __size_to_integer(unsigned long __n) { return __n; }
  inline constexpr long long
  __size_to_integer(long long __n) { return __n; }
  inline constexpr unsigned long long
  __size_to_integer(unsigned long long __n) { return __n; }
  __extension__ inline constexpr __int128
  __size_to_integer(__int128 __n) { return __n; }
  __extension__ inline constexpr unsigned __int128
  __size_to_integer(unsigned __int128 __n) { return __n; }
  inline constexpr long long
  __size_to_integer(float __n) { return (long long)__n; }
  inline constexpr long long
  __size_to_integer(double __n) { return (long long)__n; }
  inline constexpr long long
  __size_to_integer(long double __n) { return (long long)__n; }
  __extension__ inline constexpr long long
  __size_to_integer(__float128 __n) { return (long long)__n; }
#pragma GCC diagnostic pop
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
#pragma GCC diagnostic ignored "-Wlong-long"
  template<typename _OutputIterator, typename _Size, typename _Tp>
   
    inline _OutputIterator
    __fill_n_a1(_OutputIterator __first, _Size __n, const _Tp& __value)
    {
      const bool __load_outside_loop =
     __is_trivially_constructible(_Tp, const _Tp&)
     && __is_trivially_assignable(__decltype(*__first), const _Tp&)
     && sizeof(_Tp) <= sizeof(long long);
      typedef typename __gnu_cxx::__conditional_type<__load_outside_loop,
           const _Tp,
           const _Tp&>::__type _Up;
      _Up __val(__value);
      for (; __n > 0; --__n, (void) ++__first)
 *__first = __val;
      return __first;
    }
#pragma GCC diagnostic pop
  template<typename _Ite, typename _Seq, typename _Cat, typename _Size,
    typename _Tp>
   
    ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>
    __fill_n_a(const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>& __first,
        _Size __n, const _Tp& __value,
        std::input_iterator_tag);
  template<typename _OutputIterator, typename _Size, typename _Tp>
    __attribute__((__always_inline__))
   
    inline _OutputIterator
    __fill_n_a(_OutputIterator __first, _Size __n, const _Tp& __value,
        std::output_iterator_tag)
    {
      static_assert(is_integral<_Size>{}, "fill_n must pass integral size");
      return __fill_n_a1(__first, __n, __value);
    }
  template<typename _OutputIterator, typename _Size, typename _Tp>
    __attribute__((__always_inline__))
   
    inline _OutputIterator
    __fill_n_a(_OutputIterator __first, _Size __n, const _Tp& __value,
        std::input_iterator_tag)
    {
      static_assert(is_integral<_Size>{}, "fill_n must pass integral size");
      return __fill_n_a1(__first, __n, __value);
    }
  template<typename _OutputIterator, typename _Size, typename _Tp>
    __attribute__((__always_inline__))
   
    inline _OutputIterator
    __fill_n_a(_OutputIterator __first, _Size __n, const _Tp& __value,
        std::random_access_iterator_tag)
    {
      static_assert(is_integral<_Size>{}, "fill_n must pass integral size");
      if (__n <= 0)
 return __first;
      typename iterator_traits<_OutputIterator>::difference_type __d = __n;
      ;
      _OutputIterator __last = __first + __d;
      std::__fill_a(__first, __last, __value);
      return __last;
    }
  template<typename _OI, typename _Size, typename _Tp>
    __attribute__((__always_inline__))
   
    inline _OI
    fill_n(_OI __first, _Size __n, const _Tp& __value)
    {
     
      return std::__fill_n_a(__first, std::__size_to_integer(__n), __value,
          std::__iterator_category(__first));
    }
  template<bool _BoolType>
    struct __equal
    {
      template<typename _II1, typename _II2>

 static bool
 equal(_II1 __first1, _II1 __last1, _II2 __first2)
 {
   for (; __first1 != __last1; ++__first1, (void) ++__first2)
     if (!(*__first1 == *__first2))
       return false;
   return true;
 }
    };
  template<>
    struct __equal<true>
    {
      template<typename _Tp>

 static bool
 equal(const _Tp* __first1, const _Tp* __last1, const _Tp* __first2)
 {
   if (const size_t __len = (__last1 - __first1))
     return !std::__memcmp(__first1, __first2, __len);
   return true;
 }
    };
  template<typename _Tp, typename _Ref, typename _Ptr, typename _II>
    typename __gnu_cxx::__enable_if<
      __is_any_random_access_iter<_II>::__value, bool>::__type
    __equal_aux1(std::_Deque_iterator<_Tp, _Ref, _Ptr>,
   std::_Deque_iterator<_Tp, _Ref, _Ptr>,
   _II);
  template<typename _Tp1, typename _Ref1, typename _Ptr1,
    typename _Tp2, typename _Ref2, typename _Ptr2>
    bool
    __equal_aux1(std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
   std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
   std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>);
  template<typename _II, typename _Tp, typename _Ref, typename _Ptr>
    typename __gnu_cxx::__enable_if<
      __is_any_random_access_iter<_II>::__value, bool>::__type
    __equal_aux1(_II, _II,
  std::_Deque_iterator<_Tp, _Ref, _Ptr>);
  template<typename _II1, typename _II2>
   
    inline bool
    __equal_aux1(_II1 __first1, _II1 __last1, _II2 __first2)
    {
      typedef typename iterator_traits<_II1>::value_type _ValueType1;
      const bool __simple = ((__is_integer<_ValueType1>::__value
    || __is_pointer(_ValueType1)
    || is_same_v<_ValueType1, byte>
        ) && __memcmpable<_II1, _II2>::__value);
      return std::__equal<__simple>::equal(__first1, __last1, __first2);
    }
  template<typename _II1, typename _II2>
    __attribute__((__always_inline__))
   
    inline bool
    __equal_aux(_II1 __first1, _II1 __last1, _II2 __first2)
    {
      return std::__equal_aux1(std::__niter_base(__first1),
          std::__niter_base(__last1),
          std::__niter_base(__first2));
    }
  template<typename _II1, typename _Seq1, typename _Cat1, typename _II2>
   
    bool
    __equal_aux(const ::__gnu_debug::_Safe_iterator<_II1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_II1, _Seq1, _Cat1>&,
  _II2);
  template<typename _II1, typename _II2, typename _Seq2, typename _Cat2>
   
    bool
    __equal_aux(_II1, _II1,
  const ::__gnu_debug::_Safe_iterator<_II2, _Seq2, _Cat2>&);
  template<typename _II1, typename _Seq1, typename _Cat1,
    typename _II2, typename _Seq2, typename _Cat2>
   
    bool
    __equal_aux(const ::__gnu_debug::_Safe_iterator<_II1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_II1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_II2, _Seq2, _Cat2>&);
  template<typename, typename>
    struct __lc_rai
    {
      template<typename _II1, typename _II2>

 static _II1
 __newlast1(_II1, _II1 __last1, _II2, _II2)
 { return __last1; }
      template<typename _II>

 static bool
 __cnd2(_II __first, _II __last)
 { return __first != __last; }
    };
  template<>
    struct __lc_rai<random_access_iterator_tag, random_access_iterator_tag>
    {
      template<typename _RAI1, typename _RAI2>

 static _RAI1
 __newlast1(_RAI1 __first1, _RAI1 __last1,
     _RAI2 __first2, _RAI2 __last2)
 {
   typedef typename iterator_traits<_RAI1>::difference_type _Diff1;
   typedef typename iterator_traits<_RAI2>::difference_type _Diff2;
   const _Diff1 __diff1 = __last1 - __first1;
   const _Diff2 __diff2 = __last2 - __first2;
   return __diff2 < __diff1 ? __first1 + _Diff1(__diff2) : __last1;
 }
      template<typename _RAI>
 static bool
 __cnd2(_RAI, _RAI)
 { return true; }
    };
  template<typename _II1, typename _II2, typename _Compare>
   
    bool
    __lexicographical_compare_impl(_II1 __first1, _II1 __last1,
       _II2 __first2, _II2 __last2,
       _Compare __comp)
    {
      typedef __decltype(std::__iter_concept_or_category<_II1>()) _Category1;
      typedef __decltype(std::__iter_concept_or_category<_II2>()) _Category2;
      typedef std::__lc_rai<_Category1, _Category2> __rai_type;
      __last1 = __rai_type::__newlast1(__first1, __last1, __first2, __last2);
      for (; __first1 != __last1 && __rai_type::__cnd2(__first2, __last2);
    ++__first1, (void)++__first2)
 {
   if (__comp(*__first1, *__first2))
     return true;
   if (__comp(*__first2, *__first1))
     return false;
 }
      return __first1 == __last1 && __first2 != __last2;
    }
  template<bool _BoolType>
    struct __lexicographical_compare
    {
      template<typename _II1, typename _II2>

 static bool
 __lc(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
 {
   using __gnu_cxx::__ops::less;
   return std::__lexicographical_compare_impl(__first1, __last1,
           __first2, __last2,
           less());
 }
      template<typename _II1, typename _II2>

 static int
 __3way(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
 {
   while (__first1 != __last1)
     {
       if (__first2 == __last2)
  return +1;
       if (*__first1 < *__first2)
  return -1;
       if (*__first2 < *__first1)
  return +1;
       ++__first1;
       ++__first2;
     }
   return int(__first2 == __last2) - 1;
 }
    };
  template<>
    struct __lexicographical_compare<true>
    {
      template<typename _Tp, typename _Up>

 static bool
 __lc(const _Tp* __first1, const _Tp* __last1,
      const _Up* __first2, const _Up* __last2)
 { return __3way(__first1, __last1, __first2, __last2) < 0; }
      template<typename _Tp, typename _Up>

 static ptrdiff_t
 __3way(const _Tp* __first1, const _Tp* __last1,
        const _Up* __first2, const _Up* __last2)
 {
   const size_t __len1 = __last1 - __first1;
   const size_t __len2 = __last2 - __first2;
   if (const size_t __len = std::min(__len1, __len2))
     if (int __result = std::__memcmp(__first1, __first2, __len))
       return __result;
   return ptrdiff_t(__len1 - __len2);
 }
    };
  template<typename _II1, typename _II2>
   
    inline bool
    __lexicographical_compare_aux1(_II1 __first1, _II1 __last1,
       _II2 __first2, _II2 __last2)
    {
      typedef typename iterator_traits<_II1>::value_type _ValueType1;
      typedef typename iterator_traits<_II2>::value_type _ValueType2;
      const bool __simple =
 (__is_memcmp_ordered_with<_ValueType1, _ValueType2>::__value
  && __is_pointer(_II1) && __is_pointer(_II2)
  );
      return std::__lexicographical_compare<__simple>::__lc(__first1, __last1,
           __first2, __last2);
    }
  template<typename _Tp1, typename _Ref1, typename _Ptr1,
    typename _Tp2>
    bool
    __lexicographical_compare_aux1(
 std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
 std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
 _Tp2*, _Tp2*);
  template<typename _Tp1,
    typename _Tp2, typename _Ref2, typename _Ptr2>
    bool
    __lexicographical_compare_aux1(_Tp1*, _Tp1*,
 std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>,
 std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>);
  template<typename _Tp1, typename _Ref1, typename _Ptr1,
    typename _Tp2, typename _Ref2, typename _Ptr2>
    bool
    __lexicographical_compare_aux1(
 std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
 std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
 std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>,
 std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>);
  template<typename _II1, typename _II2>
   
    inline bool
    __lexicographical_compare_aux(_II1 __first1, _II1 __last1,
      _II2 __first2, _II2 __last2)
    {
      return std::__lexicographical_compare_aux1(std::__niter_base(__first1),
       std::__niter_base(__last1),
       std::__niter_base(__first2),
       std::__niter_base(__last2));
    }
  template<typename _Iter1, typename _Seq1, typename _Cat1,
    typename _II2>
   
    bool
    __lexicographical_compare_aux(
  const ::__gnu_debug::_Safe_iterator<_Iter1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_Iter1, _Seq1, _Cat1>&,
  _II2, _II2);
  template<typename _II1,
    typename _Iter2, typename _Seq2, typename _Cat2>
   
    bool
    __lexicographical_compare_aux(
  _II1, _II1,
  const ::__gnu_debug::_Safe_iterator<_Iter2, _Seq2, _Cat2>&,
  const ::__gnu_debug::_Safe_iterator<_Iter2, _Seq2, _Cat2>&);
  template<typename _Iter1, typename _Seq1, typename _Cat1,
    typename _Iter2, typename _Seq2, typename _Cat2>
   
    bool
    __lexicographical_compare_aux(
  const ::__gnu_debug::_Safe_iterator<_Iter1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_Iter1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_Iter2, _Seq2, _Cat2>&,
  const ::__gnu_debug::_Safe_iterator<_Iter2, _Seq2, _Cat2>&);
  template<typename _ForwardIterator, typename _Tp, typename _Compare>
   
    _ForwardIterator
    __lower_bound(_ForwardIterator __first, _ForwardIterator __last,
    const _Tp& __val, _Compare __comp)
    {
      typedef typename iterator_traits<_ForwardIterator>::difference_type
 _DistanceType;
      _DistanceType __len = std::distance(__first, __last);
      while (__len > 0)
 {
   _DistanceType __half = __len >> 1;
   _ForwardIterator __middle = __first;
   std::advance(__middle, __half);
   if (__comp(*__middle, __val))
     {
       __first = __middle;
       ++__first;
       __len = __len - __half - 1;
     }
   else
     __len = __half;
 }
      return __first;
    }
  template<typename _ForwardIterator, typename _Tp>
    [[__nodiscard__]]
    inline _ForwardIterator
    lower_bound(_ForwardIterator __first, _ForwardIterator __last,
  const _Tp& __val)
    {
     
     
      ;
      return std::__lower_bound(__first, __last, __val,
    __gnu_cxx::__ops::less());
    }
  template<typename _Tp>
    inline constexpr _Tp
    __lg(_Tp __n)
    {
      return std::__bit_width(make_unsigned_t<_Tp>(__n)) - 1;
    }

  template<typename _II1, typename _II2>
    [[__nodiscard__]]
    inline bool
    equal(_II1 __first1, _II1 __last1, _II2 __first2)
    {
     
     
     
      ;
      return std::__equal_aux(__first1, __last1, __first2);
    }
  template<typename _IIter1, typename _IIter2, typename _BinaryPredicate>
    [[__nodiscard__]]
    inline bool
    equal(_IIter1 __first1, _IIter1 __last1,
   _IIter2 __first2, _BinaryPredicate __binary_pred)
    {
     
     
      ;
      for (; __first1 != __last1; ++__first1, (void)++__first2)
 if (!bool(__binary_pred(*__first1, *__first2)))
   return false;
      return true;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++17-extensions"
  template<typename _II1, typename _II2>
   
    inline bool
    __equal4(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
    {
      using _RATag = random_access_iterator_tag;
      using _Cat1 = decltype(std::__iter_concept_or_category<_II1>());
      using _Cat2 = decltype(std::__iter_concept_or_category<_II2>());
      using _RAIters = __and_<is_same<_Cat1, _RATag>, is_same<_Cat2, _RATag>>;
      if constexpr (_RAIters::value)
 {
   if ((__last1 - __first1) != (__last2 - __first2))
     return false;
   return std::equal(__first1, __last1, __first2);
 }
      else
 {
   for (; __first1 != __last1 && __first2 != __last2;
        ++__first1, (void)++__first2)
     if (!(*__first1 == *__first2))
       return false;
   return __first1 == __last1 && __first2 == __last2;
 }
    }
  template<typename _II1, typename _II2, typename _BinaryPredicate>
   
    inline bool
    __equal4(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2,
      _BinaryPredicate __binary_pred)
    {
      using _RATag = random_access_iterator_tag;
      using _Cat1 = decltype(std::__iter_concept_or_category<_II1>());
      using _Cat2 = decltype(std::__iter_concept_or_category<_II2>());
      using _RAIters = __and_<is_same<_Cat1, _RATag>, is_same<_Cat2, _RATag>>;
      if constexpr (_RAIters::value)
 {
   if ((__last1 - __first1) != (__last2 - __first2))
     return false;
   return std::equal(__first1, __last1, __first2,
           __binary_pred);
 }
      else
 {
   for (; __first1 != __last1 && __first2 != __last2;
        ++__first1, (void)++__first2)
     if (!bool(__binary_pred(*__first1, *__first2)))
       return false;
   return __first1 == __last1 && __first2 == __last2;
 }
    }
#pragma GCC diagnostic pop
  template<typename _II1, typename _II2>
    [[__nodiscard__]]
    inline bool
    equal(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
    {
     
     
     
      ;
      ;
      return std::__equal4(__first1, __last1, __first2, __last2);
    }
  template<typename _IIter1, typename _IIter2, typename _BinaryPredicate>
    [[__nodiscard__]]
    inline bool
    equal(_IIter1 __first1, _IIter1 __last1,
   _IIter2 __first2, _IIter2 __last2, _BinaryPredicate __binary_pred)
    {
     
     
      ;
      ;
      return std::__equal4(__first1, __last1, __first2, __last2,
          __binary_pred);
    }
  template<typename _II1, typename _II2>
    [[__nodiscard__]]
    inline bool
    lexicographical_compare(_II1 __first1, _II1 __last1,
       _II2 __first2, _II2 __last2)
    {
     
     
     
     
      ;
      ;
      return std::__lexicographical_compare_aux(__first1, __last1,
      __first2, __last2);
    }
  template<typename _II1, typename _II2, typename _Compare>
    [[__nodiscard__]]
    inline bool
    lexicographical_compare(_II1 __first1, _II1 __last1,
       _II2 __first2, _II2 __last2, _Compare __comp)
    {
     
     
      ;
      ;
      return std::__lexicographical_compare_impl
 (__first1, __last1, __first2, __last2, __comp);
    }
  template<typename _InputIterator1, typename _InputIterator2,
    typename _BinaryPredicate>
   
    pair<_InputIterator1, _InputIterator2>
    __mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
        _InputIterator2 __first2, _BinaryPredicate __binary_pred)
    {
      while (__first1 != __last1 && __binary_pred(*__first1, *__first2))
 {
   ++__first1;
   ++__first2;
 }
      return pair<_InputIterator1, _InputIterator2>(__first1, __first2);
    }
  template<typename _InputIterator1, typename _InputIterator2>
    [[__nodiscard__]]
    inline pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
      _InputIterator2 __first2)
    {
     
     
     
      ;
      return std::__mismatch(__first1, __last1, __first2,
     __gnu_cxx::__ops::equal_to());
    }
  template<typename _InputIterator1, typename _InputIterator2,
    typename _BinaryPredicate>
    [[__nodiscard__]]
    inline pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
      _InputIterator2 __first2, _BinaryPredicate __binary_pred)
    {
     
     
      ;
      return std::__mismatch(__first1, __last1, __first2,
     __binary_pred);
    }
  template<typename _InputIterator1, typename _InputIterator2,
    typename _BinaryPredicate>
   
    pair<_InputIterator1, _InputIterator2>
    __mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
        _InputIterator2 __first2, _InputIterator2 __last2,
        _BinaryPredicate __binary_pred)
    {
      while (__first1 != __last1 && __first2 != __last2
      && __binary_pred(*__first1, *__first2))
 {
   ++__first1;
   ++__first2;
 }
      return pair<_InputIterator1, _InputIterator2>(__first1, __first2);
    }
  template<typename _InputIterator1, typename _InputIterator2>
    [[__nodiscard__]]
    inline pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
      _InputIterator2 __first2, _InputIterator2 __last2)
    {
     
     
     
      ;
      ;
      return std::__mismatch(__first1, __last1, __first2, __last2,
     __gnu_cxx::__ops::equal_to());
    }
  template<typename _InputIterator1, typename _InputIterator2,
    typename _BinaryPredicate>
    [[__nodiscard__]]
    inline pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
      _InputIterator2 __first2, _InputIterator2 __last2,
      _BinaryPredicate __binary_pred)
    {
     
     
      ;
      ;
      return std::__mismatch(__first1, __last1, __first2, __last2,
        __binary_pred);
    }

  template<typename _Iterator, typename _Predicate>
   
    inline _Iterator
    __find_if(_Iterator __first, _Iterator __last, _Predicate __pred)
    {
#pragma GCC unroll 4
      while (__first != __last && !__pred(*__first))
 ++__first;
      return __first;
    }
  template<typename _InputIterator, typename _Predicate>
   
    typename iterator_traits<_InputIterator>::difference_type
    __count_if(_InputIterator __first, _InputIterator __last, _Predicate __pred)
    {
      typename iterator_traits<_InputIterator>::difference_type __n = 0;
      for (; __first != __last; ++__first)
 if (__pred(*__first))
   ++__n;
      return __n;
    }
  template<typename _ForwardIterator, typename _Predicate>
   
    _ForwardIterator
    __remove_if(_ForwardIterator __first, _ForwardIterator __last,
  _Predicate __pred)
    {
      __first = std::__find_if(__first, __last, __pred);
      if (__first == __last)
 return __first;
      _ForwardIterator __result = __first;
      ++__first;
      for (; __first != __last; ++__first)
 if (!__pred(*__first))
   {
     *__result = std::move(*__first);
     ++__result;
   }
      return __result;
    }
  template<typename _ForwardIterator1, typename _ForwardIterator2,
    typename _BinaryPredicate>
   
    _ForwardIterator1
    __search(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
      _ForwardIterator2 __first2, _ForwardIterator2 __last2,
      _BinaryPredicate __predicate)
    {
      if (__first1 == __last1 || __first2 == __last2)
 return __first1;
      __decltype(*__first2) __first2_val(*__first2);
      __decltype(__gnu_cxx::__ops::bind2nd(__predicate, __first2_val))
 __match_first = __gnu_cxx::__ops::bind2nd(__predicate, __first2_val);
      _ForwardIterator2 __p1(__first2);
      if (++__p1 == __last2)
 return std::__find_if(__first1, __last1, __match_first);
      _ForwardIterator1 __current = __first1;
      for (;;)
 {
   __first1 = std::__find_if(__first1, __last1, __match_first);
   if (__first1 == __last1)
     return __last1;
   _ForwardIterator2 __p = __p1;
   __current = __first1;
   if (++__current == __last1)
     return __last1;
   while (__predicate(*__current, *__p))
     {
       if (++__p == __last2)
  return __first1;
       if (++__current == __last1)
  return __last1;
     }
   ++__first1;
 }
      return __first1;
    }
  template<typename _ForwardIterator1, typename _ForwardIterator2,
    typename _BinaryPredicate>
   
    bool
    __is_permutation(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
       _ForwardIterator2 __first2, _BinaryPredicate __pred)
    {
      for (; __first1 != __last1; ++__first1, (void)++__first2)
 if (!__pred(*__first1, *__first2))
   break;
      if (__first1 == __last1)
 return true;
      _ForwardIterator2 __last2 = __first2;
      std::advance(__last2, std::distance(__first1, __last1));
      for (_ForwardIterator1 __scan = __first1; __scan != __last1; ++__scan)
 {
   auto&& __scan_val = *__scan;
   auto __scaneq = __gnu_cxx::__ops::bind1st(__pred, __scan_val);
   if (__scan != std::__find_if(__first1, __scan, __scaneq))
     continue;
   auto __matches = std::__count_if(__first2, __last2, __scaneq);
   if (0 == __matches
  || std::__count_if(__scan, __last1, __scaneq) != __matches)
     return false;
 }
      return true;
    }
  template<typename _ForwardIterator1, typename _ForwardIterator2>
   
    inline bool
    is_permutation(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
     _ForwardIterator2 __first2)
    {
     
     
     
      ;
      return std::__is_permutation(__first1, __last1, __first2,
       __gnu_cxx::__ops::equal_to());
    }

  template<typename _ForwardIterator1, typename _ForwardIterator2,
    typename _BinaryPredicate>
   
    inline _ForwardIterator1
    search(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
    _ForwardIterator2 __first2, _ForwardIterator2 __last2,
    _BinaryPredicate __predicate)
    {
     
     
     
      ;
      ;
      return std::__search(__first1, __last1, __first2, __last2, __predicate);
    }


}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wc++23-extensions"
namespace std __attribute__ ((__visibility__ ("default")))
{

  enum float_round_style
  {
    round_indeterminate = -1,
    round_toward_zero = 0,
    round_to_nearest = 1,
    round_toward_infinity = 2,
    round_toward_neg_infinity = 3
  };
  enum float_denorm_style
  {
    denorm_indeterminate = -1,
    denorm_absent = 0,
    denorm_present = 1
  };
  struct __numeric_limits_base
  {
    static constexpr bool is_specialized = false;
    static constexpr int digits = 0;
    static constexpr int digits10 = 0;
    static constexpr int max_digits10 = 0;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = false;
    static constexpr int radix = 0;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = false;
    static constexpr bool is_modulo = false;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;
    static constexpr float_round_style round_style =
          round_toward_zero;
  };
  template<typename _Tp>
    struct numeric_limits : public __numeric_limits_base
    {
      static constexpr _Tp
      min() noexcept { return _Tp(); }
      static constexpr _Tp
      max() noexcept { return _Tp(); }
      static constexpr _Tp
      lowest() noexcept { return _Tp(); }
      static constexpr _Tp
      epsilon() noexcept { return _Tp(); }
      static constexpr _Tp
      round_error() noexcept { return _Tp(); }
      static constexpr _Tp
      infinity() noexcept { return _Tp(); }
      static constexpr _Tp
      quiet_NaN() noexcept { return _Tp(); }
      static constexpr _Tp
      signaling_NaN() noexcept { return _Tp(); }
      static constexpr _Tp
      denorm_min() noexcept { return _Tp(); }
    };
  template<typename _Tp>
    struct numeric_limits<const _Tp>
    : public numeric_limits<_Tp> { };
  template<typename _Tp>
    struct numeric_limits<volatile _Tp>
    : public numeric_limits<_Tp> { };
  template<typename _Tp>
    struct numeric_limits<const volatile _Tp>
    : public numeric_limits<_Tp> { };
  template<>
    struct numeric_limits<bool>
    {
      static constexpr bool is_specialized = true;
      static constexpr bool
      min() noexcept { return false; }
      static constexpr bool
      max() noexcept { return true; }
      static constexpr bool
      lowest() noexcept { return min(); }
      static constexpr int digits = 1;
      static constexpr int digits10 = 0;
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = false;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr bool
      epsilon() noexcept { return false; }
      static constexpr bool
      round_error() noexcept { return false; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr bool
      infinity() noexcept { return false; }
      static constexpr bool
      quiet_NaN() noexcept { return false; }
      static constexpr bool
      signaling_NaN() noexcept { return false; }
      static constexpr bool
      denorm_min() noexcept { return false; }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<char>
    {
      static constexpr bool is_specialized = true;
      static constexpr char
      min() noexcept { return (((char)(-1) < 0) ? -(((char)(-1) < 0) ? (((((char)1 << ((sizeof(char) * 8 - ((char)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(char)0) - 1 : (char)0); }
      static constexpr char
      max() noexcept { return (((char)(-1) < 0) ? (((((char)1 << ((sizeof(char) * 8 - ((char)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(char)0); }
      static constexpr char
      lowest() noexcept { return min(); }
      static constexpr int digits = (sizeof(char) * 8 - ((char)(-1) < 0));
      static constexpr int digits10 = ((sizeof(char) * 8 - ((char)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = ((char)(-1) < 0);
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr char
      epsilon() noexcept { return 0; }
      static constexpr char
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr
      char infinity() noexcept { return char(); }
      static constexpr char
      quiet_NaN() noexcept { return char(); }
      static constexpr char
      signaling_NaN() noexcept { return char(); }
      static constexpr char
      denorm_min() noexcept { return static_cast<char>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = !is_signed;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<signed char>
    {
      static constexpr bool is_specialized = true;
      static constexpr signed char
      min() noexcept { return -0x7f - 1; }
      static constexpr signed char
      max() noexcept { return 0x7f; }
      static constexpr signed char
      lowest() noexcept { return min(); }
      static constexpr int digits = (sizeof(signed char) * 8 - ((signed char)(-1) < 0));
      static constexpr int digits10
       = ((sizeof(signed char) * 8 - ((signed char)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr signed char
      epsilon() noexcept { return 0; }
      static constexpr signed char
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr signed char
      infinity() noexcept { return static_cast<signed char>(0); }
      static constexpr signed char
      quiet_NaN() noexcept { return static_cast<signed char>(0); }
      static constexpr signed char
      signaling_NaN() noexcept
      { return static_cast<signed char>(0); }
      static constexpr signed char
      denorm_min() noexcept
      { return static_cast<signed char>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<unsigned char>
    {
      static constexpr bool is_specialized = true;
      static constexpr unsigned char
      min() noexcept { return 0; }
      static constexpr unsigned char
      max() noexcept { return 0x7f * 2U + 1; }
      static constexpr unsigned char
      lowest() noexcept { return min(); }
      static constexpr int digits
       = (sizeof(unsigned char) * 8 - ((unsigned char)(-1) < 0));
      static constexpr int digits10
       = ((sizeof(unsigned char) * 8 - ((unsigned char)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = false;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr unsigned char
      epsilon() noexcept { return 0; }
      static constexpr unsigned char
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr unsigned char
      infinity() noexcept
      { return static_cast<unsigned char>(0); }
      static constexpr unsigned char
      quiet_NaN() noexcept
      { return static_cast<unsigned char>(0); }
      static constexpr unsigned char
      signaling_NaN() noexcept
      { return static_cast<unsigned char>(0); }
      static constexpr unsigned char
      denorm_min() noexcept
      { return static_cast<unsigned char>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = true;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<wchar_t>
    {
      static constexpr bool is_specialized = true;
      static constexpr wchar_t
      min() noexcept { return (((wchar_t)(-1) < 0) ? -(((wchar_t)(-1) < 0) ? (((((wchar_t)1 << ((sizeof(wchar_t) * 8 - ((wchar_t)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(wchar_t)0) - 1 : (wchar_t)0); }
      static constexpr wchar_t
      max() noexcept { return (((wchar_t)(-1) < 0) ? (((((wchar_t)1 << ((sizeof(wchar_t) * 8 - ((wchar_t)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(wchar_t)0); }
      static constexpr wchar_t
      lowest() noexcept { return min(); }
      static constexpr int digits = (sizeof(wchar_t) * 8 - ((wchar_t)(-1) < 0));
      static constexpr int digits10
       = ((sizeof(wchar_t) * 8 - ((wchar_t)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = ((wchar_t)(-1) < 0);
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr wchar_t
      epsilon() noexcept { return 0; }
      static constexpr wchar_t
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr wchar_t
      infinity() noexcept { return wchar_t(); }
      static constexpr wchar_t
      quiet_NaN() noexcept { return wchar_t(); }
      static constexpr wchar_t
      signaling_NaN() noexcept { return wchar_t(); }
      static constexpr wchar_t
      denorm_min() noexcept { return wchar_t(); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = !is_signed;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<char16_t>
    {
      static constexpr bool is_specialized = true;
      static constexpr char16_t
      min() noexcept { return (((char16_t)(-1) < 0) ? -(((char16_t)(-1) < 0) ? (((((char16_t)1 << ((sizeof(char16_t) * 8 - ((char16_t)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(char16_t)0) - 1 : (char16_t)0); }
      static constexpr char16_t
      max() noexcept { return (((char16_t)(-1) < 0) ? (((((char16_t)1 << ((sizeof(char16_t) * 8 - ((char16_t)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(char16_t)0); }
      static constexpr char16_t
      lowest() noexcept { return min(); }
      static constexpr int digits = (sizeof(char16_t) * 8 - ((char16_t)(-1) < 0));
      static constexpr int digits10 = ((sizeof(char16_t) * 8 - ((char16_t)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = ((char16_t)(-1) < 0);
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr char16_t
      epsilon() noexcept { return 0; }
      static constexpr char16_t
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr char16_t
      infinity() noexcept { return char16_t(); }
      static constexpr char16_t
      quiet_NaN() noexcept { return char16_t(); }
      static constexpr char16_t
      signaling_NaN() noexcept { return char16_t(); }
      static constexpr char16_t
      denorm_min() noexcept { return char16_t(); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = !is_signed;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style = round_toward_zero;
    };
  template<>
    struct numeric_limits<char32_t>
    {
      static constexpr bool is_specialized = true;
      static constexpr char32_t
      min() noexcept { return (((char32_t)(-1) < 0) ? -(((char32_t)(-1) < 0) ? (((((char32_t)1 << ((sizeof(char32_t) * 8 - ((char32_t)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(char32_t)0) - 1 : (char32_t)0); }
      static constexpr char32_t
      max() noexcept { return (((char32_t)(-1) < 0) ? (((((char32_t)1 << ((sizeof(char32_t) * 8 - ((char32_t)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(char32_t)0); }
      static constexpr char32_t
      lowest() noexcept { return min(); }
      static constexpr int digits = (sizeof(char32_t) * 8 - ((char32_t)(-1) < 0));
      static constexpr int digits10 = ((sizeof(char32_t) * 8 - ((char32_t)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = ((char32_t)(-1) < 0);
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr char32_t
      epsilon() noexcept { return 0; }
      static constexpr char32_t
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr char32_t
      infinity() noexcept { return char32_t(); }
      static constexpr char32_t
      quiet_NaN() noexcept { return char32_t(); }
      static constexpr char32_t
      signaling_NaN() noexcept { return char32_t(); }
      static constexpr char32_t
      denorm_min() noexcept { return char32_t(); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = !is_signed;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style = round_toward_zero;
    };
  template<>
    struct numeric_limits<short>
    {
      static constexpr bool is_specialized = true;
      static constexpr short
      min() noexcept { return -0x7fff - 1; }
      static constexpr short
      max() noexcept { return 0x7fff; }
      static constexpr short
      lowest() noexcept { return min(); }
      static constexpr int digits = (sizeof(short) * 8 - ((short)(-1) < 0));
      static constexpr int digits10 = ((sizeof(short) * 8 - ((short)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr short
      epsilon() noexcept { return 0; }
      static constexpr short
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr short
      infinity() noexcept { return short(); }
      static constexpr short
      quiet_NaN() noexcept { return short(); }
      static constexpr short
      signaling_NaN() noexcept { return short(); }
      static constexpr short
      denorm_min() noexcept { return short(); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<unsigned short>
    {
      static constexpr bool is_specialized = true;
      static constexpr unsigned short
      min() noexcept { return 0; }
      static constexpr unsigned short
      max() noexcept { return 0x7fff * 2U + 1; }
      static constexpr unsigned short
      lowest() noexcept { return min(); }
      static constexpr int digits
       = (sizeof(unsigned short) * 8 - ((unsigned short)(-1) < 0));
      static constexpr int digits10
       = ((sizeof(unsigned short) * 8 - ((unsigned short)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = false;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr unsigned short
      epsilon() noexcept { return 0; }
      static constexpr unsigned short
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr unsigned short
      infinity() noexcept
      { return static_cast<unsigned short>(0); }
      static constexpr unsigned short
      quiet_NaN() noexcept
      { return static_cast<unsigned short>(0); }
      static constexpr unsigned short
      signaling_NaN() noexcept
      { return static_cast<unsigned short>(0); }
      static constexpr unsigned short
      denorm_min() noexcept
      { return static_cast<unsigned short>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = true;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<int>
    {
      static constexpr bool is_specialized = true;
      static constexpr int
      min() noexcept { return -0x7fffffff - 1; }
      static constexpr int
      max() noexcept { return 0x7fffffff; }
      static constexpr int
      lowest() noexcept { return min(); }
      static constexpr int digits = (sizeof(int) * 8 - ((int)(-1) < 0));
      static constexpr int digits10 = ((sizeof(int) * 8 - ((int)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr int
      epsilon() noexcept { return 0; }
      static constexpr int
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr int
      infinity() noexcept { return static_cast<int>(0); }
      static constexpr int
      quiet_NaN() noexcept { return static_cast<int>(0); }
      static constexpr int
      signaling_NaN() noexcept { return static_cast<int>(0); }
      static constexpr int
      denorm_min() noexcept { return static_cast<int>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<unsigned int>
    {
      static constexpr bool is_specialized = true;
      static constexpr unsigned int
      min() noexcept { return 0; }
      static constexpr unsigned int
      max() noexcept { return 0x7fffffff * 2U + 1; }
      static constexpr unsigned int
      lowest() noexcept { return min(); }
      static constexpr int digits
       = (sizeof(unsigned int) * 8 - ((unsigned int)(-1) < 0));
      static constexpr int digits10
       = ((sizeof(unsigned int) * 8 - ((unsigned int)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = false;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr unsigned int
      epsilon() noexcept { return 0; }
      static constexpr unsigned int
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr unsigned int
      infinity() noexcept { return static_cast<unsigned int>(0); }
      static constexpr unsigned int
      quiet_NaN() noexcept
      { return static_cast<unsigned int>(0); }
      static constexpr unsigned int
      signaling_NaN() noexcept
      { return static_cast<unsigned int>(0); }
      static constexpr unsigned int
      denorm_min() noexcept
      { return static_cast<unsigned int>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = true;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<long>
    {
      static constexpr bool is_specialized = true;
      static constexpr long
      min() noexcept { return -0x7fffffffffffffffL - 1; }
      static constexpr long
      max() noexcept { return 0x7fffffffffffffffL; }
      static constexpr long
      lowest() noexcept { return min(); }
      static constexpr int digits = (sizeof(long) * 8 - ((long)(-1) < 0));
      static constexpr int digits10 = ((sizeof(long) * 8 - ((long)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr long
      epsilon() noexcept { return 0; }
      static constexpr long
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr long
      infinity() noexcept { return static_cast<long>(0); }
      static constexpr long
      quiet_NaN() noexcept { return static_cast<long>(0); }
      static constexpr long
      signaling_NaN() noexcept { return static_cast<long>(0); }
      static constexpr long
      denorm_min() noexcept { return static_cast<long>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<unsigned long>
    {
      static constexpr bool is_specialized = true;
      static constexpr unsigned long
      min() noexcept { return 0; }
      static constexpr unsigned long
      max() noexcept { return 0x7fffffffffffffffL * 2UL + 1; }
      static constexpr unsigned long
      lowest() noexcept { return min(); }
      static constexpr int digits
       = (sizeof(unsigned long) * 8 - ((unsigned long)(-1) < 0));
      static constexpr int digits10
       = ((sizeof(unsigned long) * 8 - ((unsigned long)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = false;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr unsigned long
      epsilon() noexcept { return 0; }
      static constexpr unsigned long
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr unsigned long
      infinity() noexcept
      { return static_cast<unsigned long>(0); }
      static constexpr unsigned long
      quiet_NaN() noexcept
      { return static_cast<unsigned long>(0); }
      static constexpr unsigned long
      signaling_NaN() noexcept
      { return static_cast<unsigned long>(0); }
      static constexpr unsigned long
      denorm_min() noexcept
      { return static_cast<unsigned long>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = true;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<long long>
    {
      static constexpr bool is_specialized = true;
      static constexpr long long
      min() noexcept { return -0x7fffffffffffffffLL - 1; }
      static constexpr long long
      max() noexcept { return 0x7fffffffffffffffLL; }
      static constexpr long long
      lowest() noexcept { return min(); }
      static constexpr int digits
       = (sizeof(long long) * 8 - ((long long)(-1) < 0));
      static constexpr int digits10
       = ((sizeof(long long) * 8 - ((long long)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr long long
      epsilon() noexcept { return 0; }
      static constexpr long long
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr long long
      infinity() noexcept { return static_cast<long long>(0); }
      static constexpr long long
      quiet_NaN() noexcept { return static_cast<long long>(0); }
      static constexpr long long
      signaling_NaN() noexcept
      { return static_cast<long long>(0); }
      static constexpr long long
      denorm_min() noexcept { return static_cast<long long>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  template<>
    struct numeric_limits<unsigned long long>
    {
      static constexpr bool is_specialized = true;
      static constexpr unsigned long long
      min() noexcept { return 0; }
      static constexpr unsigned long long
      max() noexcept { return 0x7fffffffffffffffLL * 2ULL + 1; }
      static constexpr unsigned long long
      lowest() noexcept { return min(); }
      static constexpr int digits
       = (sizeof(unsigned long long) * 8 - ((unsigned long long)(-1) < 0));
      static constexpr int digits10
       = ((sizeof(unsigned long long) * 8 - ((unsigned long long)(-1) < 0)) * 643L / 2136);
      static constexpr int max_digits10 = 0;
      static constexpr bool is_signed = false;
      static constexpr bool is_integer = true;
      static constexpr bool is_exact = true;
      static constexpr int radix = 2;
      static constexpr unsigned long long
      epsilon() noexcept { return 0; }
      static constexpr unsigned long long
      round_error() noexcept { return 0; }
      static constexpr int min_exponent = 0;
      static constexpr int min_exponent10 = 0;
      static constexpr int max_exponent = 0;
      static constexpr int max_exponent10 = 0;
      static constexpr bool has_infinity = false;
      static constexpr bool has_quiet_NaN = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr float_denorm_style has_denorm
       = denorm_absent;
      static constexpr bool has_denorm_loss = false;
      static constexpr unsigned long long
      infinity() noexcept
      { return static_cast<unsigned long long>(0); }
      static constexpr unsigned long long
      quiet_NaN() noexcept
      { return static_cast<unsigned long long>(0); }
      static constexpr unsigned long long
      signaling_NaN() noexcept
      { return static_cast<unsigned long long>(0); }
      static constexpr unsigned long long
      denorm_min() noexcept
      { return static_cast<unsigned long long>(0); }
      static constexpr bool is_iec559 = false;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = true;
      static constexpr bool traps = true;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
       = round_toward_zero;
    };
  __extension__ template<> struct numeric_limits<__int128> { static constexpr bool is_specialized = true; static constexpr __int128 min() noexcept { return (((__int128)(-1) < 0) ? -(((__int128)(-1) < 0) ? (((((__int128)1 << ((128 - ((__int128)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(__int128)0) - 1 : (__int128)0); } static constexpr __int128 max() noexcept { return (((__int128)(-1) < 0) ? (((((__int128)1 << ((128 - ((__int128)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(__int128)0); } static constexpr int digits = 128 - 1; static constexpr int digits10 = (128 - 1) * 643L / 2136; static constexpr bool is_signed = true; static constexpr bool is_integer = true; static constexpr bool is_exact = true; static constexpr int radix = 2; static constexpr __int128 epsilon() noexcept { return 0; } static constexpr __int128 round_error() noexcept { return 0; } static constexpr __int128 lowest() noexcept { return min(); } static constexpr int max_digits10 = 0; static constexpr int min_exponent = 0; static constexpr int min_exponent10 = 0; static constexpr int max_exponent = 0; static constexpr int max_exponent10 = 0; static constexpr bool has_infinity = false; static constexpr bool has_quiet_NaN = false; static constexpr bool has_signaling_NaN = false; static constexpr float_denorm_style has_denorm = denorm_absent; static constexpr bool has_denorm_loss = false; static constexpr __int128 infinity() noexcept { return static_cast<__int128>(0); } static constexpr __int128 quiet_NaN() noexcept { return static_cast<__int128>(0); } static constexpr __int128 signaling_NaN() noexcept { return static_cast<__int128>(0); } static constexpr __int128 denorm_min() noexcept { return static_cast<__int128>(0); } static constexpr bool is_iec559 = false; static constexpr bool is_bounded = true; static constexpr bool is_modulo = false; static constexpr bool traps = true; static constexpr bool tinyness_before = false; static constexpr float_round_style round_style = round_toward_zero; }; __extension__ template<> struct numeric_limits<unsigned __int128> { static constexpr bool is_specialized = true; static constexpr unsigned __int128 min() noexcept { return 0; } static constexpr unsigned __int128 max() noexcept { return (((unsigned __int128)(-1) < 0) ? (((((unsigned __int128)1 << ((128 - ((unsigned __int128)(-1) < 0)) - 1)) - 1) << 1) + 1) : ~(unsigned __int128)0); } static constexpr unsigned __int128 lowest() noexcept { return min(); } static constexpr int max_digits10 = 0; static constexpr int digits = 128; static constexpr int digits10 = 128 * 643L / 2136; static constexpr bool is_signed = false; static constexpr bool is_integer = true; static constexpr bool is_exact = true; static constexpr int radix = 2; static constexpr unsigned __int128 epsilon() noexcept { return 0; } static constexpr unsigned __int128 round_error() noexcept { return 0; } static constexpr int min_exponent = 0; static constexpr int min_exponent10 = 0; static constexpr int max_exponent = 0; static constexpr int max_exponent10 = 0; static constexpr bool has_infinity = false; static constexpr bool has_quiet_NaN = false; static constexpr bool has_signaling_NaN = false; static constexpr float_denorm_style has_denorm = denorm_absent; static constexpr bool has_denorm_loss = false; static constexpr unsigned __int128 infinity() noexcept { return static_cast<unsigned __int128>(0); } static constexpr unsigned __int128 quiet_NaN() noexcept { return static_cast<unsigned __int128>(0); } static constexpr unsigned __int128 signaling_NaN() noexcept { return static_cast<unsigned __int128>(0); } static constexpr unsigned __int128 denorm_min() noexcept { return static_cast<unsigned __int128>(0); } static constexpr bool is_iec559 = false; static constexpr bool is_bounded = true; static constexpr bool is_modulo = true; static constexpr bool traps = true; static constexpr bool tinyness_before = false; static constexpr float_round_style round_style = round_toward_zero; };
  template<>
    struct numeric_limits<float>
    {
      static constexpr bool is_specialized = true;
      static constexpr float
      min() noexcept { return 1.17549435082228750796873653722224568e-38F; }
      static constexpr float
      max() noexcept { return 3.40282346638528859811704183484516925e+38F; }
      static constexpr float
      lowest() noexcept { return -3.40282346638528859811704183484516925e+38F; }
      static constexpr int digits = 24;
      static constexpr int digits10 = 6;
      static constexpr int max_digits10
  = (2 + (24) * 643L / 2136);
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = false;
      static constexpr bool is_exact = false;
      static constexpr int radix = 2;
      static constexpr float
      epsilon() noexcept { return 1.19209289550781250000000000000000000e-7F; }
      static constexpr float
      round_error() noexcept { return 0.5F; }
      static constexpr int min_exponent = (-125);
      static constexpr int min_exponent10 = (-37);
      static constexpr int max_exponent = 128;
      static constexpr int max_exponent10 = 38;
      static constexpr bool has_infinity = 1;
      static constexpr bool has_quiet_NaN = 1;
      static constexpr bool has_signaling_NaN = has_quiet_NaN;
      static constexpr float_denorm_style has_denorm
 = bool(1) ? denorm_present : denorm_absent;
      static constexpr bool has_denorm_loss
       = false;
      static constexpr float
      infinity() noexcept { return __builtin_huge_valf(); }
      static constexpr float
      quiet_NaN() noexcept { return __builtin_nanf(""); }
      static constexpr float
      signaling_NaN() noexcept { return __builtin_nansf(""); }
      static constexpr float
      denorm_min() noexcept { return 1.40129846432481707092372958328991613e-45F; }
      static constexpr bool is_iec559
 = has_infinity && has_quiet_NaN && has_denorm == denorm_present;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = false;
      static constexpr bool tinyness_before
       = false;
      static constexpr float_round_style round_style
       = round_to_nearest;
    };
  template<>
    struct numeric_limits<double>
    {
      static constexpr bool is_specialized = true;
      static constexpr double
      min() noexcept { return double(2.22507385850720138309023271733240406e-308L); }
      static constexpr double
      max() noexcept { return double(1.79769313486231570814527423731704357e+308L); }
      static constexpr double
      lowest() noexcept { return -double(1.79769313486231570814527423731704357e+308L); }
      static constexpr int digits = 53;
      static constexpr int digits10 = 15;
      static constexpr int max_digits10
  = (2 + (53) * 643L / 2136);
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = false;
      static constexpr bool is_exact = false;
      static constexpr int radix = 2;
      static constexpr double
      epsilon() noexcept { return double(2.22044604925031308084726333618164062e-16L); }
      static constexpr double
      round_error() noexcept { return 0.5; }
      static constexpr int min_exponent = (-1021);
      static constexpr int min_exponent10 = (-307);
      static constexpr int max_exponent = 1024;
      static constexpr int max_exponent10 = 308;
      static constexpr bool has_infinity = 1;
      static constexpr bool has_quiet_NaN = 1;
      static constexpr bool has_signaling_NaN = has_quiet_NaN;
      static constexpr float_denorm_style has_denorm
 = bool(1) ? denorm_present : denorm_absent;
      static constexpr bool has_denorm_loss
        = false;
      static constexpr double
      infinity() noexcept { return __builtin_huge_val(); }
      static constexpr double
      quiet_NaN() noexcept { return __builtin_nan(""); }
      static constexpr double
      signaling_NaN() noexcept { return __builtin_nans(""); }
      static constexpr double
      denorm_min() noexcept { return double(4.94065645841246544176568792868221372e-324L); }
      static constexpr bool is_iec559
 = has_infinity && has_quiet_NaN && has_denorm == denorm_present;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = false;
      static constexpr bool tinyness_before
       = false;
      static constexpr float_round_style round_style
       = round_to_nearest;
    };
  template<>
    struct numeric_limits<long double>
    {
      static constexpr bool is_specialized = true;
      static constexpr long double
      min() noexcept { return 3.36210314311209350626267781732175260e-4932L; }
      static constexpr long double
      max() noexcept { return 1.18973149535723176502126385303097021e+4932L; }
      static constexpr long double
      lowest() noexcept { return -1.18973149535723176502126385303097021e+4932L; }
      static constexpr int digits = 64;
      static constexpr int digits10 = 18;
      static constexpr int max_digits10
  = (2 + (64) * 643L / 2136);
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = false;
      static constexpr bool is_exact = false;
      static constexpr int radix = 2;
      static constexpr long double
      epsilon() noexcept { return 1.08420217248550443400745280086994171e-19L; }
      static constexpr long double
      round_error() noexcept { return 0.5L; }
      static constexpr int min_exponent = (-16381);
      static constexpr int min_exponent10 = (-4931);
      static constexpr int max_exponent = 16384;
      static constexpr int max_exponent10 = 4932;
      static constexpr bool has_infinity = 1;
      static constexpr bool has_quiet_NaN = 1;
      static constexpr bool has_signaling_NaN = has_quiet_NaN;
      static constexpr float_denorm_style has_denorm
 = bool(1) ? denorm_present : denorm_absent;
      static constexpr bool has_denorm_loss
 = false;
      static constexpr long double
      infinity() noexcept { return __builtin_huge_vall(); }
      static constexpr long double
      quiet_NaN() noexcept { return __builtin_nanl(""); }
      static constexpr long double
      signaling_NaN() noexcept { return __builtin_nansl(""); }
      static constexpr long double
      denorm_min() noexcept { return 3.64519953188247460252840593361941982e-4951L; }
      static constexpr bool is_iec559
 = has_infinity && has_quiet_NaN && has_denorm == denorm_present;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = false;
      static constexpr bool tinyness_before =
      false;
      static constexpr float_round_style round_style =
            round_to_nearest;
    };
__extension__ template<> struct numeric_limits<_Float32> { static constexpr bool is_specialized = true; static constexpr _Float32 min() noexcept { return 1.17549435082228750796873653722224568e-38F32; } static constexpr _Float32 max() noexcept { return 3.40282346638528859811704183484516925e+38F32; } static constexpr _Float32 lowest() noexcept { return -3.40282346638528859811704183484516925e+38F32; } static constexpr int digits = 24; static constexpr int digits10 = 6; static constexpr int max_digits10 = (2 + (24) * 643L / 2136); static constexpr bool is_signed = true; static constexpr bool is_integer = false; static constexpr bool is_exact = false; static constexpr int radix = 2; static constexpr _Float32 epsilon() noexcept { return 1.19209289550781250000000000000000000e-7F32; } static constexpr _Float32 round_error() noexcept { return 0.5F32; } static constexpr int min_exponent = (-125); static constexpr int min_exponent10 = (-37); static constexpr int max_exponent = 128; static constexpr int max_exponent10 = 38; static constexpr bool has_infinity = 1; static constexpr bool has_quiet_NaN = 1; static constexpr bool has_signaling_NaN = has_quiet_NaN; static constexpr float_denorm_style has_denorm = bool(1) ? denorm_present : denorm_absent; static constexpr bool has_denorm_loss = false; static constexpr _Float32 infinity() noexcept { return __builtin_huge_valf32(); } static constexpr _Float32 quiet_NaN() noexcept { return __builtin_nanf32(""); } static constexpr _Float32 signaling_NaN() noexcept { return __builtin_nansf32(""); } static constexpr _Float32 denorm_min() noexcept { return 1.40129846432481707092372958328991613e-45F32; } static constexpr bool is_iec559 = has_infinity && has_quiet_NaN && has_denorm == denorm_present; static constexpr bool is_bounded = true; static constexpr bool is_modulo = false; static constexpr bool traps = false; static constexpr bool tinyness_before = false; static constexpr float_round_style round_style = round_to_nearest; };
__extension__ template<> struct numeric_limits<_Float64> { static constexpr bool is_specialized = true; static constexpr _Float64 min() noexcept { return 2.22507385850720138309023271733240406e-308F64; } static constexpr _Float64 max() noexcept { return 1.79769313486231570814527423731704357e+308F64; } static constexpr _Float64 lowest() noexcept { return -1.79769313486231570814527423731704357e+308F64; } static constexpr int digits = 53; static constexpr int digits10 = 15; static constexpr int max_digits10 = (2 + (53) * 643L / 2136); static constexpr bool is_signed = true; static constexpr bool is_integer = false; static constexpr bool is_exact = false; static constexpr int radix = 2; static constexpr _Float64 epsilon() noexcept { return 2.22044604925031308084726333618164062e-16F64; } static constexpr _Float64 round_error() noexcept { return 0.5F64; } static constexpr int min_exponent = (-1021); static constexpr int min_exponent10 = (-307); static constexpr int max_exponent = 1024; static constexpr int max_exponent10 = 308; static constexpr bool has_infinity = 1; static constexpr bool has_quiet_NaN = 1; static constexpr bool has_signaling_NaN = has_quiet_NaN; static constexpr float_denorm_style has_denorm = bool(1) ? denorm_present : denorm_absent; static constexpr bool has_denorm_loss = false; static constexpr _Float64 infinity() noexcept { return __builtin_huge_valf64(); } static constexpr _Float64 quiet_NaN() noexcept { return __builtin_nanf64(""); } static constexpr _Float64 signaling_NaN() noexcept { return __builtin_nansf64(""); } static constexpr _Float64 denorm_min() noexcept { return 4.94065645841246544176568792868221372e-324F64; } static constexpr bool is_iec559 = has_infinity && has_quiet_NaN && has_denorm == denorm_present; static constexpr bool is_bounded = true; static constexpr bool is_modulo = false; static constexpr bool traps = false; static constexpr bool tinyness_before = false; static constexpr float_round_style round_style = round_to_nearest; };
__extension__ template<> struct numeric_limits<_Float128> { static constexpr bool is_specialized = true; static constexpr _Float128 min() noexcept { return 3.36210314311209350626267781732175260e-4932F128; } static constexpr _Float128 max() noexcept { return 1.18973149535723176508575932662800702e+4932F128; } static constexpr _Float128 lowest() noexcept { return -1.18973149535723176508575932662800702e+4932F128; } static constexpr int digits = 113; static constexpr int digits10 = 33; static constexpr int max_digits10 = (2 + (113) * 643L / 2136); static constexpr bool is_signed = true; static constexpr bool is_integer = false; static constexpr bool is_exact = false; static constexpr int radix = 2; static constexpr _Float128 epsilon() noexcept { return 1.92592994438723585305597794258492732e-34F128; } static constexpr _Float128 round_error() noexcept { return 0.5F128; } static constexpr int min_exponent = (-16381); static constexpr int min_exponent10 = (-4931); static constexpr int max_exponent = 16384; static constexpr int max_exponent10 = 4932; static constexpr bool has_infinity = 1; static constexpr bool has_quiet_NaN = 1; static constexpr bool has_signaling_NaN = has_quiet_NaN; static constexpr float_denorm_style has_denorm = bool(1) ? denorm_present : denorm_absent; static constexpr bool has_denorm_loss = false; static constexpr _Float128 infinity() noexcept { return __builtin_huge_valf128(); } static constexpr _Float128 quiet_NaN() noexcept { return __builtin_nanf128(""); } static constexpr _Float128 signaling_NaN() noexcept { return __builtin_nansf128(""); } static constexpr _Float128 denorm_min() noexcept { return 6.47517511943802511092443895822764655e-4966F128; } static constexpr bool is_iec559 = has_infinity && has_quiet_NaN && has_denorm == denorm_present; static constexpr bool is_bounded = true; static constexpr bool is_modulo = false; static constexpr bool traps = false; static constexpr bool tinyness_before = false; static constexpr float_round_style round_style = round_to_nearest; };
  __extension__
  template<>
    struct numeric_limits<__float128>
    {
      static constexpr bool is_specialized = true;
      static constexpr __float128
      min() noexcept
      {
 return double(9.3132257461547852e-10) * _S_1pm16352();
      }
      static constexpr __float128
      max() noexcept
      {
 return (__float128(double(3.4028236692093843e+38))
    + double(3.7778931862957153e+22) + double(8.35584e+6))
   * _S_1p16256();
      }
      static constexpr __float128
      lowest() noexcept
      { return -max(); }
      static constexpr int digits = 113;
      static constexpr int digits10 = 33;
      static constexpr int max_digits10 = 36;
      static constexpr bool is_signed = true;
      static constexpr bool is_integer = false;
      static constexpr bool is_exact = false;
      static constexpr int radix = 2;
      static constexpr __float128
      epsilon() noexcept
      { return double(1.9259299443872359e-34); }
      static constexpr __float128
      round_error() noexcept { return 0.5; }
      static constexpr int min_exponent = -16381;
      static constexpr int min_exponent10 = -4931;
      static constexpr int max_exponent = 16384;
      static constexpr int max_exponent10 = 4932;
      static constexpr bool has_infinity = 1;
      static constexpr bool has_quiet_NaN = 1;
      static constexpr bool has_signaling_NaN = true;
      static constexpr float_denorm_style has_denorm
 = denorm_present;
      static constexpr bool has_denorm_loss = false;
      static constexpr __float128
      infinity() noexcept
      { return __builtin_huge_val(); }
      static constexpr __float128
      quiet_NaN() noexcept
      { return __builtin_nan(""); }
      static constexpr __float128
      signaling_NaN() noexcept
      {
 return __builtin_nansq("");
      }
      static constexpr __float128
      denorm_min() noexcept
      {
 return double(1.7936620343357659e-43) * _S_1pm16352();
      }
      static constexpr bool is_iec559 = has_signaling_NaN;
      static constexpr bool is_bounded = true;
      static constexpr bool is_modulo = false;
      static constexpr bool traps = false;
      static constexpr bool tinyness_before = false;
      static constexpr float_round_style round_style
 = round_to_nearest;
    private:
      static constexpr __float128
      _S_4p(__float128 __v) noexcept
      { return __v * __v * __v * __v; }
      static constexpr __float128
      _S_1pm4088() noexcept
      { return _S_4p( double(2.2250738585072014e-308)); }
      static constexpr __float128
      _S_1pm16352() noexcept
      { return _S_4p(_S_1pm4088()); }
      static constexpr __float128
      _S_1p4064() noexcept
      { return _S_4p( double(7.0222388080559215e+305)); }
      static constexpr __float128
      _S_1p16256() noexcept
      { return _S_4p(_S_1p4064()); }
    };

}
#pragma GCC diagnostic pop
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tp>
    struct __floating_point_constant
    {
      static const _Tp __value;
    };
    template<typename _Tp>
      struct __numeric_constants
      {
        static _Tp __pi() throw()
        { return static_cast<_Tp>(3.1415926535897932384626433832795029L); }
        static _Tp __pi_2() throw()
        { return static_cast<_Tp>(1.5707963267948966192313216916397514L); }
        static _Tp __pi_3() throw()
        { return static_cast<_Tp>(1.0471975511965977461542144610931676L); }
        static _Tp __pi_4() throw()
        { return static_cast<_Tp>(0.7853981633974483096156608458198757L); }
        static _Tp __1_pi() throw()
        { return static_cast<_Tp>(0.3183098861837906715377675267450287L); }
        static _Tp __2_sqrtpi() throw()
        { return static_cast<_Tp>(1.1283791670955125738961589031215452L); }
        static _Tp __sqrt2() throw()
        { return static_cast<_Tp>(1.4142135623730950488016887242096981L); }
        static _Tp __sqrt3() throw()
        { return static_cast<_Tp>(1.7320508075688772935274463415058723L); }
        static _Tp __sqrtpio2() throw()
        { return static_cast<_Tp>(1.2533141373155002512078826424055226L); }
        static _Tp __sqrt1_2() throw()
        { return static_cast<_Tp>(0.7071067811865475244008443621048490L); }
        static _Tp __lnpi() throw()
        { return static_cast<_Tp>(1.1447298858494001741434273513530587L); }
        static _Tp __gamma_e() throw()
        { return static_cast<_Tp>(0.5772156649015328606065120900824024L); }
        static _Tp __euler() throw()
        { return static_cast<_Tp>(2.7182818284590452353602874713526625L); }
      };
    template<typename _Tp>
    inline bool __isnan(_Tp __x)
    { return std::isnan(__x); }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template <typename _Tp>
    _Tp
    __bernoulli_series(unsigned int __n)
    {
      static const _Tp __num[28] = {
        _Tp(1UL), -_Tp(1UL) / _Tp(2UL),
        _Tp(1UL) / _Tp(6UL), _Tp(0UL),
        -_Tp(1UL) / _Tp(30UL), _Tp(0UL),
        _Tp(1UL) / _Tp(42UL), _Tp(0UL),
        -_Tp(1UL) / _Tp(30UL), _Tp(0UL),
        _Tp(5UL) / _Tp(66UL), _Tp(0UL),
        -_Tp(691UL) / _Tp(2730UL), _Tp(0UL),
        _Tp(7UL) / _Tp(6UL), _Tp(0UL),
        -_Tp(3617UL) / _Tp(510UL), _Tp(0UL),
        _Tp(43867UL) / _Tp(798UL), _Tp(0UL),
        -_Tp(174611) / _Tp(330UL), _Tp(0UL),
        _Tp(854513UL) / _Tp(138UL), _Tp(0UL),
        -_Tp(236364091UL) / _Tp(2730UL), _Tp(0UL),
        _Tp(8553103UL) / _Tp(6UL), _Tp(0UL)
      };
      if (__n == 0)
        return _Tp(1);
      if (__n == 1)
        return -_Tp(1) / _Tp(2);
      if (__n % 2 == 1)
        return _Tp(0);
      if (__n < 28)
        return __num[__n];
      _Tp __fact = _Tp(1);
      if ((__n / 2) % 2 == 0)
        __fact *= _Tp(-1);
      for (unsigned int __k = 1; __k <= __n; ++__k)
        __fact *= __k / (_Tp(2) * __numeric_constants<_Tp>::__pi());
      __fact *= _Tp(2);
      _Tp __sum = _Tp(0);
      for (unsigned int __i = 1; __i < 1000; ++__i)
        {
          _Tp __term = std::pow(_Tp(__i), -_Tp(__n));
          if (__term < std::numeric_limits<_Tp>::epsilon())
            break;
          __sum += __term;
        }
      return __fact * __sum;
    }
    template<typename _Tp>
    inline _Tp
    __bernoulli(int __n)
    { return __bernoulli_series<_Tp>(__n); }
    template<typename _Tp>
    _Tp
    __log_gamma_bernoulli(_Tp __x)
    {
      _Tp __lg = (__x - _Tp(0.5L)) * std::log(__x) - __x
               + _Tp(0.5L) * std::log(_Tp(2)
               * __numeric_constants<_Tp>::__pi());
      const _Tp __xx = __x * __x;
      _Tp __help = _Tp(1) / __x;
      for ( unsigned int __i = 1; __i < 20; ++__i )
        {
          const _Tp __2i = _Tp(2 * __i);
          __help /= __2i * (__2i - _Tp(1)) * __xx;
          __lg += __bernoulli<_Tp>(2 * __i) * __help;
        }
      return __lg;
    }
    template<typename _Tp>
    _Tp
    __log_gamma_lanczos(_Tp __x)
    {
      const _Tp __xm1 = __x - _Tp(1);
      static const _Tp __lanczos_cheb_7[9] = {
       _Tp( 0.99999999999980993227684700473478L),
       _Tp( 676.520368121885098567009190444019L),
       _Tp(-1259.13921672240287047156078755283L),
       _Tp( 771.3234287776530788486528258894L),
       _Tp(-176.61502916214059906584551354L),
       _Tp( 12.507343278686904814458936853L),
       _Tp(-0.13857109526572011689554707L),
       _Tp( 9.984369578019570859563e-6L),
       _Tp( 1.50563273514931155834e-7L)
      };
      static const _Tp __LOGROOT2PI
          = _Tp(0.9189385332046727417803297364056176L);
      _Tp __sum = __lanczos_cheb_7[0];
      for(unsigned int __k = 1; __k < 9; ++__k)
        __sum += __lanczos_cheb_7[__k] / (__xm1 + __k);
      const _Tp __term1 = (__xm1 + _Tp(0.5L))
                        * std::log((__xm1 + _Tp(7.5L))
                       / __numeric_constants<_Tp>::__euler());
      const _Tp __term2 = __LOGROOT2PI + std::log(__sum);
      const _Tp __result = __term1 + (__term2 - _Tp(7));
      return __result;
    }
    template<typename _Tp>
    _Tp
    __log_gamma(_Tp __x)
    {
      if (__x > _Tp(0.5L))
        return __log_gamma_lanczos(__x);
      else
        {
          const _Tp __sin_fact
                 = std::abs(std::sin(__numeric_constants<_Tp>::__pi() * __x));
          if (__sin_fact == _Tp(0))
            std::__throw_domain_error(("Argument is nonpositive integer " "in __log_gamma"));
          return __numeric_constants<_Tp>::__lnpi()
                     - std::log(__sin_fact)
                     - __log_gamma_lanczos(_Tp(1) - __x);
        }
    }
    template<typename _Tp>
    _Tp
    __log_gamma_sign(_Tp __x)
    {
      if (__x > _Tp(0))
        return _Tp(1);
      else
        {
          const _Tp __sin_fact
                  = std::sin(__numeric_constants<_Tp>::__pi() * __x);
          if (__sin_fact > _Tp(0))
            return (1);
          else if (__sin_fact < _Tp(0))
            return -_Tp(1);
          else
            return _Tp(0);
        }
    }
    template<typename _Tp>
    _Tp
    __log_bincoef(unsigned int __n, unsigned int __k)
    {
      static const _Tp __max_bincoeff
                      = std::numeric_limits<_Tp>::max_exponent10
                      * std::log(_Tp(10)) - _Tp(1);
      _Tp __coeff = ::std::lgamma(_Tp(1 + __n))
                  - ::std::lgamma(_Tp(1 + __k))
                  - ::std::lgamma(_Tp(1 + __n - __k));
    }
    template<typename _Tp>
    _Tp
    __bincoef(unsigned int __n, unsigned int __k)
    {
      static const _Tp __max_bincoeff
                      = std::numeric_limits<_Tp>::max_exponent10
                      * std::log(_Tp(10)) - _Tp(1);
      const _Tp __log_coeff = __log_bincoef<_Tp>(__n, __k);
      if (__log_coeff > __max_bincoeff)
        return std::numeric_limits<_Tp>::quiet_NaN();
      else
        return std::exp(__log_coeff);
    }
    template<typename _Tp>
    inline _Tp
    __gamma(_Tp __x)
    { return std::exp(__log_gamma(__x)); }
    template<typename _Tp>
    _Tp
    __psi_series(_Tp __x)
    {
      _Tp __sum = -__numeric_constants<_Tp>::__gamma_e() - _Tp(1) / __x;
      const unsigned int __max_iter = 100000;
      for (unsigned int __k = 1; __k < __max_iter; ++__k)
        {
          const _Tp __term = __x / (__k * (__k + __x));
          __sum += __term;
          if (std::abs(__term / __sum) < std::numeric_limits<_Tp>::epsilon())
            break;
        }
      return __sum;
    }
    template<typename _Tp>
    _Tp
    __psi_asymp(_Tp __x)
    {
      _Tp __sum = std::log(__x) - _Tp(0.5L) / __x;
      const _Tp __xx = __x * __x;
      _Tp __xp = __xx;
      const unsigned int __max_iter = 100;
      for (unsigned int __k = 1; __k < __max_iter; ++__k)
        {
          const _Tp __term = __bernoulli<_Tp>(2 * __k) / (2 * __k * __xp);
          __sum -= __term;
          if (std::abs(__term / __sum) < std::numeric_limits<_Tp>::epsilon())
            break;
          __xp *= __xx;
        }
      return __sum;
    }
    template<typename _Tp>
    _Tp
    __psi(_Tp __x)
    {
      const int __n = static_cast<int>(__x + 0.5L);
      const _Tp __eps = _Tp(4) * std::numeric_limits<_Tp>::epsilon();
      if (__n <= 0 && std::abs(__x - _Tp(__n)) < __eps)
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__x < _Tp(0))
        {
          const _Tp __pi = __numeric_constants<_Tp>::__pi();
          return __psi(_Tp(1) - __x)
               - __pi * std::cos(__pi * __x) / std::sin(__pi * __x);
        }
      else if (__x > _Tp(100))
        return __psi_asymp(__x);
      else
        return __psi_series(__x);
    }
    template<typename _Tp>
    _Tp
    __psi(unsigned int __n, _Tp __x)
    {
      if (__x <= _Tp(0))
        std::__throw_domain_error(("Argument out of range " "in __psi"));
      else if (__n == 0)
        return __psi(__x);
      else
        {
          const _Tp __hzeta = __hurwitz_zeta(_Tp(__n + 1), __x);
          const _Tp __ln_nfact = ::std::lgamma(_Tp(__n + 1));
          _Tp __result = std::exp(__ln_nfact) * __hzeta;
          if (__n % 2 == 1)
            __result = -__result;
          return __result;
        }
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template <typename _Tp>
    void
    __gamma_temme(_Tp __mu,
                  _Tp & __gam1, _Tp & __gam2, _Tp & __gampl, _Tp & __gammi)
    {
      __gampl = _Tp(1) / ::std::tgamma(_Tp(1) + __mu);
      __gammi = _Tp(1) / ::std::tgamma(_Tp(1) - __mu);
      if (std::abs(__mu) < std::numeric_limits<_Tp>::epsilon())
        __gam1 = -_Tp(__numeric_constants<_Tp>::__gamma_e());
      else
        __gam1 = (__gammi - __gampl) / (_Tp(2) * __mu);
      __gam2 = (__gammi + __gampl) / (_Tp(2));
      return;
    }
    template <typename _Tp>
    void
    __bessel_jn(_Tp __nu, _Tp __x,
                _Tp & __Jnu, _Tp & __Nnu, _Tp & __Jpnu, _Tp & __Npnu)
    {
      if (__x == _Tp(0))
        {
          if (__nu == _Tp(0))
            {
              __Jnu = _Tp(1);
              __Jpnu = _Tp(0);
            }
          else if (__nu == _Tp(1))
            {
              __Jnu = _Tp(0);
              __Jpnu = _Tp(0.5L);
            }
          else
            {
              __Jnu = _Tp(0);
              __Jpnu = _Tp(0);
            }
          __Nnu = -std::numeric_limits<_Tp>::infinity();
          __Npnu = std::numeric_limits<_Tp>::infinity();
          return;
        }
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __fp_min = std::sqrt(std::numeric_limits<_Tp>::min());
      const int __max_iter = 15000;
      const _Tp __x_min = _Tp(2);
      const int __nl = (__x < __x_min
                    ? static_cast<int>(__nu + _Tp(0.5L))
                    : std::max(0, static_cast<int>(__nu - __x + _Tp(1.5L))));
      const _Tp __mu = __nu - __nl;
      const _Tp __mu2 = __mu * __mu;
      const _Tp __xi = _Tp(1) / __x;
      const _Tp __xi2 = _Tp(2) * __xi;
      _Tp __w = __xi2 / __numeric_constants<_Tp>::__pi();
      int __isign = 1;
      _Tp __h = __nu * __xi;
      if (__h < __fp_min)
        __h = __fp_min;
      _Tp __b = __xi2 * __nu;
      _Tp __d = _Tp(0);
      _Tp __c = __h;
      int __i;
      for (__i = 1; __i <= __max_iter; ++__i)
        {
          __b += __xi2;
          __d = __b - __d;
          if (std::abs(__d) < __fp_min)
            __d = __fp_min;
          __c = __b - _Tp(1) / __c;
          if (std::abs(__c) < __fp_min)
            __c = __fp_min;
          __d = _Tp(1) / __d;
          const _Tp __del = __c * __d;
          __h *= __del;
          if (__d < _Tp(0))
            __isign = -__isign;
          if (std::abs(__del - _Tp(1)) < __eps)
            break;
        }
      if (__i > __max_iter)
        std::__throw_runtime_error(("Argument x too large in __bessel_jn; " "try asymptotic expansion."));
      _Tp __Jnul = __isign * __fp_min;
      _Tp __Jpnul = __h * __Jnul;
      _Tp __Jnul1 = __Jnul;
      _Tp __Jpnu1 = __Jpnul;
      _Tp __fact = __nu * __xi;
      for ( int __l = __nl; __l >= 1; --__l )
        {
          const _Tp __Jnutemp = __fact * __Jnul + __Jpnul;
          __fact -= __xi;
          __Jpnul = __fact * __Jnutemp - __Jnul;
          __Jnul = __Jnutemp;
        }
      if (__Jnul == _Tp(0))
        __Jnul = __eps;
      _Tp __f= __Jpnul / __Jnul;
      _Tp __Nmu, __Nnu1, __Npmu, __Jmu;
      if (__x < __x_min)
        {
          const _Tp __x2 = __x / _Tp(2);
          const _Tp __pimu = __numeric_constants<_Tp>::__pi() * __mu;
          _Tp __fact = (std::abs(__pimu) < __eps
                      ? _Tp(1) : __pimu / std::sin(__pimu));
          _Tp __d = -std::log(__x2);
          _Tp __e = __mu * __d;
          _Tp __fact2 = (std::abs(__e) < __eps
                       ? _Tp(1) : std::sinh(__e) / __e);
          _Tp __gam1, __gam2, __gampl, __gammi;
          __gamma_temme(__mu, __gam1, __gam2, __gampl, __gammi);
          _Tp __ff = (_Tp(2) / __numeric_constants<_Tp>::__pi())
                   * __fact * (__gam1 * std::cosh(__e) + __gam2 * __fact2 * __d);
          __e = std::exp(__e);
          _Tp __p = __e / (__numeric_constants<_Tp>::__pi() * __gampl);
          _Tp __q = _Tp(1) / (__e * __numeric_constants<_Tp>::__pi() * __gammi);
          const _Tp __pimu2 = __pimu / _Tp(2);
          _Tp __fact3 = (std::abs(__pimu2) < __eps
                       ? _Tp(1) : std::sin(__pimu2) / __pimu2 );
          _Tp __r = __numeric_constants<_Tp>::__pi() * __pimu2 * __fact3 * __fact3;
          _Tp __c = _Tp(1);
          __d = -__x2 * __x2;
          _Tp __sum = __ff + __r * __q;
          _Tp __sum1 = __p;
          for (__i = 1; __i <= __max_iter; ++__i)
            {
              __ff = (__i * __ff + __p + __q) / (__i * __i - __mu2);
              __c *= __d / _Tp(__i);
              __p /= _Tp(__i) - __mu;
              __q /= _Tp(__i) + __mu;
              const _Tp __del = __c * (__ff + __r * __q);
              __sum += __del;
              const _Tp __del1 = __c * __p - __i * __del;
              __sum1 += __del1;
              if ( std::abs(__del) < __eps * (_Tp(1) + std::abs(__sum)) )
                break;
            }
          if ( __i > __max_iter )
            std::__throw_runtime_error(("Bessel y series failed to converge " "in __bessel_jn."));
          __Nmu = -__sum;
          __Nnu1 = -__sum1 * __xi2;
          __Npmu = __mu * __xi * __Nmu - __Nnu1;
          __Jmu = __w / (__Npmu - __f * __Nmu);
        }
      else
        {
          _Tp __a = _Tp(0.25L) - __mu2;
          _Tp __q = _Tp(1);
          _Tp __p = -__xi / _Tp(2);
          _Tp __br = _Tp(2) * __x;
          _Tp __bi = _Tp(2);
          _Tp __fact = __a * __xi / (__p * __p + __q * __q);
          _Tp __cr = __br + __q * __fact;
          _Tp __ci = __bi + __p * __fact;
          _Tp __den = __br * __br + __bi * __bi;
          _Tp __dr = __br / __den;
          _Tp __di = -__bi / __den;
          _Tp __dlr = __cr * __dr - __ci * __di;
          _Tp __dli = __cr * __di + __ci * __dr;
          _Tp __temp = __p * __dlr - __q * __dli;
          __q = __p * __dli + __q * __dlr;
          __p = __temp;
          int __i;
          for (__i = 2; __i <= __max_iter; ++__i)
            {
              __a += _Tp(2 * (__i - 1));
              __bi += _Tp(2);
              __dr = __a * __dr + __br;
              __di = __a * __di + __bi;
              if (std::abs(__dr) + std::abs(__di) < __fp_min)
                __dr = __fp_min;
              __fact = __a / (__cr * __cr + __ci * __ci);
              __cr = __br + __cr * __fact;
              __ci = __bi - __ci * __fact;
              if (std::abs(__cr) + std::abs(__ci) < __fp_min)
                __cr = __fp_min;
              __den = __dr * __dr + __di * __di;
              __dr /= __den;
              __di /= -__den;
              __dlr = __cr * __dr - __ci * __di;
              __dli = __cr * __di + __ci * __dr;
              __temp = __p * __dlr - __q * __dli;
              __q = __p * __dli + __q * __dlr;
              __p = __temp;
              if (std::abs(__dlr - _Tp(1)) + std::abs(__dli) < __eps)
                break;
          }
          if (__i > __max_iter)
            std::__throw_runtime_error(("Lentz's method failed " "in __bessel_jn."));
          const _Tp __gam = (__p - __f) / __q;
          __Jmu = std::sqrt(__w / ((__p - __f) * __gam + __q));
          __Jmu = ::std::copysign(__Jmu, __Jnul);
          __Nmu = __gam * __Jmu;
          __Npmu = (__p + __q / __gam) * __Nmu;
          __Nnu1 = __mu * __xi * __Nmu - __Npmu;
      }
      __fact = __Jmu / __Jnul;
      __Jnu = __fact * __Jnul1;
      __Jpnu = __fact * __Jpnu1;
      for (__i = 1; __i <= __nl; ++__i)
        {
          const _Tp __Nnutemp = (__mu + __i) * __xi2 * __Nnu1 - __Nmu;
          __Nmu = __Nnu1;
          __Nnu1 = __Nnutemp;
        }
      __Nnu = __Nmu;
      __Npnu = __nu * __xi * __Nmu - __Nnu1;
      return;
    }
    template <typename _Tp>
    void
    __cyl_bessel_jn_asymp(_Tp __nu, _Tp __x, _Tp & __Jnu, _Tp & __Nnu)
    {
      const _Tp __mu = _Tp(4) * __nu * __nu;
      const _Tp __8x = _Tp(8) * __x;
      _Tp __P = _Tp(0);
      _Tp __Q = _Tp(0);
      _Tp __k = _Tp(0);
      _Tp __term = _Tp(1);
      int __epsP = 0;
      int __epsQ = 0;
      _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      do
        {
          __term *= (__k == 0
                     ? _Tp(1)
                     : -(__mu - (2 * __k - 1) * (2 * __k - 1)) / (__k * __8x));
          __epsP = std::abs(__term) < __eps * std::abs(__P);
          __P += __term;
          __k++;
          __term *= (__mu - (2 * __k - 1) * (2 * __k - 1)) / (__k * __8x);
          __epsQ = std::abs(__term) < __eps * std::abs(__Q);
          __Q += __term;
          if (__epsP && __epsQ && __k > (__nu / 2.))
            break;
          __k++;
        }
      while (__k < 1000);
      const _Tp __chi = __x - (__nu + _Tp(0.5L))
                             * __numeric_constants<_Tp>::__pi_2();
      const _Tp __c = std::cos(__chi);
      const _Tp __s = std::sin(__chi);
      const _Tp __coef = std::sqrt(_Tp(2)
                             / (__numeric_constants<_Tp>::__pi() * __x));
      __Jnu = __coef * (__c * __P - __s * __Q);
      __Nnu = __coef * (__s * __P + __c * __Q);
      return;
    }
    template <typename _Tp>
    _Tp
    __cyl_bessel_ij_series(_Tp __nu, _Tp __x, _Tp __sgn,
                           unsigned int __max_iter)
    {
      if (__x == _Tp(0))
 return __nu == _Tp(0) ? _Tp(1) : _Tp(0);
      const _Tp __x2 = __x / _Tp(2);
      _Tp __fact = __nu * std::log(__x2);
      __fact -= ::std::lgamma(__nu + _Tp(1));
      __fact = std::exp(__fact);
      const _Tp __xx4 = __sgn * __x2 * __x2;
      _Tp __Jn = _Tp(1);
      _Tp __term = _Tp(1);
      for (unsigned int __i = 1; __i < __max_iter; ++__i)
        {
          __term *= __xx4 / (_Tp(__i) * (__nu + _Tp(__i)));
          __Jn += __term;
          if (std::abs(__term / __Jn) < std::numeric_limits<_Tp>::epsilon())
            break;
        }
      return __fact * __Jn;
    }
    template<typename _Tp>
    _Tp
    __cyl_bessel_j(_Tp __nu, _Tp __x)
    {
      if (__nu < _Tp(0) || __x < _Tp(0))
        std::__throw_domain_error(("Bad argument " "in __cyl_bessel_j."));
      else if (__isnan(__nu) || __isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__x * __x < _Tp(10) * (__nu + _Tp(1)))
        return __cyl_bessel_ij_series(__nu, __x, -_Tp(1), 200);
      else if (__x > _Tp(1000))
        {
          _Tp __J_nu, __N_nu;
          __cyl_bessel_jn_asymp(__nu, __x, __J_nu, __N_nu);
          return __J_nu;
        }
      else
        {
          _Tp __J_nu, __N_nu, __Jp_nu, __Np_nu;
          __bessel_jn(__nu, __x, __J_nu, __N_nu, __Jp_nu, __Np_nu);
          return __J_nu;
        }
    }
    template<typename _Tp>
    _Tp
    __cyl_neumann_n(_Tp __nu, _Tp __x)
    {
      if (__nu < _Tp(0) || __x < _Tp(0))
        std::__throw_domain_error(("Bad argument " "in __cyl_neumann_n."));
      else if (__isnan(__nu) || __isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__x > _Tp(1000))
        {
          _Tp __J_nu, __N_nu;
          __cyl_bessel_jn_asymp(__nu, __x, __J_nu, __N_nu);
          return __N_nu;
        }
      else
        {
          _Tp __J_nu, __N_nu, __Jp_nu, __Np_nu;
          __bessel_jn(__nu, __x, __J_nu, __N_nu, __Jp_nu, __Np_nu);
          return __N_nu;
        }
    }
    template <typename _Tp>
    void
    __sph_bessel_jn(unsigned int __n, _Tp __x,
                    _Tp & __j_n, _Tp & __n_n, _Tp & __jp_n, _Tp & __np_n)
    {
      const _Tp __nu = _Tp(__n) + _Tp(0.5L);
      _Tp __J_nu, __N_nu, __Jp_nu, __Np_nu;
      __bessel_jn(__nu, __x, __J_nu, __N_nu, __Jp_nu, __Np_nu);
      const _Tp __factor = __numeric_constants<_Tp>::__sqrtpio2()
                         / std::sqrt(__x);
      __j_n = __factor * __J_nu;
      __n_n = __factor * __N_nu;
      __jp_n = __factor * __Jp_nu - __j_n / (_Tp(2) * __x);
      __np_n = __factor * __Np_nu - __n_n / (_Tp(2) * __x);
      return;
    }
    template <typename _Tp>
    _Tp
    __sph_bessel(unsigned int __n, _Tp __x)
    {
      if (__x < _Tp(0))
        std::__throw_domain_error(("Bad argument " "in __sph_bessel."));
      else if (__isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__x == _Tp(0))
        {
          if (__n == 0)
            return _Tp(1);
          else
            return _Tp(0);
        }
      else
        {
          _Tp __j_n, __n_n, __jp_n, __np_n;
          __sph_bessel_jn(__n, __x, __j_n, __n_n, __jp_n, __np_n);
          return __j_n;
        }
    }
    template <typename _Tp>
    _Tp
    __sph_neumann(unsigned int __n, _Tp __x)
    {
      if (__x < _Tp(0))
        std::__throw_domain_error(("Bad argument " "in __sph_neumann."));
      else if (__isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__x == _Tp(0))
        return -std::numeric_limits<_Tp>::infinity();
      else
        {
          _Tp __j_n, __n_n, __jp_n, __np_n;
          __sph_bessel_jn(__n, __x, __j_n, __n_n, __jp_n, __np_n);
          return __n_n;
        }
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tp>
    _Tp
    __beta_gamma(_Tp __x, _Tp __y)
    {
      _Tp __bet;
      if (__x > __y)
        {
          __bet = ::std::tgamma(__x)
                / ::std::tgamma(__x + __y);
          __bet *= ::std::tgamma(__y);
        }
      else
        {
          __bet = ::std::tgamma(__y)
                / ::std::tgamma(__x + __y);
          __bet *= ::std::tgamma(__x);
        }
      return __bet;
    }
    template<typename _Tp>
    _Tp
    __beta_lgamma(_Tp __x, _Tp __y)
    {
      _Tp __bet = ::std::lgamma(__x)
                + ::std::lgamma(__y)
                - ::std::lgamma(__x + __y);
      __bet = std::exp(__bet);
      return __bet;
    }
    template<typename _Tp>
    _Tp
    __beta_product(_Tp __x, _Tp __y)
    {
      _Tp __bet = (__x + __y) / (__x * __y);
      unsigned int __max_iter = 1000000;
      for (unsigned int __k = 1; __k < __max_iter; ++__k)
        {
          _Tp __term = (_Tp(1) + (__x + __y) / __k)
                     / ((_Tp(1) + __x / __k) * (_Tp(1) + __y / __k));
          __bet *= __term;
        }
      return __bet;
    }
    template<typename _Tp>
    inline _Tp
    __beta(_Tp __x, _Tp __y)
    {
      if (__isnan(__x) || __isnan(__y))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else
        return __beta_lgamma(__x, __y);
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tp>
    _Tp
    __ellint_rf(_Tp __x, _Tp __y, _Tp __z)
    {
      const _Tp __min = std::numeric_limits<_Tp>::min();
      const _Tp __lolim = _Tp(5) * __min;
      if (__x < _Tp(0) || __y < _Tp(0) || __z < _Tp(0))
        std::__throw_domain_error(("Argument less than zero " "in __ellint_rf."));
      else if (__x + __y < __lolim || __x + __z < __lolim
            || __y + __z < __lolim)
        std::__throw_domain_error(("Argument too small in __ellint_rf"));
      else
        {
          const _Tp __c0 = _Tp(1) / _Tp(4);
          const _Tp __c1 = _Tp(1) / _Tp(24);
          const _Tp __c2 = _Tp(1) / _Tp(10);
          const _Tp __c3 = _Tp(3) / _Tp(44);
          const _Tp __c4 = _Tp(1) / _Tp(14);
          _Tp __xn = __x;
          _Tp __yn = __y;
          _Tp __zn = __z;
          const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
          const _Tp __errtol = std::pow(__eps, _Tp(1) / _Tp(6));
          _Tp __mu;
          _Tp __xndev, __yndev, __zndev;
          const unsigned int __max_iter = 100;
          for (unsigned int __iter = 0; __iter < __max_iter; ++__iter)
            {
              __mu = (__xn + __yn + __zn) / _Tp(3);
              __xndev = 2 - (__mu + __xn) / __mu;
              __yndev = 2 - (__mu + __yn) / __mu;
              __zndev = 2 - (__mu + __zn) / __mu;
              _Tp __epsilon = std::max(std::abs(__xndev), std::abs(__yndev));
              __epsilon = std::max(__epsilon, std::abs(__zndev));
              if (__epsilon < __errtol)
                break;
              const _Tp __xnroot = std::sqrt(__xn);
              const _Tp __ynroot = std::sqrt(__yn);
              const _Tp __znroot = std::sqrt(__zn);
              const _Tp __lambda = __xnroot * (__ynroot + __znroot)
                                 + __ynroot * __znroot;
              __xn = __c0 * (__xn + __lambda);
              __yn = __c0 * (__yn + __lambda);
              __zn = __c0 * (__zn + __lambda);
            }
          const _Tp __e2 = __xndev * __yndev - __zndev * __zndev;
          const _Tp __e3 = __xndev * __yndev * __zndev;
          const _Tp __s = _Tp(1) + (__c1 * __e2 - __c2 - __c3 * __e3) * __e2
                   + __c4 * __e3;
          return __s / std::sqrt(__mu);
        }
    }
    template<typename _Tp>
    _Tp
    __comp_ellint_1_series(_Tp __k)
    {
      const _Tp __kk = __k * __k;
      _Tp __term = __kk / _Tp(4);
      _Tp __sum = _Tp(1) + __term;
      const unsigned int __max_iter = 1000;
      for (unsigned int __i = 2; __i < __max_iter; ++__i)
        {
          __term *= (2 * __i - 1) * __kk / (2 * __i);
          if (__term < std::numeric_limits<_Tp>::epsilon())
            break;
          __sum += __term;
        }
      return __numeric_constants<_Tp>::__pi_2() * __sum;
    }
    template<typename _Tp>
    _Tp
    __comp_ellint_1(_Tp __k)
    {
      if (__isnan(__k))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (std::abs(__k) >= _Tp(1))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else
        return __ellint_rf(_Tp(0), _Tp(1) - __k * __k, _Tp(1));
    }
    template<typename _Tp>
    _Tp
    __ellint_1(_Tp __k, _Tp __phi)
    {
      if (__isnan(__k) || __isnan(__phi))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (std::abs(__k) > _Tp(1))
        std::__throw_domain_error(("Bad argument in __ellint_1."));
      else
        {
          const int __n = std::floor(__phi / __numeric_constants<_Tp>::__pi()
                                   + _Tp(0.5L));
          const _Tp __phi_red = __phi
                              - __n * __numeric_constants<_Tp>::__pi();
          const _Tp __s = std::sin(__phi_red);
          const _Tp __c = std::cos(__phi_red);
          const _Tp __F = __s
                        * __ellint_rf(__c * __c,
                                _Tp(1) - __k * __k * __s * __s, _Tp(1));
          if (__n == 0)
            return __F;
          else
            return __F + _Tp(2) * __n * __comp_ellint_1(__k);
        }
    }
    template<typename _Tp>
    _Tp
    __comp_ellint_2_series(_Tp __k)
    {
      const _Tp __kk = __k * __k;
      _Tp __term = __kk;
      _Tp __sum = __term;
      const unsigned int __max_iter = 1000;
      for (unsigned int __i = 2; __i < __max_iter; ++__i)
        {
          const _Tp __i2m = 2 * __i - 1;
          const _Tp __i2 = 2 * __i;
          __term *= __i2m * __i2m * __kk / (__i2 * __i2);
          if (__term < std::numeric_limits<_Tp>::epsilon())
            break;
          __sum += __term / __i2m;
        }
      return __numeric_constants<_Tp>::__pi_2() * (_Tp(1) - __sum);
    }
    template<typename _Tp>
    _Tp
    __ellint_rd(_Tp __x, _Tp __y, _Tp __z)
    {
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __errtol = std::pow(__eps / _Tp(8), _Tp(1) / _Tp(6));
      const _Tp __max = std::numeric_limits<_Tp>::max();
      const _Tp __lolim = _Tp(2) / std::pow(__max, _Tp(2) / _Tp(3));
      if (__x < _Tp(0) || __y < _Tp(0))
        std::__throw_domain_error(("Argument less than zero " "in __ellint_rd."));
      else if (__x + __y < __lolim || __z < __lolim)
        std::__throw_domain_error(("Argument too small " "in __ellint_rd."));
      else
        {
          const _Tp __c0 = _Tp(1) / _Tp(4);
          const _Tp __c1 = _Tp(3) / _Tp(14);
          const _Tp __c2 = _Tp(1) / _Tp(6);
          const _Tp __c3 = _Tp(9) / _Tp(22);
          const _Tp __c4 = _Tp(3) / _Tp(26);
          _Tp __xn = __x;
          _Tp __yn = __y;
          _Tp __zn = __z;
          _Tp __sigma = _Tp(0);
          _Tp __power4 = _Tp(1);
          _Tp __mu;
          _Tp __xndev, __yndev, __zndev;
          const unsigned int __max_iter = 100;
          for (unsigned int __iter = 0; __iter < __max_iter; ++__iter)
            {
              __mu = (__xn + __yn + _Tp(3) * __zn) / _Tp(5);
              __xndev = (__mu - __xn) / __mu;
              __yndev = (__mu - __yn) / __mu;
              __zndev = (__mu - __zn) / __mu;
              _Tp __epsilon = std::max(std::abs(__xndev), std::abs(__yndev));
              __epsilon = std::max(__epsilon, std::abs(__zndev));
              if (__epsilon < __errtol)
                break;
              _Tp __xnroot = std::sqrt(__xn);
              _Tp __ynroot = std::sqrt(__yn);
              _Tp __znroot = std::sqrt(__zn);
              _Tp __lambda = __xnroot * (__ynroot + __znroot)
                           + __ynroot * __znroot;
              __sigma += __power4 / (__znroot * (__zn + __lambda));
              __power4 *= __c0;
              __xn = __c0 * (__xn + __lambda);
              __yn = __c0 * (__yn + __lambda);
              __zn = __c0 * (__zn + __lambda);
            }
          _Tp __ea = __xndev * __yndev;
          _Tp __eb = __zndev * __zndev;
          _Tp __ec = __ea - __eb;
          _Tp __ed = __ea - _Tp(6) * __eb;
          _Tp __ef = __ed + __ec + __ec;
          _Tp __s1 = __ed * (-__c1 + __c3 * __ed
                                   / _Tp(3) - _Tp(3) * __c4 * __zndev * __ef
                                   / _Tp(2));
          _Tp __s2 = __zndev
                   * (__c2 * __ef
                    + __zndev * (-__c3 * __ec - __zndev * __c4 - __ea));
          return _Tp(3) * __sigma + __power4 * (_Tp(1) + __s1 + __s2)
                                        / (__mu * std::sqrt(__mu));
        }
    }
    template<typename _Tp>
    _Tp
    __comp_ellint_2(_Tp __k)
    {
      if (__isnan(__k))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (std::abs(__k) == 1)
        return _Tp(1);
      else if (std::abs(__k) > _Tp(1))
        std::__throw_domain_error(("Bad argument in __comp_ellint_2."));
      else
        {
          const _Tp __kk = __k * __k;
          return __ellint_rf(_Tp(0), _Tp(1) - __kk, _Tp(1))
               - __kk * __ellint_rd(_Tp(0), _Tp(1) - __kk, _Tp(1)) / _Tp(3);
        }
    }
    template<typename _Tp>
    _Tp
    __ellint_2(_Tp __k, _Tp __phi)
    {
      if (__isnan(__k) || __isnan(__phi))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (std::abs(__k) > _Tp(1))
        std::__throw_domain_error(("Bad argument in __ellint_2."));
      else
        {
          const int __n = std::floor(__phi / __numeric_constants<_Tp>::__pi()
                                   + _Tp(0.5L));
          const _Tp __phi_red = __phi
                              - __n * __numeric_constants<_Tp>::__pi();
          const _Tp __kk = __k * __k;
          const _Tp __s = std::sin(__phi_red);
          const _Tp __ss = __s * __s;
          const _Tp __sss = __ss * __s;
          const _Tp __c = std::cos(__phi_red);
          const _Tp __cc = __c * __c;
          const _Tp __E = __s
                        * __ellint_rf(__cc, _Tp(1) - __kk * __ss, _Tp(1))
                        - __kk * __sss
                        * __ellint_rd(__cc, _Tp(1) - __kk * __ss, _Tp(1))
                        / _Tp(3);
          if (__n == 0)
            return __E;
          else
            return __E + _Tp(2) * __n * __comp_ellint_2(__k);
        }
    }
    template<typename _Tp>
    _Tp
    __ellint_rc(_Tp __x, _Tp __y)
    {
      const _Tp __min = std::numeric_limits<_Tp>::min();
      const _Tp __lolim = _Tp(5) * __min;
      if (__x < _Tp(0) || __y < _Tp(0) || __x + __y < __lolim)
        std::__throw_domain_error(("Argument less than zero " "in __ellint_rc."));
      else
        {
          const _Tp __c0 = _Tp(1) / _Tp(4);
          const _Tp __c1 = _Tp(1) / _Tp(7);
          const _Tp __c2 = _Tp(9) / _Tp(22);
          const _Tp __c3 = _Tp(3) / _Tp(10);
          const _Tp __c4 = _Tp(3) / _Tp(8);
          _Tp __xn = __x;
          _Tp __yn = __y;
          const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
          const _Tp __errtol = std::pow(__eps / _Tp(30), _Tp(1) / _Tp(6));
          _Tp __mu;
          _Tp __sn;
          const unsigned int __max_iter = 100;
          for (unsigned int __iter = 0; __iter < __max_iter; ++__iter)
            {
              __mu = (__xn + _Tp(2) * __yn) / _Tp(3);
              __sn = (__yn + __mu) / __mu - _Tp(2);
              if (std::abs(__sn) < __errtol)
                break;
              const _Tp __lambda = _Tp(2) * std::sqrt(__xn) * std::sqrt(__yn)
                             + __yn;
              __xn = __c0 * (__xn + __lambda);
              __yn = __c0 * (__yn + __lambda);
            }
          _Tp __s = __sn * __sn
                  * (__c3 + __sn*(__c1 + __sn * (__c4 + __sn * __c2)));
          return (_Tp(1) + __s) / std::sqrt(__mu);
        }
    }
    template<typename _Tp>
    _Tp
    __ellint_rj(_Tp __x, _Tp __y, _Tp __z, _Tp __p)
    {
      const _Tp __min = std::numeric_limits<_Tp>::min();
      const _Tp __lolim = std::pow(_Tp(5) * __min, _Tp(1)/_Tp(3));
      if (__x < _Tp(0) || __y < _Tp(0) || __z < _Tp(0))
        std::__throw_domain_error(("Argument less than zero " "in __ellint_rj."));
      else if (__x + __y < __lolim || __x + __z < __lolim
            || __y + __z < __lolim || __p < __lolim)
        std::__throw_domain_error(("Argument too small " "in __ellint_rj"));
      else
        {
          const _Tp __c0 = _Tp(1) / _Tp(4);
          const _Tp __c1 = _Tp(3) / _Tp(14);
          const _Tp __c2 = _Tp(1) / _Tp(3);
          const _Tp __c3 = _Tp(3) / _Tp(22);
          const _Tp __c4 = _Tp(3) / _Tp(26);
          _Tp __xn = __x;
          _Tp __yn = __y;
          _Tp __zn = __z;
          _Tp __pn = __p;
          _Tp __sigma = _Tp(0);
          _Tp __power4 = _Tp(1);
          const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
          const _Tp __errtol = std::pow(__eps / _Tp(8), _Tp(1) / _Tp(6));
          _Tp __mu;
          _Tp __xndev, __yndev, __zndev, __pndev;
          const unsigned int __max_iter = 100;
          for (unsigned int __iter = 0; __iter < __max_iter; ++__iter)
            {
              __mu = (__xn + __yn + __zn + _Tp(2) * __pn) / _Tp(5);
              __xndev = (__mu - __xn) / __mu;
              __yndev = (__mu - __yn) / __mu;
              __zndev = (__mu - __zn) / __mu;
              __pndev = (__mu - __pn) / __mu;
              _Tp __epsilon = std::max(std::abs(__xndev), std::abs(__yndev));
              __epsilon = std::max(__epsilon, std::abs(__zndev));
              __epsilon = std::max(__epsilon, std::abs(__pndev));
              if (__epsilon < __errtol)
                break;
              const _Tp __xnroot = std::sqrt(__xn);
              const _Tp __ynroot = std::sqrt(__yn);
              const _Tp __znroot = std::sqrt(__zn);
              const _Tp __lambda = __xnroot * (__ynroot + __znroot)
                                 + __ynroot * __znroot;
              const _Tp __alpha1 = __pn * (__xnroot + __ynroot + __znroot)
                                + __xnroot * __ynroot * __znroot;
              const _Tp __alpha2 = __alpha1 * __alpha1;
              const _Tp __beta = __pn * (__pn + __lambda)
                                      * (__pn + __lambda);
              __sigma += __power4 * __ellint_rc(__alpha2, __beta);
              __power4 *= __c0;
              __xn = __c0 * (__xn + __lambda);
              __yn = __c0 * (__yn + __lambda);
              __zn = __c0 * (__zn + __lambda);
              __pn = __c0 * (__pn + __lambda);
            }
          _Tp __ea = __xndev * (__yndev + __zndev) + __yndev * __zndev;
          _Tp __eb = __xndev * __yndev * __zndev;
          _Tp __ec = __pndev * __pndev;
          _Tp __e2 = __ea - _Tp(3) * __ec;
          _Tp __e3 = __eb + _Tp(2) * __pndev * (__ea - __ec);
          _Tp __s1 = _Tp(1) + __e2 * (-__c1 + _Tp(3) * __c3 * __e2 / _Tp(4)
                            - _Tp(3) * __c4 * __e3 / _Tp(2));
          _Tp __s2 = __eb * (__c2 / _Tp(2)
                   + __pndev * (-__c3 - __c3 + __pndev * __c4));
          _Tp __s3 = __pndev * __ea * (__c2 - __pndev * __c3)
                   - __c2 * __pndev * __ec;
          return _Tp(3) * __sigma + __power4 * (__s1 + __s2 + __s3)
                                             / (__mu * std::sqrt(__mu));
        }
    }
    template<typename _Tp>
    _Tp
    __comp_ellint_3(_Tp __k, _Tp __nu)
    {
      if (__isnan(__k) || __isnan(__nu))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__nu == _Tp(1))
        return std::numeric_limits<_Tp>::infinity();
      else if (std::abs(__k) > _Tp(1))
        std::__throw_domain_error(("Bad argument in __comp_ellint_3."));
      else
        {
          const _Tp __kk = __k * __k;
          return __ellint_rf(_Tp(0), _Tp(1) - __kk, _Tp(1))
               + __nu
               * __ellint_rj(_Tp(0), _Tp(1) - __kk, _Tp(1), _Tp(1) - __nu)
               / _Tp(3);
        }
    }
    template<typename _Tp>
    _Tp
    __ellint_3(_Tp __k, _Tp __nu, _Tp __phi)
    {
      if (__isnan(__k) || __isnan(__nu) || __isnan(__phi))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (std::abs(__k) > _Tp(1))
        std::__throw_domain_error(("Bad argument in __ellint_3."));
      else
        {
          const int __n = std::floor(__phi / __numeric_constants<_Tp>::__pi()
                                   + _Tp(0.5L));
          const _Tp __phi_red = __phi
                              - __n * __numeric_constants<_Tp>::__pi();
          const _Tp __kk = __k * __k;
          const _Tp __s = std::sin(__phi_red);
          const _Tp __ss = __s * __s;
          const _Tp __sss = __ss * __s;
          const _Tp __c = std::cos(__phi_red);
          const _Tp __cc = __c * __c;
          const _Tp __Pi = __s
                         * __ellint_rf(__cc, _Tp(1) - __kk * __ss, _Tp(1))
                         + __nu * __sss
                         * __ellint_rj(__cc, _Tp(1) - __kk * __ss, _Tp(1),
                                       _Tp(1) - __nu * __ss) / _Tp(3);
          if (__n == 0)
            return __Pi;
          else
            return __Pi + _Tp(2) * __n * __comp_ellint_3(__k, __nu);
        }
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tp> _Tp __expint_E1(_Tp);
    template<typename _Tp>
    _Tp
    __expint_E1_series(_Tp __x)
    {
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      _Tp __term = _Tp(1);
      _Tp __esum = _Tp(0);
      _Tp __osum = _Tp(0);
      const unsigned int __max_iter = 1000;
      for (unsigned int __i = 1; __i < __max_iter; ++__i)
        {
          __term *= - __x / __i;
          if (std::abs(__term) < __eps)
            break;
          if (__term >= _Tp(0))
            __esum += __term / __i;
          else
            __osum += __term / __i;
        }
      return - __esum - __osum
             - __numeric_constants<_Tp>::__gamma_e() - std::log(__x);
    }
    template<typename _Tp>
    _Tp
    __expint_E1_asymp(_Tp __x)
    {
      _Tp __term = _Tp(1);
      _Tp __esum = _Tp(1);
      _Tp __osum = _Tp(0);
      const unsigned int __max_iter = 1000;
      for (unsigned int __i = 1; __i < __max_iter; ++__i)
        {
          _Tp __prev = __term;
          __term *= - __i / __x;
          if (std::abs(__term) > std::abs(__prev))
            break;
          if (__term >= _Tp(0))
            __esum += __term;
          else
            __osum += __term;
        }
      return std::exp(- __x) * (__esum + __osum) / __x;
    }
    template<typename _Tp>
    _Tp
    __expint_En_series(unsigned int __n, _Tp __x)
    {
      const unsigned int __max_iter = 1000;
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const int __nm1 = __n - 1;
      _Tp __ans = (__nm1 != 0
                ? _Tp(1) / __nm1 : -std::log(__x)
                                   - __numeric_constants<_Tp>::__gamma_e());
      _Tp __fact = _Tp(1);
      for (int __i = 1; __i <= __max_iter; ++__i)
        {
          __fact *= -__x / _Tp(__i);
          _Tp __del;
          if ( __i != __nm1 )
            __del = -__fact / _Tp(__i - __nm1);
          else
            {
              _Tp __psi = -__numeric_constants<_Tp>::gamma_e();
              for (int __ii = 1; __ii <= __nm1; ++__ii)
                __psi += _Tp(1) / _Tp(__ii);
              __del = __fact * (__psi - std::log(__x));
            }
          __ans += __del;
          if (std::abs(__del) < __eps * std::abs(__ans))
            return __ans;
        }
      std::__throw_runtime_error(("Series summation failed " "in __expint_En_series."));
    }
    template<typename _Tp>
    _Tp
    __expint_En_cont_frac(unsigned int __n, _Tp __x)
    {
      const unsigned int __max_iter = 1000;
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __fp_min = std::numeric_limits<_Tp>::min();
      const int __nm1 = __n - 1;
      _Tp __b = __x + _Tp(__n);
      _Tp __c = _Tp(1) / __fp_min;
      _Tp __d = _Tp(1) / __b;
      _Tp __h = __d;
      for ( unsigned int __i = 1; __i <= __max_iter; ++__i )
        {
          _Tp __a = -_Tp(__i * (__nm1 + __i));
          __b += _Tp(2);
          __d = _Tp(1) / (__a * __d + __b);
          __c = __b + __a / __c;
          const _Tp __del = __c * __d;
          __h *= __del;
          if (std::abs(__del - _Tp(1)) < __eps)
            {
              const _Tp __ans = __h * std::exp(-__x);
              return __ans;
            }
        }
      std::__throw_runtime_error(("Continued fraction failed " "in __expint_En_cont_frac."));
    }
    template<typename _Tp>
    _Tp
    __expint_En_recursion(unsigned int __n, _Tp __x)
    {
      _Tp __En;
      _Tp __E1 = __expint_E1(__x);
      if (__x < _Tp(__n))
        {
          __En = __E1;
          for (unsigned int __j = 2; __j < __n; ++__j)
            __En = (std::exp(-__x) - __x * __En) / _Tp(__j - 1);
        }
      else
        {
          __En = _Tp(1);
          const int __N = __n + 20;
          _Tp __save = _Tp(0);
          for (int __j = __N; __j > 0; --__j)
            {
              __En = (std::exp(-__x) - __j * __En) / __x;
              if (__j == __n)
                __save = __En;
            }
            _Tp __norm = __En / __E1;
            __En /= __norm;
        }
      return __En;
    }
    template<typename _Tp>
    _Tp
    __expint_Ei_series(_Tp __x)
    {
      _Tp __term = _Tp(1);
      _Tp __sum = _Tp(0);
      const unsigned int __max_iter = 1000;
      for (unsigned int __i = 1; __i < __max_iter; ++__i)
        {
          __term *= __x / __i;
          __sum += __term / __i;
          if (__term < std::numeric_limits<_Tp>::epsilon() * __sum)
            break;
        }
      return __numeric_constants<_Tp>::__gamma_e() + __sum + std::log(__x);
    }
    template<typename _Tp>
    _Tp
    __expint_Ei_asymp(_Tp __x)
    {
      _Tp __term = _Tp(1);
      _Tp __sum = _Tp(1);
      const unsigned int __max_iter = 1000;
      for (unsigned int __i = 1; __i < __max_iter; ++__i)
        {
          _Tp __prev = __term;
          __term *= __i / __x;
          if (__term < std::numeric_limits<_Tp>::epsilon())
            break;
          if (__term >= __prev)
            break;
          __sum += __term;
        }
      return std::exp(__x) * __sum / __x;
    }
    template<typename _Tp>
    _Tp
    __expint_Ei(_Tp __x)
    {
      if (__x < _Tp(0))
        return -__expint_E1(-__x);
      else if (__x < -std::log(std::numeric_limits<_Tp>::epsilon()))
        return __expint_Ei_series(__x);
      else
        return __expint_Ei_asymp(__x);
    }
    template<typename _Tp>
    _Tp
    __expint_E1(_Tp __x)
    {
      if (__x < _Tp(0))
        return -__expint_Ei(-__x);
      else if (__x < _Tp(1))
        return __expint_E1_series(__x);
      else if (__x < _Tp(100))
        return __expint_En_cont_frac(1, __x);
      else
        return __expint_E1_asymp(__x);
    }
    template<typename _Tp>
    _Tp
    __expint_asymp(unsigned int __n, _Tp __x)
    {
      _Tp __term = _Tp(1);
      _Tp __sum = _Tp(1);
      for (unsigned int __i = 1; __i <= __n; ++__i)
        {
          _Tp __prev = __term;
          __term *= -(__n - __i + 1) / __x;
          if (std::abs(__term) > std::abs(__prev))
            break;
          __sum += __term;
        }
      return std::exp(-__x) * __sum / __x;
    }
    template<typename _Tp>
    _Tp
    __expint_large_n(unsigned int __n, _Tp __x)
    {
      const _Tp __xpn = __x + __n;
      const _Tp __xpn2 = __xpn * __xpn;
      _Tp __term = _Tp(1);
      _Tp __sum = _Tp(1);
      for (unsigned int __i = 1; __i <= __n; ++__i)
        {
          _Tp __prev = __term;
          __term *= (__n - 2 * (__i - 1) * __x) / __xpn2;
          if (std::abs(__term) < std::numeric_limits<_Tp>::epsilon())
            break;
          __sum += __term;
        }
      return std::exp(-__x) * __sum / __xpn;
    }
    template<typename _Tp>
    _Tp
    __expint(unsigned int __n, _Tp __x)
    {
      if (__isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__n <= 1 && __x == _Tp(0))
        return std::numeric_limits<_Tp>::infinity();
      else
        {
          _Tp __E0 = std::exp(__x) / __x;
          if (__n == 0)
            return __E0;
          _Tp __E1 = __expint_E1(__x);
          if (__n == 1)
            return __E1;
          if (__x == _Tp(0))
            return _Tp(1) / static_cast<_Tp>(__n - 1);
          _Tp __En = __expint_En_recursion(__n, __x);
          return __En;
        }
    }
    template<typename _Tp>
    inline _Tp
    __expint(_Tp __x)
    {
      if (__isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else
        return __expint_Ei(__x);
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tp>
    _Tp
    __conf_hyperg_series(_Tp __a, _Tp __c, _Tp __x)
    {
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      _Tp __term = _Tp(1);
      _Tp __Fac = _Tp(1);
      const unsigned int __max_iter = 100000;
      unsigned int __i;
      for (__i = 0; __i < __max_iter; ++__i)
        {
          __term *= (__a + _Tp(__i)) * __x
                  / ((__c + _Tp(__i)) * _Tp(1 + __i));
          if (std::abs(__term) < __eps)
            {
              break;
            }
          __Fac += __term;
        }
      if (__i == __max_iter)
        std::__throw_runtime_error(("Series failed to converge " "in __conf_hyperg_series."));
      return __Fac;
    }
    template<typename _Tp>
    _Tp
    __conf_hyperg_luke(_Tp __a, _Tp __c, _Tp __xin)
    {
      const _Tp __big = std::pow(std::numeric_limits<_Tp>::max(), _Tp(0.16L));
      const int __nmax = 20000;
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __x = -__xin;
      const _Tp __x3 = __x * __x * __x;
      const _Tp __t0 = __a / __c;
      const _Tp __t1 = (__a + _Tp(1)) / (_Tp(2) * __c);
      const _Tp __t2 = (__a + _Tp(2)) / (_Tp(2) * (__c + _Tp(1)));
      _Tp __F = _Tp(1);
      _Tp __prec;
      _Tp __Bnm3 = _Tp(1);
      _Tp __Bnm2 = _Tp(1) + __t1 * __x;
      _Tp __Bnm1 = _Tp(1) + __t2 * __x * (_Tp(1) + __t1 / _Tp(3) * __x);
      _Tp __Anm3 = _Tp(1);
      _Tp __Anm2 = __Bnm2 - __t0 * __x;
      _Tp __Anm1 = __Bnm1 - __t0 * (_Tp(1) + __t2 * __x) * __x
                 + __t0 * __t1 * (__c / (__c + _Tp(1))) * __x * __x;
      int __n = 3;
      while(1)
        {
          _Tp __npam1 = _Tp(__n - 1) + __a;
          _Tp __npcm1 = _Tp(__n - 1) + __c;
          _Tp __npam2 = _Tp(__n - 2) + __a;
          _Tp __npcm2 = _Tp(__n - 2) + __c;
          _Tp __tnm1 = _Tp(2 * __n - 1);
          _Tp __tnm3 = _Tp(2 * __n - 3);
          _Tp __tnm5 = _Tp(2 * __n - 5);
          _Tp __F1 = (_Tp(__n - 2) - __a) / (_Tp(2) * __tnm3 * __npcm1);
          _Tp __F2 = (_Tp(__n) + __a) * __npam1
                   / (_Tp(4) * __tnm1 * __tnm3 * __npcm2 * __npcm1);
          _Tp __F3 = -__npam2 * __npam1 * (_Tp(__n - 2) - __a)
                   / (_Tp(8) * __tnm3 * __tnm3 * __tnm5
                   * (_Tp(__n - 3) + __c) * __npcm2 * __npcm1);
          _Tp __E = -__npam1 * (_Tp(__n - 1) - __c)
                   / (_Tp(2) * __tnm3 * __npcm2 * __npcm1);
          _Tp __An = (_Tp(1) + __F1 * __x) * __Anm1
                   + (__E + __F2 * __x) * __x * __Anm2 + __F3 * __x3 * __Anm3;
          _Tp __Bn = (_Tp(1) + __F1 * __x) * __Bnm1
                   + (__E + __F2 * __x) * __x * __Bnm2 + __F3 * __x3 * __Bnm3;
          _Tp __r = __An / __Bn;
          __prec = std::abs((__F - __r) / __F);
          __F = __r;
          if (__prec < __eps || __n > __nmax)
            break;
          if (std::abs(__An) > __big || std::abs(__Bn) > __big)
            {
              __An /= __big;
              __Bn /= __big;
              __Anm1 /= __big;
              __Bnm1 /= __big;
              __Anm2 /= __big;
              __Bnm2 /= __big;
              __Anm3 /= __big;
              __Bnm3 /= __big;
            }
          else if (std::abs(__An) < _Tp(1) / __big
                || std::abs(__Bn) < _Tp(1) / __big)
            {
              __An *= __big;
              __Bn *= __big;
              __Anm1 *= __big;
              __Bnm1 *= __big;
              __Anm2 *= __big;
              __Bnm2 *= __big;
              __Anm3 *= __big;
              __Bnm3 *= __big;
            }
          ++__n;
          __Bnm3 = __Bnm2;
          __Bnm2 = __Bnm1;
          __Bnm1 = __Bn;
          __Anm3 = __Anm2;
          __Anm2 = __Anm1;
          __Anm1 = __An;
        }
      if (__n >= __nmax)
        std::__throw_runtime_error(("Iteration failed to converge " "in __conf_hyperg_luke."));
      return __F;
    }
    template<typename _Tp>
    _Tp
    __conf_hyperg(_Tp __a, _Tp __c, _Tp __x)
    {
      const _Tp __c_nint = ::std::nearbyint(__c);
      if (__isnan(__a) || __isnan(__c) || __isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__c_nint == __c && __c_nint <= 0)
        return std::numeric_limits<_Tp>::infinity();
      else if (__a == _Tp(0))
        return _Tp(1);
      else if (__c == __a)
        return std::exp(__x);
      else if (__x < _Tp(0))
        return __conf_hyperg_luke(__a, __c, __x);
      else
        return __conf_hyperg_series(__a, __c, __x);
    }
    template<typename _Tp>
    _Tp
    __hyperg_series(_Tp __a, _Tp __b, _Tp __c, _Tp __x)
    {
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      _Tp __term = _Tp(1);
      _Tp __Fabc = _Tp(1);
      const unsigned int __max_iter = 100000;
      unsigned int __i;
      for (__i = 0; __i < __max_iter; ++__i)
        {
          __term *= (__a + _Tp(__i)) * (__b + _Tp(__i)) * __x
                  / ((__c + _Tp(__i)) * _Tp(1 + __i));
          if (std::abs(__term) < __eps)
            {
              break;
            }
          __Fabc += __term;
        }
      if (__i == __max_iter)
        std::__throw_runtime_error(("Series failed to converge " "in __hyperg_series."));
      return __Fabc;
    }
    template<typename _Tp>
    _Tp
    __hyperg_luke(_Tp __a, _Tp __b, _Tp __c, _Tp __xin)
    {
      const _Tp __big = std::pow(std::numeric_limits<_Tp>::max(), _Tp(0.16L));
      const int __nmax = 20000;
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __x = -__xin;
      const _Tp __x3 = __x * __x * __x;
      const _Tp __t0 = __a * __b / __c;
      const _Tp __t1 = (__a + _Tp(1)) * (__b + _Tp(1)) / (_Tp(2) * __c);
      const _Tp __t2 = (__a + _Tp(2)) * (__b + _Tp(2))
                     / (_Tp(2) * (__c + _Tp(1)));
      _Tp __F = _Tp(1);
      _Tp __Bnm3 = _Tp(1);
      _Tp __Bnm2 = _Tp(1) + __t1 * __x;
      _Tp __Bnm1 = _Tp(1) + __t2 * __x * (_Tp(1) + __t1 / _Tp(3) * __x);
      _Tp __Anm3 = _Tp(1);
      _Tp __Anm2 = __Bnm2 - __t0 * __x;
      _Tp __Anm1 = __Bnm1 - __t0 * (_Tp(1) + __t2 * __x) * __x
                 + __t0 * __t1 * (__c / (__c + _Tp(1))) * __x * __x;
      int __n = 3;
      while (1)
        {
          const _Tp __npam1 = _Tp(__n - 1) + __a;
          const _Tp __npbm1 = _Tp(__n - 1) + __b;
          const _Tp __npcm1 = _Tp(__n - 1) + __c;
          const _Tp __npam2 = _Tp(__n - 2) + __a;
          const _Tp __npbm2 = _Tp(__n - 2) + __b;
          const _Tp __npcm2 = _Tp(__n - 2) + __c;
          const _Tp __tnm1 = _Tp(2 * __n - 1);
          const _Tp __tnm3 = _Tp(2 * __n - 3);
          const _Tp __tnm5 = _Tp(2 * __n - 5);
          const _Tp __n2 = __n * __n;
          const _Tp __F1 = (_Tp(3) * __n2 + (__a + __b - _Tp(6)) * __n
                         + _Tp(2) - __a * __b - _Tp(2) * (__a + __b))
                         / (_Tp(2) * __tnm3 * __npcm1);
          const _Tp __F2 = -(_Tp(3) * __n2 - (__a + __b + _Tp(6)) * __n
                         + _Tp(2) - __a * __b) * __npam1 * __npbm1
                         / (_Tp(4) * __tnm1 * __tnm3 * __npcm2 * __npcm1);
          const _Tp __F3 = (__npam2 * __npam1 * __npbm2 * __npbm1
                         * (_Tp(__n - 2) - __a) * (_Tp(__n - 2) - __b))
                         / (_Tp(8) * __tnm3 * __tnm3 * __tnm5
                         * (_Tp(__n - 3) + __c) * __npcm2 * __npcm1);
          const _Tp __E = -__npam1 * __npbm1 * (_Tp(__n - 1) - __c)
                         / (_Tp(2) * __tnm3 * __npcm2 * __npcm1);
          _Tp __An = (_Tp(1) + __F1 * __x) * __Anm1
                   + (__E + __F2 * __x) * __x * __Anm2 + __F3 * __x3 * __Anm3;
          _Tp __Bn = (_Tp(1) + __F1 * __x) * __Bnm1
                   + (__E + __F2 * __x) * __x * __Bnm2 + __F3 * __x3 * __Bnm3;
          const _Tp __r = __An / __Bn;
          const _Tp __prec = std::abs((__F - __r) / __F);
          __F = __r;
          if (__prec < __eps || __n > __nmax)
            break;
          if (std::abs(__An) > __big || std::abs(__Bn) > __big)
            {
              __An /= __big;
              __Bn /= __big;
              __Anm1 /= __big;
              __Bnm1 /= __big;
              __Anm2 /= __big;
              __Bnm2 /= __big;
              __Anm3 /= __big;
              __Bnm3 /= __big;
            }
          else if (std::abs(__An) < _Tp(1) / __big
                || std::abs(__Bn) < _Tp(1) / __big)
            {
              __An *= __big;
              __Bn *= __big;
              __Anm1 *= __big;
              __Bnm1 *= __big;
              __Anm2 *= __big;
              __Bnm2 *= __big;
              __Anm3 *= __big;
              __Bnm3 *= __big;
            }
          ++__n;
          __Bnm3 = __Bnm2;
          __Bnm2 = __Bnm1;
          __Bnm1 = __Bn;
          __Anm3 = __Anm2;
          __Anm2 = __Anm1;
          __Anm1 = __An;
        }
      if (__n >= __nmax)
        std::__throw_runtime_error(("Iteration failed to converge " "in __hyperg_luke."));
      return __F;
    }
    template<typename _Tp>
    _Tp
    __hyperg_reflect(_Tp __a, _Tp __b, _Tp __c, _Tp __x)
    {
      const _Tp __d = __c - __a - __b;
      const int __intd = std::floor(__d + _Tp(0.5L));
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __toler = _Tp(1000) * __eps;
      const _Tp __log_max = std::log(std::numeric_limits<_Tp>::max());
      const bool __d_integer = (std::abs(__d - __intd) < __toler);
      if (__d_integer)
        {
          const _Tp __ln_omx = std::log(_Tp(1) - __x);
          const _Tp __ad = std::abs(__d);
          _Tp __F1, __F2;
          _Tp __d1, __d2;
          if (__d >= _Tp(0))
            {
              __d1 = __d;
              __d2 = _Tp(0);
            }
          else
            {
              __d1 = _Tp(0);
              __d2 = __d;
            }
          const _Tp __lng_c = __log_gamma(__c);
          if (__ad < __eps)
            {
              __F1 = _Tp(0);
            }
          else
            {
              bool __ok_d1 = true;
              _Tp __lng_ad, __lng_ad1, __lng_bd1;
              try
                {
                  __lng_ad = __log_gamma(__ad);
                  __lng_ad1 = __log_gamma(__a + __d1);
                  __lng_bd1 = __log_gamma(__b + __d1);
                }
              catch(...)
                {
                  __ok_d1 = false;
                }
              if (__ok_d1)
                {
                  _Tp __sum1 = _Tp(1);
                  _Tp __term = _Tp(1);
                  _Tp __ln_pre1 = __lng_ad + __lng_c + __d2 * __ln_omx
                                - __lng_ad1 - __lng_bd1;
                  for (int __i = 1; __i < __ad; ++__i)
                    {
                      const int __j = __i - 1;
                      __term *= (__a + __d2 + __j) * (__b + __d2 + __j)
                              / (_Tp(1) + __d2 + __j) / __i * (_Tp(1) - __x);
                      __sum1 += __term;
                    }
                  if (__ln_pre1 > __log_max)
                    std::__throw_runtime_error(("Overflow of gamma functions" " in __hyperg_luke."));
                  else
                    __F1 = std::exp(__ln_pre1) * __sum1;
                }
              else
                {
                  __F1 = _Tp(0);
                }
            }
          bool __ok_d2 = true;
          _Tp __lng_ad2, __lng_bd2;
          try
            {
              __lng_ad2 = __log_gamma(__a + __d2);
              __lng_bd2 = __log_gamma(__b + __d2);
            }
          catch(...)
            {
              __ok_d2 = false;
            }
          if (__ok_d2)
            {
              const int __maxiter = 2000;
              const _Tp __psi_1 = -__numeric_constants<_Tp>::__gamma_e();
              const _Tp __psi_1pd = __psi(_Tp(1) + __ad);
              const _Tp __psi_apd1 = __psi(__a + __d1);
              const _Tp __psi_bpd1 = __psi(__b + __d1);
              _Tp __psi_term = __psi_1 + __psi_1pd - __psi_apd1
                             - __psi_bpd1 - __ln_omx;
              _Tp __fact = _Tp(1);
              _Tp __sum2 = __psi_term;
              _Tp __ln_pre2 = __lng_c + __d1 * __ln_omx
                            - __lng_ad2 - __lng_bd2;
              int __j;
              for (__j = 1; __j < __maxiter; ++__j)
                {
                  const _Tp __term1 = _Tp(1) / _Tp(__j)
                                    + _Tp(1) / (__ad + __j);
                  const _Tp __term2 = _Tp(1) / (__a + __d1 + _Tp(__j - 1))
                                    + _Tp(1) / (__b + __d1 + _Tp(__j - 1));
                  __psi_term += __term1 - __term2;
                  __fact *= (__a + __d1 + _Tp(__j - 1))
                          * (__b + __d1 + _Tp(__j - 1))
                          / ((__ad + __j) * __j) * (_Tp(1) - __x);
                  const _Tp __delta = __fact * __psi_term;
                  __sum2 += __delta;
                  if (std::abs(__delta) < __eps * std::abs(__sum2))
                    break;
                }
              if (__j == __maxiter)
                std::__throw_runtime_error(("Sum F2 failed to converge " "in __hyperg_reflect"));
              if (__sum2 == _Tp(0))
                __F2 = _Tp(0);
              else
                __F2 = std::exp(__ln_pre2) * __sum2;
            }
          else
            {
              __F2 = _Tp(0);
            }
          const _Tp __sgn_2 = (__intd % 2 == 1 ? -_Tp(1) : _Tp(1));
          const _Tp __F = __F1 + __sgn_2 * __F2;
          return __F;
        }
      else
        {
          bool __ok1 = true;
          _Tp __sgn_g1ca = _Tp(0), __ln_g1ca = _Tp(0);
          _Tp __sgn_g1cb = _Tp(0), __ln_g1cb = _Tp(0);
          try
            {
              __sgn_g1ca = __log_gamma_sign(__c - __a);
              __ln_g1ca = __log_gamma(__c - __a);
              __sgn_g1cb = __log_gamma_sign(__c - __b);
              __ln_g1cb = __log_gamma(__c - __b);
            }
          catch(...)
            {
              __ok1 = false;
            }
          bool __ok2 = true;
          _Tp __sgn_g2a = _Tp(0), __ln_g2a = _Tp(0);
          _Tp __sgn_g2b = _Tp(0), __ln_g2b = _Tp(0);
          try
            {
              __sgn_g2a = __log_gamma_sign(__a);
              __ln_g2a = __log_gamma(__a);
              __sgn_g2b = __log_gamma_sign(__b);
              __ln_g2b = __log_gamma(__b);
            }
          catch(...)
            {
              __ok2 = false;
            }
          const _Tp __sgn_gc = __log_gamma_sign(__c);
          const _Tp __ln_gc = __log_gamma(__c);
          const _Tp __sgn_gd = __log_gamma_sign(__d);
          const _Tp __ln_gd = __log_gamma(__d);
          const _Tp __sgn_gmd = __log_gamma_sign(-__d);
          const _Tp __ln_gmd = __log_gamma(-__d);
          const _Tp __sgn1 = __sgn_gc * __sgn_gd * __sgn_g1ca * __sgn_g1cb;
          const _Tp __sgn2 = __sgn_gc * __sgn_gmd * __sgn_g2a * __sgn_g2b;
          _Tp __pre1, __pre2;
          if (__ok1 && __ok2)
            {
              _Tp __ln_pre1 = __ln_gc + __ln_gd - __ln_g1ca - __ln_g1cb;
              _Tp __ln_pre2 = __ln_gc + __ln_gmd - __ln_g2a - __ln_g2b
                            + __d * std::log(_Tp(1) - __x);
              if (__ln_pre1 < __log_max && __ln_pre2 < __log_max)
                {
                  __pre1 = std::exp(__ln_pre1);
                  __pre2 = std::exp(__ln_pre2);
                  __pre1 *= __sgn1;
                  __pre2 *= __sgn2;
                }
              else
                {
                  std::__throw_runtime_error(("Overflow of gamma functions " "in __hyperg_reflect"));
                }
            }
          else if (__ok1 && !__ok2)
            {
              _Tp __ln_pre1 = __ln_gc + __ln_gd - __ln_g1ca - __ln_g1cb;
              if (__ln_pre1 < __log_max)
                {
                  __pre1 = std::exp(__ln_pre1);
                  __pre1 *= __sgn1;
                  __pre2 = _Tp(0);
                }
              else
                {
                  std::__throw_runtime_error(("Overflow of gamma functions " "in __hyperg_reflect"));
                }
            }
          else if (!__ok1 && __ok2)
            {
              _Tp __ln_pre2 = __ln_gc + __ln_gmd - __ln_g2a - __ln_g2b
                            + __d * std::log(_Tp(1) - __x);
              if (__ln_pre2 < __log_max)
                {
                  __pre1 = _Tp(0);
                  __pre2 = std::exp(__ln_pre2);
                  __pre2 *= __sgn2;
                }
              else
                {
                  std::__throw_runtime_error(("Overflow of gamma functions " "in __hyperg_reflect"));
                }
            }
          else
            {
              __pre1 = _Tp(0);
              __pre2 = _Tp(0);
              std::__throw_runtime_error(("Underflow of gamma functions " "in __hyperg_reflect"));
            }
          const _Tp __F1 = __hyperg_series(__a, __b, _Tp(1) - __d,
                                           _Tp(1) - __x);
          const _Tp __F2 = __hyperg_series(__c - __a, __c - __b, _Tp(1) + __d,
                                           _Tp(1) - __x);
          const _Tp __F = __pre1 * __F1 + __pre2 * __F2;
          return __F;
        }
    }
    template<typename _Tp>
    _Tp
    __hyperg(_Tp __a, _Tp __b, _Tp __c, _Tp __x)
    {
      const _Tp __a_nint = ::std::nearbyint(__a);
      const _Tp __b_nint = ::std::nearbyint(__b);
      const _Tp __c_nint = ::std::nearbyint(__c);
      const _Tp __toler = _Tp(1000) * std::numeric_limits<_Tp>::epsilon();
      if (std::abs(__x) >= _Tp(1))
        std::__throw_domain_error(("Argument outside unit circle " "in __hyperg."));
      else if (__isnan(__a) || __isnan(__b)
            || __isnan(__c) || __isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__c_nint == __c && __c_nint <= _Tp(0))
        return std::numeric_limits<_Tp>::infinity();
      else if (std::abs(__c - __b) < __toler || std::abs(__c - __a) < __toler)
        return std::pow(_Tp(1) - __x, __c - __a - __b);
      else if (__a >= _Tp(0) && __b >= _Tp(0) && __c >= _Tp(0)
            && __x >= _Tp(0) && __x < _Tp(0.995L))
        return __hyperg_series(__a, __b, __c, __x);
      else if (std::abs(__a) < _Tp(10) && std::abs(__b) < _Tp(10))
        {
          if (__a < _Tp(0) && std::abs(__a - __a_nint) < __toler)
            return __hyperg_series(__a_nint, __b, __c, __x);
          else if (__b < _Tp(0) && std::abs(__b - __b_nint) < __toler)
            return __hyperg_series(__a, __b_nint, __c, __x);
          else if (__x < -_Tp(0.25L))
            return __hyperg_luke(__a, __b, __c, __x);
          else if (__x < _Tp(0.5L))
            return __hyperg_series(__a, __b, __c, __x);
          else
            if (std::abs(__c) > _Tp(10))
              return __hyperg_series(__a, __b, __c, __x);
            else
              return __hyperg_reflect(__a, __b, __c, __x);
        }
      else
        return __hyperg_luke(__a, __b, __c, __x);
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tp>
    _Tp
    __poly_legendre_p(unsigned int __l, _Tp __x)
    {
      if (__isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__x == +_Tp(1))
        return +_Tp(1);
      else if (__x == -_Tp(1))
        return (__l % 2 == 1 ? -_Tp(1) : +_Tp(1));
      else
        {
          _Tp __p_lm2 = _Tp(1);
          if (__l == 0)
            return __p_lm2;
          _Tp __p_lm1 = __x;
          if (__l == 1)
            return __p_lm1;
          _Tp __p_l = 0;
          for (unsigned int __ll = 2; __ll <= __l; ++__ll)
            {
              __p_l = _Tp(2) * __x * __p_lm1 - __p_lm2
                    - (__x * __p_lm1 - __p_lm2) / _Tp(__ll);
              __p_lm2 = __p_lm1;
              __p_lm1 = __p_l;
            }
          return __p_l;
        }
    }
    template<typename _Tp>
    _Tp
    __assoc_legendre_p(unsigned int __l, unsigned int __m, _Tp __x,
         _Tp __phase = _Tp(+1))
    {
      if (__m > __l)
        return _Tp(0);
      else if (__isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__m == 0)
        return __poly_legendre_p(__l, __x);
      else
        {
          _Tp __p_mm = _Tp(1);
          if (__m > 0)
            {
              _Tp __root = std::sqrt(_Tp(1) - __x) * std::sqrt(_Tp(1) + __x);
              _Tp __fact = _Tp(1);
              for (unsigned int __i = 1; __i <= __m; ++__i)
                {
                  __p_mm *= __phase * __fact * __root;
                  __fact += _Tp(2);
                }
            }
          if (__l == __m)
            return __p_mm;
          _Tp __p_mp1m = _Tp(2 * __m + 1) * __x * __p_mm;
          if (__l == __m + 1)
            return __p_mp1m;
          _Tp __p_lm2m = __p_mm;
          _Tp __P_lm1m = __p_mp1m;
          _Tp __p_lm = _Tp(0);
          for (unsigned int __j = __m + 2; __j <= __l; ++__j)
            {
              __p_lm = (_Tp(2 * __j - 1) * __x * __P_lm1m
                      - _Tp(__j + __m - 1) * __p_lm2m) / _Tp(__j - __m);
              __p_lm2m = __P_lm1m;
              __P_lm1m = __p_lm;
            }
          return __p_lm;
        }
    }
    template <typename _Tp>
    _Tp
    __sph_legendre(unsigned int __l, unsigned int __m, _Tp __theta)
    {
      if (__isnan(__theta))
        return std::numeric_limits<_Tp>::quiet_NaN();
      const _Tp __x = std::cos(__theta);
      if (__m > __l)
        return _Tp(0);
      else if (__m == 0)
        {
          _Tp __P = __poly_legendre_p(__l, __x);
          _Tp __fact = std::sqrt(_Tp(2 * __l + 1)
                     / (_Tp(4) * __numeric_constants<_Tp>::__pi()));
          __P *= __fact;
          return __P;
        }
      else if (__x == _Tp(1) || __x == -_Tp(1))
        {
          return _Tp(0);
        }
      else
        {
          const _Tp __sgn = ( __m % 2 == 1 ? -_Tp(1) : _Tp(1));
          const _Tp __y_mp1m_factor = __x * std::sqrt(_Tp(2 * __m + 3));
          const _Tp __lncirc = ::std::log1p(-__x * __x);
          const _Tp __lnpoch = ::std::lgamma(_Tp(__m + _Tp(0.5L)))
                             - ::std::lgamma(_Tp(__m));
          const _Tp __lnpre_val =
                    -_Tp(0.25L) * __numeric_constants<_Tp>::__lnpi()
                    + _Tp(0.5L) * (__lnpoch + __m * __lncirc);
          const _Tp __sr = std::sqrt((_Tp(2) + _Tp(1) / __m)
                         / (_Tp(4) * __numeric_constants<_Tp>::__pi()));
          _Tp __y_mm = __sgn * __sr * std::exp(__lnpre_val);
          _Tp __y_mp1m = __y_mp1m_factor * __y_mm;
          if (__l == __m)
            return __y_mm;
          else if (__l == __m + 1)
            return __y_mp1m;
          else
            {
              _Tp __y_lm = _Tp(0);
              for (unsigned int __ll = __m + 2; __ll <= __l; ++__ll)
                {
                  const _Tp __rat1 = _Tp(__ll - __m) / _Tp(__ll + __m);
                  const _Tp __rat2 = _Tp(__ll - __m - 1) / _Tp(__ll + __m - 1);
                  const _Tp __fact1 = std::sqrt(__rat1 * _Tp(2 * __ll + 1)
                                                       * _Tp(2 * __ll - 1));
                  const _Tp __fact2 = std::sqrt(__rat1 * __rat2 * _Tp(2 * __ll + 1)
                                                                / _Tp(2 * __ll - 3));
                  __y_lm = (__x * __y_mp1m * __fact1
                         - (__ll + __m - 1) * __y_mm * __fact2) / _Tp(__ll - __m);
                  __y_mm = __y_mp1m;
                  __y_mp1m = __y_lm;
                }
              return __y_lm;
            }
        }
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template <typename _Tp>
    void
    __bessel_ik(_Tp __nu, _Tp __x,
                _Tp & __Inu, _Tp & __Knu, _Tp & __Ipnu, _Tp & __Kpnu)
    {
      if (__x == _Tp(0))
        {
          if (__nu == _Tp(0))
            {
              __Inu = _Tp(1);
              __Ipnu = _Tp(0);
            }
          else if (__nu == _Tp(1))
            {
              __Inu = _Tp(0);
              __Ipnu = _Tp(0.5L);
            }
          else
            {
              __Inu = _Tp(0);
              __Ipnu = _Tp(0);
            }
          __Knu = std::numeric_limits<_Tp>::infinity();
          __Kpnu = -std::numeric_limits<_Tp>::infinity();
          return;
        }
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __fp_min = _Tp(10) * std::numeric_limits<_Tp>::epsilon();
      const int __max_iter = 15000;
      const _Tp __x_min = _Tp(2);
      const int __nl = static_cast<int>(__nu + _Tp(0.5L));
      const _Tp __mu = __nu - __nl;
      const _Tp __mu2 = __mu * __mu;
      const _Tp __xi = _Tp(1) / __x;
      const _Tp __xi2 = _Tp(2) * __xi;
      _Tp __h = __nu * __xi;
      if ( __h < __fp_min )
        __h = __fp_min;
      _Tp __b = __xi2 * __nu;
      _Tp __d = _Tp(0);
      _Tp __c = __h;
      int __i;
      for ( __i = 1; __i <= __max_iter; ++__i )
        {
          __b += __xi2;
          __d = _Tp(1) / (__b + __d);
          __c = __b + _Tp(1) / __c;
          const _Tp __del = __c * __d;
          __h *= __del;
          if (std::abs(__del - _Tp(1)) < __eps)
            break;
        }
      if (__i > __max_iter)
        std::__throw_runtime_error(("Argument x too large " "in __bessel_ik; " "try asymptotic expansion."));
      _Tp __Inul = __fp_min;
      _Tp __Ipnul = __h * __Inul;
      _Tp __Inul1 = __Inul;
      _Tp __Ipnu1 = __Ipnul;
      _Tp __fact = __nu * __xi;
      for (int __l = __nl; __l >= 1; --__l)
        {
          const _Tp __Inutemp = __fact * __Inul + __Ipnul;
          __fact -= __xi;
          __Ipnul = __fact * __Inutemp + __Inul;
          __Inul = __Inutemp;
        }
      _Tp __f = __Ipnul / __Inul;
      _Tp __Kmu, __Knu1;
      if (__x < __x_min)
        {
          const _Tp __x2 = __x / _Tp(2);
          const _Tp __pimu = __numeric_constants<_Tp>::__pi() * __mu;
          const _Tp __fact = (std::abs(__pimu) < __eps
                            ? _Tp(1) : __pimu / std::sin(__pimu));
          _Tp __d = -std::log(__x2);
          _Tp __e = __mu * __d;
          const _Tp __fact2 = (std::abs(__e) < __eps
                            ? _Tp(1) : std::sinh(__e) / __e);
          _Tp __gam1, __gam2, __gampl, __gammi;
          __gamma_temme(__mu, __gam1, __gam2, __gampl, __gammi);
          _Tp __ff = __fact
                   * (__gam1 * std::cosh(__e) + __gam2 * __fact2 * __d);
          _Tp __sum = __ff;
          __e = std::exp(__e);
          _Tp __p = __e / (_Tp(2) * __gampl);
          _Tp __q = _Tp(1) / (_Tp(2) * __e * __gammi);
          _Tp __c = _Tp(1);
          __d = __x2 * __x2;
          _Tp __sum1 = __p;
          int __i;
          for (__i = 1; __i <= __max_iter; ++__i)
            {
              __ff = (__i * __ff + __p + __q) / (__i * __i - __mu2);
              __c *= __d / __i;
              __p /= __i - __mu;
              __q /= __i + __mu;
              const _Tp __del = __c * __ff;
              __sum += __del;
              const _Tp __del1 = __c * (__p - __i * __ff);
              __sum1 += __del1;
              if (std::abs(__del) < __eps * std::abs(__sum))
                break;
            }
          if (__i > __max_iter)
            std::__throw_runtime_error(("Bessel k series failed to converge " "in __bessel_ik."));
          __Kmu = __sum;
          __Knu1 = __sum1 * __xi2;
        }
      else
        {
          _Tp __b = _Tp(2) * (_Tp(1) + __x);
          _Tp __d = _Tp(1) / __b;
          _Tp __delh = __d;
          _Tp __h = __delh;
          _Tp __q1 = _Tp(0);
          _Tp __q2 = _Tp(1);
          _Tp __a1 = _Tp(0.25L) - __mu2;
          _Tp __q = __c = __a1;
          _Tp __a = -__a1;
          _Tp __s = _Tp(1) + __q * __delh;
          int __i;
          for (__i = 2; __i <= __max_iter; ++__i)
            {
              __a -= 2 * (__i - 1);
              __c = -__a * __c / __i;
              const _Tp __qnew = (__q1 - __b * __q2) / __a;
              __q1 = __q2;
              __q2 = __qnew;
              __q += __c * __qnew;
              __b += _Tp(2);
              __d = _Tp(1) / (__b + __a * __d);
              __delh = (__b * __d - _Tp(1)) * __delh;
              __h += __delh;
              const _Tp __dels = __q * __delh;
              __s += __dels;
              if ( std::abs(__dels / __s) < __eps )
                break;
            }
          if (__i > __max_iter)
            std::__throw_runtime_error(("Steed's method failed " "in __bessel_ik."));
          __h = __a1 * __h;
          __Kmu = std::sqrt(__numeric_constants<_Tp>::__pi() / (_Tp(2) * __x))
                * std::exp(-__x) / __s;
          __Knu1 = __Kmu * (__mu + __x + _Tp(0.5L) - __h) * __xi;
        }
      _Tp __Kpmu = __mu * __xi * __Kmu - __Knu1;
      _Tp __Inumu = __xi / (__f * __Kmu - __Kpmu);
      __Inu = __Inumu * __Inul1 / __Inul;
      __Ipnu = __Inumu * __Ipnu1 / __Inul;
      for ( __i = 1; __i <= __nl; ++__i )
        {
          const _Tp __Knutemp = (__mu + __i) * __xi2 * __Knu1 + __Kmu;
          __Kmu = __Knu1;
          __Knu1 = __Knutemp;
        }
      __Knu = __Kmu;
      __Kpnu = __nu * __xi * __Kmu - __Knu1;
      return;
    }
    template<typename _Tp>
    _Tp
    __cyl_bessel_i(_Tp __nu, _Tp __x)
    {
      if (__nu < _Tp(0) || __x < _Tp(0))
        std::__throw_domain_error(("Bad argument " "in __cyl_bessel_i."));
      else if (__isnan(__nu) || __isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__x * __x < _Tp(10) * (__nu + _Tp(1)))
        return __cyl_bessel_ij_series(__nu, __x, +_Tp(1), 200);
      else
        {
          _Tp __I_nu, __K_nu, __Ip_nu, __Kp_nu;
          __bessel_ik(__nu, __x, __I_nu, __K_nu, __Ip_nu, __Kp_nu);
          return __I_nu;
        }
    }
    template<typename _Tp>
    _Tp
    __cyl_bessel_k(_Tp __nu, _Tp __x)
    {
      if (__nu < _Tp(0) || __x < _Tp(0))
        std::__throw_domain_error(("Bad argument " "in __cyl_bessel_k."));
      else if (__isnan(__nu) || __isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else
        {
          _Tp __I_nu, __K_nu, __Ip_nu, __Kp_nu;
          __bessel_ik(__nu, __x, __I_nu, __K_nu, __Ip_nu, __Kp_nu);
          return __K_nu;
        }
    }
    template <typename _Tp>
    void
    __sph_bessel_ik(unsigned int __n, _Tp __x,
                    _Tp & __i_n, _Tp & __k_n, _Tp & __ip_n, _Tp & __kp_n)
    {
      const _Tp __nu = _Tp(__n) + _Tp(0.5L);
      _Tp __I_nu, __Ip_nu, __K_nu, __Kp_nu;
      __bessel_ik(__nu, __x, __I_nu, __K_nu, __Ip_nu, __Kp_nu);
      const _Tp __factor = __numeric_constants<_Tp>::__sqrtpio2()
                         / std::sqrt(__x);
      __i_n = __factor * __I_nu;
      __k_n = __factor * __K_nu;
      __ip_n = __factor * __Ip_nu - __i_n / (_Tp(2) * __x);
      __kp_n = __factor * __Kp_nu - __k_n / (_Tp(2) * __x);
      return;
    }
    template <typename _Tp>
    void
    __airy(_Tp __x, _Tp & __Ai, _Tp & __Bi, _Tp & __Aip, _Tp & __Bip)
    {
      const _Tp __absx = std::abs(__x);
      const _Tp __rootx = std::sqrt(__absx);
      const _Tp __z = _Tp(2) * __absx * __rootx / _Tp(3);
      const _Tp _S_inf = std::numeric_limits<_Tp>::infinity();
      if (__isnan(__x))
        __Bip = __Aip = __Bi = __Ai = std::numeric_limits<_Tp>::quiet_NaN();
      else if (__z == _S_inf)
        {
   __Aip = __Ai = _Tp(0);
   __Bip = __Bi = _S_inf;
 }
      else if (__z == -_S_inf)
 __Bip = __Aip = __Bi = __Ai = _Tp(0);
      else if (__x > _Tp(0))
        {
          _Tp __I_nu, __Ip_nu, __K_nu, __Kp_nu;
          __bessel_ik(_Tp(1) / _Tp(3), __z, __I_nu, __K_nu, __Ip_nu, __Kp_nu);
          __Ai = __rootx * __K_nu
               / (__numeric_constants<_Tp>::__sqrt3()
                * __numeric_constants<_Tp>::__pi());
          __Bi = __rootx * (__K_nu / __numeric_constants<_Tp>::__pi()
                 + _Tp(2) * __I_nu / __numeric_constants<_Tp>::__sqrt3());
          __bessel_ik(_Tp(2) / _Tp(3), __z, __I_nu, __K_nu, __Ip_nu, __Kp_nu);
          __Aip = -__x * __K_nu
                / (__numeric_constants<_Tp>::__sqrt3()
                 * __numeric_constants<_Tp>::__pi());
          __Bip = __x * (__K_nu / __numeric_constants<_Tp>::__pi()
                      + _Tp(2) * __I_nu
                      / __numeric_constants<_Tp>::__sqrt3());
        }
      else if (__x < _Tp(0))
        {
          _Tp __J_nu, __Jp_nu, __N_nu, __Np_nu;
          __bessel_jn(_Tp(1) / _Tp(3), __z, __J_nu, __N_nu, __Jp_nu, __Np_nu);
          __Ai = __rootx * (__J_nu
                    - __N_nu / __numeric_constants<_Tp>::__sqrt3()) / _Tp(2);
          __Bi = -__rootx * (__N_nu
                    + __J_nu / __numeric_constants<_Tp>::__sqrt3()) / _Tp(2);
          __bessel_jn(_Tp(2) / _Tp(3), __z, __J_nu, __N_nu, __Jp_nu, __Np_nu);
          __Aip = __absx * (__N_nu / __numeric_constants<_Tp>::__sqrt3()
                          + __J_nu) / _Tp(2);
          __Bip = __absx * (__J_nu / __numeric_constants<_Tp>::__sqrt3()
                          - __N_nu) / _Tp(2);
        }
      else
        {
          __Ai = _Tp(0.35502805388781723926L);
          __Bi = __Ai * __numeric_constants<_Tp>::__sqrt3();
          __Aip = -_Tp(0.25881940379280679840L);
          __Bip = -__Aip * __numeric_constants<_Tp>::__sqrt3();
        }
      return;
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tp>
    _Tp
    __poly_hermite_recursion(unsigned int __n, _Tp __x)
    {
      _Tp __H_0 = 1;
      if (__n == 0)
        return __H_0;
      _Tp __H_1 = 2 * __x;
      if (__n == 1)
        return __H_1;
      _Tp __H_n, __H_nm1, __H_nm2;
      unsigned int __i;
      for (__H_nm2 = __H_0, __H_nm1 = __H_1, __i = 2; __i <= __n; ++__i)
        {
          __H_n = 2 * (__x * __H_nm1 - (__i - 1) * __H_nm2);
          __H_nm2 = __H_nm1;
          __H_nm1 = __H_n;
        }
      return __H_n;
    }
    template<typename _Tp>
    inline _Tp
    __poly_hermite(unsigned int __n, _Tp __x)
    {
      if (__isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else
        return __poly_hermite_recursion(__n, __x);
    }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tpa, typename _Tp>
    _Tp
    __poly_laguerre_large_n(unsigned __n, _Tpa __alpha1, _Tp __x)
    {
      const _Tp __a = -_Tp(__n);
      const _Tp __b = _Tp(__alpha1) + _Tp(1);
      const _Tp __eta = _Tp(2) * __b - _Tp(4) * __a;
      const _Tp __cos2th = __x / __eta;
      const _Tp __sin2th = _Tp(1) - __cos2th;
      const _Tp __th = std::acos(std::sqrt(__cos2th));
      const _Tp __pre_h = __numeric_constants<_Tp>::__pi_2()
                        * __numeric_constants<_Tp>::__pi_2()
                        * __eta * __eta * __cos2th * __sin2th;
      const _Tp __lg_b = ::std::lgamma(_Tp(__n) + __b);
      const _Tp __lnfact = ::std::lgamma(_Tp(__n + 1));
      _Tp __pre_term1 = _Tp(0.5L) * (_Tp(1) - __b)
                      * std::log(_Tp(0.25L) * __x * __eta);
      _Tp __pre_term2 = _Tp(0.25L) * std::log(__pre_h);
      _Tp __lnpre = __lg_b - __lnfact + _Tp(0.5L) * __x
                      + __pre_term1 - __pre_term2;
      _Tp __ser_term1 = std::sin(__a * __numeric_constants<_Tp>::__pi());
      _Tp __ser_term2 = std::sin(_Tp(0.25L) * __eta
                              * (_Tp(2) * __th
                               - std::sin(_Tp(2) * __th))
                               + __numeric_constants<_Tp>::__pi_4());
      _Tp __ser = __ser_term1 + __ser_term2;
      return std::exp(__lnpre) * __ser;
    }
    template<typename _Tpa, typename _Tp>
    _Tp
    __poly_laguerre_hyperg(unsigned int __n, _Tpa __alpha1, _Tp __x)
    {
      const _Tp __b = _Tp(__alpha1) + _Tp(1);
      const _Tp __mx = -__x;
      const _Tp __tc_sgn = (__x < _Tp(0) ? _Tp(1)
                         : ((__n % 2 == 1) ? -_Tp(1) : _Tp(1)));
      _Tp __tc = _Tp(1);
      const _Tp __ax = std::abs(__x);
      for (unsigned int __k = 1; __k <= __n; ++__k)
        __tc *= (__ax / __k);
      _Tp __term = __tc * __tc_sgn;
      _Tp __sum = __term;
      for (int __k = int(__n) - 1; __k >= 0; --__k)
        {
          __term *= ((__b + _Tp(__k)) / _Tp(int(__n) - __k))
                  * _Tp(__k + 1) / __mx;
          __sum += __term;
        }
      return __sum;
    }
    template<typename _Tpa, typename _Tp>
    _Tp
    __poly_laguerre_recursion(unsigned int __n, _Tpa __alpha1, _Tp __x)
    {
      _Tp __l_0 = _Tp(1);
      if (__n == 0)
        return __l_0;
      _Tp __l_1 = -__x + _Tp(1) + _Tp(__alpha1);
      if (__n == 1)
        return __l_1;
      _Tp __l_n2 = __l_0;
      _Tp __l_n1 = __l_1;
      _Tp __l_n = _Tp(0);
      for (unsigned int __nn = 2; __nn <= __n; ++__nn)
        {
            __l_n = (_Tp(2 * __nn - 1) + _Tp(__alpha1) - __x)
                  * __l_n1 / _Tp(__nn)
                  - (_Tp(__nn - 1) + _Tp(__alpha1)) * __l_n2 / _Tp(__nn);
            __l_n2 = __l_n1;
            __l_n1 = __l_n;
        }
      return __l_n;
    }
    template<typename _Tpa, typename _Tp>
    _Tp
    __poly_laguerre(unsigned int __n, _Tpa __alpha1, _Tp __x)
    {
      if (__x < _Tp(0))
        std::__throw_domain_error(("Negative argument " "in __poly_laguerre."));
      else if (__isnan(__x))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__n == 0)
        return _Tp(1);
      else if (__n == 1)
        return _Tp(1) + _Tp(__alpha1) - __x;
      else if (__x == _Tp(0))
        {
          _Tp __prod = _Tp(__alpha1) + _Tp(1);
          for (unsigned int __k = 2; __k <= __n; ++__k)
            __prod *= (_Tp(__alpha1) + _Tp(__k)) / _Tp(__k);
          return __prod;
        }
      else if (__n > 10000000 && _Tp(__alpha1) > -_Tp(1)
            && __x < _Tp(2) * (_Tp(__alpha1) + _Tp(1)) + _Tp(4 * __n))
        return __poly_laguerre_large_n(__n, __alpha1, __x);
      else if (_Tp(__alpha1) >= _Tp(0)
           || (__x > _Tp(0) && _Tp(__alpha1) < -_Tp(__n + 1)))
        return __poly_laguerre_recursion(__n, __alpha1, __x);
      else
        return __poly_laguerre_hyperg(__n, __alpha1, __x);
    }
    template<typename _Tp>
    inline _Tp
    __assoc_laguerre(unsigned int __n, unsigned int __m, _Tp __x)
    { return __poly_laguerre<unsigned int, _Tp>(__n, __m, __x); }
    template<typename _Tp>
    inline _Tp
    __laguerre(unsigned int __n, _Tp __x)
    { return __poly_laguerre<unsigned int, _Tp>(__n, 0, __x); }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace __detail
  {
    template<typename _Tp>
    _Tp
    __riemann_zeta_sum(_Tp __s)
    {
      if (__s < _Tp(1))
        std::__throw_domain_error(("Bad argument in zeta sum."));
      const unsigned int __max_iter = 10000;
      _Tp __zeta = _Tp(0);
      for (unsigned int __k = 1; __k < __max_iter; ++__k)
        {
          _Tp __term = std::pow(static_cast<_Tp>(__k), -__s);
          if (__term < std::numeric_limits<_Tp>::epsilon())
            {
              break;
            }
          __zeta += __term;
        }
      return __zeta;
    }
    template<typename _Tp>
    _Tp
    __riemann_zeta_alt(_Tp __s)
    {
      _Tp __sgn = _Tp(1);
      _Tp __zeta = _Tp(0);
      for (unsigned int __i = 1; __i < 10000000; ++__i)
        {
          _Tp __term = __sgn / std::pow(__i, __s);
          if (std::abs(__term) < std::numeric_limits<_Tp>::epsilon())
            break;
          __zeta += __term;
          __sgn *= _Tp(-1);
        }
      __zeta /= _Tp(1) - std::pow(_Tp(2), _Tp(1) - __s);
      return __zeta;
    }
    template<typename _Tp>
    _Tp
    __riemann_zeta_glob(_Tp __s)
    {
      _Tp __zeta = _Tp(0);
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __max_bincoeff = std::numeric_limits<_Tp>::max_exponent10
                               * std::log(_Tp(10)) - _Tp(1);
      if (__s < _Tp(0))
        {
          if (::std::fmod(__s,_Tp(2)) == _Tp(0))
            return _Tp(0);
          else
            {
              _Tp __zeta = __riemann_zeta_glob(_Tp(1) - __s);
              __zeta *= std::pow(_Tp(2)
                     * __numeric_constants<_Tp>::__pi(), __s)
                     * std::sin(__numeric_constants<_Tp>::__pi_2() * __s)
                     * std::exp(::std::lgamma(_Tp(1) - __s))
                     / __numeric_constants<_Tp>::__pi();
              return __zeta;
            }
        }
      _Tp __num = _Tp(0.5L);
      const unsigned int __maxit = 10000;
      for (unsigned int __i = 0; __i < __maxit; ++__i)
        {
          bool __punt = false;
          _Tp __sgn = _Tp(1);
          _Tp __term = _Tp(0);
          for (unsigned int __j = 0; __j <= __i; ++__j)
            {
              _Tp __bincoeff = ::std::lgamma(_Tp(1 + __i))
                              - ::std::lgamma(_Tp(1 + __j))
                              - ::std::lgamma(_Tp(1 + __i - __j));
              if (__bincoeff > __max_bincoeff)
                {
                  __punt = true;
                  break;
                }
              __bincoeff = std::exp(__bincoeff);
              __term += __sgn * __bincoeff * std::pow(_Tp(1 + __j), -__s);
              __sgn *= _Tp(-1);
            }
          if (__punt)
            break;
          __term *= __num;
          __zeta += __term;
          if (std::abs(__term/__zeta) < __eps)
            break;
          __num *= _Tp(0.5L);
        }
      __zeta /= _Tp(1) - std::pow(_Tp(2), _Tp(1) - __s);
      return __zeta;
    }
    template<typename _Tp>
    _Tp
    __riemann_zeta_product(_Tp __s)
    {
      static const _Tp __prime[] = {
        _Tp(2), _Tp(3), _Tp(5), _Tp(7), _Tp(11), _Tp(13), _Tp(17), _Tp(19),
        _Tp(23), _Tp(29), _Tp(31), _Tp(37), _Tp(41), _Tp(43), _Tp(47),
        _Tp(53), _Tp(59), _Tp(61), _Tp(67), _Tp(71), _Tp(73), _Tp(79),
        _Tp(83), _Tp(89), _Tp(97), _Tp(101), _Tp(103), _Tp(107), _Tp(109)
      };
      static const unsigned int __num_primes = sizeof(__prime) / sizeof(_Tp);
      _Tp __zeta = _Tp(1);
      for (unsigned int __i = 0; __i < __num_primes; ++__i)
        {
          const _Tp __fact = _Tp(1) - std::pow(__prime[__i], -__s);
          __zeta *= __fact;
          if (_Tp(1) - __fact < std::numeric_limits<_Tp>::epsilon())
            break;
        }
      __zeta = _Tp(1) / __zeta;
      return __zeta;
    }
    template<typename _Tp>
    _Tp
    __riemann_zeta(_Tp __s)
    {
      if (__isnan(__s))
        return std::numeric_limits<_Tp>::quiet_NaN();
      else if (__s == _Tp(1))
        return std::numeric_limits<_Tp>::infinity();
      else if (__s < -_Tp(19))
        {
          _Tp __zeta = __riemann_zeta_product(_Tp(1) - __s);
          __zeta *= std::pow(_Tp(2) * __numeric_constants<_Tp>::__pi(), __s)
                 * std::sin(__numeric_constants<_Tp>::__pi_2() * __s)
                 * std::exp(::std::lgamma(_Tp(1) - __s))
                 / __numeric_constants<_Tp>::__pi();
          return __zeta;
        }
      else if (__s < _Tp(20))
        {
          bool __glob = true;
          if (__glob)
            return __riemann_zeta_glob(__s);
          else
            {
              if (__s > _Tp(1))
                return __riemann_zeta_sum(__s);
              else
                {
                  _Tp __zeta = std::pow(_Tp(2)
                                * __numeric_constants<_Tp>::__pi(), __s)
                         * std::sin(__numeric_constants<_Tp>::__pi_2() * __s)
                             * ::std::tgamma(_Tp(1) - __s)
                             * __riemann_zeta_sum(_Tp(1) - __s);
                  return __zeta;
                }
            }
        }
      else
        return __riemann_zeta_product(__s);
    }
    template<typename _Tp>
    _Tp
    __hurwitz_zeta_glob(_Tp __a, _Tp __s)
    {
      _Tp __zeta = _Tp(0);
      const _Tp __eps = std::numeric_limits<_Tp>::epsilon();
      const _Tp __max_bincoeff = std::numeric_limits<_Tp>::max_exponent10
                               * std::log(_Tp(10)) - _Tp(1);
      const unsigned int __maxit = 10000;
      for (unsigned int __i = 0; __i < __maxit; ++__i)
        {
          bool __punt = false;
          _Tp __sgn = _Tp(1);
          _Tp __term = _Tp(0);
          for (unsigned int __j = 0; __j <= __i; ++__j)
            {
              _Tp __bincoeff = ::std::lgamma(_Tp(1 + __i))
                              - ::std::lgamma(_Tp(1 + __j))
                              - ::std::lgamma(_Tp(1 + __i - __j));
              if (__bincoeff > __max_bincoeff)
                {
                  __punt = true;
                  break;
                }
              __bincoeff = std::exp(__bincoeff);
              __term += __sgn * __bincoeff * std::pow(_Tp(__a + __j), -__s);
              __sgn *= _Tp(-1);
            }
          if (__punt)
            break;
          __term /= _Tp(__i + 1);
          if (std::abs(__term / __zeta) < __eps)
            break;
          __zeta += __term;
        }
      __zeta /= __s - _Tp(1);
      return __zeta;
    }
    template<typename _Tp>
    inline _Tp
    __hurwitz_zeta(_Tp __a, _Tp __s)
    { return __hurwitz_zeta_glob(__a, __s); }
  }

}
namespace std __attribute__ ((__visibility__ ("default")))
{

  inline float
  assoc_laguerref(unsigned int __n, unsigned int __m, float __x)
  { return __detail::__assoc_laguerre<float>(__n, __m, __x); }
  inline long double
  assoc_laguerrel(unsigned int __n, unsigned int __m, long double __x)
  { return __detail::__assoc_laguerre<long double>(__n, __m, __x); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    assoc_laguerre(unsigned int __n, unsigned int __m, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__assoc_laguerre<__type>(__n, __m, __x);
    }
  inline float
  assoc_legendref(unsigned int __l, unsigned int __m, float __x)
  { return __detail::__assoc_legendre_p<float>(__l, __m, __x); }
  inline long double
  assoc_legendrel(unsigned int __l, unsigned int __m, long double __x)
  { return __detail::__assoc_legendre_p<long double>(__l, __m, __x); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    assoc_legendre(unsigned int __l, unsigned int __m, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__assoc_legendre_p<__type>(__l, __m, __x);
    }
  inline float
  betaf(float __a, float __b)
  { return __detail::__beta<float>(__a, __b); }
  inline long double
  betal(long double __a, long double __b)
  { return __detail::__beta<long double>(__a, __b); }
  template<typename _Tpa, typename _Tpb>
    inline typename __gnu_cxx::__promote_2<_Tpa, _Tpb>::__type
    beta(_Tpa __a, _Tpb __b)
    {
      typedef typename __gnu_cxx::__promote_2<_Tpa, _Tpb>::__type __type;
      return __detail::__beta<__type>(__a, __b);
    }
  inline float
  comp_ellint_1f(float __k)
  { return __detail::__comp_ellint_1<float>(__k); }
  inline long double
  comp_ellint_1l(long double __k)
  { return __detail::__comp_ellint_1<long double>(__k); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    comp_ellint_1(_Tp __k)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__comp_ellint_1<__type>(__k);
    }
  inline float
  comp_ellint_2f(float __k)
  { return __detail::__comp_ellint_2<float>(__k); }
  inline long double
  comp_ellint_2l(long double __k)
  { return __detail::__comp_ellint_2<long double>(__k); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    comp_ellint_2(_Tp __k)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__comp_ellint_2<__type>(__k);
    }
  inline float
  comp_ellint_3f(float __k, float __nu)
  { return __detail::__comp_ellint_3<float>(__k, __nu); }
  inline long double
  comp_ellint_3l(long double __k, long double __nu)
  { return __detail::__comp_ellint_3<long double>(__k, __nu); }
  template<typename _Tp, typename _Tpn>
    inline typename __gnu_cxx::__promote_2<_Tp, _Tpn>::__type
    comp_ellint_3(_Tp __k, _Tpn __nu)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Tpn>::__type __type;
      return __detail::__comp_ellint_3<__type>(__k, __nu);
    }
  inline float
  cyl_bessel_if(float __nu, float __x)
  { return __detail::__cyl_bessel_i<float>(__nu, __x); }
  inline long double
  cyl_bessel_il(long double __nu, long double __x)
  { return __detail::__cyl_bessel_i<long double>(__nu, __x); }
  template<typename _Tpnu, typename _Tp>
    inline typename __gnu_cxx::__promote_2<_Tpnu, _Tp>::__type
    cyl_bessel_i(_Tpnu __nu, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote_2<_Tpnu, _Tp>::__type __type;
      return __detail::__cyl_bessel_i<__type>(__nu, __x);
    }
  inline float
  cyl_bessel_jf(float __nu, float __x)
  { return __detail::__cyl_bessel_j<float>(__nu, __x); }
  inline long double
  cyl_bessel_jl(long double __nu, long double __x)
  { return __detail::__cyl_bessel_j<long double>(__nu, __x); }
  template<typename _Tpnu, typename _Tp>
    inline typename __gnu_cxx::__promote_2<_Tpnu, _Tp>::__type
    cyl_bessel_j(_Tpnu __nu, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote_2<_Tpnu, _Tp>::__type __type;
      return __detail::__cyl_bessel_j<__type>(__nu, __x);
    }
  inline float
  cyl_bessel_kf(float __nu, float __x)
  { return __detail::__cyl_bessel_k<float>(__nu, __x); }
  inline long double
  cyl_bessel_kl(long double __nu, long double __x)
  { return __detail::__cyl_bessel_k<long double>(__nu, __x); }
  template<typename _Tpnu, typename _Tp>
    inline typename __gnu_cxx::__promote_2<_Tpnu, _Tp>::__type
    cyl_bessel_k(_Tpnu __nu, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote_2<_Tpnu, _Tp>::__type __type;
      return __detail::__cyl_bessel_k<__type>(__nu, __x);
    }
  inline float
  cyl_neumannf(float __nu, float __x)
  { return __detail::__cyl_neumann_n<float>(__nu, __x); }
  inline long double
  cyl_neumannl(long double __nu, long double __x)
  { return __detail::__cyl_neumann_n<long double>(__nu, __x); }
  template<typename _Tpnu, typename _Tp>
    inline typename __gnu_cxx::__promote_2<_Tpnu, _Tp>::__type
    cyl_neumann(_Tpnu __nu, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote_2<_Tpnu, _Tp>::__type __type;
      return __detail::__cyl_neumann_n<__type>(__nu, __x);
    }
  inline float
  ellint_1f(float __k, float __phi)
  { return __detail::__ellint_1<float>(__k, __phi); }
  inline long double
  ellint_1l(long double __k, long double __phi)
  { return __detail::__ellint_1<long double>(__k, __phi); }
  template<typename _Tp, typename _Tpp>
    inline typename __gnu_cxx::__promote_2<_Tp, _Tpp>::__type
    ellint_1(_Tp __k, _Tpp __phi)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Tpp>::__type __type;
      return __detail::__ellint_1<__type>(__k, __phi);
    }
  inline float
  ellint_2f(float __k, float __phi)
  { return __detail::__ellint_2<float>(__k, __phi); }
  inline long double
  ellint_2l(long double __k, long double __phi)
  { return __detail::__ellint_2<long double>(__k, __phi); }
  template<typename _Tp, typename _Tpp>
    inline typename __gnu_cxx::__promote_2<_Tp, _Tpp>::__type
    ellint_2(_Tp __k, _Tpp __phi)
    {
      typedef typename __gnu_cxx::__promote_2<_Tp, _Tpp>::__type __type;
      return __detail::__ellint_2<__type>(__k, __phi);
    }
  inline float
  ellint_3f(float __k, float __nu, float __phi)
  { return __detail::__ellint_3<float>(__k, __nu, __phi); }
  inline long double
  ellint_3l(long double __k, long double __nu, long double __phi)
  { return __detail::__ellint_3<long double>(__k, __nu, __phi); }
  template<typename _Tp, typename _Tpn, typename _Tpp>
    inline typename __gnu_cxx::__promote_3<_Tp, _Tpn, _Tpp>::__type
    ellint_3(_Tp __k, _Tpn __nu, _Tpp __phi)
    {
      typedef typename __gnu_cxx::__promote_3<_Tp, _Tpn, _Tpp>::__type __type;
      return __detail::__ellint_3<__type>(__k, __nu, __phi);
    }
  inline float
  expintf(float __x)
  { return __detail::__expint<float>(__x); }
  inline long double
  expintl(long double __x)
  { return __detail::__expint<long double>(__x); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    expint(_Tp __x)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__expint<__type>(__x);
    }
  inline float
  hermitef(unsigned int __n, float __x)
  { return __detail::__poly_hermite<float>(__n, __x); }
  inline long double
  hermitel(unsigned int __n, long double __x)
  { return __detail::__poly_hermite<long double>(__n, __x); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    hermite(unsigned int __n, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__poly_hermite<__type>(__n, __x);
    }
  inline float
  laguerref(unsigned int __n, float __x)
  { return __detail::__laguerre<float>(__n, __x); }
  inline long double
  laguerrel(unsigned int __n, long double __x)
  { return __detail::__laguerre<long double>(__n, __x); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    laguerre(unsigned int __n, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__laguerre<__type>(__n, __x);
    }
  inline float
  legendref(unsigned int __l, float __x)
  { return __detail::__poly_legendre_p<float>(__l, __x); }
  inline long double
  legendrel(unsigned int __l, long double __x)
  { return __detail::__poly_legendre_p<long double>(__l, __x); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    legendre(unsigned int __l, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__poly_legendre_p<__type>(__l, __x);
    }
  inline float
  riemann_zetaf(float __s)
  { return __detail::__riemann_zeta<float>(__s); }
  inline long double
  riemann_zetal(long double __s)
  { return __detail::__riemann_zeta<long double>(__s); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    riemann_zeta(_Tp __s)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__riemann_zeta<__type>(__s);
    }
  inline float
  sph_besself(unsigned int __n, float __x)
  { return __detail::__sph_bessel<float>(__n, __x); }
  inline long double
  sph_bessell(unsigned int __n, long double __x)
  { return __detail::__sph_bessel<long double>(__n, __x); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    sph_bessel(unsigned int __n, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__sph_bessel<__type>(__n, __x);
    }
  inline float
  sph_legendref(unsigned int __l, unsigned int __m, float __theta)
  { return __detail::__sph_legendre<float>(__l, __m, __theta); }
  inline long double
  sph_legendrel(unsigned int __l, unsigned int __m, long double __theta)
  { return __detail::__sph_legendre<long double>(__l, __m, __theta); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    sph_legendre(unsigned int __l, unsigned int __m, _Tp __theta)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__sph_legendre<__type>(__l, __m, __theta);
    }
  inline float
  sph_neumannf(unsigned int __n, float __x)
  { return __detail::__sph_neumann<float>(__n, __x); }
  inline long double
  sph_neumannl(unsigned int __n, long double __x)
  { return __detail::__sph_neumann<long double>(__n, __x); }
  template<typename _Tp>
    inline typename __gnu_cxx::__promote<_Tp>::__type
    sph_neumann(unsigned int __n, _Tp __x)
    {
      typedef typename __gnu_cxx::__promote<_Tp>::__type __type;
      return __detail::__sph_neumann<__type>(__n, __x);
    }

}
}
using std::abs;
using std::acos;
using std::asin;
using std::atan;
using std::atan2;
using std::cos;
using std::sin;
using std::tan;
using std::cosh;
using std::sinh;
using std::tanh;
using std::exp;
using std::frexp;
using std::ldexp;
using std::log;
using std::log10;
using std::modf;
using std::pow;
using std::sqrt;
using std::ceil;
using std::fabs;
using std::floor;
using std::fmod;
using std::fpclassify;
using std::isfinite;
using std::isinf;
using std::isnan;
using std::isnormal;
using std::signbit;
using std::isgreater;
using std::isgreaterequal;
using std::isless;
using std::islessequal;
using std::islessgreater;
using std::isunordered;
using std::acosh;
using std::asinh;
using std::atanh;
using std::cbrt;
using std::copysign;
using std::erf;
using std::erfc;
using std::exp2;
using std::expm1;
using std::fdim;
using std::fma;
using std::fmax;
using std::fmin;
using std::hypot;
using std::ilogb;
using std::lgamma;
using std::llrint;
using std::llround;
using std::log1p;
using std::log2;
using std::logb;
using std::lrint;
using std::lround;
using std::nearbyint;
using std::nextafter;
using std::nexttoward;
using std::remainder;
using std::remquo;
using std::rint;
using std::round;
using std::scalbln;
using std::scalbn;
using std::tgamma;
using std::trunc;
typedef struct b3Vec2
{
 float x;
 float y;
} b3Vec2;
typedef struct b3Vec3
{
 float x;
 float y;
 float z;
} b3Vec3;
typedef struct b3CosSin
{
 float cosine;
 float sine;
} b3CosSin;
typedef struct b3Quat
{
 b3Vec3 v;
 float s;
} b3Quat;
typedef struct b3Transform
{
 b3Vec3 p;
 b3Quat q;
} b3Transform;
typedef b3Vec3 b3Pos;
typedef b3Transform b3WorldTransform;
typedef struct b3Matrix3
{
 b3Vec3 cx, cy, cz;
} b3Matrix3;
typedef struct b3AABB
{
 b3Vec3 lowerBound;
 b3Vec3 upperBound;
} b3AABB;
typedef struct b3Plane
{
 b3Vec3 normal;
 float offset;
} b3Plane;
static const b3Vec3 b3Vec3_zero = { 0.0f, 0.0f, 0.0f };
static const b3Vec3 b3Vec3_one = { 1.0f, 1.0f, 1.0f };
static const b3Vec3 b3Vec3_axisX = { 1.0f, 0.0f, 0.0f };
static const b3Vec3 b3Vec3_axisY = { 0.0f, 1.0f, 0.0f };
static const b3Vec3 b3Vec3_axisZ = { 0.0f, 0.0f, 1.0f };
static const b3Quat b3Quat_identity = { { 0.0f, 0.0f, 0.0f }, 1.0f };
static const b3Transform b3Transform_identity = { { 0.0f, 0.0f, 0.0f }, { { 0.0f, 0.0f, 0.0f }, 1.0f } };
static const b3Matrix3 b3Mat3_zero = {
 { 0.0f, 0.0f, 0.0f },
 { 0.0f, 0.0f, 0.0f },
 { 0.0f, 0.0f, 0.0f },
};
static const b3Matrix3 b3Mat3_identity = {
 { 1.0f, 0.0f, 0.0f },
 { 0.0f, 1.0f, 0.0f },
 { 0.0f, 0.0f, 1.0f },
};
static const b3Pos b3Pos_zero = { 0.0f, 0.0f, 0.0f };
static const b3WorldTransform b3WorldTransform_identity = { { 0.0f, 0.0f, 0.0f }, { { 0.0f, 0.0f, 0.0f }, 1.0f } };
inline int b3MinInt( int a, int b )
{
 return a < b ? a : b;
}
inline int b3MaxInt( int a, int b )
{
 return a > b ? a : b;
}
inline int b3ClampInt( int a, int lower, int upper )
{
 return a < lower ? lower : ( upper < a ? upper : a );
}
inline float b3AbsFloat( float a )
{
 return a < 0 ? -a : a;
}
inline float b3MinFloat( float a, float b )
{
 return a < b ? a : b;
}
inline float b3MaxFloat( float a, float b )
{
 return a > b ? a : b;
}
inline float b3ClampFloat( float a, float lower, float upper )
{
 return a < lower ? lower : ( upper < a ? upper : a );
}
inline float b3LerpFloat( float a, float b, float alpha )
{
 return ( 1.0f - alpha ) * a + alpha * b;
}
extern "C" float b3Atan2( float y, float x );
extern "C" b3CosSin b3ComputeCosSin( float radians );
inline float b3Sin( float radians )
{
 b3CosSin cs = b3ComputeCosSin( radians );
 return cs.sine;
}
inline float b3Cos( float radians )
{
 b3CosSin cs = b3ComputeCosSin( radians );
 return cs.cosine;
}
inline float b3UnwindAngle( float radians )
{
 return remainderf( radians, 2.0f * 3.14159265359f );
}
inline b3Vec3 b3Add( b3Vec3 a, b3Vec3 b )
{
 return b3Vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
}
inline b3Vec3 b3Sub( b3Vec3 a, b3Vec3 b )
{
 return b3Vec3{ a.x - b.x, a.y - b.y, a.z - b.z };
}
inline b3Vec3 b3Mul( b3Vec3 a, b3Vec3 b )
{
 return b3Vec3{ a.x * b.x, a.y * b.y, a.z * b.z };
}
inline b3Vec3 b3Neg( b3Vec3 a )
{
 return b3Vec3{ -a.x, -a.y, -a.z };
}
inline float b3Dot( b3Vec3 a, b3Vec3 b )
{
 return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float b3Length( b3Vec3 v )
{
 return sqrtf( b3Dot( v, v ) );
}
inline float b3LengthSquared( b3Vec3 a )
{
 return a.x * a.x + a.y * a.y + a.z * a.z;
}
inline float b3Distance( b3Vec3 a, b3Vec3 b )
{
 b3Vec3 dv = { b.x - a.x, b.y - a.y, b.z - a.z };
 return b3Length( dv );
}
inline float b3DistanceSquared( b3Vec3 a, b3Vec3 b )
{
 b3Vec3 dv = { b.x - a.x, b.y - a.y, b.z - a.z };
 return dv.x * dv.x + dv.y * dv.y + dv.z * dv.z;
}
inline __attribute__((always_inline)) b3Vec3 b3Normalize( b3Vec3 a )
{
 float lengthSquared = a.x * a.x + a.y * a.y + a.z * a.z;
 if ( lengthSquared > 1000.0f * 1.17549435082228750796873653722224568e-38F )
 {
  float s = 1.0f / sqrtf( lengthSquared );
  b3Vec3 u = { s * a.x, s * a.y, s * a.z };
  return u;
 }
 return b3Vec3{ 0.0f, 0.0f, 0.0f };
}
inline b3Vec3 b3GetLengthAndNormalize( float* length, b3Vec3 a )
{
 *length = b3Length( a );
 if ( *length < 1.19209289550781250000000000000000000e-7F )
 {
  return b3Vec3_zero;
 }
 float invLength = 1.0f / *length;
 b3Vec3 n = { invLength * a.x, invLength * a.y, invLength * a.z };
 return n;
}
inline b3Vec3 b3Perp( b3Vec3 a )
{
 b3Vec3 p;
 if ( a.x < -0.5f || 0.5f < a.x )
 {
  p = b3Vec3{ a.y, -a.x, 0.0f };
 }
 else
 {
  p = b3Vec3{ 0.0f, a.z, -a.y };
 }
 return b3Normalize( p );
}
inline bool b3IsNormalized( b3Vec3 a )
{
 float aa = b3Dot( a, a );
 return b3AbsFloat( 1.0f - aa ) < 100.0f * 1.19209289550781250000000000000000000e-7F;
}
inline b3Vec3 b3MulAdd( b3Vec3 a, float s, b3Vec3 b )
{
 return b3Vec3{ a.x + s * b.x, a.y + s * b.y, a.z + s * b.z };
}
inline b3Vec3 b3MulSub( b3Vec3 a, float s, b3Vec3 b )
{
 return b3Vec3{ a.x - s * b.x, a.y - s * b.y, a.z - s * b.z };
}
inline b3Vec3 b3MulSV( float s, b3Vec3 a )
{
 return b3Vec3{ s * a.x, s * a.y, s * a.z };
}
inline b3Vec3 b3Cross( b3Vec3 a, b3Vec3 b )
{
 b3Vec3 c;
 c.x = a.y * b.z - a.z * b.y;
 c.y = a.z * b.x - a.x * b.z;
 c.z = a.x * b.y - a.y * b.x;
 return c;
}
inline b3Vec3 b3Lerp( b3Vec3 a, b3Vec3 b, float alpha )
{
 ( (void)( ( !!( 0.0f <= alpha && alpha <= 1.0f ) ) || ( b3InternalAssert( "0.0f <= alpha && alpha <= 1.0f", "/home/firtoz/work/2026/box3d-wasm/box3d/include/box3d/math_functions.h", (int)( 362 ) ), 0 ) ) );
 b3Vec3 c = {
  ( 1.0f - alpha ) * a.x + alpha * b.x,
  ( 1.0f - alpha ) * a.y + alpha * b.y,
  ( 1.0f - alpha ) * a.z + alpha * b.z,
 };
 return c;
}
inline b3Vec3 b3Blend2( float s, b3Vec3 a, float t, b3Vec3 b )
{
 b3Vec3 d = {
  s * a.x + t * b.x,
  s * a.y + t * b.y,
  s * a.z + t * b.z,
 };
 return d;
}
inline b3Vec3 b3Abs( b3Vec3 a )
{
 return b3Vec3{
  b3AbsFloat( a.x ),
  b3AbsFloat( a.y ),
  b3AbsFloat( a.z ),
 };
}
inline b3Vec3 b3Sign( b3Vec3 a )
{
 return b3Vec3{
  a.x >= 0.0f ? 1.0f : -1.0f,
  a.y >= 0.0f ? 1.0f : -1.0f,
  a.z >= 0.0f ? 1.0f : -1.0f,
 };
}
inline b3Vec3 b3Min( b3Vec3 a, b3Vec3 b )
{
 return b3Vec3{
  b3MinFloat( a.x, b.x ),
  b3MinFloat( a.y, b.y ),
  b3MinFloat( a.z, b.z ),
 };
}
inline b3Vec3 b3Max( b3Vec3 a, b3Vec3 b )
{
 return b3Vec3{
  b3MaxFloat( a.x, b.x ),
  b3MaxFloat( a.y, b.y ),
  b3MaxFloat( a.z, b.z ),
 };
}
inline b3Vec3 b3Clamp( b3Vec3 a, b3Vec3 lower, b3Vec3 upper )
{
 b3Vec3 b;
 b.x = b3ClampFloat( a.x, lower.x, upper.x );
 b.y = b3ClampFloat( a.y, lower.y, upper.y );
 b.z = b3ClampFloat( a.z, lower.z, upper.z );
 return b;
}
inline b3Vec3 b3SafeScale( b3Vec3 a )
{
 b3Vec3 absScale = b3Abs( a );
 b3Vec3 minScale = { 0.01f, 0.01f, 0.01f };
 b3Vec3 safeScale = b3Mul( b3Sign( a ), b3Max( absScale, minScale ) );
 return safeScale;
}
inline bool b3IsNormalizedQuat( b3Quat q )
{
 float qq = q.v.x * q.v.x + q.v.y * q.v.y + q.v.z * q.v.z + q.s * q.s;
 return 1.0f - 20.0f * 1.19209289550781250000000000000000000e-7F < qq && qq < 1.0f + 20.0f * 1.19209289550781250000000000000000000e-7F;
}
inline b3Vec3 b3RotateVector( b3Quat q, b3Vec3 v )
{
 b3Vec3 t1 = b3Cross( q.v, v );
 b3Vec3 t2 = b3MulAdd( t1, q.s, v );
 b3Vec3 t3 = b3Cross( q.v, t2 );
 return b3MulAdd( v, 2.0f, t3 );
}
inline b3Vec3 b3InvRotateVector( b3Quat q, b3Vec3 v )
{
 b3Vec3 t1 = b3Cross( q.v, v );
 b3Vec3 t2 = b3MulSub( t1, q.s, v );
 b3Vec3 t3 = b3Cross( q.v, t2 );
 return b3MulAdd( v, 2.0f, t3 );
}
inline float b3DotQuat( b3Quat a, b3Quat b )
{
 return a.v.x * b.v.x + a.v.y * b.v.y + a.v.z * b.v.z + a.s * b.s;
}
inline b3Quat b3MulQuat( b3Quat q1, b3Quat q2 )
{
 b3Vec3 t1 = b3Cross( q1.v, q2.v );
 b3Vec3 t2 = b3MulAdd( t1, q1.s, q2.v );
 b3Vec3 t3 = b3MulAdd( t2, q2.s, q1.v );
 b3Quat q = { t3, q1.s * q2.s - b3Dot( q1.v, q2.v ) };
 return q;
}
inline b3Quat b3InvMulQuat( b3Quat q1, b3Quat q2 )
{
 b3Vec3 t1 = b3Cross( q2.v, q1.v );
 b3Vec3 t2 = b3MulAdd( t1, q1.s, q2.v );
 b3Vec3 t3 = b3MulSub( t2, q2.s, q1.v );
 b3Quat q = { t3, q1.s * q2.s + b3Dot( q1.v, q2.v ) };
 return q;
}
inline b3Quat b3Conjugate( b3Quat q )
{
 return b3Quat{ { -q.v.x, -q.v.y, -q.v.z }, q.s };
}
inline b3Quat b3NegateQuat( b3Quat q )
{
 return b3Quat{ { -q.v.x, -q.v.y, -q.v.z }, -q.s };
}
inline b3Quat b3NormalizeQuat( b3Quat q )
{
 float lengthSq = b3DotQuat( q, q );
 if ( lengthSq > 1000.0f * 1.17549435082228750796873653722224568e-38F )
 {
  float s = 1.0f / sqrtf( lengthSq );
  b3Quat qn = { { s * q.v.x, s * q.v.y, s * q.v.z }, s * q.s };
  return qn;
 }
 return b3Quat_identity;
}
inline b3Quat b3MakeQuatFromAxisAngle( b3Vec3 axis, float radians )
{
 ( (void)( ( !!( b3IsNormalized( axis ) ) ) || ( b3InternalAssert( "b3IsNormalized( axis )", "/home/firtoz/work/2026/box3d-wasm/box3d/include/box3d/math_functions.h", (int)( 528 ) ), 0 ) ) );
 b3CosSin cs = b3ComputeCosSin( 0.5f * radians );
 b3Quat q = { { cs.sine * axis.x, cs.sine * axis.y, cs.sine * axis.z }, cs.cosine };
 return q;
}
inline b3Vec3 b3GetAxisAngle( float* radians, b3Quat q )
{
 float length = sqrtf( q.v.x * q.v.x + q.v.y * q.v.y + q.v.z * q.v.z );
 *radians = 2.0f * b3Atan2( length, q.s );
 if ( length > 0.0f )
 {
  float invLength = 1.0f / length;
  b3Vec3 axis = { invLength * q.v.x, invLength * q.v.y, invLength * q.v.z };
  return axis;
 }
 return b3Vec3_zero;
}
inline float b3GetQuatAngle( b3Quat q )
{
 float length = sqrtf( q.v.x * q.v.x + q.v.y * q.v.y + q.v.z * q.v.z );
 return 2.0f * b3Atan2( length, q.s );
}
extern "C" b3Quat b3MakeQuatFromMatrix( const b3Matrix3* m );
extern "C" b3Quat b3ComputeQuatBetweenUnitVectors( b3Vec3 v1, b3Vec3 v2 );
inline float b3GetTwistAngle( b3Quat q )
{
 float twist = q.s < 0.0f ? b3Atan2( -q.v.z, -q.s ) : b3Atan2( q.v.z, q.s );
 twist *= 2.0f;
 ( (void)( ( !!( -3.14159265359f <= twist && twist <= 3.14159265359f ) ) || ( b3InternalAssert( "-B3_PI <= twist && twist <= B3_PI", "/home/firtoz/work/2026/box3d-wasm/box3d/include/box3d/math_functions.h", (int)( 569 ) ), 0 ) ) );
 return twist;
}
inline __attribute__((always_inline)) float b3GetSwingAngle( b3Quat q )
{
 float x = sqrtf( q.v.z * q.v.z + q.s * q.s );
 float y = sqrtf( q.v.x * q.v.x + q.v.y * q.v.y );
 float swing = 2.0f * b3Atan2( y, x );
 ( (void)( ( !!( 0.0f <= swing && swing <= 3.14159265359f ) ) || ( b3InternalAssert( "0.0f <= swing && swing <= B3_PI", "/home/firtoz/work/2026/box3d-wasm/box3d/include/box3d/math_functions.h", (int)( 580 ) ), 0 ) ) );
 return swing;
}
inline b3Quat b3NLerp( b3Quat q1, b3Quat q2, float alpha )
{
 ( (void)0 );
 if ( b3DotQuat( q1, q2 ) < 0.0f )
 {
  q1 = b3Quat{ { -q1.v.x, -q1.v.y, -q1.v.z }, -q1.s };
 }
 b3Quat q;
 q.v = b3Lerp( q1.v, q2.v, alpha );
 q.s = ( 1.0f - alpha ) * q1.s + alpha * q2.s;
 return b3NormalizeQuat( q );
}
inline b3Transform b3MulTransforms( b3Transform a, b3Transform b )
{
 b3Transform out;
 out.p = b3Add( b3RotateVector( a.q, b.p ), a.p );
 out.q = b3MulQuat( a.q, b.q );
 return out;
}
inline __attribute__((always_inline)) b3Transform b3InvMulTransforms( b3Transform a, b3Transform b )
{
 b3Transform out;
 out.p = b3InvRotateVector( a.q, b3Sub( b.p, a.p ) );
 out.q = b3InvMulQuat( a.q, b.q );
 return out;
}
inline b3Transform b3InvertTransform( b3Transform t )
{
 b3Transform out;
 out.p = b3InvRotateVector( t.q, b3Neg( t.p ) );
 out.q = b3Conjugate( t.q );
 return out;
}
inline __attribute__((always_inline)) b3Vec3 b3TransformPoint( b3Transform t, b3Vec3 v )
{
 b3Vec3 rv = b3RotateVector( t.q, v );
 return b3Add( rv, t.p );
}
inline b3Vec3 b3InvTransformPoint( b3Transform t, b3Vec3 v )
{
 return b3InvRotateVector( t.q, b3Sub( v, t.p ) );
}
inline b3Pos b3ToPos( b3Vec3 v )
{
 return b3Pos{ v.x, v.y, v.z };
}
inline b3Vec3 b3ToVec3( b3Pos p )
{
 return b3Vec3{ (float)p.x, (float)p.y, (float)p.z };
}
inline float b3RoundDownFloat( double x )
{
 return (float)x;
}
inline float b3RoundUpFloat( double x )
{
 return (float)x;
}
inline b3Vec3 b3SubPos( b3Pos a, b3Pos b )
{
 return b3Vec3{ (float)( a.x - b.x ), (float)( a.y - b.y ), (float)( a.z - b.z ) };
}
inline b3Pos b3OffsetPos( b3Pos p, b3Vec3 d )
{
 return b3Pos{ p.x + d.x, p.y + d.y, p.z + d.z };
}
inline b3Pos b3LerpPosition( b3Pos a, b3Pos b, float t )
{
 return b3Pos{
  ( 1.0f - t ) * a.x + t * b.x,
  ( 1.0f - t ) * a.y + t * b.y,
  ( 1.0f - t ) * a.z + t * b.z,
 };
}
inline b3Pos b3TransformWorldPoint( b3WorldTransform t, b3Vec3 p )
{
 b3Vec3 r = b3RotateVector( t.q, p );
 return b3Pos{ t.p.x + r.x, t.p.y + r.y, t.p.z + r.z };
}
inline b3Vec3 b3InvTransformWorldPoint( b3WorldTransform t, b3Pos p )
{
 b3Vec3 d = { (float)( p.x - t.p.x ), (float)( p.y - t.p.y ), (float)( p.z - t.p.z ) };
 return b3InvRotateVector( t.q, d );
}
inline __attribute__((always_inline)) b3WorldTransform b3MulWorldTransforms( b3WorldTransform A, b3Transform B )
{
 b3WorldTransform C;
 C.q = b3MulQuat( A.q, B.q );
 b3Vec3 r = b3RotateVector( A.q, B.p );
 C.p = b3Pos{ A.p.x + r.x, A.p.y + r.y, A.p.z + r.z };
 return C;
}
inline __attribute__((always_inline)) b3Transform b3InvMulWorldTransforms( b3WorldTransform A, b3WorldTransform B )
{
 b3Transform C;
 C.q = b3InvMulQuat( A.q, B.q );
 b3Vec3 d = { (float)( B.p.x - A.p.x ), (float)( B.p.y - A.p.y ), (float)( B.p.z - A.p.z ) };
 C.p = b3InvRotateVector( A.q, d );
 return C;
}
inline b3Transform b3ToRelativeTransform( b3WorldTransform t, b3Pos base )
{
 b3Transform r;
 r.q = t.q;
 r.p = b3Vec3{ (float)( t.p.x - base.x ), (float)( t.p.y - base.y ), (float)( t.p.z - base.z ) };
 return r;
}
inline b3WorldTransform b3MakeWorldTransform( b3Transform t )
{
 b3WorldTransform w;
 w.p = b3ToPos( t.p );
 w.q = t.q;
 return w;
}
inline b3AABB b3OffsetAABB( b3AABB localBox, b3Pos origin )
{
 b3AABB out;
 out.lowerBound.x = b3RoundDownFloat( origin.x + localBox.lowerBound.x );
 out.lowerBound.y = b3RoundDownFloat( origin.y + localBox.lowerBound.y );
 out.lowerBound.z = b3RoundDownFloat( origin.z + localBox.lowerBound.z );
 out.upperBound.x = b3RoundUpFloat( origin.x + localBox.upperBound.x );
 out.upperBound.y = b3RoundUpFloat( origin.y + localBox.upperBound.y );
 out.upperBound.z = b3RoundUpFloat( origin.z + localBox.upperBound.z );
 return out;
}
inline float b3Det( b3Matrix3 m )
{
 return b3Dot( m.cx, b3Cross( m.cy, m.cz ) );
}
inline b3Vec3 b3MulMV( b3Matrix3 m, b3Vec3 a )
{
 b3Vec3 b = {
  m.cx.x * a.x + m.cy.x * a.y + m.cz.x * a.z,
  m.cx.y * a.x + m.cy.y * a.y + m.cz.y * a.z,
  m.cx.z * a.x + m.cy.z * a.y + m.cz.z * a.z,
 };
 return b;
}
inline b3Matrix3 b3NegateMat3( b3Matrix3 a )
{
 return b3Matrix3{
  { -a.cx.x, -a.cx.y, -a.cx.z },
  { -a.cy.x, -a.cy.y, -a.cy.z },
  { -a.cz.x, -a.cz.y, -a.cz.z },
 };
}
inline b3Matrix3 b3AddMM( b3Matrix3 a, b3Matrix3 b )
{
 return b3Matrix3{
  { a.cx.x + b.cx.x, a.cx.y + b.cx.y, a.cx.z + b.cx.z },
  { a.cy.x + b.cy.x, a.cy.y + b.cy.y, a.cy.z + b.cy.z },
  { a.cz.x + b.cz.x, a.cz.y + b.cz.y, a.cz.z + b.cz.z },
 };
}
inline b3Matrix3 b3SubMM( b3Matrix3 a, b3Matrix3 b )
{
 return b3Matrix3{
  { a.cx.x - b.cx.x, a.cx.y - b.cx.y, a.cx.z - b.cx.z },
  { a.cy.x - b.cy.x, a.cy.y - b.cy.y, a.cy.z - b.cy.z },
  { a.cz.x - b.cz.x, a.cz.y - b.cz.y, a.cz.z - b.cz.z },
 };
}
inline b3Matrix3 b3MulSM( float s, b3Matrix3 a )
{
 return b3Matrix3{
  { s * a.cx.x, s * a.cx.y, s * a.cx.z },
  { s * a.cy.x, s * a.cy.y, s * a.cy.z },
  { s * a.cz.x, s * a.cz.y, s * a.cz.z },
 };
}
inline b3Matrix3 b3MulMM( b3Matrix3 a, b3Matrix3 b )
{
 b3Matrix3 out;
 out.cx = b3MulMV( a, b.cx );
 out.cy = b3MulMV( a, b.cy );
 out.cz = b3MulMV( a, b.cz );
 return out;
}
inline b3Matrix3 b3Transpose( b3Matrix3 m )
{
 b3Matrix3 out;
 out.cx = b3Vec3{ m.cx.x, m.cy.x, m.cz.x };
 out.cy = b3Vec3{ m.cx.y, m.cy.y, m.cz.y };
 out.cz = b3Vec3{ m.cx.z, m.cy.z, m.cz.z };
 return out;
}
inline b3Matrix3 b3InvertMatrix( b3Matrix3 m )
{
 float det = b3Det( m );
 if ( b3AbsFloat( det ) > 1000.0f * 1.17549435082228750796873653722224568e-38F )
 {
  float invDet = 1.0f / det;
  b3Matrix3 out;
  out.cx = b3MulSV( invDet, b3Cross( m.cy, m.cz ) );
  out.cy = b3MulSV( invDet, b3Cross( m.cz, m.cx ) );
  out.cz = b3MulSV( invDet, b3Cross( m.cx, m.cy ) );
  return b3Transpose( out );
 }
 return b3Mat3_zero;
}
inline __attribute__((always_inline)) b3Vec3 b3Solve3( b3Matrix3 m, b3Vec3 a )
{
 float det = b3Det( m );
 if ( b3AbsFloat( det ) > 1000.0f * 1.17549435082228750796873653722224568e-38F )
 {
  float invDet = 1.0f / det;
  b3Matrix3 s;
  s.cx = b3Cross( m.cy, m.cz );
  s.cy = b3Cross( m.cz, m.cx );
  s.cz = b3Cross( m.cx, m.cy );
  b3Vec3 b = {
   invDet * b3Dot( s.cx, a ),
   invDet * b3Dot( s.cy, a ),
   invDet * b3Dot( s.cz, a ),
  };
  return b;
 }
 return b3Vec3_zero;
}
inline b3Matrix3 b3InvertT( b3Matrix3 m )
{
 float det = b3Det( m );
 if ( b3AbsFloat( det ) > 1000.0f * 1.17549435082228750796873653722224568e-38F )
 {
  float invDet = 1.0f / det;
  b3Matrix3 out;
  out.cx = b3MulSV( invDet, b3Cross( m.cy, m.cz ) );
  out.cy = b3MulSV( invDet, b3Cross( m.cz, m.cx ) );
  out.cz = b3MulSV( invDet, b3Cross( m.cx, m.cy ) );
  return out;
 }
 return b3Mat3_zero;
}
inline b3Matrix3 b3AbsMatrix3( b3Matrix3 m )
{
 b3Matrix3 out;
 out.cx = b3Abs( m.cx );
 out.cy = b3Abs( m.cy );
 out.cz = b3Abs( m.cz );
 return out;
}
inline __attribute__((always_inline)) b3Matrix3 b3MakeMatrixFromQuat( b3Quat q )
{
 float xx = q.v.x * q.v.x;
 float yy = q.v.y * q.v.y;
 float zz = q.v.z * q.v.z;
 float xy = q.v.x * q.v.y;
 float xz = q.v.x * q.v.z;
 float xw = q.v.x * q.s;
 float yz = q.v.y * q.v.z;
 float yw = q.v.y * q.s;
 float zw = q.v.z * q.s;
 return b3Matrix3{
  { 1.0f - 2.0f * ( yy + zz ), 2.0f * ( xy + zw ), 2.0f * ( xz - yw ) },
  { 2.0f * ( xy - zw ), 1.0f - 2.0f * ( xx + zz ), 2.0f * ( yz + xw ) },
  { 2.0f * ( xz + yw ), 2.0f * ( yz - xw ), 1.0f - 2.0f * ( xx + yy ) },
 };
}
extern "C" b3Matrix3 b3Steiner( float mass, b3Vec3 origin );
inline b3AABB b3MakeAABB( const b3Vec3* points, int count, float radius )
{
 ( (void)( ( !!( count > 0 ) ) || ( b3InternalAssert( "count > 0", "/home/firtoz/work/2026/box3d-wasm/box3d/include/box3d/math_functions.h", (int)( 955 ) ), 0 ) ) );
 b3AABB a = { points[0], points[0] };
 for ( int i = 1; i < count; ++i )
 {
  a.lowerBound = b3Min( a.lowerBound, points[i] );
  a.upperBound = b3Max( a.upperBound, points[i] );
 }
 b3Vec3 r = { radius, radius, radius };
 a.lowerBound = b3Sub( a.lowerBound, r );
 a.upperBound = b3Add( a.upperBound, r );
 return a;
}
inline bool b3AABB_Contains( b3AABB a, b3AABB b )
{
 if ( a.lowerBound.x > b.lowerBound.x || b.upperBound.x > a.upperBound.x )
  return false;
 if ( a.lowerBound.y > b.lowerBound.y || b.upperBound.y > a.upperBound.y )
  return false;
 if ( a.lowerBound.z > b.lowerBound.z || b.upperBound.z > a.upperBound.z )
  return false;
 return true;
}
inline float b3AABB_Area( b3AABB a )
{
 b3Vec3 delta = b3Sub( a.upperBound, a.lowerBound );
 return 2.0f * ( delta.x * delta.y + delta.y * delta.z + delta.z * delta.x );
}
inline b3Vec3 b3AABB_Center( b3AABB a )
{
 return b3MulSV( 0.5f, b3Add( a.upperBound, a.lowerBound ) );
}
inline b3Vec3 b3AABB_Extents( b3AABB a )
{
 return b3MulSV( 0.5f, b3Sub( a.upperBound, a.lowerBound ) );
}
inline b3AABB b3AABB_Union( b3AABB a, b3AABB b )
{
 b3AABB out;
 out.lowerBound = b3Min( a.lowerBound, b.lowerBound );
 out.upperBound = b3Max( a.upperBound, b.upperBound );
 return out;
}
inline b3AABB b3AABB_Inflate( b3AABB a, float extension )
{
 b3Vec3 radius = { extension, extension, extension };
 b3AABB out;
 out.lowerBound = b3Sub( a.lowerBound, radius );
 out.upperBound = b3Add( a.upperBound, radius );
 return out;
}
inline bool b3AABB_Overlaps( b3AABB a, b3AABB b )
{
 if ( a.upperBound.x < b.lowerBound.x || a.lowerBound.x > b.upperBound.x )
  return false;
 if ( a.upperBound.y < b.lowerBound.y || a.lowerBound.y > b.upperBound.y )
  return false;
 if ( a.upperBound.z < b.lowerBound.z || a.lowerBound.z > b.upperBound.z )
  return false;
 return true;
}
inline __attribute__((always_inline)) b3AABB b3AABB_Transform( b3Transform transform, b3AABB a )
{
 b3Vec3 center = b3TransformPoint( transform, b3AABB_Center( a ) );
 b3Matrix3 m = b3MakeMatrixFromQuat( transform.q );
 b3Vec3 extent = b3MulMV( b3AbsMatrix3( m ), b3AABB_Extents( a ) );
 b3AABB out = { b3Sub( center, extent ), b3Add( center, extent ) };
 return out;
}
inline b3Vec3 b3ClosestPointToAABB( b3Vec3 point, b3AABB a )
{
 return b3Clamp( point, a.lowerBound, a.upperBound );
}
typedef struct b3SegmentDistanceResult
{
 b3Vec3 point1;
 float fraction1;
 b3Vec3 point2;
 float fraction2;
} b3SegmentDistanceResult;
extern "C" b3Vec3 b3PointToSegmentDistance( b3Vec3 a, b3Vec3 b, b3Vec3 q );
extern "C" b3SegmentDistanceResult b3LineDistance( b3Vec3 p1, b3Vec3 d1, b3Vec3 p2, b3Vec3 d2 );
extern "C" b3SegmentDistanceResult b3SegmentDistance( b3Vec3 p1, b3Vec3 q1, b3Vec3 p2, b3Vec3 q2 );
extern "C" bool b3IsValidFloat( float a );
extern "C" bool b3IsValidVec3( b3Vec3 a );
extern "C" bool b3IsValidQuat( b3Quat q );
extern "C" bool b3IsValidTransform( b3Transform a );
extern "C" bool b3IsValidMatrix3( b3Matrix3 a );
extern "C" bool b3IsValidAABB( b3AABB a );
extern "C" bool b3IsBoundedAABB( b3AABB a );
extern "C" bool b3IsSaneAABB( b3AABB a );
extern "C" bool b3IsValidPlane( b3Plane a );
extern "C" bool b3IsValidPosition( b3Pos p );
extern "C" bool b3IsValidWorldTransform( b3WorldTransform t );
inline __attribute__((always_inline)) b3Vec3& operator+=( b3Vec3& a, b3Vec3 b )
{
 a.x += b.x;
 a.y += b.y;
 a.z += b.z;
 return a;
}
inline __attribute__((always_inline)) b3Vec3& operator-=( b3Vec3& a, b3Vec3 b )
{
 a.x -= b.x;
 a.y -= b.y;
 a.z -= b.z;
 return a;
}
inline __attribute__((always_inline)) b3Vec3& operator*=( b3Vec3& a, float s )
{
 a.x *= s;
 a.y *= s;
 a.z *= s;
 return a;
}
inline __attribute__((always_inline)) b3Vec3 operator-( b3Vec3 a )
{
 return { -a.x, -a.y, -a.z };
}
inline __attribute__((always_inline)) b3Vec3 operator*( float s, b3Vec3 a )
{
 return { s * a.x, s * a.y, s * a.z };
}
inline __attribute__((always_inline)) b3Vec3 operator*( b3Vec3 a, float s )
{
 return { s * a.x, s * a.y, s * a.z };
}
inline __attribute__((always_inline)) b3Vec3 operator*( b3Vec3 a, b3Vec3 b )
{
 return { a.x * b.x, a.y * b.y, a.z * b.z };
}
inline __attribute__((always_inline)) b3Vec3 operator+( b3Vec3 a, b3Vec3 b )
{
 return { a.x + b.x, a.y + b.y, a.z + b.z };
}
inline __attribute__((always_inline)) b3Vec3 operator-( b3Vec3 a, b3Vec3 b )
{
 return { a.x - b.x, a.y - b.y, a.z - b.z };
}
       
       
extern "C" void b3SetLengthUnitsPerMeter( float lengthUnits );
extern "C" float b3GetLengthUnitsPerMeter( void );
extern "C" void b3SetStallThreshold( float seconds );
extern "C" float b3GetStallThreshold( void );
       
typedef struct b3WorldId
{
 uint16_t index1;
 uint16_t generation;
} b3WorldId;
typedef struct b3BodyId
{
 int32_t index1;
 uint16_t world0;
 uint16_t generation;
} b3BodyId;
typedef struct b3ShapeId
{
 int32_t index1;
 uint16_t world0;
 uint16_t generation;
} b3ShapeId;
typedef struct b3JointId
{
 int32_t index1;
 uint16_t world0;
 uint16_t generation;
} b3JointId;
typedef struct b3ContactId
{
 int32_t index1;
 uint16_t world0;
 int16_t padding;
 uint32_t generation;
} b3ContactId;
static const b3WorldId b3_nullWorldId = {};
static const b3BodyId b3_nullBodyId = {};
static const b3ShapeId b3_nullShapeId = {};
static const b3JointId b3_nullJointId = {};
static const b3ContactId b3_nullContactId = {};
inline uint32_t b3StoreWorldId( b3WorldId id )
{
 return ( (uint32_t)id.index1 << 16 ) | (uint32_t)id.generation;
}
inline b3WorldId b3LoadWorldId( uint32_t x )
{
 b3WorldId id = { (uint16_t)( x >> 16 ), (uint16_t)( x ) };
 return id;
}
inline uint64_t b3StoreBodyId( b3BodyId id )
{
 return ( (uint64_t)id.index1 << 32 ) | ( (uint64_t)id.world0 ) << 16 | (uint64_t)id.generation;
}
inline b3BodyId b3LoadBodyId( uint64_t x )
{
 b3BodyId id = { (int32_t)( x >> 32 ), (uint16_t)( x >> 16 ), (uint16_t)( x ) };
 return id;
}
inline uint64_t b3StoreShapeId( b3ShapeId id )
{
 return ( (uint64_t)id.index1 << 32 ) | ( (uint64_t)id.world0 ) << 16 | (uint64_t)id.generation;
}
inline b3ShapeId b3LoadShapeId( uint64_t x )
{
 b3ShapeId id = { (int32_t)( x >> 32 ), (uint16_t)( x >> 16 ), (uint16_t)( x ) };
 return id;
}
inline uint64_t b3StoreJointId( b3JointId id )
{
 return ( (uint64_t)id.index1 << 32 ) | ( (uint64_t)id.world0 ) << 16 | (uint64_t)id.generation;
}
inline b3JointId b3LoadJointId( uint64_t x )
{
 b3JointId id = { (int32_t)( x >> 32 ), (uint16_t)( x >> 16 ), (uint16_t)( x ) };
 return id;
}
inline void b3StoreContactId( b3ContactId id, uint32_t values[3] )
{
 values[0] = (uint32_t)id.index1;
 values[1] = (uint32_t)id.world0;
 values[2] = (uint32_t)id.generation;
}
inline b3ContactId b3LoadContactId( uint32_t values[3] )
{
 b3ContactId id;
 id.index1 = (int32_t)values[0];
 id.world0 = (uint16_t)values[1];
 id.padding = 0;
 id.generation = (uint32_t)values[2];
 return id;
}
typedef void b3TaskCallback( void* taskContext );
typedef void* b3EnqueueTaskCallback( b3TaskCallback* task, void* taskContext, void* userContext, const char* taskName );
typedef void b3FinishTaskCallback( void* userTask, void* userContext );
typedef struct b3DebugShape b3DebugShape;
typedef void* b3CreateDebugShapeCallback( const b3DebugShape* debugShape, void* userContext );
typedef void b3DestroyDebugShapeCallback( void* userShape, void* userContext );
typedef float b3FrictionCallback( float frictionA, uint64_t userMaterialIdA, float frictionB, uint64_t userMaterialIdB );
typedef float b3RestitutionCallback( float restitutionA, uint64_t userMaterialIdA, float restitutionB, uint64_t userMaterialIdB );
typedef bool b3CustomFilterFcn( b3ShapeId shapeIdA, b3ShapeId shapeIdB, void* context );
typedef bool b3PreSolveFcn( b3ShapeId shapeIdA, b3ShapeId shapeIdB, b3Pos point, b3Vec3 normal, void* context );
typedef bool b3OverlapResultFcn( b3ShapeId shapeId, void* context );
typedef float b3CastResultFcn( b3ShapeId shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t userMaterialId,
          int triangleIndex, int childIndex, void* context );
typedef struct b3Capacity
{
 int staticShapeCount;
 int dynamicShapeCount;
 int staticBodyCount;
 int dynamicBodyCount;
 int contactCount;
} b3Capacity;
typedef struct b3WorldDef
{
 b3Vec3 gravity;
 float restitutionThreshold;
 float hitEventThreshold;
 float contactHertz;
 float contactDampingRatio;
 float contactSpeed;
 float maximumLinearSpeed;
 b3FrictionCallback* frictionCallback;
 b3RestitutionCallback* restitutionCallback;
 bool enableSleep;
 bool enableContinuous;
 uint32_t workerCount;
 b3EnqueueTaskCallback* enqueueTask;
 b3FinishTaskCallback* finishTask;
 void* userTaskContext;
 void* userData;
 b3CreateDebugShapeCallback* createDebugShape;
 b3DestroyDebugShapeCallback* destroyDebugShape;
 void* userDebugShapeContext;
 b3Capacity capacity;
 int internalValue;
} b3WorldDef;
extern "C" b3WorldDef b3DefaultWorldDef( void );
typedef enum b3BodyType
{
 b3_staticBody = 0,
 b3_kinematicBody = 1,
 b3_dynamicBody = 2,
 b3_bodyTypeCount,
} b3BodyType;
typedef struct b3MotionLocks
{
 bool linearX;
 bool linearY;
 bool linearZ;
 bool angularX;
 bool angularY;
 bool angularZ;
} b3MotionLocks;
typedef struct b3BodyDef
{
 b3BodyType type;
 b3Pos position;
 b3Quat rotation;
 b3Vec3 linearVelocity;
 b3Vec3 angularVelocity;
 float linearDamping;
 float angularDamping;
 float gravityScale;
 float sleepThreshold;
 const char* name;
 void* userData;
 b3MotionLocks motionLocks;
 bool enableSleep;
 bool isAwake;
 bool isBullet;
 bool isEnabled;
 bool allowFastRotation;
 bool enableContactRecycling;
 int internalValue;
} b3BodyDef;
extern "C" b3BodyDef b3DefaultBodyDef( void );
typedef struct b3Filter
{
 uint64_t categoryBits;
 uint64_t maskBits;
 int groupIndex;
} b3Filter;
extern "C" b3Filter b3DefaultFilter( void );
typedef struct b3SurfaceMaterial
{
 float friction;
 float restitution;
 float rollingResistance;
 b3Vec3 tangentVelocity;
 uint64_t userMaterialId;
 uint32_t customColor;
 uint32_t padding;
} b3SurfaceMaterial;
extern "C" b3SurfaceMaterial b3DefaultSurfaceMaterial( void );
typedef enum b3ShapeType
{
 b3_capsuleShape,
 b3_compoundShape,
 b3_heightShape,
 b3_hullShape,
 b3_meshShape,
 b3_sphereShape,
 b3_shapeTypeCount
} b3ShapeType;
typedef struct b3ShapeDef
{
 const char* name;
 void* userData;
 b3SurfaceMaterial* materials;
 int materialCount;
 b3SurfaceMaterial baseMaterial;
 float density;
 float explosionScale;
 b3Filter filter;
 bool enableCustomFiltering;
 bool isSensor;
 bool enableSensorEvents;
 bool enableContactEvents;
 bool enableHitEvents;
 bool enablePreSolveEvents;
 bool invokeContactCreation;
 bool updateBodyMass;
 bool enableSpeculativeContact;
 int internalValue;
} b3ShapeDef;
extern "C" b3ShapeDef b3DefaultShapeDef( void );
typedef struct b3Profile
{
 float step;
 float pairs;
 float collide;
 float solve;
 float solverSetup;
 float constraints;
 float prepareConstraints;
 float integrateVelocities;
 float warmStart;
 float solveImpulses;
 float integratePositions;
 float relaxImpulses;
 float applyRestitution;
 float storeImpulses;
 float splitIslands;
 float transforms;
 float sensorHits;
 float jointEvents;
 float hitEvents;
 float refit;
 float bullets;
 float sleepIslands;
 float sensors;
} b3Profile;
typedef struct b3Counters
{
 int bodyCount;
 int shapeCount;
 int contactCount;
 int jointCount;
 int islandCount;
 int stackUsed;
 int arenaCapacity;
 int staticTreeHeight;
 int treeHeight;
 int satCallCount;
 int satCacheHitCount;
 int byteCount;
 int taskCount;
 int colorCounts[24];
 int manifoldCounts[8];
 int awakeContactCount;
 int recycledContactCount;
 int distanceIterations;
 int pushBackIterations;
 int rootIterations;
} b3Counters;
typedef enum b3JointType
{
 b3_parallelJoint,
 b3_distanceJoint,
 b3_filterJoint,
 b3_motorJoint,
 b3_prismaticJoint,
 b3_revoluteJoint,
 b3_sphericalJoint,
 b3_weldJoint,
 b3_wheelJoint,
} b3JointType;
typedef struct b3JointDef
{
 void* userData;
 b3BodyId bodyIdA;
 b3BodyId bodyIdB;
 b3Transform localFrameA;
 b3Transform localFrameB;
 float forceThreshold;
 float torqueThreshold;
 float constraintHertz;
 float constraintDampingRatio;
 float drawScale;
 bool collideConnected;
 int internalValue;
} b3JointDef;
typedef struct b3DistanceJointDef
{
 b3JointDef base;
 float length;
 bool enableSpring;
 float lowerSpringForce;
 float upperSpringForce;
 float hertz;
 float dampingRatio;
 bool enableLimit;
 float minLength;
 float maxLength;
 bool enableMotor;
 float maxMotorForce;
 float motorSpeed;
} b3DistanceJointDef;
extern "C" b3DistanceJointDef b3DefaultDistanceJointDef( void );
typedef struct b3MotorJointDef
{
 b3JointDef base;
 b3Vec3 linearVelocity;
 float maxVelocityForce;
 b3Vec3 angularVelocity;
 float maxVelocityTorque;
 float linearHertz;
 float linearDampingRatio;
 float maxSpringForce;
 float angularHertz;
 float angularDampingRatio;
 float maxSpringTorque;
} b3MotorJointDef;
extern "C" b3MotorJointDef b3DefaultMotorJointDef( void );
typedef struct b3FilterJointDef
{
 b3JointDef base;
} b3FilterJointDef;
extern "C" b3FilterJointDef b3DefaultFilterJointDef( void );
typedef struct b3ParallelJointDef
{
 b3JointDef base;
 float hertz;
 float dampingRatio;
 float maxTorque;
} b3ParallelJointDef;
extern "C" b3ParallelJointDef b3DefaultParallelJointDef( void );
typedef struct b3PrismaticJointDef
{
 b3JointDef base;
 bool enableSpring;
 float hertz;
 float dampingRatio;
 float targetTranslation;
 bool enableLimit;
 float lowerTranslation;
 float upperTranslation;
 bool enableMotor;
 float maxMotorForce;
 float motorSpeed;
} b3PrismaticJointDef;
extern "C" b3PrismaticJointDef b3DefaultPrismaticJointDef( void );
typedef struct b3RevoluteJointDef
{
 b3JointDef base;
 float targetAngle;
 bool enableSpring;
 float hertz;
 float dampingRatio;
 bool enableLimit;
 float lowerAngle;
 float upperAngle;
 bool enableMotor;
 float maxMotorTorque;
 float motorSpeed;
} b3RevoluteJointDef;
extern "C" b3RevoluteJointDef b3DefaultRevoluteJointDef( void );
typedef struct b3SphericalJointDef
{
 b3JointDef base;
 bool enableSpring;
 float hertz;
 float dampingRatio;
 b3Quat targetRotation;
 bool enableConeLimit;
 float coneAngle;
 bool enableTwistLimit;
 float lowerTwistAngle;
 float upperTwistAngle;
 bool enableMotor;
 float maxMotorTorque;
 b3Vec3 motorVelocity;
} b3SphericalJointDef;
extern "C" b3SphericalJointDef b3DefaultSphericalJointDef( void );
typedef struct b3WeldJointDef
{
 b3JointDef base;
 float linearHertz;
 float angularHertz;
 float linearDampingRatio;
 float angularDampingRatio;
} b3WeldJointDef;
extern "C" b3WeldJointDef b3DefaultWeldJointDef( void );
typedef struct b3WheelJointDef
{
 b3JointDef base;
 bool enableSuspensionSpring;
 float suspensionHertz;
 float suspensionDampingRatio;
 bool enableSuspensionLimit;
 float lowerSuspensionLimit;
 float upperSuspensionLimit;
 bool enableSpinMotor;
 float maxSpinTorque;
 float spinSpeed;
 bool enableSteering;
 float steeringHertz;
 float steeringDampingRatio;
 float targetSteeringAngle;
 float maxSteeringTorque;
 bool enableSteeringLimit;
 float lowerSteeringLimit;
 float upperSteeringLimit;
} b3WheelJointDef;
extern "C" b3WheelJointDef b3DefaultWheelJointDef( void );
typedef struct b3ExplosionDef
{
 uint64_t maskBits;
 b3Pos position;
 float radius;
 float falloff;
 float impulsePerArea;
} b3ExplosionDef;
extern "C" b3ExplosionDef b3DefaultExplosionDef( void );
typedef struct b3SensorBeginTouchEvent
{
 b3ShapeId sensorShapeId;
 b3ShapeId visitorShapeId;
} b3SensorBeginTouchEvent;
typedef struct b3SensorEndTouchEvent
{
 b3ShapeId sensorShapeId;
 b3ShapeId visitorShapeId;
} b3SensorEndTouchEvent;
typedef struct b3SensorEvents
{
 b3SensorBeginTouchEvent* beginEvents;
 b3SensorEndTouchEvent* endEvents;
 int beginCount;
 int endCount;
} b3SensorEvents;
typedef struct b3ContactBeginTouchEvent
{
 b3ShapeId shapeIdA;
 b3ShapeId shapeIdB;
 b3ContactId contactId;
} b3ContactBeginTouchEvent;
typedef struct b3ContactEndTouchEvent
{
 b3ShapeId shapeIdA;
 b3ShapeId shapeIdB;
 b3ContactId contactId;
} b3ContactEndTouchEvent;
typedef struct b3ContactHitEvent
{
 b3ShapeId shapeIdA;
 b3ShapeId shapeIdB;
 b3ContactId contactId;
 b3Pos point;
 b3Vec3 normal;
 float approachSpeed;
 uint64_t userMaterialIdA;
 uint64_t userMaterialIdB;
} b3ContactHitEvent;
typedef struct b3ContactEvents
{
 b3ContactBeginTouchEvent* beginEvents;
 b3ContactEndTouchEvent* endEvents;
 b3ContactHitEvent* hitEvents;
 int beginCount;
 int endCount;
 int hitCount;
} b3ContactEvents;
typedef struct b3BodyMoveEvent
{
 void* userData;
 b3WorldTransform transform;
 b3BodyId bodyId;
 bool fellAsleep;
} b3BodyMoveEvent;
typedef struct b3BodyEvents
{
 b3BodyMoveEvent* moveEvents;
 int moveCount;
} b3BodyEvents;
typedef struct b3JointEvent
{
 b3JointId jointId;
 void* userData;
} b3JointEvent;
typedef struct b3JointEvents
{
 b3JointEvent* jointEvents;
 int count;
} b3JointEvents;
typedef struct b3ContactData
{
 b3ContactId contactId;
 b3ShapeId shapeIdA;
 b3ShapeId shapeIdB;
 const struct b3Manifold* manifolds;
 int manifoldCount;
} b3ContactData;
typedef struct b3QueryFilter
{
 uint64_t categoryBits;
 uint64_t maskBits;
 uint64_t id;
 const char* name;
} b3QueryFilter;
extern "C" b3QueryFilter b3DefaultQueryFilter( void );
typedef struct b3RayCastInput
{
 b3Vec3 origin;
 b3Vec3 translation;
 float maxFraction;
} b3RayCastInput;
typedef struct b3RayResult
{
 b3ShapeId shapeId;
 b3Pos point;
 b3Vec3 normal;
 uint64_t userMaterialId;
 float fraction;
 int triangleIndex;
 int childIndex;
 int nodeVisits;
 int leafVisits;
 bool hit;
} b3RayResult;
typedef struct b3ShapeProxy
{
 const b3Vec3* points;
 int count;
 float radius;
} b3ShapeProxy;
typedef struct b3ShapeCastInput
{
 b3ShapeProxy proxy;
 b3Vec3 translation;
 float maxFraction;
 bool canEncroach;
} b3ShapeCastInput;
typedef struct b3BoxCastInput
{
 b3AABB box;
 b3Vec3 translation;
 float maxFraction;
} b3BoxCastInput;
typedef struct b3CastOutput
{
 b3Vec3 normal;
 b3Vec3 point;
 float fraction;
 int iterations;
 int triangleIndex;
 int childIndex;
 int materialIndex;
 bool hit;
} b3CastOutput;
typedef b3CastOutput b3WorldCastOutput;
typedef struct b3BodyCastResult
{
 b3ShapeId shapeId;
 b3Pos point;
 b3Vec3 normal;
 float fraction;
 int triangleIndex;
 uint64_t userMaterialId;
 int iterations;
 bool hit;
} b3BodyCastResult;
typedef struct b3SimplexCache
{
 float metric;
 uint16_t count;
 uint8_t indexA[4];
 uint8_t indexB[4];
} b3SimplexCache;
static const b3SimplexCache b3_emptyDistanceCache = {};
typedef struct b3ShapeCastPairInput
{
 b3ShapeProxy proxyA;
 b3ShapeProxy proxyB;
 b3Transform transform;
 b3Vec3 translationB;
 float maxFraction;
 bool canEncroach;
} b3ShapeCastPairInput;
typedef struct b3DistanceInput
{
 b3ShapeProxy proxyA;
 b3ShapeProxy proxyB;
 b3Transform transform;
 bool useRadii;
} b3DistanceInput;
typedef struct b3DistanceOutput
{
 b3Vec3 pointA;
 b3Vec3 pointB;
 b3Vec3 normal;
 float distance;
 int iterations;
 int simplexCount;
} b3DistanceOutput;
typedef struct b3SimplexVertex
{
 b3Vec3 wA;
 b3Vec3 wB;
 b3Vec3 w;
 float a;
 int indexA;
 int indexB;
} b3SimplexVertex;
typedef struct b3Simplex
{
 b3SimplexVertex vertices[4];
 int count;
} b3Simplex;
typedef struct b3Sweep
{
 b3Vec3 localCenter;
 b3Vec3 c1;
 b3Vec3 c2;
 b3Quat q1;
 b3Quat q2;
} b3Sweep;
typedef struct b3TOIInput
{
 b3ShapeProxy proxyA;
 b3ShapeProxy proxyB;
 b3Sweep sweepA;
 b3Sweep sweepB;
 float maxFraction;
} b3TOIInput;
typedef enum b3TOIState
{
 b3_toiStateUnknown,
 b3_toiStateFailed,
 b3_toiStateOverlapped,
 b3_toiStateHit,
 b3_toiStateSeparated
} b3TOIState;
typedef struct b3TOIOutput
{
 b3TOIState state;
 b3Vec3 point;
 b3Vec3 normal;
 float fraction;
 float distance;
 int distanceIterations;
 int pushBackIterations;
 int rootIterations;
 bool usedFallback;
} b3TOIOutput;
typedef enum b3TreeNodeFlags
{
 b3_allocatedNode = 0x0001,
 b3_enlargedNode = 0x0002,
 b3_leafNode = 0x0004,
} b3TreeNodeFlags;
typedef struct b3TreeNodeChildren
{
 int child1;
 int child2;
} b3TreeNodeChildren;
typedef struct b3TreeNode
{
 b3AABB aabb;
 uint64_t categoryBits;
 union
 {
  b3TreeNodeChildren children;
  uint64_t userData;
 };
 union
 {
  int parent;
  int next;
 };
 uint16_t height;
 uint16_t flags;
} b3TreeNode;
typedef struct b3DynamicTree
{
 uint64_t version;
 b3TreeNode* nodes;
 int root;
 int nodeCount;
 int nodeCapacity;
 int proxyCount;
 int freeList;
 int* leafIndices;
 b3AABB* leafBoxes;
 b3Vec3* leafCenters;
 int* binIndices;
 int rebuildCapacity;
} b3DynamicTree;
typedef struct b3TreeStats
{
 int nodeVisits;
 int leafVisits;
} b3TreeStats;
typedef bool b3TreeQueryCallbackFcn( int proxyId, uint64_t userData, void* context );
typedef float b3TreeQueryClosestCallbackFcn( float distanceSqrMin, int proxyId, uint64_t userData, void* context );
typedef float b3TreeBoxCastCallbackFcn( const b3BoxCastInput* input, int proxyId, uint64_t userData, void* context );
typedef float b3TreeRayCastCallbackFcn( const b3RayCastInput* input, int proxyId, uint64_t userData, void* context );
typedef struct b3PlaneResult
{
 b3Plane plane;
 b3Vec3 point;
} b3PlaneResult;
typedef struct b3CollisionPlane
{
 b3Plane plane;
 float pushLimit;
 float push;
 bool clipVelocity;
} b3CollisionPlane;
typedef struct b3PlaneSolverResult
{
 b3Vec3 delta;
 int iterationCount;
} b3PlaneSolverResult;
typedef struct b3BodyPlaneResult
{
 b3ShapeId shapeId;
 b3PlaneResult result;
} b3BodyPlaneResult;
typedef bool b3PlaneResultFcn( b3ShapeId shapeId, const b3PlaneResult* plane, int planeCount, void* context );
typedef bool b3MoverFilterFcn( b3ShapeId shapeId, void* context );
typedef struct b3MassData
{
 float mass;
 b3Vec3 center;
 b3Matrix3 inertia;
} b3MassData;
typedef struct b3Sphere
{
 b3Vec3 center;
 float radius;
} b3Sphere;
typedef struct b3Capsule
{
 b3Vec3 center1;
 b3Vec3 center2;
 float radius;
} b3Capsule;
typedef struct b3HullVertex
{
 uint8_t edge;
} b3HullVertex;
typedef struct b3HullHalfEdge
{
 uint8_t next;
 uint8_t twin;
 uint8_t origin;
 uint8_t face;
} b3HullHalfEdge;
typedef struct b3HullFace
{
 uint8_t edge;
} b3HullFace;
typedef struct b3HullData
{
 uint64_t version;
 uint64_t hash;
 b3AABB aabb;
 float surfaceArea;
 float volume;
 float innerRadius;
 b3Vec3 center;
 b3Matrix3 centralInertia;
 int32_t vertexCount;
 int32_t vertexOffset;
 int32_t pointOffset;
 int32_t edgeCount;
 int32_t edgeOffset;
 int32_t faceCount;
 int32_t planeOffset;
 int32_t faceOffset;
 int32_t soaVertexOffset;
 int32_t soaNormalOffset;
 int32_t byteCount;
} b3HullData;
typedef struct b3BoxHull
{
 b3HullData base;
 b3HullVertex boxVertices[8];
 b3Vec3 boxPoints[8];
 b3HullHalfEdge boxEdges[24];
 b3Plane boxPlanes[6];
 b3HullFace boxFaces[6];
 uint8_t padding[2];
 float vx[8];
 float vy[8];
 float vz[8];
 float nx[8];
 float ny[8];
 float nz[8];
} b3BoxHull;
typedef struct b3MeshDef
{
 b3Vec3* vertices;
 int32_t* indices;
 uint8_t* materialIndices;
 float weldTolerance;
 int vertexCount;
 int triangleCount;
 bool weldVertices;
 bool useMedianSplit;
 bool identifyEdges;
} b3MeshDef;
typedef enum b3MeshEdgeFlags
{
 b3_concaveEdge1 = 0x01,
 b3_concaveEdge2 = 0x02,
 b3_concaveEdge3 = 0x04,
 b3_inverseConcaveEdge1 = 0x10,
 b3_inverseConcaveEdge2 = 0x20,
 b3_inverseConcaveEdge3 = 0x40,
 b3_allConcaveEdges = b3_concaveEdge1 | b3_concaveEdge2 | b3_concaveEdge3,
 b3_flatEdge1 = b3_concaveEdge1 | b3_inverseConcaveEdge1,
 b3_flatEdge2 = b3_concaveEdge2 | b3_inverseConcaveEdge2,
 b3_flatEdge3 = b3_concaveEdge3 | b3_inverseConcaveEdge3,
 b3_allFlatEdges = b3_flatEdge1 | b3_flatEdge2 | b3_flatEdge3,
} b3MeshEdgeFlags;
typedef struct b3MeshTriangle
{
 int32_t index1;
 int32_t index2;
 int32_t index3;
} b3MeshTriangle;
typedef struct b3MeshNode
{
 b3Vec3 lowerBound;
 union
 {
  struct
  {
   uint32_t axis : 2;
   uint32_t childOffset : 30;
  } asNode;
  struct
  {
   uint32_t type : 2;
   uint32_t triangleCount : 30;
  } asLeaf;
 } data;
 b3Vec3 upperBound;
 uint32_t triangleOffset;
} b3MeshNode;
typedef struct b3MeshData
{
 uint64_t version;
 uint64_t hash;
 int32_t byteCount;
 b3AABB bounds;
 float surfaceArea;
 int32_t treeHeight;
 int32_t degenerateCount;
 int32_t nodeOffset;
 int32_t nodeCount;
 int32_t vertexOffset;
 int32_t vertexCount;
 int32_t triangleOffset;
 int32_t triangleCount;
 int32_t materialOffset;
 int32_t materialCount;
 int32_t flagsOffset;
 int32_t padding;
} b3MeshData;
typedef struct b3Mesh
{
 const b3MeshData* data;
 b3Vec3 scale;
} b3Mesh;
typedef struct b3HeightFieldDef
{
 float* heights;
 uint8_t* materialIndices;
 b3Vec3 scale;
 int countX;
 int countZ;
 float globalMinimumHeight;
 float globalMaximumHeight;
 bool clockwiseWinding;
} b3HeightFieldDef;
typedef struct b3HeightFieldData
{
 uint64_t version;
 uint64_t hash;
 int byteCount;
 b3AABB aabb;
 float minHeight;
 float maxHeight;
 float heightScale;
 b3Vec3 scale;
 int32_t columnCount;
 int32_t rowCount;
 int32_t heightsOffset;
 int32_t materialOffset;
 int32_t flagsOffset;
 uint8_t clockwise;
 uint8_t padding[7];
} b3HeightFieldData;
typedef struct b3CompoundCapsuleDef
{
 b3Capsule capsule;
 b3SurfaceMaterial material;
} b3CompoundCapsuleDef;
typedef struct b3CompoundHullDef
{
 const b3HullData* hull;
 b3Transform transform;
 b3SurfaceMaterial material;
} b3CompoundHullDef;
typedef struct b3CompoundMeshDef
{
 const b3MeshData* meshData;
 b3Transform transform;
 b3Vec3 scale;
 const b3SurfaceMaterial* materials;
 int materialCount;
} b3CompoundMeshDef;
typedef struct b3CompoundSphereDef
{
 b3Sphere sphere;
 b3SurfaceMaterial material;
} b3CompoundSphereDef;
typedef struct b3CompoundDef
{
 b3CompoundCapsuleDef* capsules;
 int capsuleCount;
 b3CompoundHullDef* hulls;
 int hullCount;
 b3CompoundMeshDef* meshes;
 int meshCount;
 b3CompoundSphereDef* spheres;
 int sphereCount;
} b3CompoundDef;
typedef struct b3CompoundData
{
 uint64_t version;
 int byteCount;
 int nodeOffset;
 b3DynamicTree tree;
 int materialOffset;
 int materialCount;
 int capsuleOffset;
 int capsuleCount;
 int hullOffset;
 int hullCount;
 int sharedHullCount;
 int meshOffset;
 int meshCount;
 int sharedMeshCount;
 int sphereOffset;
 int sphereCount;
} b3CompoundData;
typedef struct b3CompoundCapsule
{
 b3Capsule capsule;
 int materialIndex;
} b3CompoundCapsule;
typedef struct b3CompoundHull
{
 const b3HullData* hull;
 b3Transform transform;
 int materialIndex;
} b3CompoundHull;
typedef struct b3CompoundMesh
{
 const b3MeshData* meshData;
 b3Transform transform;
 b3Vec3 scale;
 int materialIndices[4];
} b3CompoundMesh;
typedef struct b3CompoundSphere
{
 b3Sphere sphere;
 int materialIndex;
} b3CompoundSphere;
typedef struct b3ChildShape
{
 union
 {
  b3Capsule capsule;
  const b3HullData* hull;
  b3Mesh mesh;
  b3Sphere sphere;
 };
 b3Transform transform;
 int materialIndices[4];
 b3ShapeType type;
} b3ChildShape;
typedef bool b3CompoundQueryFcn( const b3CompoundData* compound, int childIndex, void* context );
typedef struct b3ManifoldPoint
{
 b3Vec3 anchorA;
 b3Vec3 anchorB;
 float separation;
 float baseSeparation;
 float normalImpulse;
 float totalNormalImpulse;
 float normalVelocity;
 uint32_t featureId;
 int triangleIndex;
 bool persisted;
} b3ManifoldPoint;
typedef struct b3Manifold
{
 b3ManifoldPoint points[4];
 b3Vec3 normal;
 float twistImpulse;
 b3Vec3 frictionImpulse;
 b3Vec3 rollingImpulse;
 int pointCount;
} b3Manifold;
typedef enum
{
 b3_invalidAxis = 0,
 b3_backsideAxis,
 b3_faceAxisA,
 b3_faceAxisB,
 b3_edgePairAxis,
 b3_closestPointsAxis,
 b3_manualFaceAxisA,
 b3_manualFaceAxisB,
 b3_manualEdgePairAxis,
} b3SeparatingFeature;
typedef enum
{
 b3_featureNone = 0,
 b3_featureTriangleFace,
 b3_featureHullFace,
 b3_featureEdge1,
 b3_featureEdge2,
 b3_featureEdge3,
 b3_featureVertex1,
 b3_featureVertex2,
 b3_featureVertex3
} b3TriangleFeature;
typedef struct
{
 float separation;
 uint8_t type;
 uint8_t indexA;
 uint8_t indexB;
 uint8_t hit;
} b3SATCache;
typedef struct b3FeaturePair
{
 uint8_t owner1;
 uint8_t index1;
 uint8_t owner2;
 uint8_t index2;
} b3FeaturePair;
typedef struct b3LocalManifoldPoint
{
 b3Vec3 point;
 float separation;
 b3FeaturePair pair;
 int triangleIndex;
} b3LocalManifoldPoint;
typedef struct b3LocalManifold
{
 b3Vec3 normal;
 b3Vec3 triangleNormal;
 b3LocalManifoldPoint* points;
 int pointCount;
 int triangleIndex;
 int i1;
 int i2;
 int i3;
 float squaredDistance;
 b3TriangleFeature feature;
 int triangleFlags;
} b3LocalManifold;
typedef enum b3HexColor
{
 b3_colorAliceBlue = 0xF0F8FF,
 b3_colorAntiqueWhite = 0xFAEBD7,
 b3_colorAqua = 0x00FFFF,
 b3_colorAquamarine = 0x7FFFD4,
 b3_colorAzure = 0xF0FFFF,
 b3_colorBeige = 0xF5F5DC,
 b3_colorBisque = 0xFFE4C4,
 b3_colorBlack = 0x000000,
 b3_colorBlanchedAlmond = 0xFFEBCD,
 b3_colorBlue = 0x0000FF,
 b3_colorBlueViolet = 0x8A2BE2,
 b3_colorBrown = 0xA52A2A,
 b3_colorBurlywood = 0xDEB887,
 b3_colorCadetBlue = 0x5F9EA0,
 b3_colorChartreuse = 0x7FFF00,
 b3_colorChocolate = 0xD2691E,
 b3_colorCoral = 0xFF7F50,
 b3_colorCornflowerBlue = 0x6495ED,
 b3_colorCornsilk = 0xFFF8DC,
 b3_colorCrimson = 0xDC143C,
 b3_colorCyan = 0x00FFFF,
 b3_colorDarkBlue = 0x00008B,
 b3_colorDarkCyan = 0x008B8B,
 b3_colorDarkGoldenRod = 0xB8860B,
 b3_colorDarkGray = 0xA9A9A9,
 b3_colorDarkGreen = 0x006400,
 b3_colorDarkKhaki = 0xBDB76B,
 b3_colorDarkMagenta = 0x8B008B,
 b3_colorDarkOliveGreen = 0x556B2F,
 b3_colorDarkOrange = 0xFF8C00,
 b3_colorDarkOrchid = 0x9932CC,
 b3_colorDarkRed = 0x8B0000,
 b3_colorDarkSalmon = 0xE9967A,
 b3_colorDarkSeaGreen = 0x8FBC8F,
 b3_colorDarkSlateBlue = 0x483D8B,
 b3_colorDarkSlateGray = 0x2F4F4F,
 b3_colorDarkTurquoise = 0x00CED1,
 b3_colorDarkViolet = 0x9400D3,
 b3_colorDeepPink = 0xFF1493,
 b3_colorDeepSkyBlue = 0x00BFFF,
 b3_colorDimGray = 0x696969,
 b3_colorDodgerBlue = 0x1E90FF,
 b3_colorFireBrick = 0xB22222,
 b3_colorFloralWhite = 0xFFFAF0,
 b3_colorForestGreen = 0x228B22,
 b3_colorFuchsia = 0xFF00FF,
 b3_colorGainsboro = 0xDCDCDC,
 b3_colorGhostWhite = 0xF8F8FF,
 b3_colorGold = 0xFFD700,
 b3_colorGoldenRod = 0xDAA520,
 b3_colorGray = 0x808080,
 b3_colorGreen = 0x008000,
 b3_colorGreenYellow = 0xADFF2F,
 b3_colorHoneyDew = 0xF0FFF0,
 b3_colorHotPink = 0xFF69B4,
 b3_colorIndianRed = 0xCD5C5C,
 b3_colorIndigo = 0x4B0082,
 b3_colorIvory = 0xFFFFF0,
 b3_colorKhaki = 0xF0E68C,
 b3_colorLavender = 0xE6E6FA,
 b3_colorLavenderBlush = 0xFFF0F5,
 b3_colorLawnGreen = 0x7CFC00,
 b3_colorLemonChiffon = 0xFFFACD,
 b3_colorLightBlue = 0xADD8E6,
 b3_colorLightCoral = 0xF08080,
 b3_colorLightCyan = 0xE0FFFF,
 b3_colorLightGoldenRodYellow = 0xFAFAD2,
 b3_colorLightGray = 0xD3D3D3,
 b3_colorLightGreen = 0x90EE90,
 b3_colorLightPink = 0xFFB6C1,
 b3_colorLightSalmon = 0xFFA07A,
 b3_colorLightSeaGreen = 0x20B2AA,
 b3_colorLightSkyBlue = 0x87CEFA,
 b3_colorLightSlateGray = 0x778899,
 b3_colorLightSteelBlue = 0xB0C4DE,
 b3_colorLightYellow = 0xFFFFE0,
 b3_colorLime = 0x00FF00,
 b3_colorLimeGreen = 0x32CD32,
 b3_colorLinen = 0xFAF0E6,
 b3_colorMagenta = 0xFF00FF,
 b3_colorMaroon = 0x800000,
 b3_colorMediumAquaMarine = 0x66CDAA,
 b3_colorMediumBlue = 0x0000CD,
 b3_colorMediumOrchid = 0xBA55D3,
 b3_colorMediumPurple = 0x9370DB,
 b3_colorMediumSeaGreen = 0x3CB371,
 b3_colorMediumSlateBlue = 0x7B68EE,
 b3_colorMediumSpringGreen = 0x00FA9A,
 b3_colorMediumTurquoise = 0x48D1CC,
 b3_colorMediumVioletRed = 0xC71585,
 b3_colorMidnightBlue = 0x191970,
 b3_colorMintCream = 0xF5FFFA,
 b3_colorMistyRose = 0xFFE4E1,
 b3_colorMoccasin = 0xFFE4B5,
 b3_colorNavajoWhite = 0xFFDEAD,
 b3_colorNavy = 0x000080,
 b3_colorOldLace = 0xFDF5E6,
 b3_colorOlive = 0x808000,
 b3_colorOliveDrab = 0x6B8E23,
 b3_colorOrange = 0xFFA500,
 b3_colorOrangeRed = 0xFF4500,
 b3_colorOrchid = 0xDA70D6,
 b3_colorPaleGoldenRod = 0xEEE8AA,
 b3_colorPaleGreen = 0x98FB98,
 b3_colorPaleTurquoise = 0xAFEEEE,
 b3_colorPaleVioletRed = 0xDB7093,
 b3_colorPapayaWhip = 0xFFEFD5,
 b3_colorPeachPuff = 0xFFDAB9,
 b3_colorPeru = 0xCD853F,
 b3_colorPink = 0xFFC0CB,
 b3_colorPlum = 0xDDA0DD,
 b3_colorPowderBlue = 0xB0E0E6,
 b3_colorPurple = 0x800080,
 b3_colorRebeccaPurple = 0x663399,
 b3_colorRed = 0xFF0000,
 b3_colorRosyBrown = 0xBC8F8F,
 b3_colorRoyalBlue = 0x4169E1,
 b3_colorSaddleBrown = 0x8B4513,
 b3_colorSalmon = 0xFA8072,
 b3_colorSandyBrown = 0xF4A460,
 b3_colorSeaGreen = 0x2E8B57,
 b3_colorSeaShell = 0xFFF5EE,
 b3_colorSienna = 0xA0522D,
 b3_colorSilver = 0xC0C0C0,
 b3_colorSkyBlue = 0x87CEEB,
 b3_colorSlateBlue = 0x6A5ACD,
 b3_colorSlateGray = 0x708090,
 b3_colorSnow = 0xFFFAFA,
 b3_colorSpringGreen = 0x00FF7F,
 b3_colorSteelBlue = 0x4682B4,
 b3_colorTan = 0xD2B48C,
 b3_colorTeal = 0x008080,
 b3_colorThistle = 0xD8BFD8,
 b3_colorTomato = 0xFF6347,
 b3_colorTurquoise = 0x40E0D0,
 b3_colorViolet = 0xEE82EE,
 b3_colorWheat = 0xF5DEB3,
 b3_colorWhite = 0xFFFFFF,
 b3_colorWhiteSmoke = 0xF5F5F5,
 b3_colorYellow = 0xFFFF00,
 b3_colorYellowGreen = 0x9ACD32,
 b3_colorBox2DRed = 0xDC3132,
 b3_colorBox2DBlue = 0x30AEBF,
 b3_colorBox2DGreen = 0x8CC924,
 b3_colorBox2DYellow = 0xFFEE8C
} b3HexColor;
typedef enum b3DebugMaterial
{
 b3_debugMaterialDefault = 0,
 b3_debugMaterialMatte,
 b3_debugMaterialSoft,
 b3_debugMaterialDead,
 b3_debugMaterialGlossy,
 b3_debugMaterialMetallic
} b3DebugMaterial;
inline uint32_t b3MakeDebugColor( b3HexColor rgb, b3DebugMaterial material )
{
 return ( (uint32_t)rgb & 0x00FFFFFFu ) | ( (uint32_t)material << 24 );
}
extern "C" b3HexColor b3GetGraphColor( int index );
typedef struct b3DebugShape
{
 b3ShapeId shapeId;
 b3ShapeType type;
 union
 {
  const b3Capsule* capsule;
  const b3CompoundData* compound;
  const b3HeightFieldData* heightField;
  const b3HullData* hull;
  const b3Mesh* mesh;
  const b3Sphere* sphere;
 };
} b3DebugShape;
typedef struct b3DebugDraw
{
 void ( *DrawShapeFcn )( void* userShape, b3WorldTransform transform, b3HexColor color, void* context );
 void ( *DrawSegmentFcn )( b3Pos p1, b3Pos p2, b3HexColor color, void* context );
 void ( *DrawTransformFcn )( b3WorldTransform transform, void* context );
 void ( *DrawPointFcn )( b3Pos p, float size, b3HexColor color, void* context );
 void ( *DrawSphereFcn )( b3Pos p, float radius, b3HexColor color, float alpha, void* context );
 void ( *DrawCapsuleFcn )( b3Pos p1, b3Pos p2, float radius, b3HexColor color, float alpha, void* context );
 void ( *DrawBoundsFcn )( b3AABB aabb, b3HexColor color, void* context );
 void ( *DrawBoxFcn )( b3Vec3 extents, b3WorldTransform transform, b3HexColor color, void* context );
 void ( *DrawStringFcn )( b3Pos p, const char* s, b3HexColor color, void* context );
 b3AABB drawingBounds;
 float forceScale;
 float jointScale;
 bool drawShapes;
 bool drawJoints;
 bool drawJointExtras;
 bool drawBounds;
 bool drawMass;
 bool drawSleep;
 bool drawBodyNames;
 bool drawContacts;
 bool drawAnchorA;
 bool drawGraphColors;
 bool drawContactFeatures;
 bool drawContactNormals;
 bool drawContactForces;
 bool drawIslands;
 void* context;
} b3DebugDraw;
extern "C" b3DebugDraw b3DefaultDebugDraw( void );
typedef long int ptrdiff_t;
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
  typedef decltype(nullptr) nullptr_t;
extern "C" b3DynamicTree b3DynamicTree_Create( int proxyCapacity );
extern "C" void b3DynamicTree_Destroy( b3DynamicTree* tree );
extern "C" int b3DynamicTree_CreateProxy( b3DynamicTree* tree, b3AABB aabb, uint64_t categoryBits, uint64_t userData );
extern "C" void b3DynamicTree_DestroyProxy( b3DynamicTree* tree, int proxyId );
extern "C" void b3DynamicTree_MoveProxy( b3DynamicTree* tree, int proxyId, b3AABB aabb );
extern "C" void b3DynamicTree_EnlargeProxy( b3DynamicTree* tree, int proxyId, b3AABB aabb );
extern "C" void b3DynamicTree_SetCategoryBits( b3DynamicTree* tree, int proxyId, uint64_t categoryBits );
extern "C" uint64_t b3DynamicTree_GetCategoryBits( b3DynamicTree* tree, int proxyId );
extern "C" b3TreeStats b3DynamicTree_Query( const b3DynamicTree* tree, b3AABB aabb, uint64_t maskBits, bool requireAllBits,
          b3TreeQueryCallbackFcn* callback, void* context );
extern "C" b3TreeStats b3DynamicTree_QueryClosest( const b3DynamicTree* tree, b3Vec3 point, uint64_t maskBits, bool requireAllBits,
              b3TreeQueryClosestCallbackFcn* callback, void* context, float* minDistanceSqr );
extern "C" b3TreeStats b3DynamicTree_RayCast( const b3DynamicTree* tree, const b3RayCastInput* input, uint64_t maskBits,
            bool requireAllBits, b3TreeRayCastCallbackFcn* callback, void* context );
extern "C" b3TreeStats b3DynamicTree_BoxCast( const b3DynamicTree* tree, const b3BoxCastInput* input, uint64_t maskBits,
            bool requireAllBits, b3TreeBoxCastCallbackFcn* callback, void* context );
extern "C" int b3DynamicTree_GetHeight( const b3DynamicTree* tree );
extern "C" float b3DynamicTree_GetAreaRatio( const b3DynamicTree* tree );
extern "C" b3AABB b3DynamicTree_GetRootBounds( const b3DynamicTree* tree );
extern "C" int b3DynamicTree_GetProxyCount( const b3DynamicTree* tree );
extern "C" int b3DynamicTree_Rebuild( b3DynamicTree* tree, bool fullBuild );
extern "C" int b3DynamicTree_GetByteCount( const b3DynamicTree* tree );
extern "C" void b3DynamicTree_Validate( const b3DynamicTree* tree );
extern "C" void b3DynamicTree_ValidateNoEnlarged( const b3DynamicTree* tree );
extern "C" void b3DynamicTree_Save( const b3DynamicTree* tree, const char* fileName );
extern "C" b3DynamicTree b3DynamicTree_Load( const char* fileName, float scale );
inline uint64_t b3DynamicTree_GetUserData( const b3DynamicTree* tree, int proxyId )
{
 return tree->nodes[proxyId].userData;
}
inline b3AABB b3DynamicTree_GetAABB( const b3DynamicTree* tree, int proxyId )
{
 return tree->nodes[proxyId].aabb;
}
inline const b3HullVertex* b3GetHullVertices( const b3HullData* hull )
{
 if ( hull->vertexOffset == 0 )
 {
  return __null;
 }
 return (const b3HullVertex*)( (intptr_t)hull + hull->vertexOffset );
}
inline const b3Vec3* b3GetHullPoints( const b3HullData* hull )
{
 if ( hull->pointOffset == 0 )
 {
  return __null;
 }
 return (const b3Vec3*)( (intptr_t)hull + hull->pointOffset );
}
inline const b3HullHalfEdge* b3GetHullEdges( const b3HullData* hull )
{
 if ( hull->edgeOffset == 0 )
 {
  return __null;
 }
 return (const b3HullHalfEdge*)( (intptr_t)hull + hull->edgeOffset );
}
inline const b3Plane* b3GetHullPlanes( const b3HullData* hull )
{
 if ( hull->planeOffset == 0 )
 {
  return __null;
 }
 return (const b3Plane*)( (intptr_t)hull + hull->planeOffset );
}
inline const b3HullFace* b3GetHullFaces( const b3HullData* hull )
{
 if ( hull->faceOffset == 0 )
 {
  return __null;
 }
 return (const b3HullFace*)( (intptr_t)hull + hull->faceOffset );
}
inline const float* b3GetHullSoaVertices( const b3HullData* hull )
{
 if ( hull->soaVertexOffset == 0 )
 {
  return __null;
 }
 return (const float*)( (intptr_t)hull + hull->soaVertexOffset );
}
inline const float* b3GetHullSoaNormals( const b3HullData* hull )
{
 if ( hull->soaNormalOffset == 0 )
 {
  return __null;
 }
 return (const float*)( (intptr_t)hull + hull->soaNormalOffset );
}
extern "C" b3HullData* b3CreateCylinder( float height, float radius, float yOffset, int sides );
extern "C" b3HullData* b3CreateCone( float height, float radius1, float radius2, int slices );
extern "C" b3HullData* b3CreateRock( float radius );
extern "C" b3HullData* b3CreateHull( const b3Vec3* points, int pointCount, int maxVertexCount );
extern "C" b3HullData* b3CloneHull( const b3HullData* hull );
extern "C" b3HullData* b3CloneAndTransformHull( const b3HullData* original, b3Transform transform, b3Vec3 scale );
extern "C" void b3DestroyHull( b3HullData* hull );
extern "C" b3BoxHull b3MakeCubeHull( float halfWidth );
extern "C" b3BoxHull b3MakeBoxHull( float hx, float hy, float hz );
extern "C" b3BoxHull b3MakeOffsetBoxHull( float hx, float hy, float hz, b3Vec3 offset );
extern "C" b3BoxHull b3MakeTransformedBoxHull( float hx, float hy, float hz, b3Transform transform );
extern "C" b3BoxHull b3MakeScaledBoxHull( b3Vec3 halfWidths, b3Transform transform, b3Vec3 postScale );
extern "C" void b3ScaleBox( b3Vec3* halfWidths, b3Transform* transform, b3Vec3 postScale, float minHalfWidth );
inline const b3MeshNode* b3GetMeshNodes( const b3MeshData* mesh )
{
 if ( mesh->nodeOffset == 0 )
 {
  return __null;
 }
 return (const b3MeshNode*)( (intptr_t)mesh + mesh->nodeOffset );
}
inline const b3Vec3* b3GetMeshVertices( const b3MeshData* mesh )
{
 if ( mesh->vertexOffset == 0 )
 {
  return __null;
 }
 return (const b3Vec3*)( (intptr_t)mesh + mesh->vertexOffset );
}
inline const b3MeshTriangle* b3GetMeshTriangles( const b3MeshData* mesh )
{
 if ( mesh->triangleOffset == 0 )
 {
  return __null;
 }
 return (const b3MeshTriangle*)( (intptr_t)mesh + mesh->triangleOffset );
}
inline const uint8_t* b3GetMeshMaterialIndices( const b3MeshData* mesh )
{
 if ( mesh->materialOffset == 0 )
 {
  return __null;
 }
 return (const uint8_t*)( (intptr_t)mesh + mesh->materialOffset );
}
inline const uint8_t* b3GetMeshFlags( const b3MeshData* mesh )
{
 if ( mesh->flagsOffset == 0 )
 {
  return __null;
 }
 return (const uint8_t*)( (intptr_t)mesh + mesh->flagsOffset );
}
extern "C" b3MeshData* b3CreateGridMesh( int xCount, int zCount, float cellWidth, int materialCount, bool identifyEdges );
extern "C" b3MeshData* b3CreateWaveMesh( int xCount, int zCount, float cellWidth, float amplitude, float rowFrequency,
          float columnFrequency );
extern "C" b3MeshData* b3CreateTorusMesh( int radialResolution, int tubularResolution, float radius, float thickness );
extern "C" b3MeshData* b3CreateBoxMesh( b3Vec3 center, b3Vec3 extent, bool identifyEdges );
extern "C" b3MeshData* b3CreateHollowBoxMesh( b3Vec3 center, b3Vec3 extent );
extern "C" b3MeshData* b3CreatePlatformMesh( b3Vec3 center, float height, float topWidth, float bottomWidth );
extern "C" b3MeshData* b3CreateMesh( const b3MeshDef* def, int* degenerateTriangleIndices, int degenerateCapacity );
extern "C" void b3DestroyMesh( b3MeshData* mesh );
extern "C" int b3GetHeight( const b3MeshData* mesh );
inline const uint16_t* b3GetHeightFieldCompressedHeights( const b3HeightFieldData* hf )
{
 if ( hf->heightsOffset == 0 )
 {
  return __null;
 }
 return (const uint16_t*)( (intptr_t)hf + hf->heightsOffset );
}
inline const uint8_t* b3GetHeightFieldMaterialIndices( const b3HeightFieldData* hf )
{
 if ( hf->materialOffset == 0 )
 {
  return __null;
 }
 return (const uint8_t*)( (intptr_t)hf + hf->materialOffset );
}
inline const uint8_t* b3GetHeightFieldFlags( const b3HeightFieldData* hf )
{
 if ( hf->flagsOffset == 0 )
 {
  return __null;
 }
 return (const uint8_t*)( (intptr_t)hf + hf->flagsOffset );
}
extern "C" b3HeightFieldData* b3CreateHeightField( const b3HeightFieldDef* data );
extern "C" b3HeightFieldData* b3CreateGrid( int rowCount, int columnCount, b3Vec3 scale, bool makeHoles );
extern "C" b3HeightFieldData* b3CreateWave( int rowCount, int columnCount, b3Vec3 scale, float rowFrequency, float columnFrequency,
          bool makeHoles );
extern "C" void b3DestroyHeightField( b3HeightFieldData* heightField );
extern "C" void b3DumpHeightData( const b3HeightFieldDef* data, const char* fileName );
extern "C" b3HeightFieldData* b3LoadHeightField( const char* fileName );
extern "C" b3ChildShape b3GetCompoundChild( const b3CompoundData* compound, int childIndex );
extern "C" void b3QueryCompound( const b3CompoundData* compound, b3AABB aabb, b3CompoundQueryFcn* fcn, void* context );
extern "C" b3CompoundCapsule b3GetCompoundCapsule( const b3CompoundData* compound, int index );
extern "C" b3CompoundHull b3GetCompoundHull( const b3CompoundData* compound, int index );
extern "C" b3CompoundMesh b3GetCompoundMesh( const b3CompoundData* compound, int index );
extern "C" b3CompoundSphere b3GetCompoundSphere( const b3CompoundData* compound, int index );
extern "C" const b3SurfaceMaterial* b3GetCompoundMaterials( const b3CompoundData* compound );
extern "C" b3CompoundData* b3CreateCompound( const b3CompoundDef* def );
extern "C" void b3DestroyCompound( b3CompoundData* compound );
extern "C" uint8_t* b3ConvertCompoundToBytes( b3CompoundData* compound );
extern "C" b3CompoundData* b3ConvertBytesToCompound( uint8_t* bytes, int byteCount );
extern "C" b3MassData b3ComputeSphereMass( const b3Sphere* shape, float density );
extern "C" b3MassData b3ComputeCapsuleMass( const b3Capsule* shape, float density );
extern "C" b3MassData b3ComputeHullMass( const b3HullData* shape, float density );
extern "C" b3AABB b3ComputeSphereAABB( const b3Sphere* shape, b3Transform transform );
extern "C" b3AABB b3ComputeCapsuleAABB( const b3Capsule* shape, b3Transform transform );
extern "C" b3AABB b3ComputeHullAABB( const b3HullData* shape, b3Transform transform );
extern "C" b3AABB b3ComputeMeshAABB( const b3MeshData* shape, b3Transform transform, b3Vec3 scale );
extern "C" b3AABB b3ComputeHeightFieldAABB( const b3HeightFieldData* shape, b3Transform transform );
extern "C" b3AABB b3ComputeCompoundAABB( const b3CompoundData* shape, b3Transform transform );
extern "C" bool b3IsValidRay( const b3RayCastInput* input );
extern "C" bool b3OverlapCapsule( const b3Capsule* shape, b3Transform shapeTransform, const b3ShapeProxy* proxy );
extern "C" bool b3OverlapCompound( const b3CompoundData* shape, b3Transform shapeTransform, const b3ShapeProxy* proxy );
extern "C" bool b3OverlapHeightField( const b3HeightFieldData* shape, b3Transform shapeTransform, const b3ShapeProxy* proxy );
extern "C" bool b3OverlapHull( const b3HullData* shape, b3Transform shapeTransform, const b3ShapeProxy* proxy );
extern "C" bool b3OverlapMesh( const b3Mesh* shape, b3Transform shapeTransform, const b3ShapeProxy* proxy );
extern "C" bool b3OverlapSphere( const b3Sphere* shape, b3Transform shapeTransform, const b3ShapeProxy* proxy );
extern "C" b3CastOutput b3RayCastSphere( const b3Sphere* shape, const b3RayCastInput* input );
extern "C" b3CastOutput b3RayCastHollowSphere( const b3Sphere* shape, const b3RayCastInput* input );
extern "C" b3CastOutput b3RayCastCapsule( const b3Capsule* shape, const b3RayCastInput* input );
extern "C" b3CastOutput b3RayCastCompound( const b3CompoundData* shape, const b3RayCastInput* input );
extern "C" b3CastOutput b3RayCastHull( const b3HullData* shape, const b3RayCastInput* input );
extern "C" b3CastOutput b3RayCastMesh( const b3Mesh* shape, const b3RayCastInput* input );
extern "C" b3CastOutput b3RayCastHeightField( const b3HeightFieldData* shape, const b3RayCastInput* input );
extern "C" b3CastOutput b3ShapeCastSphere( const b3Sphere* shape, const b3ShapeCastInput* input );
extern "C" b3CastOutput b3ShapeCastCapsule( const b3Capsule* shape, const b3ShapeCastInput* input );
extern "C" b3CastOutput b3ShapeCastCompound( const b3CompoundData* shape, const b3ShapeCastInput* input );
extern "C" b3CastOutput b3ShapeCastHull( const b3HullData* shape, const b3ShapeCastInput* input );
extern "C" b3CastOutput b3ShapeCastMesh( const b3Mesh* shape, const b3ShapeCastInput* input );
extern "C" b3CastOutput b3ShapeCastHeightField( const b3HeightFieldData* shape, const b3ShapeCastInput* input );
typedef bool b3MeshQueryFcn( b3Vec3 a, b3Vec3 b, b3Vec3 c, int triangleIndex, void* context );
extern "C" void b3QueryMesh( const b3Mesh* mesh, const b3AABB bounds, b3MeshQueryFcn* fcn, void* context );
extern "C" void b3QueryHeightField( const b3HeightFieldData* heightField, b3AABB bounds, b3MeshQueryFcn* fcn, void* context );
extern "C" b3DistanceOutput b3ShapeDistance( const b3DistanceInput* input, b3SimplexCache* cache, b3Simplex* simplexes,
           int simplexCapacity );
extern "C" b3CastOutput b3ShapeCast( const b3ShapeCastPairInput* input );
extern "C" b3Transform b3GetSweepTransform( const b3Sweep* sweep, float time );
extern "C" b3TOIOutput b3TimeOfImpact( const b3TOIInput* input );
extern "C" void b3CollideSpheres( b3LocalManifold* manifold, int capacity, const b3Sphere* sphereA, const b3Sphere* sphereB,
         b3Transform transformBtoA );
extern "C" void b3CollideCapsuleAndSphere( b3LocalManifold* manifold, int capacity, const b3Capsule* capsuleA,
            const b3Sphere* sphereB, b3Transform transformBtoA );
extern "C" void b3CollideHullAndSphere( b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3Sphere* sphereB,
         b3Transform transformBtoA, b3SimplexCache* cache );
extern "C" void b3CollideCapsules( b3LocalManifold* manifold, int capacity, const b3Capsule* capsuleA, const b3Capsule* capsuleB,
          b3Transform transformBtoA );
extern "C" void b3CollideHullAndCapsule( b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3Capsule* capsuleB,
          b3Transform transformBtoA, b3SimplexCache* cache );
extern "C" void b3CollideHulls( b3LocalManifold* manifold, int capacity, const b3HullData* hullA, const b3HullData* hullB,
       b3Transform transformBtoA, b3SATCache* cache );
extern "C" void b3CollideTriangleAndCapsule( b3LocalManifold* manifold, int capacity, const b3Vec3* triangleA,
           const b3Capsule* capsuleB, b3SimplexCache* cache );
extern "C" void b3CollideTriangleAndHull( b3LocalManifold* manifold, int capacity, b3Vec3 v1, b3Vec3 v2, b3Vec3 v3, int triangleFlags,
           const b3HullData* hullB, b3SATCache* cache, bool enableSpeculative );
extern "C" void b3CollideTriangleAndSphere( b3LocalManifold* manifold, int capacity, const b3Vec3* triangleA,
          const b3Sphere* sphereB );
extern "C" b3PlaneSolverResult b3SolvePlanes( b3Vec3 targetDelta, b3CollisionPlane* planes, int count );
extern "C" b3Vec3 b3ClipVector( b3Vec3 vector, const b3CollisionPlane* planes, int count );
extern "C" b3WorldId b3CreateWorld( const b3WorldDef* def );
extern "C" void b3DestroyWorld( b3WorldId worldId );
extern "C" int b3GetWorldCount( void );
extern "C" int b3GetMaxWorldCount( void );
extern "C" bool b3World_IsValid( b3WorldId id );
extern "C" void b3World_Step( b3WorldId worldId, float timeStep, int subStepCount );
extern "C" void b3World_Draw( b3WorldId worldId, b3DebugDraw* draw, uint64_t maskBits );
extern "C" b3AABB b3World_GetBounds( b3WorldId worldId );
extern "C" b3BodyEvents b3World_GetBodyEvents( b3WorldId worldId );
extern "C" b3SensorEvents b3World_GetSensorEvents( b3WorldId worldId );
extern "C" b3ContactEvents b3World_GetContactEvents( b3WorldId worldId );
extern "C" b3JointEvents b3World_GetJointEvents( b3WorldId worldId );
extern "C" b3TreeStats b3World_OverlapAABB( b3WorldId worldId, b3AABB aabb, b3QueryFilter filter, b3OverlapResultFcn* fcn,
          void* context );
extern "C" b3TreeStats b3World_OverlapShape( b3WorldId worldId, b3Pos origin, const b3ShapeProxy* proxy, b3QueryFilter filter,
           b3OverlapResultFcn* fcn, void* context );
extern "C" b3TreeStats b3World_CastRay( b3WorldId worldId, b3Pos origin, b3Vec3 translation, b3QueryFilter filter,
         b3CastResultFcn* fcn, void* context );
extern "C" b3RayResult b3World_CastRayClosest( b3WorldId worldId, b3Pos origin, b3Vec3 translation, b3QueryFilter filter );
extern "C" b3TreeStats b3World_CastShape( b3WorldId worldId, b3Pos origin, const b3ShapeProxy* proxy, b3Vec3 translation,
           b3QueryFilter filter, b3CastResultFcn* fcn, void* context );
extern "C" float b3World_CastMover( b3WorldId worldId, b3Pos origin, const b3Capsule* mover, b3Vec3 translation, b3QueryFilter filter,
        b3MoverFilterFcn* fcn, void* context );
extern "C" void b3World_CollideMover( b3WorldId worldId, b3Pos origin, const b3Capsule* mover, b3QueryFilter filter,
          b3PlaneResultFcn* fcn, void* context );
extern "C" void b3World_EnableSleeping( b3WorldId worldId, bool flag );
extern "C" bool b3World_IsSleepingEnabled( b3WorldId worldId );
extern "C" void b3World_EnableContinuous( b3WorldId worldId, bool flag );
extern "C" bool b3World_IsContinuousEnabled( b3WorldId worldId );
extern "C" void b3World_SetRestitutionThreshold( b3WorldId worldId, float value );
extern "C" float b3World_GetRestitutionThreshold( b3WorldId worldId );
extern "C" void b3World_SetHitEventThreshold( b3WorldId worldId, float value );
extern "C" float b3World_GetHitEventThreshold( b3WorldId worldId );
extern "C" void b3World_SetCustomFilterCallback( b3WorldId worldId, b3CustomFilterFcn* fcn, void* context );
extern "C" void b3World_SetPreSolveCallback( b3WorldId worldId, b3PreSolveFcn* fcn, void* context );
extern "C" void b3World_SetGravity( b3WorldId worldId, b3Vec3 gravity );
extern "C" b3Vec3 b3World_GetGravity( b3WorldId worldId );
extern "C" void b3World_Explode( b3WorldId worldId, const b3ExplosionDef* explosionDef );
extern "C" void b3World_SetContactTuning( b3WorldId worldId, float hertz, float dampingRatio, float contactSpeed );
extern "C" void b3World_SetContactRecycleDistance( b3WorldId worldId, float recycleDistance );
extern "C" float b3World_GetContactRecycleDistance( b3WorldId worldId );
extern "C" void b3World_SetMaximumLinearSpeed( b3WorldId worldId, float maximumLinearSpeed );
extern "C" float b3World_GetMaximumLinearSpeed( b3WorldId worldId );
extern "C" void b3World_EnableWarmStarting( b3WorldId worldId, bool flag );
extern "C" bool b3World_IsWarmStartingEnabled( b3WorldId worldId );
extern "C" int b3World_GetAwakeBodyCount( b3WorldId worldId );
extern "C" b3Profile b3World_GetProfile( b3WorldId worldId );
extern "C" b3Counters b3World_GetCounters( b3WorldId worldId );
extern "C" b3Capacity b3World_GetMaxCapacity( b3WorldId worldId );
extern "C" void b3World_SetUserData( b3WorldId worldId, void* userData );
extern "C" void* b3World_GetUserData( b3WorldId worldId );
extern "C" void b3World_SetFrictionCallback( b3WorldId worldId, b3FrictionCallback* callback );
extern "C" void b3World_SetRestitutionCallback( b3WorldId worldId, b3RestitutionCallback* callback );
extern "C" void b3World_SetWorkerCount( b3WorldId worldId, int count );
extern "C" int b3World_GetWorkerCount( b3WorldId worldId );
extern "C" void b3World_DumpMemoryStats( b3WorldId worldId );
extern "C" void b3World_DumpShapeBounds( b3WorldId worldId, b3BodyType type );
extern "C" void b3World_RebuildStaticTree( b3WorldId worldId );
extern "C" void b3World_EnableSpeculative( b3WorldId worldId, bool flag );
typedef struct b3Recording b3Recording;
extern "C" b3Recording* b3CreateRecording( int byteCapacity );
extern "C" void b3DestroyRecording( b3Recording* recording );
extern "C" const uint8_t* b3Recording_GetData( const b3Recording* recording );
extern "C" int b3Recording_GetSize( const b3Recording* recording );
extern "C" void b3World_StartRecording( b3WorldId worldId, b3Recording* recording );
extern "C" void b3World_StopRecording( b3WorldId worldId );
extern "C" bool b3SaveRecordingToFile( const b3Recording* recording, const char* path );
extern "C" b3Recording* b3LoadRecordingFromFile( const char* path );
extern "C" bool b3ValidateReplay( const void* data, int size, int workerCount );
typedef struct b3RecPlayer b3RecPlayer;
typedef struct b3RecPlayerInfo
{
 int frameCount;
 int workerCount;
 float timeStep;
 int subStepCount;
 float lengthScale;
 b3AABB bounds;
} b3RecPlayerInfo;
extern "C" b3RecPlayer* b3CreatePlayer( const void* data, int size, int workerCount );
extern "C" void b3DestroyPlayer( b3RecPlayer* player );
extern "C" bool b3RecPlayer_StepFrame( b3RecPlayer* player );
extern "C" void b3RecPlayer_SubStepFrame( b3RecPlayer* player );
extern "C" void b3RecPlayer_Restart( b3RecPlayer* player );
extern "C" void b3RecPlayer_SeekFrame( b3RecPlayer* player, int targetFrame );
extern "C" b3WorldId b3RecPlayer_GetWorldId( const b3RecPlayer* player );
extern "C" int b3RecPlayer_GetFrame( const b3RecPlayer* player );
extern "C" int b3RecPlayer_GetFrameCount( const b3RecPlayer* player );
extern "C" bool b3RecPlayer_IsAtEnd( const b3RecPlayer* player );
extern "C" bool b3RecPlayer_IsAtPreStep( const b3RecPlayer* player );
extern "C" bool b3RecPlayer_HasDiverged( const b3RecPlayer* player );
extern "C" b3RecPlayerInfo b3RecPlayer_GetInfo( const b3RecPlayer* player );
extern "C" int b3RecPlayer_GetDivergeFrame( const b3RecPlayer* player );
extern "C" void b3RecPlayer_SetWorkerCount( b3RecPlayer* player, int count );
extern "C" void b3RecPlayer_SetKeyframePolicy( b3RecPlayer* player, size_t budgetBytes, int minIntervalFrames );
extern "C" size_t b3RecPlayer_GetKeyframeBudget( const b3RecPlayer* player );
extern "C" int b3RecPlayer_GetKeyframeMinInterval( const b3RecPlayer* player );
extern "C" int b3RecPlayer_GetKeyframeInterval( const b3RecPlayer* player );
extern "C" size_t b3RecPlayer_GetKeyframeBytes( const b3RecPlayer* player );
extern "C" int b3RecPlayer_GetBodyCount( const b3RecPlayer* player );
extern "C" b3BodyId b3RecPlayer_GetBodyId( const b3RecPlayer* player, int index );
extern "C" void b3RecPlayer_SetDebugShapeCallbacks( b3RecPlayer* player, b3CreateDebugShapeCallback* createDebugShape,
            b3DestroyDebugShapeCallback* destroyDebugShape, void* context );
extern "C" void b3RecPlayer_DrawFrameQueries( b3RecPlayer* player, b3DebugDraw* draw, int queryIndex, int selectedIndex );
typedef enum b3RecQueryType
{
 b3_recQueryOverlapAABB,
 b3_recQueryOverlapShape,
 b3_recQueryCastRay,
 b3_recQueryCastShape,
 b3_recQueryCastRayClosest,
 b3_recQueryCastMover,
 b3_recQueryCollideMover,
} b3RecQueryType;
typedef struct b3RecQueryInfo
{
 b3RecQueryType type;
 b3QueryFilter filter;
 b3AABB aabb;
 b3Pos origin;
 b3Vec3 translation;
 int hitCount;
 uint64_t key;
 uint64_t id;
 const char* name;
} b3RecQueryInfo;
typedef struct b3RecQueryHit
{
 b3ShapeId shape;
 b3Pos point;
 b3Vec3 normal;
 float fraction;
} b3RecQueryHit;
extern "C" int b3RecPlayer_GetFrameQueryCount( const b3RecPlayer* player );
extern "C" b3RecQueryInfo b3RecPlayer_GetFrameQuery( const b3RecPlayer* player, int index );
extern "C" b3RecQueryHit b3RecPlayer_GetFrameQueryHit( const b3RecPlayer* player, int queryIndex, int hitIndex );
extern "C" b3BodyId b3CreateBody( b3WorldId worldId, const b3BodyDef* def );
extern "C" void b3DestroyBody( b3BodyId bodyId );
extern "C" bool b3Body_IsValid( b3BodyId id );
extern "C" b3BodyType b3Body_GetType( b3BodyId bodyId );
extern "C" void b3Body_SetType( b3BodyId bodyId, b3BodyType type );
extern "C" void b3Body_SetName( b3BodyId bodyId, const char* name );
extern "C" const char* b3Body_GetName( b3BodyId bodyId );
extern "C" void b3Body_SetUserData( b3BodyId bodyId, void* userData );
extern "C" void* b3Body_GetUserData( b3BodyId bodyId );
extern "C" b3Pos b3Body_GetPosition( b3BodyId bodyId );
extern "C" b3Quat b3Body_GetRotation( b3BodyId bodyId );
extern "C" b3WorldTransform b3Body_GetTransform( b3BodyId bodyId );
extern "C" void b3Body_SetTransform( b3BodyId bodyId, b3Pos position, b3Quat rotation );
extern "C" b3Vec3 b3Body_GetLocalPoint( b3BodyId bodyId, b3Pos worldPoint );
extern "C" b3Pos b3Body_GetWorldPoint( b3BodyId bodyId, b3Vec3 localPoint );
extern "C" b3Vec3 b3Body_GetLocalVector( b3BodyId bodyId, b3Vec3 worldVector );
extern "C" b3Vec3 b3Body_GetWorldVector( b3BodyId bodyId, b3Vec3 localVector );
extern "C" b3Vec3 b3Body_GetLinearVelocity( b3BodyId bodyId );
extern "C" b3Vec3 b3Body_GetAngularVelocity( b3BodyId bodyId );
extern "C" void b3Body_SetLinearVelocity( b3BodyId bodyId, b3Vec3 linearVelocity );
extern "C" void b3Body_SetAngularVelocity( b3BodyId bodyId, b3Vec3 angularVelocity );
extern "C" void b3Body_SetTargetTransform( b3BodyId bodyId, b3WorldTransform target, float timeStep, bool wake );
extern "C" b3Vec3 b3Body_GetLocalPointVelocity( b3BodyId bodyId, b3Vec3 localPoint );
extern "C" b3Vec3 b3Body_GetWorldPointVelocity( b3BodyId bodyId, b3Pos worldPoint );
extern "C" void b3Body_ApplyForce( b3BodyId bodyId, b3Vec3 force, b3Pos point, bool wake );
extern "C" void b3Body_ApplyForceToCenter( b3BodyId bodyId, b3Vec3 force, bool wake );
extern "C" void b3Body_ApplyTorque( b3BodyId bodyId, b3Vec3 torque, bool wake );
extern "C" void b3Body_ApplyLinearImpulse( b3BodyId bodyId, b3Vec3 impulse, b3Pos point, bool wake );
extern "C" void b3Body_ApplyLinearImpulseToCenter( b3BodyId bodyId, b3Vec3 impulse, bool wake );
extern "C" void b3Body_ApplyAngularImpulse( b3BodyId bodyId, b3Vec3 impulse, bool wake );
extern "C" float b3Body_GetMass( b3BodyId bodyId );
extern "C" b3Matrix3 b3Body_GetLocalRotationalInertia( b3BodyId bodyId );
extern "C" float b3Body_GetInverseMass( b3BodyId bodyId );
extern "C" b3Matrix3 b3Body_GetWorldInverseRotationalInertia( b3BodyId bodyId );
extern "C" b3Vec3 b3Body_GetLocalCenter( b3BodyId bodyId );
extern "C" b3Pos b3Body_GetWorldCenter( b3BodyId bodyId );
extern "C" void b3Body_SetMassData( b3BodyId bodyId, b3MassData massData );
extern "C" b3MassData b3Body_GetMassData( b3BodyId bodyId );
extern "C" void b3Body_ApplyMassFromShapes( b3BodyId bodyId );
extern "C" void b3Body_SetLinearDamping( b3BodyId bodyId, float linearDamping );
extern "C" float b3Body_GetLinearDamping( b3BodyId bodyId );
extern "C" void b3Body_SetAngularDamping( b3BodyId bodyId, float angularDamping );
extern "C" float b3Body_GetAngularDamping( b3BodyId bodyId );
extern "C" void b3Body_SetGravityScale( b3BodyId bodyId, float gravityScale );
extern "C" float b3Body_GetGravityScale( b3BodyId bodyId );
extern "C" bool b3Body_IsAwake( b3BodyId bodyId );
extern "C" void b3Body_SetAwake( b3BodyId bodyId, bool awake );
extern "C" void b3Body_EnableSleep( b3BodyId bodyId, bool enableSleep );
extern "C" bool b3Body_IsSleepEnabled( b3BodyId bodyId );
extern "C" void b3Body_SetSleepThreshold( b3BodyId bodyId, float sleepThreshold );
extern "C" float b3Body_GetSleepThreshold( b3BodyId bodyId );
extern "C" bool b3Body_IsEnabled( b3BodyId bodyId );
extern "C" void b3Body_Disable( b3BodyId bodyId );
extern "C" void b3Body_Enable( b3BodyId bodyId );
extern "C" void b3Body_SetMotionLocks( b3BodyId bodyId, b3MotionLocks locks );
extern "C" b3MotionLocks b3Body_GetMotionLocks( b3BodyId bodyId );
extern "C" void b3Body_SetBullet( b3BodyId bodyId, bool flag );
extern "C" bool b3Body_IsBullet( b3BodyId bodyId );
extern "C" void b3Body_AllowFastRotation( b3BodyId bodyId, bool flag );
extern "C" bool b3Body_IsFastRotationAllowed( b3BodyId bodyId );
extern "C" void b3Body_EnableContactRecycling( b3BodyId bodyId, bool flag );
extern "C" bool b3Body_IsContactRecyclingEnabled( b3BodyId bodyId );
extern "C" void b3Body_EnableHitEvents( b3BodyId bodyId, bool flag );
extern "C" b3WorldId b3Body_GetWorld( b3BodyId bodyId );
extern "C" int b3Body_GetShapeCount( b3BodyId bodyId );
extern "C" int b3Body_GetShapes( b3BodyId bodyId, b3ShapeId* shapeArray, int capacity );
extern "C" int b3Body_GetJointCount( b3BodyId bodyId );
extern "C" int b3Body_GetJoints( b3BodyId bodyId, b3JointId* jointArray, int capacity );
extern "C" int b3Body_GetContactCapacity( b3BodyId bodyId );
extern "C" int b3Body_GetContactData( b3BodyId bodyId, b3ContactData* contactData, int capacity );
extern "C" b3AABB b3Body_ComputeAABB( b3BodyId bodyId );
extern "C" float b3Body_GetClosestPoint( b3BodyId bodyId, b3Vec3* result, b3Vec3 target );
extern "C" b3BodyCastResult b3Body_CastRay( b3BodyId bodyId, b3Pos origin, b3Vec3 translation, b3QueryFilter filter,
          float maxFraction, b3WorldTransform bodyTransform );
extern "C" b3BodyCastResult b3Body_CastShape( b3BodyId bodyId, b3Pos origin, const b3ShapeProxy* proxy, b3Vec3 translation,
            b3QueryFilter filter, float maxFraction, bool canEncroach,
            b3WorldTransform bodyTransform );
extern "C" bool b3Body_OverlapShape( b3BodyId bodyId, b3Pos origin, const b3ShapeProxy* proxy, b3QueryFilter filter,
         b3WorldTransform bodyTransform );
extern "C" int b3Body_CollideMover( b3BodyId bodyId, b3BodyPlaneResult* bodyPlanes, int planeCapacity, b3Pos origin,
        const b3Capsule* mover, b3QueryFilter filter, b3WorldTransform bodyTransform );
extern "C" b3ShapeId b3CreateSphereShape( b3BodyId bodyId, const b3ShapeDef* def, const b3Sphere* sphere );
extern "C" b3ShapeId b3CreateCapsuleShape( b3BodyId bodyId, const b3ShapeDef* def, const b3Capsule* capsule );
extern "C" b3ShapeId b3CreateHullShape( b3BodyId bodyId, const b3ShapeDef* def, const b3HullData* hull );
extern "C" b3ShapeId b3CreateTransformedHullShape( b3BodyId bodyId, const b3ShapeDef* def, const b3HullData* hull,
              b3Transform transform, b3Vec3 scale );
extern "C" b3ShapeId b3CreateMeshShape( b3BodyId bodyId, const b3ShapeDef* def, const b3MeshData* mesh, b3Vec3 scale );
extern "C" b3ShapeId b3CreateHeightFieldShape( b3BodyId bodyId, const b3ShapeDef* def, const b3HeightFieldData* heightField );
extern "C" b3ShapeId b3CreateBakedCompoundShape( b3BodyId bodyId, b3ShapeDef* def, const b3CompoundData* compound );
extern "C" void b3DestroyShape( b3ShapeId shapeId, bool updateBodyMass );
extern "C" bool b3Shape_IsValid( b3ShapeId id );
extern "C" b3ShapeType b3Shape_GetType( b3ShapeId shapeId );
extern "C" b3BodyId b3Shape_GetBody( b3ShapeId shapeId );
extern "C" b3WorldId b3Shape_GetWorld( b3ShapeId shapeId );
extern "C" bool b3Shape_IsSensor( b3ShapeId shapeId );
extern "C" void b3Shape_SetName( b3ShapeId shapeId, const char* name );
extern "C" const char* b3Shape_GetName( b3ShapeId shapeId );
extern "C" void b3Shape_SetUserData( b3ShapeId shapeId, void* userData );
extern "C" void* b3Shape_GetUserData( b3ShapeId shapeId );
extern "C" void b3Shape_SetDensity( b3ShapeId shapeId, float density, bool updateBodyMass );
extern "C" float b3Shape_GetDensity( b3ShapeId shapeId );
extern "C" void b3Shape_SetFriction( b3ShapeId shapeId, float friction );
extern "C" float b3Shape_GetFriction( b3ShapeId shapeId );
extern "C" void b3Shape_SetRestitution( b3ShapeId shapeId, float restitution );
extern "C" float b3Shape_GetRestitution( b3ShapeId shapeId );
extern "C" void b3Shape_SetSurfaceMaterial( b3ShapeId shapeId, b3SurfaceMaterial surfaceMaterial );
extern "C" b3SurfaceMaterial b3Shape_GetSurfaceMaterial( b3ShapeId shapeId );
extern "C" int b3Shape_GetMeshMaterialCount( b3ShapeId shapeId );
extern "C" void b3Shape_SetMeshMaterial( b3ShapeId shapeId, b3SurfaceMaterial surfaceMaterial, int index );
extern "C" b3SurfaceMaterial b3Shape_GetMeshSurfaceMaterial( b3ShapeId shapeId, int index );
extern "C" b3Filter b3Shape_GetFilter( b3ShapeId shapeId );
extern "C" void b3Shape_SetFilter( b3ShapeId shapeId, b3Filter filter, bool invokeContacts );
extern "C" void b3Shape_EnableSensorEvents( b3ShapeId shapeId, bool flag );
extern "C" bool b3Shape_AreSensorEventsEnabled( b3ShapeId shapeId );
extern "C" void b3Shape_EnableContactEvents( b3ShapeId shapeId, bool flag );
extern "C" bool b3Shape_AreContactEventsEnabled( b3ShapeId shapeId );
extern "C" void b3Shape_EnablePreSolveEvents( b3ShapeId shapeId, bool flag );
extern "C" bool b3Shape_ArePreSolveEventsEnabled( b3ShapeId shapeId );
extern "C" void b3Shape_EnableHitEvents( b3ShapeId shapeId, bool flag );
extern "C" bool b3Shape_AreHitEventsEnabled( b3ShapeId shapeId );
extern "C" b3WorldCastOutput b3Shape_RayCast( b3ShapeId shapeId, b3Pos origin, b3Vec3 translation );
extern "C" b3Sphere b3Shape_GetSphere( b3ShapeId shapeId );
extern "C" b3Capsule b3Shape_GetCapsule( b3ShapeId shapeId );
extern "C" const b3HullData* b3Shape_GetHull( b3ShapeId shapeId );
extern "C" b3Mesh b3Shape_GetMesh( b3ShapeId shapeId );
extern "C" const b3HeightFieldData* b3Shape_GetHeightField( b3ShapeId shapeId );
extern "C" void b3Shape_SetSphere( b3ShapeId shapeId, const b3Sphere* sphere );
extern "C" void b3Shape_SetCapsule( b3ShapeId shapeId, const b3Capsule* capsule );
extern "C" void b3Shape_SetHull( b3ShapeId shapeId, const b3HullData* hull );
extern "C" void b3Shape_SetMesh( b3ShapeId shapeId, const b3MeshData* meshData, b3Vec3 scale );
extern "C" int b3Shape_GetContactCapacity( b3ShapeId shapeId );
extern "C" int b3Shape_GetContactData( b3ShapeId shapeId, b3ContactData* contactData, int capacity );
extern "C" int b3Shape_GetSensorCapacity( b3ShapeId shapeId );
extern "C" int b3Shape_GetSensorData( b3ShapeId shapeId, b3ShapeId* visitorIds, int capacity );
extern "C" b3AABB b3Shape_GetAABB( b3ShapeId shapeId );
extern "C" b3MassData b3Shape_ComputeMassData( b3ShapeId shapeId );
extern "C" b3Vec3 b3Shape_GetClosestPoint( b3ShapeId shapeId, b3Vec3 target );
extern "C" void b3Shape_ApplyWind( b3ShapeId shapeId, b3Vec3 wind, float drag, float lift, float maxSpeed, bool wake );
extern "C" void b3DestroyJoint( b3JointId jointId, bool wakeAttached );
extern "C" bool b3Joint_IsValid( b3JointId id );
extern "C" b3JointType b3Joint_GetType( b3JointId jointId );
extern "C" b3BodyId b3Joint_GetBodyA( b3JointId jointId );
extern "C" b3BodyId b3Joint_GetBodyB( b3JointId jointId );
extern "C" b3WorldId b3Joint_GetWorld( b3JointId jointId );
extern "C" void b3Joint_SetLocalFrameA( b3JointId jointId, b3Transform localFrame );
extern "C" b3Transform b3Joint_GetLocalFrameA( b3JointId jointId );
extern "C" void b3Joint_SetLocalFrameB( b3JointId jointId, b3Transform localFrame );
extern "C" b3Transform b3Joint_GetLocalFrameB( b3JointId jointId );
extern "C" void b3Joint_SetCollideConnected( b3JointId jointId, bool shouldCollide );
extern "C" bool b3Joint_GetCollideConnected( b3JointId jointId );
extern "C" void b3Joint_SetUserData( b3JointId jointId, void* userData );
extern "C" void* b3Joint_GetUserData( b3JointId jointId );
extern "C" void b3Joint_WakeBodies( b3JointId jointId );
extern "C" b3Vec3 b3Joint_GetConstraintForce( b3JointId jointId );
extern "C" b3Vec3 b3Joint_GetConstraintTorque( b3JointId jointId );
extern "C" float b3Joint_GetLinearSeparation( b3JointId jointId );
extern "C" float b3Joint_GetAngularSeparation( b3JointId jointId );
extern "C" void b3Joint_SetConstraintTuning( b3JointId jointId, float hertz, float dampingRatio );
extern "C" void b3Joint_GetConstraintTuning( b3JointId jointId, float* hertz, float* dampingRatio );
extern "C" void b3Joint_SetForceThreshold( b3JointId jointId, float threshold );
extern "C" float b3Joint_GetForceThreshold( b3JointId jointId );
extern "C" void b3Joint_SetTorqueThreshold( b3JointId jointId, float threshold );
extern "C" float b3Joint_GetTorqueThreshold( b3JointId jointId );
extern "C" b3JointId b3CreateParallelJoint( b3WorldId worldId, const b3ParallelJointDef* def );
extern "C" void b3ParallelJoint_SetSpringHertz( b3JointId jointId, float hertz );
extern "C" void b3ParallelJoint_SetSpringDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3ParallelJoint_GetSpringHertz( b3JointId jointId );
extern "C" float b3ParallelJoint_GetSpringDampingRatio( b3JointId jointId );
extern "C" void b3ParallelJoint_SetMaxTorque( b3JointId jointId, float force );
extern "C" float b3ParallelJoint_GetMaxTorque( b3JointId jointId );
extern "C" b3JointId b3CreateDistanceJoint( b3WorldId worldId, const b3DistanceJointDef* def );
extern "C" void b3DistanceJoint_SetLength( b3JointId jointId, float length );
extern "C" float b3DistanceJoint_GetLength( b3JointId jointId );
extern "C" void b3DistanceJoint_EnableSpring( b3JointId jointId, bool enableSpring );
extern "C" bool b3DistanceJoint_IsSpringEnabled( b3JointId jointId );
extern "C" void b3DistanceJoint_SetSpringForceRange( b3JointId jointId, float lowerForce, float upperForce );
extern "C" void b3DistanceJoint_GetSpringForceRange( b3JointId jointId, float* lowerForce, float* upperForce );
extern "C" void b3DistanceJoint_SetSpringHertz( b3JointId jointId, float hertz );
extern "C" void b3DistanceJoint_SetSpringDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3DistanceJoint_GetSpringHertz( b3JointId jointId );
extern "C" float b3DistanceJoint_GetSpringDampingRatio( b3JointId jointId );
extern "C" void b3DistanceJoint_EnableLimit( b3JointId jointId, bool enableLimit );
extern "C" bool b3DistanceJoint_IsLimitEnabled( b3JointId jointId );
extern "C" void b3DistanceJoint_SetLengthRange( b3JointId jointId, float minLength, float maxLength );
extern "C" float b3DistanceJoint_GetMinLength( b3JointId jointId );
extern "C" float b3DistanceJoint_GetMaxLength( b3JointId jointId );
extern "C" float b3DistanceJoint_GetCurrentLength( b3JointId jointId );
extern "C" void b3DistanceJoint_EnableMotor( b3JointId jointId, bool enableMotor );
extern "C" bool b3DistanceJoint_IsMotorEnabled( b3JointId jointId );
extern "C" void b3DistanceJoint_SetMotorSpeed( b3JointId jointId, float motorSpeed );
extern "C" float b3DistanceJoint_GetMotorSpeed( b3JointId jointId );
extern "C" void b3DistanceJoint_SetMaxMotorForce( b3JointId jointId, float force );
extern "C" float b3DistanceJoint_GetMaxMotorForce( b3JointId jointId );
extern "C" float b3DistanceJoint_GetMotorForce( b3JointId jointId );
extern "C" b3JointId b3CreateMotorJoint( b3WorldId worldId, const b3MotorJointDef* def );
extern "C" void b3MotorJoint_SetLinearVelocity( b3JointId jointId, b3Vec3 velocity );
extern "C" b3Vec3 b3MotorJoint_GetLinearVelocity( b3JointId jointId );
extern "C" void b3MotorJoint_SetAngularVelocity( b3JointId jointId, b3Vec3 velocity );
extern "C" b3Vec3 b3MotorJoint_GetAngularVelocity( b3JointId jointId );
extern "C" void b3MotorJoint_SetMaxVelocityForce( b3JointId jointId, float maxForce );
extern "C" float b3MotorJoint_GetMaxVelocityForce( b3JointId jointId );
extern "C" void b3MotorJoint_SetMaxVelocityTorque( b3JointId jointId, float maxTorque );
extern "C" float b3MotorJoint_GetMaxVelocityTorque( b3JointId jointId );
extern "C" void b3MotorJoint_SetLinearHertz( b3JointId jointId, float hertz );
extern "C" float b3MotorJoint_GetLinearHertz( b3JointId jointId );
extern "C" void b3MotorJoint_SetLinearDampingRatio( b3JointId jointId, float damping );
extern "C" float b3MotorJoint_GetLinearDampingRatio( b3JointId jointId );
extern "C" void b3MotorJoint_SetAngularHertz( b3JointId jointId, float hertz );
extern "C" float b3MotorJoint_GetAngularHertz( b3JointId jointId );
extern "C" void b3MotorJoint_SetAngularDampingRatio( b3JointId jointId, float damping );
extern "C" float b3MotorJoint_GetAngularDampingRatio( b3JointId jointId );
extern "C" void b3MotorJoint_SetMaxSpringForce( b3JointId jointId, float maxForce );
extern "C" float b3MotorJoint_GetMaxSpringForce( b3JointId jointId );
extern "C" void b3MotorJoint_SetMaxSpringTorque( b3JointId jointId, float maxTorque );
extern "C" float b3MotorJoint_GetMaxSpringTorque( b3JointId jointId );
extern "C" b3JointId b3CreateFilterJoint( b3WorldId worldId, const b3FilterJointDef* def );
extern "C" b3JointId b3CreatePrismaticJoint( b3WorldId worldId, const b3PrismaticJointDef* def );
extern "C" void b3PrismaticJoint_EnableSpring( b3JointId jointId, bool enableSpring );
extern "C" bool b3PrismaticJoint_IsSpringEnabled( b3JointId jointId );
extern "C" void b3PrismaticJoint_SetSpringHertz( b3JointId jointId, float hertz );
extern "C" float b3PrismaticJoint_GetSpringHertz( b3JointId jointId );
extern "C" void b3PrismaticJoint_SetSpringDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3PrismaticJoint_GetSpringDampingRatio( b3JointId jointId );
extern "C" void b3PrismaticJoint_SetTargetTranslation( b3JointId jointId, float targetTranslation );
extern "C" float b3PrismaticJoint_GetTargetTranslation( b3JointId jointId );
extern "C" void b3PrismaticJoint_EnableLimit( b3JointId jointId, bool enableLimit );
extern "C" bool b3PrismaticJoint_IsLimitEnabled( b3JointId jointId );
extern "C" float b3PrismaticJoint_GetLowerLimit( b3JointId jointId );
extern "C" float b3PrismaticJoint_GetUpperLimit( b3JointId jointId );
extern "C" void b3PrismaticJoint_SetLimits( b3JointId jointId, float lower, float upper );
extern "C" void b3PrismaticJoint_EnableMotor( b3JointId jointId, bool enableMotor );
extern "C" bool b3PrismaticJoint_IsMotorEnabled( b3JointId jointId );
extern "C" void b3PrismaticJoint_SetMotorSpeed( b3JointId jointId, float motorSpeed );
extern "C" float b3PrismaticJoint_GetMotorSpeed( b3JointId jointId );
extern "C" void b3PrismaticJoint_SetMaxMotorForce( b3JointId jointId, float force );
extern "C" float b3PrismaticJoint_GetMaxMotorForce( b3JointId jointId );
extern "C" float b3PrismaticJoint_GetMotorForce( b3JointId jointId );
extern "C" float b3PrismaticJoint_GetTranslation( b3JointId jointId );
extern "C" float b3PrismaticJoint_GetSpeed( b3JointId jointId );
extern "C" b3JointId b3CreateRevoluteJoint( b3WorldId worldId, const b3RevoluteJointDef* def );
extern "C" void b3RevoluteJoint_EnableSpring( b3JointId jointId, bool enableSpring );
extern "C" bool b3RevoluteJoint_IsSpringEnabled( b3JointId jointId );
extern "C" void b3RevoluteJoint_SetSpringHertz( b3JointId jointId, float hertz );
extern "C" float b3RevoluteJoint_GetSpringHertz( b3JointId jointId );
extern "C" void b3RevoluteJoint_SetSpringDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3RevoluteJoint_GetSpringDampingRatio( b3JointId jointId );
extern "C" void b3RevoluteJoint_SetTargetAngle( b3JointId jointId, float targetRadians );
extern "C" float b3RevoluteJoint_GetTargetAngle( b3JointId jointId );
extern "C" float b3RevoluteJoint_GetAngle( b3JointId jointId );
extern "C" void b3RevoluteJoint_EnableLimit( b3JointId jointId, bool enableLimit );
extern "C" bool b3RevoluteJoint_IsLimitEnabled( b3JointId jointId );
extern "C" float b3RevoluteJoint_GetLowerLimit( b3JointId jointId );
extern "C" float b3RevoluteJoint_GetUpperLimit( b3JointId jointId );
extern "C" void b3RevoluteJoint_SetLimits( b3JointId jointId, float lowerLimitRadians, float upperLimitRadians );
extern "C" void b3RevoluteJoint_EnableMotor( b3JointId jointId, bool enableMotor );
extern "C" bool b3RevoluteJoint_IsMotorEnabled( b3JointId jointId );
extern "C" void b3RevoluteJoint_SetMotorSpeed( b3JointId jointId, float motorSpeed );
extern "C" float b3RevoluteJoint_GetMotorSpeed( b3JointId jointId );
extern "C" float b3RevoluteJoint_GetMotorTorque( b3JointId jointId );
extern "C" void b3RevoluteJoint_SetMaxMotorTorque( b3JointId jointId, float torque );
extern "C" float b3RevoluteJoint_GetMaxMotorTorque( b3JointId jointId );
extern "C" b3JointId b3CreateSphericalJoint( b3WorldId worldId, const b3SphericalJointDef* def );
extern "C" void b3SphericalJoint_EnableConeLimit( b3JointId jointId, bool enableLimit );
extern "C" bool b3SphericalJoint_IsConeLimitEnabled( b3JointId jointId );
extern "C" float b3SphericalJoint_GetConeLimit( b3JointId jointId );
extern "C" void b3SphericalJoint_SetConeLimit( b3JointId jointId, float angleRadians );
extern "C" float b3SphericalJoint_GetConeAngle( b3JointId jointId );
extern "C" void b3SphericalJoint_EnableTwistLimit( b3JointId jointId, bool enableLimit );
extern "C" bool b3SphericalJoint_IsTwistLimitEnabled( b3JointId jointId );
extern "C" float b3SphericalJoint_GetLowerTwistLimit( b3JointId jointId );
extern "C" float b3SphericalJoint_GetUpperTwistLimit( b3JointId jointId );
extern "C" void b3SphericalJoint_SetTwistLimits( b3JointId jointId, float lowerLimitRadians, float upperLimitRadians );
extern "C" float b3SphericalJoint_GetTwistAngle( b3JointId jointId );
extern "C" void b3SphericalJoint_EnableSpring( b3JointId jointId, bool enableSpring );
extern "C" bool b3SphericalJoint_IsSpringEnabled( b3JointId jointId );
extern "C" void b3SphericalJoint_SetSpringHertz( b3JointId jointId, float hertz );
extern "C" float b3SphericalJoint_GetSpringHertz( b3JointId jointId );
extern "C" void b3SphericalJoint_SetSpringDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3SphericalJoint_GetSpringDampingRatio( b3JointId jointId );
extern "C" void b3SphericalJoint_SetTargetRotation( b3JointId jointId, b3Quat targetRotation );
extern "C" b3Quat b3SphericalJoint_GetTargetRotation( b3JointId jointId );
extern "C" void b3SphericalJoint_EnableMotor( b3JointId jointId, bool enableMotor );
extern "C" bool b3SphericalJoint_IsMotorEnabled( b3JointId jointId );
extern "C" void b3SphericalJoint_SetMotorVelocity( b3JointId jointId, b3Vec3 motorVelocity );
extern "C" b3Vec3 b3SphericalJoint_GetMotorVelocity( b3JointId jointId );
extern "C" b3Vec3 b3SphericalJoint_GetMotorTorque( b3JointId jointId );
extern "C" void b3SphericalJoint_SetMaxMotorTorque( b3JointId jointId, float torque );
extern "C" float b3SphericalJoint_GetMaxMotorTorque( b3JointId jointId );
extern "C" b3JointId b3CreateWeldJoint( b3WorldId worldId, const b3WeldJointDef* def );
extern "C" void b3WeldJoint_SetLinearHertz( b3JointId jointId, float hertz );
extern "C" float b3WeldJoint_GetLinearHertz( b3JointId jointId );
extern "C" void b3WeldJoint_SetLinearDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3WeldJoint_GetLinearDampingRatio( b3JointId jointId );
extern "C" void b3WeldJoint_SetAngularHertz( b3JointId jointId, float hertz );
extern "C" float b3WeldJoint_GetAngularHertz( b3JointId jointId );
extern "C" void b3WeldJoint_SetAngularDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3WeldJoint_GetAngularDampingRatio( b3JointId jointId );
extern "C" b3JointId b3CreateWheelJoint( b3WorldId worldId, const b3WheelJointDef* def );
extern "C" void b3WheelJoint_EnableSuspension( b3JointId jointId, bool flag );
extern "C" bool b3WheelJoint_IsSuspensionEnabled( b3JointId jointId );
extern "C" void b3WheelJoint_SetSuspensionHertz( b3JointId jointId, float hertz );
extern "C" float b3WheelJoint_GetSuspensionHertz( b3JointId jointId );
extern "C" void b3WheelJoint_SetSuspensionDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3WheelJoint_GetSuspensionDampingRatio( b3JointId jointId );
extern "C" void b3WheelJoint_EnableSuspensionLimit( b3JointId jointId, bool flag );
extern "C" bool b3WheelJoint_IsSuspensionLimitEnabled( b3JointId jointId );
extern "C" float b3WheelJoint_GetLowerSuspensionLimit( b3JointId jointId );
extern "C" float b3WheelJoint_GetUpperSuspensionLimit( b3JointId jointId );
extern "C" void b3WheelJoint_SetSuspensionLimits( b3JointId jointId, float lower, float upper );
extern "C" void b3WheelJoint_EnableSpinMotor( b3JointId jointId, bool flag );
extern "C" bool b3WheelJoint_IsSpinMotorEnabled( b3JointId jointId );
extern "C" void b3WheelJoint_SetSpinMotorSpeed( b3JointId jointId, float speed );
extern "C" float b3WheelJoint_GetSpinMotorSpeed( b3JointId jointId );
extern "C" void b3WheelJoint_SetMaxSpinTorque( b3JointId jointId, float torque );
extern "C" float b3WheelJoint_GetMaxSpinTorque( b3JointId jointId );
extern "C" float b3WheelJoint_GetSpinSpeed( b3JointId jointId );
extern "C" float b3WheelJoint_GetSpinTorque( b3JointId jointId );
extern "C" void b3WheelJoint_EnableSteering( b3JointId jointId, bool flag );
extern "C" bool b3WheelJoint_IsSteeringEnabled( b3JointId jointId );
extern "C" void b3WheelJoint_SetSteeringHertz( b3JointId jointId, float hertz );
extern "C" float b3WheelJoint_GetSteeringHertz( b3JointId jointId );
extern "C" void b3WheelJoint_SetSteeringDampingRatio( b3JointId jointId, float dampingRatio );
extern "C" float b3WheelJoint_GetSteeringDampingRatio( b3JointId jointId );
extern "C" void b3WheelJoint_SetMaxSteeringTorque( b3JointId jointId, float torque );
extern "C" float b3WheelJoint_GetMaxSteeringTorque( b3JointId jointId );
extern "C" void b3WheelJoint_EnableSteeringLimit( b3JointId jointId, bool flag );
extern "C" bool b3WheelJoint_IsSteeringLimitEnabled( b3JointId jointId );
extern "C" float b3WheelJoint_GetLowerSteeringLimit( b3JointId jointId );
extern "C" float b3WheelJoint_GetUpperSteeringLimit( b3JointId jointId );
extern "C" void b3WheelJoint_SetSteeringLimits( b3JointId jointId, float lowerRadians, float upperRadians );
extern "C" void b3WheelJoint_SetTargetSteeringAngle( b3JointId jointId, float radians );
extern "C" float b3WheelJoint_GetTargetSteeringAngle( b3JointId jointId );
extern "C" float b3WheelJoint_GetSteeringAngle( b3JointId jointId );
extern "C" float b3WheelJoint_GetSteeringTorque( b3JointId jointId );
extern "C" bool b3Contact_IsValid( b3ContactId id );
extern "C" b3ContactData b3Contact_GetData( b3ContactId contactId );
extern "C" {
extern void __assert_fail (const char *__assertion, const char *__file,
      unsigned int __line, const char *__function)
     noexcept (true) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));
extern void __assert_perror_fail (int __errnum, const char *__file,
      unsigned int __line, const char *__function)
     noexcept (true) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));
extern void __assert (const char *__assertion, const char *__file, int __line)
     noexcept (true) __attribute__ ((__noreturn__)) __attribute__ ((__cold__));
}
namespace std
{
  using ::int8_t;
  using ::int16_t;
  using ::int32_t;
  using ::int64_t;
  using ::int_fast8_t;
  using ::int_fast16_t;
  using ::int_fast32_t;
  using ::int_fast64_t;
  using ::int_least8_t;
  using ::int_least16_t;
  using ::int_least32_t;
  using ::int_least64_t;
  using ::intmax_t;
  using ::intptr_t;
  using ::uint8_t;
  using ::uint16_t;
  using ::uint32_t;
  using ::uint64_t;
  using ::uint_fast8_t;
  using ::uint_fast16_t;
  using ::uint_fast32_t;
  using ::uint_fast64_t;
  using ::uint_least8_t;
  using ::uint_least16_t;
  using ::uint_least32_t;
  using ::uint_least64_t;
  using ::uintmax_t;
  using ::uintptr_t;
}
extern "C" {
typedef __builtin_va_list __gnuc_va_list;
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
struct _IO_FILE;
typedef struct _IO_FILE __FILE;
struct _IO_FILE;
typedef struct _IO_FILE FILE;
struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;
typedef void _IO_lock_t;
struct _IO_FILE
{
  int _flags;
  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;
  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;
  struct _IO_marker *_markers;
  struct _IO_FILE *_chain;
  int _fileno;
  int _flags2:24;
  char _short_backupbuf[1];
  __off_t _old_offset;
  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];
  _IO_lock_t *_lock;
  __off64_t _offset;
  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  struct _IO_FILE **_prevchain;
  int _mode;
  int _unused3;
  __uint64_t _total_written;
  char _unused2[12 * sizeof (int) - 5 * sizeof (void *)];
};
typedef __ssize_t cookie_read_function_t (void *__cookie, char *__buf,
                                          size_t __nbytes);
typedef __ssize_t cookie_write_function_t (void *__cookie, const char *__buf,
                                           size_t __nbytes);
typedef int cookie_seek_function_t (void *__cookie, __off64_t *__pos, int __w);
typedef int cookie_close_function_t (void *__cookie);
typedef struct _IO_cookie_io_functions_t
{
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
typedef __gnuc_va_list va_list;
typedef __fpos_t fpos_t;
typedef __fpos64_t fpos64_t;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
extern int remove (const char *__filename) noexcept (true);
extern int rename (const char *__old, const char *__new) noexcept (true);
extern int renameat (int __oldfd, const char *__old, int __newfd,
       const char *__new) noexcept (true);
extern int renameat2 (int __oldfd, const char *__old, int __newfd,
        const char *__new, unsigned int __flags) noexcept (true);
extern int fclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern FILE *tmpfile (void)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern FILE *tmpfile64 (void)
   __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern char *tmpnam (char[20]) noexcept (true) ;
extern char *tmpnam_r (char __s[20]) noexcept (true) ;
extern char *tempnam (const char *__dir, const char *__pfx)
   noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (__builtin_free, 1)));
extern int fflush (FILE *__stream);
extern int fflush_unlocked (FILE *__stream);
extern int fcloseall (void);
extern FILE *fopen (const char *__restrict __filename,
      const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern FILE *freopen (const char *__restrict __filename,
        const char *__restrict __modes,
        FILE *__restrict __stream) __attribute__ ((__nonnull__ (3)));
extern FILE *fopen64 (const char *__restrict __filename,
        const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern FILE *freopen64 (const char *__restrict __filename,
   const char *__restrict __modes,
   FILE *__restrict __stream) __attribute__ ((__nonnull__ (3)));
extern FILE *fdopen (int __fd, const char *__modes) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern FILE *fopencookie (void *__restrict __magic_cookie,
     const char *__restrict __modes,
     cookie_io_functions_t __io_funcs) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
  noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) noexcept (true)
  __attribute__ ((__nonnull__ (1)));
extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void setlinebuf (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...) __attribute__ ((__nonnull__ (1)));
extern int printf (const char *__restrict __format, ...);
extern int sprintf (char *__restrict __s,
      const char *__restrict __format, ...) noexcept (true);
extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nonnull__ (1)));
extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);
extern int vsprintf (char *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) noexcept (true);
extern int snprintf (char *__restrict __s, size_t __maxlen,
       const char *__restrict __format, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 4)));
extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        const char *__restrict __format, __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 0)));
extern int vasprintf (char **__restrict __ptr, const char *__restrict __f,
        __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 0))) ;
extern int __asprintf (char **__restrict __ptr,
         const char *__restrict __fmt, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3))) ;
extern int asprintf (char **__restrict __ptr,
       const char *__restrict __fmt, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3))) ;
extern int vdprintf (int __fd, const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));
extern int fscanf (FILE *__restrict __stream,
     const char *__restrict __format, ...) __attribute__ ((__nonnull__ (1)));
extern int scanf (const char *__restrict __format, ...) ;
extern int sscanf (const char *__restrict __s,
     const char *__restrict __format, ...) noexcept (true);
extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc23_fscanf") __attribute__ ((__nonnull__ (1)));
extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc23_scanf") ;
extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) noexcept (true) __asm__ ("" "__isoc23_sscanf");
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__nonnull__ (1)));
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) ;
extern int vsscanf (const char *__restrict __s,
      const char *__restrict __format, __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__scanf__, 2, 0)));
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc23_vfscanf")
     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__nonnull__ (1)));
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc23_vscanf")
     __attribute__ ((__format__ (__scanf__, 1, 0))) ;
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) noexcept (true) __asm__ ("" "__isoc23_vsscanf")
     __attribute__ ((__format__ (__scanf__, 2, 0)));
extern int fgetc (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getc (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getchar (void);
extern int getc_unlocked (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getchar_unlocked (void);
extern int fgetc_unlocked (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int fputc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putchar (int __c);
extern int fputc_unlocked (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putc_unlocked (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putchar_unlocked (int __c);
extern int getw (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int putw (int __w, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     __attribute__ ((__access__ (__write_only__, 1, 2))) __attribute__ ((__nonnull__ (3)));
extern char *fgets_unlocked (char *__restrict __s, int __n,
        FILE *__restrict __stream)
    __attribute__ ((__access__ (__write_only__, 1, 2))) __attribute__ ((__nonnull__ (3)));
extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream) __attribute__ ((__nonnull__ (4)));
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream) __attribute__ ((__nonnull__ (4)));
extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) __attribute__ ((__nonnull__ (3)));
extern int fputs (const char *__restrict __s, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (2)));
extern int puts (const char *__s);
extern int ungetc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (4)));
extern size_t fwrite (const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) __attribute__ ((__nonnull__ (4)));
extern int fputs_unlocked (const char *__restrict __s,
      FILE *__restrict __stream) __attribute__ ((__nonnull__ (2)));
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (4)));
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (4)));
extern int fseek (FILE *__stream, long int __off, int __whence)
  __attribute__ ((__nonnull__ (1)));
extern long int ftell (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern void rewind (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int fseeko (FILE *__stream, __off_t __off, int __whence)
  __attribute__ ((__nonnull__ (1)));
extern __off_t ftello (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos)
  __attribute__ ((__nonnull__ (1)));
extern int fsetpos (FILE *__stream, const fpos_t *__pos) __attribute__ ((__nonnull__ (1)));
extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence)
  __attribute__ ((__nonnull__ (1)));
extern __off64_t ftello64 (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int fgetpos64 (FILE *__restrict __stream, fpos64_t *__restrict __pos)
  __attribute__ ((__nonnull__ (1)));
extern int fsetpos64 (FILE *__stream, const fpos64_t *__pos) __attribute__ ((__nonnull__ (1)));
extern void clearerr (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int feof (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int ferror (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void clearerr_unlocked (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int feof_unlocked (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int ferror_unlocked (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void perror (const char *__s) __attribute__ ((__cold__));
extern int fileno (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int fileno_unlocked (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int pclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern FILE *popen (const char *__command, const char *__modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (pclose, 1))) ;
extern char *ctermid (char *__s) noexcept (true)
  __attribute__ ((__access__ (__write_only__, 1)));
extern char *cuserid (char *__s)
  __attribute__ ((__access__ (__write_only__, 1)));
struct obstack;
extern int obstack_printf (struct obstack *__restrict __obstack,
      const char *__restrict __format, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3)));
extern int obstack_vprintf (struct obstack *__restrict __obstack,
       const char *__restrict __format,
       __gnuc_va_list __args)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 0)));
extern void flockfile (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int ftrylockfile (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void funlockfile (FILE *__stream) noexcept (true) __attribute__ ((__nonnull__ (1)));
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);
}
namespace std
{
  using ::FILE;
  using ::fpos_t;
  using ::clearerr;
  using ::fclose;
  using ::feof;
  using ::ferror;
  using ::fflush;
  using ::fgetc;
  using ::fgetpos;
  using ::fgets;
  using ::fopen;
  using ::fprintf;
  using ::fputc;
  using ::fputs;
  using ::fread;
  using ::freopen;
  using ::fscanf;
  using ::fseek;
  using ::fsetpos;
  using ::ftell;
  using ::fwrite;
  using ::getc;
  using ::getchar;
  using ::perror;
  using ::printf;
  using ::putc;
  using ::putchar;
  using ::puts;
  using ::remove;
  using ::rename;
  using ::rewind;
  using ::scanf;
  using ::setbuf;
  using ::setvbuf;
  using ::sprintf;
  using ::sscanf;
  using ::tmpfile;
  using ::tmpnam;
  using ::ungetc;
  using ::vfprintf;
  using ::vprintf;
  using ::vsprintf;
}
namespace __gnu_cxx
{
  using ::snprintf;
  using ::vfscanf;
  using ::vscanf;
  using ::vsnprintf;
  using ::vsscanf;
}
namespace std
{
  using ::__gnu_cxx::snprintf;
  using ::__gnu_cxx::vfscanf;
  using ::__gnu_cxx::vscanf;
  using ::__gnu_cxx::vsnprintf;
  using ::__gnu_cxx::vsscanf;
}
extern "C" { int gpu_shape_geometry_mirror_count(void); }
extern "C" { void SetSelectedBody(b3BodyId){} void SetComparisonSelectedBody(b3BodyId){} }
static int observations=0,mismatches=0;
static void check(const char* phase,const char* field,uint64_t got,uint64_t expected) {
    bool match=got==expected;
    std::printf("{\"phase\":\"%s\",\"field\":\"%s\",\"got\":%llu,\"expected\":%llu,\"match\":%s}\n",phase,field,(unsigned long long)got,(unsigned long long)expected,match?"true":"false");
    ++observations;if(!match)++mismatches;
}
static b3ShapeId make_compound(b3WorldId world,const b3CompoundData* compound) {
    auto bd=b3DefaultBodyDef();auto body=b3CreateBody(world,&bd);
    auto sd=b3DefaultShapeDef();sd.userData=(void*)uintptr_t(222);
    return b3CreateBakedCompoundShape(body,&sd,compound);
}
int main() {
    auto box=b3MakeBoxHull(.5f,.5f,.5f);b3CompoundHullDef hulls[2]{};
    for(int i=0;i<2;++i){hulls[i].hull=&box.base;hulls[i].material=b3DefaultSurfaceMaterial();hulls[i].transform={{float(i)*2.f,0,0},b3Quat_identity};}
    b3CompoundDef cd{};cd.hulls=hulls;cd.hullCount=2;auto* compound=b3CreateCompound(&cd);((compound) ? void (1 ? 1 : bool (compound)) : __assert_fail ("compound", __builtin_FILE (), __builtin_LINE (), __extension__ __PRETTY_FUNCTION__));
    auto wd=b3DefaultWorldDef();auto old=b3CreateWorld(&wd);auto stale=make_compound(old,compound);
    b3DestroyWorld(old);auto fresh=b3CreateWorld(&wd);auto live=make_compound(fresh,compound);
    auto other=b3CreateWorld(&wd);auto isolated=make_compound(other,compound);
    check("recreate","distinct-world-id",b3StoreWorldId(old)!=b3StoreWorldId(fresh),true);
    check("recreate","old-invalid",b3World_IsValid(old),false);
    check("recreate","stale-shape-invalid",b3Shape_IsValid(stale),false);
    int mirrors=gpu_shape_geometry_mirror_count();check("recreate","four-owned-hull-mirrors",mirrors,4);
    b3DestroyWorld(old);
    check("stale-world","owned-geometry-preserved",gpu_shape_geometry_mirror_count(),mirrors);
    auto forged=fresh;forged.generation=uint16_t(forged.generation+1);
    b3DestroyWorld(forged);
    check("forged-world","owned-geometry-preserved",gpu_shape_geometry_mirror_count(),mirrors);
    b3DestroyShape(stale,false);
    check("stale-parent","owned-child-geometry-preserved",gpu_shape_geometry_mirror_count(),mirrors);
    auto wrong=live;wrong.generation=uint16_t(wrong.generation+1);b3DestroyShape(wrong,false);
    check("forged-parent","owned-child-geometry-preserved",gpu_shape_geometry_mirror_count(),mirrors);
    check("after-invalid","live-world-valid",b3World_IsValid(fresh),true);
    check("after-invalid","live-shape-valid",b3Shape_IsValid(live),true);
    check("after-invalid","live-user-data",uintptr_t(b3Shape_GetUserData(live)),222);
    check("after-invalid","other-shape-valid",b3Shape_IsValid(isolated),true);
    check("after-invalid","other-user-data",uintptr_t(b3Shape_GetUserData(isolated)),222);
    b3DestroyShape(live,false);check("valid-destroy","live-shape-invalid",b3Shape_IsValid(live),false);
    check("valid-destroy","isolated-mirrors-only",gpu_shape_geometry_mirror_count(),2);
    check("valid-destroy","other-world-valid",b3World_IsValid(other),true);
    b3DestroyWorld(fresh);b3DestroyWorld(other);b3DestroyCompound(compound);
    check("cleanup","no-owned-mirrors",gpu_shape_geometry_mirror_count(),0);
    std::printf("{\"summary\":true,\"observations\":%d,\"mismatches\":%d}\n",observations,mismatches);
    return mismatches?1:0;
}
