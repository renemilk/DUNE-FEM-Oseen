/** \file stokespass.hh
    \brief  stokespass.hh
 **/

#ifndef STOKESPASS_HH
#define STOKESPASS_HH

#include <cmake_config.h>

#include <dune/fem/pass/pass.hh>
#include <dune/fem/oseen/assembler/ported_matrixobject.hh>
#include <dune/fem/space/dgspace.hh>

#include <dune/fem/oseen/defaulttraits.hh>
#include <dune/fem/oseen/datacontainer.hh>
#include <dune/fem/oseen/solver/solvercaller.hh>
#include <dune/fem/oseen/assembler/all.hh>
#include <dune/fem/oseen/assembler/factory.hh>
#include <dune/fem/oseen/runinfo.hh>

#include <dune/stuff/fem/customprojection.hh>
#include <dune/stuff/common/matrix.hh>
#include <dune/stuff/common/tuple.hh>
#ifndef NLOG
#   include <dune/stuff/common/print.hh>
#   include <dune/stuff/common/misc.hh>
#   include <dune/stuff/common/logging.hh>
#endif
#include <dune/stuff/grid/entity.hh>
#include <dune/stuff/common/profiler.hh>

namespace Dune {

/**
 *  \brief  OseenLDGMethod
 *  \todo   doc
 **/
template <  class DiscreteModelImp,
		  template <class> class TraitsImp = StokesTraits >
class OseenLDGMethod
{
    public:
        typedef OseenLDGMethod< DiscreteModelImp, TraitsImp >
            ThisType;
        typedef DiscreteModelImp
            DiscreteModelType;
        typedef TraitsImp< DiscreteModelType >
            Traits;
        typedef typename Traits::DiscreteOseenFunctionWrapperType
            DomainType;
        typedef typename Traits::DiscreteOseenFunctionWrapperType
            RangeType;

        //!
        OseenLDGMethod( DiscreteModelType discreteModel,
					typename Traits::GridPartType& gridPart,
                    const typename Traits::DiscreteOseenFunctionSpaceWrapperType& spaceWrapper,
                    const typename Traits::DiscreteVelocityFunctionType beta,
					const bool do_oseen_discretization )//! \todo move to model
            : discreteModel_( discreteModel ),
            gridPart_( gridPart ),
            spaceWrapper_( spaceWrapper ),
            velocitySpace_( spaceWrapper.discreteVelocitySpace() ),
            pressureSpace_( spaceWrapper.discretePressureSpace() ),
			sigmaSpace_( gridPart ),
			beta_( beta ),
			do_oseen_discretization_( do_oseen_discretization ),
	      info_( SaddlepointInverseOperatorInfo() )
        {}

        //! used in Postprocessing to get refs to gridparts, spaces
        const typename Traits::DiscreteOseenFunctionSpaceWrapperType& GetFunctionSpaceWrapper()
        {
            return spaceWrapper_;
        }

        /**
         *  \todo doc
         **/
        template < class OtherTraitsImp = Traits  >
        void apply( const DomainType &arg, RangeType &dest, Dune::Oseen::RhsDatacontainer<OtherTraitsImp>* rhs_datacontainer = nullptr )
        {
            // profiler information
            DSC_PROFILER.startTiming("Pass_init");
            typedef Oseen::Assembler::Factory< Traits >
                Factory;
            // M\in R^{M\times M}
            auto MInversMatrix = Factory::matrix( sigmaSpace_, sigmaSpace_ );
            ASSERT_EQ( MInversMatrix->matrix().rows(), MInversMatrix->matrix().cols() );
            // W\in R^{M\times L}
            auto Wmatrix = Factory::matrix( sigmaSpace_, velocitySpace_ );
            // X\in R^{L\times M}
            auto Xmatrix = Factory::matrix( velocitySpace_, sigmaSpace_ );
            // O,Y\in R^{L\times L}
            auto Ymatrix = Factory::matrix( velocitySpace_, velocitySpace_ );
            auto Omatrix = Factory::matrix( velocitySpace_, velocitySpace_ );
            // Z\in R^{L\times K}
            auto Zmatrix = Factory::matrix( velocitySpace_, pressureSpace_ );
            // E\in R^{K\times L}
            auto Ematrix = Factory::matrix( pressureSpace_, velocitySpace_ );
            // R\in R^{K\times K}
            auto Rmatrix = Factory::matrix( pressureSpace_, pressureSpace_ );
            // H_{1}\in R^{M}
            auto H1rhs = Factory::rhs( "H1", sigmaSpace_ );
            // H_{2}\in R^{L}
            auto H2rhs = Factory::rhs( "H2", velocitySpace_ );
            auto H2_O_rhs = Factory::rhs( "H2_O", velocitySpace_ );
            // H_{3}\in R^{K}
            auto H3rhs = Factory::rhs( "H3", pressureSpace_ );
            auto m_integrator = typename Factory::MmatrixIntegratorType(*MInversMatrix);
            auto w_integrator = typename Factory::WmatrixIntegratorType(*Wmatrix);
            auto x_integrator = typename Factory::XmatrixIntegratorType(*Xmatrix);
            auto y_integrator = typename Factory::YmatrixIntegratorType(*Ymatrix);
            auto o_integrator = typename Factory::OmatrixIntegratorType(*Omatrix, beta_);
            auto z_integrator = typename Factory::ZmatrixIntegratorType(*Zmatrix);
            auto e_integrator = typename Factory::EmatrixIntegratorType(*Ematrix);
            auto r_integrator = typename Factory::RmatrixIntegratorType(*Rmatrix);
            auto h1_integrator = typename Factory::H1_IntegratorType(*H1rhs);
            auto h2_integrator = typename Factory::H2_IntegratorType(*H2rhs);
            auto h2_o_integrator = typename Factory::H2_O_IntegratorType(*H2_O_rhs, beta_);
            *H2rhs += *H2_O_rhs;
            auto h3_integrator = typename Factory::H3_IntegratorType(*H3rhs);
            DSC_PROFILER.stopTiming("Pass_init");

#ifndef STOKES_CONV_ONLY
            if ( do_oseen_discretization_ )
            {
                Oseen::Assembler::Coordinator< Traits, typename Factory::OseenIntegratorTuple >
                        coordinator ( discreteModel_, gridPart_, velocitySpace_, pressureSpace_, sigmaSpace_  );

                auto tuple = std::make_tuple(	m_integrator, w_integrator, x_integrator, y_integrator,
                                        o_integrator, z_integrator, e_integrator, r_integrator,
                                        h1_integrator, h2_integrator,h2_o_integrator, h3_integrator );
                coordinator.apply( tuple );
            }
            else
            {
                Oseen::Assembler::Coordinator< Traits, typename Factory::StokesIntegratorTuple >
                        coordinator ( discreteModel_, gridPart_, velocitySpace_, pressureSpace_, sigmaSpace_  );

                typename Factory::StokesIntegratorTuple tuple(	m_integrator, w_integrator, x_integrator, y_integrator,
                                        z_integrator, e_integrator, r_integrator,
                                        h1_integrator, h2_integrator,h3_integrator );
                coordinator.apply( tuple );
            }
#else
            Oseen::Assembler::Coordinator< Traits, typename Factory::ConvIntegratorTuple >
                    coordinator ( discreteModel_, gridPart_, velocitySpace_, pressureSpace_, sigmaSpace_  );

            typename Factory::ConvIntegratorTuple tuple(	o_integrator, h2_integrator, h2_o_integrator );
            coordinator.apply( tuple );
#endif
            // do the actual lgs solving
            DSC_LOG_INFO << "Solving system with " << dest.discreteVelocity().size() << " + " << dest.discretePressure().size() << " unknowns" << std::endl;
            info_ = Oseen::SolverCallerProxy< ThisType >::call( do_oseen_discretization_, rhs_datacontainer, dest,
                                            arg, *Xmatrix, *MInversMatrix, *Ymatrix, *Omatrix, *Ematrix,
                                            *Rmatrix, *Zmatrix, *Wmatrix, *H1rhs, *H2rhs, *H3rhs, beta_ );
        } // end of apply

		void getRuninfo( DSC::RunInfo& info )
        {
			info.iterations_inner_avg = int( info_.iterations_inner_avg );
            info.iterations_inner_min = info_.iterations_inner_min;
            info.iterations_inner_max = info_.iterations_inner_max;
            info.iterations_outer_total = info_.iterations_outer_total;
            info.max_inner_accuracy = info_.max_inner_accuracy;
        }

    private:
        DiscreteModelType discreteModel_;
		const typename Traits::GridPartType& gridPart_;
        const typename Traits::DiscreteOseenFunctionSpaceWrapperType& spaceWrapper_;
		const typename Traits::DiscreteVelocityFunctionSpaceType& velocitySpace_;
		const typename Traits::DiscretePressureFunctionSpaceType& pressureSpace_;
		typename Traits::DiscreteSigmaFunctionSpaceType sigmaSpace_;
        const typename Traits::DiscreteVelocityFunctionType beta_;
		const bool do_oseen_discretization_;
        SaddlepointInverseOperatorInfo info_;

};

} // namespace Dune
#endif  // end of stokespass.hh

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

