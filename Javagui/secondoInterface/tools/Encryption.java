package tools;

import java.io.InputStream;
import java.io.OutputStream;
import java.security.Key;
import java.security.PublicKey;
import java.security.PrivateKey;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.InvalidKeyException;
import java.security.spec.X509EncodedKeySpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.KeyFactory;
 
import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.IllegalBlockSizeException;


public class Encryption{
  private KeyPair keys = null;
  private static String mode = "RSA";
  private Cipher encCipher = null;
  private Cipher decCipher = null;

  public Encryption createInstance(){
    try{
     KeyPairGenerator keygen = KeyPairGenerator.getInstance(mode);
     keygen.initialize(1024);
     KeyPair rsaKeys = keygen.genKeyPair();
     return new Encryption(rsaKeys);
    } catch(Exception e){
      return null;
    }
  }

  public Encryption(KeyPair kp) throws NoSuchAlgorithmException,NoSuchPaddingException,InvalidKeyException{
     init(kp);
  } 

 
  public Encryption(PublicKey k) throws NoSuchAlgorithmException,NoSuchPaddingException,InvalidKeyException{
     init(new KeyPair(k,null));
  } 

  private void init(KeyPair kp) throws NoSuchAlgorithmException, NoSuchPaddingException,InvalidKeyException{
    keys = kp;
    Key puk = kp.getPublic();
    if(puk!=null){
      encCipher = Cipher.getInstance(mode);
      encCipher.init(Cipher.ENCRYPT_MODE, kp.getPublic());
    }
    Key prk = kp.getPrivate();
    if(prk!=null){
      decCipher = Cipher.getInstance(mode);
      decCipher.init(Cipher.DECRYPT_MODE, kp.getPrivate());
    }
  }

  public byte[] encrypt(byte[] plain){
    try {
       return encCipher!=null?encCipher.doFinal(plain):null;
    } catch(Exception e){
      return null;
    }
  }

  public byte[] decrypt(byte[] crypted){
    try{
       return decCipher!=null?decCipher.doFinal(crypted):null;
    } catch(Exception e){
      return null;
    }
  }

  public PublicKey getPublicKey(){
    return keys.getPublic();
  }


  public static PublicKey generatePublicKey(byte[] bytes){
    try {
        return KeyFactory.getInstance(mode).generatePublic(new X509EncodedKeySpec(bytes));
    } catch(Exception e){
        return null;
    }
  }

  public static PrivateKey generatePrivateKey(byte[] bytes){
     try {
       return KeyFactory.getInstance(mode).generatePrivate(new PKCS8EncodedKeySpec(bytes));
    } catch(Exception e){
        return null;
    }
  }


}
