#!/usr/bin/env perl

use strict;
use warnings;
use autodie;
use JSON::PP;

die "Usage: $0 <JSON>\n" unless @ARGV;

open my $fh, "<", shift;
my $json = do { local $/; <$fh> };
close $fh;

my $patterns = decode_json($json);

print "/* THIS FILE IS AUTO-GENERATED. DO NOT EDIT MANUALLY. */\n";
print "const PatternDef pattern_defs[] = {\n";
for my $p (@$patterns) {
	print "\t{",
		"{$p->{rows}, $p->{cols}, \"$p->{rule}\"},",
		" \"$p->{name}\", \"$p->{desc}\", \"$p->{rle}\"",
	"},\n";
}
print "\t{{0}, NULL, NULL, NULL}\n};\n";
