package secondoPostgisUtil;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import java.util.logging.Logger;
import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.SecretKeySpec;
import sun.misc.BASE64Decoder;
import sun.misc.BASE64Encoder;

public class ReadWriteDES
{
  public final String strSCHLUSSELALGORITHMUS = "DES";
  
  public String encode(byte[] bytes, String pass)
  {
    String s = "";
    try
    {
      Cipher c = Cipher.getInstance("DES");
      Key k = new SecretKeySpec(pass.getBytes(), "DES");
      c.init(1, k);
      
      ByteArrayOutputStream out = new ByteArrayOutputStream();
      OutputStream cos = new CipherOutputStream(out, c);
      cos.write(bytes);
      cos.close();
      
      s = new BASE64Encoder().encode(out.toByteArray());
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
    catch (InvalidKeyException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
    catch (NoSuchAlgorithmException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
    catch (NoSuchPaddingException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
    return s;
  }
  
  public String decode(String s, String pass)
  {
    ByteArrayOutputStream bos = null;
    try
    {
      Cipher c = Cipher.getInstance("DES");
      Key k = new SecretKeySpec(pass.getBytes(), "DES");
      c.init(2, k);
      
      byte[] decode = new BASE64Decoder().decodeBuffer(s);
      InputStream is = new ByteArrayInputStream(decode);
      
      bos = new ByteArrayOutputStream();
      CipherInputStream cis = new CipherInputStream(is, c);
      
      int b = 0;
      while ((b = cis.read()) != -1) {
        bos.write(b);
      }
      cis.close();
    }
    catch (IOException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
    catch (InvalidKeyException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
    catch (NoSuchAlgorithmException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
    catch (NoSuchPaddingException e)
    {
      LogFileHandler.mlogger.warning(e.getMessage());
    }
    if (bos != null) {
      return bos.toString();
    }
    return "";
  }
}
