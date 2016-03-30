package appGuiUtil;

import java.util.Locale;
import javax.swing.JOptionPane;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

/*
 * This class is used for determination of the UI language
 * Language in this context are either German (DE) or English (DE)
 */

public class SetUI
{
  public void setUIAndLanguage()
  {
    try
    {
      UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
    }
    catch (ClassNotFoundException e1)
    {
      e1.printStackTrace();
    }
    catch (InstantiationException e1)
    {
      e1.printStackTrace();
    }
    catch (IllegalAccessException e1)
    {
      e1.printStackTrace();
    }
    catch (UnsupportedLookAndFeelException e1)
    {
      e1.printStackTrace();
    }
    Locale.setDefault(new Locale("en", "US"));
    JOptionPane.setDefaultLocale(Locale.getDefault());
  }
}