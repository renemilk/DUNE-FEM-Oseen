#ifndef DUNE_OSEEN_NEW_BICGSTAB_HH
#define DUNE_OSEEN_NEW_BICGSTAB_HH

#include <dune/stuff/printing.hh>
#include <dune/stuff/logging.hh>

#include <limits>
#include <cmath>
#include <utility>
#include <boost/format.hpp>

#ifdef SOLVER_NAMESPACE
	namespace SOLVER_NAMESPACE {
#else
	namespace Stuff {
#endif



template < class PressureDiscreteFunctionType, class OperatorType >
class NewBicgStab {

public:
	typedef std::pair < int , double >
		ReturnValueType;

	NewBicgStab(	const OperatorType& op,
				const double relLimit,
				const double absLimit,
				const unsigned int max_iter,
				const unsigned int solverVerbosity_ )
		: operator_(op),
		  relLimit_(relLimit),
		  absLimit_(absLimit),
		  max_iter_(max_iter),
		  solverVerbosity_(solverVerbosity_)
	{}

	//! for bfg interface compliance
	void setAbsoluteLimit( const double  ){}

	void apply(const PressureDiscreteFunctionType& rhs, PressureDiscreteFunctionType& dest ) const
	{
		ReturnValueType info;
		apply( rhs, dest, info );
	}

	void apply(const PressureDiscreteFunctionType& rhs, PressureDiscreteFunctionType& dest, ReturnValueType& info ) const
	{
		unsigned int iteration = 1;
		const std::string cg_name( "OuterCG");
		Stuff::Logging::LogStream& logDebug = Logger().Dbg();

		PressureDiscreteFunctionType residuum( "residuum", dest.space() );
		PressureDiscreteFunctionType start_residuum( "start_residuum", dest.space() );
		PressureDiscreteFunctionType s( "s", dest.space() );
		PressureDiscreteFunctionType t( "t", dest.space() );
		PressureDiscreteFunctionType v( "v", dest.space() );
		PressureDiscreteFunctionType search_direction( "search_direction", dest.space() );

		// r^0 = - S * p^0 + rhs
		operator_.apply( dest, residuum );
		if ( solverVerbosity_ > 3 )
			Stuff::printFunctionMinMax( logDebug, residuum );
		residuum -= rhs;
		residuum *= -1.;

		// r_^0 = r^0
		start_residuum.assign( residuum );
		search_direction.assign( residuum );

		double rho(0);
		double delta(0); //norm of residuum

		double alpha,omega,last_rho;
		alpha = omega = last_rho  = std::numeric_limits<double>::max();

		PressureDiscreteFunctionType residuum_T( "s", dest.space() );
		residuum_T.assign( start_residuum );//??

		if ( solverVerbosity_ > 2 )
			Logger().Info() << " -- \n";

		while( iteration < max_iter_) {
			rho = residuum_T.scalarProductDofs( residuum );
			if ( rho == 0.0 ) {
				if ( solverVerbosity_ > 3 )
					logDebug << boost::format( "%s: abort, theta = %e") % cg_name % rho << std::endl;
				break;
			}
			assert( !std::isnan(rho) );
			assert( std::isfinite(rho) );

			if ( iteration == 1 ) {
				search_direction.assign( start_residuum );
			}
			else {
				const double beta = (rho/last_rho) * (alpha/omega);
				search_direction *= beta;
				search_direction.axpy( -beta*omega, v );
				search_direction += residuum;
			}

			operator_.apply( search_direction, v );//v=S*p
			alpha = rho/ residuum_T.scalarProductDofs( v );
			assert( !std::isnan(alpha) );
			assert( std::isfinite(alpha) );

			s.assign( residuum );
			s.axpy( -alpha, v );
			const double s_norm = std::sqrt( s.scalarProductDofs( s ) );
			if ( s_norm < absLimit_ ) {
				dest.axpy( alpha, search_direction );
				logDebug << boost::format( "%s: iter %i\taborted: s: %e") % cg_name % iteration % s_norm << std::endl;
				break;
			}
			operator_.apply( s, t );
			omega = t.scalarProductDofs( s ) / t.scalarProductDofs( t );

			if ( solverVerbosity_ > 3 )
				Stuff::printFunctionMinMax( logDebug, search_direction );
			dest.axpy( alpha, search_direction );
			if ( solverVerbosity_ > 3 )
				Stuff::printFunctionMinMax( logDebug, dest );
			dest.axpy( omega, s );

			residuum.assign( s );
			residuum.axpy( - omega, t );

			delta = std::sqrt( residuum.scalarProductDofs( residuum ) );
			if ( delta < absLimit_ ) {
				logDebug << boost::format( "%s: aborted, iter %i\tres %e") % cg_name % iteration % delta << std::endl;
				break;
			}

			if ( solverVerbosity_ > 3 ) {
				logDebug << boost::format( "%s: iter %i\tres %e alpha %e \trho %e") % cg_name % iteration
							% delta % alpha %  rho << std::endl;
				Stuff::printFunctionMinMax( logDebug, dest );
			}
			assert( omega != 0.0 );

			last_rho = rho;
			iteration++;
		} //end while

		info = ReturnValueType( iteration, rho );
	}

	const OperatorType& operator_;
	const double relLimit_;
	const double absLimit_;
	const unsigned int max_iter_;
	const unsigned int solverVerbosity_;
};
} //namespace


#endif // DUNE_OSEEN_NEW_BICGSTAB_HH