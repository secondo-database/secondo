import java.math.*;

class NumberTest {


    public static void main (String[] args) {

	int n = 1000000;
	System.out.println("n = "+n);

	System.out.println("\ndouble:");
	double d1 = Math.PI;
	System.out.println("initial value for double: "+d1);

	long time1 = System.currentTimeMillis();
	for (int i = 1; i <= n; i++) {
	    d1 = d1 + (d1 / i); } 
	long time2 = System.currentTimeMillis();
	System.out.println("result: "+d1);
	System.out.println("computed in: "+(time2-time1)+"ms");

	System.out.println("\nRational:");
	Rational r1 = new Rational(Math.PI);
	System.out.println("initial value for Rational: "+r1);

	long time3 = System.currentTimeMillis();
	for (int i = 1; i <= n; i++) {
	    r1 = r1.plus(r1.dividedby(i)); }
	long time4 = System.currentTimeMillis();
	System.out.println("result: "+r1);
	System.out.println("computed in: "+(time4-time3)+"ms");

	/*
	System.out.println("\nBigDecimal:");
	BigDecimal b1 = new BigDecimal(Math.PI);
	System.out.println("initial value for BigDecimal: "+b1);
	
	long time5 = System.currentTimeMillis();
	for (int i = 1; i <= n; i++) {
	    b1 = b1.add(b1.divide(new BigDecimal(i),0)); }
	long time6 = System.currentTimeMillis();
	System.out.println("result: "+b1);
	System.out.println("computed in: "+(time6-time5)+"ms");
	*/

    }//end main



}//end class NumberTest
