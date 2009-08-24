#!/usr/bin/perl

$inf = shift;

if (open (OUT, "$inf")) {
    @lines = <OUT>;
    close OUT;
    foreach $line (@lines)
    {
        $line =~ s/\s+/ /g;
        $line =~ s/\"/\\\"/g;
    }

    if (open (OUT, ">$inf")) {
	print OUT "static const string xrc=\"";
        print OUT @lines;
	print OUT "\";\n";
    }
}
