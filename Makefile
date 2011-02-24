all: rippled tests/ripple_tests

nuke: clean
	rm ripple.db

clean:
	rm *.o rippled tests/ripple_tests tests/ripple.db

rippled: rippled.cpp Ripple.o RippleInterface.o ripple.db mongoose.o restcomet.o
	g++ -g -o rippled rippled.cpp Ripple.o RippleInterface.o mongoose.o restcomet.o -lsoci_sqlite3 -lsoci_core -lpthread -lboost_regex -lboost_thread

tests/ripple_tests: tests/ripple_tests.cpp Ripple.o tests/ripple.db restcomet.o
	g++ -g -o tests/ripple_tests tests/ripple_tests.cpp Ripple.o restcomet.o -lsoci_sqlite3 -lsoci_core -lboost_unit_test_framework -lboost_regex -lboost_thread

Ripple.o: Ripple.cpp Ripple.hpp RippleTask.hpp RippleUser.hpp RippleLog.hpp
	g++ -g -c Ripple.cpp -I/usr/local/include/soci 

RippleInterface.o: RippleInterface.hpp RippleInterface.cpp 
	g++ -g -c RippleInterface.cpp 

restcomet.o: restcomet/restcomet.cpp
	g++ -g -c restcomet/restcomet.cpp

mongoose.o: mongoose/mongoose.c
	gcc -g -c mongoose/mongoose.c

ripple.db: ripple_schema.sql
	sqlite3 ripple.db < ripple_schema.sql

tests/ripple.db: ripple_schema.sql
	sqlite3 tests/ripple.db < ripple_schema.sql

# DO NOT DELETE

Ripple.o: Ripple.hpp RippleUser.hpp RippleException.hpp RippleTask.hpp
Ripple.o: RippleDefines.hpp RippleLog.hpp /usr/include/time.h
Ripple.o: /usr/include/features.h /usr/include/bits/predefs.h
Ripple.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
Ripple.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
Ripple.o: /usr/include/bits/time.h /usr/include/bits/types.h
Ripple.o: /usr/include/bits/typesizes.h /usr/include/xlocale.h
Ripple.o: restcomet/restcomet.h /usr/include/boost/thread.hpp
Ripple.o: /usr/include/boost/thread/thread.hpp
Ripple.o: /usr/include/boost/thread/detail/platform.hpp
Ripple.o: /usr/include/boost/config.hpp
Ripple.o: /usr/include/boost/config/select_compiler_config.hpp
Ripple.o: /usr/include/boost/config/compiler/gcc.hpp
Ripple.o: /usr/include/boost/config/select_stdlib_config.hpp
Ripple.o: /usr/include/boost/config/no_tr1/utility.hpp
Ripple.o: /usr/include/boost/config/select_platform_config.hpp
Ripple.o: /usr/include/boost/config/platform/linux.hpp
Ripple.o: /usr/include/boost/config/posix_features.hpp /usr/include/unistd.h
Ripple.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
Ripple.o: /usr/include/bits/confname.h /usr/include/getopt.h
Ripple.o: /usr/include/boost/config/suffix.hpp
Ripple.o: /usr/include/boost/config/requires_threads.hpp
Ripple.o: /usr/include/boost/thread/detail/thread.hpp
Ripple.o: /usr/include/boost/thread/exceptions.hpp
Ripple.o: /usr/include/boost/thread/detail/config.hpp
Ripple.o: /usr/include/boost/config.hpp
Ripple.o: /usr/include/boost/detail/workaround.hpp
Ripple.o: /usr/include/boost/thread/detail/platform.hpp
Ripple.o: /usr/include/boost/config/auto_link.hpp
Ripple.o: /usr/include/boost/config/abi_prefix.hpp
Ripple.o: /usr/include/boost/config/abi_suffix.hpp
Ripple.o: /usr/include/boost/thread/detail/move.hpp
Ripple.o: /usr/include/boost/utility/enable_if.hpp
Ripple.o: /usr/include/boost/type_traits/is_convertible.hpp
Ripple.o: /usr/include/boost/type_traits/intrinsics.hpp
Ripple.o: /usr/include/boost/type_traits/config.hpp
Ripple.o: /usr/include/boost/type_traits/detail/yes_no_type.hpp
Ripple.o: /usr/include/boost/type_traits/is_array.hpp
Ripple.o: /usr/include/boost/type_traits/detail/bool_trait_def.hpp
Ripple.o: /usr/include/boost/type_traits/detail/template_arity_spec.hpp
Ripple.o: /usr/include/boost/mpl/int.hpp /usr/include/boost/mpl/int_fwd.hpp
Ripple.o: /usr/include/boost/mpl/aux_/adl_barrier.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/adl.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/msvc.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/intel.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/gcc.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/workaround.hpp
Ripple.o: /usr/include/boost/mpl/aux_/nttp_decl.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/nttp.hpp
Ripple.o: /usr/include/boost/preprocessor/cat.hpp
Ripple.o: /usr/include/boost/preprocessor/config/config.hpp
Ripple.o: /usr/include/boost/mpl/aux_/integral_wrapper.hpp
Ripple.o: /usr/include/boost/mpl/integral_c_tag.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/static_constant.hpp
Ripple.o: /usr/include/boost/mpl/aux_/static_cast.hpp
Ripple.o: /usr/include/boost/mpl/aux_/template_arity_fwd.hpp
Ripple.o: /usr/include/boost/mpl/aux_/preprocessor/params.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/preprocessor.hpp
Ripple.o: /usr/include/boost/preprocessor/comma_if.hpp
Ripple.o: /usr/include/boost/preprocessor/punctuation/comma_if.hpp
Ripple.o: /usr/include/boost/preprocessor/control/if.hpp
Ripple.o: /usr/include/boost/preprocessor/control/iif.hpp
Ripple.o: /usr/include/boost/preprocessor/logical/bool.hpp
Ripple.o: /usr/include/boost/preprocessor/facilities/empty.hpp
Ripple.o: /usr/include/boost/preprocessor/punctuation/comma.hpp
Ripple.o: /usr/include/boost/preprocessor/repeat.hpp
Ripple.o: /usr/include/boost/preprocessor/repetition/repeat.hpp
Ripple.o: /usr/include/boost/preprocessor/debug/error.hpp
Ripple.o: /usr/include/boost/preprocessor/detail/auto_rec.hpp
Ripple.o: /usr/include/boost/preprocessor/tuple/eat.hpp
Ripple.o: /usr/include/boost/preprocessor/inc.hpp
Ripple.o: /usr/include/boost/preprocessor/arithmetic/inc.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/lambda.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/ttp.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/ctps.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/overload_resolution.hpp
Ripple.o: /usr/include/boost/type_traits/integral_constant.hpp
Ripple.o: /usr/include/boost/mpl/bool.hpp /usr/include/boost/mpl/bool_fwd.hpp
Ripple.o: /usr/include/boost/mpl/integral_c.hpp
Ripple.o: /usr/include/boost/mpl/integral_c_fwd.hpp
Ripple.o: /usr/include/boost/mpl/aux_/lambda_support.hpp
Ripple.o: /usr/include/boost/mpl/aux_/yes_no.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/arrays.hpp
Ripple.o: /usr/include/boost/mpl/aux_/na_fwd.hpp
Ripple.o: /usr/include/boost/mpl/aux_/preprocessor/enum.hpp
Ripple.o: /usr/include/boost/preprocessor/tuple/to_list.hpp
Ripple.o: /usr/include/boost/preprocessor/list/for_each_i.hpp
Ripple.o: /usr/include/boost/preprocessor/list/adt.hpp
Ripple.o: /usr/include/boost/preprocessor/detail/is_binary.hpp
Ripple.o: /usr/include/boost/preprocessor/detail/check.hpp
Ripple.o: /usr/include/boost/preprocessor/logical/compl.hpp
Ripple.o: /usr/include/boost/preprocessor/repetition/for.hpp
Ripple.o: /usr/include/boost/preprocessor/repetition/detail/for.hpp
Ripple.o: /usr/include/boost/preprocessor/control/expr_iif.hpp
Ripple.o: /usr/include/boost/preprocessor/tuple/elem.hpp
Ripple.o: /usr/include/boost/preprocessor/tuple/rem.hpp
Ripple.o: /usr/include/boost/type_traits/detail/bool_trait_undef.hpp
Ripple.o: /usr/include/boost/type_traits/add_reference.hpp
Ripple.o: /usr/include/boost/type_traits/is_reference.hpp
Ripple.o: /usr/include/boost/type_traits/detail/type_trait_def.hpp
Ripple.o: /usr/include/boost/type_traits/detail/type_trait_undef.hpp
Ripple.o: /usr/include/boost/type_traits/ice.hpp
Ripple.o: /usr/include/boost/type_traits/detail/ice_or.hpp
Ripple.o: /usr/include/boost/type_traits/detail/ice_and.hpp
Ripple.o: /usr/include/boost/type_traits/detail/ice_not.hpp
Ripple.o: /usr/include/boost/type_traits/detail/ice_eq.hpp
Ripple.o: /usr/include/boost/type_traits/is_arithmetic.hpp
Ripple.o: /usr/include/boost/type_traits/is_integral.hpp
Ripple.o: /usr/include/boost/type_traits/is_float.hpp
Ripple.o: /usr/include/boost/type_traits/is_void.hpp
Ripple.o: /usr/include/boost/type_traits/is_abstract.hpp
Ripple.o: /usr/include/boost/static_assert.hpp
Ripple.o: /usr/include/boost/type_traits/is_class.hpp
Ripple.o: /usr/include/boost/type_traits/is_union.hpp
Ripple.o: /usr/include/boost/type_traits/remove_cv.hpp
Ripple.o: /usr/include/boost/type_traits/broken_compiler_spec.hpp
Ripple.o: /usr/include/boost/type_traits/detail/cv_traits_impl.hpp
Ripple.o: /usr/include/boost/thread/mutex.hpp
Ripple.o: /usr/include/boost/thread/xtime.hpp /usr/include/boost/cstdint.hpp
Ripple.o: /usr/include/stdint.h /usr/include/bits/wchar.h
Ripple.o: /usr/include/boost/thread/thread_time.hpp
Ripple.o: /usr/include/boost/date_time/microsec_time_clock.hpp
Ripple.o: /usr/include/boost/shared_ptr.hpp
Ripple.o: /usr/include/boost/smart_ptr/shared_ptr.hpp
Ripple.o: /usr/include/boost/config/no_tr1/memory.hpp
Ripple.o: /usr/include/boost/assert.hpp /usr/include/assert.h
Ripple.o: /usr/include/boost/checked_delete.hpp
Ripple.o: /usr/include/boost/throw_exception.hpp
Ripple.o: /usr/include/boost/exception/detail/attribute_noreturn.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/shared_count.hpp
Ripple.o: /usr/include/boost/smart_ptr/bad_weak_ptr.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/sp_counted_base.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/sp_has_sync.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/sp_counted_base_gcc_x86.hpp
Ripple.o: /usr/include/boost/detail/sp_typeinfo.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/sp_counted_impl.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/sp_convertible.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/spinlock_pool.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/spinlock.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/spinlock_pt.hpp
Ripple.o: /usr/include/pthread.h /usr/include/endian.h
Ripple.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
Ripple.o: /usr/include/sched.h /usr/include/bits/sched.h
Ripple.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/setjmp.h
Ripple.o: /usr/include/boost/memory_order.hpp
Ripple.o: /usr/include/boost/smart_ptr/detail/operator_bool.hpp
Ripple.o: /usr/include/boost/date_time/compiler_config.hpp
Ripple.o: /usr/include/boost/date_time/locale_config.hpp
Ripple.o: /usr/include/boost/date_time/c_time.hpp /usr/include/sys/time.h
Ripple.o: /usr/include/sys/select.h /usr/include/bits/select.h
Ripple.o: /usr/include/bits/sigset.h
Ripple.o: /usr/include/boost/date_time/time_clock.hpp
Ripple.o: /usr/include/boost/date_time/filetime_functions.hpp
Ripple.o: /usr/include/boost/date_time/posix_time/posix_time_types.hpp
Ripple.o: /usr/include/boost/date_time/posix_time/ptime.hpp
Ripple.o: /usr/include/boost/date_time/posix_time/posix_time_system.hpp
Ripple.o: /usr/include/boost/date_time/posix_time/posix_time_config.hpp
Ripple.o: /usr/include/boost/limits.hpp
Ripple.o: /usr/include/boost/config/no_tr1/cmath.hpp
Ripple.o: /usr/include/boost/date_time/time_duration.hpp
Ripple.o: /usr/include/boost/operators.hpp /usr/include/boost/iterator.hpp
Ripple.o: /usr/include/boost/date_time/time_defs.hpp
Ripple.o: /usr/include/boost/date_time/special_defs.hpp
Ripple.o: /usr/include/boost/date_time/time_resolution_traits.hpp
Ripple.o: /usr/include/boost/date_time/int_adapter.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/gregorian_types.hpp
Ripple.o: /usr/include/boost/date_time/date.hpp
Ripple.o: /usr/include/boost/date_time/year_month_day.hpp
Ripple.o: /usr/include/boost/date_time/period.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_calendar.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_weekday.hpp
Ripple.o: /usr/include/boost/date_time/constrained_value.hpp
Ripple.o: /usr/include/boost/mpl/if.hpp
Ripple.o: /usr/include/boost/mpl/aux_/value_wknd.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/integral.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/eti.hpp
Ripple.o: /usr/include/boost/mpl/aux_/na_spec.hpp
Ripple.o: /usr/include/boost/mpl/lambda_fwd.hpp
Ripple.o: /usr/include/boost/mpl/void_fwd.hpp
Ripple.o: /usr/include/boost/mpl/aux_/na.hpp
Ripple.o: /usr/include/boost/mpl/aux_/arity.hpp
Ripple.o: /usr/include/boost/mpl/aux_/config/dtp.hpp
Ripple.o: /usr/include/boost/mpl/aux_/preprocessor/def_params_tail.hpp
Ripple.o: /usr/include/boost/mpl/limits/arity.hpp
Ripple.o: /usr/include/boost/preprocessor/logical/and.hpp
Ripple.o: /usr/include/boost/preprocessor/logical/bitand.hpp
Ripple.o: /usr/include/boost/preprocessor/identity.hpp
Ripple.o: /usr/include/boost/preprocessor/facilities/identity.hpp
Ripple.o: /usr/include/boost/preprocessor/empty.hpp
Ripple.o: /usr/include/boost/preprocessor/arithmetic/add.hpp
Ripple.o: /usr/include/boost/preprocessor/arithmetic/dec.hpp
Ripple.o: /usr/include/boost/preprocessor/control/while.hpp
Ripple.o: /usr/include/boost/preprocessor/list/fold_left.hpp
Ripple.o: /usr/include/boost/preprocessor/list/detail/fold_left.hpp
Ripple.o: /usr/include/boost/preprocessor/list/fold_right.hpp
Ripple.o: /usr/include/boost/preprocessor/list/detail/fold_right.hpp
Ripple.o: /usr/include/boost/preprocessor/list/reverse.hpp
Ripple.o: /usr/include/boost/preprocessor/control/detail/while.hpp
Ripple.o: /usr/include/boost/preprocessor/arithmetic/sub.hpp
Ripple.o: /usr/include/boost/mpl/aux_/lambda_arity_param.hpp
Ripple.o: /usr/include/boost/type_traits/is_base_of.hpp
Ripple.o: /usr/include/boost/type_traits/is_base_and_derived.hpp
Ripple.o: /usr/include/boost/type_traits/is_same.hpp
Ripple.o: /usr/include/boost/date_time/date_defs.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_day_of_year.hpp
Ripple.o: /usr/include/boost/date_time/gregorian_calendar.hpp
Ripple.o: /usr/include/boost/date_time/gregorian_calendar.ipp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_ymd.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_day.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_year.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_month.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_duration.hpp
Ripple.o: /usr/include/boost/date_time/date_duration.hpp
Ripple.o: /usr/include/boost/date_time/date_duration_types.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_duration_types.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/greg_date.hpp
Ripple.o: /usr/include/boost/date_time/adjust_functors.hpp
Ripple.o: /usr/include/boost/date_time/wrapping_int.hpp
Ripple.o: /usr/include/boost/date_time/date_generators.hpp
Ripple.o: /usr/include/boost/date_time/date_clock_device.hpp
Ripple.o: /usr/include/boost/date_time/date_iterator.hpp
Ripple.o: /usr/include/boost/date_time/time_system_split.hpp
Ripple.o: /usr/include/boost/date_time/time_system_counted.hpp
Ripple.o: /usr/include/boost/date_time/time.hpp
Ripple.o: /usr/include/boost/date_time/posix_time/date_duration_operators.hpp
Ripple.o: /usr/include/boost/date_time/posix_time/posix_time_duration.hpp
Ripple.o: /usr/include/boost/date_time/posix_time/time_period.hpp
Ripple.o: /usr/include/boost/date_time/time_iterator.hpp
Ripple.o: /usr/include/boost/date_time/dst_rules.hpp
Ripple.o: /usr/include/boost/date_time/posix_time/conversion.hpp
Ripple.o: /usr/include/boost/date_time/gregorian/conversion.hpp
Ripple.o: /usr/include/boost/thread/detail/thread_heap_alloc.hpp
Ripple.o: /usr/include/boost/utility.hpp
Ripple.o: /usr/include/boost/utility/addressof.hpp
Ripple.o: /usr/include/boost/utility/base_from_member.hpp
Ripple.o: /usr/include/boost/preprocessor/repetition/enum_binary_params.hpp
Ripple.o: /usr/include/boost/preprocessor/repetition/enum_params.hpp
Ripple.o: /usr/include/boost/preprocessor/repetition/repeat_from_to.hpp
Ripple.o: /usr/include/boost/utility/binary.hpp
Ripple.o: /usr/include/boost/preprocessor/control/deduce_d.hpp
Ripple.o: /usr/include/boost/preprocessor/seq/cat.hpp
Ripple.o: /usr/include/boost/preprocessor/seq/fold_left.hpp
Ripple.o: /usr/include/boost/preprocessor/seq/seq.hpp
Ripple.o: /usr/include/boost/preprocessor/seq/elem.hpp
Ripple.o: /usr/include/boost/preprocessor/seq/size.hpp
Ripple.o: /usr/include/boost/preprocessor/seq/transform.hpp
Ripple.o: /usr/include/boost/preprocessor/arithmetic/mod.hpp
Ripple.o: /usr/include/boost/preprocessor/arithmetic/detail/div_base.hpp
Ripple.o: /usr/include/boost/preprocessor/comparison/less_equal.hpp
Ripple.o: /usr/include/boost/preprocessor/logical/not.hpp
Ripple.o: /usr/include/boost/next_prior.hpp
Ripple.o: /usr/include/boost/noncopyable.hpp /usr/include/boost/ref.hpp
Ripple.o: /usr/include/boost/bind.hpp /usr/include/boost/bind/bind.hpp
Ripple.o: /usr/include/boost/mem_fn.hpp /usr/include/boost/bind/mem_fn.hpp
Ripple.o: /usr/include/boost/get_pointer.hpp
Ripple.o: /usr/include/boost/bind/mem_fn_template.hpp
Ripple.o: /usr/include/boost/bind/mem_fn_cc.hpp /usr/include/boost/type.hpp
Ripple.o: /usr/include/boost/is_placeholder.hpp
Ripple.o: /usr/include/boost/bind/arg.hpp /usr/include/boost/visit_each.hpp
Ripple.o: /usr/include/boost/bind/storage.hpp
Ripple.o: /usr/include/boost/bind/bind_template.hpp
Ripple.o: /usr/include/boost/bind/bind_cc.hpp
Ripple.o: /usr/include/boost/bind/bind_mf_cc.hpp
Ripple.o: /usr/include/boost/bind/bind_mf2_cc.hpp
Ripple.o: /usr/include/boost/bind/placeholders.hpp /usr/include/stdlib.h
Ripple.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
Ripple.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
Ripple.o: /usr/include/alloca.h
Ripple.o: /usr/include/boost/type_traits/remove_reference.hpp
Ripple.o: /usr/include/boost/thread/condition_variable.hpp
Ripple.o: /usr/include/boost/thread/once.hpp
Ripple.o: /usr/include/boost/thread/recursive_mutex.hpp
Ripple.o: /usr/include/boost/thread/tss.hpp
Ripple.o: /usr/include/boost/thread/locks.hpp
Ripple.o: /usr/include/boost/thread/shared_mutex.hpp
Ripple.o: /usr/include/boost/thread/barrier.hpp /usr/include/boost/regex.hpp
Ripple.o: /usr/include/boost/regex/config.hpp
Ripple.o: /usr/include/boost/regex/v4/regex.hpp JSON.hpp
rippled.o: Ripple.hpp RippleUser.hpp RippleException.hpp RippleTask.hpp
rippled.o: RippleDefines.hpp RippleLog.hpp /usr/include/time.h
rippled.o: /usr/include/features.h /usr/include/bits/predefs.h
rippled.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
rippled.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
rippled.o: /usr/include/bits/time.h /usr/include/bits/types.h
rippled.o: /usr/include/bits/typesizes.h /usr/include/xlocale.h
rippled.o: RippleInterface.hpp /usr/include/stdlib.h
rippled.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
rippled.o: /usr/include/endian.h /usr/include/bits/endian.h
rippled.o: /usr/include/bits/byteswap.h /usr/include/sys/types.h
rippled.o: /usr/include/sys/select.h /usr/include/bits/select.h
rippled.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
rippled.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
rippled.o: mongoose/mongoose.h
RippleInterface.o: /usr/include/stdlib.h /usr/include/features.h
RippleInterface.o: /usr/include/bits/predefs.h /usr/include/sys/cdefs.h
RippleInterface.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
RippleInterface.o: /usr/include/gnu/stubs-64.h /usr/include/bits/waitflags.h
RippleInterface.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
RippleInterface.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
RippleInterface.o: /usr/include/sys/types.h /usr/include/bits/types.h
RippleInterface.o: /usr/include/bits/typesizes.h /usr/include/time.h
RippleInterface.o: /usr/include/bits/time.h /usr/include/xlocale.h
RippleInterface.o: /usr/include/sys/select.h /usr/include/bits/select.h
RippleInterface.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
RippleInterface.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
RippleInterface.o: /usr/include/stdio.h /usr/include/libio.h
RippleInterface.o: /usr/include/_G_config.h /usr/include/wchar.h
RippleInterface.o: /usr/include/bits/stdio_lim.h
RippleInterface.o: /usr/include/bits/sys_errlist.h /usr/include/assert.h
RippleInterface.o: /usr/include/sys/stat.h /usr/include/bits/stat.h
RippleInterface.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
RippleInterface.o: /usr/include/bits/environments.h
RippleInterface.o: /usr/include/bits/confname.h /usr/include/getopt.h
RippleInterface.o: /usr/include/string.h RippleInterface.hpp
RippleInterface.o: mongoose/mongoose.h Ripple.hpp RippleUser.hpp
RippleInterface.o: RippleException.hpp RippleTask.hpp RippleDefines.hpp
RippleInterface.o: RippleLog.hpp JSON.hpp
