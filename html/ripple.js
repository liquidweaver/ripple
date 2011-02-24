var ripple = {
	backendurl: '',
	page: 'assigned',
	user_id: $.cookie( 'user_id' ),
	email: $.cookie( 'email' ),
	name: $.cookie( 'name' ),
	event_sequence: -1
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
			ripple.refresh_profile();
			break;
		default:
			alert( 'Error: view not found.' );
	}
};

ripple.query = function( method, args, callback ) {
	
	args.method = method;
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
			ripple.add_task( task );
		}); 
		$('<div style="clear:both;">').appendTo("#assigned_tasks");
		$('<div style="clear:both;">').appendTo("#stakeholder_tasks");
	});
};

ripple.delete_task = function( task ) {
	//Remove if it exists.
	$( '#' + task.task_id ).fadeOut( 'slow' , function() { $( this ).remove(); });
}

ripple.add_task = function( task ) {
	var task_container = $('<div>')
		.addClass('task_container ui-helper-hidden ')
		.attr( 'id', 'task_' + task.task_id );
	var task_element =  $('<div>')
		.addClass('task ui-widget-content ui-corner-all ui-selectable shadow')
		.appendTo( task_container );
	var taskInfo = $('<div class="task_info ui-widget-header ui-corner-right">')
		.hover( function() {
			$(this)
				.css( 'z-index', 10 )
				.addClass('task_info_content');
			$(this).children().show();
			ripple.query( "get_possible_actions", { task_id: task.task_id }, function( data ) {
				var task_actions = $( '#task_' + data.task_id + ' .task_actions' );
				task_actions.html('');
				var action_list = $('<div>');
				for ( i=0; i < data.possible_actions.length; i++ ) {
					ripple.add_action( data.possible_actions[i], action_list );
				}
				action_list.accordion({ animated: false,
												collapsible: true,
												autoHeight: false,
												active: false
				});
				action_list.appendTo( task_actions );
			});
		},
		function() {
			$(this)
				.css( 'z-index', '' )
				.removeClass('task_info_content');
			$(this).children().hide();
		})
		.appendTo( task_container );
	if ( task.stakeholder_avatar )
		$('<img src="' + task.stakeholder_avatar + '" class="avatar" />').prependTo( taskInfo );
	else
		$('<img src="anonymous.png" class="avatar" />').prependTo( taskInfo );
	$('<div>')
		.addClass( "stakeholder" )
		.html( task.stakeholder_name )
		.appendTo( taskInfo );
	if ( task.start_date != "" ) {
		var start_date = new Date( task.start_date );
		$('<div>')
			.addClass( "task_time" )
			.html( "Starts " + ripple.pretty_datetime( start_date ) )
			.appendTo( taskInfo );
	}
	if ( task.due_date != "" ) {
		var due_date = new Date( task.due_date );
		$('<div>')
			.addClass( "task_time" )
			.html( "Due by " + ripple.pretty_datetime( due_date ) )
			.appendTo( taskInfo );
	}

	$('<hr />')
		.addClass( 'task_info' )
		.appendTo( taskInfo );
	$('<div>')
		.addClass( "task_actions" )
		.appendTo( taskInfo );

	var i = 0;

	$.each( task.logs, function( index, log ) {
		var log_entry = $('<div>')
			.addClass('log_entry_flavor_' + log.flavor)
			.appendTo( task_element );

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
		if ( task.assigned == ripple.user_id )   
			task_container.prependTo('#assigned_tasks');
		else if ( task.stakeholder == ripple.user_id )
			task_container.prependTo('#stakeholder_tasks');
		task_container.fadeIn('slow');
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

ripple.add_action = function( action_id, accordian ) {
	var content = '<textarea rows="3" cols="18"></textarea>'
	switch ( action_id ) {
		case ripple.task_flavors.RLF_NOTE:
			title_text = "add note";
			break;
		case ripple.task_flavors.RLF_FORWARDED:
			title_text = "forward this task";
			break;
		case ripple.task_flavors.RLF_FEEDBACK:
			title_text = "request feedback";
			break;
		case ripple.task_flavors.RLF_DECLINED:
			title_text = "decline task";
			break;
		case ripple.task_flavors.RLF_ACCEPTED:
			title_text = "accept this task";
			break;
		case ripple.task_flavors.RLF_STARTED:
			title_text = "start working";
			break;
		case ripple.task_flavors.RLF_COMPLETED:
			title_text = "mark as complete";
			break;
		case ripple.task_flavors.RLF_REOPENED:
			title_text = "reopen this task";
			break;
		case ripple.task_flavors.RLF_CLOSED:
			title_text = "close permanently";
			break;
		case ripple.task_flavors.RLF_CANCELED:
			title_text = "cancel this task";
			break;
		default:
			throw "Invalid action: " + action_id;
	}

	var h3 = $('<h3>').appendTo( accordian );
	$('<a href="#">')
		.html( title_text )
		.appendTo( h3 );
	$('<div class="task_action_content">')
		.html( content )
		.appendTo(accordian);
}

ripple.refresh_profile = function() {
	ripple.query( "get_user", { user_id: $.cookie( "user_id" ) }, function( data ) {
			$('#name').val( data.name );
			$('#email').val( data.email );
			if ( data.avatar_file )
				$('#avatar_preview').html( '<img src="' + data.avatar_file + '" class="avatar" />' );
			else
				$('#avatar_preview').html( '<img src="anonymous.png" class="avatar" />' );
			});
};

ripple.event_manager = function() {

	params = {
		type: 'POST',
		global: false,
		dataType:'text',
		async: true,
		cache: false,
		data: { events: ripple.user_id },
		url: 'http://' + window.location.hostname + ':8081/EVENTS.LIST',
		success: function( data ) {
			ripple.parse_events( data );
			ripple.event_manager();
		},
		error: function( e ) {
			//Wait 5 seconds before we try again
			setTimeout( ripple.event_manager, 5000 );
		},
		timeout: function() {
			alert( 'timeout' );
			ripple.event_manager();
		}
	};

	if ( ripple.event_sequence != -1 ) {
		params.beforeSend = function( xhr ) {
			xhr.setRequestHeader( 'X-RESTCOMET-SEQUENCE', ripple.event_sequence + 1 );
		};
	}
	$.ajax( params );
	
}

ripple.parse_events = function( data ) {
	var event_rx=/S\[(\d+)\] G\[\d+] T\[\d+] L\[(\d+)\]\s+/g
	while( m = event_rx.exec( data ) ) {
		if ( m[1] > ripple.event_sequence )
			ripple.event_sequence = Number( m[1] );
		var task = eval('(' + data.substr( event_rx.lastIndex, m[2]  ) + ')');
		ripple.delete_task( task );
		ripple.add_task( task );
	}
}
