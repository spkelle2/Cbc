#include <memory>
#include <vector>
#include <iostream>


class Model {

  std::shared_ptr<std::vector<int> > nodeList_;

public:

  std::shared_ptr<std::vector<int> > nodeList() const
  {
    return nodeList_;
  }
  void addNode(int number)
  {
    nodeList_->push_back(number);
  }

  Model(){
    nodeList_ = std::make_shared<std::vector<int> >();
  }
};

int main() {
  Model mdl;
  mdl.addNode(0);
  Model mdl_copy = mdl;
  mdl_copy.addNode(1);
  std::cout << "nodeList has length " << mdl.nodeList()->size() << "\n";
  std::cout << "nodeList copy has length " << mdl_copy.nodeList()->size() << "\n";
}