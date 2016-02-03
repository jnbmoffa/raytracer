#include <cstdint>
#include <cstring>
#include <iostream>

namespace FastMath
{
  // ################################################################
  // ### Vectors
  // ################################################################

  // Multiply and add the two arrays like so:
  //    a[0]*b[0] + a[1]*b[1] + ...
  template<class Type, size_t N, size_t I>
  struct MultArrays
  {
    static_assert(N > 0, "Array must have size > 0");
    enum : size_t { NxtI = I + 1  /* Counter */ };
    static inline Type eval(const Type (&a)[N], const Type (&b)[N])
    {
      return a[I]*b[I] + MultArrays<Type,N,NxtI>::eval(a,b);
    }
  };
  // Specialize for 4D and 3D
  template<> struct MultArrays<double,4,4> {
    static inline double eval(const double (&)[4], const double (&)[4])
    { return 0.0; }
  };
  template<> struct MultArrays<double,3,3> {
    static inline double eval(const double (&)[3], const double (&)[3])
    { return 0.0; }
  };


  template<size_t N, class StorageType = double>
  class Vector
  {
  public:
    using value_type = StorageType;
    using vec_type = Vector<N,value_type>;

    Vector() 
    {
      std::memset(v_,0,sizeof(value_type)*N);
    }
    Vector(const vec_type& other)
    {
      std::copy(other.v_, other.v_+N, v_);
    }

    // Creates a ctor that takes N elements to initialize v_ with
    // i,e: Vector4D(double x, double y, double z, double w){ ... }
    // static_asserts are used to enforce the correct number of arguments
    template<typename... Args>
    explicit Vector(const value_type& x, const Args&... args)
    {
      v_[0] = x;
      ctorHelper(std::move<decltype(args)>(args)...);
    }
    template<size_t Idx = 1,typename... Args>
    void ctorHelper(const value_type& x, const Args&... args)
    {
      static_assert(Idx < N, "Too many args passed to Vector constructor");
      v_[Idx] = x;
      ctorHelper<Idx+1>(std::move<decltype(args)>(args)...);
    }
    template<size_t Idx>
    void ctorHelper()
    {
      static_assert(Idx == N, "Too few args passed to Vector constructor");
    }

    inline vec_type& operator =(const vec_type& other)
    {
      std::copy(other.v_, other.v_+N, v_);
      return *this;
    }

    inline value_type& operator[](size_t idx)
    {
      return v_[idx];
    }
    inline value_type operator[](size_t idx) const
    {
      return v_[idx];
    }

    value_type dot(const vec_type& other) const
    {
      return MultArrays<value_type,N,0>::eval(v_,other.v_);
    }
    value_type length2() const
    {
      return MultArrays<value_type,N,0>::eval(v_,v_);
    }
    value_type length() const
    {
      return sqrt(length2());
    }

  private:
    value_type v_[N];
  };

  // Common Types
  using Vector4D = FastMath::Vector<4>;
  using Vector3D = FastMath::Vector<3>;

  // Cross product specific to 3D vectors
  inline Vector3D cross(const Vector3D& a, const Vector3D& b)
  {
    return Vector3D(a[1]*b[2] - a[2]*b[1],
                    a[2]*b[0] - a[0]*b[2],
                    a[0]*b[1] - a[1]*b[0]);
  }

  // ################################################################
  // ### Matrix Operations
  // ################################################################

  // Unrolls the inner loop of matrix mult into:
  // a[R][0] * b[0][C] + a[R][1] * b[1][C] + ...
  template<class Mtx, size_t I>
  struct MultMtxRowCol
  {
    enum : size_t { NxtI = I + 1  /* Counter */ };
    static inline auto eval(const Mtx& a, const Mtx& b,
      const size_t& R, const size_t& C)
    {
      return a[R][I] * b[I][C] + MultMtxRowCol<Mtx,NxtI>::eval(a,b,R,C);
    }
  };

  // Multiplication of matricies
  // Must specialize this class to terminate for each type of matrix
  template<class Mtx, size_t N, size_t C, size_t R, size_t I>
  struct MultMtxImpl
  {
    enum : size_t
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

  template<class Mtx, size_t N>
  struct MultMtx
  {
    static inline void eval( Mtx& ret, const Mtx& a, const Mtx& b )
    {
      MultMtxImpl< Mtx, N, 0, 0, 0 >::eval( ret, a, b );
    }
  };



  // ################################################################
  // ### Factorial
  // ################################################################

  template<uint64_t N>
  struct Factorial
  {
    enum : uint64_t { val = N * Factorial<N-1>::val };
  };
  // Base case
  template<> struct Factorial<1> { enum : uint64_t { val = 1 }; };



  // ################################################################
  // ### Fibonacci
  // ################################################################

  template<uint64_t N>
  struct Fibonacci
  {
    enum : uint64_t { val = Fibonacci<N-1>::val + Fibonacci<N-2>::val };
  };
  // Base cases
  template<> struct Fibonacci<0> { enum : uint64_t { val = 0 }; };
  template<> struct Fibonacci<1> { enum : uint64_t { val = 1 }; };
} // namespace FastMath