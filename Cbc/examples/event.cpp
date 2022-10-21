// event.cpp: template for CBC event handler and callback.

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
  if (whereFrom == 5){
    // std::cout << "osiList has " << model->getOsiList().size() << " elements\n";
  }
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
  // examine each node when fathoming
  if ((model_->specialOptions() & 2048) == 0 && whichEvent == node) {
    std::cout << "examining a node\n";
  }
  if ((model_->specialOptions() & 2048) == 0 && whichEvent == solution) {
    std::cout << "solution was found during branch and bound\n";
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

  // set up lp solver
  OsiClpSolverInterface lp;
  lp.readMps(argv[1]);

  // set up branch and cut solver
  CbcModel model(lp);
  model.persistNodes(true);
  SolHandler sh;
  model.passInEventHandler(&sh);

  CbcMain0(model, cbcData);
  CbcMain1(argc - 1, (const char **)(argv + 1), model, callBack, cbcData);

  model.nodeMap()[0].first.get();
  model.nodeMap()[0].second.get();

  model.nodeMap()[0].first.get()->nodeMapIndex();
  model.nodeMap()[0].first.get()->nodeMapLineage();
  model.nodeMap()[0].first.get()->nodeMapLeafStatus();

  for (int node_idx = 0; node_idx < model.nodeMap().size(); node_idx++){
    if (model.nodeMap()[node_idx].first.get()->feasible() == 0){
      std::cout << "node " << node_idx << " was pruned on bound" << std::endl;
    }
    if (model.nodeMap()[node_idx].first.get()->feasible() == 2){
      std::cout << "node " << node_idx << " was pruned on infeasibility" << std::endl;
    }
  }

  return 0;
}
