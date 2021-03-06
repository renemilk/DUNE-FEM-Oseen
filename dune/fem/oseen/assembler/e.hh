#ifndef DUNE_OSEEN_INTEGRATORS_E_HH
#define DUNE_OSEEN_INTEGRATORS_E_HH

#include <dune/fem/oseen/assembler/base.hh>
#include <dune/stuff/common/matrix.hh>

namespace Dune {
namespace Oseen {
namespace Assembler {

	template < class MatrixObjectType, class Traits >
	class E
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
		typedef typename Traits::EntityType::Geometry
			EntityGeometryType;
		typedef typename Dune::FieldMatrix< typename EntityGeometryType::ctype,
											EntityGeometryType::coorddimension,
											EntityGeometryType::mydimension >
			JacobianInverseTransposedType;
		typedef DSFe::LocalMatrixProxy<MatrixObjectType>
			LocalMatrixProxyType;

		MatrixObjectType& matrix_object_;
		public:
			E( MatrixObjectType& matrix_object	)
				:matrix_object_(matrix_object)
			{}

			template < class InfoContainerVolumeType >
			void applyVolume( const InfoContainerVolumeType& info )
			{
				LocalMatrixProxyType localEmatrixElement( matrix_object_, info.entity, info.entity, info.eps );
				// (E)_{i,j} += -\int_{T}v_{j}\cdot\nabla q_{i}dx // E's volume integral
				//                                                // see also "E's entitity surface integral", "E's neighbour surface integral" and "E's boundary integral" below
				for ( int i = 0; i < info.numPressureBaseFunctionsElement; ++i ) {
					for ( int j = 0; j < info.numVelocityBaseFunctionsElement; ++j ) {
						double E_i_j = 0.0;
                        for ( size_t quad = 0; quad < info.volumeQuadratureElement.nop(); ++ quad ) {
                            const auto x = info.volumeQuadratureElement.point( quad );
                            const double elementVolume = info.geometry.integrationElement( x );
                            const double integrationWeight = info.volumeQuadratureElement.weight( quad );
							// compute v_{j}\cdot(\nabla q_i)
							VelocityRangeType v_j( 0.0 );
							info.velocity_basefunction_set_element.evaluate( j, x, v_j );
							PressureJacobianRangeType jacobian_of_q_i( 0.0 );
							info.pressure_basefunction_set_element.jacobian( i, x, jacobian_of_q_i );
							const VelocityRangeType gradient_of_q_i_untransposed( jacobian_of_q_i[0] );
							const JacobianInverseTransposedType jacobianInverseTransposed = info.geometry.jacobianInverseTransposed( x );
							VelocityRangeType gradient_of_q_i( 0.0 );
							jacobianInverseTransposed.mv( gradient_of_q_i_untransposed, gradient_of_q_i );
							const double gradient_of_q_i_times_v_j = gradient_of_q_i * v_j;
							E_i_j += -1.0
								* elementVolume
								* integrationWeight
								* gradient_of_q_i_times_v_j;
						} // done sum over all quadrature points
						localEmatrixElement.add( i, j, E_i_j );
					}
				} // done computing E's volume integral
			}

			template < class InfoContainerInteriorFaceType >
			void applyInteriorFace( const InfoContainerInteriorFaceType& info )
			{
				LocalMatrixProxyType localEmatrixElement( matrix_object_, info.entity, info.entity, info.eps );
				LocalMatrixProxyType localEmatrixNeighbour( matrix_object_, info.neighbour, info.entity, info.eps );
				// (E)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{u}_{p}^{U^{+}}(v_{j})\cdot n_{T}q_{i}ds // E's element surface integral
				//           += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{u}_{p}^{U^{-}}(v_{j})\cdot n_{T}q_{i}ds // E's neighbour surface integral
				//                                                                                                // see also "E's boundary integral" below
				//                                                                                                // and "E's volume integral" above
//                        if ( info.discrete_model.hasVelocityPressureFlux() ) {
					for ( int j = 0; j < info.numVelocityBaseFunctionsElement; ++j ) {
                        for ( int i = 0; i < info.numPressureBaseFunctionsElement; ++i ) {
							double E_i_j = 0.0;
                            for ( size_t quad = 0; quad < info.faceQuadratureElement.nop(); ++quad ) {
                                const auto x = info.faceQuadratureElement.point( quad );
                                const auto xLocal = info.faceQuadratureElement.localPoint( quad );
                                const double elementVolume = info.intersectionGeometry.integrationElement( xLocal );
                                const double integrationWeight = info.faceQuadratureElement.weight( quad );
								// compute \hat{u}_{p}^{U^{+}}(v_{j})\cdot n_{T}q_{i}
								const VelocityRangeType outerNormal = info.intersection.unitOuterNormal( xLocal );
								VelocityRangeType v_j( 0.0 );
								info.velocity_basefunction_set_element.evaluate( j, x, v_j );
								PressureRangeType q_i( 0.0 );
								info.pressure_basefunction_set_element.evaluate( i, x, q_i );
								VelocityRangeType flux_value = v_j;
								flux_value *= 0.5;
								const double v_j_times_outerNormal = v_j * outerNormal;
								VelocityRangeType jump = info.D_12;
								jump *= v_j_times_outerNormal;
								flux_value += jump;
								VelocityRangeType q_i_times_flux = flux_value;
								q_i_times_flux *= q_i;
								const double q_i_times_flux_times_outerNormal = q_i_times_flux * outerNormal;

								E_i_j += elementVolume
									* integrationWeight
									* q_i_times_flux_times_outerNormal;
                            }
							localEmatrixElement.add( i, j, E_i_j );
						} // done computing E's element surface integral

                        // compute E's neighbour surface integral
						for ( int i = 0; i < info.numPressureBaseFunctionsNeighbour; ++i ) {
							double E_i_j = 0.0;
                            for ( size_t quad = 0; quad < info.faceQuadratureNeighbour.nop(); ++quad ) {
                                const auto xInside = info.faceQuadratureElement.point( quad );
                                const auto xOutside = info.faceQuadratureNeighbour.point( quad );
                                const auto xLocal = info.faceQuadratureNeighbour.localPoint( quad );
                                const double elementVolume = info.intersectionGeometry.integrationElement( xLocal );
                                const double integrationWeight = info.faceQuadratureNeighbour.weight( quad );
								// compute \hat{u}_{p}^{U^{-}}(v_{j})\cdot n_{T}q_{i}
								const VelocityRangeType outerNormal = info.intersection.unitOuterNormal( xLocal );
								VelocityRangeType v_j( 0.0 );
								info.velocity_basefunction_set_element.evaluate( j, xInside, v_j );
								PressureRangeType q_i( 0.0 );
								info.pressure_basefunction_set_neighbour.evaluate( i, xOutside, q_i );

								VelocityRangeType flux_value = v_j;
								flux_value *= 0.5;
								const double v_j_times_outerNormal = v_j * outerNormal;
								VelocityRangeType jump = info.D_12;
								jump *= v_j_times_outerNormal;
								flux_value += jump;
								VelocityRangeType q_i_times_flux = flux_value;
								q_i_times_flux *= q_i;
								const double q_i_times_flux_times_outerNormal = q_i_times_flux * outerNormal;

								E_i_j -= elementVolume
									* integrationWeight
									* q_i_times_flux_times_outerNormal;
                            }
							localEmatrixNeighbour.add( i, j, E_i_j );
                        }
                    }
//                        }
			}

			template < class InfoContainerFaceType >
			void applyBoundaryFace( const InfoContainerFaceType&  )
			{}

			static const std::string name;
	};

	template < class T, class R > const std::string E<T,R>::name = "E";

} // end namespace Assembler
} // end namespace Oseen
} // end namespace Dune

#endif // DUNE_OSEEN_INTEGRATORS_E_HH

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

