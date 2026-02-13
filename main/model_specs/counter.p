
%-----DEVS TFF TYPES

tff(internal_transition_type,type,internal_transition : $o).

tff(external_transition_type,type,external_transition : $o).

tff(confluence_transition_type,type,confluence_transition : $o).

tff(output_type,type,output : $o).

tff(time_advance_type,type,time_advance : $real).

tff(time_passed_type,type,time_passed : $real).

tff(infinity_type,type,infinity : $real).

tff(input_rcvd_type,type,input_rcvd: $o).


%-----STATE VARIABLE DEFINITIONS

tff(count_type,type,(count : $real)).

tff(next_count_type,type,(next_count : $real)).

tff(increment_type,type,(increment : $real)).

tff(next_increment_type,type,(next_increment : $real)).

tff(countUp_type,type,(countUp : $o)).

tff(next_countUp_type,type,(next_countUp : $o)).

tff(sigma_type,type,(sigma : $real)).

tff(next_sigma_type,type,(next_sigma : $real)).


%-----INPUT PORT DEFINITIONS

tff(i_port_type,type,(i_port : $tType)).

tff(num_rcvd_type,type,(num_rcvd : i_port > $int)).

tff(direction_in_type,type,(direction_in : i_port)).

tff(val_rcvd_direction_in_type,type,(val_rcvd_direction_in : $o)).

tff(increment_in_type,type,(increment_in : i_port)).

tff(val_rcvd_increment_in_type,type,(val_rcvd_increment_in : $real)).

tff(only_i_ports,axiom,(
	! [IP : i_port] : (
		(IP = direction_in) | 
		(IP = increment_in)))).

tff(i_ports_are_distinct,axiom,
	$distinct(direction_in,increment_in)).

tff(always_pos_num_rcvd,axiom,
	! [IP : i_port] :
		$greatereq(num_rcvd(IP),0)).


%-----OUTPUT PORT DEFINITIONS

tff(o_port_type,type,(o_port : $tType)).

tff(num_output_type,type,(num_output : o_port > $int)).

tff(count_out_type,type,(count_out : o_port)).

tff(val_output_count_out_type,type,(val_output_count_out : $real)).

tff(only_o_ports,axiom,(
	! [OP : o_port] : 
		(OP = count_out))).


%-----INTERNAL TRANSITION FUNCTION AXIOMS

tff(delta_int_axiom_0,axiom,(((
		(internal_transition = $true) & 
		(countUp = $true)) => 
		(next_count = 
		$sum(count,increment))))).

tff(delta_int_axiom_1,axiom,(((
		(internal_transition = $true) & 
		(countUp = $false)) => 
		(next_count = 
		$difference(count,increment))))).

tff(delta_int_axiom_2,axiom,((((
		~(countUp = $true) & 
		~(countUp = $false)) & 
		(internal_transition = $true)) => (((
		(next_count = count) & 
		(next_increment = increment)) & 
		(next_countUp = countUp)) & 
		(next_sigma = sigma))))).


%-----EXTERNAL TRANSITION FUNCTION AXIOMS

tff(delta_ext_axiom_0,axiom,((((
		(external_transition = $true) & 
		(num_rcvd(direction_in) != 0)) & 
		(num_rcvd(increment_in) = 0)) => 
		(next_countUp = val_rcvd_direction_in)))).

tff(delta_ext_axiom_1,axiom,((((
		(external_transition = $true) & 
		(num_rcvd(direction_in) != 0)) & 
		(num_rcvd(increment_in) != 0)) => (
		(next_countUp = val_rcvd_direction_in) & 
		(next_increment = val_rcvd_increment_in))))).

tff(delta_ext_axiom_2,axiom,((((
		(external_transition = $true) & 
		(num_rcvd(direction_in) = 0)) & 
		(num_rcvd(increment_in) != 0)) => 
		(next_increment = val_rcvd_increment_in)))).

tff(delta_ext_axiom_3,axiom,((((
		~(num_rcvd(direction_in) != 0) & 
		~(num_rcvd(direction_in) = 0)) & 
		(external_transition = $true)) => (((
		(next_count = count) & 
		(next_increment = increment)) & 
		(next_countUp = countUp)) & 
		(next_sigma = sigma))))).


%-----CONFLUENCE TRANSITION FUNCTION AXIOMS

tff(delta_con_axiom_0,axiom,(((((
		(confluence_transition = $true) & 
		(num_rcvd(direction_in) != 0)) & 
		(num_rcvd(increment_in) != 0)) & 
		(val_rcvd_direction_in = $true)) => ((
		(next_count = 
		$sum(count,val_rcvd_increment_in)) & 
		(next_increment = val_rcvd_increment_in)) & 
		(next_countUp = val_rcvd_direction_in))))).

tff(delta_con_axiom_1,axiom,(((((
		(confluence_transition = $true) & 
		(num_rcvd(direction_in) != 0)) & 
		(num_rcvd(increment_in) != 0)) & 
		(val_rcvd_direction_in = $false)) => ((
		(next_count = 
		$difference(count,val_rcvd_increment_in)) & 
		(next_increment = val_rcvd_increment_in)) & 
		(next_countUp = val_rcvd_direction_in))))).

tff(delta_con_axiom_2,axiom,(((((
		(confluence_transition = $true) & 
		(num_rcvd(direction_in) != 0)) & 
		(num_rcvd(increment_in) = 0)) & 
		(val_rcvd_direction_in = $true)) => (
		(next_count = 
		$sum(count,increment)) & 
		(next_countUp = val_rcvd_direction_in))))).

tff(delta_con_axiom_3,axiom,(((((
		(confluence_transition = $true) & 
		(num_rcvd(direction_in) != 0)) & 
		(num_rcvd(increment_in) = 0)) & 
		(val_rcvd_direction_in = $false)) => (
		(next_count = 
		$difference(count,increment)) & 
		(next_countUp = val_rcvd_direction_in))))).

tff(delta_con_axiom_4,axiom,(((((
		(confluence_transition = $true) & 
		(num_rcvd(direction_in) = 0)) & 
		(num_rcvd(increment_in) != 0)) & 
		(countUp = $true)) => (
		(next_increment = val_rcvd_increment_in) & 
		(next_count = 
		$sum(count,val_rcvd_increment_in)))))).

tff(delta_con_axiom_5,axiom,(((((
		(confluence_transition = $true) & 
		(num_rcvd(direction_in) = 0)) & 
		(num_rcvd(increment_in) != 0)) & 
		(countUp = $false)) => (
		(next_increment = val_rcvd_increment_in) & 
		(next_count = 
		$difference(count,val_rcvd_increment_in)))))).

tff(delta_con_axiom_6,axiom,(((((
		(confluence_transition = $true) & 
		(num_rcvd(direction_in) = 0)) & 
		(num_rcvd(increment_in) = 0)) & 
		(countUp = $true)) => 
		(next_count = 
		$sum(count,increment))))).

tff(delta_con_axiom_7,axiom,(((((
		(confluence_transition = $true) & 
		(num_rcvd(direction_in) = 0)) & 
		(num_rcvd(increment_in) = 0)) & 
		(countUp = $false)) => 
		(next_count = 
		$difference(count,increment))))).

tff(delta_con_axiom_8,axiom,((((
		~(num_rcvd(direction_in) != 0) & 
		~(num_rcvd(direction_in) = 0)) & 
		(confluence_transition = $true)) => (((
		(next_count = count) & 
		(next_increment = increment)) & 
		(next_countUp = countUp)) & 
		(next_sigma = sigma))))).


%-----LAMBDA AXIOMS

tff(lambda_axiom_0,axiom,((
		(output = $true) => 
		(val_output_count_out = count)))).


%-----TIME ADVANCE AXIOMS

tff(ta_axiom_0,axiom,(
		(time_advance = sigma))).


%-----DEVS TFF AXIOMS

tff(input_was_rcvd,axiom,
	? [IP : i_port] :
		num_rcvd(IP) != 0 => input_rcvd = $true).

tff(input_not_rcvd,axiom,
	! [IP : i_port] :
		num_rcvd(IP) = 0 => input_rcvd = $false).

tff(internal_transition_occurred,axiom,
	((($greatereq(time_passed,time_advance)) & 
		(~input_rcvd)) => (
		(internal_transition = $true) &
		(external_transition = $false) &
		(confluence_transition = $false) &
		(output = $true)))).

tff(external_transition_occurred,axiom,
	((($less(time_passed,time_advance)) & 
		(input_rcvd)) => (
		(internal_transition = $false) &
		(external_transition = $true) &
		(confluence_transition = $false) &
		(output = $false)))).

tff(confluence_transition_occurred,axiom,
	((($greatereq(time_passed,time_advance)) & 
		(input_rcvd)) => (
		(internal_transition = $false) &
		(external_transition = $false) &
		(confluence_transition = $true) &
		(output = $true)))).

tff(infinity_is_greater,axiom,
	infinity = $sum(time_passed,1.0)).

