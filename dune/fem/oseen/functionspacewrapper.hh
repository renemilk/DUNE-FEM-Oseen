/**
 *  \file   discretestokesfunctionspacewrapper.hh
 *  \todo   doc
 **/

#ifndef DISCRETESTOKESFUNCTIONSPACEWRAPPER_HH_INCLUDED
#define DISCRETESTOKESFUNCTIONSPACEWRAPPER_HH_INCLUDED

#include <dune/fem/space/common/discretefunctionspace.hh>
#include <dune/fem/space/common/restrictprolonginterface.hh>
#include <dune/fem/io/file/vtkio.hh>
#include <dune/fem/operator/projection/l2projection.hh>
#include <dune/fem/oseen/adaption.hh>

namespace Dune
{

// forward
template < class DiscreteOseenFunctionSpaceWrapperTraitsImp >
class DiscreteOseenFunctionSpaceWrapper;

/**
 *  \todo   doc
 **/
template < class DiscreteVelocitySpaceImp, class DiscretePressureSpaceImp >
class DiscreteOseenFunctionSpaceWrapperTraits
{
    public:

        //! type of discrete velocity function space
        typedef DiscreteVelocitySpaceImp
            DiscreteVelocityFunctionSpaceType;

        //! type of discrete pressure function space
        typedef DiscretePressureSpaceImp
            DiscretePressureFunctionSpaceType;

        /**
         *  \name for interface compliance
         *  \{
         **/
        //! own type (CRTP) (for interface compliance)
        typedef DiscreteOseenFunctionSpaceWrapper<
                    DiscreteOseenFunctionSpaceWrapperTraits<   DiscreteVelocityFunctionSpaceType,
                                                                DiscretePressureFunctionSpaceType > >
            DiscreteFunctionSpaceType;

        //! type of function space
        typedef typename DiscreteVelocityFunctionSpaceType::FunctionSpaceType
            FunctionSpaceType;

        //! type of base function set
        typedef typename DiscreteVelocityFunctionSpaceType::BaseFunctionSetType
            BaseFunctionSetType;

        //! type of DoF mapper
        typedef typename DiscreteVelocityFunctionSpaceType::MapperType
            MapperType;

        //! type of block mapper
        typedef typename DiscreteVelocityFunctionSpaceType::BlockMapperType
            BlockMapperType;

        //! type of underlying grid part
        typedef typename DiscreteVelocityFunctionSpaceType::GridPartType
            GridPartType;

		template< class DiscreteFunction,
				  class Operation =  // get default type from traits
				  typename DiscreteVelocityFunctionSpaceType::Traits:: template CommDataHandle< DiscreteFunction > :: OperationType
				>
		//! TODO this probably isn't too sane to do...
		struct CommDataHandle
		{
		  //! type of communication data handle
		  typename DiscreteVelocityFunctionSpaceType::Traits
			:: template CommDataHandle< DiscreteFunction, Operation > :: Type
			Type;

		  //! type of operation to perform on scatter
		  typename DiscreteVelocityFunctionSpaceType::Traits
			:: template CommDataHandle< DiscreteFunction, Operation > :: OperationType
			OperationType;
		};

		//! type of communication manager
		typedef CommunicationManager< DiscreteFunctionSpaceType >
				CommunicationManagerType;
        /**
         *  \}
         **/
}; // end of DiscreteOseenFunctionSpaceWrapperTraits

/**
 *  \todo   doc
 **/
template < class DiscreteOseenFunctionSpaceWrapperTraitsImp >
class DiscreteOseenFunctionSpaceWrapper
    : public DiscreteFunctionSpaceDefault< DiscreteOseenFunctionSpaceWrapperTraitsImp >
{
    public:

        typedef DiscreteOseenFunctionSpaceWrapperTraitsImp
            Traits;

        //! base type
        typedef DiscreteFunctionSpaceDefault< DiscreteOseenFunctionSpaceWrapperTraitsImp >
            BaseType;

        //! type of discrete velocity function space
        typedef typename Traits::DiscreteVelocityFunctionSpaceType
            DiscreteVelocityFunctionSpaceType;

        //! type of discrete pressure function space
        typedef typename Traits::DiscretePressureFunctionSpaceType
            DiscretePressureFunctionSpaceType;

        //! own type (CRTP)
        typedef typename BaseType::DiscreteFunctionSpaceType
            DiscreteFunctionSpaceType;

        //! type of function space (of the discrete velocity function space)
        typedef typename BaseType::FunctionSpaceType
            FunctionSpaceType;

        //! type of base function set (of the discrete velocity function space)
        typedef typename BaseType::BaseFunctionSetType
            BaseFunctionSetType;

        //! type of DoF mapper (of the discrete velocity function space)
        typedef typename BaseType::MapperType
            MapperType;

        //! type of block mapper (of the discrete velocity function space)
        typedef typename BaseType::BlockMapperType
            BlockMapperType;

        //! type of underlying grid part (of the discrete velocity function space)
        typedef typename BaseType::GridPartType
            GridPartType;

        //! type of underlying grid (of the discrete velocity function space)
        typedef typename GridPartType::GridType
            GridType;

        //! type of used index set (of the discrete velocity function space)
        typedef typename GridPartType::IndexSetType
            IndexSetType;

        //! type of iterator of codim 0 entities for grid traversal (of the discrete velocity function space)
        typedef typename GridPartType::template Codim< 0 >::IteratorType
            IteratorType;

        //! type of codim 0 entity (of the discrete velocity function space)
        typedef typename IteratorType::Entity
            EntityType;

        /**
         *  \brief  constructor
         *  \todo   doc
         **/
        DiscreteOseenFunctionSpaceWrapper( GridPartType& gridPart_in )
            : BaseType( gridPart_in ),
            velocitySpace_( gridPart_in ),
            pressureSpace_( gridPart_in )
        {}

        /**
         *  \brief  destructor
         **/
		virtual ~DiscreteOseenFunctionSpaceWrapper()
        {}

        /**
         *  \brief  returns the discrete velocity space
         *  \todo   doc
         **/
        DiscreteVelocityFunctionSpaceType& discreteVelocitySpace()
        {
            return velocitySpace_;
        }
        const DiscreteVelocityFunctionSpaceType& discreteVelocitySpace() const
        {
            return velocitySpace_;
        }

        /**
         *  \brief  return the discrete pressure space
         *  \todo   doc
         **/
        DiscretePressureFunctionSpaceType& discretePressureSpace()
        {
            return pressureSpace_;
        }
        const DiscretePressureFunctionSpaceType& discretePressureSpace() const
        {
            return pressureSpace_;
        }

        /**
         *  \brief  return type identifier of the discrete velocity function space
         **/
        DFSpaceIdentifier type() const
        {
            return velocitySpace_.type();
        }

        /**
         *  \brief  get base function set of the discrete velocity function space for given entity
         *  \todo   doc
         **/
        inline const BaseFunctionSetType baseFunctionSet( const EntityType& entity ) const
        {
            return velocitySpace_.baseFunctionSet( entity );
        }

        /**
         *  \brief  returns true if the discrete velocity function space contains DoFs of given codimension
         *  \todo   doc
         **/
        inline bool contains( const int codim ) const
        {
            return velocitySpace_.contains( codim );
        }

        /**
         *  \brief  returns true if the discrete velocity function space contains only globally continuous functions
         *  \todo   doc
         **/
        inline bool continuous() const
        {
            return velocitySpace_.continuous();
        }

        /**
         *  \brief  get index of the sequence in the discrete velocity function spaces grid sequences
         *  \todo   doc
         **/
        inline int sequence() const
        {
            return velocitySpace_.sequence();
        }

        /**
         *  \brief  get global order of the discrete velocity function space
         *  \todo doc
         **/
        inline int order() const
        {
            return velocitySpace_.order();
        }

        /**
         *  \brief  get a reference to the discrete velocity function spacees DoF mapper
         *  \todo   doc
         **/
        inline MapperType& mapper() const
        {
            return velocitySpace_.mapper();
        }

        /**
         *  \brief  get a reference to the discrete velocity function spaces block mapper
         *  \todo   doc
         **/
        inline BlockMapperType& blockMapper() const
        {
            return velocitySpace_.blockMapper();
        }

        /**
         *  \brief  get reference to grid the discrete velocity function space belongs to
         *  \todo   doc
         **/
        inline const GridType& grid() const
        {
            return velocitySpace_.grid();
        }

        /**
         *  \brief  get reference to grid the discrete velocity function space belongs to
         *  \todo   doc
         **/
        inline GridType& grid()
        {
            return velocitySpace_.grid();
        }

        /**
         *  \brief  get a reference to the discrete velocity function space grid partition
         *  \todo   doc
         **/
        inline const GridPartType& gridPart() const
        {
            return velocitySpace_.gridPart();
        }

        /**
         *  \brief  get a reference to the discrete velocity function space grid partition
         *  \todo   doc
         **/
        inline GridPartType& gridPart()
        {
            return velocitySpace_.gridPart();
        }

        /**
         *  \brief  get a reference to the discrete velocity function space index set
         *  \todo   doc
         **/
        inline const IndexSetType& indexSet() const
        {
            return velocitySpace_.indexSet();
        }

        /**
         *  \brief  get number of DoFs for the discrete velocity function space
         *  \todo   doc
         **/
        inline int size() const
        {
            return velocitySpace_.size();
        }

        /**
         *  \brief  get discrete velocity function space iterator pointing to the first entity of the associated grid partition
         *  \todo   doc
         **/
        inline IteratorType begin() const
        {
            return velocitySpace_.begin();
        }

        /**
         *  \brief  get discrete velocity function space iterator pointing behind the last entity of the associated grid partition
         *  \todo   doc
         **/
        inline IteratorType end() const
        {
            return velocitySpace_.end();
        }

        /**
         *  \brief  apply discrete velocity function space functor to each entity in the associated grid partition
         *  \todo   doc
         **/
        template< class FunctorType >
        inline void forEach( FunctorType& f ) const
        {
            return velocitySpace_.forEach( f );
        }

        /**
         *  \brief  returns true if the discrete velocity function space grid has more than one geometry type
         *  \todo   doc
         **/
        inline bool multipleGeometryTypes() const
        {
            return velocitySpace_.multipleGeometryTypes();
        }

        /**
         *  \brief  returns true if discrete velocity fucntion space base function sets depend on the entity
         *  \todo   doc
         **/
        inline bool multipleBaseFunctionSets() const
        {
            return velocitySpace_.multipleBaseFunctionSets();
        }

        /**
         *  \brief  map local DoF number to global DoF number (discrete velocity function space)
         *  \todo   doc
         **/
        inline int mapToGlobal( const EntityType& entity,
                                const int localDof ) const
        {
            return velocitySpace_.mapToGlobal( entity, localDof );
        }

        /**
         *  \brief  return maximal number of local DoFs in discrete velocity funtion space
         *  \todo   doc
         **/
        inline int maxNumLocalDofs() const
        {
            return velocitySpace_.maxNumLocalDofs();
        }

        /**
         *  \brief  creates DataHandle for given discrete function (from dicrete velocity function space)
         *  \todo   doc
         **/
        template< class DiscreteFunction, class Operation >
        inline typename BaseType::template CommDataHandle< DiscreteFunction, Operation >::Type createDataHandle(    DiscreteFunction& discreteFunction,
                                                                                                                    const Operation* operation ) const
        {
            return velocitySpace_.createDataHandle( discreteFunction,
                                                    operation );
        }

    private:

        DiscreteVelocityFunctionSpaceType velocitySpace_;
        DiscretePressureFunctionSpaceType pressureSpace_;


}; // end of DiscreteOseenFunctionSpaceWrapper

//! forward
template < class DiscreteOseenFunctionWrapperTraitsImp >
class DiscreteOseenFunctionWrapper;

/**
 *  \todo   doc
 **/
template <  class DiscreteOseenFunctionSpaceWrapperImp,
            class DiscreteVelocityFunctionImp,
            class DiscretePressureFunctionImp >
class DiscreteOseenFunctionWrapperTraits
{
    public:

        //! own type (CRTP)
        typedef DiscreteOseenFunctionWrapper<
                    DiscreteOseenFunctionWrapperTraits<
                        DiscreteOseenFunctionSpaceWrapperImp,
                        DiscreteVelocityFunctionImp,
                        DiscretePressureFunctionImp > >
            DiscreteFunctionType;

        //! type of associated discrete function space
        typedef DiscreteOseenFunctionSpaceWrapperImp
            DiscreteFunctionSpaceType;

        //! type of discrete velocity function
        typedef DiscreteVelocityFunctionImp
            DiscreteVelocityFunctionType;

        //! type of discrete pressure function
        typedef DiscretePressureFunctionImp
            DiscretePressureFunctionType;

        typedef SubsamplingVTKIO < typename DiscretePressureFunctionType::DiscreteFunctionSpaceType::GridPartType >
			VtkWriterType;

        typedef Dune::tuple<const DiscreteVelocityFunctionType*,const DiscretePressureFunctionType*>
			FunctionTupleType;

}; // end of DiscreteOseenFunctionWrapperTraits

/**
 *  \todo   doc,
 *          should comply with the DiscreteFunctionInterface some time
 **/
template < class DiscreteOseenFunctionWrapperTraitsImp >
class DiscreteOseenFunctionWrapper
//    : public DiscreteFunctionInterface< DiscreteOseenFunctionWrapperTraitsImp >
{
    public:

        typedef DiscreteOseenFunctionWrapperTraitsImp
            Traits;
		typedef DiscreteOseenFunctionWrapper<Traits>
			ThisType;

        typedef typename Traits::DiscreteFunctionType
            DiscreteFunctionType;

        //! DiscreteOseenFunctionSpaceWrapper
        typedef typename Traits::DiscreteFunctionSpaceType
            DiscreteFunctionSpaceType;

        //! type of discrete velocity function
        typedef typename Traits::DiscreteVelocityFunctionType
            DiscreteVelocityFunctionType;

        //! type of discrete pressure function
        typedef typename Traits::DiscretePressureFunctionType
            DiscretePressureFunctionType;

        //! type of range field
        typedef typename DiscreteVelocityFunctionType::RangeFieldType
            RangeFieldType;
		typedef typename DiscreteFunctionSpaceType::GridPartType
			GridPartType;
		typedef typename GridPartType::GridType
			GridType;


    protected:

		typedef DiscreteOseenFunctionWrapperAdaptionManager< ThisType >
            AdaptionManagerType;

    public:

        /**
         *  \brief  constructor
         *  \todo   doc
         **/
        DiscreteOseenFunctionWrapper(  const std::string name_in,
                                        DiscreteFunctionSpaceType& space_in,
                                        GridPartType& gridPart_in )
            : space_( space_in )
            , velocity_( name_in + std::string("_velocity"), space_in.discreteVelocitySpace() )
            , pressure_( name_in + std::string("_pressure"), space_in.discretePressureSpace() )
            , adaptionManager_ ( gridPart_in.grid(), *this )
        {}

        /**
         *  \brief      constructor
         *
         *              wraps existing velocity and pressure
         *  \attention  by copying
         **/
        DiscreteOseenFunctionWrapper(  DiscreteFunctionSpaceType& space_in,
                                        DiscreteVelocityFunctionType& velocity_in,
                                        DiscretePressureFunctionType& pressure_in )
            : space_( space_in )
            , velocity_( velocity_in )
            , pressure_( pressure_in )
            , adaptionManager_ ( pressure_in.space().gridPart().grid(), *this )
        {}

        /**
         *  \todo   doc
         **/
        const DiscreteVelocityFunctionType& discreteVelocity() const
        {
            return velocity_;
        }

        /**
         *  \todo   doc
         **/
        DiscreteVelocityFunctionType& discreteVelocity()
        {
            return velocity_;
        }

        /**
         *  \todo   doc
         **/
        const DiscretePressureFunctionType& discretePressure() const
        {
            return pressure_;
        }

        /**
         *  \todo   doc
         **/
        DiscretePressureFunctionType& discretePressure()
        {
            return pressure_;
        }

        /**
         *  \brief  obtain the name of the discrete function
         *  \todo   doc
         **/
        inline const std::string& name() const
        {
            return velocity_.name();
        }

		//! get the grid from velo space (we're assuming same grid for both functions everywhere anyways)
        const typename DiscreteFunctionSpaceType::GridType& grid() const {
            return velocity_.space().grid();
        }

        /**
         *  \brief  set all degrees of freedom to zero
         *  \todo   doc
         **/
        inline void clear()
        {
            velocity_.clear();
        }

        /**
         *  \brief  obtain total number of DoFs
         *  \todo   doc
         **/
        inline int size() const
        {
            return velocity_.size();
        }

        /**
         *  \brief  add another discrete function to this one
         *  \todo   doc
         **/
        inline DiscreteFunctionType& operator+= ( const DiscreteFunctionType& arg )
        {
            velocity_ += arg.discreteVelocity();
			pressure_ += arg.discretePressure();
            return *this;
        }

        /**
         *  \brief  substract all degrees of freedom from given discrete function using the dof iterators
         *  \todo   doc
         **/
        template < class DFType >
        DiscreteFunctionType& operator-=( const DFType& arg )
        {
            velocity_ -= arg.discreteVelocity();
			pressure_ -= arg.discretePressure();
            return *this;
        }

        /**
         *  \brief  multiply all DoFs by a scalar factor
         *  \todo   doc
         **/
        inline DiscreteFunctionType& operator*=( const RangeFieldType& scalar )
        {
            velocity_ *= scalar;
			pressure_ *= scalar;
            return *this;
        }

        /**
         *  \brief  devide all DoFs by a scalar factor
         *  \todo   doc
         **/
        inline DiscreteFunctionType& operator/=( const RangeFieldType& scalar )
        {
            velocity_ /= scalar;
			pressure_ /= scalar;
            return *this;
        }

		/**
         *  \brief  discreteFunction/grid adaption
         *  this will both refine/coarsen all marked grid entities and adapt both member functions to that new grid layout
         **/
        void adapt()
        {
            adaptionManager_.adapt();
        }

		void writeVTK( const std::string& path, const int number_postfix )
		{
            std::stringstream s;
            s << std::setfill('0') << std::setw(6) << number_postfix;
            writeVTK( path, s.str() );
		}

		//! write both wrapped functions to "path/{pressure,velocity}.name()+postfix+.vtk"
		void writeVTK( const std::string& path, const std::string postfix = std::string() )
		{
            typename Traits::VtkWriterType vtkWriter(velocity_.space().gridPart());
            if ( DiscreteVelocityFunctionType::FunctionSpaceType::DimRange > 1 ){
                vtkWriter.addVectorVertexData( velocity_ );
                vtkWriter.addVectorCellData( velocity_ );
            }
            else {
                vtkWriter.addVertexData( velocity_ );
                vtkWriter.addCellData( velocity_ );
            }

            vtkWriter.write( getPath( velocity_, path, postfix ) );
            vtkWriter.clear();

            if ( DiscretePressureFunctionType::FunctionSpaceType::DimRange > 1 ){
                vtkWriter.addVectorVertexData( pressure_ );
                vtkWriter.addVectorCellData( pressure_ );
            }
            else {
                vtkWriter.addVertexData( pressure_ );
                vtkWriter.addCellData( pressure_ );
            }
            vtkWriter.write( getPath( pressure_, path, postfix ) );
            vtkWriter.clear();
		}

		typename Traits::FunctionTupleType& functionTuple() const
		{
			static typename Traits::FunctionTupleType tuple( &velocity_, &pressure_ );
			return tuple;
		}

		template < class ContinuousVelocityType, class ContinuousPressureType >
		void projectInto( const ContinuousVelocityType& continuous_velocity, const ContinuousPressureType& continuous_pressure )
		{
			typedef Dune::L2Projection< double, double, ContinuousPressureType, DiscretePressureFunctionType >
				PressureProjection;
			PressureProjection().operator()( continuous_pressure, pressure_ );

			typedef Dune::L2Projection< double, double, ContinuousVelocityType, DiscreteVelocityFunctionType >
				VelocityProjection;
			VelocityProjection().operator()( continuous_velocity, velocity_ );
		}

		void assign( const ThisType& other )
		{
			velocity_.assign( other.discreteVelocity() );
			pressure_.assign( other.discretePressure() );
		}

		const DiscreteFunctionSpaceType& space() const
		{
			return space_;
		}

		DiscreteFunctionSpaceType& space()
		{
			return space_;
		}

    private:
		template <class FunctionType>
		inline const char* getPath( const FunctionType& f, const std::string& base_path, const std::string& postfix ) const
		{
			std::stringstream ss;
			ss << base_path;
			if ( base_path.at(base_path.size()-1) != '/' )
				ss << '/';
			ss << f.name() << postfix;
			return ss.str().c_str();
		}

		DiscreteFunctionSpaceType& space_;
        DiscreteVelocityFunctionType velocity_;
        DiscretePressureFunctionType pressure_;
		AdaptionManagerType adaptionManager_;

		// we can uncomment this if the adpation manager copy-problem is resolved
		//DiscreteOseenFunctionWrapper( const DiscreteOseenFunctionWrapper& );

}; // end of DiscreteOseenFunctionWrapper

} // end of namespace Dune

#endif // end of discretestokesfunctionspacewrapper.hh

/** Copyright (c) 2012, Felix Albrecht, Rene Milk      
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

