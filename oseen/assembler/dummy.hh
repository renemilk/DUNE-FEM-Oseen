#ifndef DUNE_OSEEN_INTEGRATORS_DUMMY_HH
#define DUNE_OSEEN_INTEGRATORS_DUMMY_HH

#include <dune/oseen/assembler/base.hh>

namespace Dune {
namespace Oseen {
namespace Assembler {

	template < class MatrixObjectType, class Traits >
	class Dummy
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
			Dummy( MatrixObjectType& matrix_object	)
				:matrix_object_(matrix_object)
			{}

			template < class InfoContainerVolumeType >
			void applyVolume( const InfoContainerVolumeType& info )
			{}

			template < class InfoContainerInteriorFaceType >
			void applyInteriorFace( const InfoContainerInteriorFaceType& info )
			{
				typename MatrixObjectType::LocalMatrixType
						localWmatrixElement = matrix_object_.localMatrix( info.entity, info.entity );
				typename MatrixObjectType::LocalMatrixType
						localWmatrixNeighbour = matrix_object_.localMatrix( info.neighbour, info.entity );
			}

			template < class InfoContainerFaceType >
			void applyBoundaryFace( const InfoContainerFaceType& info )
			{
				typename MatrixObjectType::LocalMatrixType
						localWmatrixElement = matrix_object_.localMatrix( info.entity, info.entity );
			}
	};

} // end namespace Assembler
} // end namespace Oseen
} // end namespace Dune


#endif // DUNE_OSEEN_INTEGRATORS_DUMMY_HH
