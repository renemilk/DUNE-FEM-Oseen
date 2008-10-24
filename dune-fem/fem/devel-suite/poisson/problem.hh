#ifndef DUNE_PROBLEM_HH
#define DUNE_PROBLEM_HH

namespace Dune
{

  // right hand side of governing problem 
  template< class FunctionSpaceImp >
  class RHSFunction
  : public Function< FunctionSpaceImp, RHSFunction< FunctionSpaceImp > >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef RHSFunction< FunctionSpaceType > ThisType;
    typedef Function< FunctionSpaceType, ThisType > BaseType;

  public:
    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    inline RHSFunction ( const FunctionSpaceType &functionSpace )
    : BaseType( functionSpace )
    {
    }
     
    // f( x, y, z ) = 2 \sum_{i=1}^{dimworld} \prod_{j \neq i} (x_j-x_j^2)
    inline void evaluate( const DomainType &x , RangeType &phi ) const
    {
      enum { dimension = DomainType :: dimension };
      
      phi = 0;
      for( int i = 0; i < dimension; ++i ) { 
        RangeType tmp = 2;
        for( int j = 0; j < dimension; ++j ) {
          if( i == j )
            continue;
          const DomainFieldType &x_j = x[ j ];
          tmp *= x_j - SQR( x_j );
        }
        phi += tmp;
      }
    }
  }; // end class RHSFunction



  //! the exact solution to the problem for EOC calculation 
  template< class FunctionSpaceImp >
  class ExactSolution
  : public Function < FunctionSpaceImp, ExactSolution< FunctionSpaceImp > >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef ExactSolution< FunctionSpaceType > ThisType;
    typedef Function< FunctionSpaceType, ThisType > BaseType;

  public:
    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: JacobianRangeType JacobianRangeType;

    enum { dimDomain = DomainType :: dimension };

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    inline ExactSolution ( const FunctionSpaceType &functionSpace )
    : BaseType( functionSpace )
    {
    }
   
    // u( x, y, z ) = \prod_{i=1}^{dimworld} (x_i - x_i^2).
    inline void evaluate ( const DomainType &x , RangeType &phi ) const
    {
      phi = 1;
      for( int i = 0; i < dimDomain; ++i )
      {
        const DomainFieldType &x_i = x[ i ];
        phi *= x_i - SQR( x_i );
      }
    }

    inline void jacobian ( const DomainType &x, JacobianRangeType &ret ) const
    {
      for( unsigned int i = 0; i < dimDomain; ++i )
      {
        const DomainFieldType &x_i = x[ i ];
        ret[ 0 ][ i ] = 1.0 - 2.0 * x_i;

        for( unsigned int j = 0; j < dimDomain; ++j )
        {
          const DomainFieldType &x_j = x[ j ];
          if( i != j )
            ret[ 0 ][ i ] *= x_j - SQR( x_j );
        }
      }
    }
   
    inline void evaluate ( const DomainType &x , RangeFieldType time, RangeType &phi ) const
    {
      evaluate( x, phi );
    }
  }; // end class ExactSolution



  // diffusion coefficient for this problem
  template< class FunctionSpaceImp >
  class Tensor
  : public Function< FunctionSpaceImp, Tensor< FunctionSpaceImp > >
  {
  public:
    typedef FunctionSpaceImp FunctionSpaceType;

  private:
    typedef Tensor< FunctionSpaceType > ThisType;
    typedef Function< FunctionSpaceType, ThisType > BaseType;

  public:
    typedef typename FunctionSpaceType :: DomainType DomainType;
    typedef typename FunctionSpaceType :: RangeType RangeType;

    typedef typename FunctionSpaceType :: DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType :: RangeFieldType RangeFieldType;

  public:
    inline Tensor ( const FunctionSpaceType &functionSpace )
    : BaseType( functionSpace )
    {
    }
    
    inline void evaluate ( int i, int j, const DomainType &x, RangeType &phi ) const
    {
      evaluate( x, phi );
    }
    
    inline void evaluate ( const DomainType &x, RangeType &phi ) const
    {
      phi = 1;
    }

    inline void evaluate ( const DomainType &x, RangeFieldType time, RangeType &phi ) const
    {
      evaluate( x, phi );
    }
  }; //end class Tensor


}

#endif