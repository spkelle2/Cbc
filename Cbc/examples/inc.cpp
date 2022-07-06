// inc.cpp: example of event handler to save
// every incumbent solution to a new file

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <OsiCbcSolverInterface.hpp>
#include <CbcModel.hpp>
#include <CglPreProcess.hpp>
#include <CbcSolver.hpp>
#include <Cbc_C_Interface.h>
#include "CbcEventHandler.hpp"

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

  double bestCost;
};

SolHandler::SolHandler()
  : CbcEventHandler()
  , bestCost(COIN_DBL_MAX)
{
}

SolHandler::SolHandler(const SolHandler &rhs)
  : CbcEventHandler(rhs)
  , bestCost(rhs.bestCost)
{
}

SolHandler::SolHandler(CbcModel *model)
  : CbcEventHandler(model)
  , bestCost(COIN_DBL_MAX)
{
}

SolHandler::~SolHandler()
{
}

SolHandler &SolHandler::operator=(const SolHandler &rhs)
{
  if (this != &rhs) {
    CbcEventHandler::operator=(rhs);
    this->bestCost = rhs.bestCost;
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
      std::cout << "there are " << this->model_->globalCuts()->sizeRowCuts() <<
        " cuts added after solving the root nodes\n";
    }
  }

  return noAction;
}

int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "enter instance name\n");
    exit(EXIT_FAILURE);
  }

  CbcSolverUsefulData cbcData;
  cbcData.noPrinting_ = false;
  OsiClpSolverInterface lp;

  lp.readMps(argv[1]);

  CbcModel model(lp);
//  model.setMaximumNodes(1);
  model.mappingNodes(true);

  SolHandler sh;
  model.passInEventHandler(&sh);

  CbcMain0(model, cbcData);
  CbcMain1(argc - 1, (const char **)(argv + 1), model, callBack, cbcData);

  return 0;
}
