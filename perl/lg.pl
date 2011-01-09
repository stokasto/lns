sub lg {
    my $x = shift;
    my $tol = shift || 1e-13;
    my $ip = 0.0;
 
    while ($x < 1) {
	$ip -= 1;
	$x   *= 2;
    }
    while ($x >= 2) {
	$ip += 1;
	$x   /= 2;
    }
    
    my $frac = 0.;
    my $fp = 1.0;
    while ($fp >= $tol) {
	$fp /= 2;
	$x  *= $x;
	if ($x >= 2) {
		$x   /= 2;
		$frac += $fp;
	}
    }

    printf "integral part = %g\n", $ip;
    printf "integral part = %g\n", $frac;
    $ip + $frac;
}
 
printf "x = %g\nlg(x) = %g\n", 4.5, lg(4.5);
printf "x = %g\nlg(x) = %g\n", 0.35335, lg(0.35335);
