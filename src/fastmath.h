#include <cstdint>

namespace FastMath
{
  // Unrolls the inner loop of matrix mult into:
  // a[R][0] * b[0][C] + a[R][1] * b[1][C] + ...
  template<class Mtx, unsigned I>
  struct MultMtxRowCol
  {
    enum { NxtI = I + 1  /* Counter */ };
    static inline auto eval(const Mtx& a, const Mtx& b,
      const unsigned& R, const unsigned& C)
    {
      return a[R][I] * b[I][C] + MultMtxRowCol<Mtx,NxtI>::eval(a,b,R,C);
    }
  };

  // Multiplication of matricies
  // Must specialize this class to terminate for each type of matrix
  template<class Mtx, unsigned N, unsigned C, unsigned R, unsigned I>
  struct MultMtxImpl
  {
    enum : unsigned
    {
      NxtI = I + 1,          // Counter
      NxtC = NxtI % N,       // Column
      NxtR = NxtI / N % N    // Row
    };
    static inline void eval(Mtx& ret, const Mtx& a, const Mtx& b)
    {
      ret[R][C] = MultMtxRowCol<Mtx,0>::eval(a,b,R,C);
      MultMtxImpl<Mtx,N,NxtC,NxtR,NxtI>::eval(ret,a,b);
    }
  };

  template<class Mtx, unsigned N>
  struct MultMtx
  {
    static inline void eval( Mtx& ret, const Mtx& a, const Mtx& b )
    {
      MultMtxImpl< Mtx, N, 0, 0, 0 >::eval( ret, a, b );
    }
  };


  // Factorial
  template<uint64_t N>
  struct Factorial
  {
    enum : uint64_t { val = N * Factorial<N-1>::val };
  };
  // Base case
  template<> struct Factorial<1> { enum : uint64_t { val = 1 }; };


  // Fibonacci
  template<uint64_t N>
  struct Fibonacci
  {
    enum : uint64_t { val = Fibonacci<N-1>::val + Fibonacci<N-2>::val };
  };
  // Base cases
  template<> struct Fibonacci<0> { enum : uint64_t { val = 0 }; };
  template<> struct Fibonacci<1> { enum : uint64_t { val = 1 }; };
} // namespace FastMath