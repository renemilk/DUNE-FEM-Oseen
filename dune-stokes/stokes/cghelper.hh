#ifndef INNERCG_HH_INCLUDED
#define INNERCG_HH_INCLUDED

#include <dune/fem/solver/oemsolver/oemsolver.hh>
#include "../src/logging.hh"
#include "../src/stuff.hh"

namespace Dune {


/** \brief Operator to evaluate Matrix multiplication

    multOEM method evaluates matrix vector multiplication\n
    \f$ A := -1 \cdot X  M^{-1} W x  + Yx \f$


**/
template <  class WMatType,
            class MMatType,
            class XMatType,
            class YMatType,
            class DiscreteSigmaFunctionType >
class MatrixA_Operator {

    public:

        typedef MatrixA_Operator<   WMatType,
                        MMatType,
                        XMatType,
                        YMatType,
                        DiscreteSigmaFunctionType >
                    ThisType;
        /** The operator needs the


        **/
        MatrixA_Operator ( const WMatType& w_mat,
                const MMatType& m_mat,
                const XMatType& x_mat,
                const YMatType& y_mat,
                const typename DiscreteSigmaFunctionType::DiscreteFunctionSpaceType& sig_space )
            :  w_mat_(w_mat),
            m_mat_(m_mat),
            x_mat_(x_mat),
            y_mat_(y_mat),
            sig_tmp1( "sig_tmp1", sig_space ),
            sig_tmp2( "sig_tmp2", sig_space )
        {}

        ~MatrixA_Operator()
        {}

        template <class VECtype>
        void multOEM(const VECtype *x, VECtype * ret) const
        {
#ifdef EXTRALOG
            Logging::MatlabLogStream& matlabLogStream = Logger().Matlab();
            static unsigned int i = 0;
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( x, w_mat_.cols(), "mult_A__x", matlabLogStream );
            }
#endif
            sig_tmp1.clear();
            sig_tmp2.clear();
            Logging::LogStream& dbg = Logger().Dbg();
			// ret = ( ( X * ( -1* ( M_inv * ( W * x ) ) ) ) + ( Y * x ) )
            w_mat_.multOEM( x, sig_tmp1.leakPointer() );
#ifdef EXTRALOG
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( sig_tmp1.leakPointer(), sig_tmp1.size() , "mult_A__W_times_x", matlabLogStream );
            }
#endif
            m_mat_.apply( sig_tmp1, sig_tmp2 );//Stuff:DiagmUlt
#ifdef EXTRALOG
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( sig_tmp2.leakPointer(), sig_tmp2.size() , "mult_A__Minvers_times_W_times_x", matlabLogStream );
            }
#endif
            sig_tmp2 *= ( -1 );
            x_mat_.multOEM( sig_tmp2.leakPointer(), ret );
#ifdef EXTRALOG
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( ret, ( sig_tmp1.size() / 2 ) , "mult_A__minus_X_times_Minvers_times_W_times_x", matlabLogStream );
            }
#endif
            y_mat_.multOEMAdd( x, ret );
#ifdef EXTRALOG
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( ret, ( sig_tmp1.size() / 2 ) , "mult_A__Y_times_x_minus_X_times_Minvers_times_W_times_x", matlabLogStream );
            }
            i++;
#endif
        }

#ifdef USE_BFG_CG_SCHEME
        template <class VECtype>
        void multOEM(const VECtype *x, VECtype * ret, const IterationInfo& info ) const
        {
            multOEM(x,ret);
        }
#endif

        ThisType& systemMatrix ()
        {
            return *this;
        }

    private:
        const WMatType& w_mat_;
        const MMatType& m_mat_;
        const XMatType& x_mat_;
        const YMatType& y_mat_;
        mutable DiscreteSigmaFunctionType sig_tmp1;
        mutable DiscreteSigmaFunctionType sig_tmp2;
};


template <  class A_SolverType,
            class B_t_matrixType,
            class CmatrixType,
            class BmatrixType,
            class MmatrixType,
            class DiscreteVelocityFunctionType ,
            class DiscretePressureFunctionType>
class SchurkomplementOperator //: public OEMSolver::PreconditionInterface
{
    public:

        typedef SchurkomplementOperator <   A_SolverType,
                                            B_t_matrixType,
                                            CmatrixType,
                                            BmatrixType,
                                            MmatrixType,
                                            DiscreteVelocityFunctionType,
                                            DiscretePressureFunctionType>
                ThisType;

        typedef typename A_SolverType::ReturnValueType
                ReturnValueType;

        SchurkomplementOperator( A_SolverType& a_solver,
                                const B_t_matrixType& b_t_mat,
                                const CmatrixType& c_mat,
                                const BmatrixType& b_mat,
                                const MmatrixType& m_mat,
                                const typename DiscreteVelocityFunctionType::DiscreteFunctionSpaceType& velocity_space,
                                const typename DiscretePressureFunctionType::DiscreteFunctionSpaceType& pressure_space )
            : a_solver_(a_solver),
            b_t_mat_(b_t_mat),
            c_mat_(c_mat),
            b_mat_(b_mat),
            m_mat_(m_mat),
            tmp1 ( "tmp1", velocity_space ),
            tmp2 ( "tmp2", velocity_space ),
            do_bfg( Parameters().getParam( "do-bfg", true ) ),
            total_inner_iterations( 0 )
        {}

        template <class VECtype>
        void multOEM(const VECtype *x, VECtype * ret) const
        {
#ifdef EXTRALOG
            static unsigned int i = 0;
            Logging::MatlabLogStream& matlabLogStream = Logger().Matlab();
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( x, b_mat_.cols(), "mult_S__x", matlabLogStream );
            }
#endif
            Logging::LogStream& info = Logger().Info();

            const bool solverVerbosity = Parameters().getParam( "solverVerbosity", 0 );

            tmp1.clear();
//            Stuff::addScalarToFunc( tmp2, 0.1 );

			// ret = ( ( B_t * ( A^-1 * ( B * x ) ) ) + ( C * x ) )
//			Stuff::oneLinePrint( std::cout, tmp1 ) ;
            b_mat_.multOEM( x, tmp1.leakPointer() );
#ifdef EXTRALOG
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( tmp1.leakPointer(), b_mat_.rows(), "mult_S__B_times_x", matlabLogStream );
            }
#endif
//            Stuff::oneLinePrint( std::cout, tmp1 ) ;
//            Stuff::printDoubleVec( std::cout, x, b_mat_.cols() );
            ReturnValueType cg_info;
            a_solver_.apply( tmp1, tmp2, cg_info );
#ifdef EXTRALOG
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( tmp2.leakPointer(), b_t_mat_.cols(), "mult_S__Ainv_times_B_times_x", matlabLogStream );
            }
#endif
            info << "\t\t\t\t\t inner iterations: " << cg_info.first << std::endl;
            total_inner_iterations += cg_info.first;
            b_t_mat_.multOEM( tmp2.leakPointer(), ret );
#ifdef EXTRALOG
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( ret, b_t_mat_.rows(), "mult_S__Bt_times_Ainv_times_B_times_x", matlabLogStream );
            }
#endif
            c_mat_.multOEMAdd( x, ret );
#ifdef EXTRALOG
            if ( i == 1 ) {
                Stuff::printDoubleVectorMatlabStyle( ret, b_t_mat_.rows(), "mult_S__Bt_times_Ainv_times_B_times_x_plus_C_times_x", matlabLogStream );
            }
            i++;
#endif
			//
        }

#ifdef USE_BFG_CG_SCHEME
        template <class VECtype>
        void multOEM(const VECtype *x, VECtype * ret, const IterationInfo& info ) const
        {
            if ( do_bfg ) {
                static const double tau = Parameters().getParam( "bfg-tau", 0.1 );
                double limit = info.second.first;
                const double residuum = fabs(info.second.second);
                const int n = info.first;
//		Logger().Info() << "res: "<<residuum<<std::endl;
                limit = tau * std::min( 1. , limit / std::min ( residuum * n  , 1.0 ) );
//                limit = tau * std::min( 1. , limit / std::min ( std::pow( residuum, n ) , 1.0 ) );
                a_solver_.setAbsoluteLimit( limit );
                Logger().Info() << "\t\t\t Set inner error limit to: "<< limit << std::endl;
            }
            multOEM( x, ret );
        }
#endif

        ThisType& systemMatrix ()
        {
            return *this;
        }

        ThisType& preconditionMatrix()
        {
            return *this;
        }

        bool hasPreconditionMatrix () const
        {
            return false;
        }

        bool rightPrecondition() const
        {
            return false;
        }

        template <class VecType>
        void precondition( const VecType* tmp, VecType* dest ) const
        {

        }

        long getTotalInnerIterations()
        {
            return total_inner_iterations;
        }

    private:
        mutable A_SolverType& a_solver_;
        const B_t_matrixType& b_t_mat_;
        const CmatrixType& c_mat_;
        const BmatrixType& b_mat_;
        const MmatrixType& m_mat_;
        mutable DiscreteVelocityFunctionType tmp1;
        mutable DiscreteVelocityFunctionType tmp2;
        bool do_bfg;
        mutable long total_inner_iterations;
};

template <  class WMatType,
            class MMatType,
            class XMatType,
            class YMatType,
            class DiscreteSigmaFunctionType,
            class DiscreteVelocityFunctionType >
class A_SolverCaller {
    public:
        typedef MatrixA_Operator<   WMatType,
                                    MMatType,
                                    XMatType,
                                    YMatType,
                                    DiscreteSigmaFunctionType >
                A_OperatorType;

        typedef INNER_CG_SOLVERTYPE< DiscreteVelocityFunctionType, A_OperatorType >
            CG_SolverType;
        typedef typename CG_SolverType::ReturnValueType
            ReturnValueType;

        A_SolverCaller( const WMatType& w_mat,
                const MMatType& m_mat,
                const XMatType& x_mat,
                const YMatType& y_mat,
                const typename DiscreteSigmaFunctionType::DiscreteFunctionSpaceType& sig_space,
                const double relLimit,
                const double absLimit,
                const bool verbose )
            :  w_mat_(w_mat),
            m_mat_(m_mat),
            x_mat_(x_mat),
            y_mat_(y_mat),
            sig_tmp1( "sig_tmp1", sig_space ),
            sig_tmp2( "sig_tmp2", sig_space ),
            a_op_( w_mat, m_mat, x_mat, y_mat, sig_space ),
            cg_solver( a_op_,   relLimit,
                                absLimit,
                                2000, //inconsequential anyways
                                verbose )
        {}

        void apply ( const DiscreteVelocityFunctionType& arg, DiscreteVelocityFunctionType& dest, ReturnValueType& ret )
        {
            cg_solver.apply(arg,dest, ret);
        }

        void apply ( const DiscreteVelocityFunctionType& arg, DiscreteVelocityFunctionType& dest )
        {
            cg_solver.apply(arg,dest);
        }

        const A_OperatorType& getA_operator( )
        {
            return a_op_;
        }

#ifdef USE_BFG_CG_SCHEME
        void setAbsoluteLimit( const double abs )
        {
            cg_solver.setAbsoluteLimit( abs );
        }
#endif

    private:
        const MMatType precond_;
        const WMatType& w_mat_;
        const MMatType& m_mat_;
        const XMatType& x_mat_;
        const YMatType& y_mat_;
        mutable DiscreteSigmaFunctionType sig_tmp1;
        mutable DiscreteSigmaFunctionType sig_tmp2;
        A_OperatorType a_op_;
        CG_SolverType cg_solver;
};

}

#endif // INNERCG_HH_INCLUDED