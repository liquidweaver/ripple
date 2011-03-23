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
#include <signal.h>
#include "Ripple.hpp"
#include "RippleInterface.hpp"

#define EVENTS_PORT 8081

using namespace std;
bool terminated = false;

void sigproc( int ) {      
    terminated = true;
}

int main( int argc, char* argv[] ) {
  Ripple* ripple = Ripple::Instance();
  RippleInterface* ri = RippleInterface::Instance( ripple );
  signal( SIGINT, sigproc );

  cout << "Ctrl-c to quit..." << endl;
  while ( !terminated ) { sleep( 5000 ); }

  RippleInterface::Release();
  Ripple::Release();
  return 0;
}
// vim: ts=2 sw=2 ai et
