#include <iostream>
#include <pulse.h>
#include <TTree.h>
#include <TFile.h>

int main(int argc, char** argv)
{
  pulse* p = new pulse();
  p->Loop();
  delete p;
  return 0;
}
