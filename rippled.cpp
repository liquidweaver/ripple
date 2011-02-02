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

using namespace std;

int main( int argc, char* argv[] ) {
	Ripple ripple;
	RippleUser ru( "Joshua Weaver", "josh@metropark.com");
	ripple.InsertUser( ru );
	cout << ru.user_id << ' '
			<< ru.name << ' '
			<< ru.email << endl;

	RippleTask rt = ripple.CreateTask( ru, "This is a test task\nIsn't it grand." );

	cout << rt.task_id << ' '
		<< rt.state << ' ' 
		<< rt.assigned << endl;

	return 0;
}
