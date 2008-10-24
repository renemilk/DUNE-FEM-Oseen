// $Id: integration.cc 184 2007-10-16 12:18:29Z robertk $

// Dune includes
#include"config.h"           // file constructed by ./configure script
#include<dune/grid/sgrid.hh> // load sgrid definition
#include <dune/common/mpihelper.hh> // include mpi helper class 

// checks for defined gridtype and inlcudes appropriate dgfparser implementation
#include <dune/grid/io/file/dgfparser/dgfgridtype.hh>

#include"functors.hh"
#include"integrateentity.hh"

//! uniform refinement test
template<class Grid>
void uniformintegration (Grid& grid)
{
  // function to integrate
  Exp<typename Grid::ctype,Grid::dimension> f;

  // get iterator type
  typedef typename Grid::template Codim<0>::LeafIterator LeafIterator;

  // loop over grid sequence
  double oldvalue=1E100;
  for (int k=0; k<10; k++)
	{
	  // compute integral with some order
	  double value = 0.0;
	  LeafIterator eendit = grid.template leafend<0>();
	  for (LeafIterator it = grid.template leafbegin<0>(); it!=eendit; ++it)
              value += integrateentity(it,f,1);        /*@\label{ic:call}@*/

	  // print result and error estimate
	  std::cout << "elements=" 
				<< std::setw(8) << std::right
				<< grid.size(0)
				<< " integral=" 
				<< std::scientific << std::setprecision(12) 
				<< value 
				<< " error=" << std::abs(value-oldvalue) 
				<< std::endl;

	  // save value of integral
	  oldvalue=value;

	  // refine all elements
	  grid.globalRefine(1);
	}
}

int main(int argc, char **argv)
{
  // initialize MPI, finalize is done automatically on exit 
  Dune::MPIHelper::instance(argc,argv);

  // start try/catch block to get error messages from dune
  try {
    using namespace Dune;

    // use unitcube from grids 
    std::stringstream dgfFileName;
    dgfFileName << "grids/unitcube" << GridType :: dimension << ".dgf";

    // create grid pointer, GridType is defined by gridtype.hh
    GridPtr<GridType> gridPtr( dgfFileName.str() );

    // integrate and compute error with extrapolation
    uniformintegration( *gridPtr );
  }
  catch (std::exception & e) {
    std::cout << "STL ERROR: " << e.what() << std::endl;
    return 1;
  }
  catch (Dune::Exception & e) {
    std::cout << "DUNE ERROR: " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cout << "Unknown ERROR" << std::endl;
    return 1;
  }

  // done
  return 0;
}