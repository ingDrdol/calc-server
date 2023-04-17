#!/usr/bin/env bash
#testy pro IPk projekt 1

function echo_fail(){
	echo -e "                              [\033[1;31mFail\033[0m]"
}
function echo_pass(){
	echo -e "                              [\033[1;32mPass\033[1;0m]"
}
binfile='./ipkcpd';
host='localhost';
portudp='2025';
porttcp='2026';


function testudp(){ # $1 - name $2 - description
	echo "test $1:";
	echo "-----------------------------------";
	echo "$2";
	wcount=$(cat `echo "testfiles/$1.in"` | `echo "$binfile -h $host -p $portudp -m udp"`| diff testfiles/`echo $1`.out - );
	if [[ -z "$wcount" ]]
	then
		echo_pass;
	else
		cat `echo "testfiles/$1.in"` | `echo "$binfile -h $host -p $portudp -m udp"`;
		echo_fail;
	fi
}

function testtcp(){ # $1 - name $2 - description
	echo "test $1:";
	echo "-----------------------------------";
	echo "$2"
	wcount=$(cat `echo "testfiles/$1.in"` | `echo "$binfile -h $host -p $porttcp -m tcp"`| diff testfiles/`echo $1`.out - );
	if [[ -z "$wcount" ]]
	then
		echo_pass;
	else
		cat `echo "testfiles/$1.in"` | `echo "$binfile -h $host -p $porttcp -m tcp"`;
		echo_fail;
	fi
}

echo "Testing UDP:";
echo "===================================";
testudp udp_add "adding two numbers";
testudp udp_sub "subtracting two numbers";
testudp udp_mul "multiplying two numbers";
testudp udp_div "division with two numbers";
testudp udp_complex "more complex expression";
testudp udp_div_by_zero "division with zero as second argument";

echo "===================================";
echo "";
echo "Testing TCP:";
echo "===================================";
testtcp udp_add "adding two numbers";
testtcp udp_sub "subtracting two numbers";
testtcp udp_mul "multiplying two numbers";
testtcp udp_div "division with two numbers";
testtcp udp_complex "more complex expression";
testtcp udp_div_by_zero "division with zero as second argument";
echo "===================================";