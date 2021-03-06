#ifndef DUNE_OSEEN_INTEGRATORS_W_HH
#define DUNE_OSEEN_INTEGRATORS_W_HH

#include <dune/fem/oseen/assembler/base.hh>
#include <dune/stuff/common/matrix.hh>

namespace Dune {
namespace Oseen {
namespace Assembler {
    template < class MatrixObjectType, class Traits >
	class W
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
		typedef DSFe::LocalMatrixProxy<MatrixObjectType>
			LocalMatrixProxyType;

        MatrixObjectType& matrix_object_;
        public:
			W( MatrixObjectType& matrix_object	)
				:matrix_object_(matrix_object)
			{}

			template < class InfoContainerVolumeType >
			void applyVolume( const InfoContainerVolumeType& info )
			{
				LocalMatrixProxyType local_matrix( matrix_object_, info.entity, info.entity, info.eps );
				const double viscosity = info.discrete_model.viscosity();
                ASSERT_EQ( int(local_matrix.rows()), info.numSigmaBaseFunctionsElement );
                ASSERT_EQ( int(local_matrix.cols()), info.numVelocityBaseFunctionsElement );
				//                                                        // we will call this one
				// (W)_{i,j} += \mu\int_{T}v_{j}\cdot(\nabla\cdot\tau_{i})dx // W's volume integral
				//                                                        // see also "W's entitity surface integral", "W's neighbour surface integral" and "W's boundary integral" below
				for ( int i = 0; i < info.numSigmaBaseFunctionsElement; ++i ) {
					for ( int j = 0; j < info.numVelocityBaseFunctionsElement; ++j ) {
						double W_i_j = 0.0;
						for ( size_t quad = 0; quad < info.volumeQuadratureElement.nop(); ++quad ) {
							const auto x = info.volumeQuadratureElement.point( quad );
							const double elementVolume = info.geometry.integrationElement( x );
                            const double integrationWeight = info.volumeQuadratureElement.weight( quad );
							// compute v_j^t \cdot ( \nabla \cdot \tau_i^t )
							VelocityRangeType v_j( 0.0 );
							info.velocity_basefunction_set_element.evaluate( j, x, v_j );
							SigmaJacobianRangeType v_j_temp_for_div
									= prepareVelocityRangeTypeForSigmaDivergence<SigmaJacobianRangeType,VelocityRangeType>( v_j );
							const double divergence_of_tau_i_times_v_j
									= info.sigma_basefunction_set_element.evaluateGradientSingle( i, info.entity,
																								 x, v_j_temp_for_div );
							W_i_j += elementVolume
									* integrationWeight
									* viscosity
									* divergence_of_tau_i_times_v_j;
                        }
                        local_matrix.add( i, j, W_i_j );
					}
                }
			}

			template < class InfoContainerFaceType >
			void applyBoundaryFace( const InfoContainerFaceType& )
			{}

			template < class InfoContainerFaceType >
			void applyInteriorFace( const InfoContainerFaceType& info )
			{
				LocalMatrixProxyType localWmatrixElement( matrix_object_, info.entity, info.entity, info.eps );
				LocalMatrixProxyType localWmatrixNeighbour( matrix_object_, info.neighbour, info.entity, info.eps );

				//                                                                                                               // we will call this one
				// (W)_{i,j} += \int_{\varepsilon\in \Epsilon_{I}^{T}}-\hat{u}_{\sigma}^{U^{+}}(v_{j})\cdot\tau_{i}\cdot n_{T}ds // W's element surface integral
				//           += \int_{\varepsilon\in \Epsilon_{I}^{T}}-\hat{u}_{\sigma}^{U^{-}}(v_{j})\cdot\tau_{i}\cdot n_{T}ds // W's neighbour surface integral
				//                                                                                                               // see also "W's boundary integral" below
				//                                                                                                               // and "W's volume integral" above
				//                        if ( info.discrete_model.hasVelocitySigmaFlux() ) {
				for ( size_t quad = 0; quad < info.faceQuadratureElement.nop(); ++quad ) {
                    const auto xInside = info.faceQuadratureElement.point( quad );
					const auto xLocal = info.faceQuadratureElement.localPoint( quad );
					const auto xOutside = info.faceQuadratureNeighbour.point( quad );
                    const double elementVolume = info.intersectionGeometry.integrationElement( xLocal );
                    const double integrationWeight = info.faceQuadratureElement.weight( quad );
					// compute \hat{u}_{\sigma}^{U^{+}}(v_{j})\cdot\tau_{j}\cdot n_{T}
					const VelocityRangeType outerNormal = info.intersection.unitOuterNormal( xLocal );
					for ( int j = 0; j < info.numVelocityBaseFunctionsElement; ++j ) {
                        VelocityRangeType v_j( 0.0 );
						info.velocity_basefunction_set_element.evaluate( j, xInside, v_j );
						VelocityJacobianRangeType v_j_dyadic_normal
								= DSC::dyadicProduct<VelocityJacobianRangeType,VelocityRangeType>( v_j, outerNormal );
						VelocityRangeType v_j_dyadic_normal_times_C12( 0.0 );
						typename Traits::C12 c_12( outerNormal, info.discrete_model.getStabilizationCoefficients() );
						v_j_dyadic_normal.mv( c_12, v_j_dyadic_normal_times_C12 );
						VelocityRangeType flux_value = v_j;
						flux_value *= 0.5;
						flux_value -= v_j_dyadic_normal_times_C12;

						SigmaRangeType tau_i( 0.0 );
						VelocityRangeType tau_i_times_normal( 0.0 );
						for ( int i = 0; i < info.numSigmaBaseFunctionsElement; ++i ) {
							info.sigma_basefunction_set_element.evaluate( i, xInside, tau_i );
							tau_i.mv( outerNormal, tau_i_times_normal );

							double flux_times_tau_i_times_normal = flux_value * tau_i_times_normal;
							const double W_i_j = -elementVolume
									* integrationWeight
									* info.viscosity
									* flux_times_tau_i_times_normal;
							localWmatrixElement.add( i, j, W_i_j );
						}

						for ( int i = 0; i < info.numSigmaBaseFunctionsNeighbour; ++i ) {
							info.sigma_basefunction_set_neighbour.evaluate( i, xOutside, tau_i );
							tau_i.mv( outerNormal, tau_i_times_normal );

							double flux_times_tau_i_times_normal = flux_value * tau_i_times_normal;
							const double W_i_j = elementVolume
									* integrationWeight
									* info.viscosity
									* flux_times_tau_i_times_normal;
							localWmatrixNeighbour.add( i, j, W_i_j );
                        }
                    }
                }
			}
			static const std::string name;
	};

	template < class T, class R > const std::string W<T,R>::name = "W";

} // end namespace Assembler
} // end namespace Oseen
} // end namespace Dune

#include <dune/fem/oseen/assembler/rhs.hh>

#endif // DUNE_OSEEN_INTEGRATORS_W_HH

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

