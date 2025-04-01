all:	adsl_fec_example adsl_fec_test

adsl_fec_example:	adsl_fec_example.cc adsl_rs.h
	g++ -Wall -Wno-misleading-indentation -O2 -o adsl_fec_example adsl_fec_example.cc

adsl_fec_test:	adsl_fec_test.cc adsl_rs.h
	g++ -Wall -Wno-misleading-indentation -O2 -o adsl_fec_test adsl_fec_test.cc

uat_fec_test:	uat_fec_test.cc uat_rs.h
	g++ -Wall -Wno-misleading-indentation -O2 -o uat_fec_test uat_fec_test.cc

clean:
	rm adsl_fec_example adsl_fec_test uat_fec_test

