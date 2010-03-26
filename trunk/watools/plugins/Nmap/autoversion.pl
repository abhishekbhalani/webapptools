#!/usr/bin/perl

$mode = shift;

@lines = ();
srand(localtime());
open (LNS, "version.h") || die "Can't open the file!";
while (<LNS>) {
	$ln = $_;
	if ($mode ne "") {
		# add new build number
		if ($ln =~ /^(\s*static const long REVISION =)\s*(\d+);/) {
			$nm = $2 + int(rand(10));
			$ln = "$1 $nm;\n";
		} else {if ($ln =~ /^(\s*static const long BUILD =)\s*(\d+);/) {
			$nm = $2 + 1;
			$ln = "$1 $nm;\n";
		}}
	} else {
		# increase build count
		if ($ln =~ /^(\s*static const long BUILDS_COUNT =)\s*(\d+);/) {
			$nm = $2 + 1;
			$ln = "$1 $nm;\n";
		}
	}
	push(@lines, $ln);
}
close(LNS);

open (LNS, ">version.h") || die "Can't open the file!";
print LNS @lines;
close(LNS);
