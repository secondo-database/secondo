/*
 * 12/12/99	 0.0.7 Renamed class, additional constructor arguments 
 *			 and larger write buffers. mdm@techie.com.
 *
 * 15/02/99 ,Java Conversion by E.B ,ebsp@iname.com, JavaLayer
 */
package javazoom.jl.converter;

import  java.io.RandomAccessFile;
import javazoom.jl.decoder.Obuffer;

/**
 * Implements an Obuffer by writing the data to
 * a file in RIFF WAVE format. 
 *  
 * @since 0.0
 */


public class WaveFileObuffer extends Obuffer
{
  private short[] 		buffer;
  private short[] 		bufferp;
  private int 			channels;
  private WaveFile      outWave;

  /**
   * Creates a new WareFileObuffer instance. 
   * 
   * @param number_of_channels	
   *				The number of channels of audio data
   *				this buffer will receive. 
   * 
   * @param freq	The sample frequency of the samples in the buffer.
   * 
   * @param fileName	The filename to write the data to.
   */
  public WaveFileObuffer(int number_of_channels, int freq, String FileName)
  {
  	if (FileName==null)
		throw new NullPointerException("FileName");
	
	buffer = new short[OBUFFERSIZE];
	bufferp = new short[MAXCHANNELS];
	channels = number_of_channels;
	
	for (int i = 0; i < number_of_channels; ++i) 
		bufferp[i] = (short)i;
	
	outWave = new WaveFile();
	
    int rc = outWave.OpenForWrite (FileName,freq,(short)16,(short)channels);
  }

  /**
   * Takes a 16 Bit PCM sample.
   */
  public void append(int channel, short value)
  {
    buffer[bufferp[channel]] = value;
    bufferp[channel] += channels;
  }

  /**
   * Write the samples to the file (Random Acces).
   */
  short[] myBuffer = new short[2];
  public void write_buffer(int val)
  {
	  
    int k = 0;
    int rc = 0;
    
	rc = outWave.WriteData(buffer, bufferp[0]);
	// REVIEW: handle RiffFile errors. 
	/*
    for (int j=0;j<bufferp[0];j=j+2)
    {
        
        //myBuffer[0] = (short)(((buffer[j]>>8)&0x000000FF) | ((buffer[j]<<8)&0x0000FF00));
        //myBuffer[1] = (short) (((buffer[j+1]>>8)&0x000000FF) | ((buffer[j+1]<<8)&0x0000FF00));
        myBuffer[0] = buffer[j];
        myBuffer[1] = buffer[j+1];
        rc = outWave.WriteData (myBuffer,2);
    }
	*/
    for (int i = 0; i < channels; ++i) bufferp[i] = (short)i;
  }

  public void close()
  {
    outWave.Close(); 
  }
  
  /**
   *
   */
  public void clear_buffer()
  {}

  /**
   *
   */
  public void set_stop_flag()
  {}

  /*
   * Create STDOUT buffer
   *
   *
  public static Obuffer create_stdout_obuffer(MPEG_Args maplay_args)
  {
  	Obuffer thebuffer = null;
    int mode = maplay_args.MPEGheader.mode();
    int which_channels = maplay_args.which_c;
     if (mode == Header.single_channel || which_channels != MPEG_Args.both)
        thebuffer = new FileObuffer(1,maplay_args.output_filename);
     else
        thebuffer = new FileObuffer(2,maplay_args.output_filename);
     return(thebuffer);
  }
  */
}
