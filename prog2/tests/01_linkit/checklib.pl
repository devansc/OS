#!/usr/bin/perl

%found = (
    "lwp_create"	=> 0,
    "lwp_exit"		=> 0,
    "lwp_yield"		=> 0,
    "lwp_start"		=> 0,
    "lwp_stop"		=> 0,   
    "lwp_set_scheduler"	=> 0,
    "lwp_get_scheduler"	=> 0,
    "tid2thread"	=> 0,
    );

# Find the library 
$path = $ENV{"LD_LIBRARY_PATH"};

foreach $d ( split /:/,$path ) {
    $lib = "$d/liblwp.so";
    last if ( -e $lib );		# found it
}

if ( "$lib" eq "" ) {
    print "Library not found.\n";
} else { 
    # look for the required symbols
    print "Checking library for required symbols...";
    open(IN,"nm $lib |") or die "unable to run nm\n";
    while ( $l = <IN> ) {
	if ( $l =~ /\s(\S+)\s*$/ ) {
	    $f = $1;
	    $found{$f} = 1;
	}
    }
    close(IN);
}

$missing = 0;
foreach $f (sort keys %found ) {
    if ( ! $found{$f} ) {
	printf "\n    %s(): symbol not found.",$f;
	$missing++;
    }
}

if ( !$missing ) {
    print "ok.\n";
} else {
    print "\n";
}

exit $missing;


