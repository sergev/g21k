#!/usr/zeus1/perl-4.036/perl
#
$cc = "wcc386p ";
$cflags = "-zW -d2 -w4 ";
$link = "wlink ";
do get_files();
while($#files>=0)
{
  do handle_file(pop(@files));
}

foreach $key (sort(keys %names))
{
    print $key,'=',$names{$key}, "\n";
}

sub get_files 
{
    local($prev_line)=0;
    open(inp,@ARGV[0]);
    open(outp,">" . @ARGV[1]);
    while(<inp>)
    {
	if (/g386.*-c.* \.*\/*(.*\.c)/ || ($prev_line && /.*\/(.*\.c)/))
	{
	    local($new_f) = do new_filename($1);
	    print outp $new_f, "\n";
	    $new_f=~/(.*).c/;
#	    print outp $cc , $cflags , $new_f, "\n";
	}
	$prev_line = (($prev_line || /g386/) && /\\$/);
#	if (/g386 .* -o *(cc1) /)
#	{
#	    print outp $link, "-o ", $1;
#	    local(@to_link)=split(/ /);
#	    while($#to_link >=0)
#	    {
#		local($obj)=pop(@to_link);
#		if ($obj =~ /.*\.o/)
#		{
#		    print outp " ",$obj;
#		}
#	    }
#	    print outp "\n";
#	}
    }
    close(inp);
    close(outp);
}


sub handle_file
{
    local($filename)=@_;
    local($new_filename) = do new_filename($filename);
    open($inp,$filename);
    open(outp,">". $new_filename);
    while(<$inp>)
    {
	s/(\#include.*\")(.*)(\")/$1.do new_filename($2).$3/e;
	print outp $_;
    }
    close($inp);
}
	
sub new_filename
{
    local($in_name)=@_;
    if(! $names{$in_name})
    {
	local(@x)=keys %names;
	$in_name !~ /(.*)(\..*)/;
	$names{$in_name}="file" . ($#x+1) . $2;
	push(files,$in_name);
    }
    return $names{$in_name};
}
	
	

