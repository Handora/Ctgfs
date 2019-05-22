// compile time version of min and max

#ifndef algorithm_h
#define algorithm_h

namespace ctgfs {
namespace rpc {

template <int A, int B>
struct static_max
{
    static const int value = A > B ? A : B;
};

template <int A, int B>
struct static_min
{
    static const int value = A < B ? A : B;
};

}} // namespace rpc, ctgfs

#endif
