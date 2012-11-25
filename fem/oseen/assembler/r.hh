#ifndef DUNE_OSEEN_INTEGRATORS_R_HH
#define DUNE_OSEEN_INTEGRATORS_R_HH

#include <dune/fem/oseen/assembler/base.hh>
#include <dune/stuff/matrix.hh>

namespace Dune {
namespace Oseen {
namespace Assembler {

	template < class MatrixPointerType, class Traits >
	class R
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
		typedef Stuff::Matrix::LocalMatrixProxy<MatrixPointerType>
			LocalMatrixProxyType;

		MatrixPointerType& matrix_pointer_;
		public:
			R( MatrixPointerType& matrix_object	)
				:matrix_pointer_(matrix_object)
			{}

			template < class InfoContainerVolumeType >
			void applyVolume( const InfoContainerVolumeType& /*info*/ )
			{}

			template < class InfoContainerInteriorFaceType >
			void applyInteriorFace( const InfoContainerInteriorFaceType& info )
			{
				LocalMatrixProxyType localRmatrixElement( matrix_pointer_, info.entity, info.entity, info.eps );
				LocalMatrixProxyType localRmatrixNeighbour( matrix_pointer_, info.entity, info.neighbour, info.eps );
				// (R)_{i,j} += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{u}_{p}^{P^{+}}(q_{j})\cdot n_{T}q_{i}ds // R's element surface integral
				//           += \int_{\varepsilon\in\Epsilon_{I}^{T}}\hat{u}_{p}^{P^{-}}(q_{j})\cdot n_{T}q_{i}ds // R's neighbour surface integral
				//                                                                                                // see also "R's boundary integral" below
//                        if ( info.discrete_model.hasVelocityPressureFlux() ) {
					for ( int j = 0; j < info.numPressureBaseFunctionsElement; ++j ) {
						// compute R's element surface integral
						for ( int i = 0; i < info.numPressureBaseFunctionsElement; ++i ) {
							double R_i_j = 0.0;
							// sum over all quadrature points
							for ( size_t quad = 0; quad < info.faceQuadratureElement.nop(); ++quad ) {
								// get x codim<0> and codim<1> coordinates
								const ElementCoordinateType x = info.faceQuadratureElement.point( quad );
								const LocalIntersectionCoordinateType xLocal = info.faceQuadratureElement.localPoint( quad );
								// get the integration factor
								const double elementVolume = info.intersectionGeometry.integrationElement( xLocal );
								// get the quadrature weight
								const double integrationWeight = info.faceQuadratureElement.weight( quad );
								// compute \hat{u}_{p}^{P^{+}}(q_{j})\cdot n_{T}q_{i}
//								const VelocityRangeType outerNormal = info.intersection.unitOuterNormal( xLocal );
								PressureRangeType q_i( 0.0 );
								info.pressure_basefunction_set_element.evaluate( i, x, q_i );
								PressureRangeType q_j( 0.0 );
								info.pressure_basefunction_set_element.evaluate( j, x, q_j );
								const double q_i_times_q_j = q_i * q_j;
								R_i_j += info.D_11
									* elementVolume
									* integrationWeight
									* q_i_times_q_j;
							} // done sum over all quadrature points
							localRmatrixElement.add( i, j, R_i_j );
						} // done computing R's element surface integral
						// compute R's neighbour surface integral
						for ( int i = 0; i < info.numPressureBaseFunctionsNeighbour; ++i ) {
							double R_i_j = 0.0;
							// sum over all quadrature points
							for ( size_t quad = 0; quad < info.faceQuadratureNeighbour.nop(); ++quad ) {
								// get x codim<0> and codim<1> coordinates
								const ElementCoordinateType xInside = info.faceQuadratureElement.point( quad );
								const ElementCoordinateType xOutside = info.faceQuadratureNeighbour.point( quad );
								const LocalIntersectionCoordinateType xLocal = info.faceQuadratureNeighbour.localPoint( quad );
								// get the integration factor
								const double elementVolume = info.intersectionGeometry.integrationElement( xLocal );
								// get the quadrature weight
								const double integrationWeight = info.faceQuadratureNeighbour.weight( quad );
								// compute \hat{u}_{p}^{P^{-}}(q_{j})\cdot n_{T}q_{i}
//								const VelocityRangeType outerNormal = info.intersection.unitOuterNormal( xLocal );
								PressureRangeType q_j( 0.0 );
								info.pressure_basefunction_set_neighbour.evaluate( j, xOutside, q_j );
								PressureRangeType q_i( 0.0 );
								info.pressure_basefunction_set_element.evaluate( i, xInside, q_i );
								const double q_i_times_q_j = q_i * q_j;
								R_i_j += -1.0
									* info.D_11
									* elementVolume
									* integrationWeight
									* q_i_times_q_j;
							} // done sum over all quadrature points
							localRmatrixNeighbour.add( i, j, R_i_j );
						} // done computing R's neighbour surface integral
					} // done computing R's surface integrals
//                        }
			}

			template < class InfoContainerFaceType >
			void applyBoundaryFace( const InfoContainerFaceType& )
			{}

			static const std::string name;
	};

	template < class T, class Y > const std::string R<T,Y>::name = "R";

} // end namespace Assembler
} // end namespace Oseen
} // end namespace Dune

#endif // DUNE_OSEEN_INTEGRATORS_R_HH

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

