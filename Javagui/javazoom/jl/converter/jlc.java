/* 12/12/99 JavaLayer 0.0.7		mdm@techie.com
 * Adapted from javalayer and MPEG_Args.
 * Doc'ed and integerated with JL converter. Removed
 * Win32 specifics from original Maplay code.
 *
 * MPEG_Args Based Class - E.B 14/02/99 , JavaLayer
 */



package javazoom.jl.converter;

import java.io.*;

import javazoom.jl.decoder.*;

/**
 * The <code>jlc</code> class presents the JavaLayer
 * Conversion functionality as a command-line program.
 *
 * @since 0.0.7
 */

public class jlc
{

	static public void main(String args[])
	{
		String[] argv;
		long start = System.currentTimeMillis();
		int argc = args.length + 1;
		argv = new String[argc];
		argv[0] = "jlc";
		for(int i=0;i<args.length;i++)
			argv[i+1] = args[i];

		jlcArgs ma = new jlcArgs();
		if (!ma.processArgs(argv))
			System.exit(1);

		Converter conv = new Converter();

		int detail = (ma.verbose_mode ?
					  ma.verbose_level :
				Converter.PrintWriterProgressListener.NO_DETAIL);

		Converter.ProgressListener listener =
			new Converter.PrintWriterProgressListener(
				new PrintWriter(System.out, true), detail);

		try
		{
			conv.convert(ma.filename, ma.output_filename, listener);
		}
		catch (JavaLayerException ex)
		{
			System.err.println("Convertion failure: "+ex);
		}

		System.exit(0);
  }


	/**
	 * Class to contain arguments for maplay.
	 */
	static class jlcArgs
	{
		// channel constants moved into OutputChannels class.
	  //public static final int	both = 0;
	  //public static final int	left = 1;
	  //public static final int	right = 2;
	  //public static final int	downmix = 3;

	  public int				which_c;
	  public int				output_mode;
	  public boolean 			use_own_scalefactor;
	  public float				scalefactor;
	  public String				output_filename;
	  public String				filename;
	  //public boolean 			stdout_mode;
	  public boolean 			verbose_mode;
	  public int				verbose_level = 3;

	  public jlcArgs()
	  {
		which_c = OutputChannels.BOTH_CHANNELS;
		use_own_scalefactor = false;
	   	scalefactor = (float) 32768.0;
	    //stdout_mode = false;
	    verbose_mode = false;
	  }

	  /**
	   * Process user arguments.
	   *
	   * Returns true if successful.
	   */
	  public boolean processArgs(String[] argv)
	  {
		 filename = null;
		 Crc16[] crc;
		 crc = new Crc16[1];
	     int i;
		 int argc = argv.length;

		 //stdout_mode  = false;
	     verbose_mode = false;
	     output_mode = OutputChannels.BOTH_CHANNELS;
	     output_filename = "";
	     if (argc < 2 || argv[1].equals("-h"))
			 return Usage();

	  	 i = 1;
	     while (i < argc)
		 {
		   /* System.out.println("Option = "+argv[i]);*/
		   if (argv[i].charAt(0) == '-')
		   {
			 if (argv[i].startsWith("-v"))
			 {
			 	verbose_mode = true;
				if (argv[i].length()>2)
				{
					try
					{
						String level = argv[i].substring(2);
						verbose_level = Integer.parseInt(level);
					}
					catch (NumberFormatException ex)
					{
						System.err.println("Invalid verbose level. Using default.");
					}
				}
				System.out.println("Verbose Activated (level "+verbose_level+")");
			 }
			 /* else if (argv[i].equals("-s"))
				ma.stdout_mode = true; */
			 else if (argv[i].equals("-p"))
			 {
	      		if (++i == argc)
			  	{
		           System.out.println("Please specify an output filename after the -p option!");
	 	           System.exit (1);
	          	}
		        //output_mode = O_WAVEFILE;
	  		    output_filename = argv[i];
			 }
			 /*else if (argv[i].equals("-f"))
			 {
		        if (++i == argc)
				{
		           System.out.println("Please specify a new scalefactor after the -f option!");
		           System.exit(1);
		   		}
		   	    ma.use_own_scalefactor = true;
		   		// ma.scalefactor = argv[i];
			 }*/
		     else return Usage();
	      }
		  else
		  {
		  	filename = argv[i];
			System.out.println("FileName = "+argv[i]);
			if (filename == null) return Usage();
		  }
		  i++;
	    }
		if (filename == null)
			return Usage();

		return true;
	  }


	   /**
	    * Usage of JavaLayer.
		*/
	   public boolean Usage()
	   {
	  	 System.out.println("JavaLayer Converter V0.0.8 :");
		 System.out.println("  -v[x]         verbose mode. ");
		 System.out.println("                default = 2");
	     /* System.out.println("  -s         write u-law samples at 8 kHz rate to stdout");
	     System.out.println("  -l         decode only the left channel");
	     System.out.println("  -r         decode only the right channel");
	     System.out.println("  -d         downmix mode (layer III only)");
	     System.out.println("  -s         write pcm samples to stdout");
	     System.out.println("  -d         downmix mode (layer III only)");*/
	     System.out.println("  -p name    output as a PCM wave file");
	     System.out.println("");
	     System.out.println("  More info on http://www.javazoom.net");
	     /* System.out.println("  -f ushort  use this scalefactor instead of the default value 32768");*/
		 return false;
	   }
	};
};