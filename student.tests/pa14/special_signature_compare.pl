#!/usr/bin/perl
# Compare two student outputs with the course validation/canonicalization rules.
# The course CLI treats its first input as a canonically ordered reference;
# neither benchmark compiler is that reference. Do not impose reference-only
# top-level presentation order on either output (pa8/lowir.md).
use strict;
use warnings;
my ($common, $a, $b, $source, $out_a, $out_b) = @ARGV;
die "comparison arguments required\n" unless defined $out_b;
sub read_all {
    open my $file, '<', $_[0] or die "$_[0]: $!\n";
    local $/; return <$file>;
}
my $definitions = read_all($common);
$definitions =~ s/\nif \(scalar\(\@ARGV\) == 2 && \$ARGV\[0\] eq 'canonicalize-machine-ir'\).*\z//s
    or die "course comparator entrypoint changed\n";
eval $definitions;
die $@ if $@;
my @data = (read_all($a), read_all($b));
for my $input (@data) {
    my ($valid, $error) = main::validate_lowir_text($input, $source, { strict_presentation_order => 0 });
    die "invalid student LowIR: $error\n" unless $valid;
}
my @canonical = main::canonicalize_lowir_pair_for_compare(@data);
for my $i (0,1) {
    my $path = $i ? $out_b : $out_a;
    open my $file, '>', $path or die "$path: $!\n";
    print $file $canonical[$i]; close $file or die "$path: $!\n";
}
die "course canonical LowIR differs\n" unless $canonical[0] eq $canonical[1];
print "course student validation and canonical comparison PASS\n";
