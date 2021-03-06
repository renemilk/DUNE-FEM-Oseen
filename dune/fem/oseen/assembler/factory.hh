#ifndef DUNE_OSEEN_INTEGRATORS_FACTORY_HH
#define DUNE_OSEEN_INTEGRATORS_FACTORY_HH

#include <dune/common/static_assert.hh>
#include <dune/stuff/common/memory.hh>
#include <dune/fem/oseen/assembler/ported_matrixobject.hh>

template <class RowSpaceImp, class ColSpaceImp = RowSpaceImp>
struct MatrixTraits : public Dune::SparseRowMatrixTraits<RowSpaceImp,ColSpaceImp> {
    struct StencilType {
        template < typename T >
        static int nonZerosEstimate( const T& rangeSpace ) {
            return rangeSpace.mapper().maxNumDofs() * 1.5f;
        }
    };
};


#define MK_FUNC_NAME(name) Discrete ## name ## FunctionSpaceType
#define TYPEDEF_MATRIX_AND_INTEGRATOR( Name, Row, Col ) \
    typedef typename MatrixObject< MK_FUNC_NAME(Row), MK_FUNC_NAME(Col) >::Type \
        Name ## matrixType; \
    typedef std::unique_ptr< Name ## matrixType > \
        Name ##  matrixInternalType; \
    typedef Oseen::Assembler:: Name < Name ## matrixType, StokesTraitsType > \
        Name ## matrixIntegratorType

#define SPECIALIZE_IntegratorSelector(Name) \
    template < class FactoryType > \
    struct IntegratorSelector< FactoryType, typename FactoryType:: Name ## matrixInternalType> \
    { typedef typename FactoryType:: Name ## matrixIntegratorType Type; }



namespace Dune { namespace Oseen { namespace Assembler {


//! A static map of DiscreteFunctionSpace onto DiscreteFunction
template < class FactoryType, class DiscreteFunctionSpaceType, bool = true >
struct DiscreteFunctionSelector {};

template < class FactoryType >
struct DiscreteFunctionSelector< FactoryType, typename FactoryType::DiscreteSigmaFunctionSpaceType >
{ typedef std::unique_ptr<typename FactoryType::DiscreteSigmaFunctionType> Type; };

template < class FactoryType >
struct DiscreteFunctionSelector< FactoryType, typename FactoryType::DiscreteVelocityFunctionSpaceType >
{ typedef std::unique_ptr<typename FactoryType::DiscreteVelocityFunctionType> Type; };

template < class FactoryType >
struct DiscreteFunctionSelector< FactoryType, typename FactoryType::DiscretePressureFunctionSpaceType >
{ typedef std::unique_ptr<typename FactoryType::DiscretePressureFunctionType> Type; };

template < class FactoryType, class DiscreteFunctionSpaceType >
//! this specialization is needed incase function types are equal
struct DiscreteFunctionSelector< FactoryType, DiscreteFunctionSpaceType, false >
{ typedef std::unique_ptr<typename FactoryType::DiscretePressureFunctionType> Type; };

//!
template < class StokesTraitsType >
class Factory {
    typedef Factory< StokesTraitsType >
        ThisType;
public:
    typedef typename StokesTraitsType::DiscreteSigmaFunctionSpaceType
        DiscreteSigmaFunctionSpaceType;
    typedef typename StokesTraitsType::DiscreteSigmaFunctionType
        DiscreteSigmaFunctionType;
    typedef typename StokesTraitsType::DiscreteVelocityFunctionSpaceType
        DiscreteVelocityFunctionSpaceType;
    typedef typename StokesTraitsType::DiscreteVelocityFunctionType
        DiscreteVelocityFunctionType;
    typedef typename StokesTraitsType::DiscretePressureFunctionSpaceType
        DiscretePressureFunctionSpaceType;
    typedef typename StokesTraitsType::DiscretePressureFunctionType
        DiscretePressureFunctionType;

    template < class T, class R >
    struct MatrixObject {
        typedef MatrixTraits<T,R> Traits;
        typedef PortedSparseRowMatrixObject<  T, R, Traits >Type;
    };
    TYPEDEF_MATRIX_AND_INTEGRATOR( M, Sigma, Sigma );
    TYPEDEF_MATRIX_AND_INTEGRATOR( W, Sigma, Velocity );
    TYPEDEF_MATRIX_AND_INTEGRATOR( X, Velocity, Sigma );
    TYPEDEF_MATRIX_AND_INTEGRATOR( Y, Velocity, Velocity );
    TYPEDEF_MATRIX_AND_INTEGRATOR( Z, Velocity, Pressure );
    TYPEDEF_MATRIX_AND_INTEGRATOR( E, Pressure, Velocity );
    TYPEDEF_MATRIX_AND_INTEGRATOR( R, Pressure, Pressure );
    static const bool verbose_ = true;

    typedef Oseen::Assembler::O< YmatrixType, StokesTraitsType, DiscreteVelocityFunctionType >
        OmatrixIntegratorType;
    typedef Oseen::Assembler::H1< DiscreteSigmaFunctionType, StokesTraitsType >
        H1_IntegratorType;
    typedef Oseen::Assembler::H2< DiscreteVelocityFunctionType , StokesTraitsType >
        H2_IntegratorType;
    typedef Oseen::Assembler::H2_O< DiscreteVelocityFunctionType , StokesTraitsType, DiscreteVelocityFunctionType >
        H2_O_IntegratorType;
    typedef Oseen::Assembler::H3< DiscretePressureFunctionType, StokesTraitsType >
        H3_IntegratorType;
    typedef tuple<	MmatrixIntegratorType,
                    WmatrixIntegratorType,
                    XmatrixIntegratorType,
                    YmatrixIntegratorType,
                    OmatrixIntegratorType,
                    ZmatrixIntegratorType,
                    EmatrixIntegratorType,
                    RmatrixIntegratorType,
                    H1_IntegratorType,
                    H2_IntegratorType,
                    H2_O_IntegratorType,
                    H3_IntegratorType >
        OseenIntegratorTuple;
    typedef tuple<	OmatrixIntegratorType,
                    H2_IntegratorType,
                    H2_O_IntegratorType>
        ConvIntegratorTuple;
    typedef tuple<	MmatrixIntegratorType,
                    WmatrixIntegratorType,
                    XmatrixIntegratorType,
                    YmatrixIntegratorType,
                    ZmatrixIntegratorType,
                    EmatrixIntegratorType,
                    RmatrixIntegratorType,
                    H1_IntegratorType,
                    H2_IntegratorType,
                    H3_IntegratorType >
        StokesIntegratorTuple;

    template < class RowSpace, class ColSpace >
    struct magic {
        //! more magic on the inside to avoid ambiguous DiscretefunctionSelector instantiation in 1D
            typedef RowSpace RowType;
            typedef ColSpace ColType;
        typedef typename MatrixObject< RowType, ColType >::Type
            InternalMatrixType;
        typedef std::unique_ptr<InternalMatrixType>
            PointerType;
    };
    template < class F, class G >
    static auto matrix( const F& f, const G& g ) -> typename magic<F,G>::PointerType
    {
        typedef typename magic<F,G>::InternalMatrixType
            InternalMatrixType;
        typename magic<F,G>::PointerType m( new InternalMatrixType(f,g) );
        m->reserve( verbose_ );
        return m;
    }
    template < class DiscreteFunctionSpaceType >
    static typename DiscreteFunctionSelector< ThisType, DiscreteFunctionSpaceType, DiscreteFunctionSpaceType::dimensionworld != 1 >::Type
        rhs( const std::string name, const DiscreteFunctionSpaceType& space )
    {
        auto f = DSC::make_unique<
        typename DiscreteFunctionSelector< ThisType,
                    DiscreteFunctionSpaceType,
                    DiscreteFunctionSpaceType::dimensionworld != 1 >
            ::Type::element_type>( name, space );
        f->clear();
        return f;
    }
};

#undef STOKES_MATRIX_OBJECT
#undef STOKES_MATRIX_OBJECT_TRAITS
/*namespace Dune*/ } /*namespace Oseen*/ } /*namespace Assembler*/ }
#endif // DUNE_OSEEN_INTEGRATORS_FACTORY_HH

/** Copyright (c) 2012, Rene Milk 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
**/

