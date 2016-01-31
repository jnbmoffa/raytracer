// Metaprogramming multiplication of matricies
// Must specialize this class to terminate for each type of matrix
template<class Mtx, unsigned N, unsigned C, unsigned R, unsigned I>
struct MultMtxImpl
{
  enum
  {
    NxtI = I + 1,          // Counter
    NxtC = NxtI % N,       // Column
    NxtR = NxtI / N % N    // Row
  };
  static inline void eval(Mtx& ret, const Mtx& a, const Mtx& b)
  {
    ret[R][C] = a[R][0] * b[0][C] + a[R][1] * b[1][C] +
                a[R][2] * b[2][C] + a[R][3] * b[3][C];
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