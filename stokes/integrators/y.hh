#ifndef DUNE_STOKES_INTEGRATORS_HH
#define DUNE_STOKES_INTEGRATORS_HH

#include <dune/stokes/integrators/base.hh>

namespace Dune {
namespace Stokes {
namespace Integrators {

	template < class MatrixObjectType, class Traits >
	class Y
	{
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
		typedef typename Traits::SigmaJacobianRangeType
			SigmaJacobianRangeType;
		typedef typename Traits::LocalIntersectionCoordinateType
			LocalIntersectionCoordinateType;


		MatrixObjectType& matrix_object_;
		public:
			Y( MatrixObjectType& matrix_object	)
				:matrix_object_(matrix_object)
			{}

			template < class InfoContainerVolumeType >
			void applyVolume( const InfoContainerVolumeType& info )
			{
				// (Y)
				//
//                if ( discreteModel_.isGeneralized() )
				{
				for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
					for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
						double Y_i_j = 0.0;
						// sum over all quadrature points
						for ( size_t quad = 0; quad < volumeQuadratureElement.nop(); ++quad ) {
							// get x
							const ElementCoordinateType x = volumeQuadratureElement.point( quad );
							// get the integration factor
							const double elementVolume = geometry.integrationElement( x );
							// get the quadrature weight
							const double integrationWeight = volumeQuadratureElement.weight( quad );
							// compute \tau_{j}:\nabla v_{i}
							VelocityRangeType v_i( 0.0 );
							velocityBaseFunctionSetElement.evaluate( i, x, v_i );
							VelocityRangeType v_j( 0.0 );
							velocityBaseFunctionSetElement.evaluate( j, x, v_j );
							const double v_i_times_v_j = v_i * v_j;
							Y_i_j += elementVolume
								* integrationWeight
								* alpha
								* v_i_times_v_j;
						} // done sum over quadrature points
						// if small, should be zero
						if ( fabs( Y_i_j ) < eps ) {
							Y_i_j = 0.0;
						}
						else
							// add to matrix
							localYmatrixElement.add( i, j, Y_i_j );
					}
				} // done computing Y's volume integral
				}
			}

			template < class InfoContainerInteriorFaceType >
			void applyInteriorFace( const InfoContainerInteriorFaceType& info )
			{
				typename MatrixObjectType::LocalMatrixType
						localWmatrixElement = matrix_object_.localMatrix( info.entity, info.entity );
				typename MatrixObjectType::LocalMatrixType
						localWmatrixNeighbour = matrix_object_.localMatrix( info.neighbour, info.entity );
				// (Y)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U{+}}(v{j})\cdot n_{t}ds // Y's element surface integral
				//           += \int_{\varepsilon\in\Epsilon_{I}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U{-}}(v{j})\cdot n_{t}ds // Y's neighbour surface integral
				//                                                                                                         // see also "Y's boundary integral" below
//                        if ( discreteModel_.hasSigmaFlux() ) {
					for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
						// compute Y's element surface integral
						for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
							double Y_i_j = 0.0;
							// sum over all quadrature points
							for ( size_t quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
								// get x codim<0> and codim<1> coordinates
								const ElementCoordinateType x = faceQuadratureElement.point( quad );
								const LocalIntersectionCoordinateType xLocal = faceQuadratureElement.localPoint( quad );
								// get the integration factor
								const double elementVolume = intersectionGeometry.integrationElement( xLocal );
								// get the quadrature weight
								const double integrationWeight = faceQuadratureElement.weight( quad );
								// compute -\mu v_{i}\cdot\hat{\sigma}^{U{+}}(v{j})\cdot n_{t}
								const VelocityRangeType outerNormal = intersection.unitOuterNormal( xLocal );
								VelocityRangeType v_j( 0.0 );
								velocityBaseFunctionSetElement.evaluate( j, x, v_j );
								VelocityRangeType v_i( 0.0 );
								velocityBaseFunctionSetElement.evaluate( i, x, v_i );
								const double v_i_times_v_j = v_i * v_j;
								Y_i_j += C_11
									* elementVolume
									* integrationWeight
									* v_i_times_v_j;
							} // done sum over all quadrature points
							// if small, should be zero
							if ( fabs( Y_i_j ) < eps ) {
								Y_i_j = 0.0;
							}
							else
								// add to matrix
								localYmatrixElement.add( i, j, Y_i_j );
						} // done computing Y's element surface integral
						// compute Y's neighbour surface integral
						for ( int i = 0; i < numVelocityBaseFunctionsNeighbour; ++i ) {
							double Y_i_j = 0.0;
							// sum over all quadrature points
							for ( size_t quad = 0; quad < faceQuadratureNeighbour.nop(); ++quad ) {
								// get x codim<0> and codim<1> coordinates
								const ElementCoordinateType xInside = faceQuadratureElement.point( quad );
								const ElementCoordinateType xOutside = faceQuadratureNeighbour.point( quad );
								const LocalIntersectionCoordinateType xLocal = faceQuadratureNeighbour.localPoint( quad );
								// get the integration factor
								const double elementVolume = intersectionGeometry.integrationElement( xLocal );
								// get the quadrature weight
								const double integrationWeight = faceQuadratureNeighbour.weight( quad );
								// compute -\mu v_{i}\cdot\hat{\sigma}^{U{-}}(v{j})\cdot n_{t}
								const VelocityRangeType outerNormal = intersection.unitOuterNormal( xLocal );
								VelocityRangeType v_i( 0.0 );
								velocityBaseFunctionSetNeighbour.evaluate( i, xOutside, v_i );
								VelocityRangeType v_j( 0.0 );
								velocityBaseFunctionSetElement.evaluate( j, xInside, v_j );
								const double v_i_times_v_j = v_i * v_j;
								Y_i_j += -1.0
									* C_11
									* elementVolume
									* integrationWeight
									* v_i_times_v_j;
							} // done sum over all quadrature points
							// if small, should be zero
							if ( fabs( Y_i_j ) < eps ) {
								Y_i_j = 0.0;
							}
							else
								// add to matrix
								localYmatrixNeighbour.add( i, j, Y_i_j );
						} // done computing Y's neighbour surface integral
					} // done computing Y's surface integrals
//                        }

			}

			template < class InfoContainerFaceType >
			void applyBoundaryFace( const InfoContainerFaceType& info )
			{
				typename MatrixObjectType::LocalMatrixType
						localWmatrixElement = matrix_object_.localMatrix( info.entity, info.entity );
				// (Y)_{i,j} += \int_{\varepsilon\in\Epsilon_{D}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U^{+}}(v_{j})\cdot n_{t}ds // Y's boundary integral
				//                                                                                                           // see also "Y's element surface integral" and "Y's neighbour surface integral" above
//                        if ( discreteModel_.hasSigmaFlux() ) {
					for ( int i = 0; i < numVelocityBaseFunctionsElement; ++i ) {
						for ( int j = 0; j < numVelocityBaseFunctionsElement; ++j ) {
							double Y_i_j = 0.0;
							// sum over all quadrature points
							for ( size_t quad = 0; quad < faceQuadratureElement.nop(); ++quad ) {
								// get x codim<0> and codim<1> coordinates
								const ElementCoordinateType x = faceQuadratureElement.point( quad );
								const LocalIntersectionCoordinateType xLocal = faceQuadratureElement.localPoint( quad );
								// get the integration factor
								const double elementVolume = intersectionGeometry.integrationElement( xLocal );
								// get the quadrature weight
								const double integrationWeight = faceQuadratureElement.weight( quad );
								// compute -\mu v_{i}\cdot\hat{\sigma}^{U^{+}}(v_{j})\cdot n_{t}
								const VelocityRangeType outerNormal = intersection.unitOuterNormal( xLocal );
								VelocityRangeType v_j( 0.0 );
								velocityBaseFunctionSetElement.evaluate( j, x, v_j );
								VelocityRangeType v_i( 0.0 );
								velocityBaseFunctionSetElement.evaluate( i, x, v_i );
								const double v_i_times_v_j = v_i * v_j;
								Y_i_j += C_11
									* elementVolume
									* integrationWeight
									* v_i_times_v_j;
							} // done sum over all quadrature points
							// if small, should be zero
							if ( fabs( Y_i_j ) < eps ) {
								Y_i_j = 0.0;
							}
							else
								// add to matrix
								localYmatrixElement.add( i, j, Y_i_j );
						}
					} // done computing Y's boundary integral
//                        }

			}
	};

} // end namespace Integrators
} // end namespace Stokes
} // end namespace Dune

#endif // DUNE_STOKES_INTEGRATORS_HH