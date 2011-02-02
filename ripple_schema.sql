CREATE TABLE users (
	user_id INTEGER PRIMARY KEY NOT NULL,
	user_name TEXT NOT NULL,
	user_email text NOT NULL
);

CREATE TABLE tasks (
	task_id INTEGER PRIMARY KEY NOT NULL,
	stakeholder INTEGER NOT NULL REFERENCES users,
	assigned INTEGER REFERENCES users,
	start_date INTEGER,
	due_date INTEGER,
	state INTEGER,
	parent_task INTEGER REFERENCES tasks
);

CREATE TABLE logs (
	log_id INTEGER PRIMARY KEY NOT NULL,
	flavor INTEGER NOT NULL,
	body TEXT NOT NULL,
	user_id INTEGER NOT NULL REFERENCES users,
	task_id INTEGER NOT NULL REFERENCES tasks,
	created_date INTEGER NOT NULL
);

CREATE TABLE watchers (
	task_id NOT NULL REFERENCES tasks,
	user_id NOT NULL REFERENCES users
);
