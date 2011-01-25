#ifndef RIPPLE_H
#define RIPPLE_h
#include "RippleUser.hpp"
#include "RippleTask.hpp"
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

using namespace soci;


backend_factory const &backEnd = *soci::factory_sqlite3();
class Ripple {
public:
	Ripple() {
		sql.open( backEnd, "ripple.db"  );
		sql << "PRAGMA foreign_keys = ON";
	}

	void InsertUser( RippleUser& ru ) {
		assert( ru.user_id == -1 );
		sql << "INSERT INTO users VALUES (:user_id, :name, :email)",
			 use( static_cast<const RippleUser&>( ru ) );

		sql << "SELECT last_insert_rowid()", into( ru.user_id );
	}

	void InsertTask( RippleTask& task ) {

		sql << "INSERT INTO tasks VALUES ("
			<< ":task_id, :stakeholder, :assigned, :start_date,"
			<<" :due_date, :state, :parent_task)",
		use( static_cast<const RippleTask&>( task ) );

		sql << "SELECT last_insert_rowid()", into( task.task_id );
	}

protected:
	session sql;
};
#endif//RIPPLE_H
