package viewer.hoese.algebras.fixedmregion;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

/**
 * Class Interval represents a time interval.
 * 
 * @author Florian Heinz <fh@sysv.de>
 */
public class Interval {
    /** start and end instant of time interval in ms since epoch */
    private double start, end;
    /** Determines if the interval is left and/or right closed */
    private boolean leftClosed, rightClosed;

    /**
     * Constructs an empty interval.
     * 
     */
    public Interval() {        
    }

    /**
     * Constructs an interval from all parameters.
     * 
     * @param start start instant
     * @param end end instant
     * @param leftClosed true, if interval is left closed
     * @param rightClosed true, if interval is right closed
     */
    public Interval(double start, double end, boolean leftClosed, boolean rightClosed) {
        this.start = start;
        this.end = end;
        this.leftClosed = leftClosed;
        this.rightClosed = rightClosed;
    }

    /**
     * Constructs an interval from start and end instant.
     * The interval is closed on both ends.
     * 
     * @param start start instant
     * @param end end instant
     */
    public Interval(double start, double end) {
        this.start = start;
        this.end = end;
        this.leftClosed = this.rightClosed = true;
    }

    /**
     * Calculate the itnersection with a given interval.
     * Also handles the closed attributes correctly.
     * 
     * @param iv the interval to intersect with
     * @return intersection interval
     */
    public Interval intersect (Interval iv) {
        Interval ret = new Interval();
        ret.setStart(Math.max(iv.getStart(), getStart()));
        ret.setEnd(Math.min(iv.getEnd(), getEnd()));
        ret.setLeftClosed(iv.leftClosed && leftClosed);
        ret.setRightClosed(iv.rightClosed && rightClosed);
        if ((ret.start > ret.end) || (ret.start == ret.end && (!ret.leftClosed || !ret.rightClosed)))
            return null;
        
        return ret;
    }
    
    /**
     * Get the fraction of the interval for currentTime.
     * Returns for example 0.5 if currentTime is in the middle between
     * start and end
     * 
     * @param currentTime
     * @return fraction of interval
     */
    public double project(double currentTime) {
        return ((double) (currentTime - getStart())) / ((double) (getEnd() - getStart()));
    }

    /**
     * Get the start instant
     * 
     * @return start instant
     */
    public double getStart() {
        return start;
    }

    /**
     * Set the start instant
     * 
     * @param start start instant
     */
    public void setStart(double start) {
        this.start = start;
    }

    /**
     * Get the end instant
     * 
     * @return end instant
     */
    public double getEnd() {
        return end;
    }

    /**
     * Set the end instant
     * 
     * @param end end instant
     */
    public void setEnd(double end) {
        this.end = end;
    }

    /**
     * Convert this interval to its Nested List representation.
     * 
     * @return Nested List representation of this interval
     */
    public NL toNL() {
        NL interval = new NL();
        interval.addStr(getDate(start));
        interval.addStr(getDate(end));
        interval.addBoolean(isLeftClosed());
        interval.addBoolean(isRightClosed());
        
        return interval;

    }
    
    /**
     * Construct an interval from its Nested List representation.
     * 
     * @param nl Nested List representation of an interval
     */
    public static Interval fromNL(NL nl) {
        Interval iv = new Interval();
        
        iv.setStart(parseDate(nl.get(0)));
        iv.setEnd(parseDate(nl.get(1)));
        iv.setLeftClosed(nl.get(2).getBool());
        iv.setRightClosed(nl.get(3).getBool());
        
        return iv;
    }

    /**
     * Test if interval is left closed.
     * 
     * @return true, if interval is left closed
     */
    public boolean isLeftClosed() {
        return leftClosed;
    }

    /**
     * Set if interval is left closed.
     * 
     * @param leftClosed true, if interval is left closed
     */
    public void setLeftClosed(boolean leftClosed) {
        this.leftClosed = leftClosed;
    }

    /**
     * Test if interval is right closed.
     * 
     * @return true, if interval is right closed
     */
    public boolean isRightClosed() {
        return rightClosed;
    }

    /**
     * Set if interval is right closed.
     * 
     * @param rightClosed true, if interval is right closed
     */
    public void setRightClosed(boolean rightClosed) {
        this.rightClosed = rightClosed;
    }
    
    /**
     * Test, if instant time is inside this interval.
     * 
     * @param time instant to test
     * @return true, if time is inside the interval
     */
    public boolean inside (double time) {
        return ((time > start) && (time < end) ||
                (time == start && leftClosed) ||
                (time == end && rightClosed)); 
    }
    
    /**
     * Get the fraction of the interval at instant time.
     * Returns for example 0.5 if time is in the middle between end and start.
     * 
     * @param time the instant to get the fraction
     * @return fraction of interval at time
     */
    public double getFrac (double time) {
        return (time-start)/(end-start);
    }
    
    /**
     * Convert a date string to an instant
     * 
     * @param date The date string
     * @return an instant
     */
    private static Double parseDate(String date) {
        String[] formats = {
            "yyyy-MM-DD-HH:mm:ss.SS",
            "yyyy-MM-DD-HH:mm:ss",
            "yyyy-MM-DD-HH:mm",
            "yyyy-MM-DD",};

        for (String format : formats) {
            try {
                SimpleDateFormat sdf = new SimpleDateFormat(format);
                sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
                double ret = sdf.parse(date).getTime();

                return ret;
            } catch (Exception e) {
            }
        }

        return null;
    }
    
    /**
     * Convert a nested list date string to an instant
     * 
     * @param nl The nested list string element
     * @return an instant
     */
    public static Double parseDate (NL nl) {
        return parseDate(nl.getStr());
    }

    /**
     * Convert an instant to a date string
     * 
     * @param time the instant to convert
     * @return the date string
     */
    private static String getDate (double time) {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-DD-HH:mm:ss");
        sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
        Date d = new Date((long)(time));
        
        return sdf.format(d);
    }
}
