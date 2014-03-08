use strict;

use constant {
	PI => 3.14159265
};

my $rotation_tics = 8;
my $block_size = 16;

print <<EOF;
static const struct offset {
	int dx, dy;
} rotation_offsets[4][FALLING_BLOCK_ROTATION_TICS] = {
EOF

for my $r (0 .. 3) {
	my ($from, $to) = ($r*.5*PI, ($r + 1)*.5*PI);

	print "\t{ ";

	for my $i (0 .. $rotation_tics - 1) {
		my $angle = $from + ($i/$rotation_tics)*($to - $from);
		my ($dx, $dy) = (int cos($angle)*$block_size, int sin($angle)*$block_size);
		print "{ $dx, $dy }, ";
	}

	print "},\n";
}

print <<EOF;
};
EOF
