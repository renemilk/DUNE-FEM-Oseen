#ifndef DUNE_OSEEN_INTEGRATORS_Y_HH
#define DUNE_OSEEN_INTEGRATORS_Y_HH

#include <dune/fem/oseen/assembler/base.hh>
#include <dune/stuff/common/matrix.hh>

namespace Dune {
namespace Oseen {
namespace Assembler {

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
		typedef DSFe::LocalMatrixProxy<MatrixObjectType>
			LocalMatrixProxyType;

		MatrixObjectType& matrix_object_;
		public:
			Y( MatrixObjectType& matrix_object	)
				:matrix_object_(matrix_object)
			{}

			template < class InfoContainerVolumeType >
			void applyVolume( const InfoContainerVolumeType& info )
			{
                LocalMatrixProxyType localYmatrixElement(matrix_object_, info.entity, info.entity, info.eps);
//                if ( info.discrete_model.isGeneralized() )
				{
				for ( int i = 0; i < info.numVelocityBaseFunctionsElement; ++i ) {
					for ( int j = 0; j < info.numVelocityBaseFunctionsElement; ++j ) {
						double Y_i_j = 0.0;
                        for ( size_t quad = 0; quad < info.volumeQuadratureElement.nop(); ++quad ) {
                            const auto x = info.volumeQuadratureElement.point( quad );
                            const double elementVolume = info.geometry.integrationElement( x );
                            const double integrationWeight = info.volumeQuadratureElement.weight( quad );
							// compute \tau_{j}:\nabla v_{i}
							VelocityRangeType v_i( 0.0 );
							info.velocity_basefunction_set_element.evaluate( i, x, v_i );
							VelocityRangeType v_j( 0.0 );
							info.velocity_basefunction_set_element.evaluate( j, x, v_j );
							const double v_i_times_v_j = v_i * v_j;
							Y_i_j += elementVolume
								* integrationWeight
								* info.alpha
								* v_i_times_v_j;
                        }
                        localYmatrixElement.add( i, j, Y_i_j );
					}
                }
				}
			}

			template < class InfoContainerInteriorFaceType >
			void applyInteriorFace( const InfoContainerInteriorFaceType& info )
			{
				LocalMatrixProxyType localYmatrixElement( matrix_object_, info.entity, info.entity, info.eps );
				LocalMatrixProxyType localYmatrixNeighbour( matrix_object_, info.neighbour, info.entity, info.eps );
				// (Y)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U{+}}(v{j})\cdot n_{t}ds // Y's element surface integral
				//           += \int_{\varepsilon\in\Epsilon_{I}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U{-}}(v{j})\cdot n_{t}ds // Y's neighbour surface integral
				//                                                                                                         // see also "Y's boundary integral" below
//                        if ( info.discrete_model.hasSigmaFlux() ) {
					for ( int j = 0; j < info.numVelocityBaseFunctionsElement; ++j ) {
                        for ( int i = 0; i < info.numVelocityBaseFunctionsElement; ++i ) {
							double Y_i_j = 0.0;
                            for ( size_t quad = 0; quad < info.faceQuadratureElement.nop(); ++quad ) {
                                const auto x = info.faceQuadratureElement.point( quad );
								const auto xLocal = info.faceQuadratureElement.localPoint( quad );
                                const double elementVolume = info.intersectionGeometry.integrationElement( xLocal );
                                const double integrationWeight = info.faceQuadratureElement.weight( quad );
								// compute -\mu v_{i}\cdot\hat{\sigma}^{U{+}}(v{j})\cdot n_{t}
								VelocityRangeType v_j( 0.0 );
								info.velocity_basefunction_set_element.evaluate( j, x, v_j );
								VelocityRangeType v_i( 0.0 );
								info.velocity_basefunction_set_element.evaluate( i, x, v_i );
								const double v_i_times_v_j = v_i * v_j;
								Y_i_j += info.C_11
									* elementVolume
									* integrationWeight
									* v_i_times_v_j;
                            }
							localYmatrixElement.add( i, j, Y_i_j );
                        }
						// compute Y's neighbour surface integral
						for ( int i = 0; i < info.numVelocityBaseFunctionsNeighbour; ++i ) {
							double Y_i_j = 0.0;
                            for ( size_t quad = 0; quad < info.faceQuadratureNeighbour.nop(); ++quad ) {
                                const auto xInside = info.faceQuadratureElement.point( quad );
								const auto xOutside = info.faceQuadratureNeighbour.point( quad );
								const auto xLocal = info.faceQuadratureNeighbour.localPoint( quad );
                                const double elementVolume = info.intersectionGeometry.integrationElement( xLocal );
                                const double integrationWeight = info.faceQuadratureNeighbour.weight( quad );
								// compute -\mu v_{i}\cdot\hat{\sigma}^{U{-}}(v{j})\cdot n_{t}
								VelocityRangeType v_i( 0.0 );
								info.velocity_basefunction_set_neighbour.evaluate( i, xOutside, v_i );
								VelocityRangeType v_j( 0.0 );
								info.velocity_basefunction_set_element.evaluate( j, xInside, v_j );
								const double v_i_times_v_j = v_i * v_j;
								Y_i_j += -1.0
									* info.C_11
									* elementVolume
									* integrationWeight
									* v_i_times_v_j;
                            }
							localYmatrixNeighbour.add( i, j, Y_i_j );
                        }
                    }
//                        }

			}

			template < class InfoContainerFaceType >
			void applyBoundaryFace( const InfoContainerFaceType& info )
			{
				LocalMatrixProxyType localYmatrixElement( matrix_object_, info.entity, info.entity, info.eps );
				// (Y)_{i,j} += \int_{\varepsilon\in\Epsilon_{D}^{T}}-\mu v_{i}\cdot\hat{\sigma}^{U^{+}}(v_{j})\cdot n_{t}ds // Y's boundary integral
				//                                                                                                           // see also "Y's element surface integral" and "Y's neighbour surface integral" above
//                        if ( info.discrete_model.hasSigmaFlux() ) {
					for ( int i = 0; i < info.numVelocityBaseFunctionsElement; ++i ) {
						for ( int j = 0; j < info.numVelocityBaseFunctionsElement; ++j ) {
							double Y_i_j = 0.0;
                            for ( size_t quad = 0; quad < info.faceQuadratureElement.nop(); ++quad ) {
                                const auto x = info.faceQuadratureElement.point( quad );
								const auto xLocal = info.faceQuadratureElement.localPoint( quad );
                                const double elementVolume = info.intersectionGeometry.integrationElement( xLocal );
                                const double integrationWeight = info.faceQuadratureElement.weight( quad );
								// compute -\mu v_{i}\cdot\hat{\sigma}^{U^{+}}(v_{j})\cdot n_{t}
								VelocityRangeType v_j( 0.0 );
								info.velocity_basefunction_set_element.evaluate( j, x, v_j );
								VelocityRangeType v_i( 0.0 );
								info.velocity_basefunction_set_element.evaluate( i, x, v_i );
								const double v_i_times_v_j = v_i * v_j;
								Y_i_j += info.C_11
									* elementVolume
									* integrationWeight
									* v_i_times_v_j;
                            }
							localYmatrixElement.add( i, j, Y_i_j );
						}
                    }
//                        }

			}
			static const std::string name;
	};

	template < class T, class R > const std::string Y<T,R>::name = "Y";

} // end namespace Assembler
} // end namespace Oseen
} // end namespace Dune

#endif // DUNE_OSEEN_INTEGRATORS_Y_HH

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

