#!/usr/bin/perl -w

#    Author: James Manning <jmm@users.sf.net>
#       This software may be used and distributed according to the terms of
#       the GNU General Public License, http://www.gnu.org/copyleft/gpl.html
#
#    Description:
#       Perl wrapper for calling tiobench.pl and displaying results
#       graphically using gnuplot

use strict;

my $args = join(" ",@ARGV);
my %input_fields; my %output_fields; my %values_present;
my %data; my $dir; my $size; my $blk; my $thr; my $read; my $read_cpu;
my $field;           my $write; my $write_cpu; my $seek; my $seek_cpu;
open(TIO,"tiobench.pl $args 2> /dev/null |") or die "failed on tiobench";

while(<TIO> !~ m/^---/) {} # get rid of header stuff

while(my $line = <TIO>) {
   $line =~ s/^\s+//g; # remove any leading whitespace
   ($input_fields{'dir'},    $input_fields{'size'}, 
    $input_fields{'blk'},    $input_fields{'thr'},

    $output_fields{'read'},  $output_fields{'read_cpu'},
    $output_fields{'write'}, $output_fields{'write_cpu'},
    $output_fields{'seek'},  $output_fields{'seek_cpu'}
      ) = split(/[\s%]+/, $line);
   foreach $field (keys %input_fields) { # mark values that appear
      $values_present{$field}{$input_fields{$field}}=1;
   }
   foreach $field (keys %output_fields) { # mark values that appear
      $data{$input_fields{'dir'}}{$input_fields{'thr'}}{$field}
         =$output_fields{$field};
   }
}

my $gnuplot_input = "\n".
   "set terminal png medium color;\n".
   "set output \"reads.png\";\n".
   "set title \"Reads\";\n".
   "set xlabel \"Threads\";\n".
   "set ylabel \"MB/s\";\n".
   "plot ";

my @gnuplot_files;

foreach my $dir (sort keys %{$values_present{'dir'}}) {
   my $file="read_dir=$dir";
   $file =~ s#/#_#g;
   push(@gnuplot_files,"\"$file\" with lines");
   open(FILE,"> $file") or die $file;
   foreach my $thr (sort {$a <=> $b} keys %{$values_present{'thr'}}) {
      print FILE "$thr $data{$dir}{$thr}{'read'}\n";
      print "DEBUG: $thr $data{$dir}{$thr}{'read'}\n";
   }
   close(FILE);
}

$gnuplot_input .= join(", ",@gnuplot_files) . ";\n";

print "DEBUG: feeding gnuplot $gnuplot_input";

open(GNUPLOT,"|gnuplot") or die "could not run gnuplot";
print GNUPLOT $gnuplot_input;
close(GNUPLOT);
