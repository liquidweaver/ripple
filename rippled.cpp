/* Ripple Distributed Project/Task System
 * 2011 Joshua Weaver
 *
 *
 * TODO:
 * 	Task Actions: Create, Accept, Forward, Decline, Complete, ReOpen
 * 	Task Calculations: Actual State ( isleaf? state : min( children statuses),
 * 								Get Children ( select task_id where parent_task = :task_id )
 * 								Get Logs From task ( select * from logs where task_id=:task_id )
 * 								Load Task Tree ( Recurse through given task and all children)
 * 	User Information: Visible tasks ( stakeholder U assigned U in watchers )
 */
#include <iostream>
#include <exception>
#include <string>
#include <memory>
#include "Ripple.hpp"
#include "RippleInterface.hpp"

static const char *options[] = {
  "document_root", "html",
  "listening_ports", "8080",
  "num_threads", "5",
  "index_files", "main.html",
  NULL
};

using namespace std;

int main( int argc, char* argv[] ) {
  auto_ptr<Ripple> ripple( new Ripple() );
  auto_ptr<RippleInterface> ri( RippleInterface::Instance( ripple.get(), options ) );

  cout << "Press any key to stop..." << endl;
  cin.get();

  return 0;
}
// vim: ts=2 sw=2 ai et
