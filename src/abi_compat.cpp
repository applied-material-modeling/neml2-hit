// Forward-compatibility shim for __cxa_call_terminate.
//
// GCC >= 13 emits calls to the libstdc++ helper __cxa_call_terminate (introduced
// at CXXABI_1.3.15) in the terminate-on-noexcept cleanup paths of exception
// handling -- including the Bison-generated parser (generated/Parser.cpp). When
// nmhit is built with such a compiler (the manylinux wheels use GCC 14) and then
// statically linked into a consumer that links with an older toolchain -- e.g.
// GCC 11, whose libstdc++ predates the symbol -- the consumer's link fails with:
//
//     undefined reference to `__cxa_call_terminate'
//
// (NEML2 links libnmhit.a into libneml2.so, which MOOSE in turn links into its
// executables; a MOOSE built with GCC 11 hit exactly this.)
//
// A weak definition makes the static archive self-contained so it links under any
// toolchain. Because it is weak, when the process's libstdc++ exports the real
// (strong) symbol -- every GCC >= 13 runtime does -- that definition overrides
// this one, so behavior is unchanged. This fallback only takes effect as a last
// resort on an older runtime, where it does what libstdc++'s version does: claim
// the in-flight exception object and terminate.

#include <exception>

// Only libstdc++ (GCC, and clang-with-libstdc++) has/needs __cxa_call_terminate.
// libc++ (e.g. macOS) uses a different terminate helper, and MSVC has neither
// libstdc++ nor <cxxabi.h>, so this whole file is a no-op on both -- the entire
// body (including the <cxxabi.h> include, which does not exist on MSVC) sits
// behind the guard. __GLIBCXX__ is defined by the <exception> include above only
// under libstdc++.
#ifdef __GLIBCXX__
#include <cxxabi.h>

extern "C" __attribute__((weak)) void
__cxa_call_terminate(void * exc) noexcept
{
  if (exc)
    __cxxabiv1::__cxa_begin_catch(exc);
  std::terminate();
}
#endif
