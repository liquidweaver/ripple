var ripple = {
	backendurl: '',
	page: 'assigned',
};

ripple.change_view = function( page ) {
	//stubbin
	//watching assigned accepted stakeholder
	var content = $('#content').html('');
	$('<div>')
		.attr( 'id', 'tasks' )
		.appendTo( content );

	switch( page ) {
		case 'watching':
			break;
		case 'assigned':
			break;
		case 'stakeholder':
			ripple.load_all_assigned_tasks();	
			break;
		default:
			alert( 'Error: view not found.' );
	}
};

ripple.query = function( method, args, callback ) {
	
	args.method = method;
	//$.post( '/ajax/query', args, callback, 'json' );  
	$.ajax({
		type: 'POST',
		url: '/ajax/query',
		data: args,
		success: callback,
		dataType: 'json'
	});
};

ripple.load_all_assigned_tasks = function() {
	ripple.query( 'get_assigned_tasks', {}, function( data ) {
		$.each( data, function( index, task ) { 
			ripple.add_task( task );
		}); 
	});
};

ripple.add_task = function( task_data ) {
	var task =  $('<div>').addClass('task').appendTo('#tasks');
	$('<span>')	
		.addClass('task_stakeholder')
		.html( task_data.stakeholder_name )
		.appendTo( task );

	var description = $('<div>').addClass('task_description').appendTo( task );
	$('<span>')
		.addClass('log_subject')
		.html( task_data.logs[0].subject )
		.appendTo( description );
	if ( task_data.logs[0].body != "" ) {
		$('<span>')
			.addClass('log_body')
			.html( task_data.logs[0].body )
			.appendTo( description );
	}

	$.each( task_data.logs, function( index, log ) {
		var log_entry = $('<div>').appendTo( task );

		if ( index == 0 )
			log_entry.addClass('task_description')
		else
			log_entry.addClass('log_entry');

		log_entry.addClass( log.flavor );

		$('<span>')
			.addClass('log_subject')
			.html( log.subject )
			.appendTo( log_entry );

		if ( log.body != "" ) {
			$('<span>')
				.addClass('log_body')
				.html( log.body )
				.appendTo( log_entry );
		}
		//TODO: Actions

	});

};
