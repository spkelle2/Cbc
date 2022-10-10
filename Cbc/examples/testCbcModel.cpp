// testCbcModel.cpp tests for sean's additions to CbcModel
// run args: -cuts off -heuristics off -preprocess off -presolve off -strong 0 -log 3 -passC 0 -passT 0 -solve
// add -std=c++11 to ADDINCFLAGS in line 39 of makefile in <build-folder>/cbc/<version>/examples

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <assert.h>
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
  CbcSolverUsefulData cbcData;
  cbcData.noPrinting_ = false;

  // set up lp solver for test instance
  ClpSimplex lp;

  // create variables
  lp.addColumn(0, NULL, NULL, 0, 10, -1);
  lp.setInteger(0);
  lp.addColumn(0, NULL, NULL, 0, 10, -1);
  lp.setInteger(1);
  lp.addColumn(0, NULL, NULL, 0, 10, -1);
  lp.setInteger(2);

  // create constraints
  std::vector<int> indices0 = {0, 2};
  std::vector<double> elements0 = {1, 1};
  lp.addRow(elements0.size(), &indices0[0], &elements0[0], 0, 1.5);

  std::vector<int> indices1 = {1};
  std::vector<double> elements1 = {1};
  lp.addRow(elements1.size(), &indices1[0], &elements1[0], 0, 1.25);

  // write to read back in since the osi constructor is broken
  lp.writeLp("test");

  // set up branch and cut solver
  OsiClpSolverInterface osi;
  osi.readLp("test.lp");
  CbcModel model(osi);
  model.persistNodes(true);
  SolHandler sh;
  model.passInEventHandler(&sh);

  CbcMain0(model, cbcData);
  CbcMain1(argc - 1, (const char **)(argv + 1), model, callBack, cbcData);

  // check number of nodes
  assert(model.nodeMap().size() == 3);

  // check lower bounds
  assert(model.nodeMap()[0].second->getNumRows() == 4);
  assert(model.nodeMap()[0].second->getRowLower()[0] == -1.5);
  assert(model.nodeMap()[0].second->getRowLower()[1] == 0);
  assert(model.nodeMap()[0].second->getRowLower()[2] == -1.25);
  assert(model.nodeMap()[0].second->getRowLower()[3] == 0);

  // check upper bounds
  assert(model.nodeMap()[0].second->getRowUpper()[0] == COIN_DBL_MAX);
  assert(model.nodeMap()[0].second->getRowUpper()[1] == COIN_DBL_MAX);
  assert(model.nodeMap()[0].second->getRowUpper()[2] == COIN_DBL_MAX);
  assert(model.nodeMap()[0].second->getRowUpper()[3] == COIN_DBL_MAX);

  // check elements
  std::vector<int> indices = {0, 1, 2, 3, 0, 1};
  std::vector<double> elements = {-1, 1, -1, 1, -1, 1};
  assert(model.nodeMap()[0].second->matrix()->isColOrdered());
  assert(model.nodeMap()[0].second->matrix()->getNumElements() == elements.size());
  for (int idx = 0; idx < elements.size(); idx++){
    assert(model.nodeMap()[0].second->matrix()->getElements()[idx] == elements[idx]);
    assert(model.nodeMap()[0].second->matrix()->getIndices()[idx] == indices[idx]);
  }

  return 0;
}
