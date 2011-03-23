var ripple = {
	backendurl: '',
	page: 'assigned',
	user_id: $.cookie( 'user_id' ),
	email: $.cookie( 'email' ),
	name: $.cookie( 'name' ),
	show_event_stream: false,
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

ripple.task_states = {
	RTS_OPEN:0,
	RTS_ACCEPTED:1,
	RTS_STARTED:2,
	RTS_COMPLETED:3,
	RTS_CLOSED:4, //positive confirmation
	RTS_CANCELED:5
}

ripple.change_view = function( page ) {
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
	ripple.query( 'get_my_tasks', {}, function( data ) {
		$("#assigned_tasks")
			.html( '<legend class="ui-corner-all ui-widget-header task_region">assigned to you</legend>' )
			.show();
		$("#stakeholder_tasks")
			.html( '<legend class="ui-corner-all ui-widget-header task_region">you own</legend>' )
			.show();
		$.each( data.tasks, function( index, task ) { 
			ripple.query( 'get_task', { task_id: task }, function( data ) {
				ripple.process_task( data.task );
			});
		}); 
		$('<div style="clear:both;">').appendTo("#assigned_tasks");
		$('<div style="clear:both;">').appendTo("#stakeholder_tasks");
	});
};

ripple.delete_task = function( task ) {
	//Remove if it exists.
	$( '#task_' + task.task_id ).fadeOut( 'slow' , function() { $( this ).remove(); });
}

ripple.process_task = function( task ) {
	if ( task.state >= ripple.task_states.RTS_CLOSED )
		ripple.delete_task( task );
	else if ( task.assigned == ripple.user_id ) {
		if ( $( '#assigned_tasks #task_' + task.task_id ).length ) 
			ripple.update_task( task );
		else {
			ripple.delete_task( task );
			ripple.generate_task_dom( task ).prependTo('#assigned_tasks').fadeIn('slow');
		}
	}
	else if ( task.stakeholder == ripple.user_id ) {
		if ( $( '#stakeholder_tasks #task_' + task.task_id ).length ) 
			ripple.update_task( task );
		else {
			ripple.delete_task( task );
			ripple.generate_task_dom( task ).prependTo('#stakeholder_tasks').fadeIn('slow');
		}
	}
	else
		ripple.delete_task( task );
}

ripple.update_task = function( task ) {
		var old_task = $( '#task_' +  task.task_id );
		old_task.removeAttr( 'id' );
		var new_task = ripple.generate_task_dom( task );
		old_task.replaceWith( new_task );
		new_task.show();
}

ripple.generate_task_dom = function( task ) {
	var task_container = $('<div>')
		.attr( 'id', 'task_' + task.task_id )
		.addClass('task_container ui-helper-hidden ');
	var task_element =  $('<div>')
		.addClass('task ui-widget-content ui-corner-all ui-selectable shadow')
		.appendTo( task_container );
	if ( task.state == 0 && task.stakeholder != ripple.user_id )
		task_element.addClass( 'need_attention' );
	var taskInfo = $('<div class="task_info ui-widget-header ui-corner-right ">')
		.css( 'cursor', 'pointer' )
		.click( function( event ) {
			if ( event.target != this ) return;
			if ( $(this).hasClass( 'task_info_content' ) ) {
				$(this)
					.css( 'cursor', 'pointer' )
					.css( 'z-index', '' )
					.removeClass('task_info_content shadow');
				$(this).children().hide();
			}
			else {
				$(this)
					.css( 'z-index', 10 )
					.css( 'cursor', 'default' )
					.addClass('task_info_content shadow');
				$(this).children().show();
				ripple.query( "get_possible_actions", { task_id: task.task_id }, function( data ) {
					var task_actions = $( '#task_' + data.task_id + ' .task_actions' );
					task_actions.html('');
					var action_list = $('<div>');
					for ( i=0; i < data.possible_actions.length; i++ ) {
						ripple.add_action( task.task_id, data.possible_actions[i], action_list );
					}
					action_list.accordion({ animated: false,
													collapsible: true,
													autoHeight: false,
													active: false
					});
					action_list.children().css( 'readonly: false');
					action_list.appendTo( task_actions );
				});
			}
		})
		.appendTo( task_container );
	if ( ripple.user_id != task.assigned ) {
		if ( task.assigned_avatar )
			$('<img src="' + task.assigned_avatar + '" class="avatar" />').prependTo( taskInfo );
		else
			$('<img src="anonymous.png" class="avatar" />').prependTo( taskInfo );
		$('<div>')
			.addClass( "assigned" )
			.html( "Assigned: " + task.assigned_name )
			.appendTo( taskInfo );
	}
	else {
		if ( task.stakeholder_avatar )
			$('<img src="' + task.stakeholder_avatar + '" class="avatar" />').prependTo( taskInfo );
		else
			$('<img src="anonymous.png" class="avatar" />').prependTo( taskInfo );
		$('<div>')
			.addClass( "stakeholder" )
			.html( "Stakeholder: " + task.stakeholder_name )
			.appendTo( taskInfo );
	}
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

	for( i = task.logs.length - 1; i >= 0; i-- ) {
		var log = task.logs[i]
		var log_entry = $('<div class="log_entry">')
			.attr( 'title', "click to show details" )
			.addClass('log_flavor_' + task.logs[i].flavor);



		if ( task.logs[i].subject != "" ) {
			var avatar_img =  ( task.logs[i].user_avatar ) ? '<img src="' + task.logs[i].user_avatar  + '" class="small_avatar" />' : '<img src="anonymous.png" class="small_avatar" />';

			$('<p>')
				.addClass('log_subject')
				.addClass('log_flavor_' + task.logs[i].flavor )
				.text( task.logs[i].subject )
				.append( ripple.get_action_img( task.logs[i].flavor ) ) 
				.prepend( avatar_img )
				.appendTo( log_entry );
			log_entry
				.css( 'cursor', 'pointer' )
				.click( function() {
					$( this ).children().not('.log_subject').toggle();
				});
		}
		else {
			$('<p>')
				.addClass( 'log_details' )
				.html( ripple.get_action_img( task.logs[i].flavor ) + 
						'user: ' + task.logs[i].user_name + '<br />' + 
						'['  + ripple.flavor_name( task.logs[i].flavor ) + '] ' +
						ripple.pretty_datetime( new Date( task.logs[i].created_date ) ) )
				.prepend( '<img class="small_avatar" src="' + task.logs[i].user_avatar  + '" />' )
				.appendTo( log_entry );
		}
		


		if ( task.logs[i].body != "" ) {
			log_entry
				.attr( 'title', "click to show full text and details" );
			$('<p>')
				.html( "..." )
				.appendTo( log_entry );
			$('<p>')
				.addClass('log_body ui-helper-hidden log_flavor_' + task.logs[i].flavor )
				.text( task.logs[i].body )
				.appendTo( log_entry );
		}

		if ( task.logs[i].subject != "" ) {
			$('<p>')
				.addClass('log_details ui-helper-hidden top_line log_flavor_' + task.logs[i].flavor )
				.html( 'user: ' + task.logs[i].user_name + '<br />' + 
						'['  + ripple.flavor_name( task.logs[i].flavor ) + '] ' +
						ripple.pretty_datetime( new Date( task.logs[i].created_date ) ) )
				.appendTo( log_entry );
		}

		if ( i != 0 )
			log_entry.appendTo( task_element );
		else
			log_entry.prependTo( task_element );

	};

	$('<div class="task_status ui-state-error ui-corner-bl ui-corner-br">')
		.text( ripple.state_name( task.state) )
		.prependTo( task_element );

	taskInfo.children().hide();

	return task_container;
};

ripple.flavor_name = function( flavor_id ) {
	switch ( flavor_id ) {
		case ripple.task_flavors.RLF_CREATED:
			return 'created';
		case ripple.task_flavors.RLF_NOTE:
			return 'note';
		case ripple.task_flavors.RLF_FORWARDED:
			return 'forward';
		case ripple.task_flavors.RLF_FEEDBACK:
			return 'feedback';
		case ripple.task_flavors.RLF_DECLINED:
			return 'task';
		case ripple.task_flavors.RLF_ACCEPTED:
			return 'accept';
		case ripple.task_flavors.RLF_STARTED:
			return 'start';
		case ripple.task_flavors.RLF_COMPLETED:
			return 'complete';
		case ripple.task_flavors.RLF_REOPENED:
			return 'reopen';
		case ripple.task_flavors.RLF_CLOSED:
			return 'close';
		case ripple.task_flavors.RLF_CANCELED:
			return 'cancel';
		default:
			return '';
	}
}

ripple.state_name = function( state ) {
	switch ( state ) {
		case ripple.task_states.RTS_OPEN:
			return "OPEN";
		case ripple.task_states.RTS_ACCEPTED:
			return "ACCEPTED";
		case ripple.task_states.RTS_STARTED:
			return "STARTED";
		case ripple.task_states.RTS_COMPLETED:
			return "COMPLETED";
		case ripple.task_states.RTS_CLOSED:
			return "CLOSED";
		case ripple.task_states.RTS_CANCELED:
			return "CANCELED";
		default:
				return "unknown";
	}
}

ripple.pretty_datetime = function( datum ) {
	var today = new Date();
	var date_string;
	var time_string;


	if ( datum.toDateString() == today.toDateString() )
		date_string = "today";
	else {
		date_string = Number( datum.getMonth() + 1 ) + '/' + datum.getDate();	
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

ripple.get_action_img = function( action_id ) {
	switch ( action_id ) {
		case ripple.task_flavors.RLF_NOTE:
			return '<img class="action_img" src="note.png" />';
		case ripple.task_flavors.RLF_FORWARDED:
			return '<img class="action_img" src="forward.png" />';
		case ripple.task_flavors.RLF_FEEDBACK:
			return '<img class="action_img" src="feedback.png" />';
		case ripple.task_flavors.RLF_DECLINED:
			return '<img class="action_img" src="decline.png" />';
		case ripple.task_flavors.RLF_ACCEPTED:
			return '<img class="action_img" src="accept.png" />';
		case ripple.task_flavors.RLF_STARTED:
			return '<img class="action_img" src="start.png" />';
		case ripple.task_flavors.RLF_COMPLETED:
			return '<img class="action_img" src="complete.png" />';
		case ripple.task_flavors.RLF_REOPENED:
			return '<img class="action_img" src="reopen.png" />';
		case ripple.task_flavors.RLF_CLOSED:
			return '<img class="action_img" src="close.png" />';
		case ripple.task_flavors.RLF_CANCELED:
			return '<img class="action_img" src="cancel.png" />';
		default:
			return '';
	}
}

ripple.add_action = function( task_id, action_id, accordion ) {
	var content = '<textarea cols="12" rows="3" placeholder="Subject is first line or first 30 characters."></textarea>';
	var tac = $('<div class="task_action_content">').html( content );
	switch ( action_id ) {
		case ripple.task_flavors.RLF_NOTE:
			var title_text = ripple.get_action_img( action_id ) + 'add note';
			var button_text = "add";
			break;
		case ripple.task_flavors.RLF_FORWARDED:
			var title_text = ripple.get_action_img( action_id ) + 'forward this task';
			var button_text = "forward";
			tac.append( "Forward to:<br />" );
			$('<select class="user_list">')
				.appendTo( tac );
			ripple.load_user_lists();
			break;
		case ripple.task_flavors.RLF_FEEDBACK:
			var title_text = ripple.get_action_img( action_id ) + 'request feedback';
			var button_text = "request";
			break;
		case ripple.task_flavors.RLF_DECLINED:
			var title_text = ripple.get_action_img( action_id ) + 'decline task';
			var button_text = "decline";
			break;
		case ripple.task_flavors.RLF_ACCEPTED:
			var title_text = ripple.get_action_img( action_id ) + 'accept this task';
			var button_text = "accept";
			break;
		case ripple.task_flavors.RLF_STARTED:
			var title_text = ripple.get_action_img( action_id ) + 'start this task';
			var button_text = "start";
			break;
		case ripple.task_flavors.RLF_COMPLETED:
			var title_text = ripple.get_action_img( action_id ) + 'mark as complete';
			var button_text = "complete";
			break;
		case ripple.task_flavors.RLF_REOPENED:
			var title_text = ripple.get_action_img( action_id ) + 'reopen this task';
			var button_text = "reopen";
			break;
		case ripple.task_flavors.RLF_CLOSED:
			var title_text = ripple.get_action_img( action_id ) + 'close permanently';
			var button_text = "close";
			break;
		case ripple.task_flavors.RLF_CANCELED:
			var title_text = ripple.get_action_img( action_id ) + 'cancel this task';
			var button_text = "cancel";
			break;
		default:
			throw "Invalid action: " + action_id;
	}

	var h3 = $('<h3>').appendTo( accordion );
	$('<a href="#">')
		.html( title_text )
		.appendTo( h3 );
	tac.appendTo(accordion);
	$('<input type="button" action="' + action_id +
		'" task_id="' + task_id +
		'" value="' + button_text  + '">')
		.button()
		.click( function() {
			ripple.query( "do_action", { 
					action: $( this ).attr( 'action' ), 
					description: $( this ).siblings('textarea').val(),
					task_id: $( this ).attr( 'task_id' ),
					forwardee_id: $( this ).siblings('.user_list').val()
				}, function( data ) {
					//TODO: Clean this up
					if (data.error_msg != '' )
						alert( data.error_msg );
				}
			);
		})
		.addClass( "action_button" )
		.appendTo( tac );
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
		ripple.process_task( task );
	}
}

ripple.load_user_lists = function() {
	ripple.query( "get_users", {}, function( data ) {
		var users_data = [];
		$.each( data.users, function( index, user ) {
			users_data.push('<option value="'+ user.user_id +'">'+ user.name + '</option>');
		});

		$('select.user_list:empty')
			.html( users_data.join('') )
			.selectmenu({
				style:'popup',
				width: '8em'
			});
	});
}

ripple.get_attention = function() {
	$('.need_attention').effect( 'highlight', {}, 3000 );
}


ripple.calculate_subject = function( description ) {
	var subject_rx =/^((.{1,30})( +|$\n?)|(.{1,30}))/g;
	var match = description.match( subject_rx );
	return match;
}

ripple.render_stream = function( ) {
	if ( !ripple.show_event_stream ) {
		$('#stream')
			.html( '<span class="ui-icon ui-icon-triangle-1-s"></span>' );
	}
	else {
		ripple.query( "get_stream", {}, function( data ) {
		$('#stream')
			.html( '<span class="ui-icon ui-icon-triangle-1-n"></span><hr />' );

			for( i=0; i < data.logs.length; i++ ) {
				var log = data.logs[i]
				var log_entry = $('<div class="log_entry">')
										.attr( 'title', "click to show details" );


				if ( log.subject != "" ) {
					$('<p>')
						.addClass('log_subject')
						.text( log.subject )
						.append( ripple.get_action_img( log.flavor ) ) 
						.prepend( '<img class="small_avatar" src="' + log.user_avatar  + '" />' )
						.appendTo( log_entry );
					log_entry
						.css( 'cursor', 'pointer' )
						.click( function() {
								$( this ).children().not('.log_subject').toggle();
							});
				}
				else {
					$('<p>')
						.addClass( 'log_details' )
						.html( ripple.get_action_img( log.flavor ) + 
								'user: ' + log.user_name + '<br />' + 
								'['  + ripple.flavor_name( log.flavor ) + '] ' +
								ripple.pretty_datetime( new Date( log.created_date ) ) )
						.prepend( '<img class="small_avatar" src="' + log.user_avatar  + '" />' )
						.appendTo( log_entry );
				}



				if ( log.body != "" ) {
					log_entry
						.attr( 'title', "click to show full text and details" );
					$('<p>')
						.html( "..." )
						.appendTo( log_entry );
					$('<p>')
						.addClass('log_body ui-helper-hidden log_flavor_' + log.flavor )
						.text( log.body )
						.appendTo( log_entry );
				}

				if ( log.subject != "" ) {
					$('<p>')
						.addClass('log_details ui-helper-hidden top_line log_flavor_' + log.flavor )
						.html( 'user: ' + log.user_name + '<br />' + 
								'['  + ripple.flavor_name( log.flavor ) + '] ' +
								ripple.pretty_datetime( new Date( log.created_date ) ) )
						.appendTo( log_entry );
				}
				$('#stream').append( log_entry );
			};
		});
	}
	ripple.show_event_stream = !ripple.show_event_stream;
}
