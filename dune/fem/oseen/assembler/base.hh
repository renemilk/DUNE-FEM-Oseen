#ifndef DUNE_OSEEN_INTEGRATORS_BASE_HH
#define DUNE_OSEEN_INTEGRATORS_BASE_HH

#include <dune/stuff/grid/entity.hh>
#include <dune/stuff/common/misc.hh>
#include <dune/stuff/common/profiler.hh>
#include <dune/stuff/fem/localmatrix_proxy.hh>
#include <dune/fem/oseen/stab_coeff.hh>

#include <boost/integer/static_min_max.hpp>


namespace Dune {
namespace Oseen {
namespace Assembler {

    namespace {
        template<int i,typename IntegratorTuple,typename InfoType,typename FunctorType>
        struct ApplySingle
        {
            static inline void visit(IntegratorTuple& tuple, const InfoType& info)
            {
                FunctorType::apply(get<tuple_size<IntegratorTuple>::value-i>(tuple), info);
                ApplySingle<i-1,IntegratorTuple,InfoType,FunctorType>::visit(tuple,info);
            }
        };

        template<typename IntegratorTuple,typename InfoType,typename FunctorType>
	struct ApplySingle<0, IntegratorTuple,InfoType,FunctorType>
        {
            static inline void visit(IntegratorTuple&, const InfoType&)
            {}
        };
    }
	template < class SigmaJacobianRangeType, class VelocityRangeType >
	static SigmaJacobianRangeType
			prepareVelocityRangeTypeForSigmaDivergence( const VelocityRangeType& arg )
	{
		SigmaJacobianRangeType ret( 0.0 );
		assert( arg.dim() == ret[0].dim() );
		for ( int i = 0; i < int(arg.dim()) ; ++i ) {
			for ( int j = 0; j < int(arg.dim()); ++j ) {
				VelocityRangeType row( 0.0 );
				row[ j ] = arg[ i ];
				ret[ i * arg.dim() + j ] = row;
			}
		}
		return ret;
	}

	template < class Traits >
	static typename Traits::VelocityJacobianRangeType preparePressureRangeTypeForVelocityDivergence(
												const typename Traits::PressureRangeType& arg )
	{
		typename Traits::VelocityJacobianRangeType ret( 0.0 );
		for ( unsigned int i = 0; i < ret[0].dim(); ++i ) {
			typename Traits::VelocityRangeType row( 0.0 );
			row[ i ] = arg;
			ret[ i ] = row;
		}
		return ret;
	}

    //! bring back the super useful evaluateSingle and evaluateGradientSingle which were removed from FEM's Basefunctionset
    template < class WrappedBasefuntionsetImp >
    class BasefunctionsetWrapper : public WrappedBasefuntionsetImp {
            typedef typename WrappedBasefuntionsetImp::RangeFieldType
                RangeFieldType;
            typedef typename WrappedBasefuntionsetImp::RangeType
                RangeType;
            typedef typename WrappedBasefuntionsetImp::JacobianRangeType
                JacobianRangeType;
            typedef typename WrappedBasefuntionsetImp::DomainType
                DomainType;
            typedef typename WrappedBasefuntionsetImp::FunctionSpaceType
                FunctionSpaceType;
        public:
            typedef WrappedBasefuntionsetImp
                WrappedBasefuntionsetType;
        BasefunctionsetWrapper( const WrappedBasefuntionsetType& wrapped )
            : WrappedBasefuntionsetType( wrapped )
        {}

        /** \copydoc Dune::BaseFunctionSetInterface::evaluateSingle(const int baseFunction,const PointType &x,const RangeType &psi) const */
        template< class PointType >
        inline RangeFieldType evaluateSingle ( const int baseFunction,
                                               const PointType &x,
                                               const RangeType &psi ) const
        {
          RangeType phi;
          WrappedBasefuntionsetType::evaluate( baseFunction, x, phi );
          return phi * psi;
        }

        /** \copydoc Dune::BaseFunctionSetInterface::evaluateGradientSingle(const int baseFunction,const EntityType &entity,const PointType &x,const JacobianRangeType &psi) const */
        template< class EntityType, class PointType >
        inline RangeFieldType evaluateGradientSingle( const int baseFunction,
                                                      const EntityType &entity,
                                                      const PointType &x,
                                                      const JacobianRangeType &psi ) const
        {
          const auto& geometry = entity.geometry();
          const auto& jacobianInverseTransposed
            = geometry.jacobianInverseTransposed( coordinate( x ) );

          JacobianRangeType gradPhi;
          WrappedBasefuntionsetType::jacobian( baseFunction, x, gradPhi );

          RangeFieldType result = 0;
          for( int i = 0; i < FunctionSpaceType :: dimRange; ++i )
          {
            DomainType gradScaled( 0 );
            jacobianInverseTransposed.umv( gradPhi[ i ], gradScaled );
            result += gradScaled * psi[ i ];
          }
          return result;
        }
    };

    template < class Traits >
    struct PolOrder {
        static const int value = 2 * ( boost::static_signed_max<
                                            boost::static_signed_max< Traits::pressureSpaceOrder,
                                                                Traits::velocitySpaceOrder >::value,
                                            Traits::sigmaSpaceOrder >::value
                                      + 1 ) ;
    };

	template < class Traits, class IntegratorTuple >
	class Coordinator
	{
	protected:
		typedef Coordinator< Traits, IntegratorTuple >
			CoordinatorType;

		const typename Traits::DiscreteModelType&					discrete_model_;
		const typename Traits::GridPartType&						grid_part_;
		const typename Traits::DiscreteVelocityFunctionSpaceType&	velocity_space_;
		const typename Traits::DiscretePressureFunctionSpaceType&	pressure_space_;
		const typename Traits::DiscreteSigmaFunctionSpaceType&		sigma_space_;

	public:
		typedef typename Traits::ElementCoordinateType
			ElementCoordinateType;
		typedef typename Traits::SigmaRangeType
			SigmaRangeType;
		typedef typename Traits::VelocityRangeType
			VelocityRangeType;
		typedef typename Traits::PressureRangeType
			PressureRangeType;
		typedef typename Traits::VelocityJacobianRangeType
			VelocityJacobianRangeType;
		typedef typename Traits::PressureJacobianRangeType
			PressureJacobianRangeType;
        typedef BasefunctionsetWrapper< typename Traits::DiscreteSigmaFunctionSpaceType::BaseFunctionSetType >
				SigmaBaseFunctionSetType;
        typedef BasefunctionsetWrapper< typename Traits::DiscreteVelocityFunctionSpaceType::BaseFunctionSetType >
			VelocityBaseFunctionSetType;
        typedef BasefunctionsetWrapper< typename Traits::DiscretePressureFunctionSpaceType::BaseFunctionSetType >
			PressureBaseFunctionSetType;

		Coordinator(const typename Traits::DiscreteModelType&					discrete_model,
						const typename Traits::GridPartType&						grid_part,
						const typename Traits::DiscreteVelocityFunctionSpaceType&	velocity_space,
						const typename Traits::DiscretePressureFunctionSpaceType&	pressure_space,
						const typename Traits::DiscreteSigmaFunctionSpaceType&		sigma_space)
					:discrete_model_(discrete_model),
					grid_part_(grid_part),
					velocity_space_(velocity_space),
					pressure_space_(pressure_space),
					sigma_space_(sigma_space)
		{}

		//! just to avoid overly long argument lists
		struct InfoContainerVolume {
			const typename Traits::EntityType& entity;
            const typename Traits::EntityType::Geometry geometry;

			const SigmaBaseFunctionSetType
					sigma_basefunction_set_element;
			const VelocityBaseFunctionSetType
					velocity_basefunction_set_element;
			const PressureBaseFunctionSetType
					pressure_basefunction_set_element;
			const int numSigmaBaseFunctionsElement;
			const int numVelocityBaseFunctionsElement;
			const int numPressureBaseFunctionsElement;
			const typename Traits::VolumeQuadratureType volumeQuadratureElement;
			const typename Traits::DiscreteModelType&	discrete_model;
			const double eps;
			const double viscosity;
			const double convection_scaling;
			const double pressure_gradient_scaling;
			//! generalized stokes alpha
			const double alpha;
			const typename Traits::GridPartType& grid_part;
			InfoContainerVolume(const CoordinatorType& interface,
								const typename Traits::EntityType& ent,
								const typename Traits::DiscreteModelType& discrete_modelIn,
								const typename Traits::GridPartType& grid_partIn)
				: entity( ent ),
				  geometry( entity.geometry() ),
				  sigma_basefunction_set_element( interface.sigma_space_.baseFunctionSet( entity ) ),
				  velocity_basefunction_set_element( interface.velocity_space_.baseFunctionSet( entity ) ),
				  pressure_basefunction_set_element( interface.pressure_space_.baseFunctionSet( entity ) ),
                  numSigmaBaseFunctionsElement( sigma_basefunction_set_element.size() ),
                  numVelocityBaseFunctionsElement( velocity_basefunction_set_element.size() ),
                  numPressureBaseFunctionsElement( pressure_basefunction_set_element.size() ),
                  volumeQuadratureElement( entity, PolOrder<Traits>::value ),
				  discrete_model( discrete_modelIn ),
				  eps( DSC_CONFIG_GET( "eps", 1.0e-14 ) ),
				  viscosity( discrete_modelIn.viscosity() ),
				  convection_scaling( discrete_modelIn.convection_scaling() ),
				  pressure_gradient_scaling( discrete_modelIn.pressure_gradient_scaling() ),
				  alpha( discrete_modelIn.alpha() ),
				  grid_part( grid_partIn )
			{}
			virtual ~InfoContainerVolume() {}
		};
		struct InfoContainerFace : public InfoContainerVolume {
            const typename Traits::IntersectionIteratorType::Intersection& intersection;
            const typename Traits::IntersectionIteratorType::Intersection::Geometry intersectionGeometry;
			const typename Traits::FaceQuadratureType faceQuadratureElement;
			const double lengthOfIntersection;
			const StabilizationCoefficients& stabil_coeff;
			const double C_11;
			const double D_11;
			typename Traits::VelocityRangeType D_12;

			InfoContainerFace (const CoordinatorType& interface,
								const typename Traits::EntityType& ent,
							   const typename Traits::IntersectionIteratorType::Intersection& inter,
								const typename Traits::DiscreteModelType& discrete_modelIn,
							   const typename Traits::GridPartType& grid_partIn )
				:InfoContainerVolume( interface, ent, discrete_modelIn, grid_partIn ),
				  intersection( inter ),
				  intersectionGeometry( intersection.geometry() ),
				  faceQuadratureElement( interface.sigma_space_.gridPart(),
																  intersection,
                                                                  PolOrder<Traits>::value,
																  Traits::FaceQuadratureType::INSIDE ),
                  lengthOfIntersection( intersection.geometry().volume() ),
				  stabil_coeff( discrete_modelIn.getStabilizationCoefficients() ),
				  C_11( penalty<-1>(ent,intersection, stabil_coeff, lengthOfIntersection ) ),
				  D_11( penalty<1>(ent,intersection, stabil_coeff, lengthOfIntersection ) ),
				  D_12( 1 )//vector!
			{
				D_12 /= D_12.two_norm();
				D_12 *= stabil_coeff.Factor("D12");
			}
		};

		static double charactisticSize(const typename Traits::EntityType& entity,
									   const typename Traits::IntersectionIteratorType::Intersection& intersection)
		{
            return entity.geometry().volume() / intersection.geometry().volume();
		}

		template <int power>
		static double penalty(const typename Traits::EntityType& entity,
							  const typename Traits::EntityType& neighbour,
							  const typename Traits::IntersectionIteratorType::Intersection& intersection,
							  const Dune::StabilizationCoefficients& stabil_coeff,
							  const double lengthOfIntersection )
		{
			const int penalty_form = DSC_CONFIG_GET( "penalty_form", 1 );
			switch (penalty_form) {
				case 1:
				{
					const double entity_measure = std::pow( charactisticSize( entity, intersection), double(power) );
					const double neighbour_measure = std::pow( charactisticSize( neighbour, intersection), double(power) );
					return (entity_measure+neighbour_measure)/2.0;
				}
				case 2:
				{
                    const double entity_diameter = std::pow( DSG::geometryDiameter( entity ), double(power) );
                    const double neighbour_diameter = std::pow( DSG::geometryDiameter( neighbour ), double(power) );
					return std::max(entity_diameter, neighbour_diameter);
				}
				case 3:
				{
                    const double entity_diameter = std::pow( DSG::geometryDiameter( entity ), double(power) );
                    const double neighbour_diameter = std::pow( DSG::geometryDiameter( neighbour ), double(power) );
					return std::max(entity_diameter, neighbour_diameter) * ( power > 0 ? stabil_coeff.Factor("C11") : stabil_coeff.Factor("D11") );
				}
				default:
				case 4:
				{
//					return std::pow( intersection.intersectionGlobal().volume(), double(power) );
					if (power==-1)
						return stabil_coeff.Factor("C11") * std::pow( lengthOfIntersection, stabil_coeff.Power("C11") );
					else
						return stabil_coeff.Factor("D11") * std::pow( lengthOfIntersection, stabil_coeff.Power("D11") );
				}
			}


		}
		template <int power>
		static double penalty(const typename Traits::EntityType& entity,
							  const typename Traits::IntersectionIteratorType::Intersection& intersection,
							  const Dune::StabilizationCoefficients& stabil_coeff,
							  const double lengthOfIntersection)
		{
			return penalty<power>( entity, entity, intersection, stabil_coeff, lengthOfIntersection );
		}

		struct InfoContainerInteriorFace : public InfoContainerFace {
            const typename Traits::EntityType& neighbour;
			const SigmaBaseFunctionSetType
					sigma_basefunction_set_neighbour;
			const VelocityBaseFunctionSetType
					velocity_basefunction_set_neighbour;
			const PressureBaseFunctionSetType
					pressure_basefunction_set_neighbour;
			const int numSigmaBaseFunctionsNeighbour;
			const int numVelocityBaseFunctionsNeighbour;
			const int numPressureBaseFunctionsNeighbour;
			const typename Traits::FaceQuadratureType faceQuadratureNeighbour;
			const double C_11;//yupp, we're hiding the base class members here
			const double D_11;

			InfoContainerInteriorFace (const CoordinatorType& interface,
								const typename Traits::EntityType& ent,
							   const typename Traits::EntityType& nei,
							   const typename Traits::IntersectionIteratorType::Intersection& inter,
								const typename Traits::DiscreteModelType& discrete_modelIn,
								const typename Traits::GridPartType& grid_partIn )
                :InfoContainerFace( interface, ent, inter, discrete_modelIn, grid_partIn ),
				  neighbour( nei ),
				  sigma_basefunction_set_neighbour( interface.sigma_space_.baseFunctionSet( neighbour ) ),
				  velocity_basefunction_set_neighbour( interface.velocity_space_.baseFunctionSet( neighbour ) ),
				  pressure_basefunction_set_neighbour( interface.pressure_space_.baseFunctionSet( neighbour ) ),
                  numSigmaBaseFunctionsNeighbour( sigma_basefunction_set_neighbour.size() ),
                  numVelocityBaseFunctionsNeighbour( velocity_basefunction_set_neighbour.size() ),
                  numPressureBaseFunctionsNeighbour( pressure_basefunction_set_neighbour.size() ),
				  faceQuadratureNeighbour( interface.sigma_space_.gridPart(),
																  inter,
                                                                  PolOrder<Traits>::value,
																  Traits::FaceQuadratureType::OUTSIDE ),
				  C_11( penalty<-1>( ent, nei, inter, InfoContainerFace::stabil_coeff, InfoContainerFace::lengthOfIntersection ) ),
				  D_11( penalty<1>( ent, nei, inter, InfoContainerFace::stabil_coeff, InfoContainerFace::lengthOfIntersection ) )
			{
				//some integration logic depends on this
				assert( InfoContainerFace::faceQuadratureElement.nop() == faceQuadratureNeighbour.nop() );
			}
		};

		struct ApplyVolume {
			template < class IntegratorType >
			static void apply( IntegratorType& integrator, const InfoContainerVolume& info )
			{
				DSC::Profiler::ScopedTiming s(integrator.name);
				integrator.applyVolume( info );
			}
		};

		struct ApplyInteriorFace {
			template < class IntegratorType >
			static void apply( IntegratorType& integrator, const InfoContainerInteriorFace& info )
			{
				DSC::Profiler::ScopedTiming s(integrator.name);
				integrator.applyInteriorFace( info );
			}
		};

		struct ApplyBoundaryFace {
			template < class IntegratorType >
			static void apply( IntegratorType& integrator, const InfoContainerFace& info )
			{
				DSC::Profiler::ScopedTiming s(integrator.name);
				integrator.applyBoundaryFace( info );
			}
		};

        //! A gloryfied For loop in 28 lines
		template < class FunctorType, class InfoType >
		class ForEachIntegrator {
		    public:
			//! \brief Constructor
			//! \param tuple The tuple which we want to process.
			ForEachIntegrator(IntegratorTuple& tuple, const InfoType& info)
			{
			    ApplySingle<tuple_size<IntegratorTuple>::value,IntegratorTuple,InfoType,FunctorType>::visit(tuple, info);
			}
		};

		void apply ( IntegratorTuple& integrator_tuple ) const
		{
			DSC::Profiler::ScopedTiming assembler_time("assembler");
            const auto& gridView = grid_part_.grid().leafView();

            for ( const auto& entity : DSC::viewRange(gridView))
			{
                const InfoContainerVolume e_info( *this, entity, discrete_model_,grid_part_ );
                ForEachIntegrator<ApplyVolume,InfoContainerVolume>( integrator_tuple, e_info );

				// walk the intersections
				const typename Traits::IntersectionIteratorType intItEnd = gridView.iend( entity );
				for (   typename Traits::IntersectionIteratorType intIt = gridView.ibegin( entity );
						intIt != intItEnd;
						++intIt )
				{
					const typename Traits::IntersectionIteratorType::Intersection& intersection = *intIt;

					// if we are inside the grid
					if ( intersection.neighbor() && !intersection.boundary() )
					{
						//! DO NOT TRY TO DEREF outside() DIRECTLY
                        const typename Traits::IntersectionIteratorType::Intersection::EntityPointer neighbourPtr = intersection.outside();
                        const InfoContainerInteriorFace i_info( *this, entity, *neighbourPtr, intersection, discrete_model_,grid_part_ );
                        ForEachIntegrator<ApplyInteriorFace,InfoContainerInteriorFace>( integrator_tuple, i_info );
					}
					else if ( !intersection.neighbor() && intersection.boundary() )
					{
                        const InfoContainerFace o_info( *this, entity, intersection, discrete_model_, grid_part_ );
                        ForEachIntegrator<ApplyBoundaryFace,InfoContainerFace>( integrator_tuple, o_info );
					}
				}
			}
		}
	};

} // end namespace Assembler
} // end namespace Oseen
} // end namespace Dune

#endif // DUNE_OSEEN_INTEGRATORS_BASE_HH

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

