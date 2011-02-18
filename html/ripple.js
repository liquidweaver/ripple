var ripple = {
	backendurl: '',
	page: 'assigned',
	user_id: $.cookie( 'user_id' ),
	email: $.cookie( 'email' ),
	name: $.cookie( 'name' )
};

ripple.task_flavors = {
	RLF_CREATED:0,
	RLF_NOTE:1,
	RLF_FORWARDED:2,
	RLF_FEEDBACK:3,
	RLF_DECLINED:4,
	RLF_ACCEPTED:5,
	RLF_STARTED:6,
	RLF_COMPLETED:7,
	RLF_REOPENED:8,
	RLF_CLOSED:9,
	RLF_CANCELED:10
};

ripple.change_view = function( page ) {
	//stubbin
	//watching assigned accepted stakeholder
	var content = $('#content').html('');
	$('<div>')
		.attr( 'id', 'tasks' )
		.appendTo( content );

	if ( page != 'tasks' ) {
		$("#assigned_tasks").hide();
		$("#stakeholder_tasks").hide();
	}

	ripple.page = page;

	switch( page ) {
		case 'tasks':
			ripple.load_all_assigned_tasks();	
			break;
		case 'profile':
			$('#name').val( ripple.name );
			$('#email').val( ripple.email );
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
		$("#assigned_tasks")
			.html( '<legend class="ui-corner-all ui-widget-header task_region">assigned to you</legend>' )
			.show();
		$("#stakeholder_tasks")
			.html( '<legend class="ui-corner-all ui-widget-header task_region">you own</legend>' )
			.show();
		$.each( data, function( index, task ) { 
			ripple.add_task( task.possible_actions, task.task_data );
		}); 
		$('<div style="clear:both;">').appendTo("#assigned_tasks");
		$('<div style="clear:both;">').appendTo("#stakeholder_tasks");
	});
};

ripple.add_task = function( actions, task_data ) {
	var task =  $('<div>')
		.addClass('task ui-helper-hidden ui-widget-content ui-corner-all ui-selectable shadow');
	var taskInfo = $('<div class="task_info ui-widget-header ui-corner-right">')
		.hover( function() {
			$(this).addClass('task_info_content');
			$(this).children().show();
		},
		function() {
			$(this).removeClass('task_info_content');
			$(this).children().hide();
		})
		.appendTo( task );
	$('<img src="anonymous.png" class="avatar" />').prependTo( taskInfo );
	$('<div>')
		.addClass( "stakeholder" )
		.html( task_data.stakeholder_name )
		.appendTo( taskInfo );
	if ( task_data.start_date != "" ) {
		var start_date = new Date( task_data.start_date );
		$('<div>')
			.addClass( "task_time" )
			.html( "Starts " + ripple.pretty_datetime( start_date ) )
			.appendTo( taskInfo );
	}
	if ( task_data.due_date != "" ) {
		var due_date = new Date( task_data.due_date );
		$('<div>')
			.addClass( "task_time" )
			.html( "Due by " + ripple.pretty_datetime( due_date ) )
			.appendTo( taskInfo );
	}

	$('<hr />')
		.addClass( 'task_info' )
		.appendTo( taskInfo );

	var i = 0;
	$.each( actions, function( index, action ) {
		$('<span>')
			.html( ripple.get_action_link( action ) )
			.appendTo( taskInfo );
	});


	$.each( task_data.logs, function( index, log ) {
		var log_entry = $('<div>')
			.addClass('log_entry_flavor_' + log.flavor)
			.appendTo( task );

		log_entry.addClass( "log_flavor_" + log.flavor );
		if ( log.subject != "" ) {
			$('<p>')
				.addClass('log_subject')
				.addClass('log_flavor_' + log.flavor )
				.html( log.subject )
				.appendTo( log_entry );
		}

		if ( log.body != "" ) {
			$('<p>')
				.addClass('log_body')
				.addClass('log_flavor_' + log.flavor )
				.html( log.body.replace( /\n/g, '<br />\n' ) )
				.appendTo( log_entry );
		}

		taskInfo.children().hide();
		console.log( ripple );
		console.log( task_data );
		if ( task_data.assigned == ripple.user_id )   
			task.appendTo('#assigned_tasks');
		else if ( task_data.stakeholder == ripple.user_id )
			task.appendTo('#stakeholder_tasks');
		task.fadeIn('slow');
	});
};

ripple.pretty_datetime = function( datum ) {
	var today = new Date();
	var date_string;
	var time_string;


	if ( datum.toDateString() == today.toDateString() )
		date_string = "today";
	else {
		date_string = datum.getMonth() + '/' + datum.getDate();	
		if ( datum.getFullYear() != today.getFullYear() )
			date_string  += '/' + datum.getFullYear()
	}

	if ( !( datum.getHours() == 0 && datum.getSeconds() == 0 ) ) { 
		var seconds = datum.getSeconds();
		if ( seconds < 10 )
			seconds = String( 0 ) + seconds;

		if ( datum.getHours() > 12 ) 
			time_string = String(datum.getHours() - 12) + ':' + seconds + ' pm';
		else
			time_string = datum.getHours() + ':' + seconds + ' am';

		return date_string + ", " + time_string;
	}
	else
		return date_string;


};

ripple.get_action_link = function( action_id ) {
	var image_name;
	switch ( action_id ) {
	case ripple.task_flavors.RLF_NOTE:
		image_name = "note.png";
		title_text = "add a note";
		break;
	case ripple.task_flavors.RLF_FORWARDED:
		image_name = "forward.png"
		title_text = "forward this task";
		break;
	case ripple.task_flavors.RLF_FEEDBACK:
		image_name = "feedback.png"
		title_text = "request feedback";
		break;
	case ripple.task_flavors.RLF_DECLINED:
		image_name = "decline.png"
		title_text = "decline task";
		break;
	case ripple.task_flavors.RLF_ACCEPTED:
		image_name = "accept.png"
		title_text = "accept this task";
		break;
	case ripple.task_flavors.RLF_STARTED:
		image_name = "start.png"
		title_text = "start working on this task";
		break;
	case ripple.task_flavors.RLF_COMPLETED:
		image_name = "complete.png"
		title_text = "mark this task as complete";
		break;
	case ripple.task_flavors.RLF_REOPENED:
		image_name = "reopen.png"
		title_text = "reopen this task";
		break;
	case ripple.task_flavors.RLF_CLOSED:
		image_name = "close.png"
		title_text = "close this task permanently";
		break;
	case ripple.task_flavors.RLF_CANCELED:
		image_name = "cancel.png"
		title_text = "cancel this task";
		break;
	default:
		throw "Invalid action: " + action_id;
	}

	return "<img src=\"" + image_name + "\" title=\"" + title_text + "\" class=\"action_img\"/>";
};
