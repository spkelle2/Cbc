// rootCutsAndDuals.cpp: example of using CLP and CBC event handlers together. The former
// prints out dual values of the root node while the latter prints out globally valid cuts
// To get both in the space of the input formulation, run with the following arguments
// -import <model.mps> -preprocess off -presolve off -solve

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <OsiCbcSolverInterface.hpp>
#include "OsiClpSolverInterface.hpp"
#include <CbcModel.hpp>
#include <CglPreProcess.hpp>
#include <CbcSolver.hpp>
#include <Cbc_C_Interface.h>
#include "CbcEventHandler.hpp"
#include "ClpEventHandler.hpp"

static int callBack(CbcModel *model, int whereFrom)
{
  return 0;
}

class SolHandler : public CbcEventHandler {
public:
  virtual CbcAction event(CbcEvent whichEvent);
  SolHandler();
  SolHandler(CbcModel *model);
  virtual ~SolHandler();
  SolHandler(const SolHandler &rhs);
  SolHandler &operator=(const SolHandler &rhs);
  virtual CbcEventHandler *clone() const;
};

SolHandler::SolHandler()
  : CbcEventHandler()
{
}

SolHandler::SolHandler(const SolHandler &rhs)
  : CbcEventHandler(rhs)
{
}

SolHandler::SolHandler(CbcModel *model)
  : CbcEventHandler(model)
{
}

SolHandler::~SolHandler()
{
}

SolHandler &SolHandler::operator=(const SolHandler &rhs)
{
  if (this != &rhs) {
    CbcEventHandler::operator=(rhs);
  }
  return *this;
}

CbcEventHandler *SolHandler::clone() const
{
  return new SolHandler(*this);
}

CbcEventHandler::CbcAction SolHandler::event(CbcEvent whichEvent)
{
  // examine root node (excluding subtrees - avoids primal heuristics)
  if (this->model_->getNodeCount() == 1 && (model_->specialOptions() & 2048) == 0) {
    if (whichEvent == node) {
      for (int cut_idx = 0; cut_idx < this->model_->globalCuts()->sizeRowCuts(); cut_idx++){
        int num_elements = this->model_->globalCuts()->rowCutPtr(cut_idx)->row().getNumElements();
        std::cout << this->model_->globalCuts()->rowCutPtr(cut_idx)->lb() << " <= ";
        for (int element_idx = 0; element_idx < num_elements; element_idx++){
          // format is <coefficient> * x_<index>
          std::cout << this->model_->globalCuts()->rowCutPtr(cut_idx)->row().getElements()[element_idx] << "x_" <<
            this->model_->globalCuts()->rowCutPtr(cut_idx)->row().getIndices()[element_idx];
          if (element_idx < num_elements - 1){
            std::cout << " + ";
          }
        }
        std::cout << " <= " << this->model_->globalCuts()->rowCutPtr(cut_idx)->ub() << "\n\n";
      }
    }
  }
  return noAction;
}

class MyEventHandler3 : public ClpEventHandler {

public:
  /**@name Overrides */
  //@{
  virtual int event(Event whichEvent);
  //@}

  /**@name Constructors, destructor etc*/
  //@{
  /** Default constructor. */
  MyEventHandler3();
  /// Constructor with pointer to model (redundant as setEventHandler does)
  MyEventHandler3(ClpSimplex *model);
  /** Destructor */
  virtual ~MyEventHandler3();
  /** The copy constructor. */
  MyEventHandler3(const MyEventHandler3 &rhs);
  /// Assignment
  MyEventHandler3 &operator=(const MyEventHandler3 &rhs);
  /// Clone
  virtual ClpEventHandler *clone() const;
  //@}

  int rows_;
  int cols_;
  bool called_before_;

protected:
  // data goes here
};
//-------------------------------------------------------------------
// Default Constructor
//-------------------------------------------------------------------
MyEventHandler3::MyEventHandler3()
    : ClpEventHandler()
{
  rows_ = 0;
  cols_ = 0;
  called_before_ = false;
}

//-------------------------------------------------------------------
// Copy constructor
//-------------------------------------------------------------------
MyEventHandler3::MyEventHandler3(const MyEventHandler3 &rhs)
    : ClpEventHandler(rhs)
{
  rows_ = rhs.rows_;
  cols_ = rhs.cols_;
  called_before_ = rhs.called_before_;
}

// Constructor with pointer to model
MyEventHandler3::MyEventHandler3(ClpSimplex *model)
    : ClpEventHandler(model)
{
  rows_ = 0;
  cols_ = 0;
  called_before_ = false;
}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
MyEventHandler3::~MyEventHandler3()
{
}

//----------------------------------------------------------------
// Assignment operator
//-------------------------------------------------------------------
MyEventHandler3 &
MyEventHandler3::operator=(const MyEventHandler3 &rhs)
{
  if (this != &rhs) {
    ClpEventHandler::operator=(rhs);
  }
  return *this;
}
//-------------------------------------------------------------------
// Clone
//-------------------------------------------------------------------
ClpEventHandler *MyEventHandler3::clone() const
{
  return new MyEventHandler3(*this);
}

int MyEventHandler3::event(Event whichEvent)
{
  // get the size of the problem if we haven't before
  if (!called_before_){
    rows_ = this->model_->numberRows();
    cols_ = this->model_->numberColumns();
    called_before_ = true;
  }
  // After each dual iteration
  if (whichEvent == endOfIteration && model_->algorithm() < 0){
    // if working on the full problem, get the dual solutions
    if (this->model_->numberColumns() == cols_ && this->model_->numberRows() == rows_) {
      std::cout << "Iteration " << this->model_->numberIterations() << " dual solutions \n";
      // column dual variables
      std::cout << "s = [ ";
      for (int col_idx = 0; col_idx < this->model_->numberColumns(); col_idx++) {
        if (this->model_->rowScale()) {
          std::cout << this->model_->djRegion(1)[col_idx] /
                       this->model_->columnScale()[col_idx] << " ";
        }
        else {
          std::cout << this->model_->djRegion(1)[col_idx] << " ";
        }
      }
      std::cout << "]\n";
      // row dual variables
      std::cout << "y = [ ";
      for (int row_idx = 0; row_idx < this->model_->numberRows(); row_idx++) {
        if (this->model_->rowScale()) {
          std::cout << this->model_->djRegion(0)[row_idx] *
                       this->model_->rowScale()[row_idx] << " ";
        }
        else {
          std::cout << this->model_->djRegion(0)[row_idx] << " ";
        }
      }
      std::cout << "]\n";
      std::cout << "\n";
    }
  }
  return -1; // carry on
}

int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "enter instance name\n");
    exit(EXIT_FAILURE);
  }

  CbcSolverUsefulData cbcData;
  cbcData.noPrinting_ = false;

  // set up lp solver
  OsiClpSolverInterface lp;
  lp.readMps(argv[1]);
  ClpSimplex *lp_model = lp.getModelPtr();
  MyEventHandler3 eventHandler(lp_model);
  lp_model->passInEventHandler(&eventHandler);

  // set up branch and cut solver
  CbcModel model(lp);
  model.setMaximumNodes(1);  // stop after one node since we only care about root node
  SolHandler sh;
  model.passInEventHandler(&sh);

  CbcMain0(model, cbcData);
  CbcMain1(argc - 1, (const char **)(argv + 1), model, callBack, cbcData);

  return 0;
}
