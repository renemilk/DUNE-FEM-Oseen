#ifndef STOKES_PROBLEMS_GENERALIZED_HH
#define STOKES_PROBLEMS_GENERALIZED_HH

#include <dune/fem/function/common/function.hh>
#include <dune/stuff/common/misc.hh>
#include "common.hh"

namespace StokesProblems {
namespace Generalized {

ALLGOOD_SETUPCHECK;
static const std::string identifier = "Generalized";
static const bool hasExactSolution	= true;

template < class FunctionSpaceImp >
class Force : public Dune::Fem::Function < FunctionSpaceImp , Force < FunctionSpaceImp > >
{
	public:
		typedef Force< FunctionSpaceImp >
			ThisType;
		typedef Dune::Fem::Function< FunctionSpaceImp, ThisType >
			BaseType;
		typedef typename BaseType::DomainType
			DomainType;
		typedef typename BaseType::RangeType
			RangeType;

        Force( const double viscosity, const double alpha = 0.0, const double scaling_factor = 1.0 )
            : viscosity_( viscosity ),
			  alpha_( alpha ),
			  scaling_factor_( scaling_factor )
		{}

		inline void evaluate( const DomainType& arg, RangeType& ret ) const
		{
			dune_static_assert( dim_ == 2, "Force_Unsuitable_WorldDim");
			const double x = arg[0];
			const double y = arg[1];
			const double cos_p = std::cos( M_PI_2 * (x+y) );
			const double cos_m = std::cos( M_PI_2 * (x-y) );
			const double pi_sq = M_PI_2 * M_PI ;//std::pow( M_PI_2 , 2 );
			ret[0]  = cos_p * ( alpha_ + viscosity_ * pi_sq ) + M_PI_2 * cos_m;
			ret[1]  = cos_p * ( - alpha_ - viscosity_ * pi_sq ) - M_PI_2 * cos_m;
		}

	private:
		const double viscosity_;
		const double alpha_;
		const double scaling_factor_;
		static const int dim_ = FunctionSpaceImp::dimDomain;
};

template < class FunctionSpaceImp >
class DirichletData : public Dune::Fem::Function < FunctionSpaceImp, DirichletData < FunctionSpaceImp > >
{
	public:
		typedef DirichletData< FunctionSpaceImp >
			ThisType;
		typedef Dune::Fem::Function< FunctionSpaceImp, ThisType >
			BaseType;
		typedef typename BaseType::DomainType
			DomainType;
		typedef typename BaseType::RangeType
			RangeType;

		template < class IntersectionType >
		void evaluate( const DomainType& arg, RangeType& ret, const IntersectionType& /*intersection*/ ) const
		{
			dune_static_assert( dim_ == 2, "DirichletData_Unsuitable_WorldDim");
			const double x1 = arg[0];
			const double x2 = arg[1];
			const double tmp = std::cos( ( M_PI_2 ) * ( x1 + x2 ) );
			ret[0] = tmp;
			ret[1] = -1.0 * tmp;
		}

        inline void evaluate( const DomainType& /*arg*/, RangeType& /*ret*/ ) const { assert( false ); }

	private:
		static const int dim_ = FunctionSpaceImp::dimDomain;
};

template < class FunctionSpaceImp  >
class Velocity : public Dune::Fem::Function < FunctionSpaceImp , Velocity < FunctionSpaceImp > >
{
	public:
		typedef Velocity< FunctionSpaceImp >
			ThisType;
		typedef Dune::Fem::Function< FunctionSpaceImp, ThisType >
			BaseType;
		typedef typename BaseType::DomainType
			DomainType;
		typedef typename BaseType::RangeType
			RangeType;

		inline void evaluate( const DomainType& arg, RangeType& ret ) const
		{
			dune_static_assert( dim_ == 2, "Velocity_Unsuitable_WorldDim");
			const double x1 = arg[0];
			const double x2 = arg[1];
			const double tmp = std::cos( ( M_PI_2 ) * ( x1 + x2 ) );
			ret[0] = tmp;
			ret[1] = -1.0 * tmp;
		}

	private:
		static const int dim_ = FunctionSpaceImp::dimDomain;
};

template < class FunctionSpaceImp  >
class Pressure : public Dune::Fem::Function < FunctionSpaceImp , Pressure < FunctionSpaceImp > >
{
	public:
		typedef Pressure< FunctionSpaceImp >
			ThisType;
		typedef Dune::Fem::Function< FunctionSpaceImp, ThisType >
			BaseType;
		typedef typename BaseType::DomainType
			DomainType;
		typedef typename BaseType::RangeType
			RangeType;

		inline void evaluate( const DomainType& arg, RangeType& ret ) const
		{
			dune_static_assert( dim_ == 2, "Pressure_Unsuitable_WorldDim");
			const double x1 = arg[0];
			const double x2 = arg[1];
			ret[0] = std::sin( ( M_PI_2 ) * ( x1 - x2 ) );
		}

	private:
		static const int dim_ = FunctionSpaceImp::dimDomain;
};

} // namespace Generalized {
} // namespace StokesProblems {

#endif // STOKES_PROBLEMS_GENERALIZED_HH

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

