#!/usr/bin/perl

sub cstr2pod {
    local($_) = shift;
    s/\\"/"/go;
    s/"([^\"]*)"/"C<$1>"/go;
    $_;
}

$section = 1;

@section_order = (
		  'NAME',
		  'SYNOPSIS',
		  'DESCRIPTION',
		  'OPTIONS',
		  'RETURN VALUE',
		  'ERRORS',
		  'EXAMPLES',
		  'ENVIRONMENT',
		  'FILES',
		  'SEE ALSO',
		  'NOTES',
		  'CAVEATS',
		  'WARNINGS',
		  'DIAGNOSTICS',
		  'BUGS',
		  'RESTRICTIONS',
		  'AUTHOR',
		  'AUTHORS',
		  'HISTORY'
		  );

sub type2word {
    my($type) = shift;
    return 'INT' if $type eq 'integer';
    return 'UINT' if $type eq 'uinteger';
    return 'STR' if $type eq 'string' || $type eq 'stringlist';
    return '' if $type eq 'flag' || $type eq 'counter';
    die "Invalid cli option type '$type'";
}

sub add_option {
    my($short, $long, $type, $desc) = @_;

    my $s = '[B<';
    my $o = '=item B<';
    if($short) {
	$s .= "-$short";
	$o .= "-$short";
	if($type) {
	    $s .= " $type";
	    $o .= " $type";
	}
    }
    if($short && $long) {
	$s .= ">]\n[B<";
	$o .= ">, B<";
    }
    if($long) {
	$s .= "--$long";
	$o .= "--$long";
	if($type) {
	    $s .= "=$type";
	    $o .= "=$type";
	}
    }
    $s .= ">]\n";
    $o .= ">\n\n$desc\n\n";

    $synopsis .= $s;
    $options = "=over 8\n\n" unless $options;
    $options .= $o;
}

sub parse_option {
    local($_) = shift;
    s/^\s*\{\s*//o;
    s/\s*\},?\s*/ /o;

    my $short = $1 if s/^'([^\'])',\s*//o;
    die "Invalid cli option" unless $short || s/^0,\s*//o;

    my $long = $1 if s/^"([^\"]+)",\s*//o;
    die "Invalid cli_option" unless $long || s/^0,\s*//o;

    my $type = $1 if s/^cli_option::(\S+),\s*//o;
    die "Invalid cli_option" unless $type;
    $type = &type2word($type);

    my $val = $1 if s/^([^,]+),\s*//o;
    my $var = $1 if s/^&([^,]+),\s*//o;

    my $desc = cstr2pod($1) if s/^"([^,]+)",\s*//o;
    die "Invalid cli_option" unless $desc;
    $desc =~ s/\.?$/./o if $desc;

    my $default = $1 if s/^"([^\"]+)"\s+//o;
    die "Invalid cli_option" unless $default || s/^0\s+//o;
    $desc .= " Defaults to $default." if $default;

    s/\s*\/\/\s*/ /go;
    s/^\s*//o;

    add_option($short, $long, $type, $_ || $desc);
}

sub parse_options {
    $synopsis = "B<$program>\n";

    my $line;
    while(<>) {
	s/^\s+//o;
	s/\s+$//o;
	if($line && /^\{/o) {
	    &parse_option($line);
	    $line = "";
	}
	next if /^\{\s*0\s*\},?/o;
	next if /^\{\s*0\s*,\s*\},?/o;
	last if /^\s*\};/o;
	$line =~ s/$/ $_/;
    }
    &parse_option($line) if $line;

    $synopsis .= "I<$usage>" if $usage;
    $options .= "=back" if $options;
    $sections{'SYNOPSIS'} = $synopsis;
    $sections{'OPTIONS'} = $options;
}

sub parse_notes {
    my $section;
    my $title;
    while(<>) {
	chomp;
	last unless /^$/o || s/^\/\/\s*//o;
	if(/^[\sA-Z]+$/o) {
	    $sections{$title} = $section if $title && $section;
	    undef $section;
	    $title = $_;
	} else {
	    $section .= "$_\n";
	}
    }
    $sections{$title} = $section if $title && $section;
}

sub parse_header_line {
    local($_, $comment) = @_;
    if(s/^\s*const\s+char\s*\*\s*cli_(\S+)\s*=\s*//o) {
	my $name = $1;
	s/;\s*$//o;
	s/^\"//;
	s/\"$//o;
	s/\\n$//o;
	s/\\n""/\n/go;
	$program = $_ if $name eq 'program';
	$prefix = $_ if $name eq 'help_prefix';
	$usage = $_ if $name eq 'args_usage';
	$suffix = $_ if $name eq 'help_suffix';
    }
}

sub parse_header {
    my $comment = '';
    my $line = '';
    while(<>) {
	s/^\s+//o;
	s/\s+$//o;
	if(s/^.*Copyright\s*\(C\)\s*[\d,]+\s*//o) {
	    $author = $_;
	} else {
	    last if ($program && $prefix && /^$/o);
	    next if /^#/o;
	    $comment .= "$1\n" if s|\s*//\s*(.*)$||o;
	    $line =~ s/$/\n$_/;
	    if(/;$/o) {
		&parse_header_line($line, $comment);
		undef $line;
		undef $comment;
	    }
	}
    }
}

sub parse_description {
    while(<>) {
	s/^\s+//o;
	s/\s+$//o;
	last if / cli_options\[\]\s*=\s*\{/o;
	next unless s/^\/\/\s*//o;
	$description .= "$_\n";
    }
}

&parse_header;
&parse_description;
&parse_options;
&parse_notes;

$description .= "\n\n$suffix\n" if $suffix;

$sections{'NAME'} = "$program - $prefix";
$sections{'DESCRIPTION'} = $description;
$sections{'AUTHORS'} = $author if $author;

foreach $section (@section_order) {
    print "=head1 $section\n\n$sections{$section}\n\n"
	if $sections{$section};
}

1;
